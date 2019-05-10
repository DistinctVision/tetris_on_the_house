#include "objectedgestracker.h"
#include <cassert>
#include <algorithm>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <qmath.h>
#include <climits>
#include <limits>

#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>

#include <QDebug>

#include "performancemonitor.h"
#include "pinholecamera.h"
#include "poseoptimizer.h"
#include "poseoptimizer2.h"

using namespace std;
using namespace std::chrono;
using namespace Eigen;

ObjectEdgesTracker::ObjectEdgesTracker(const QSharedPointer<PerformanceMonitor> & monitor):
    m_monitor(monitor),
    m_controlPixelDistance(10.0f),
    m_cannyThresholdA(50.0),
    m_cannyThresholdB(100.0),
    m_model(ObjectModel::createHouse(Vector3f(10.0f, 18.0f, 10.0f)))
{
    m_R = Matrix3f::Identity();
    m_t = Vector3f(0.0f, 8.0f, 40.0f);
}

float ObjectEdgesTracker::controlPixelDistance() const
{
    return m_controlPixelDistance;
}

void ObjectEdgesTracker::setControlPixelDistance(float controlPixelDistance)
{
    if (m_controlPixelDistance == controlPixelDistance)
        return;
    m_controlPixelDistance = controlPixelDistance;
    emit controlPixelDistanceChanged();
}

double ObjectEdgesTracker::cannyThresholdA() const
{
    return m_cannyThresholdA;
}

void ObjectEdgesTracker::setCannyThresholdA(double cannyThresholdA)
{
    if (m_cannyThresholdA == cannyThresholdA)
        return;
    m_cannyThresholdA = cannyThresholdA;
    emit cannyThresholdAChanged();
}

double ObjectEdgesTracker::cannyThresholdB() const
{
    return m_cannyThresholdB;
}

void ObjectEdgesTracker::setCannyThresholdB(double cannyThresholdB)
{
    if (m_cannyThresholdB == cannyThresholdB)
        return;
    m_cannyThresholdB = cannyThresholdB;
    emit cannyThresholdBChanged();
}

shared_ptr<PinholeCamera> ObjectEdgesTracker::camera() const
{
    return m_camera;
}

void ObjectEdgesTracker::setCamera(const shared_ptr<PinholeCamera> & camera)
{
    if (m_camera == camera)
        return;
    m_camera = camera;
    emit cameraChanged();
}

QVector2D ObjectEdgesTracker::focalLength() const
{
    return m_camera ? m_camera->focalLength() : QVector2D(0.0f, 0.0f);
}

QVector2D ObjectEdgesTracker::opticalCenter() const
{
    return m_camera ? m_camera->opticalCenter() : QVector2D(0.0f, 0.0f);
}

QSize ObjectEdgesTracker::frameSize() const
{
    return m_camera ? QSize(m_camera->imageSize().x(), m_camera->imageSize().y()) :
                      QSize(-1, -1);
}

QMatrix4x4 ObjectEdgesTracker::viewMatrix() const
{
    QMatrix4x4 M;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
            M(i, j) = m_R(i, j);
        M(i, 3) = m_t(i);
    }
    return M;
}

void ObjectEdgesTracker::compute(cv::Mat image)
{
    assert(image.channels() == 1);
    assert(m_camera);

    static cv::Mat binImage;
    cv::threshold(image, binImage, m_cannyThresholdA, 255.0, cv::THRESH_BINARY_INV);

    cv::medianBlur(binImage, binImage, 5);

    cv::dilate(binImage, binImage, cv::getStructuringElement(cv::MORPH_DILATE, cv::Size(3, 3), cv::Point(1, 1)),
               cv::Point(1, 1), 1);
    cv::erode(binImage, binImage, cv::getStructuringElement(cv::MORPH_ERODE, cv::Size(3, 3), cv::Point(1, 1)),
              cv::Point(1, 1), 1);

    m_debugImage = binImage;
    _tracking1(binImage);
    return;

    m_monitor->startTimer("Canny");
    cv::Mat edges;
    cv::Canny(image, edges, m_cannyThresholdA, m_cannyThresholdB);
    m_monitor->endTimer("Canny");

    m_monitor->startTimer("Inverting");
    cv::bitwise_not(edges, edges);
    m_monitor->endTimer("Inverting");

    float E = _tracking1(edges);

    qDebug() << "Error =" << E << edges.cols << edges.rows;

    if (debugEnabled())
    {
        m_monitor->startTimer("Debug");
        cv::cvtColor(edges, edges, cv::COLOR_GRAY2BGR);
        m_model.draw(edges, m_camera, m_R, m_t);

        m_monitor->endTimer("Debug");
    }
    m_debugImage = edges;
}

cv::Mat ObjectEdgesTracker::debugImage() const
{
    return m_debugImage;
}

float ObjectEdgesTracker::_tracking1(const cv::Mat & edges)
{
    m_monitor->startTimer("Distance transfrom [1]");
    cv::Mat distancesMap;
    cv::distanceTransform(edges, distancesMap, cv::DIST_L2, 3);
    m_monitor->endTimer("Distance transfrom [1]");

    Vectors3f controlModelPoints;
    float E = numeric_limits<float>::max();

    Matrix<double, 6, 1> x;
    x.segment<3>(0) = m_t.cast<double>();
    x.segment<3>(3) = ln_rotationMatrix(m_R.cast<double>().eval());

    cv::Mat dImage;
    cv::cvtColor(edges, dImage, cv::COLOR_GRAY2BGR);
    {
        cv::Mat d = dImage.clone();
        m_model.draw(d, m_camera, m_R, m_t);
        cv::imshow("w", d);
        cv::waitKey(-1);
    }

    m_monitor->startTimer("Tracking [1]");

    for (int i = 0; i < 30; ++i)
    {

        string iterName = QString("    Tracking [1] iter_%1").arg(i).toStdString();
        controlModelPoints = m_model.getControlPoints(m_camera, m_controlPixelDistance,
                                                      m_R, m_t);
        m_monitor->startTimer(iterName);
        /*if (controlModelPoints.size() < 4)
        {
            E = numeric_limits<float>::max();
            break;
        }*/

        E = optimize_pose(x,
                          QThreadPool::globalInstance(), 1,
                          distancesMap, m_camera, controlModelPoints, 30.0, 3);

        m_t = x.segment<3>(0).cast<float>();
        m_R = exp_rotationMatrix(x.segment<3>(3).eval()).cast<float>();
        m_monitor->endTimer(iterName);

        cv::Mat d = dImage.clone();
        m_model.draw(d, m_camera, m_R, m_t);
        cv::imshow("w", d);
        cv::waitKey(133);
    }

     m_monitor->endTimer("Tracking [1]");

    Vector2f bb_min(numeric_limits<float>::max(), numeric_limits<float>::max());
    Vector2f bb_max(- numeric_limits<float>::max(), - numeric_limits<float>::max());

    for (const Vector3f & v : controlModelPoints)
    {
        Vector2f p = m_camera->project((m_R * v + m_t).eval());
        if (p.x() < bb_min.x())
            bb_min.x() = p.x();
        if (p.y() < bb_min.y())
            bb_min.y() = p.y();
        if (p.x() > bb_max.x())
            bb_max.x() = p.x();
        if (p.y() > bb_max.y())
            bb_max.y() = p.y();
    }
    float area = (bb_max.x() - bb_min.x()) * (bb_max.y() - bb_min.y());
    if (area < 100.0f)
        E = numeric_limits<float>::max();
    if (E > 20.0f)
    {
        m_R = Matrix3f::Identity();
        m_t = Vector3f(0.0f, 0.0f, 5.0f);
    }

    return E;
}

float ObjectEdgesTracker::_tracking2(const cv::Mat & edges)
{
    m_monitor->startTimer("Distance transfrom [2]");
    cv::Mat distancesMap, labels;
    cv::distanceTransform(edges, distancesMap, labels, cv::DIST_L2, 3, cv::DIST_LABEL_PIXEL);
    m_monitor->endTimer("Distance transfrom [2]");

    m_monitor->startTimer("Indexing [2]");
    std::unordered_map<int, Vector2i> index2point;
    cv::Point2i p;
    for (p.y = 0; p.y < edges.rows; ++p.y)
    {
        const uchar * e_ptr = edges.ptr<uchar>(p.y, 0);
        const int * l_ptr = labels.ptr<int>(p.y, 0);
        for (p.x = 0; p.x < edges.cols; ++p.x)
        {
            if (e_ptr[p.x] == 0)
            {
                index2point[l_ptr[p.x]] = Vector2i(p.x, p.y);
            }
        }
    }
    m_monitor->endTimer("Indexing [2]");

    Vectors3f controlModelPoints;
    Vectors2f imagePoints;
    float E = numeric_limits<float>::max();

    Matrix<double, 6, 1> x;
    x.segment<3>(0) = m_t.cast<double>();
    x.segment<3>(3) = ln_rotationMatrix(m_R).cast<double>();

    m_monitor->startTimer("Tracking [2]");

    tie(controlModelPoints, imagePoints) = m_model.getControlAndImagePoints(m_camera, m_controlPixelDistance,
                                                                            m_R, m_t);

    for (size_t j = 0; j < controlModelPoints.size(); )
    {
        Vector2f & imagePoint = imagePoints[j];
        Vector2i imagePoint_i = imagePoint.cast<int>();
        auto it = index2point.find(labels.at<int>(imagePoint_i.y(), imagePoint_i.x()));
        if (it == index2point.cend())
        {
            controlModelPoints.erase(controlModelPoints.begin() + j);
            imagePoints.erase(imagePoints.begin() + j);
            continue;
        }
        imagePoints[j] = it->second.cast<float>();
        ++j;
    }

    cv::Mat dImage;
    cv::cvtColor(edges, dImage, cv::COLOR_GRAY2RGB);
    {
        cv::Mat d = dImage.clone();
        m_model.draw(d, m_camera, m_R, m_t);
        draw_residuals(d, m_R, m_t, m_camera, controlModelPoints, imagePoints);
        cv::imshow("w", d);
        cv::waitKey(33);
    }

    for (int i = 0; i < 30; ++i)
    {
        string iterName = QString("    Tracking [2] iter_%1").arg(i).toStdString();
        m_monitor->startTimer(iterName);

        /*if (controlModelPoints.size() < 4)
        {
            E = numeric_limits<float>::max();
            break;
        }*/

        E = optimize_pose(x, m_camera, controlModelPoints, imagePoints, 150.0f, 6);

        m_t = x.segment<3>(0).cast<float>();
        m_R = exp_rotationMatrix(x.segment<3>(3).eval()).cast<float>();

        m_monitor->endTimer(iterName);

        cv::Mat d = dImage.clone();
        m_model.draw(d, m_camera, m_R, m_t);
        draw_residuals(d, m_R, m_t, m_camera, controlModelPoints, imagePoints);
        cv::imshow("w", d);
        cv::waitKey(-1);
    }

    m_monitor->endTimer("Tracking [2]");

    Vector2f bb_min(numeric_limits<float>::max(), numeric_limits<float>::max());
    Vector2f bb_max(- numeric_limits<float>::max(), - numeric_limits<float>::max());

    for (const Vector3f & v : controlModelPoints)
    {
        Vector2f p = m_camera->project((m_R * v + m_t).eval());
        if (p.x() < bb_min.x())
            bb_min.x() = p.x();
        if (p.y() < bb_min.y())
            bb_min.y() = p.y();
        if (p.x() > bb_max.x())
            bb_max.x() = p.x();
        if (p.y() > bb_max.y())
            bb_max.y() = p.y();
    }
    float area = (bb_max.x() - bb_min.x()) * (bb_max.y() - bb_min.y());
    if (area < 100.0f)
        E = numeric_limits<float>::max();
    if (E > 7.0f)
    {
        m_R = Matrix3f::Identity();
        m_t = Vector3f(0.0f, 0.0f, 5.0f);
    }

    return E;
}
