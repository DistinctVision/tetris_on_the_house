#ifndef FRAMEHANDLER_H
#define FRAMEHANDLER_H

#include <memory>

#include <QVideoFrame>
#include <QVideoSurfaceFormat>
#include <QAbstractVideoFilter>

class BinaryImageGenerator;
class DistanceMapGenerator;

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
    std::shared_ptr<BinaryImageGenerator> m_binImageGenerator;
    std::shared_ptr<DistanceMapGenerator> m_distanceMapGenerator;
};

#endif // FRAMEHANDLER_H
