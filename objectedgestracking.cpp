#include "objectedgestracking.h"
#include <cassert>
#include <map>
#include <unordered_map>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <QDebug>

#include "performancemonitor.h"
#include "pinholecamera.h"

using namespace Eigen;

ObjectEdgesTracking::ObjectEdgesTracking():
    m_controlPixelDistance(10.0f),
    m_model(ObjectModel::createCubikRurbik())
{
    m_monitor = std::make_shared<PerformanceMonitor>();
    m_R = Matrix3d::Identity();
    m_t = Vector3d(0.0, 0.0, 5.0);
}

std::shared_ptr<PinholeCamera> ObjectEdgesTracking::camera() const
{
    return m_camera;
}

void ObjectEdgesTracking::setCamera(const std::shared_ptr<PinholeCamera> & camera)
{
    m_camera = camera;
}

void ObjectEdgesTracking::compute(cv::Mat image)
{
    assert(image.channels() == 1);
    assert(m_camera);

    m_monitor->start();

    m_monitor->startTimer("Canny");
    cv::Mat edges;
    cv::Canny(image, edges, 150.0, 200.0);
    m_monitor->endTimer("Canny");

    m_monitor->startTimer("Inverting");
    cv::bitwise_not(edges, edges);
    m_monitor->endTimer("Inverting");

    m_monitor->startTimer("Distance transfrom");
    cv::Mat labels;
    cv::Mat distancesMap;
    cv::distanceTransform(edges, distancesMap, labels,
                          cv::DIST_L2, 3, cv::DIST_LABEL_PIXEL);
    m_monitor->endTimer("Distance transfrom");

    m_monitor->startTimer("Label process");
    std::unordered_map<int, cv::Point2i> index2point;
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
    m_monitor->endTimer("Label process");

    m_monitor->end();

    qDebug().noquote() << QString::fromStdString(m_monitor->report());

    cv::cvtColor(edges, edges, CV_GRAY2BGR);

    m_model.draw(edges, m_camera, m_R, m_t);

    Vectors3d controlModelPoints;
    Vectors2f controlImagePoints;
    std::tie(controlModelPoints, controlImagePoints) = _getCurrentResiduals(labels, index2point);
    _drawCurrentResiduals(edges, controlModelPoints, controlImagePoints);

    cv::Point2i center(image.cols / 2, image.rows / 2);

    cv::line(edges, center, index2point[labels.at<int>(center)], cv::Scalar(255, 0, 0), 2);

    cv::normalize(distancesMap, distancesMap, 0.0, 1.0, cv::NORM_MINMAX);
    cv::imshow("dis", distancesMap);

    cv::imshow("edges", edges);
    cv::waitKey(33);
}

std::tuple<Vectors3d, Vectors2f>
ObjectEdgesTracking::_getCurrentResiduals(const cv::Mat & labels,
                                          const std::unordered_map<int, cv::Point2i> & index2point) const
{
    Vectors3d controlModelPoints;
    Vectors2f controlViewPoints;
    std::tie(controlModelPoints, controlViewPoints) =
            m_model.getControlPoints(m_camera, m_controlPixelDistance, m_R, m_t);
    std::size_t numberPoints = controlModelPoints.size();
    for (std::size_t i = 0; i < numberPoints; ++i)
    {
        Vector2f & viewPoint = controlViewPoints[i];
        cv::Point2i viewPoint_i(static_cast<int>(viewPoint.x()),
                                static_cast<int>(viewPoint.y()));
        cv::Point2f s(viewPoint.x() - viewPoint_i.x, viewPoint.y() - viewPoint_i.y);
        cv::Point2f i_s(1.0f - s.x, 1.0f - s.y);
        cv::Point2f nP[4] = {
            index2point.at(labels.at<int>(viewPoint_i.y, viewPoint_i.x)),
            index2point.at(labels.at<int>(viewPoint_i.y, viewPoint_i.x + 1)),
            index2point.at(labels.at<int>(viewPoint_i.y + 1, viewPoint_i.x)),
            index2point.at(labels.at<int>(viewPoint_i.y + 1, viewPoint_i.x + 1)),
        };
        cv::Point2f p = nP[0] * (i_s.x * i_s.y) + nP[1] * (s.x * i_s.y) +
                        nP[2] * (i_s.x * s.y) + nP[3] * (s.x * s.y);
        viewPoint << p.x, p.y;
    }
    return std::make_tuple(controlModelPoints, controlViewPoints);
}

void ObjectEdgesTracking::_drawCurrentResiduals(const cv::Mat & image,
                                                const Vectors3d & controlModelPoints,
                                                const Vectors2f & controlImagePoints) const
{
    std::size_t numberPoints = controlModelPoints.size();
    for (std::size_t i = 0; i < numberPoints; ++i)
    {
        const Vector3d & modelPoint = controlModelPoints[i];
        const Vector2f & imagePoint = controlImagePoints[i];
        Vector2f viewPoint = m_camera->project((m_R * modelPoint + m_t).cast<float>());
        cv::line(image, cv::Point2f(viewPoint.x(), viewPoint.y()),
                        cv::Point2f(imagePoint.x(), imagePoint.y()), cv::Scalar(0, 0, 255), 1);
    }
}
