#ifndef FRAMEHANDLER_H
#define FRAMEHANDLER_H

#include <memory>

#include <QVideoFrame>
#include <QVideoSurfaceFormat>
#include <QAbstractVideoFilter>

class ObjectEdgesTracking;

class FrameHandlerRunnable;

class FrameHandler: public QAbstractVideoFilter
{
public:
    QVideoFilterRunnable * createFilterRunnable() override;
};

class FrameHandlerRunnable: public QVideoFilterRunnable
{
public:
    FrameHandlerRunnable();

    QVideoFrame run(QVideoFrame * videoFrame,
                    const QVideoSurfaceFormat &surfaceFormat,
                    RunFlags flags) override;

private:
    std::shared_ptr<ObjectEdgesTracking> m_objectEdgesTracking;
};

#endif // FRAMEHANDLER_H
