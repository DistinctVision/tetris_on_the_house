#include "objectedgestracking.h"
#include <cassert>
#include <algorithm>
#include <vector>
#include <random>
#include <chrono>

#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>

#include <QDebug>

#include "performancemonitor.h"
#include "pinholecamera.h"
#include "poseoptimizer.h"

using namespace std;
using namespace std::chrono;
using namespace Eigen;

ObjectEdgesTracking::ObjectEdgesTracking():
    m_controlPixelDistance(10.0f),
    m_model(ObjectModel::createCubikRurbik())
{
    m_monitor = make_shared<PerformanceMonitor>();
    m_R = Matrix3d::Identity();
    m_t = Vector3d(0.0, 0.0, 5.0);
}

shared_ptr<PinholeCamera> ObjectEdgesTracking::camera() const
{
    return m_camera;
}

void ObjectEdgesTracking::setCamera(const shared_ptr<PinholeCamera> & camera)
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
    cv::Mat distancesMap;
    cv::distanceTransform(edges, distancesMap, cv::DIST_L2, 3);
    m_monitor->endTimer("Distance transfrom");

    m_monitor->startTimer("Tracking");
    Vectors3d controlModelPoints;
    double E = 0.0;
    for (int i = 0; i < 6; ++i)
    {
        controlModelPoints = m_model.getControlPoints(m_camera, m_controlPixelDistance, m_R, m_t);
        if (controlModelPoints.size() < 4)
        {
            E = numeric_limits<double>::max();
            break;
        }
        E = optimize_pose(m_R, m_t, distancesMap, m_camera, controlModelPoints, 20.0f, 5);
    }
    if (E > 2.5)
    {
        m_R = Matrix3d::Identity();
        m_t = Vector3d(0.0, 0.0, 5.0);
    }

    m_monitor->endTimer("Tracking");

    m_monitor->end();

    qDebug() << "Error =" << E;

    cv::cvtColor(edges, edges, CV_GRAY2BGR);
    m_model.draw(edges, m_camera, m_R, m_t);
    cv::normalize(distancesMap, distancesMap, 0.0, 1.0, cv::NORM_MINMAX);
    cv::imshow("dis", distancesMap);

    //qDebug().noquote() << QString::fromStdString(m_monitor->report());

    cv::imshow("edges", edges);
    cv::waitKey(33);
}
