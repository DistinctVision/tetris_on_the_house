#include "framehandler.h"

#include <QDebug>
#include <QThreadPool>
#include <QTime>

#include "performancemonitor.h"
#include "pinholecamera.h"
#include "objectedgestracker.h"
#include "texture2grayimageconvertor.h"
#include "gl/gl_view.h"
#include "texturereceiver.h"

#include <opencv2/imgproc.hpp>

using namespace Eigen;

FrameHandler::FrameHandler():
    m_frameSize(-1, -1),
    m_maxFrameSize(640, 480),
    m_orientation(0),
    m_flipHorizontally(false),
    m_focalLength(1.0f, 1.0f),
    m_opticalCenter(0.5f, 0.5f),
    m_gl_view(nullptr),
    m_textureReceiver(nullptr)
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

GL_View * FrameHandler::gl_view() const
{
    return m_gl_view;
}

void FrameHandler::setGl_view(GL_View * gl_view)
{
    if (m_gl_view == gl_view)
        return;
    m_gl_view = gl_view;
    emit gl_viewChanged();
}

TextureReceiver * FrameHandler::textureReceiver() const
{
    return m_textureReceiver;
}

void FrameHandler::setTextureReceiver(TextureReceiver * textureReceiver)
{
    if (m_textureReceiver == textureReceiver)
        return;
    m_textureReceiver = textureReceiver;
    emit textureReceiverChanged();
}

void FrameHandler::_setFrameSize(const QSize & frameSize)
{
    if (frameSize == m_frameSize)
        return;
    m_frameSize = frameSize;
    emit frameSizeChanged();
}

FrameHandlerRunnable::FrameHandlerRunnable(FrameHandler * parent):
    m_parent(parent),
    m_frameTextureId(0)
{
    initializeOpenGLFunctions();
}

FrameHandlerRunnable::~FrameHandlerRunnable()
{
    if (m_frameTextureId > 0)
    {
        glDeleteTextures(1, &m_frameTextureId);
    }
}

QVideoFrame FrameHandlerRunnable::run(QVideoFrame * videoFrame,
                                      const QVideoSurfaceFormat & surfaceFormat,
                                      QVideoFilterRunnable::RunFlags flags)
{
    Q_UNUSED(flags);

    cv::Mat frame;
    QVector2D viewScale(1.0f, 1.0f);

    QSharedPointer<PerformanceMonitor> monitor = m_parent->monitor();
    TextureReceiver * textureReceiver = m_parent->textureReceiver();

    monitor->start();
    monitor->startTimer("Getting frame");
    GLuint textureId = 0;
    QSize frameSize;
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
        _transformImage(frame);
        if (textureReceiver != nullptr)
        {
            if (m_frameTextureId == 0)
                glGenTextures(1, &m_frameTextureId);
            glBindTexture(GL_TEXTURE_2D, m_frameTextureId);
#if defined(__ANDROID__)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame.cols, frame.rows,
                         0, GL_RGBA, GL_UNSIGNED_BYTE, frame.data);
#else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA, frame.cols, frame.rows,
                         0, GL_BGRA, GL_UNSIGNED_BYTE, frame.data);
#endif
            frameSize = QSize(frame.cols, frame.rows);
            glGenerateMipmap(GL_TEXTURE_2D);
            textureReceiver->setTextureId(m_frameTextureId, frameSize, 0);
        }
        cv::cvtColor(frame, frame, cv::COLOR_BGRA2GRAY);
        QSize maxSize = m_parent->maxFrameSize();
        double scale1 = maxSize.width() / static_cast<double>(frame.cols);
        double scale2 = maxSize.height() / static_cast<double>(frame.rows);
        double scale = std::min(scale1, scale2);
        if (scale < 1.0)
            cv::resize(frame, frame, cv::Size(), scale, scale);
        textureId = m_frameTextureId;
    }
    else if (surfaceFormat.handleType() == QAbstractVideoBuffer::GLTextureHandle)
    {
        textureId = videoFrame->handle().toUInt();
        frameSize = videoFrame->size();
        int orientation = m_parent->orientation();
        bool flipHorizontally = m_parent->flipHorizontally();
        //orientation = 180;
        //flipHorizontally = true;
        if (textureReceiver != nullptr)
        {
            textureReceiver->setTextureId(textureId, frameSize, orientation);
        }

        if (!m_texture2GrayImageConverter)
            m_texture2GrayImageConverter = QSharedPointer<Texture2GrayImageConvertor>::create();

        GL_View * gl_view = m_parent->gl_view();
        QSize viewportSize = ((gl_view != nullptr) &&
                                (gl_view->fillFrameMode() == FillMode::PreserveAspectCrop)) ?
                              gl_view->viewportSize() : QSize(-1, -1);

        if ((viewportSize.width() > 0) && (viewportSize.height() > 0))
        {
            std::tie(frame, viewScale) = m_texture2GrayImageConverter->read_cropped(this,
                                                               textureId,
                                                               frameSize, m_parent->maxFrameSize(),
                                                               viewportSize.width() / static_cast<float>(viewportSize.height()),
                                                               orientation,
                                                               flipHorizontally);
        }
        else
        {
            frame = m_texture2GrayImageConverter->read(this,
                                                       textureId,
                                                       frameSize, m_parent->maxFrameSize(),
                                                       orientation,
                                                       flipHorizontally);
        }
    }
    monitor->endTimer("Getting frame");

    if (!frame.empty())
    {
        viewScale = QVector2D(std::fabs(viewScale.x()), std::fabs(viewScale.y()));

        QVector2D focalLength = m_parent->focalLength();
        QVector2D opticalCenter = m_parent->opticalCenter();
        Vector2i v_imageSize(frame.cols, frame.rows);
        Vector2f v_focalLength(frame.cols * focalLength.x() * viewScale.x(),
                               - frame.cols * focalLength.y() * viewScale.x());
        Vector2f v_opticalCenter(frame.cols * ((opticalCenter.x() - 0.5f) * viewScale.x() + 0.5f),
                                 frame.rows * ((opticalCenter.y() - 0.5f) * viewScale.y() + 0.5f));
        ObjectEdgesTracker * objectEdgesTracking = m_parent->objectEdgesTracker();
        std::shared_ptr<const PinholeCamera> prevCamera = objectEdgesTracking->camera();
        if (!prevCamera || (prevCamera->imageSize() != v_imageSize) ||
                ((v_focalLength - prevCamera->pixelFocalLength()).array().sum() > 1e-4f) ||
                ((v_opticalCenter - prevCamera->pixelOpticalCenter()).array().sum() > 1e-4f))
        {
            objectEdgesTracking->setCamera(std::make_shared<PinholeCamera>(v_imageSize,
                                                                           v_focalLength,
                                                                           v_opticalCenter));
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
