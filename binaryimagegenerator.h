#ifndef BINARYIMAGEGENERATOR_H
#define BINARYIMAGEGENERATOR_H

#include <QThreadPool>

#include "sonar/General/Image.h"

class BinaryImageGenerator
{
public:
    BinaryImageGenerator(QThreadPool * threadPool);

    int numberWorkThreads() const;
    void setNumberWorkThreads(int numberWorkThreads);

    sonar::ConstImage<uchar> compute(const sonar::Image<uchar> & image);

private:
    QThreadPool * m_threadPool;
    int m_numberWorkThreads;

    sonar::Image<int> m_integralImage;
    sonar::Image<uchar> m_binImage;

    sonar::Point2i m_binWinSize;
    int m_binAdaptiveThreshold;
};

#endif // BINARYIMAGEGENERATOR_H
