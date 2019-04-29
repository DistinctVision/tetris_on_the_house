#ifndef OBJECTEDGESTRACKING_H
#define OBJECTEDGESTRACKING_H

#include <opencv2/core.hpp>

class ObjectEdgesTracking
{
public:
    ObjectEdgesTracking();

    void compute(cv::Mat image);
};

#endif // OBJECTEDGESTRACKING_H
