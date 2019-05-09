#ifndef POSEOPTIMIZER2_H
#define POSEOPTIMIZER2_H

#include "poseoptimizer.h"

float optimize_pose(Eigen::Matrix3f & R, Eigen::Vector3f & t,
                    const std::shared_ptr<const PinholeCamera> & camera,
                    const Vectors3f & controlModelPoints,
                    const Vectors2f & imagePoints,
                    float maxDistance,
                    int numberIterations);

float optimize_pose(Eigen::Matrix<float, 6, 1> & x,
                    const std::shared_ptr<const PinholeCamera> & camera,
                    const Vectors3f & controlModelPoints,
                    const Vectors2f & imagePoints,
                    float maxDistance,
                    int numberIterations);

#endif // POSEOPTIMIZER2_H
