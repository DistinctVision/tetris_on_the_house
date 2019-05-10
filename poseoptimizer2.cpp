#include "poseoptimizer2.h"
#include <cmath>
#include <QtMath>

#include "pinholecamera.h"

#include <opencv2/imgproc.hpp>

using namespace std;
using namespace Eigen;

void draw_residuals(const cv::Mat & image,
                    Matrix3f & R, Vector3f & t,
                    const shared_ptr<const PinholeCamera> & camera,
                    const Vectors3f & controlModelPoints, const Vectors2f & imagePoints)
{
    assert(controlModelPoints.size() == imagePoints.size());
    for (size_t i = 0; i < controlModelPoints.size(); ++i)
    {
        const Vector3f & point = controlModelPoints[i];
        const Vector2f & imagePoint1 = imagePoints[i];
        Vector2f imagePoint2 = camera->project((R * point + t).eval());
        cv::line(image, cv::Point2f(imagePoint1.x(), imagePoint1.y()),
                        cv::Point2f(imagePoint2.x(), imagePoint2.y()), cv::Scalar(0, 0, 255), 2);
    }
}

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
    Matrix<double, 6, 1> x;
    x.segment<3>(0) = t.cast<double>();
    x.segment<3>(3) = ln_rotationMatrix(R).cast<double>();
    double E = optimize_pose(x, camera, controlModelPoints, imagePoints,
                             static_cast<double>(maxDistance), numberIterations);
    t = x.segment<3>(0).cast<float>();
    R = exp_rotationMatrix(x.segment<3>(3).eval()).cast<float>();
    return static_cast<float>(E);
}


double optimize_pose(Matrix<double, 6, 1> & x,
                     const std::shared_ptr<const PinholeCamera> & camera,
                     const Vectors3f & modelPoints,
                     const Vectors2f & imagePoints,
                     double maxDistance,
                     int numberIterations)
{
    double weightFunction_k2 = 1.0 / (3.0 * (maxDistance * maxDistance));
    double weightFunction_k1 = 1.0 / ((maxDistance) * (1.0 - weightFunction_k2 * (maxDistance * maxDistance)));

    auto weightFunction = [&] (double x) -> double
    {
        return (x >= maxDistance) ? 1.0 : (weightFunction_k1 * x * (1.0 - (x * x) * weightFunction_k2));
    };

    auto dif_weightFunction = [&] (double x) -> double
    {
        return (x >= maxDistance) ? 0.0 : (weightFunction_k1 - 3.0 * weightFunction_k1 * weightFunction_k2 * x * x);
    };

    auto get_x_weightFunction = [&] (double y) -> double
    {
        double b = - 1.0 / weightFunction_k2;
        double c = y / (weightFunction_k1 * weightFunction_k2);
        double Q = ( - 3.0 * b) / 9.0;
        double R = (27.0 * c) / 54.0;

        double t = acos(R / sqrt(Q * Q * Q)) / 3.0;
        double x = - 2.0 * sqrt(Q) * cos(t - (2.0 / 3.0) * (M_PI));

        return x;
    };

    /*auto weightFunction = [&] (double x) -> double
    {
        return x;
    };

    auto dif_weightFunction = [&] (double x) -> double
    {
        return 1.0;
    };

    auto get_x_weightFunction = [&] (double y) -> double
    {
        return y;
    };*/

    size_t numberPoints = modelPoints.size();

    Vector2d focalLength = camera->pixelFocalLength().cast<double>();
    Vector2d opticaCenter = camera->pixelOpticalCenter().cast<double>();

    Vector3d w;
    Matrix3d R;
    Vector3d t;
    Matrix3d rW;

    auto getJacobianAndResidual = [&]
            (Matrix<double, 1, 6> & J_x, Matrix<double, 1, 6> & J_y, Vector2d & e,
             const Vector3f & point, const Vector2f & imagePoint) -> bool
    {
        Matrix3d rJ = R * skewMatrix(point.cast<double>().eval()) * rW;
        //Matrix3d rJ = exp_jacobian(w, point.cast<double>());

        Vector3d v = R * point.cast<double>() + t;
        if (v.z() < 1e-5)
            return false;

        double z_inv = 1.0 / v.z();
        double z_inv_squared = z_inv * z_inv;

        Vector2d f = v.segment<2>(0) * z_inv;

        e = Vector2d(f.x() * focalLength.x() + opticaCenter.x(),
                     f.y() * focalLength.y() + opticaCenter.y()) - imagePoint.cast<double>();

        Vector2d k(dif_weightFunction(e.x()) * focalLength.x(),
                   dif_weightFunction(e.y()) * focalLength.y());
        e.x() = weightFunction(e.x());
        e.y() = weightFunction(e.y());

        J_x(0) = z_inv * k.x();
        J_x(1) = 0.0;
        J_x(2) = - v.x() * z_inv_squared * k.x();
        J_x(3) = (rJ(0, 0) * v.z() - v.x() * rJ(2, 0)) * z_inv_squared * k.x();
        J_x(4) = (rJ(0, 1) * v.z() - v.x() * rJ(2, 1)) * z_inv_squared * k.x();
        J_x(5) = (rJ(0, 2) * v.z() - v.x() * rJ(2, 2)) * z_inv_squared * k.x();

        J_y(0) = 0.0;
        J_y(1) = z_inv * k.y();
        J_y(2) = - v.y() * z_inv_squared * k.y();
        J_y(3) = (rJ(1, 0) * v.z() - v.y() * rJ(2, 0)) * z_inv_squared * k.y();
        J_y(4) = (rJ(1, 1) * v.z() - v.y() * rJ(2, 1)) * z_inv_squared * k.y();
        J_y(5) = (rJ(1, 2) * v.z() - v.y() * rJ(2, 2)) * z_inv_squared * k.y();

         return true;
    };

    auto getResidual = [&] (Vector2d & e, const Vector3f & point, const Vector2f & imagePoint) -> bool
    {
        Vector3d v = R * point.cast<double>() + t;
        if (v.z() < 1e-5)
            return false;

        Vector2d f = v.segment<2>(0) / v.z();

        e = Vector2d(f.x() * focalLength.x() + opticaCenter.x(),
                     f.y() * focalLength.y() + opticaCenter.y()) - imagePoint.cast<double>();
        e.x() = weightFunction(e.x());
        e.y() = weightFunction(e.y());

        //if ((e.x() == 0.0f) || (e.y() == 0.0f))
        //    return false;
        return true;
    };

    using MinimizationInfo = tuple<Matrix<double, 6, 6>, Matrix<double, 6, 1>, double, size_t>;
    auto computeMinimzationInfo = [&] (size_t begin_index, size_t end_index) -> MinimizationInfo
    {
        size_t count = 0;
        Matrix<double, 6, 6> JtJ = Matrix<double, 6, 6>::Zero();
        Matrix<double, 6, 1> Je = Matrix<double, 6, 1>::Zero();
        double Fsq = 0.0;
        Matrix<double, 1, 6> J_i_x, J_i_y;
        Vector2d e;
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
    auto computeResiduals = [&] (size_t begin_index, size_t end_index) -> tuple<double, size_t>
    {
        size_t count = 0;
        double Fsq = 0.0;
        Vector2d e;
        for (size_t i = begin_index; i < end_index; ++i)
        {
            if (!getResidual(e, modelPoints[i], imagePoints[i]))
                continue;
            Fsq += e.dot(e);
            count += 2;
        }
        return make_tuple(Fsq, count);
    };

    double firstError = - 1.0;

    double Fsq = numeric_limits<double>::max();
    double factor = 10.0;

    //QSemaphore semaphore;
    //size_t workPartSize = static_cast<size_t>(ceil(numberPoints / numberWorkThreads));

    for (int iter = 0; iter < numberIterations; ++iter)
    {
        t = x.segment<3>(0);
        w = x.segment<3>(3);
        R = exp_rotationMatrix(w);
        double lw = w.dot(w);
        if (lw < 1e-5)
        {
            R = Matrix3d::Identity();
            rW = - Matrix3d::Identity();
        }
        else
        {
            rW = ((w * w.transpose() + (R.transpose() - Matrix3d::Identity()) * skewMatrix(w)) / (- lw));
        }

        Matrix<double, 6, 6> JtJ;
        Matrix<double, 6, 1> Je;
        size_t count;
        {
            tie(JtJ, Je, Fsq, count) = computeMinimzationInfo(0, numberPoints);
        }
        Fsq /= static_cast<double>(count);
        if (firstError < 0.0)
            firstError = Fsq;
        double Fsq_next = Fsq;
        for (int n_try = 0; n_try < 10; ++n_try)
        {
            JtJ.diagonal() += Matrix<double, 6, 1>::Ones() * factor;
            Matrix<double, 6, 1> dx = JtJ.ldlt().solve(Je);
            Matrix<double, 6, 1> x_next = x - dx;

            t = x_next.segment<3>(0);
            R = exp_rotationMatrix(x_next.segment<3>(3).eval());

            count = 0;
            Fsq_next = 0.0;
            {
                tie(Fsq_next, count) = computeResiduals(0, numberPoints);
            }
            Fsq_next /= static_cast<double>(count);

            if (Fsq_next < Fsq)
            {
                x = x_next;
                break;
            }
            else
            {
                factor *= 2.0;
            }
        }
        double deltaFsq = Fsq - Fsq_next;
        Fsq = Fsq_next;
        if (deltaFsq < numeric_limits<double>::epsilon())
        {
            break;
        }
    }
    return get_x_weightFunction(sqrt(Fsq));
}
