/**
* This file is part of sonar library
* Copyright (C) 2019 Vlasov Aleksey ijonsilent53@gmail.com
* For more information see <https://github.com/DistinctVision/sonar>
**/

#ifndef SONAR_MATHUTILS_IMPL_HPP
#define SONAR_MATHUTILS_IMPL_HPP

namespace sonar {

namespace math_utils {

int pow(int a, int b)
{
    int r = 1;
    while (b > 0) {
        r *= a;
        --b;
    }
    return r;
}

static char signum(char x)
{
    return (x == 0) ? 0 : ((x < 0) ? -1 : 1);
}

static int signum(int x)
{
    return (x == 0) ? 0 : ((x < 0) ? -1 : 1);
}

static float signum(float x)
{
    return (std::abs(x) < std::numeric_limits<float>::epsilon()) ? 0.0f : ((x < 0.0f) ? -1.0f : 1.0f);
}

static double signum(double x)
{
    return (std::abs(x) < std::numeric_limits<double>::epsilon()) ? 0.0 : ((x < 0.0) ? -1.0 : 1.0);
}

template < typename Type >
Point2<Type> projectToPoint(const Eigen::Matrix<Type, 3, 1> & v)
{
    return Point2<Type>(v(0) / v(2), v(1) / v(2));
}

template < typename Type >
Eigen::Matrix<Type, 3, 1> unprojectPoint(const Point2<Type> & p)
{
    return Eigen::Matrix<Type, 3, 1>(p.x, p.y, 1);
}

template < typename Type >
Eigen::Matrix<Type, 2, 1> project(const Eigen::Matrix<Type, 3, 1> & v)
{
    return Eigen::Matrix<Type, 2, 1>(v(0) / v(2), v(1) / v(2));
}

template < typename Type >
Eigen::Matrix<Type, 3, 1> project(const Eigen::Matrix<Type, 4, 1> &v)
{
    return Eigen::Matrix<Type, 3, 1>(v(0) / v(3), v(1) / v(3), v(2) / v(3));
}

template < typename Type >
Eigen::Matrix<Type, 3, 1> unproject(const Eigen::Matrix<Type, 2, 1> & v)
{
    return Eigen::Matrix<Type, 3, 1>(v(0), v(1), 1);
}

template < typename Type >
Eigen::Matrix<Type, 4, 1> unproject(const Eigen::Matrix<Type, 3, 1> & v)
{
    return Eigen::Matrix<Type, 4, 1>(v(0), v(1), v(2), 1);
}

template < typename Type >
Eigen::Matrix<Type, 3, 3> skewMatrix(const Eigen::Matrix<Type, 3, 1> & a)
{
    Eigen::Matrix<Type, 3, 3> A;
    A << Type(0), - a(2), a(1),
         a(2), Type(0), - a(0),
         - a(1), a(0), Type(0);
    return A;
}

template < typename Type >
Eigen::AlignedBox<Type, 2> getBoundedRect(const std::vector<Point2<Type>> & points)
{
    Eigen::AlignedBox<Type, 2> b;
    for (auto it = std::begin(points); it != std::end(points); ++it)
        b.extend(Eigen::Matrix<Type, 2, 1>(it->x, it->y));
    return b;
}

template < typename Type, int size >
Eigen::AlignedBox<Type, size> getBoundedRect(const std::vector<Eigen::Matrix<Type, size, 1>> & points)
{
    Eigen::AlignedBox<Type, size> b;
    for (auto it = std::begin(points); it != std::end(points); ++it)
        b.extend(*it);
    return b;
}

template < typename Type, int size >
Type supportValue(const Eigen::Matrix<Type, size, 1> & dir,
                  const std::vector<Eigen::Matrix<Type, size, 1>> & convex_points)
{
    Type maxValue = - std::numeric_limits<float>::max();
    for (auto it = convex_points.cbegin(); it != convex_points.cend(); ++it) {
        Type value = dir.dot(*it);
        if (value > maxValue) {
            maxValue = value;
        }
    }
    return maxValue;
}

template < typename Type >
std::vector<Point2<Type>> transformPoints(const Eigen::Matrix<Type, 3, 3> & transformMatrix,
                                          const std::vector<Point2<Type>> & points)
{
    std::vector<Point2<Type>> r(points.size());
    for (size_t i = 0; i < points.size(); ++i) {
        r[i] = project(transformMatrix * unprojectPoint(points[i]));
    }
    return r;
}

template < typename Type >
Eigen::Matrix<Type, 3, 3> calculateHomography(const std::vector<Point2<Type>> & pointsA,
                                              const std::vector<Point2<Type>> & pointsB)
{
    assert(pointsA.size() >= 4);
    assert(pointsB.size() == pointsA.size());

    std::size_t offset = 0;
    Eigen::Matrix<Type, Eigen::Dynamic, 9> M(std::max((int)(pointsA.size() * 2), 9));
    for (std::size_t i = 0; i < pointsA.size(); ++i) {

        const Point2<Type> & pointA = pointsA[i];
        const Point2<Type> & pointB = pointsB[i];

        M(offset, 0) = pointA.x;
        M(offset, 1) = pointA.y;
        M(offset, 2) = 1;
        M(offset, 3) = 0;
        M(offset, 4) = 0;
        M(offset, 5) = 0;
        M(offset, 6) = - pointA.x * pointB.x;
        M(offset, 7) = - pointA.y * pointB.x;
        M(offset, 8) = - pointB.x;

        ++offset;

        M(offset, 0) = 0;
        M(offset, 1) = 0;
        M(offset, 2) = 0;
        M(offset, 3) = pointA.x;
        M(offset, 4) = pointA.y;
        M(offset, 5) = 1;
        M(offset, 6) = - pointA.x * pointB.y;
        M(offset, 7) = - pointA.y * pointB.y;
        M(offset, 8) = - pointB.y;

        ++offset;
    }

    if (pointsA.size() == 4) {
        for (int i = 0; i < 9; ++i)
            M(8, i) = 0;
    }

    Eigen::BDCSVD<Eigen::Matrix<Type, Eigen::Dynamic, 9>> svd(M, Eigen::ComputeThinV);
    Eigen::Matrix<Type, 9, 1> h = svd.matrixV().col(8);

    Eigen::Matrix<Type, 3, 3> H;
    H << h(0), h(1), h(2), h(3), h(4), h(5), h(6), h(7), h(8);
    return H;
}

template < typename Type >
Eigen::Matrix<Type, 3, 3> calculateRigidTransform(const std::vector<Point2<Type>> & pointsA,
                                                  const std::vector<Point2<Type>> & pointsB)
{
    assert(pointsA.size() >= 2);
    assert(pointsB.size() == pointsA.size());

    Eigen::Matrix<Type, Eigen::Dynamic, 4> M((int)(pointsA.size() * 2));
    Eigen::Matrix<Type, Eigen::Dynamic, 1> b(M.rows);
    size_t offset = 0;
    for (std::size_t i = 0; i < pointsA.size(); ++i) {

        const Point2<Type> & pointA = pointsA[i];
        const Point2<Type> & pointB = pointsB[i];

        M(offset, 0) = pointA.x;
        M(offset, 1) = pointA.y;
        M(offset, 2) = 1;
        M(offset, 3) = 0;

        b(offset) = pointB.x;
        ++offset;

        M(offset, 0) = pointA.y;
        M(offset, 1) = - pointA.x;
        M(offset, 2) = 0;
        M(offset, 3) = 1;

        b(offset) = pointB.y;
        ++offset;
    }

    Eigen::Matrix<Type, 4, 1> a_dir = (M.transpose() * M).invert() * M.transpose() * b;

    Eigen::Matrix<Type, 3, 3> A;
    A(0, 0) = a_dir(0);
    A(0, 1) = a_dir(1);
    A(0, 2) = a_dir(2);
    A(1, 0) = - a_dir(1);
    A(1, 1) = a_dir(0);
    A(1, 2) = a_dir(3);
    A(2, 0) = 0;
    A(2, 1) = 0;
    A(2, 2) = 1;

    return A;
}

template < typename Type >
bool collisionLinesOnPlane(Type & tA, Type & tB,
                           const Eigen::Matrix<Type, 3, 1> & pointA, const Eigen::Matrix<Type, 3, 1> & dirA,
                           const Eigen::Matrix<Type, 3, 1> & pointB, const Eigen::Matrix<Type, 3, 1> & dirB)
{
    Eigen::Matrix<Type, 3, 1> planeNormal = dirA.cross(dirB);
    Eigen::Matrix<Type, 3, 1> crossPlaneNormal = planeNormal.cross(dirB);
    Eigen::Matrix<Type, 3, 1> r;
    if (!collisionPlaneRay(r, tA, crossPlaneNormal, pointB, pointA, dirA))
        return false;
    crossPlaneNormal = planeNormal.cross(dirA);
    if (!collisionPlaneRay(r, tB, crossPlaneNormal, pointA, pointB, dirB))
        return false;
    return true;
}

template < typename Type >
bool collisionPlaneRay(Eigen::Matrix<Type, 3, 1> & result, Type & t,
                       const Eigen::Matrix<Type, 3, 1> & planeNormal, const Eigen::Matrix<Type, 3, 1> & planePoint,
                       const Eigen::Matrix<Type, 3, 1> & rayPoint, const Eigen::Matrix<Type, 3, 1> & rayDir)
{
    t = planeNormal.dot(rayDir);
    if (std::abs(t) < std::numeric_limits<Type>::epsilon())
        return false;
    t = - ((planeNormal.dot(rayPoint - planePoint)) / t);
    result = rayPoint + rayDir * t;
    return true;
}

template < typename Type >
Eigen::Matrix<Type, 3, 3> exp_rodrigues(const Eigen::Matrix<Type, 3, 1> & w, Type A, Type B)
{
    Eigen::Matrix<Type, 3, 3> rotationMatrix;
    {
        const Type wx2 = w(0) * w(0);
        const Type wy2 = w(1) * w(1);
        const Type wz2 = w(2) * w(2);

        rotationMatrix(0, 0) = Type(1) - B * (wy2 + wz2);
        rotationMatrix(1, 1) = Type(1) - B * (wx2 + wz2);
        rotationMatrix(2, 2) = Type(1) - B * (wx2 + wy2);
    }
    {
        const Type a = A * w(2);
        const Type b = B * (w(0) * w(1));
        rotationMatrix(0, 1) = b - a;
        rotationMatrix(1, 0) = b + a;
    }
    {
        const Type a = A * w(1);
        const Type b = B * (w(0) * w(2));
        rotationMatrix(0, 2) = b + a;
        rotationMatrix(2, 0) = b - a;
    }
    {
        const Type a = A * w(0);
        const Type b = B * (w(1) * w(2));
        rotationMatrix(1, 2) = b - a;
        rotationMatrix(2, 1) = b + a;
    }
    return rotationMatrix;
}

template < typename Type >
Eigen::Matrix<Type, 3, 3> exp_rotationMatrix(const Eigen::Matrix<Type, 3, 1> & w)
{
    static const Type one_6th = cast<Type>(1.0 / 6.0);
    static const Type one_20th = cast<Type>(1.0 / 20.0);

    const Type theta_sq = w.dot(w);
    const Type theta = std::sqrt(theta_sq);
    Type A, B;
    //Use a Taylor series expansion near zero. This is required for
    //accuracy, since sin t / t and (1-cos t)/t^2 are both 0/0.
    if (theta_sq < cast<Type>(1e-8)) {
        A = cast<Type>(1.0) - one_6th * theta_sq;
        B = cast<Type>(0.5);
    } else {
        if (theta_sq < 1e-6) {
            B = cast<Type>(0.5) - cast<Type>(0.25 * one_6th * theta_sq);
            A = cast<Type>(1.0) - cast<Type>(theta_sq * one_6th * (1.0 - one_20th * theta_sq));
        } else {
            const Type inv_theta = cast<Type>(1) / theta;
            A = std::sin(theta) * inv_theta;
            B = (cast<Type>(1) - std::cos(theta)) * (inv_theta * inv_theta);
        }
    }
    return exp_rodrigues(w, A, B);
}

template < typename Type >
void exp_transform(Eigen::Matrix<Type, 3, 3> & outRotationMatrix, Eigen::Matrix<Type, 3, 1> & outTranslation,
                   const Eigen::Matrix<Type, 6, 1> & mu)
{
    static const Type one_6th = cast<Type>(1.0 / 6.0);
    static const Type one_20th = cast<Type>(1.0 / 20.0);
    const Eigen::Matrix<Type, 3, 1> mu_3(mu(0), mu(1), mu(2));
    const Eigen::Matrix<Type, 3, 1> w(mu(3), mu(4), mu(5));
    const Type theta_square = w.dot(w);
    const Type theta = std::sqrt(theta_square);
    Type A, B;

    const Eigen::Matrix<Type, 3, 1> crossVector = w.cross(mu_3);
    if (theta_square < cast<Type>(1e-8)) {
        A = cast<Type>(1.0) - one_6th * theta_square;
        B = cast<Type>(0.5);
        outTranslation = mu_3 + cast<Type>(0.5) * crossVector;
    } else {
        Type C;
        if (theta_square < cast<Type>(1e-6)) {
            C = one_6th * (cast<Type>(1) - one_20th * theta_square);
            A = cast<Type>(1.0) - theta_square * C;
            B = cast<Type>(0.5) - cast<Type>(0.25 * one_6th * theta_square);
        } else {
            const Type inv_theta = cast<Type>(1) / theta;
            A = std::sin(theta) * inv_theta;
            B = (cast<Type>(1) - std::cos(theta)) * (inv_theta * inv_theta);
            C = (cast<Type>(1) - A) * (inv_theta * inv_theta);
        }
        outTranslation = mu_3 + B * crossVector + C * w.cross(crossVector);
    }
    outRotationMatrix = exp_rodrigues(w, A, B);
}

template < typename Type >
Eigen::Matrix<Type, 3, 1> ln_rotationMatrix(const Eigen::Matrix<Type, 3, 3> & rotationMatrix)
{
    static const Type m_sqrt1_2 = cast<Type>(0.707106781186547524401);

    Eigen::Matrix<Type, 3, 1> result;

    const Type cos_angle = (rotationMatrix(0, 0) + rotationMatrix(1, 1) + rotationMatrix(2, 2) - cast<Type>(1)) * cast<Type>(0.5);
    result(0) = (rotationMatrix(2, 1) - rotationMatrix(1, 2)) * cast<Type>(0.5);
    result(1) = (rotationMatrix(0, 2) - rotationMatrix(2, 0)) * cast<Type>(0.5);
    result(2) = (rotationMatrix(1, 0) - rotationMatrix(0, 1)) * cast<Type>(0.5);

    Type sin_angle_abs = std::sqrt(result(0) * result(0) + result(1) * result(1) + result(2) * result(2));
    if (cos_angle > m_sqrt1_2) {            // [0 - Pi/4[ use asin
        if (sin_angle_abs > cast<Type>(0)) {
            result *= std::asin(sin_angle_abs) / sin_angle_abs;
        }
    } else if (cos_angle > - m_sqrt1_2) {    // [Pi/4 - 3Pi/4[ use acos, but antisymmetric part
        const Type angle = std::acos(cos_angle);
        result *= angle / sin_angle_abs;
    } else {  // rest use symmetric part
        // antisymmetric part vanishes, but still large rotation, need information from symmetric part
        const Type angle = cast<Type>(M_PI) - std::asin(sin_angle_abs);
        const Type d0 = rotationMatrix(0, 0) - cos_angle,
            d1 = rotationMatrix(1, 1) - cos_angle,
            d2 = rotationMatrix(2, 2) - cos_angle;
        Eigen::Matrix<Type, 3, 1> r2;
        if (((d0 * d0) > (d1 * d1)) && ((d0 * d0) > (d2 * d2))) { // first is largest, fill with first column
            r2(0) = d0;
            r2(1) = (rotationMatrix(1, 0) + rotationMatrix(0, 1)) * cast<Type>(0.5);
            r2(2) = (rotationMatrix(0, 2) + rotationMatrix(2, 0)) * cast<Type>(0.5);
        } else if ((d1 * d1) > (d2 * d2)) { 			    // second is largest, fill with second column
            r2(0) = (rotationMatrix(1, 0) + rotationMatrix(0, 1)) * cast<Type>(0.5);
            r2(1) = d1;
            r2(2) = (rotationMatrix(2, 1) + rotationMatrix(1, 2)) * cast<Type>(0.5);
        } else {							    // third is largest, fill with third column
            r2(0) = (rotationMatrix(0, 2) + rotationMatrix(2, 0)) * cast<Type>(0.5);
            r2(1) = (rotationMatrix(2, 1) + rotationMatrix(1, 2)) * cast<Type>(0.5);
            r2(2) = d2;
        }
        // flip, if we point in the wrong direction!
        if (r2.dot(result) < cast<Type>(0))
            r2 *= cast<Type>(-1);
        r2.normalize();
        result = r2 * angle;
    }
    return result;
}

template < typename Type >
Eigen::Matrix<Type, 6, 1> ln_transform(const Eigen::Matrix<Type, 3, 3> & rotationMatrix,
                                       const Eigen::Matrix<Type, 3, 1> & translation)
{
    Eigen::Matrix<Type, 3, 1> rot = ln_rotationMatrix(rotationMatrix);
    const Type square_theta = rot.dot(rot);
    const Type theta = std::sqrt(square_theta);

    Type shtot = cast<Type>(0.5);
    if (theta > cast<Type>(0.00001)) {
        shtot = std::sin(theta * cast<Type>(0.5)) / theta;
    }

    // now do the rotation
    const Eigen::Matrix<Type, 3, 3> halfrotator = exp_rotationMatrix(rot * cast<Type>(-0.5));
    Eigen::Matrix<Type, 3, 1> rottrans = halfrotator * translation;

    if (theta > cast<Type>(0.001)) {
        rottrans -= (rot * ((dot(translation, rot) * (cast<Type>(1) - cast<Type>(2) * shtot) / (square_theta))));
    } else {
        rottrans -= (rot * ((dot(translation, rot) / cast<Type>(24))));
    }

    rottrans /= (cast<Type>(2) * shtot);

    Eigen::Matrix<Type, 6, 1> result;
    result(0) = rottrans(0);
    result(1) = rottrans(1);
    result(2) = rottrans(2);
    result(3) = rot(0);
    result(4) = rot(1);
    result(5) = rot(2);
    return result;
}

template < typename Type >
Eigen::Matrix<Type, 4, 1> generator_field(int i, const Eigen::Matrix<Type, 4, 1> & pos)
{
    Eigen::Matrix<Type, 4, 1> result;
    if (i < 3) {
        result.setZero();
        result(i) = pos(3);
        return result;
    }
    result(i % 3) = (Type)0;
    result((i + 1) % 3) = - pos((i + 2) % 3);
    result((i + 2) % 3) = pos((i + 1) % 3);
    result(3) = (Type)0;
    return result;
}

template < typename Type >
Eigen::Matrix<Type, 3, 3> exp_jacobian(const Eigen::Matrix<Type, 3, 1> & w,
                                       const Eigen::Matrix<Type, 3, 1> & point)
{
    Type l = w.dot(w);
    if (l < Type(1e-5)) {
        Eigen::Matrix<Type, 3, 3> result;
        result << Type(0), point.z(), - point.y(),
                  - point.z(), Type(0), point.x(),
                  point.y(), - point.x(), Type(0);
        return result;
    }
    Eigen::Matrix<Type, 3, 3> R = exp_rotationMatrix(w);
    return - R * skewMatrix(point) *
            ((w * w.transpose() +
              (R.transpose() - Eigen::Matrix<Type, 3, 3>::Identity()) * skewMatrix(w)) / l);
}

template <typename Type >
bool rayIntersectsAABB(Type & resultNear, Type & resultFar,
                       const Eigen::Matrix<Type, 2, 1> & origin, const Eigen::Matrix<Type, 2, 1> & ray,
                       const Eigen::Matrix<Type, 2, 1> & aabb_min, const Eigen::Matrix<Type, 2, 1> & aabb_max)
{
    Eigen::Matrix<Type, 2, 1> tMin;
    Eigen::Matrix<Type, 2, 1> tMax;
    if (std::abs(ray(0)) > std::numeric_limits<float>::epsilon()) {
        tMin(0) = (aabb_min(0) - origin(0)) / ray(0);
        tMax(0) = (aabb_max(0) - origin(0)) / ray(0);
    } else {
        tMin(0) = std::numeric_limits<float>::max();
        tMax(0) = - std::numeric_limits<float>::max();
    }
    if (std::abs(ray(1)) > std::numeric_limits<float>::epsilon()) {
        tMin(1) = (aabb_min(1) - origin(1)) / ray(1);
        tMax(1) = (aabb_max(1) - origin(1)) / ray(1);
    } else {
        tMin(1) = std::numeric_limits<float>::max();
        tMax(1) = - std::numeric_limits<float>::max();
    }
    Eigen::Matrix<Type, 2, 1> t1, t2;
    if (tMin(0) < tMax(0)) {
        t1(0) = tMin(0);
        t2(0) = tMax(0);
    } else {
        t1(0) = tMax(0);
        t2(0) = tMin(0);
    }
    if (tMin(1) < tMax(1)) {
        t1(1) = tMin(1);
        t2(1) = tMax(1);
    } else {
        t1(1) = tMax(1);
        t2(1) = tMin(1);
    }
    resultNear = std::max(t1(0), t1(1));
    resultFar = std::min(t2(0), t2(1));

    // if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
    if (resultFar < Type(0))
        return false;

    // if tmin > tmax, ray doesn't intersect AABB
    if (resultNear > resultFar)
        return false;

    return true;
}

template <typename Type >
bool rayIntersectsAABB(Type & resultNear, Type & resultFar,
                       const Eigen::Matrix<Type, 3, 1> & origin, const Eigen::Matrix<Type, 3, 1> & ray,
                       const Eigen::Matrix<Type, 3, 1> & aabb_min, const Eigen::Matrix<Type, 3, 1> & aabb_max)
{
    Eigen::Matrix<Type, 3, 1> tMin;
    Eigen::Matrix<Type, 3, 1> tMax;
    if (std::abs(ray(0)) > std::numeric_limits<float>::epsilon()) {
        tMin(0) = (aabb_min(0) - origin(0)) / ray(0);
        tMax(0) = (aabb_max(0) - origin(0)) / ray(0);
    } else {
        tMin(0) = std::numeric_limits<float>::max();
        tMax(0) = - std::numeric_limits<float>::max();
    }
    if (std::abs(ray(1)) > std::numeric_limits<float>::epsilon()) {
        tMin(1) = (aabb_min(1) - origin(1)) / ray(1);
        tMax(1) = (aabb_max(1) - origin(1)) / ray(1);
    } else {
        tMin(1) = std::numeric_limits<float>::max();
        tMax(1) = - std::numeric_limits<float>::max();
    }
    if (std::abs(ray(2)) > std::numeric_limits<float>::epsilon()) {
        tMin(2) = (aabb_min(2) - origin(2)) / ray(2);
        tMax(2) = (aabb_max(2) - origin(2)) / ray(2);
    } else {
        tMin(2) = std::numeric_limits<float>::max();
        tMax(2) = - std::numeric_limits<float>::max();
    }

    Eigen::Matrix<Type, 3, 1> t1, t2;
    if (tMin(0) < tMax(0)) {
        t1(0) = tMin(0);
        t2(0) = tMax(0);
    } else {
        t1(0) = tMax(0);
        t2(0) = tMin(0);
    }
    if (tMin(1) < tMax(1)) {
        t1(1) = tMin(1);
        t2(1) = tMax(1);
    } else {
        t1(1) = tMax(1);
        t2(1) = tMin(1);
    }
    if (tMin(2) < tMax(2)) {
        t1(2) = tMin(2);
        t2(2) = tMax(2);
    } else {
        t1(2) = tMax(2);
        t2(2) = tMin(2);
    }
    resultNear = std::max(std::max(t1(0), t1(1)), t1(2));
    resultFar = std::min(std::min(t2(0), t2(1)), t2(2));

    // if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
    if (resultFar < Type(0))
        return false;

    // if tmin > tmax, ray doesn't intersect AABB
    if (resultNear > resultFar)
        return false;

    return true;
}

void gridRaycast(std::vector<Eigen::Vector3i> & output,
                 const Eigen::Vector3f & start, const Eigen::Vector3f & end,
                 const Eigen::Vector3i & min, const Eigen::Vector3i & max)
{
    //TODO maybe change to 'Xiaolin Wu's line algorithm' ?

    auto mod = [] (float value, float modulus)
    {
        return std::fmod(std::fmod(value, modulus) + modulus,  modulus);
    };

    auto intbound = [&mod] (float s, float ds)
    {
        // Find the smallest positive t such that s+t*ds is an integer.
        if (ds < 0.0f) {
            s = mod(- s, 1.0f);
            // problem is now s + t * ds = 1
            return (1.0f - s) / - ds;
        } else {
            s = mod(s, 1.0f);
            // problem is now s + t * ds = 1
            return (1.0f - s) / ds;
        }
    };

    // From "A Fast Voxel Traversal Algorithm for Ray Tracing"
    // by John Amanatides and Andrew Woo, 1987
    // <http://www.cse.yorku.ca/~amana/research/grid.pdf>
    // <http://citeseer.ist.psu.edu/viewdoc/summary?doi=10.1.1.42.3443>
    // Extensions to the described algorithm:
    //   • Imposed a distance limit.
    //   • The face passed through to reach the current cube is provided to
    //     the callback.

    // The foundation of this algorithm is a parameterized representation of
    // the provided ray,
    //                    origin + t * direction,
    // except that t is not actually stored; rather, at any given point in the
    // traversal, we keep track of the *greater* t values which we would have
    // if we took a step sufficient to cross a cube boundary along that axis
    // (i.e. change the integer part of the coordinate) in the variables
    // tMaxX, tMaxY, and tMaxZ.

    // Cube containing origin point.
    /*int x = (int)((start(0) > 0.0f) ? floor(start(0)) : ceil(start(0)));
    int y = (int)((start(1) > 0.0f) ? floor(start(1)) : ceil(start(1)));
    int z = (int)((start(2) > 0.0f) ? floor(start(2)) : ceil(start(2)));
    int endX = (int)((end(0) > 0.0f) ? ceil(end(0)) : floor(end(0)));
    int endY = (int)((end(1) > 0.0f) ? ceil(end(1)) : floor(end(1)));
    int endZ = (int)((end(2) > 0.0f) ? ceil(end(2)) : floor(end(2)));*/
    int x = (int)(floor(start(0)));
    int y = (int)(floor(start(1)));
    int z = (int)(floor(start(2)));
    int endX = (int)(floor(end(0)));
    int endY = (int)(floor(end(1)));
    int endZ = (int)(floor(end(2)));
    float maxDist = (end - start).squaredNorm(); (void)maxDist;

    // Break out direction vector.
    float dx = (float)(endX - x);
    float dy = (float)(endY - y);
    float dz = (float)(endZ - z);

    // Direction to increment x,y,z when stepping.
    int stepX = signum((int)dx);
    int stepY = signum((int)dy);
    int stepZ = signum((int)dz);

    // See description above. The initial values depend on the fractional
    // part of the origin.
    float tMaxX = intbound(start(0), dx);
    float tMaxY = intbound(start(1), dy);
    float tMaxZ = intbound(start(2), dz);

    // The change in t when taking a step (always positive).
    float tDeltaX = ((float)stepX) / dx;
    float tDeltaY = ((float)stepY) / dy;
    float tDeltaZ = ((float)stepZ) / dz;

    // Avoids an infinite loop.
    if ((stepX == 0) && (stepY == 0) && (stepZ == 0)) {
        if ((x >= min(0)) && (x < max(0)) &&
            (y >= min(1)) && (y < max(1)) &&
            (z >= min(2)) && (z < max(2))) {

            output.push_back(Eigen::Vector3i(x, y, z));
        }
        return;
    }

    float dist = 0.0f; (void)dist;

    while (true) {

        if ((x >= min(0)) && (x < max(0)) &&
            (y >= min(1)) && (y < max(1)) &&
            (z >= min(2)) && (z < max(2))) {

            output.push_back(Eigen::Vector3i(x, y, z));

            //dist = (Eigen::Vector3f((float)x, (float)y, (float)z) - start).squaredNorm() - std::sqrt(2.0f) * 4.0f;
            //assert((dist <= maxDist));

            /*if (output.size() > 1500) {
                std::cerr << "Error, too many racyast voxels." << std::endl;
                throw std::out_of_range("Too many raycast voxels");
            }*/
        }

        //NOTE ?
        //if (((x - endX) * stepX > 0) || ((y - endY) * stepY > 0) || ((z - endZ) * stepZ > 0))
        //    break;
        dist = (Eigen::Vector3f((float)x, (float)y, (float)z) - start).squaredNorm();
        if (dist > maxDist)
            break;

        // tMaxX stores the t-value at which we cross a cube boundary along the
        // X axis, and similarly for Y and Z. Therefore, choosing the least tMax
        // chooses the closest cube boundary. Only the first case of the four
        // has been commented in detail.
        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) {
                // Update which cube we are now in.
                x += stepX;
                // Adjust tMaxX to the next X-oriented boundary crossing.
                tMaxX += tDeltaX;
            } else {
                z += stepZ;
                tMaxZ += tDeltaZ;
            }
        } else {
            if (tMaxY < tMaxZ) {
                y += stepY;
                tMaxY += tDeltaY;
            } else {
                z += stepZ;
                tMaxZ += tDeltaZ;
            }
        }
    }
}

} // namespace math_utils

} // namespace sonar

#endif // SONAR_MATHUTILS_IMPL_HPP
