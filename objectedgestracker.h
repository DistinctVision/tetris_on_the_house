#ifndef OBJECTEDGESTRACKER_H
#define OBJECTEDGESTRACKER_H

#include <memory>
#include <unordered_map>
#include <tuple>

#include <QMatrix4x4>

#include <Eigen/Eigen>

#include <opencv2/core.hpp>

#include "objectmodel.h"
#include "debugimageobject.h"

class PerformanceMonitor;
class PinholeCamera;

class ObjectEdgesTracker:
        public DebugImageObject
{
    Q_OBJECT
public:
    ObjectEdgesTracker();

    std::shared_ptr<PinholeCamera> camera() const;
    void setCamera(const std::shared_ptr<PinholeCamera> & camera);

    QMatrix4x4 viewMatrix() const;

    void compute(cv::Mat image);

    cv::Mat debugImage() const override;

private:
    float m_controlPixelDistance;

    std::shared_ptr<PerformanceMonitor> m_monitor;

    ObjectModel m_model;
    std::shared_ptr<PinholeCamera> m_camera;

    Eigen::Matrix3d m_R;
    Eigen::Vector3d m_t;

    cv::Mat m_debugImage;

    double _tracking(const cv::Mat & distanceMap);
};

#endif // OBJECTEDGESTRACKER_H
