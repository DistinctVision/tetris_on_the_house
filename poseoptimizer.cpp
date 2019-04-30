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

void exp_transform(Matrix3d & outRotationMatrix, Vector3d & outTranslation,
                   const Matrix<double, 6, 1> & mu)
{
    static const double one_6th = 1.0 / 6.0;
    static const double one_20th = 1.0 / 20.0;
    const Vector3d mu_3(mu(0), mu(1), mu(2));
    const Vector3d w(mu(3), mu(4), mu(5));
    const double theta_square = w.dot(w);
    const double theta = sqrt(theta_square);
    double A, B;

    const Vector3d crossVector = w.cross(mu_3);
    if (theta_square < (1e-8))
    {
        A = 1.0 - one_6th * theta_square;
        B = 0.5;
        outTranslation = mu_3 + 0.5 * crossVector;
    }
    else
    {
        double C;
        if (theta_square < (1e-6))
        {
            C = one_6th * (1.0 - one_20th * theta_square);
            A = 1.0 - theta_square * C;
            B = 0.5 - 0.25 * one_6th * theta_square;
        }
        else
        {
            const double inv_theta = 1.0 / theta;
            A = sin(theta) * inv_theta;
            B = (1.0 - cos(theta)) * (inv_theta * inv_theta);
            C = (1.0 - A) * (inv_theta * inv_theta);
        }
        outTranslation = mu_3 + B * crossVector + C * w.cross(crossVector);
    }
    outRotationMatrix = exp_rodrigues(w, A, B);
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

Matrix<double, 6, 1> ln_transform(const Matrix3d & rotationMatrix,
                                  const Vector3d & translation)
{
    Vector3d rot = ln_rotationMatrix(rotationMatrix);
    const double square_theta = rot.dot(rot);
    const double theta = std::sqrt(square_theta);

    double shtot = 0.5;
    if (theta > 0.00001)
    {
        shtot = sin(theta * 0.5) / theta;
    }

    // now do the rotation
    const Matrix3d halfrotator = exp_rotationMatrix(rot * (-0.5));
    Vector3d rottrans = halfrotator * translation;

    if (theta > (0.001))
    {
        rottrans -= (rot * ((translation.dot(rot) * (1.0 - 2.0 * shtot) / (square_theta))));
    }
    else
    {
        rottrans -= (rot * ((translation.dot(rot) / 24.0)));
    }

    rottrans /= (2.0 * shtot);

    Matrix<double, 6, 1> result;
    result(0) = rottrans(0);
    result(1) = rottrans(1);
    result(2) = rottrans(2);
    result(3) = rot(0);
    result(4) = rot(1);
    result(5) = rot(2);
    return result;
}

void optimize_pose(Matrix3d & R, Vector3d & t,
                   const shared_ptr<const PinholeCamera> & camera,
                   const Vectors3d & controlModelPoints,
                   const Vectors2f & controlImagePoints)
{
    static const int numberIterations = 30;

    size_t numberPoints = controlModelPoints.size();

    Vectors2d uv_points(controlImagePoints.size());
    for (size_t i = 0; i < numberPoints; ++i)
    {
        uv_points[i] = camera->unproject(controlImagePoints[i]).segment<2>(0).cast<double>();
    }

    Matrix<double, 6, 1> x = ln_transform(R, t);

    auto getJacobianAndResiduals = [&]
            (const Vector3d & point, const Vector2d & uv) -> tuple<Matrix<double, 2, 6>, Vector2d>
    {
        Vector3d v = R * point + t;
        double z_inv = 1.0 / v.z();
        double z_inv_squared = z_inv * z_inv;

        Matrix<double, 2, 6> J;

        J(0, 0) = z_inv;                      // - 1 / z
        J(0, 1) = 0.0;                        // 0
        J(0, 2) = - v.x() * z_inv_squared;    // x / z^2
        J(0, 3) = - v.y() * J(0, 2);          // x * y / z^2
        J(0, 4) = 1.0 + v.x() * J(0, 2);      // -(1.0 + x^2 / z^2)
        J(0, 5) = - v.y() * z_inv;            // y / z

        J(1, 0) = 0.0;                        // 0
        J(1, 1) = z_inv;                      // - 1 / z
        J(1, 2) = - v.y() * z_inv_squared;    // y / z^2
        J(1, 3) = - (1.0 + v.y() * J(1, 2));  // 1.0 + y^2 / z^2
        J(1, 4) = J(0, 3);                    // -x * y / z^2
        J(1, 5) = v.x() * z_inv;              // - x / z

        Vector2d e = Vector2d(v.x() / v.z(), v.y() / v.z()) - uv;

        return make_tuple(J, e);
    };

    auto toUV = [&] (const Vector3d & point)
    {
        Vector3d v = R * point + t;
        return Vector2d(v.x() / v.z(), v.y() / v.z());
    };

    double firstError = -1.0;

    double Fsq = numeric_limits<double>::max();
    double factor = 1e-5;
    Matrix<double, 2, 6> J_i;
    Vector2d e_i;

    for (int iter = 0; iter < numberIterations; ++iter)
    {
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

            exp_transform(R, t, x_next);

            Fsq_next = 0.0;
            for (size_t i = 0; i < numberPoints; ++i)
            {
                e_i = toUV(controlModelPoints[i]) - uv_points[i];
                Fsq_next += e_i.dot(e_i);
            }

            if (Fsq_next < Fsq)
            {
                x = x_next;
                exp_transform(R, t, x);
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
}
