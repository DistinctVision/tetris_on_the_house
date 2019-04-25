#ifndef FRAMEHANDLER_H
#define FRAMEHANDLER_H

#include <memory>

#include <QVideoFrame>
#include <QVideoSurfaceFormat>
#include <QAbstractVideoFilter>

class LinesDetector;

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
    std::shared_ptr<LinesDetector> m_linesDetector;
};

#endif // FRAMEHANDLER_H
