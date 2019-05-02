#include "poseoptimizer.h"

#include <cmath>
#include <qmath.h>
#include <tuple>
#include <climits>
#include <limits>

#include "pinholecamera.h"

using namespace std;
using namespace Eigen;

Matrix3d exp_rodrigues(const Vector3d & w, double A, double B)
{
    Matrix3d rotationMatrix;
    {
        const double wx2 = w(0) * w(0);
        const double wy2 = w(1) * w(1);
        const double wz2 = w(2) * w(2);

        rotationMatrix(0, 0) = 1.0 - B * (wy2 + wz2);
        rotationMatrix(1, 1) = 1.0 - B * (wx2 + wz2);
        rotationMatrix(2, 2) = 1.0 - B * (wx2 + wy2);
    }
    {
        const double a = A * w(2);
        const double b = B * (w(0) * w(1));
        rotationMatrix(0, 1) = b - a;
        rotationMatrix(1, 0) = b + a;
    }
    {
        const double a = A * w(1);
        const double b = B * (w(0) * w(2));
        rotationMatrix(0, 2) = b + a;
        rotationMatrix(2, 0) = b - a;
    }
    {
        const double a = A * w(0);
        const double b = B * (w(1) * w(2));
        rotationMatrix(1, 2) = b - a;
        rotationMatrix(2, 1) = b + a;
    }
    return rotationMatrix;
}

Matrix3d exp_rotationMatrix(const Vector3d & w)
{
    static const double one_6th = 1.0 / 6.0;
    static const double one_20th = 1.0 / 20.0;

    const double theta_sq = w.dot(w);
    const double theta = sqrt(theta_sq);
    double A, B;

    if (theta_sq < (1e-8))
    {
        A = 1.0 - one_6th * theta_sq;
        B = 0.5;
    }
    else
    {
        if (theta_sq < 1e-6)
        {
            B = 0.5 - 0.25 * one_6th * theta_sq;
            A = 1.0 - theta_sq * one_6th * (1.0 - one_20th * theta_sq);
        }
        else
        {
            const double inv_theta = 1.0 / theta;
            A = sin(theta) * inv_theta;
            B = (1.0 - cos(theta)) * (inv_theta * inv_theta);
        }
    }
    return exp_rodrigues(w, A, B);
}

Vector3d ln_rotationMatrix(const Matrix3d & rotationMatrix)
{
    static const double m_sqrt1_2 = 0.707106781186547524401;

    Vector3d result;

    const double cos_angle = (rotationMatrix(0, 0) + rotationMatrix(1, 1) +
                              rotationMatrix(2, 2) - 1.0) * 0.5;
    result(0) = (rotationMatrix(2, 1) - rotationMatrix(1, 2)) * 0.5;
    result(1) = (rotationMatrix(0, 2) - rotationMatrix(2, 0)) * 0.5;
    result(2) = (rotationMatrix(1, 0) - rotationMatrix(0, 1)) * 0.5;

    double sin_angle_abs = std::sqrt(result(0) * result(0) + result(1) * result(1) + result(2) * result(2));
    if (cos_angle > m_sqrt1_2)
    {
        if (sin_angle_abs > 0.0)
        {
            result *= asin(sin_angle_abs) / sin_angle_abs;
        }
    }
    else if (cos_angle > - m_sqrt1_2)
    {
        const double angle = acos(cos_angle);
        result *= angle / sin_angle_abs;
    }
    else
    {
        const double angle = M_PI - asin(sin_angle_abs);
        const double d0 = rotationMatrix(0, 0) - cos_angle,
                     d1 = rotationMatrix(1, 1) - cos_angle,
                     d2 = rotationMatrix(2, 2) - cos_angle;
        Vector3d r2;
        if (((d0 * d0) > (d1 * d1)) && ((d0 * d0) > (d2 * d2)))
        {
            r2(0) = d0;
            r2(1) = (rotationMatrix(1, 0) + rotationMatrix(0, 1)) * 0.5;
            r2(2) = (rotationMatrix(0, 2) + rotationMatrix(2, 0)) * 0.5;
        }
        else if ((d1 * d1) > (d2 * d2))
        {
            r2(0) = (rotationMatrix(1, 0) + rotationMatrix(0, 1)) * 0.5;
            r2(1) = d1;
            r2(2) = (rotationMatrix(2, 1) + rotationMatrix(1, 2)) * 0.5;
        }
        else
        {
            r2(0) = (rotationMatrix(0, 2) + rotationMatrix(2, 0)) * 0.5;
            r2(1) = (rotationMatrix(2, 1) + rotationMatrix(1, 2)) * 0.5;
            r2(2) = d2;
        }
        if (r2.dot(result) < 0.0)
            r2 *= -1.0;
        r2.normalize();
        result = r2 * angle;
    }
    return result;
}

Matrix3d skewMatrix(const Vector3d & a)
{
    Matrix3d A;
    A << 0.0, - a(2), a(1),
         a(2), 0.0, - a(0),
         - a(1), a(0), 0.0;
    return A;
}

Matrix3d exp_jacobian(const Vector3d & w,
                      const Vector3d & point)
{
    double l = w.dot(w);
    if (l < 1e-5)
    {
        return - skewMatrix(point);
    }
    Matrix3d R = exp_rotationMatrix(w);
    return - R * skewMatrix(point) *
            ((w * w.transpose() +
              (R.transpose() - Matrix3d::Identity()) * skewMatrix(w)) / l);
}

void test_exp()
{
    auto generate_random_unit_vector_3d = [] () -> Vector3d
    {
        return Vector3d((rand() % 2000) / 1000.0 - 1.0,
                        (rand() % 2000) / 1000.0 - 1.0,
                        (rand() % 2000) / 1000.0 - 1.0).normalized();
    };

    for (int i = 0; i < 100; ++i)
    {
        Vector3d x = generate_random_unit_vector_3d();
        Matrix3d R = exp_rotationMatrix(x);
        Vector3d x1 = ln_rotationMatrix(R);
        double s1 = (x1 - x).array().abs().sum();
        assert(static_cast<float>(s1) < numeric_limits<float>::epsilon());
    }
}

void optimize_pose(Matrix3d & R, Vector3d & t,
                   const shared_ptr<const PinholeCamera> & camera,
                   const Vectors3d & controlModelPoints,
                   const Vectors2f & controlImagePoints,
                   int numberIterations)
{
    size_t numberPoints = controlModelPoints.size();

    Vectors2d uv_points(controlImagePoints.size());
    for (size_t i = 0; i < numberPoints; ++i)
    {
        uv_points[i] = camera->unproject(controlImagePoints[i]).segment<2>(0).cast<double>();
    }

    Vector3d w = ln_rotationMatrix(R);
    Matrix3d rW;

    Matrix<double, 6, 1> x;
    x.segment<3>(0) = t;
    x.segment<3>(3) = w;

    auto getJacobianAndResiduals = [&]
            (const Vector3d & point, const Vector2d & uv) -> tuple<Matrix<double, 2, 6>, Vector2d>
    {
        Matrix3d rJ = R * skewMatrix(point) * rW;

        Vector3d v = R * point + t;
        double z_inv = 1.0 / v.z();
        double z_inv_squared = z_inv * z_inv;

        Matrix<double, 2, 6> J;

        J(0, 0) = z_inv;
        J(0, 1) = 0.0;
        J(0, 2) = - v.x() * z_inv_squared;
        J(0, 3) = (rJ(0, 0) * v.z() - v.x() * rJ(2, 0)) * z_inv_squared;
        J(0, 4) = (rJ(0, 1) * v.z() - v.x() * rJ(2, 1)) * z_inv_squared;
        J(0, 5) = (rJ(0, 2) * v.z() - v.x() * rJ(2, 2)) * z_inv_squared;

        J(1, 0) = 0.0;
        J(1, 1) = z_inv;
        J(1, 2) = - v.y() * z_inv_squared;
        J(1, 3) = (rJ(1, 0) * v.z() - v.y() * rJ(2, 0)) * z_inv_squared;
        J(1, 4) = (rJ(1, 1) * v.z() - v.y() * rJ(2, 1)) * z_inv_squared;
        J(1, 5) = (rJ(1, 2) * v.z() - v.y() * rJ(2, 2)) * z_inv_squared;

        Vector2d e = v.segment<2>(0) * z_inv - uv;

        return make_tuple(J, e);
    };

    auto toUV = [&] (const Vector3d & point)
    {
        Vector3d v = R * point + t;
        return Vector2d(v.x() / v.z(), v.y() / v.z());
    };

    double firstError = - 1.0;

    double Fsq = numeric_limits<double>::max();
    double factor = 1e-5;
    Matrix<double, 2, 6> J_i;
    Vector2d e_i;

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
            rW = (- ((w * w.transpose() + (R.transpose() - Matrix3d::Identity()) * skewMatrix(w)) / lw));
        }

        Matrix<double, 6, 6> JtJ = Matrix<double, 6, 6>::Zero();
        Matrix<double, 6, 1> Je = Matrix<double, 6, 1>::Zero();
        Fsq = 0.0;
        for (size_t i = 0; i < numberPoints; ++i)
        {
            tie(J_i, e_i) = getJacobianAndResiduals(controlModelPoints[i], uv_points[i]);
            JtJ += J_i.row(0).transpose() * J_i.row(0);
            Je += J_i.row(0).transpose() * e_i.x();
            JtJ += J_i.row(1).transpose() * J_i.row(1);
            Je += J_i.row(1).transpose() * e_i.y();
            Fsq += e_i.dot(e_i);
        }
        if (firstError < 0.0)
            firstError = Fsq;
        double Fsq_next = Fsq;
        for (int n_try = 0; n_try < 100; ++n_try)
        {
            JtJ.diagonal() += Matrix<double, 6, 1>::Ones() * factor;
            Matrix<double, 6, 1> dx = JtJ.ldlt().solve(Je).eval();
            Matrix<double, 6, 1> x_next = x - dx;

            t = x_next.segment<3>(0);
            R = exp_rotationMatrix(x_next.segment<3>(3));

            Fsq_next = 0.0;
            for (size_t i = 0; i < numberPoints; ++i)
            {
                e_i = toUV(controlModelPoints[i]) - uv_points[i];
                Fsq_next += e_i.dot(e_i);
            }

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
    t = x.segment<3>(0);
    R = exp_rotationMatrix(x.segment<3>(3));
}

double optimize_pose(Matrix3d & R, Vector3d & t,
                     const cv::Mat & distanceMap,
                     const shared_ptr<const PinholeCamera> & camera,
                     const Vectors3d & modelPoints,
                     float maxDistance,
                     int numberIterations)
{
    Matrix<double, 6, 1> x;
    x.segment<3>(0) = t;
    x.segment<3>(3) = ln_rotationMatrix(R);
    double E = optimize_pose(x, distanceMap, camera, modelPoints, maxDistance, numberIterations);
    t = x.segment<3>(0);
    R = exp_rotationMatrix(x.segment<3>(3));
    return E;
}

double optimize_pose(Matrix<double, 6, 1> & x,
                     const cv::Mat & distanceMap,
                     const shared_ptr<const PinholeCamera> & camera,
                     const Vectors3d & modelPoints,
                     float maxDistance,
                     int numberIterations)
{
    float weightFunction_k2 = 1.0f / (3.0f * maxDistance * maxDistance);
    float weightFunction_k1 = 1.0f / (maxDistance * (1.0f - weightFunction_k2 * maxDistance * maxDistance));

    auto weightFunction = [&] (float x) -> float
    {
        return (x >= maxDistance) ? 0.0f : (weightFunction_k1 * x * (1.0f - (x * x) * weightFunction_k2));
    };

    auto div_weightFunction = [&] (float x) -> float
    {
        return (x >= maxDistance) ? 0.0f : (weightFunction_k1 - 3.0f * weightFunction_k1 * weightFunction_k2 * x * x);
    };

    auto get_x_weightFunction = [&] (double y) -> double
    {
        double b = - 1.0 / static_cast<double>(weightFunction_k2);
        double c = y / static_cast<double>(weightFunction_k1 * weightFunction_k2);
        double Q = ( - 3.0 * b) / 9.0;
        double R = (27.0 * c) / 54.0;

        double t = acos(R / sqrt(Q * Q * Q)) / 3.0;
        double x = - 2.0 * sqrt(Q) * cos(t - (2.0 / 3.0) * M_PI);

        return x;
    };

    size_t numberPoints = modelPoints.size();

    Vector2f focalLength = camera->focalLength();
    Vector2f opticalCenter = camera->opticalCenter();
    Vector2f imageCorner(static_cast<float>(distanceMap.cols - 2), static_cast<float>(distanceMap.rows - 2));

    Vector3d w;
    Matrix3d R;
    Vector3d t;
    Matrix3d rW;

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
            (Matrix<double, 1, 6> & J, double & e, const Vector3d & point) -> bool
    {
        Matrix3d rJ = R * skewMatrix(point) * rW;

        Vector3d v = R * point + t;
        if (v.z() < 1e-5)
            return false;

        double z_inv = 1.0 / v.z();
        double z_inv_squared = z_inv * z_inv;

        Matrix<double, 1, 6> J_x, J_y;

        J_x(0) = z_inv;
        J_x(1) = 0.0;
        J_x(2) = - v.x() * z_inv_squared;
        J_x(3) = (rJ(0, 0) * v.z() - v.x() * rJ(2, 0)) * z_inv_squared;
        J_x(4) = (rJ(0, 1) * v.z() - v.x() * rJ(2, 1)) * z_inv_squared;
        J_x(5) = (rJ(0, 2) * v.z() - v.x() * rJ(2, 2)) * z_inv_squared;

        J_y(0) = 0.0;
        J_y(1) = z_inv;
        J_y(2) = - v.y() * z_inv_squared;
        J_y(3) = (rJ(1, 0) * v.z() - v.y() * rJ(2, 0)) * z_inv_squared;
        J_y(4) = (rJ(1, 1) * v.z() - v.y() * rJ(2, 1)) * z_inv_squared;
        J_y(5) = (rJ(1, 2) * v.z() - v.y() * rJ(2, 2)) * z_inv_squared;

        Vector2f uv = (v.segment<2>(0) * z_inv).cast<float>();
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
        if (w == 0.0f)
            return false;

        float div_w = div_weightFunction(dis);

        J = (J_x * (focalLength.x() * dis_dx * div_w) + J_y * (focalLength.y() * dis_dy * div_w)).cast<double>();
        e = static_cast<double>(w);

        return true;
    };

    auto getResidual = [&] (double & e, const Vector3d & point) -> bool
    {
        Vector3d v = R * point + t;
        if (v.z() < 1e-5)
            return false;

        Vector2f uv = (v.segment<2>(0) / v.z()).cast<float>();
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
        if (w == 0.0f)
            return false;
        e = static_cast<double>(w);

        return true;
    };

    double firstError = - 1.0;

    double Fsq = numeric_limits<double>::max();
    double factor = 1e-5;
    Matrix<double, 1, 6> J_i;
    double e;

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
            rW = (- ((w * w.transpose() + (R.transpose() - Matrix3d::Identity()) * skewMatrix(w)) / lw));
        }

        size_t count = 0;
        Matrix<double, 6, 6> JtJ = Matrix<double, 6, 6>::Zero();
        Matrix<double, 6, 1> Je = Matrix<double, 6, 1>::Zero();
        Fsq = 0.0;
        for (size_t i = 0; i < numberPoints; ++i)
        {
            if (!getJacobianAndResidual(J_i, e, modelPoints[i]))
                continue;
            JtJ += J_i.transpose() * J_i;
            Je += J_i.transpose() * e;
            Fsq += e * e;
            ++count;
        }
        Fsq /= static_cast<double>(count);
        if (firstError < 0.0)
            firstError = Fsq;
        double Fsq_next = Fsq;
        for (int n_try = 0; n_try < 10; ++n_try)
        {
            JtJ.diagonal() += Matrix<double, 6, 1>::Ones() * factor;
            Matrix<double, 6, 1> dx = JtJ.ldlt().solve(Je).eval();
            Matrix<double, 6, 1> x_next = x - dx;

            t = x_next.segment<3>(0);
            R = exp_rotationMatrix(x_next.segment<3>(3));

            count = 0;
            Fsq_next = 0.0;
            for (size_t i = 0; i < numberPoints; ++i)
            {
                if (!getResidual(e, modelPoints[i]))
                    continue;
                Fsq_next += e * e;
                ++count;
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
