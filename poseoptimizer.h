#ifndef POSEOPTIMIZER_H
#define POSEOPTIMIZER_H

#include <memory>

#include <Eigen/Eigen>

#include "objectmodel.h"

class PinholeCamera;

void test_transfroms();

Eigen::Matrix3d skewMatrix(const Eigen::Vector3d & a);

Eigen::Matrix3d exp_rodrigues(const Eigen::Vector3d & w, double A, double B);
Eigen::Matrix3d exp_rotationMatrix(const Eigen::Vector3d & w);
Eigen::Vector3d ln_rotationMatrix(const Eigen::Matrix3d & rotationMatrix);

void optimize_pose(Eigen::Matrix3d & R, Eigen::Vector3d & t,
                   const std::shared_ptr<const PinholeCamera> & camera,
                   const Vectors3d & controlModelPoints,
                   const Vectors2f & controlImagePoints,
                   int numberIterations);

#endif // POSEOPTIMIZER_H
