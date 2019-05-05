#include "framehandler.h"

#include <QDebug>
#include <QThreadPool>
#include <QTime>

#include "pinholecamera.h"
#include "objectedgestracker.h"
#include "texture2grayimageconvertor.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace Eigen;

FrameHandler::FrameHandler():
    m_frameSize(-1, -1),
    m_maxFrameSize(640, 480)
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
    }
    else if (surfaceFormat.handleType() == QAbstractVideoBuffer::GLTextureHandle)
    {
        if (!m_texture2GrayImageConverter)
        {
            m_texture2GrayImageConverter = QSharedPointer<Texture2GrayImageConvertor>::create();
        }
        frame = m_texture2GrayImageConverter->read(this, videoFrame->handle().toUInt(),
                                                   videoFrame->size(), m_parent->maxFrameSize());
    }

    if (!frame.empty())
    {
        ObjectEdgesTracker * objectEdgesTracking = m_parent->objectEdgesTracker();
        if (frame.type() == CV_8UC3)
            cv::cvtColor(frame, frame, cv::COLOR_BGRA2GRAY);
        if (!objectEdgesTracking->camera() ||
                (objectEdgesTracking->camera()->imageSize() != Vector2i(frame.cols, frame.rows)))
        {
            //TODO set camera parameters
            objectEdgesTracking->setCamera(std::make_shared<PinholeCamera>(Vector2i(frame.cols, frame.rows),
                                                                           Vector2f(frame.cols, frame.cols) * 1.2f,
                                                                           Vector2f(frame.cols, frame.rows) * 0.5f));
        }
        objectEdgesTracking->compute(frame);
    }

    return QVideoFrame(*videoFrame);
}
