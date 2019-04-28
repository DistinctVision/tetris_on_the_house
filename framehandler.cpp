#include "framehandler.h"

#include <QDebug>
#include <QThreadPool>
#include <QTime>

#include <sonar/General/Image.h>
#include <sonar/General/ImageUtils.h>
#include <sonar/General/Paint.h>

#include "binaryimagegenerator.h"
#include "distancemapgenerator.h"

#if defined(DEBUG_TOOLS_ENABLED)
#include <sonar/DebugTools/debug_tools.h>
#endif

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace sonar;

QVideoFilterRunnable * FrameHandler::createFilterRunnable()
{
    return new FrameHandlerRunnable();
}

FrameHandlerRunnable::FrameHandlerRunnable()
{
    m_binImageGenerator = std::make_shared<BinaryImageGenerator>(QThreadPool::globalInstance());
    m_distanceMapGenerator = std::make_shared<DistanceMapGenerator>();
}

QVideoFrame FrameHandlerRunnable::run(QVideoFrame * videoFrame,
                                      const QVideoSurfaceFormat &surfaceFormat,
                                      QVideoFilterRunnable::RunFlags flags)
{
    Q_UNUSED(flags);

    if (surfaceFormat.handleType() == QAbstractVideoBuffer::NoHandle)
    {
        Image<Rgba_u> image(videoFrame->width(), videoFrame->height());
        if (videoFrame->map(QAbstractVideoBuffer::ReadOnly))
        {
            std::memcpy(image.data(), videoFrame->bits(), static_cast<size_t>(image.area() * 4));
            videoFrame->unmap();
        }
        else
        {
            qFatal("Coudn't read video frame");
        }

        cv::Mat cvImage = image_utils::convertToCvMat(image);

        QTime time;
        time.start();
        cv::cvtColor(cvImage, cvImage, CV_BGR2GRAY);
        cv::Mat tImage;
        cv::adaptiveThreshold(cvImage, tImage, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 51, 3.0);
        cv::Mat kernel1 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7, 7), cv::Point(3, 3));
        cv::dilate(tImage, tImage, kernel1, cv::Point(3, 3), 1);

        cv::Mat d3;
        cv::distanceTransform(tImage, d3, CV_DIST_L2, 3);
        qDebug() << "2" << time.elapsed();

        cv::normalize(d3, d3, 0, 1., cv::NORM_MINMAX);
        cv::imshow("bin", tImage);
        cv::imshow("d3", d3);

        cv::waitKey(33);
    }

    return QVideoFrame(*videoFrame);
}
