#ifndef FRAMEHANDLER_H
#define FRAMEHANDLER_H

#include <QSize>
#include <QSharedPointer>
#include <QVideoFrame>
#include <QVector2D>
#include <QVideoSurfaceFormat>
#include <QAbstractVideoFilter>
#include <QOpenGLFunctions>

#include <opencv2/core.hpp>

class PerformanceMonitor;
class ObjectEdgesTracker;
class Texture2GrayImageConvertor;

class FrameHandlerRunnable;

class FrameHandler: public QAbstractVideoFilter
{
    Q_OBJECT

    Q_PROPERTY(ObjectEdgesTracker* objectEdgesTracker READ objectEdgesTracker CONSTANT)
    Q_PROPERTY(QSize frameSize READ frameSize NOTIFY frameSizeChanged)
    Q_PROPERTY(QSize maxFrameSize READ maxFrameSize WRITE setMaxFrameSize NOTIFY maxFrameSizeChanged)
    Q_PROPERTY(int orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(bool flipHorizontally READ flipHorizontally WRITE setFlipHorizontally NOTIFY flipHorizontallyChanged)
    Q_PROPERTY(QVector2D focalLength READ focalLength WRITE setFocalLength NOTIFY focalLengthChanged)
    Q_PROPERTY(QVector2D opticalCenter READ opticalCenter WRITE setOpticalCenter NOTIFY opticalCenterChanged)

public:
    FrameHandler();

    QVideoFilterRunnable * createFilterRunnable() override;

    ObjectEdgesTracker * objectEdgesTracker() const;

    QSize frameSize() const;

    QSize maxFrameSize() const;
    void setMaxFrameSize(const QSize & maxFrameSize);

    int orientation() const;
    void setOrientation(int orientation);

    bool flipHorizontally() const;
    void setFlipHorizontally(bool flipHorizontally);

    QVector2D focalLength() const;
    void setFocalLength(const QVector2D & focalLength);

    QVector2D opticalCenter() const;
    void setOpticalCenter(const QVector2D & opticalCenter);

    QSharedPointer<PerformanceMonitor> monitor() const;

signals:
    void frameSizeChanged();
    void maxFrameSizeChanged();
    void orientationChanged();
    void flipHorizontallyChanged();
    void focalLengthChanged();
    void opticalCenterChanged();

private:
    friend class FrameHandlerRunnable;

    QSharedPointer<PerformanceMonitor> m_monitor;
    QSharedPointer<ObjectEdgesTracker> m_objectEdgesTracker;
    QSize m_frameSize;
    QSize m_maxFrameSize;
    int m_orientation;
    bool m_flipHorizontally;

    QVector2D m_focalLength;
    QVector2D m_opticalCenter;

    void _setFrameSize(const QSize & frameSize);
};

class FrameHandlerRunnable: public QVideoFilterRunnable, QOpenGLFunctions
{
public:
    FrameHandlerRunnable(FrameHandler * parent);

    QVideoFrame run(QVideoFrame * videoFrame,
                    const QVideoSurfaceFormat &surfaceFormat,
                    RunFlags flags) override;

private:
    FrameHandler * m_parent;
    QSharedPointer<Texture2GrayImageConvertor> m_texture2GrayImageConverter;
    void _transformImage(cv::Mat & image) const;
};

#endif // FRAMEHANDLER_H
