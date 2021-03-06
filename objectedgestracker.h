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

struct TrackingQuality
{
    Q_GADGET
public:
    enum Enum
    {
        Good,
        Bad,
        Ugly
    };
    Q_ENUM(Enum)
};

class PerformanceMonitor;
class PinholeCamera;

class ObjectEdgesTracker:
        public DebugImageObject
{
    Q_OBJECT

    Q_PROPERTY(bool useLaplacian READ useLaplacian WRITE setUseLaplacian NOTIFY useLaplacianChanged)
    Q_PROPERTY(bool useAdaptiveBinarization READ useAdaptiveBinarization
               WRITE setUseAdaptiveBinarization NOTIFY useAdaptiveBinarizationChanged)
    Q_PROPERTY(int adaptiveBinarizationWinSize READ adaptiveBinarizationWinSize
               WRITE setAdaptiveBinarizationWinSize NOTIFY adaptiveBinarizationWinSizeChanged)
    Q_PROPERTY(bool useDilate READ useDilate WRITE setUseDilate NOTIFY useDilateChanged)
    Q_PROPERTY(bool useErode READ useErode WRITE setUseErode NOTIFY useErodeChanged)

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
    Q_PROPERTY(TrackingQuality::Enum trackingQuality READ trackingQuality NOTIFY trackingQualityChanged)
public:
    using Pose = PoseFilter::Pose;

    ObjectEdgesTracker(const QSharedPointer<PerformanceMonitor> & monitor);

    bool useLaplacian() const;
    void setUseLaplacian(bool useLaplacian);

    bool useAdaptiveBinarization() const;
    void setUseAdaptiveBinarization(bool useAdaptiveBinarization);

    int adaptiveBinarizationWinSize() const;
    void setAdaptiveBinarizationWinSize(int adaptiveBinarizationWinSize);

    bool useDilate() const;
    void setUseDilate(bool useDilate);

    bool useErode() const;
    void setUseErode(bool useErode);

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

    TrackingQuality::Enum trackingQuality() const;

    QMatrix4x4 viewMatrix() const;

    void compute(cv::Mat image);

    cv::Mat debugImage() const override;

signals:
    void controlPixelDistanceChanged();
    void binaryThresholdChanged();
    void minBlobAreaChanged();
    void maxBlobCircularityChanged();
    void cameraChanged();
    void trackingQualityChanged();
    void useLaplacianChanged();
    void useAdaptiveBinarizationChanged();
    void adaptiveBinarizationWinSizeChanged();
    void useDilateChanged();
    void useErodeChanged();

private:
    bool m_useLaplacian;
    bool m_useAdaptiveBinarization;
    int m_adaptiveBinarizationWinSize;
    bool m_useDilate;
    bool m_useErode;

    QSharedPointer<PerformanceMonitor> m_monitor;
    PoseFilter m_poseFilter;

    float m_controlPixelDistance;
    double m_binaryThreshold;
    double m_minBlobArea;
    double m_maxBlobCircularity;

    ObjectModel m_model;
    std::shared_ptr<PinholeCamera> m_camera;

    Pose m_resetCameraPose;

    TrackingQuality::Enum m_trackingQuality;

    cv::Mat m_debugImage;

    Eigen::Matrix<double, 6, 1> _pose2x(const Pose & pose) const;
    Pose _x2pose(const Eigen::Matrix<double, 6, 1> & x) const;

    float _tracking1(const cv::Mat & edges);
    float _tracking2(const cv::Mat & edges);

    TrackingQuality::Enum _error2quality(float error) const;
    void _setTrackingQuality(TrackingQuality::Enum quality);
};

#endif // OBJECTEDGESTRACKER_H
