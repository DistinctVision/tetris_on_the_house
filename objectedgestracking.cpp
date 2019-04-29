#include "objectedgestracking.h"
#include <cassert>
#include <map>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <QTime>
#include <QDebug>

ObjectEdgesTracking::ObjectEdgesTracking():
    m_model(ObjectModel::createBox())
{

}

void ObjectEdgesTracking::compute(cv::Mat image)
{
    assert(image.channels() == 1);

    QTime time;
    time.start();
    cv::Mat edges;
    cv::Canny(image, edges, 100.0, 200.0);
    qDebug() << "canny =" << time.elapsed();

    time.start();
    cv::bitwise_not(edges, edges);
    qDebug() << "invert =" << time.elapsed();

    time.start();
    cv::Mat labels;
    cv::Mat distancesMap;
    cv::distanceTransform(edges, distancesMap, labels,
                          cv::DIST_L2, 3, cv::DIST_LABEL_PIXEL);
    qDebug() << "distance transform=" << time.elapsed();

    time.start();
    std::map<int, cv::Point2i> index2point;
    cv::Point2i p;
    for (p.y = 0; p.y < edges.rows; ++p.y)
    {
        uchar * str_edges = edges.ptr<uchar>(p.y);
        int * str_labels = labels.ptr<int>(p.y);
        for (p.x = 0; p.x < edges.cols; ++p.x)
        {
            if (str_edges[p.x] == 0)
                index2point[str_labels[p.x]] = p;
        }
    }
    qDebug() << "labels =" << time.elapsed();

    cv::Point2i center(image.cols / 2, image.rows / 2);

    cv::cvtColor(edges, edges, CV_GRAY2BGR);

    cv::line(edges, center, index2point[labels.at<int>(center)], cv::Scalar(255, 0, 0), 2);

    cv::normalize(distancesMap, distancesMap, 0.0, 1.0, cv::NORM_MINMAX);
    cv::imshow("dis", distancesMap);

    cv::imshow("edges", edges);
    cv::waitKey(33);
}
