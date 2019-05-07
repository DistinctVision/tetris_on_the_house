#ifndef POSEOPTIMIZER_H
#define POSEOPTIMIZER_H

#include <memory>

#include <QThreadPool>

#include <Eigen/Eigen>

#include <opencv2/core.hpp>

#include "objectmodel.h"

class PinholeCamera;

void test_transfroms();

Eigen::Matrix3f skewMatrix(const Eigen::Vector3f & a);

Eigen::Matrix3f exp_rodrigues(const Eigen::Vector3f & w, float A, float B);
Eigen::Matrix3f exp_rotationMatrix(const Eigen::Vector3f & w);
Eigen::Vector3f ln_rotationMatrix(const Eigen::Matrix3f & rotationMatrix);

double optimize_pose(Eigen::Matrix3d & R, Eigen::Vector3d & t,
                     const std::shared_ptr<const PinholeCamera> & camera,
                     const Vectors3d & controlModelPoints,
                     const Vectors2f & viewPoints,
                     float maxDistance,
                     int numberIterations);

double optimize_pose(Eigen::Matrix<double, 6, 1> & x,
                     const std::shared_ptr<const PinholeCamera> & camera,
                     const Vectors3d & controlModelPoints,
                     const Vectors2f & viewPoints,
                     float maxDistance,
                     int numberIterations);

float optimize_pose(Eigen::Matrix3f & R, Eigen::Vector3f & t,
                    QThreadPool * pool, size_t numberWorkThreads,
                    const cv::Mat & distanceMap,
                    const std::shared_ptr<const PinholeCamera> & camera,
                    const Vectors3f & modelPoints,
                    float maxDistance,
                    int numberIterations);

float optimize_pose(Eigen::Matrix<float, 6, 1> & x,
                     QThreadPool * pool, size_t numberWorkThreads,
                     const cv::Mat & distanceMap,
                     const std::shared_ptr<const PinholeCamera> & camera,
                     const Vectors3f & modelPoints,
                     float maxDistance,
                     int numberIterations);

#endif // POSEOPTIMIZER_H
