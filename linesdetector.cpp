#include "linesdetector.h"
#include <sonar/General/ImageUtils.h>

#include <limits>
#include <climits>

#include <sonar/General/Paint.h>

#if defined(DEBUG_TOOLS_ENABLED)
#include <sonar/DebugTools/debug_tools.h>
#endif

using namespace std;
using namespace sonar;
using namespace Eigen;

LinesDetector::LinesDetector(QThreadPool * threadPool):
    m_threadPool(threadPool)
{
    setNumberWorkThreads(min(QThread::idealThreadCount(), threadPool->maxThreadCount()));
    setLineEpsilons(4.0f, cast<float>(M_PI / 135.0));
    m_binWinSize = Point2i(15, 15);
    m_binAdaptiveThreshold = 10;
    m_houghThreshold = 200;
    m_houghWinSize = Point2i(51, 21);
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

vector<LinesDetector::Line_f> LinesDetector::detect(const ImageRef<uchar> & image)
{
    _computeBinaryImage(image);
    _computeHoughImage();

    debug::showImage("bin", m_binImage);
    debug::showImage("hough", m_houghImage.convert<float>([](const Point2i &, const int& val) { return cast<float>(val); }));

    return _findCommonLines();
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

    float lineRadius = max(imageCenter.x, imageCenter.y);

    Point2i houghSize(cast<int>(ceil(max(m_binImage.width(), m_binImage.height()) / m_linePixelEps)),
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
                        int j = cast<int>(((r + lineRadius) / m_linePixelEps));
                        ++hough_str[min(max(j, 0), m_houghImage.width() - 1)];
                    }
                });
            }
            semaphore.release();
        });
    }
    semaphore.acquire(m_numberWorkThreads);
}

vector<LinesDetector::Line_f> LinesDetector::_findCommonLines()
{
    Point2f imageCenter = cast<float>(m_binImage.size()) * 0.5f;

    float lineRadius = max(imageCenter.x, imageCenter.y);

    int h_step = cast<int>(ceil(m_houghImage.height() / cast<float>(m_numberWorkThreads)));

    Point2i winDeltaSize = (m_houghWinSize - Point2i(1)) / 2;

    vector<vector<Line_f>> lines(cast<size_t>(m_numberWorkThreads));

    Vector3f h_borders[2] = {
        Vector3f(0.0f, 1.0f, 0.0f),
        Vector3f(0.0f, 1.0f, - cast<float>(m_binImage.height())),
    };

    Vector3f v_borders[2] = {
        Vector3f(1.0f, 0.0f, 0.0f),
        Vector3f(1.0f, 0.0f, - cast<float>(m_binImage.width())),
    };

    Image<uchar> d_hough(m_houghImage.size());
    d_hough.fill(0);

    QSemaphore semaphore;
    for (int n_thread = 0; n_thread < m_numberWorkThreads; ++n_thread)
    {
        QtConcurrent::run(m_threadPool, [&, n_thread] () {
            vector<pair<Point2f, float>> part_workLines;

            int begin_i = n_thread * h_step;
            int end_i = min(begin_i + h_step, m_houghImage.height());

            Point2i p, w_begin, w_end, w_p;
            Point2i max_val_p;
            int max_val;
            for (p.y = begin_i; p.y < end_i; ++p.y)
            {
                w_begin.y = max(p.y - winDeltaSize.y, 0);
                w_end.y = min(p.y + winDeltaSize.y, m_houghImage.height() - 1);
                int * hough_str = m_houghImage.pointer(0, p.y);
                for (p.x = 0; p.x < m_houghImage.width(); ++p.x)
                {
                    if (hough_str[p.x] <= m_houghThreshold)
                        continue;
                    w_begin.x = max(p.x - winDeltaSize.x, 0);
                    w_end.x = min(p.x + winDeltaSize.x, m_houghImage.width() - 1);
                    max_val_p = p;
                    max_val = hough_str[p.x];
                    for (w_p.y = w_begin.y; w_p.y < w_end.y; ++w_p.y)
                    {
                        int * w_hough_str = m_houghImage.pointer(0, w_p.y);
                        for (w_p.x = w_begin.x; w_p.x < w_end.x; ++w_p.x)
                        {
                            if (w_hough_str[w_p.x] < max_val)
                                continue;
                            max_val = w_hough_str[w_p.x];
                            max_val_p = w_p;
                        }
                    }
                    if (max_val_p == p)
                    {
                        paint::drawCross<uchar>(d_hough, p, 10.0f, 255);

                        float angle = ((p.y) / cast<float>(m_houghImage.height())) * cast<float>(M_PI);
                        Point2f dir(cos(angle), sin(angle));
                        float d = (lineRadius - (p.x + 0.5f) * m_linePixelEps) - dir.dot(imageCenter);
                        part_workLines.emplace_back(dir, d);
                    }
                }
            }
            vector<Line_f> & part_lines = lines[cast<size_t>(n_thread)];
            part_lines.reserve(part_workLines.size());

            for (auto itL = part_workLines.cbegin(); itL != part_workLines.cend(); ++itL)
            {
                Vector3f h_l(itL->first.x, itL->first.y, itL->second);
                Vector3f h_p1 = h_l.cross(h_borders[0]);
                Vector3f h_p2 = h_l.cross(h_borders[1]);
                Vector3f v_p1 = h_l.cross(v_borders[0]);
                Vector3f v_p2 = h_l.cross(v_borders[1]);

                Line_f l;
                if (fabs(h_p1.z()) > numeric_limits<float>::epsilon())
                {
                    l.first.set(h_p1.x() / h_p1.z(), h_p1.y() / h_p1.z());
                    if (l.first.x < 0.0f)
                        l.first.set(v_p1.x() / v_p1.z(), v_p1.y() / v_p1.z());
                    else if (l.first.x > cast<float>(m_binImage.height() - 1))
                        l.first.set(v_p2.x() / v_p2.z(), v_p2.y() / v_p2.z());

                    l.second.set(h_p2.x() / h_p2.z(), h_p2.y() / h_p2.z());
                    if (l.second.x < 0.0f)
                        l.second.set(v_p1.x() / v_p1.z(), v_p1.y() / v_p1.z());
                    else if (l.second.x > cast<float>(m_binImage.height() - 1))
                        l.second.set(v_p2.x() / v_p2.z(), v_p2.y() / v_p2.z());
                }
                else
                {
                    l.first.set(v_p1.x() / v_p1.z(), v_p1.y() / v_p1.z());
                    l.second.set(v_p2.x() / v_p2.z(), v_p2.y() / v_p2.z());
                }
                part_lines.push_back(l);
            }
            semaphore.release();
        });
    }
    semaphore.acquire(m_numberWorkThreads);

    debug::showImage("d_hough", d_hough);

    vector<Line_f> result;
    for (auto it = lines.begin(); it != lines.end(); ++it)
        result.insert(result.end(), it->begin(), it->end());

    return result;
}
