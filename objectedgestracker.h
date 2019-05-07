#ifndef OBJECTEDGESTRACKER_H
#define OBJECTEDGESTRACKER_H

#include <memory>
#include <unordered_map>
#include <tuple>

#include <QMatrix4x4>
#include <QSharedPointer>

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

    Q_PROPERTY(float controlPixelDistance READ controlPixelDistance WRITE setControlPixelDistance NOTIFY controlPixelDistanceChanged)
    Q_PROPERTY(float cannyThresholdA READ cannyThresholdA WRITE setCannyThresholdA NOTIFY cannyThresholdAChanged)
    Q_PROPERTY(float cannyThresholdB READ cannyThresholdB WRITE setCannyThresholdB NOTIFY cannyThresholdBChanged)
public:
    ObjectEdgesTracker(const QSharedPointer<PerformanceMonitor> & monitor);

    float controlPixelDistance() const;
    void setControlPixelDistance(float controlPixelDistance);

    double cannyThresholdA() const;
    void setCannyThresholdA(double cannyThresholdA);

    double cannyThresholdB() const;
    void setCannyThresholdB(double cannyThresholdB);

    std::shared_ptr<PinholeCamera> camera() const;
    void setCamera(const std::shared_ptr<PinholeCamera> & camera);

    QMatrix4x4 viewMatrix() const;

    void compute(cv::Mat image);

    cv::Mat debugImage() const override;

signals:
    void controlPixelDistanceChanged();
    void cannyThresholdAChanged();
    void cannyThresholdBChanged();

private:
    QSharedPointer<PerformanceMonitor> m_monitor;

    float m_controlPixelDistance;
    double m_cannyThresholdA;
    double m_cannyThresholdB;

    ObjectModel m_model;
    std::shared_ptr<PinholeCamera> m_camera;

    Eigen::Matrix3d m_R;
    Eigen::Vector3d m_t;

    cv::Mat m_debugImage;

    double _tracking1(const cv::Mat & edges);
    double _tracking2(const cv::Mat & edges);
};

#endif // OBJECTEDGESTRACKER_H
