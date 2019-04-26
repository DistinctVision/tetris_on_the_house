#include "framehandler.h"
#include "linesdetector.h"

#include <QDebug>
#include <QThreadPool>
#include <QTime>

#include <sonar/General/Image.h>
#include <sonar/General/ImageUtils.h>
#include <sonar/General/Paint.h>

#if defined(DEBUG_TOOLS_ENABLED)
#include <sonar/DebugTools/debug_tools.h>
#endif

using namespace sonar;

QVideoFilterRunnable * FrameHandler::createFilterRunnable()
{
    return new FrameHandlerRunnable();
}

FrameHandlerRunnable::FrameHandlerRunnable()
{
    m_linesDetector = std::make_shared<LinesDetector>(QThreadPool::globalInstance());
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

        Image<uchar> bw_image = image_utils::convertToGrayscale(image);
        QTime time;
        time.start();
        std::vector<LinesDetector::Line_f> lines = m_linesDetector->detect(bw_image);

        qDebug() << time.elapsed() << lines.size();

        for (const auto & line: lines)
            paint::drawLine(image, line.first, line.second, Rgba_u(0, 0, 255));

        debug::showImage("bw", image);
        //debug::waitKey(33);
    }

    return QVideoFrame(*videoFrame);
}
