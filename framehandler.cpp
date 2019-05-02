#include "framehandler.h"

#include <QDebug>
#include <QThreadPool>
#include <QTime>

#include "pinholecamera.h"
#include "objectedgestracking.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace Eigen;

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
        Vector2i imageSize(videoFrame->width(), videoFrame->height());
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
        //imageSize /= 2;

        cv::flip(frame, frame, -1);
        cv::flip(frame, frame, 1);
        cv::cvtColor(frame, frame, cv::COLOR_BGRA2GRAY);
        cv::resize(frame, frame, cv::Size(imageSize.x(), imageSize.y()));
        if (!m_objectEdgesTracking->camera() ||
                (m_objectEdgesTracking->camera()->imageSize() != imageSize))
        {
            m_objectEdgesTracking->setCamera(std::make_shared<PinholeCamera>(imageSize,
                                                                             Vector2f(imageSize.x(), imageSize.x()) * 1.2f,
                                                                             imageSize.cast<float>() * 0.5));
        }
        m_objectEdgesTracking->compute(frame);
    }

    return QVideoFrame(*videoFrame);
}
