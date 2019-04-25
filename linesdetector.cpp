#include "linesdetector.h"
#include <sonar/General/ImageUtils.h>

#if defined(DEBUG_TOOLS_ENABLED)
#include <sonar/DebugTools/debug_tools.h>
#endif

using namespace std;
using namespace sonar;

LinesDetector::LinesDetector(QThreadPool * threadPool):
    m_threadPool(threadPool)
{
    setNumberWorkThreads(min(QThread::idealThreadCount(), threadPool->maxThreadCount()));
    setLineEpsilons(2.0f, cast<float>((M_PI) / 180.0));
    m_binWinSize = Point2i(33, 33);
    m_binAdaptiveThreshold = 10;
}

int LinesDetector::numberWorkThreads() const
{
    return m_numberWorkThreads;
}

void LinesDetector::setNumberWorkThreads(int numberWorkThreads)
{
    assert(numberWorkThreads > 0);
    m_numberWorkThreads = numberWorkThreads;
}

void LinesDetector::setLineEpsilons(float pixelEps, float angleEps)
{
    m_linePixelEps = pixelEps;
    m_lineAngleEps = angleEps;
}

vector<LinesDetector::Line> LinesDetector::detect(const ImageRef<uchar> & image)
{
    _computeBinaryImage(image);
    _computeHoughImage();

    debug::showImage("bin", m_binImage);
    debug::showImage("hough", m_houghImage.convert<float>([](const Point2i &, const int& val) { return cast<float>(val); }));

    return vector<Line>();
}

void LinesDetector::_computeBinaryImage(const ImageRef<uchar> & image)
{
    if (m_integralImage.size() != image.size())
        m_integralImage = Image<int>(image.size());
    image_utils::computeIntegralImage<int, uchar>(m_integralImage, image);

    if (m_binImage.size() != image.size())
        m_binImage = Image<uchar>(image.size());

    int h_step = cast<int>(ceil(image.height() / cast<float>(m_numberWorkThreads)));

    Point2i winDeltaSize = (m_binWinSize - Point2i(1)) / 2;

    QSemaphore semaphore;
    for (int n_thread = 0; n_thread < m_numberWorkThreads; ++n_thread)
    {
        QtConcurrent::run(m_threadPool, [&, n_thread] () {
            int begin_y = n_thread * h_step;
            int end_y = min(begin_y + h_step, image.height());
            Point2i p, w_begin, w_end, w_delta;

            for (p.y = begin_y; p.y < end_y; ++p.y)
            {
                w_begin.y = max(p.y - winDeltaSize.y, 0);
                w_end.y = min(p.y + winDeltaSize.y, image.height() - 1);
                w_delta.y = w_end.y - w_begin.y;

                const uchar * imageStr = image.pointer(0, p.y);
                uchar * binStr = m_binImage.pointer(0, p.y);

                for (p.x = 0; p.x < image.width(); ++p.x)
                {
                    w_begin.x = max(p.x - winDeltaSize.x, 0);
                    w_end.x = min(p.x + winDeltaSize.x, image.width() - 1);
                    w_delta.x = w_end.x - w_begin.x;

                    int sum = m_integralImage(w_end) + m_integralImage(w_begin) -
                            (m_integralImage(w_begin.x, w_end.y) + m_integralImage(w_end.x, w_begin.y));
                    int value = imageStr[p.x] - sum / (w_delta.y * w_delta.x);
                    binStr[p.x] = (value > m_binAdaptiveThreshold) * 255;
                }
            }
            semaphore.release();
        });
    }
    semaphore.acquire(m_numberWorkThreads);
}

void LinesDetector::_computeHoughImage()
{
    Point2f imageCenter = cast<float>(m_binImage.size()) * 0.5f;

    Point2i houghSize(cast<int>(ceil(max(imageCenter.x, imageCenter.y) / m_linePixelEps)),
                      cast<int>(ceil(cast<float>(M_PI) / m_lineAngleEps)));
    if (m_houghImage.size() != houghSize)
        m_houghImage = Image<int>(houghSize);
    m_houghImage.fill(0);

    int h_step = cast<int>(ceil(m_houghImage.height() / cast<float>(m_numberWorkThreads)));

    QSemaphore semaphore;
    for (int n_thread = 0; n_thread < m_numberWorkThreads; ++n_thread)
    {
        QtConcurrent::run(m_threadPool, [&, n_thread] () {
            int begin_i = n_thread * h_step;
            int end_i = min(begin_i + h_step, m_houghImage.height());

            for (int i = begin_i; i < end_i; ++i)
            {
                int * hough_str = m_houghImage.pointer(0, i);
                float angle = ((i) / cast<float>(m_houghImage.height())) * cast<float>(M_PI);
                float cosA = cos(angle), sinA = sin(angle);
                m_binImage.for_each([&] (const Point2i &p, const uchar &val) {
                    if (val > 0)
                    {
                        Point2f delta = cast<float>(p) - imageCenter;
                        float r = delta.x * cosA + delta.y * sinA;
                        int j = cast<int>((fabs(r) / m_linePixelEps));
                        ++hough_str[j];
                    }
                });
            }
            semaphore.release();
        });
    }
    semaphore.acquire(m_numberWorkThreads);
}
