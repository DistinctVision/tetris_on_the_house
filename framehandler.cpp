#include "framehandler.h"

#include <QDebug>
#include <QThreadPool>
#include <QTime>

#include "objectedgestracking.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

QVideoFilterRunnable * FrameHandler::createFilterRunnable()
{
    return new FrameHandlerRunnable();
}

FrameHandlerRunnable::FrameHandlerRunnable()
{
    m_objectEdgesTracking = std::make_shared<ObjectEdgesTracking>();
}

QVideoFrame FrameHandlerRunnable::run(QVideoFrame * videoFrame,
                                      const QVideoSurfaceFormat &surfaceFormat,
                                      QVideoFilterRunnable::RunFlags flags)
{
    Q_UNUSED(flags);

    if (surfaceFormat.handleType() == QAbstractVideoBuffer::NoHandle)
    {
        cv::Mat frame(videoFrame->height(), videoFrame->width(), CV_8UC4);
        if (videoFrame->map(QAbstractVideoBuffer::ReadOnly))
        {
            std::memcpy(frame.data, videoFrame->bits(),
                        static_cast<size_t>(videoFrame->width() * videoFrame->height() * 4));
            videoFrame->unmap();
        }
        else
        {
            qFatal("Coudn't read video frame");
        }

        cv::cvtColor(frame, frame, CV_BGRA2GRAY);
        m_objectEdgesTracking->compute(frame);
    }

    return QVideoFrame(*videoFrame);
}
