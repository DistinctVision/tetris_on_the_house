#ifndef POSEOPTIMIZER_H
#define POSEOPTIMIZER_H

#include <Eigen/Eigen>

#include "objectmodel.h"

Eigen::Matrix3d exp_rodrigues(const Eigen::Vector3d & w, double A, double B);

Eigen::Matrix3d exp_rotationMatrix(const Eigen::Vector3d & w);

void exp_transform(Eigen::Matrix3d & outRotationMatrix, Eigen::Vector3d & outTranslation,
                   const Eigen::Matrix<double, 6, 1> & mu);

void optimize_pose(Eigen::Matrix3d & R, Eigen::Vector3d & t,
                   const Vectors3d & controlModelPoints,
                   const Vectors2f & controlImagePoints);

#endif // POSEOPTIMIZER_H
