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

#include <Eigen/Eigen>

class LinesDetector
{
public:
    using Line_i = std::pair<sonar::Point2i, sonar::Point2i>;
    using Line_f = std::pair<sonar::Point2f, sonar::Point2f>;

    LinesDetector(QThreadPool * threadPool);

    int numberWorkThreads() const;
    void setNumberWorkThreads(int numberWorkThreads);

    void setLineEpsilons(float pixelEps, float angleEps);

    std::vector<Line_f> detect(const sonar::ImageRef<uchar> & image);

private:
    QThreadPool * m_threadPool;
    int m_numberWorkThreads;

    float m_linePixelEps;
    float m_lineAngleEps;

    sonar::Point2i m_binWinSize;
    int m_binAdaptiveThreshold;

    int m_houghThreshold;
    sonar::Point2i m_houghWinSize;

    sonar::Image<int> m_integralImage;
    sonar::Image<uchar> m_binImage;
    sonar::Image<int> m_houghImage;

    void _computeBinaryImage(const sonar::ImageRef<uchar> & image);
    void _computeHoughImage();
    std::vector<Line_f> _findCommonLines();
};

#endif // LINESDETECTOR_H
