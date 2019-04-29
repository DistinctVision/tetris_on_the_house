#ifndef OBJECTEDGESTRACKING_H
#define OBJECTEDGESTRACKING_H

#include <memory>

#include <Eigen/Eigen>

#include <opencv2/core.hpp>

#include "objectmodel.h"

class PerformanceMonitor;

class ObjectEdgesTracking
{
public:
    ObjectEdgesTracking();

    void compute(cv::Mat image);

private:
    std::shared_ptr<PerformanceMonitor> m_monitor;

    ObjectModel m_model;

    Eigen::Matrix3d m_R;
    Eigen::Vector3d m_t;
};

#endif // OBJECTEDGESTRACKING_H
