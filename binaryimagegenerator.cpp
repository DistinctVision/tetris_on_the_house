#include "binaryimagegenerator.h"

#include <QSemaphore>
#include <QtConcurrent/QtConcurrent>

#include <sonar/General/ImageUtils.h>

using namespace std;
using namespace sonar;
using namespace sonar::image_utils;

BinaryImageGenerator::BinaryImageGenerator(QThreadPool * threadPool):
    m_threadPool(threadPool)
{
    setNumberWorkThreads(min(QThread::idealThreadCount(), threadPool->maxThreadCount()));
    m_binWinSize = Point2i(31, 31);
    m_binAdaptiveThreshold = 10;
}

int BinaryImageGenerator::numberWorkThreads() const
{
    return m_numberWorkThreads;
}

void BinaryImageGenerator::setNumberWorkThreads(int numberWorkThreads)
{
    assert(numberWorkThreads > 0);
    m_numberWorkThreads = numberWorkThreads;
}

ConstImage<uchar> BinaryImageGenerator::compute(const Image<uchar> & image)
{
    if (m_integralImage.size() != image.size())
        m_integralImage = Image<int>(image.size());
    computeIntegralImage<int, uchar>(m_integralImage, image);

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
                    binStr[p.x] = (value > m_binAdaptiveThreshold);
                }
            }
            semaphore.release();
        });
    }
    semaphore.acquire(m_numberWorkThreads);
    return m_binImage;
}
