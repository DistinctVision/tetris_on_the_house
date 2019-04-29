#ifndef OBJECTEDGESTRACKING_H
#define OBJECTEDGESTRACKING_H

#include <opencv2/core.hpp>

#include "objectmodel.h"

class ObjectEdgesTracking
{
public:
    ObjectEdgesTracking();

    void compute(cv::Mat image);

private:
    ObjectModel m_model;
};

#endif // OBJECTEDGESTRACKING_H
