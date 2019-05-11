#ifndef OBJECTEDGESTRACKER_H
#define OBJECTEDGESTRACKER_H

#include <memory>
#include <unordered_map>
#include <tuple>

#include <QVector2D>
#include <QMatrix4x4>
#include <QSharedPointer>

#include <Eigen/Eigen>

#include <opencv2/core.hpp>

#include "objectmodel.h"
#include "debugimageobject.h"
#include "posefilter.h"

class PerformanceMonitor;
class PinholeCamera;

class ObjectEdgesTracker:
        public DebugImageObject
{
    Q_OBJECT

    Q_PROPERTY(float controlPixelDistance READ controlPixelDistance WRITE setControlPixelDistance
               NOTIFY controlPixelDistanceChanged)
    Q_PROPERTY(double binaryThreshold READ binaryThreshold WRITE setBinaryThreshold
               NOTIFY binaryThresholdChanged)
    Q_PROPERTY(double minBlobArea READ minBlobArea WRITE setMinBlobArea NOTIFY minBlobAreaChanged)
    Q_PROPERTY(double maxBlobCircularity READ maxBlobCircularity WRITE setMaxBlobCircularity
               NOTIFY maxBlobCircularityChanged)
    Q_PROPERTY(QVector2D focalLength READ focalLength NOTIFY cameraChanged)
    Q_PROPERTY(QVector2D opticalCenter READ opticalCenter NOTIFY cameraChanged)
    Q_PROPERTY(QSize frameSize READ frameSize NOTIFY cameraChanged)
public:
    using Pose = PoseFilter::Pose;

    ObjectEdgesTracker(const QSharedPointer<PerformanceMonitor> & monitor);

    float controlPixelDistance() const;
    void setControlPixelDistance(float controlPixelDistance);

    double binaryThreshold() const;
    void setBinaryThreshold(double binaryThreshold);

    double minBlobArea() const;
    void setMinBlobArea(double minBlobArea);

    double maxBlobCircularity() const;
    void setMaxBlobCircularity(double maxBlobCircularity);

    std::shared_ptr<PinholeCamera> camera() const;
    void setCamera(const std::shared_ptr<PinholeCamera> & camera);

    QVector2D focalLength() const;
    QVector2D opticalCenter() const;
    QSize frameSize() const;

    QMatrix4x4 viewMatrix() const;

    void compute(cv::Mat image);

    cv::Mat debugImage() const override;

signals:
    void controlPixelDistanceChanged();
    void binaryThresholdChanged();
    void minBlobAreaChanged();
    void maxBlobCircularityChanged();
    void cameraChanged();

private:
    QSharedPointer<PerformanceMonitor> m_monitor;
    PoseFilter m_poseFilter;

    float m_controlPixelDistance;
    double m_binaryThreshold;
    double m_minBlobArea;
    double m_maxBlobCircularity;

    ObjectModel m_model;
    std::shared_ptr<PinholeCamera> m_camera;

    Pose m_resetCameraPose;

    cv::Mat m_debugImage;

    Eigen::Matrix<double, 6, 1> _pose2x(const Pose & pose) const;
    Pose _x2pose(const Eigen::Matrix<double, 6, 1> & x) const;

    float _tracking1(const cv::Mat & edges);
    float _tracking2(const cv::Mat & edges);
};

#endif // OBJECTEDGESTRACKER_H
