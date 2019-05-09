#include "poseoptimizer2.h"
#include <cmath>
#include <QtMath>

#include "pinholecamera.h"

using namespace std;
using namespace Eigen;

void test_exp()
{
    auto generate_random_unit_vector_3f = [] () -> Vector3f
    {
        return Vector3f((rand() % 2000) / 1000.0f - 1.0f,
                        (rand() % 2000) / 1000.0f - 1.0f,
                        (rand() % 2000) / 1000.0f - 1.0f).normalized();
    };

    for (int i = 0; i < 100; ++i)
    {
        Vector3f x = generate_random_unit_vector_3f();
        Matrix3f R = exp_rotationMatrix(x);
        Vector3f x1 = ln_rotationMatrix(R);
        float s1 = (x1 - x).array().abs().sum();
        assert(s1 < numeric_limits<float>::epsilon());
    }
}

float optimize_pose(Matrix3f & R, Vector3f & t,
                    const std::shared_ptr<const PinholeCamera> & camera,
                    const Vectors3f & controlModelPoints,
                    const Vectors2f & imagePoints,
                    float maxDistance,
                    int numberIterations)
{
    Matrix<float, 6, 1> x;
    x.segment<3>(0) = t;
    x.segment<3>(3) = ln_rotationMatrix(R);
    float E = optimize_pose(x, camera, controlModelPoints, imagePoints, maxDistance, numberIterations);
    t = x.segment<3>(0);
    R = exp_rotationMatrix(x.segment<3>(3));
    return E;
}


float optimize_pose(Matrix<float, 6, 1> & x,
                    const std::shared_ptr<const PinholeCamera> & camera,
                    const Vectors3f & modelPoints,
                    const Vectors2f & imagePoints,
                    float maxDistance,
                    int numberIterations)
{
    /*float weightFunction_k2 = 1.0f / (3.0f * maxDistance * maxDistance);
    float weightFunction_k1 = 1.0f / (maxDistance * (1.0f - weightFunction_k2 * maxDistance * maxDistance));

    auto weightFunction = [&] (float x) -> float
    {
        return (x >= maxDistance) ? 1.0f : (weightFunction_k1 * x * (1.0f - (x * x) * weightFunction_k2));
    };

    auto dif_weightFunction = [&] (float x) -> float
    {
        return (x >= maxDistance) ? 1.0f : (weightFunction_k1 - 3.0f * weightFunction_k1 * weightFunction_k2 * x * x);
    };

    auto get_x_weightFunction = [&] (float y) -> float
    {
        float b = - 1.0f / weightFunction_k2;
        float c = y / (weightFunction_k1 * weightFunction_k2);
        float Q = ( - 3.0f * b) / 9.0f;
        float R = (27.0f * c) / 54.0f;

        float t = acos(R / sqrt(Q * Q * Q)) / 3.0f;
        float x = - 2.0f * sqrt(Q) * cos(t - (2.0f / 3.0f) * static_cast<float>(M_PI));

        return x;
    };*/

    auto weightFunction = [&] (float x) -> float
    {
        return x;
    };

    auto dif_weightFunction = [&] (float x) -> float
    {
        return 1.0f;
    };

    auto get_x_weightFunction = [&] (float y) -> float
    {
        return y;
    };

    size_t numberPoints = modelPoints.size();

    Vector2f focalLength = camera->pixelFocalLength();
    Vector2f opticaCenter = camera->pixelOpticalCenter();

    Vector3f w;
    Matrix3f R;
    Vector3f t;
    Matrix3f rW;

    auto getJacobianAndResidual = [&]
            (Matrix<float, 1, 6> & J_x, Matrix<float, 1, 6> & J_y, Vector2f & e,
             const Vector3f & point, const Vector2f & imagePoint) -> bool
    {
        //Matrix3f rJ = R * skewMatrix(point) * rW;
        Matrix3f rJ = exp_jacobian(w, point);

        Vector3f v = R * point + t;
        if (v.z() < 1e-5f)
            return false;

        float z_inv = 1.0f / v.z();
        float z_inv_squared = z_inv * z_inv;

        Vector2f f = v.segment<2>(0) * z_inv;

        e = Vector2f(f.x() * focalLength.x() + opticaCenter.x(),
                     f.y() * focalLength.y() + opticaCenter.y()) - imagePoint;

        Vector2f k(dif_weightFunction(e.x()) * focalLength.x(),
                   dif_weightFunction(e.y()) * focalLength.y());
        e.x() = weightFunction(e.x());
        e.y() = weightFunction(e.y());

        /*J_x(0) = - z_inv;                 // - 1 / z
        J_x(1) = 0.0;                     // 0
        J_x(2) = x * z_inv_squared;       // x / z^2
        J_x(3) = y * J_x(2);              // x * y / z^2
        J_x(4) = - (1.0 + x * J_x(2));    // -(1.0 + x^2 / z^2)
        J_x(5) = y * z_inv;               // y / z

        J_y(0) = 0.0;                     // 0
        J_y(1) = - z_inv;                 // - 1 / z
        J_y(2) = y * z_inv_squared;       // y / z^2
        J_y(3) = 1.0 + y * J_y(2);        // 1.0 + y^2 / z^2
        J_y(4) = - J_x(3);                // -x * y / z^2
        J_y(5) = - x * z_inv;             // - x / z*/

        J_x(0) = z_inv * k.x();
        J_x(1) = 0.0f;
        J_x(2) = - v.x() * z_inv_squared * k.x();
        J_x(3) = (- (v.x() * v.y()) / (v.z() * v.z())) * k.x();
        J_x(4) = (1.0f + (v.x() * v.x()) / (v.z() * v.z())) * k.x();
        J_x(5) = (- v.y() / v.z()) * k.x();
        //J_x(3) = (rJ(0, 0) * v.z() - v.x() * rJ(2, 0)) * z_inv_squared * k.x();
        //J_x(4) = (rJ(0, 1) * v.z() - v.x() * rJ(2, 1)) * z_inv_squared * k.x();
        //J_x(5) = (rJ(0, 2) * v.z() - v.x() * rJ(2, 2)) * z_inv_squared * k.x();

        J_y(0) = 0.0f;
        J_y(1) = z_inv * k.y();
        J_y(2) = - v.y() * z_inv_squared * k.y();
        J_y(3) = - (1.0f + (v.y() * v.y()) / (v.z() * v.z())) * k.y();
        J_y(4) = ((v.x() * v.y()) / (v.z() * v.z())) * k.y();
        J_y(5) = (v.x() / v.z()) * k.y();
        //J_y(3) = (rJ(1, 0) * v.z() - v.y() * rJ(2, 0)) * z_inv_squared * k.y();
        //J_y(4) = (rJ(1, 1) * v.z() - v.y() * rJ(2, 1)) * z_inv_squared * k.y();
        //J_y(5) = (rJ(1, 2) * v.z() - v.y() * rJ(2, 2)) * z_inv_squared * k.y();

         return true;
    };

    auto getResidual = [&] (Vector2f & e, const Vector3f & point, const Vector2f & imagePoint) -> bool
    {
        Vector3f v = R * point + t;
        if (v.z() < 1e-5f)
            return false;

        Vector2f f = v.segment<2>(0) / v.z();


        e = Vector2f(f.x() * focalLength.x() + opticaCenter.x(),
                     f.y() * focalLength.y() + opticaCenter.y()) - imagePoint;
        e.x() = weightFunction(e.x());
        e.y() = weightFunction(e.y());

        //if ((e.x() == 0.0f) || (e.y() == 0.0f))
        //    return false;
        return true;
    };

    using MinimizationInfo = tuple<Matrix<float, 6, 6>, Matrix<float, 6, 1>, float, size_t>;
    auto computeMinimzationInfo = [&] (size_t begin_index, size_t end_index) -> MinimizationInfo
    {
        size_t count = 0;
        Matrix<float, 6, 6> JtJ = Matrix<float, 6, 6>::Zero();
        Matrix<float, 6, 1> Je = Matrix<float, 6, 1>::Zero();
        float Fsq = 0.0f;
        Matrix<float, 1, 6> J_i_x, J_i_y;
        Vector2f e;
        for (size_t i = begin_index; i < end_index; ++i)
        {
            if (!getJacobianAndResidual(J_i_x, J_i_y, e, modelPoints[i], imagePoints[i]))
                continue;
            JtJ += J_i_x.transpose() * J_i_x;
            Je += J_i_x.transpose() * e.x();
            JtJ += J_i_y.transpose() * J_i_y;
            Je += J_i_y.transpose() * e.y();
            Fsq += e.dot(e);
            count += 2;
        }
        return make_tuple(JtJ, Je, Fsq, count);
    };
    auto computeResiduals = [&] (size_t begin_index, size_t end_index) -> tuple<float, size_t>
    {
        size_t count = 0;
        float Fsq = 0.0f;
        Vector2f e;
        for (size_t i = begin_index; i < end_index; ++i)
        {
            if (!getResidual(e, modelPoints[i], imagePoints[i]))
                continue;
            Fsq += e.dot(e);
            count += 2;
        }
        return make_tuple(Fsq, count);
    };

    float firstError = - 1.0f;

    float Fsq = numeric_limits<float>::max();
    float factor = 1e-5f;

    //QSemaphore semaphore;
    //size_t workPartSize = static_cast<size_t>(ceil(numberPoints / numberWorkThreads));

    for (int iter = 0; iter < numberIterations; ++iter)
    {
        t = x.segment<3>(0);
        w = x.segment<3>(3);
        R = exp_rotationMatrix(w);
        float lw = w.dot(w);
        if (lw < 1e-5f)
        {
            R = Matrix3f::Identity();
            rW = - Matrix3f::Identity();
        }
        else
        {
            rW = ((w * w.transpose() + (R.transpose() - Matrix3f::Identity()) * skewMatrix(w)) / (- lw));
        }

        Matrix<float, 6, 6> JtJ;
        Matrix<float, 6, 1> Je;
        size_t count;
        {
            tie(JtJ, Je, Fsq, count) = computeMinimzationInfo(0, numberPoints);
        }
        Fsq /= static_cast<float>(count);
        if (firstError < 0.0f)
            firstError = Fsq;
        float Fsq_next = Fsq;
        for (int n_try = 0; n_try < 10; ++n_try)
        {
            JtJ.diagonal() += Matrix<float, 6, 1>::Ones() * factor;
            Matrix<float, 6, 1> dx = JtJ.ldlt().solve(Je).eval();
            Matrix<float, 6, 1> x_next = x - dx;

            t = x_next.segment<3>(0);
            R = exp_rotationMatrix(x_next.segment<3>(3));

            count = 0;
            Fsq_next = 0.0;
            {
                tie(Fsq_next, count) = computeResiduals(0, numberPoints);
            }
            Fsq_next /= static_cast<float>(count);

            if (Fsq_next < Fsq)
            {
                x = x_next;
                break;
            }
            else
            {
                factor *= 2.0f;
            }
        }
        float deltaFsq = Fsq - Fsq_next;
        Fsq = Fsq_next;
        if (deltaFsq < numeric_limits<float>::epsilon())
        {
            break;
        }
    }
    return get_x_weightFunction(sqrt(Fsq));
}
