#include "framehandler.h"

#include <QDebug>
#include <QThreadPool>
#include <QTime>

#include "pinholecamera.h"
#include "objectedgestracker.h"
#include "texture2grayimageconvertor.h"

#include <opencv2/imgproc.hpp>

using namespace Eigen;

FrameHandler::FrameHandler():
    m_frameSize(-1, -1),
    m_maxFrameSize(640, 480),
    m_orientation(0),
    m_flipHorizontally(false)
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

    if (!frame.empty())
    {
        ObjectEdgesTracker * objectEdgesTracking = m_parent->objectEdgesTracker();
        if (!objectEdgesTracking->camera() ||
                (objectEdgesTracking->camera()->imageSize() != Vector2i(frame.cols, frame.rows)))
        {
            //TODO set camera parameters
            objectEdgesTracking->setCamera(std::make_shared<PinholeCamera>(Vector2i(frame.cols, frame.rows),
                                                                           Vector2f(frame.cols, frame.cols) * 0.6f,
                                                                           Vector2f(frame.cols, frame.rows) * 0.5f));
        }
        objectEdgesTracking->compute(frame);
    }

    return QVideoFrame(*videoFrame);
}

void FrameHandlerRunnable::_transformImage(cv::Mat & image) const
{
    int orientation = ((m_parent->orientation() / 90) % 4) * 90;
    //orientation = (orientation + 180) % 360; // TODO why?
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
