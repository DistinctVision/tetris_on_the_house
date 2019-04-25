#ifndef LINESDETECTOR_H
#define LINESDETECTOR_H

#include <vector>
#include <utility>
#include <cmath>
#include <qmath.h>

#include <QThreadPool>
#include <QSemaphore>
#include <QtConcurrent/QtConcurrent>

#include <sonar/General/cast.h>
#include <sonar/General/Point2.h>
#include <sonar/General/Image.h>

class LinesDetector
{
public:
    using Line = std::pair<sonar::Point2i, sonar::Point2i>;

    LinesDetector(QThreadPool * threadPool);

    int numberWorkThreads() const;
    void setNumberWorkThreads(int numberWorkThreads);

    void setLineEpsilons(float pixelEps, float angleEps);

    std::vector<Line> detect(const sonar::ImageRef<uchar> & image);

private:
    QThreadPool * m_threadPool;
    int m_numberWorkThreads;

    float m_linePixelEps;
    float m_lineAngleEps;

    sonar::Point2i m_binWinSize;
    int m_binAdaptiveThreshold;

    sonar::Image<int> m_integralImage;
    sonar::Image<uchar> m_binImage;
    sonar::Image<int> m_houghImage;

    void _computeBinaryImage(const sonar::ImageRef<uchar> & image);
    void _computeHoughImage();
};

#endif // LINESDETECTOR_H
