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

class PerformanceMonitor;
class PinholeCamera;

class ObjectEdgesTracker:
        public DebugImageObject
{
    Q_OBJECT

    Q_PROPERTY(float controlPixelDistance READ controlPixelDistance WRITE setControlPixelDistance NOTIFY controlPixelDistanceChanged)
    Q_PROPERTY(float cannyThresholdA READ cannyThresholdA WRITE setCannyThresholdA NOTIFY cannyThresholdAChanged)
    Q_PROPERTY(float cannyThresholdB READ cannyThresholdB WRITE setCannyThresholdB NOTIFY cannyThresholdBChanged)
    Q_PROPERTY(QVector2D focalLength READ focalLength NOTIFY cameraChanged)
    Q_PROPERTY(QVector2D opticalCenter READ opticalCenter NOTIFY cameraChanged)
    Q_PROPERTY(QSize frameSize READ frameSize NOTIFY cameraChanged)
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

    QVector2D focalLength() const;
    QVector2D opticalCenter() const;
    QSize frameSize() const;

    QMatrix4x4 viewMatrix() const;

    void compute(cv::Mat image);

    cv::Mat debugImage() const override;

signals:
    void controlPixelDistanceChanged();
    void cannyThresholdAChanged();
    void cannyThresholdBChanged();
    void cameraChanged();

private:
    QSharedPointer<PerformanceMonitor> m_monitor;

    float m_controlPixelDistance;
    double m_cannyThresholdA;
    double m_cannyThresholdB;

    ObjectModel m_model;
    std::shared_ptr<PinholeCamera> m_camera;

    Eigen::Matrix3f m_R;
    Eigen::Vector3f m_t;

    cv::Mat m_debugImage;

    float _tracking1(const cv::Mat & edges);
    float _tracking2(const cv::Mat & edges);
};

#endif // OBJECTEDGESTRACKER_H
