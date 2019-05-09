#include "framehandler.h"

#include <QDebug>
#include <QThreadPool>
#include <QTime>

#include "performancemonitor.h"
#include "pinholecamera.h"
#include "objectedgestracker.h"
#include "texture2grayimageconvertor.h"

#include <opencv2/imgproc.hpp>

using namespace Eigen;

FrameHandler::FrameHandler():
    m_frameSize(-1, -1),
    m_maxFrameSize(640, 480),
    m_orientation(0),
    m_flipHorizontally(false),
    m_focalLength(1.0f, 1.0f),
    m_opticalCenter(0.5f, 0.5f)
{
    m_monitor = QSharedPointer<PerformanceMonitor>::create();
    m_objectEdgesTracker = QSharedPointer<ObjectEdgesTracker>::create(m_monitor);
}

QVideoFilterRunnable * FrameHandler::createFilterRunnable()
{
    return new FrameHandlerRunnable(this);
}

ObjectEdgesTracker * FrameHandler::objectEdgesTracker() const
{
    return m_objectEdgesTracker.get();
}

QSize FrameHandler::frameSize() const
{
    return m_frameSize;
}

QSize FrameHandler::maxFrameSize() const
{
    return m_maxFrameSize;
}

void FrameHandler::setMaxFrameSize(const QSize & maxFrameSize)
{
    if (maxFrameSize == m_maxFrameSize)
        return;
    m_maxFrameSize = maxFrameSize;
    emit maxFrameSizeChanged();
}

int FrameHandler::orientation() const
{
    return m_orientation;
}

void FrameHandler::setOrientation(int orientation)
{
    if (orientation == m_orientation)
        return;
    m_orientation = orientation;
    emit orientationChanged();
}

bool FrameHandler::flipHorizontally() const
{
    return m_flipHorizontally;
}

void FrameHandler::setFlipHorizontally(bool flipHorizontally)
{
    if (flipHorizontally == m_flipHorizontally)
        return;
    m_flipHorizontally = flipHorizontally;
    emit flipHorizontallyChanged();
}

QVector2D FrameHandler::focalLength() const
{
    return m_focalLength;
}

void FrameHandler::setFocalLength(const QVector2D & focalLength)
{
    if (m_focalLength == focalLength)
        return;
    m_focalLength = focalLength;
    emit focalLengthChanged();
}

QVector2D FrameHandler::opticalCenter() const
{
    return m_opticalCenter;
}

void FrameHandler::setOpticalCenter(const QVector2D & opticalCenter)
{
    if (m_opticalCenter == opticalCenter)
        return;
    m_opticalCenter = opticalCenter;
    emit opticalCenterChanged();
}

QSharedPointer<PerformanceMonitor> FrameHandler::monitor() const
{
    return m_monitor;
}

void FrameHandler::_setFrameSize(const QSize & frameSize)
{
    if (frameSize == m_frameSize)
        return;
    m_frameSize = frameSize;
    emit frameSizeChanged();
}

FrameHandlerRunnable::FrameHandlerRunnable(FrameHandler * parent):
    m_parent(parent)
{
    initializeOpenGLFunctions();
}

QVideoFrame FrameHandlerRunnable::run(QVideoFrame * videoFrame,
                                      const QVideoSurfaceFormat & surfaceFormat,
                                      QVideoFilterRunnable::RunFlags flags)
{
    Q_UNUSED(flags);

    cv::Mat frame;

    QSharedPointer<PerformanceMonitor> monitor = m_parent->monitor();

    monitor->start();
    monitor->startTimer("Getting frame");
    if (surfaceFormat.handleType() == QAbstractVideoBuffer::NoHandle)
    {
        Vector2i imageSize(videoFrame->width(), videoFrame->height());
        m_parent->_setFrameSize(videoFrame->size());
        frame = cv::Mat(imageSize.y(), imageSize.x(), CV_8UC4);
        if (videoFrame->map(QAbstractVideoBuffer::ReadOnly))
        {
            std::memcpy(frame.data, videoFrame->bits(),
                        static_cast<size_t>(imageSize.x() * imageSize.y() * 4));
            videoFrame->unmap();
        }
        else
        {
            qFatal("Coudn't read video frame");
        }
        cv::cvtColor(frame, frame, cv::COLOR_BGRA2GRAY);
        _transformImage(frame);
        QSize maxSize = m_parent->maxFrameSize();
        double scale1 = maxSize.width() / static_cast<double>(frame.cols);
        double scale2 = maxSize.height() / static_cast<double>(frame.rows);
        double scale = std::min(scale1, scale2);
        if (scale < 1.0)
            cv::resize(frame, frame, cv::Size(), scale, scale);
    }
    else if (surfaceFormat.handleType() == QAbstractVideoBuffer::GLTextureHandle)
    {
        if (!m_texture2GrayImageConverter)
        {
            m_texture2GrayImageConverter = QSharedPointer<Texture2GrayImageConvertor>::create();
        }
        frame = m_texture2GrayImageConverter->read(this, videoFrame->handle().toUInt(),
                                                   videoFrame->size(), m_parent->maxFrameSize(),
                                                   m_parent->orientation(),
                                                   m_parent->flipHorizontally());
    }
    monitor->endTimer("Getting frame");

    if (!frame.empty())
    {
        QVector2D focalLength = m_parent->focalLength();
        QVector2D opticalCenter = m_parent->opticalCenter();
        Vector2i v_imageSize(frame.cols, frame.rows);
        Vector2f v_focalLength(frame.cols * focalLength.x(),
                               frame.cols * focalLength.y());
        Vector2f v_opticalCenter(frame.cols * opticalCenter.x(),
                                 frame.rows * opticalCenter.y());
        ObjectEdgesTracker * objectEdgesTracking = m_parent->objectEdgesTracker();
        std::shared_ptr<const PinholeCamera> prevCamera = objectEdgesTracking->camera();
        if (!prevCamera || (prevCamera->imageSize() == v_imageSize) ||
                ((v_focalLength - prevCamera->focalLength()).array().sum() < 1e-4f) ||
                ((v_opticalCenter - prevCamera->opticalCenter()).array().sum() < 1e-4f))
        {
            objectEdgesTracking->setCamera(std::make_shared<PinholeCamera>(v_imageSize, v_focalLength, v_opticalCenter));
        }
        objectEdgesTracking->compute(frame);
    }

    monitor->end();
    qDebug().noquote() << QString::fromStdString(monitor->report());

    return QVideoFrame(*videoFrame);
}

void FrameHandlerRunnable::_transformImage(cv::Mat & image) const
{
    int orientation = ((m_parent->orientation() / 90) % 4) * 90;
    bool transpose = false;
    bool flipVertically = false, flipHorizontally = m_parent->flipHorizontally();
    switch (orientation)
    {
    case 90:
        transpose = true;
        std::swap(flipVertically, flipHorizontally);
        flipHorizontally = !flipHorizontally;
        break;
    case 180:
        transpose = false;
        flipHorizontally = !flipHorizontally;
        flipVertically = !flipVertically;
        break;
    case 270:
        transpose = true;
        std::swap(flipVertically, flipHorizontally);
        flipVertically = !flipVertically;
        break;
    }
    if (transpose)
        cv::transpose(image, image);
    if (flipHorizontally)
    {
        if (flipVertically)
            cv::flip(image, image, -1);
        else
            cv::flip(image, image, 0);
    }
    else if (flipVertically)
    {
        cv::flip(image, image, 1);
    }
}
