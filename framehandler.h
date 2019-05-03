#ifndef FRAMEHANDLER_H
#define FRAMEHANDLER_H

#include <QSize>
#include <QSharedPointer>
#include <QVideoFrame>
#include <QVideoSurfaceFormat>
#include <QAbstractVideoFilter>

class ObjectEdgesTracker;

class FrameHandlerRunnable;

class FrameHandler: public QAbstractVideoFilter
{
    Q_OBJECT

    Q_PROPERTY(QObject* READ objectEdgesTracker CONSTANT)
public:
    FrameHandler();

    QVideoFilterRunnable * createFilterRunnable() override;

    ObjectEdgesTracker * objectEdgesTracker() const;

    QSize frameSize() const;

signals:
    void frameSizeChanged();

private:
    friend class FrameHandlerRunnable;

    QSharedPointer<ObjectEdgesTracker> m_objectEdgesTracker;
    QSize m_frameSize;

    void _setFrameSize(const QSize & frameSize);
};

class FrameHandlerRunnable: public QVideoFilterRunnable
{
public:
    FrameHandlerRunnable(FrameHandler * parent);

    QVideoFrame run(QVideoFrame * videoFrame,
                    const QVideoSurfaceFormat &surfaceFormat,
                    RunFlags flags) override;

private:
    FrameHandler * m_parent;
};

#endif // FRAMEHANDLER_H
