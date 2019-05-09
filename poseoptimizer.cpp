#include "poseoptimizer.h"

#include <cmath>
#include <qmath.h>
#include <tuple>
#include <climits>
#include <limits>

#include <QSemaphore>
#include <QtConcurrent/QtConcurrent>

#include "pinholecamera.h"

using namespace std;
using namespace Eigen;

Matrix3f exp_rodrigues(const Vector3f & w, float A, float B)
{
    Matrix3f rotationMatrix;
    {
        const float wx2 = w(0) * w(0);
        const float wy2 = w(1) * w(1);
        const float wz2 = w(2) * w(2);

        rotationMatrix(0, 0) = 1.0f - B * (wy2 + wz2);
        rotationMatrix(1, 1) = 1.0f - B * (wx2 + wz2);
        rotationMatrix(2, 2) = 1.0f - B * (wx2 + wy2);
    }
    {
        const float a = A * w(2);
        const float b = B * (w(0) * w(1));
        rotationMatrix(0, 1) = b - a;
        rotationMatrix(1, 0) = b + a;
    }
    {
        const float a = A * w(1);
        const float b = B * (w(0) * w(2));
        rotationMatrix(0, 2) = b + a;
        rotationMatrix(2, 0) = b - a;
    }
    {
        const float a = A * w(0);
        const float b = B * (w(1) * w(2));
        rotationMatrix(1, 2) = b - a;
        rotationMatrix(2, 1) = b + a;
    }
    return rotationMatrix;
}

Matrix3f exp_rotationMatrix(const Vector3f & w)
{
    static const float one_6th = 1.0f / 6.0f;
    static const float one_20th = 1.0f / 20.0f;

    const float theta_sq = w.dot(w);
    const float theta = sqrt(theta_sq);
    float A, B;

    if (theta_sq < (1e-6f))
    {
        A = 1.0f - one_6th * theta_sq;
        B = 0.5f;
    }
    else
    {
        if (theta_sq < 1e-4f)
        {
            B = 0.5f - 0.25f * one_6th * theta_sq;
            A = 1.0f - theta_sq * one_6th * (1.0f - one_20th * theta_sq);
        }
        else
        {
            const float inv_theta = 1.0f / theta;
            A = sin(theta) * inv_theta;
            B = (1.0f - cos(theta)) * (inv_theta * inv_theta);
        }
    }
    return exp_rodrigues(w, A, B);
}

Vector3f ln_rotationMatrix(const Matrix3f & rotationMatrix)
{
    static const float m_sqrt1_2 = 0.707106781186547524401f;

    Vector3f result;

    const float cos_angle = (rotationMatrix(0, 0) + rotationMatrix(1, 1) +
                             rotationMatrix(2, 2) - 1.0f) * 0.5f;
    result(0) = (rotationMatrix(2, 1) - rotationMatrix(1, 2)) * 0.5f;
    result(1) = (rotationMatrix(0, 2) - rotationMatrix(2, 0)) * 0.5f;
    result(2) = (rotationMatrix(1, 0) - rotationMatrix(0, 1)) * 0.5f;

    float sin_angle_abs = sqrt(result(0) * result(0) + result(1) * result(1) + result(2) * result(2));
    if (cos_angle > m_sqrt1_2)
    {
        if (sin_angle_abs > 0.0f)
        {
            result *= asin(sin_angle_abs) / sin_angle_abs;
        }
    }
    else if (cos_angle > - m_sqrt1_2)
    {
        const float angle = acos(cos_angle);
        result *= angle / sin_angle_abs;
    }
    else
    {
        const float angle = static_cast<float>(M_PI) - asin(sin_angle_abs);
        const float d0 = rotationMatrix(0, 0) - cos_angle,
                    d1 = rotationMatrix(1, 1) - cos_angle,
                    d2 = rotationMatrix(2, 2) - cos_angle;
        Vector3f r2;
        if (((d0 * d0) > (d1 * d1)) && ((d0 * d0) > (d2 * d2)))
        {
            r2(0) = d0;
            r2(1) = (rotationMatrix(1, 0) + rotationMatrix(0, 1)) * 0.5f;
            r2(2) = (rotationMatrix(0, 2) + rotationMatrix(2, 0)) * 0.5f;
        }
        else if ((d1 * d1) > (d2 * d2))
        {
            r2(0) = (rotationMatrix(1, 0) + rotationMatrix(0, 1)) * 0.5f;
            r2(1) = d1;
            r2(2) = (rotationMatrix(2, 1) + rotationMatrix(1, 2)) * 0.5f;
        }
        else
        {
            r2(0) = (rotationMatrix(0, 2) + rotationMatrix(2, 0)) * 0.5f;
            r2(1) = (rotationMatrix(2, 1) + rotationMatrix(1, 2)) * 0.5f;
            r2(2) = d2;
        }
        if (r2.dot(result) < 0.0f)
            r2 *= -1.0;
        r2.normalize();
        result = r2 * angle;
    }
    return result;
}

Matrix3f skewMatrix(const Vector3f & a)
{
    Matrix3f A;
    A << 0.0f, - a(2), a(1),
         a(2),  0.0f,  - a(0),
         - a(1), a(0), 0.0f;
    return A;
}

Matrix3f exp_jacobian(const Vector3f & w,
                      const Vector3f & point)
{
    float l = w.dot(w);
    if (l < 1e-5f)
    {
        return - skewMatrix(point);
    }
    Matrix3f R = exp_rotationMatrix(w);
    return - R * skewMatrix(point) *
            ((w * w.transpose() +
              (R.transpose() - Matrix3f::Identity()) * skewMatrix(w)) / l);
}

float optimize_pose(Matrix3f & R, Vector3f & t,
                    QThreadPool * pool, size_t numberWorkThreads,
                    const cv::Mat & distanceMap,
                    const shared_ptr<const PinholeCamera> & camera,
                    const Vectors3f & controlModelPoints,
                    float maxDistance,
                    int numberIterations)
{
    Matrix<float, 6, 1> x;
    x.segment<3>(0) = t;
    x.segment<3>(3) = ln_rotationMatrix(R);
    float E = optimize_pose(x,
                            pool, numberWorkThreads, distanceMap,
                            camera, controlModelPoints, maxDistance, numberIterations);
    t = x.segment<3>(0);
    R = exp_rotationMatrix(x.segment<3>(3));
    return E;
}

float optimize_pose(Matrix<float, 6, 1> & x,
                    QThreadPool * pool, size_t numberWorkThreads,
                    const cv::Mat & distanceMap,
                    const shared_ptr<const PinholeCamera> & camera,
                    const Vectors3f & modelPoints,
                    float maxDistance,
                    int numberIterations)
{
    /*float weightFunction_k2 = 1.0f / (3.0f * maxDistance * maxDistance);
    float weightFunction_k1 = 1.0f / (maxDistance * (1.0f - weightFunction_k2 * maxDistance * maxDistance));

    auto weightFunction = [&] (float x) -> float
    {
        return (x >= maxDistance) ? 1.0f : (weightFunction_k1 * x * (1.0f - (x * x) * weightFunction_k2));
    };

    auto div_weightFunction = [&] (float x) -> float
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
    Vector2f opticalCenter = camera->pixelOpticalCenter();
    Vector2f imageCorner(static_cast<float>(distanceMap.cols - 2), static_cast<float>(distanceMap.rows - 2));

    Vector3f w;
    Matrix3f R;
    Vector3f t;
    Matrix3f rW;

    auto getResidualAndDiffs = [&]
            (float & dis, float & dis_dx, float & dis_dy, const Vector2f & imagePoint)
    {
        Vector2i imagePoint_i = imagePoint.cast<int>();
        Vector2f sp(imagePoint.x() - static_cast<float>(imagePoint_i.x()),
                    imagePoint.y() - static_cast<float>(imagePoint_i.y()));
        Vector2f i_sp(1.0f - sp.x(), 1.0f - sp.y());

        float w1 = i_sp.x() * i_sp.y();
        float w2 = sp.x() * i_sp.y();
        float w3 = i_sp.x() * sp.y();
        float w4 = sp.x() * sp.y();

        const float * d_ptr = distanceMap.ptr<float>(imagePoint_i.y(), imagePoint_i.x());
        const float * d_ptr_next = distanceMap.ptr<float>(imagePoint_i.y() + 1, imagePoint_i.x());
        dis = d_ptr[0] * w1 + d_ptr[1] * w2 + d_ptr_next[0] * w3 + d_ptr_next[1] * w4;

        const float * d_ptr_prev = distanceMap.ptr<float>(imagePoint_i.y() - 1, imagePoint_i.x());
        dis_dx = ((d_ptr[0] - d_ptr[-1]) * w1 + (d_ptr[1] - d_ptr[0]) * w2 +
                  (d_ptr_next[0] - d_ptr_next[-1]) * w3 + (d_ptr_next[1] - d_ptr_next[0]) * w4);
        dis_dy = ((d_ptr[0] - d_ptr_prev[0]) * w1 + (d_ptr[1] - d_ptr_prev[1]) * w2 +
                  (d_ptr_next[0] - d_ptr[0]) * w3 + (d_ptr_next[1] - d_ptr[1]) * w4);
    };

    auto getJacobianAndResidual = [&]
            (Matrix<float, 1, 6> & J, float & e, const Vector3f & point) -> bool
    {
        //Matrix3f rJ = R * skewMatrix(point) * rW;
        Matrix3f rJ = exp_jacobian(w, point);

        Vector3f v = R * point + t;
        if (v.z() < 1e-5f)
            return false;

        float z_inv = 1.0f / v.z();
        float z_inv_squared = z_inv * z_inv;

        Matrix<float, 1, 6> J_x, J_y;

        J_x(0) = z_inv;
        J_x(1) = 0.0f;
        J_x(2) = - v.x() * z_inv_squared;
        J_x(3) = (rJ(0, 0) * v.z() - v.x() * rJ(2, 0)) * z_inv_squared;
        J_x(4) = (rJ(0, 1) * v.z() - v.x() * rJ(2, 1)) * z_inv_squared;
        J_x(5) = (rJ(0, 2) * v.z() - v.x() * rJ(2, 2)) * z_inv_squared;

        J_y(0) = 0.0f;
        J_y(1) = z_inv;
        J_y(2) = - v.y() * z_inv_squared;
        J_y(3) = (rJ(1, 0) * v.z() - v.y() * rJ(2, 0)) * z_inv_squared;
        J_y(4) = (rJ(1, 1) * v.z() - v.y() * rJ(2, 1)) * z_inv_squared;
        J_y(5) = (rJ(1, 2) * v.z() - v.y() * rJ(2, 2)) * z_inv_squared;

        Vector2f uv = (v.segment<2>(0) * z_inv);
        Vector2f imagePoint(uv.x() * focalLength.x() + opticalCenter.x(),
                            uv.y() * focalLength.y() + opticalCenter.y());

        if ((imagePoint.x() < 1.0f) || (imagePoint.y() < 1.0f) ||
                (imagePoint.x() >= imageCorner.x()) || (imagePoint.y() >= imageCorner.y()))
        {
            return false;
        }

        float dis, dis_dx, dis_dy;
        getResidualAndDiffs(dis, dis_dx, dis_dy, imagePoint);

        float w = weightFunction(dis);
        //if (w == 0.0f)
        //    return false;

        float dif_w = dif_weightFunction(dis);

        J = (J_x * (focalLength.x() * dis_dx * dif_w) + J_y * (focalLength.y() * dis_dy * dif_w));
        e = w;

        return true;
    };

    auto getResidual = [&] (float & e, const Vector3f & point) -> bool
    {
        Vector3f v = R * point + t;
        if (v.z() < 1e-5f)
            return false;

        Vector2f uv = (v.segment<2>(0) / v.z());
        Vector2f imagePoint(uv.x() * focalLength.x() + opticalCenter.x(),
                            uv.y() * focalLength.y() + opticalCenter.y());

        if ((imagePoint.x() < 1.0f) || (imagePoint.y() < 1.0f) ||
                (imagePoint.x() >= imageCorner.x()) || (imagePoint.y() >= imageCorner.y()))
        {
            return false;
        }

        Vector2i imagePoint_i = imagePoint.cast<int>();
        Vector2f sp(imagePoint.x() - static_cast<float>(imagePoint_i.x()),
                    imagePoint.y() - static_cast<float>(imagePoint_i.y()));
        Vector2f i_sp(1.0f - sp.x(), 1.0f - sp.y());

        float w1 = i_sp.x() * i_sp.y();
        float w2 = sp.x() * i_sp.y();
        float w3 = i_sp.x() * sp.y();
        float w4 = sp.x() * sp.y();

        const float * d_ptr = distanceMap.ptr<float>(imagePoint_i.y(), imagePoint_i.x());
        const float * d_ptr_next = distanceMap.ptr<float>(imagePoint_i.y() + 1, imagePoint_i.x());
        float dis = d_ptr[0] * w1 + d_ptr[1] * w2 + d_ptr_next[0] * w3 + d_ptr_next[1] * w4;

        float w = weightFunction(dis);
        //if (w == 0.0f)
        //    return false;
        e = (w);

        return true;
    };

    using MinimizationInfo = tuple<Matrix<float, 6, 6>, Matrix<float, 6, 1>, float, size_t>;
    auto computeMinimzationInfo = [&] (size_t begin_index, size_t end_index) -> MinimizationInfo
    {
        size_t count = 0;
        Matrix<float, 6, 6> JtJ = Matrix<float, 6, 6>::Zero();
        Matrix<float, 6, 1> Je = Matrix<float, 6, 1>::Zero();
        float Fsq = 0.0f;
        Matrix<float, 1, 6> J_i;
        float e;
        for (size_t i = begin_index; i < end_index; ++i)
        {
            if (!getJacobianAndResidual(J_i, e, modelPoints[i]))
                continue;
            JtJ += J_i.transpose() * J_i;
            Je += J_i.transpose() * e;
            Fsq += e * e;
            ++count;
        }
        return make_tuple(JtJ, Je, Fsq, count);
    };
    auto computeResiduals = [&] (size_t begin_index, size_t end_index) -> tuple<float, size_t>
    {
        size_t count = 0;
        float Fsq = 0.0f, e;
        for (size_t i = begin_index; i < end_index; ++i)
        {
            if (!getResidual(e, modelPoints[i]))
            {
                Fsq += maxDistance * maxDistance;
                ++count;
                continue;
            }
            Fsq += e * e;
            ++count;
        }
        return make_tuple(Fsq, count);
    };

    float firstError = - 1.0f;

    float Fsq = numeric_limits<float>::max();
    float factor = 0.f;

    QSemaphore semaphore;
    size_t workPartSize = static_cast<size_t>(ceil(numberPoints / numberWorkThreads));

    for (int iter = 0; iter < numberIterations; ++iter)
    {
        t = x.segment<3>(0);
        w = x.segment<3>(3);
        float lw = w.dot(w);
        if (lw < 1e-5f)
        {
            R = Matrix3f::Identity();
            rW = - Matrix3f::Identity();
        }
        else
        {
            R = exp_rotationMatrix(w);
            rW = ((w * w.transpose() + (R.transpose() - Matrix3f::Identity()) * skewMatrix(w)) / (- lw));
        }

        Matrix<float, 6, 6> JtJ = Matrix<float, 6, 6>::Zero();
        Matrix<float, 6, 1> Je = Matrix<float, 6, 1>::Zero();
        Fsq = 0.0f;
        size_t count = 0;
        {
            vector<MinimizationInfo> results(numberWorkThreads);
            for (size_t i = 0; i < numberWorkThreads; ++i)
            {
                QtConcurrent::run(pool, [&, i] () {
                    size_t begin_index = i * workPartSize;
                    size_t end_index = min(begin_index + workPartSize, numberPoints);
                    results[i] = computeMinimzationInfo(begin_index, end_index);
                    semaphore.release();
                });
            }
            semaphore.acquire(static_cast<int>(numberWorkThreads));
            for (size_t i = 0; i < numberWorkThreads; ++i)
            {
                const MinimizationInfo & info = results[i];
                JtJ += get<0>(info);
                Je += get<1>(info);
                Fsq += get<2>(info);
                count += get<3>(info);
            }
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
                vector<tuple<float, size_t>> results(numberWorkThreads);
                for (size_t i = 0; i < numberWorkThreads; ++i)
                {
                    QtConcurrent::run(pool, [&, i] () {
                        size_t begin_index = i * workPartSize;
                        size_t end_index = min(begin_index + workPartSize, numberPoints);
                        results[i] = computeResiduals(begin_index, end_index);
                        semaphore.release();
                    });
                }
                semaphore.acquire(static_cast<int>(numberWorkThreads));
                for (size_t i = 0; i < numberWorkThreads; ++i)
                {
                    Fsq_next += get<0>(results[i]);
                    count += get<1>(results[i]);
                }
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
