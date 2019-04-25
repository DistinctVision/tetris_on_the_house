/**
* This file is part of sonar library
* Copyright (C) 2019 Vlasov Aleksey ijonsilent53@gmail.com
* For more information see <https://github.com/DistinctVision/sonar>
**/

#ifndef SONAR_MATHUTILS_H
#define SONAR_MATHUTILS_H

#include <vector>
#include <list>
#include <cmath>
#include <limits>
#include <climits>
#include <initializer_list>
#include <exception>
#include <cstdlib>
#include<iostream>

#include <Eigen/Core>
#include <Eigen/SVD>
#include <Eigen/Geometry>

#include <sonar/General/Point2.h>
#include <sonar/General/cast.h>

namespace sonar {

namespace math_utils {

static int pow(int a, int b);

static char signum(char x);
static int signum(int x);
static float signum(float x);
static double signum(double x);

template < typename Type>
inline Point2<Type> projectToPoint(const Eigen::Matrix<Type, 3, 1> & v);

template < typename Type >
inline Eigen::Matrix<Type, 3, 1> unprojectPoint(const Point2<Type> & p);

template < typename Type >
inline Eigen::Matrix<Type, 2, 1> project(const Eigen::Matrix<Type, 3, 1> & v);

template < typename Type >
inline Eigen::Matrix<Type, 3, 1> project(const Eigen::Matrix<Type, 4, 1> & v);

template < typename Type >
inline Eigen::Matrix<Type, 3, 1> unproject(const Eigen::Matrix<Type, 2, 1> & v);

template < typename Type >
inline Eigen::Matrix<Type, 4, 1> unproject(const Eigen::Matrix<Type, 3, 1> & v);

template < typename Type >
inline Eigen::Matrix<Type, 3, 3> skewMatrix(const Eigen::Matrix<Type, 3, 1> & a);

template < typename Type >
static Eigen::AlignedBox<Type, 2> getBoundedRect(const std::vector<Point2<Type>> & points);

template < typename Type, int size >
inline Eigen::AlignedBox<Type, size> getBoundedRect(const std::vector<Eigen::Matrix<Type, size, 1>> & points);

template < typename Type, int size >
inline Type supportValue(const Eigen::Matrix<Type, size, 1> & dir,
                         const std::vector<Eigen::Matrix<Type, size, 1>> & convex_points);

template < typename Type >
static std::vector<Point2<Type>> transformPoints(const Eigen::Matrix<Type, 3, 3> & transformMatrix,
                                                 const std::vector<Point2<Type>> & points);

template < typename Type >
static Eigen::Matrix<Type, 3, 3> calculateHomography(const std::vector<Point2<Type>> & pointsA,
                                                     const std::vector<Point2<Type>> & pointsB);

template < typename Type >
static Eigen::Matrix<Type, 3, 3> calculateRigidTransform(const std::vector<Point2<Type>> & pointsA,
                                                         const std::vector<Point2<Type>> & pointsB);

template < typename Type >
static bool collisionLinesOnPlane(Type & tA, Type & tB,
                                  const Eigen::Matrix<Type, 3, 1> & pointA, const Eigen::Matrix<Type, 3, 1> & dirA,
                                  const Eigen::Matrix<Type, 3, 1> & pointB, const Eigen::Matrix<Type, 3, 1> & dirB);

template < typename Type >
static bool collisionPlaneRay(Eigen::Matrix<Type, 3, 1> & result, Type & t,
                              const Eigen::Matrix<Type, 3, 1> & planeNormal, const Eigen::Matrix<Type, 3, 1> & planePoint,
                              const Eigen::Matrix<Type, 3, 1> & rayPoint, const Eigen::Matrix<Type, 3, 1> & rayDir);

template < typename Type>
static Eigen::Matrix<Type, 3, 3> exp_rodrigues(const Eigen::Matrix<Type, 3, 1> & w, const Type A, const Type B);

template < typename Type>
static Eigen::Matrix<Type, 3, 3> exp_rotationMatrix(const Eigen::Matrix<Type, 3, 1> & w);

template < typename Type >
static void exp_transform(Eigen::Matrix<Type, 3, 3> & outRotationMatrix, Eigen::Matrix<Type, 3, 1> & outTranslation,
                          const Eigen::Matrix<Type, 6, 1> & mu);

template < typename Type >
static Eigen::Matrix<Type, 3, 1> ln_rotationMatrix(const Eigen::Matrix<Type, 3, 3> & rotationMatrix);

template < typename Type >
static Eigen::Matrix<Type, 6, 1> ln_transform(const Eigen::Matrix<Type, 3, 3> & rotationMatrix,
                                              const Eigen::Matrix<Type, 3, 1> & translation);

template < typename Type >
inline static Eigen::Matrix<Type, 4, 1> generator_field(int i, const Eigen::Matrix<Type, 4, 1> & pos);

template < typename Type >
inline static Eigen::Matrix<Type, 3, 3> exp_jacobian(const Eigen::Matrix<Type, 3, 1> & w,
                                                     const Eigen::Matrix<Type, 3, 1> & point);

template < typename Type >
inline static bool rayIntersectsAABB(Type & resultNear, Type & resultFar,
                                     const Eigen::Matrix<Type, 2, 1> & origin,
                                     const Eigen::Matrix<Type, 2, 1> & ray,
                                     const Eigen::Matrix<Type, 2, 1> & aabb_min,
                                     const Eigen::Matrix<Type, 2, 1> & aabb_max);

template < typename Type >
inline static bool rayIntersectsAABB(Type & resultNear, Type & resultFar,
                                     const Eigen::Matrix<Type, 2, 1> & origin,
                                     const Eigen::Matrix<Type, 2, 1> & ray,
                                     const Eigen::Matrix<Type, 3, 1> & aabb_min,
                                     const Eigen::Matrix<Type, 3, 1> & aabb_max);

static void gridRaycast(std::vector<Eigen::Vector3i> & output,
                        const Eigen::Vector3f & start, const Eigen::Vector3f & end,
                        const Eigen::Vector3i & min, const Eigen::Vector3i & max);

} // namespace math_utils

} // namespace sonar

#include "impl/MathUtils_impl.hpp"
#endif// SONAR_MATHUTILS_H
