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
Eigen::Matrix3d skewMatrix(const Eigen::Vector3d & a);

Eigen::Matrix3f exp_rodrigues(const Eigen::Vector3f & w, float A, float B);
Eigen::Matrix3f exp_rotationMatrix(const Eigen::Vector3f & w);
Eigen::Vector3f ln_rotationMatrix(const Eigen::Matrix3f & rotationMatrix);
Eigen::Matrix3d exp_rodrigues(const Eigen::Vector3d & w, double A, double B);
Eigen::Matrix3d exp_rotationMatrix(const Eigen::Vector3d & w);
Eigen::Vector3d ln_rotationMatrix(const Eigen::Matrix3d & rotationMatrix);

Eigen::Matrix3f exp_jacobian(const Eigen::Vector3f & w, const Eigen::Vector3f & point);
Eigen::Matrix3d exp_jacobian(const Eigen::Vector3d & w, const Eigen::Vector3d & point);

float optimize_pose(Eigen::Matrix3f & R, Eigen::Vector3f & t,
                    QThreadPool * pool, size_t numberWorkThreads,
                    const cv::Mat & distanceMap,
                    const std::shared_ptr<const PinholeCamera> & camera,
                    const Vectors3f & modelPoints,
                    float maxDistance,
                    int numberIterations,
                    double lambdaViewPosition = -1.0,
                    const Eigen::Vector3d & prevViewPosition = Eigen::Vector3d::Zero());

double optimize_pose(Eigen::Matrix<double, 6, 1> & x,
                     QThreadPool * pool, size_t numberWorkThreads,
                     const cv::Mat & distanceMap,
                     const std::shared_ptr<const PinholeCamera> & camera,
                     const Vectors3f & modelPoints,
                     double maxDistance,
                     int numberIterations,
                     double lambdaViewPosition = -1.0,
                     const Eigen::Vector3d & prevViewPosition = Eigen::Vector3d::Zero());

#endif // POSEOPTIMIZER_H
