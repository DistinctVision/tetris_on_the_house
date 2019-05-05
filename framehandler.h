#ifndef FRAMEHANDLER_H
#define FRAMEHANDLER_H

#include <QSize>
#include <QSharedPointer>
#include <QVideoFrame>
#include <QVideoSurfaceFormat>
#include <QAbstractVideoFilter>
#include <QOpenGLFunctions>

class ObjectEdgesTracker;
class Texture2GrayImageConvertor;

class FrameHandlerRunnable;

class FrameHandler: public QAbstractVideoFilter
{
    Q_OBJECT

    Q_PROPERTY(ObjectEdgesTracker* objectEdgesTracker READ objectEdgesTracker CONSTANT)
    Q_PROPERTY(QSize frameSize READ frameSize NOTIFY frameSizeChanged)
    Q_PROPERTY(QSize maxFrameSize READ maxFrameSize NOTIFY maxFrameSizeChanged)

public:
    FrameHandler();

    QVideoFilterRunnable * createFilterRunnable() override;

    ObjectEdgesTracker * objectEdgesTracker() const;

    QSize frameSize() const;

    QSize maxFrameSize() const;
    void setMaxFrameSize(const QSize & maxFrameSize);

signals:
    void frameSizeChanged();
    void maxFrameSizeChanged();

private:
    friend class FrameHandlerRunnable;

    QSharedPointer<ObjectEdgesTracker> m_objectEdgesTracker;
    QSize m_frameSize;
    QSize m_maxFrameSize;

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
};

#endif // FRAMEHANDLER_H
