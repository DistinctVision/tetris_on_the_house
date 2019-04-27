#include "linesdetector.h"
#include <sonar/General/ImageUtils.h>

#include <limits>
#include <climits>

#include <sonar/General/Paint.h>

#if defined(DEBUG_TOOLS_ENABLED)
#include <sonar/DebugTools/debug_tools.h>
#endif

using namespace std;
using namespace Eigen;
using namespace sonar;
using namespace sonar::image_utils;

LinesDetector::LinesDetector(QThreadPool * threadPool):
    m_threadPool(threadPool)
{
    setNumberWorkThreads(min(QThread::idealThreadCount(), threadPool->maxThreadCount()));
    setLineEpsilons(4.0f, cast<float>(M_PI / 60.0));
    m_binWinSize = Point2i(31, 31);
    m_binAdaptiveThreshold = 10;
    m_houghThreshold = 50;
    m_houghWinSize = Point2i(31, 31);
    m_minLineLengthSquared = 15.0f * 15.0f;
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
    _computeBinaryImage(halfSample<Sampler_avg<uchar>>(image));
    computeIntegralImage<int, uchar>(m_integralImage, m_binImage);
    erode(m_binImage, m_integralImage, 8, 0.7f);

    m_binImage.for_each([] (const Point2i &p, uchar & val) {
        if (val > 0)
            val = 255;
    });
    _computeHoughImage();

    debug::showImage("bin", m_binImage);

    return _findLines();
}

void LinesDetector::_computeBinaryImage(const ImageRef<uchar> & image)
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
                        if ((j < 0) || (j >= m_houghImage.width()))
                            return;
                        ++hough_str[j];
                    }
                });
            }
            semaphore.release();
        });
    }
    semaphore.acquire(m_numberWorkThreads);
}

vector<LinesDetector::Line_f> LinesDetector::_findLines()
{
    Point2f imageCenter = cast<float>(m_binImage.size()) * 0.5f;

    float lineRadius = max(imageCenter.x, imageCenter.y);

    int h_step = cast<int>(ceil(m_houghImage.height() / cast<float>(m_numberWorkThreads)));

    Point2i winDeltaSize = (m_houghWinSize - Point2i(1)) / 2;

    vector<vector<Line_f>> w_common_lines(cast<size_t>(m_numberWorkThreads));

    Vector3f h_borders[2] = {
        Vector3f(0.0f, 1.0f, 0.0f),
        Vector3f(0.0f, 1.0f, - cast<float>(m_binImage.height())),
    };

    Vector3f v_borders[2] = {
        Vector3f(1.0f, 0.0f, 0.0f),
        Vector3f(1.0f, 0.0f, - cast<float>(m_binImage.width())),
    };

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
                w_begin.y = (p.y - winDeltaSize.y);
                w_end.y = (p.y + winDeltaSize.y);
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
                        int * w_hough_str = m_houghImage.pointer(0, (w_p.y < 0) ? (m_houghImage.height() + w_p.y) :
                                                                                  (w_p.y % m_houghImage.height()));
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
                        float angle = ((p.y) / cast<float>(m_houghImage.height())) * cast<float>(M_PI);
                        Point2f dir(cos(angle), sin(angle));
                        float d = (lineRadius - (p.x + 0.5f) * m_linePixelEps) - dir.dot(imageCenter);
                        part_workLines.emplace_back(dir, d);
                    }
                }
            }
            vector<Line_f> & part_lines = w_common_lines[cast<size_t>(n_thread)];
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
                    else if (l.first.x > cast<float>(m_binImage.width() - 1))
                        l.first.set(v_p2.x() / v_p2.z(), v_p2.y() / v_p2.z());

                    l.second.set(h_p2.x() / h_p2.z(), h_p2.y() / h_p2.z());
                    if (l.second.x < 0.0f)
                        l.second.set(v_p1.x() / v_p1.z(), v_p1.y() / v_p1.z());
                    else if (l.second.x > cast<float>(m_binImage.width() - 1))
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

    size_t nCommonLines = 0;
    for (auto itV = w_common_lines.begin(); itV != w_common_lines.end(); ++itV)
        nCommonLines += itV->size();
    vector<Line_f> commonLines;
    commonLines.reserve(nCommonLines);
    for (auto itV = w_common_lines.begin(); itV != w_common_lines.end(); ++itV)
        commonLines.insert(commonLines.end(), itV->begin(), itV->end());

    vector<Line_f> result;
    {
        vector<vector<Line_f>> w_sep_lines(cast<size_t>(m_numberWorkThreads));
        size_t workPartSize = cast<size_t>(ceil(commonLines.size() / cast<float>(m_numberWorkThreads)));
        for (int n_thread = 0; n_thread < m_numberWorkThreads; ++n_thread)
        {
            QtConcurrent::run(m_threadPool, [&, n_thread] () {
                vector<Line_f> & sep_lines = w_sep_lines[cast<size_t>(n_thread)];
                size_t begin_i = cast<size_t>(n_thread) * workPartSize;
                size_t end_i = min(begin_i + workPartSize, commonLines.size());
                for (size_t i = begin_i; i < end_i; ++i)
                    _separateLine(sep_lines, commonLines[i]);
                semaphore.release();
            });
        }
        semaphore.acquire(m_numberWorkThreads);
        for (auto itV = w_sep_lines.begin(); itV != w_sep_lines.end(); ++itV)
            result.insert(result.end(), itV->begin(), itV->end());
    }

    return result;
}

void LinesDetector::_separateLine(vector<LinesDetector::Line_f> &out, LinesDetector::Line_f line) const
{
    const int linePixelEps_i = cast<int>(ceil(m_linePixelEps));
    Point2f d = line.second - line.first;
    if (fabs(d.y) > fabs(d.x))
    {
        if (line.first.y > line.second.y)
        {
            swap(line.first, line.second);
            d = - d;
        }
        float k = d.x / d.y;
        Line_i l_i(cast<int>(line.first), cast<int>(line.second));
        const uchar * str = m_binImage.pointer(0, l_i.first.y);
        Line_f cur_line;
        cur_line.first.x = -1.0f;
        Point2f sum(0.0f);
        int n_sum = 0;
        for (int y_ = l_i.first.y; y_ <= l_i.second.y; ++y_)
        {
            float x = k * (y_ - line.first.y) + line.first.x;
            int begin_x = max(cast<int>(x - m_linePixelEps * 0.5f), 0);
            int end_x = min(cast<int>(x + m_linePixelEps * 0.5f), m_binImage.width() - 1);
            for (int x_ = begin_x; x_ < end_x; ++x_)
            {
                if (str[x_] > 0)
                {
                    cur_line.second.set(x, cast<float>(y_));
                    if (n_sum == 0)
                        cur_line.first = cur_line.second;
                    sum.x += cast<float>(x_);
                    sum.y += cast<float>(y_);
                    ++n_sum;
                }
            }
            if (n_sum > 0)
            {
                if (abs(y_ - cast<int>(cur_line.second.y)) > linePixelEps_i)
                {
                    Point2f line_d = cur_line.second - cur_line.first;
                    float line_lS = line_d.lengthSquared();
                    if (line_lS > m_minLineLengthSquared)
                    {
                        Point2f n = Point2f(line_d.y, - line_d.x) / sqrt(line_lS);
                        Point2f d = n * n.dot(sum / cast<float>(n_sum) - (cur_line.second + cur_line.first) * 0.5f);
                        cur_line.first += d;
                        cur_line.second += d;
                        out.push_back(cur_line);
                    }
                    n_sum = 0;
                    sum.setZero();
                }
            }
            str += m_binImage.widthStep();
        }
        if (n_sum > 0)
        {
            Point2f line_d = cur_line.second - cur_line.first;
            float line_lS = line_d.lengthSquared();
            if (line_lS > m_minLineLengthSquared)
            {
                Point2f n = Point2f(line_d.y, - line_d.x) / sqrt(line_lS);
                Point2f d = n * n.dot(sum / cast<float>(n_sum) - (cur_line.second + cur_line.first) * 0.5f);
                cur_line.first += d;
                cur_line.second += d;
                out.push_back(cur_line);
            }
        }
    }
    else
    {
        if (line.first.x > line.second.x)
        {
            swap(line.first, line.second);
            d = - d;
        }
        float k = d.y / d.x;
        Line_i l_i(cast<int>(line.first), cast<int>(line.second));
        const uchar * column_data = m_binImage.pointer(l_i.first.x, 0);
        Line_f cur_line;
        cur_line.first.x = -1.0f;
        Point2f sum(0.0f);
        int n_sum = 0;
        for (int x_ = l_i.first.x; x_ <= l_i.second.x; ++x_)
        {
            float y = k * (x_ - line.first.x) + line.first.y;
            int begin_y = max(cast<int>(y - m_linePixelEps * 0.5f), 0);
            int end_y = min(cast<int>(y + m_linePixelEps * 0.5f), m_binImage.height() - 1);
            for (int y_ = begin_y; y_ < end_y; ++y_)
            {
                if (column_data[y_ * m_binImage.widthStep()] > 0)
                {
                    cur_line.second.set(cast<float>(x_), y);
                    if (n_sum == 0)
                        cur_line.first = cur_line.second;
                    sum.x += cast<float>(x_);
                    sum.y += cast<float>(y_);
                    ++n_sum;
                }
            }
            if (n_sum > 0)
            {
                if (abs(x_ - cast<int>(cur_line.second.x)) > linePixelEps_i)
                {
                    Point2f line_d = cur_line.second - cur_line.first;
                    float line_lS = line_d.lengthSquared();
                    if (line_lS > m_minLineLengthSquared)
                    {
                        Point2f n = Point2f(line_d.y, - line_d.x) / sqrt(line_lS);
                        Point2f d = n * n.dot(sum / cast<float>(n_sum) - (cur_line.second + cur_line.first) * 0.5f);
                        cur_line.first += d;
                        cur_line.second += d;
                        out.push_back(cur_line);
                    }
                    n_sum = 0;
                    sum.setZero();
                }
            }
            ++column_data;
        }
        if (n_sum > 0)
        {
            Point2f line_d = cur_line.second - cur_line.first;
            float line_lS = line_d.lengthSquared();
            if (line_lS > m_minLineLengthSquared)
            {
                Point2f n = Point2f(line_d.y, - line_d.x) / sqrt(line_lS);
                Point2f d = n * n.dot(sum / cast<float>(n_sum) - (cur_line.second + cur_line.first) * 0.5f);
                cur_line.first += d;
                cur_line.second += d;
                out.push_back(cur_line);
            }
        }
    }
}
