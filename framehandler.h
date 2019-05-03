#ifndef FRAMEHANDLER_H
#define FRAMEHANDLER_H

#include <memory>

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

private:
    std::shared_ptr<ObjectEdgesTracker> m_objectEdgesTracker;
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
