#ifndef OBJECTEDGESTRACKING_H
#define OBJECTEDGESTRACKING_H

#include <memory>
#include <unordered_map>
#include <tuple>

#include <Eigen/Eigen>

#include <opencv2/core.hpp>

#include "objectmodel.h"

class PerformanceMonitor;
class PinholeCamera;

class ObjectEdgesTracking
{
public:
    ObjectEdgesTracking();

    std::shared_ptr<PinholeCamera> camera() const;
    void setCamera(const std::shared_ptr<PinholeCamera> & camera);

    void compute(cv::Mat image);

private:
    float m_controlPixelDistance;

    std::shared_ptr<PerformanceMonitor> m_monitor;

    ObjectModel m_model;
    std::shared_ptr<PinholeCamera> m_camera;

    Eigen::Matrix3d m_R;
    Eigen::Vector3d m_t;

    std::tuple<Vectors3d, Vectors2f>
    _getCurrentResiduals(const cv::Mat & labels,
                         const std::unordered_map<int, cv::Point2i> & index2point) const;
    void _drawCurrentResiduals(const cv::Mat & image,
                               const Vectors3d & controlModelPoints,
                               const Vectors2f & controlImagePoints) const;
};

#endif // OBJECTEDGESTRACKING_H
