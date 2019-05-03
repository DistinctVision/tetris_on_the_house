#include "framehandler.h"

#include <QDebug>
#include <QThreadPool>
#include <QTime>

#include "pinholecamera.h"
#include "objectedgestracker.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace Eigen;

FrameHandler::FrameHandler():
    m_frameSize(-1, -1)
{
    m_objectEdgesTracker = QSharedPointer<ObjectEdgesTracker>::create();
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
}

QVideoFrame FrameHandlerRunnable::run(QVideoFrame * videoFrame,
                                      const QVideoSurfaceFormat &surfaceFormat,
                                      QVideoFilterRunnable::RunFlags flags)
{
    Q_UNUSED(flags);

    ObjectEdgesTracker * objectEdgesTracking = m_parent->objectEdgesTracker();

    if (surfaceFormat.handleType() == QAbstractVideoBuffer::NoHandle)
    {
        Vector2i imageSize(videoFrame->width(), videoFrame->height());
        m_parent->_setFrameSize(videoFrame->size());
        cv::Mat frame(imageSize.y(), imageSize.x(), CV_8UC4);
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

        cv::flip(frame, frame, -1);
        cv::flip(frame, frame, 1);
        cv::cvtColor(frame, frame, cv::COLOR_BGRA2GRAY);
        cv::resize(frame, frame, cv::Size(imageSize.x(), imageSize.y()));
        if (!objectEdgesTracking->camera() ||
                (objectEdgesTracking->camera()->imageSize() != imageSize))
        {
            objectEdgesTracking->setCamera(std::make_shared<PinholeCamera>(imageSize,
                                                                           Vector2f(imageSize.x(), imageSize.x()) * 1.2f,
                                                                           imageSize.cast<float>() * 0.5));
        }
        objectEdgesTracking->compute(frame);
    }

    return QVideoFrame(*videoFrame);
}
