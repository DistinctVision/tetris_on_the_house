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

    std::vector<Line_f> detect(const sonar::ImageRef<uchar> & binImage);

private:
    QThreadPool * m_threadPool;
    int m_numberWorkThreads;

    float m_linePixelEps;
    float m_lineAngleEps;

    int m_houghThreshold;
    sonar::Point2i m_houghWinSize;

    float m_minLineLengthSquared;

    sonar::Image<int> m_houghImage;

    void _computeHoughImage(const sonar::ImageRef<uchar> & binImage);
    std::vector<Line_f> _findLines(const sonar::ImageRef<uchar> & binImage);
    void _separateLine(std::vector<Line_f> & out,
                       const sonar::ImageRef<uchar> & binImage,
                       Line_f line) const;
};

#endif // LINESDETECTOR_H
