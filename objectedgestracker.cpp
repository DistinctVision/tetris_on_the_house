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
    m_controlPixelDistance(20.0f),
    m_binaryThreshold(50.0),
    m_minBlobArea(30.0),
    m_maxBlobCircularity(0.25),
    m_model(ObjectModel::createHouse())
{
    m_resetCameraPose = Pose(Vector3d(0.0, 10.0, -100.0), Quaterniond(1.0, 0.0, 0.0, 0.0));
    m_poseFilter.reset(m_resetCameraPose);
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

double ObjectEdgesTracker::binaryThreshold() const
{
    return m_binaryThreshold;
}

void ObjectEdgesTracker::setBinaryThreshold(double binaryThreshold)
{
    if (m_binaryThreshold == binaryThreshold)
        return;
    m_binaryThreshold = binaryThreshold;
    emit binaryThresholdChanged();
}

double ObjectEdgesTracker::maxBlobCircularity() const
{
    return m_maxBlobCircularity;
}

void ObjectEdgesTracker::setMaxBlobCircularity(double maxBlobCircularity)
{
    if (maxBlobCircularity == m_maxBlobCircularity)
        return;
    m_maxBlobCircularity = maxBlobCircularity;
    emit maxBlobCircularityChanged();
}

double ObjectEdgesTracker::minBlobArea() const
{
    return m_minBlobArea;
}

void ObjectEdgesTracker::setMinBlobArea(double minBlobArea)
{
    if (minBlobArea == m_minBlobArea)
        return;
    m_minBlobArea = minBlobArea;
    emit minBlobAreaChanged();
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
    Pose currentCameraPose = m_poseFilter.currentPose();
    Matrix3f R = currentCameraPose.rotation.conjugate().toRotationMatrix().cast<float>();
    Vector3f t = - R * currentCameraPose.position.cast<float>();
    QMatrix4x4 M;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
            M(i, j) = R(i, j);
        M(i, 3) = t(i);
    }
    return M;
}

void ObjectEdgesTracker::compute(cv::Mat image)
{
    assert(image.channels() == 1);
    assert(m_camera);

    cv::Mat kernel = (cv::Mat_<float>(3,3) <<
                      1,  1, 1,
                      1, -8, 1,
                      1,  1, 1);
    cv::Mat imgLaplacian;
    cv::filter2D(image, imgLaplacian, CV_32F, kernel);
    imgLaplacian.convertTo(imgLaplacian, CV_8UC1);

    cv::Mat binImage;
    m_monitor->startTimer("Threshold");
    //cv::adaptiveThreshold(imgLaplacian, binImage, 255.0, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 33, m_binaryThreshold - 128.0);
    cv::threshold(image, binImage, m_binaryThreshold, 255.0, cv::THRESH_BINARY);
    m_monitor->endTimer("Threshold");

    m_monitor->startTimer("Dilate");
    //cv::dilate(binImage, binImage, cv::getStructuringElement(cv::MORPH_DILATE, cv::Size(3, 3), cv::Point(1, 1)), cv::Point(1, 1), 1);
    m_monitor->startTimer("Dilate");

    m_monitor->startTimer("Find contours");
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
    m_monitor->endTimer("Find contours");

    m_monitor->startTimer("Filter contours");
    for (size_t contourIdx = 0; contourIdx < contours.size(); ++contourIdx)
    {
        cv::Moments moms = cv::moments(contours[contourIdx]);
        {
            double area = moms.m00;
            if (area < m_minBlobArea)
            {
                cv::drawContours(binImage, contours,
                                 static_cast<int>(contourIdx), cv::Scalar(0), -1);
                continue;
            }
        }

        {
            double area = moms.m00;
            double perimeter = arcLength(contours[contourIdx], true);
            double ratio = 4.0 * M_PI * area / (perimeter * perimeter);
            if (ratio > m_maxBlobCircularity)
            {
                cv::drawContours(binImage, contours,
                                 static_cast<int>(contourIdx), cv::Scalar(0), -1);
                continue;
            }
        }
    }
    m_monitor->endTimer("Filter contours");

    m_monitor->startTimer("Erode");
    cv::erode(binImage, binImage, cv::getStructuringElement(cv::MORPH_ERODE, cv::Size(3, 3), cv::Point(1, 1)), cv::Point(1, 1), 1);
    m_monitor->endTimer("Erode");

    m_monitor->startTimer("Inverting");
    cv::bitwise_not(binImage, binImage);
    m_monitor->endTimer("Inverting");

    qDebug() << "Error =" << _tracking1(binImage);
    m_debugImage = binImage;
    if (debugEnabled())
    {
        Pose currentCameraPose = m_poseFilter.currentPose();
        m_monitor->startTimer("Debug");
        cv::cvtColor(m_debugImage, m_debugImage, cv::COLOR_GRAY2BGR);
        Quaterniond q = currentCameraPose.rotation.normalized().conjugate();
        Matrix3f R = q.toRotationMatrix().cast<float>();
        Vector3f t = - (q * currentCameraPose.position).cast<float>();
        m_model.draw(m_debugImage, m_camera, R, t);
        m_monitor->endTimer("Debug");
    }
}

cv::Mat ObjectEdgesTracker::debugImage() const
{
    return m_debugImage;
}

Matrix<double, 6, 1> ObjectEdgesTracker::_pose2x(const Pose & pose) const
{
    Matrix<double, 6, 1> x;
    Quaterniond q = pose.rotation.normalized().conjugate();
    x.segment<3>(0) = - (q * pose.position);
    AngleAxisd aa(q);
    x.segment<3>(3) = aa.axis() * aa.angle();
    return x;
}

ObjectEdgesTracker::Pose ObjectEdgesTracker::_x2pose(const Matrix<double, 6, 1> & x) const
{
    double l = x.segment<3>(3).norm();
    Quaterniond q = (l > 1e-6) ? Quaterniond(AngleAxisd(l, x.segment<3>(3) / l)).conjugate() :
                                 Quaterniond(1.0, 0.0, 0.0, 0.0);
    return Pose(- (q * x.segment<3>(0)), q);
}

float ObjectEdgesTracker::_tracking1(const cv::Mat & edges)
{
    m_monitor->startTimer("Distance transfrom [1]");
    cv::Mat distancesMap;
    cv::distanceTransform(edges, distancesMap, cv::DIST_L2, 3);
    m_monitor->endTimer("Distance transfrom [1]");

    Vectors3f controlModelPoints;
    float E = numeric_limits<float>::max();

    Matrix<double, 6, 1> x = _pose2x(m_poseFilter.currentPose());

    Matrix3f R = exp_rotationMatrix(x.segment<3>(3).eval()).cast<float>();
    Vector3f t = x.segment<3>(0).cast<float>();

    m_monitor->startTimer("Tracking [1]");

    Vector3d prevViewPostition = m_poseFilter.currentPose().position;

    for (int i = 0; i < 2; ++i)
    {
        string iterName = QString("    Tracking [1] iter_%1").arg(i).toStdString();
        m_monitor->startTimer(iterName);
        controlModelPoints = m_model.getControlPoints(m_camera, m_controlPixelDistance, R, t);
        if (controlModelPoints.size() < 4)
        {
            E = numeric_limits<float>::max();
            break;
        }

        if (m_poseFilter.currentStep() > 1)
        {
            E = static_cast<float>(optimize_pose(x,
                              QThreadPool::globalInstance(), QThread::idealThreadCount(),
                              distancesMap, m_camera, controlModelPoints, 30.0, 10,
                                                 1.5 / 3.0, prevViewPostition));
        }
        else
        {
            E = static_cast<float>(optimize_pose(x,
                              QThreadPool::globalInstance(), QThread::idealThreadCount(),
                              distancesMap, m_camera, controlModelPoints, 30.0, 10));
        }

        R = exp_rotationMatrix(x.segment<3>(3).eval()).cast<float>();
        t = x.segment<3>(0).cast<float>();

        m_monitor->endTimer(iterName);
    }
    m_monitor->endTimer("Tracking [1]");

    Vector2f bb_min(numeric_limits<float>::max(), numeric_limits<float>::max());
    Vector2f bb_max(- numeric_limits<float>::max(), - numeric_limits<float>::max());

    for (const Vector3f & v : controlModelPoints)
    {
        Vector2f p = m_camera->project((R * v + t).eval());
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
    if (E > 2.0f)
    {
        m_poseFilter.reset(m_resetCameraPose);
    }
    else
    {
        Vector3d pose = _x2pose(x).position;
        qDebug().noquote() << QString("pose = %1 %2 %3").arg(pose.x()).arg(pose.y()).arg(pose.z());
        m_poseFilter.next(_x2pose(x));
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

    Matrix<double, 6, 1> x = _pose2x(m_poseFilter.currentPose());

    Matrix3f R = exp_rotationMatrix(x.segment<3>(3).eval()).cast<float>();
    Vector3f t = x.segment<3>(0).cast<float>();

    m_monitor->startTimer("Tracking [2]");

    tie(controlModelPoints, imagePoints) = m_model.getControlAndImagePoints(m_camera, m_controlPixelDistance, R, t);

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

    for (int i = 0; i < 5; ++i)
    {
        string iterName = QString("    Tracking [2] iter_%1").arg(i).toStdString();
        m_monitor->startTimer(iterName);

        if (controlModelPoints.size() < 4)
        {
            E = numeric_limits<float>::max();
            break;
        }

        E = optimize_pose(x, m_camera, controlModelPoints, imagePoints, 150.0f, 6);

        R = exp_rotationMatrix(x.segment<3>(3).eval()).cast<float>();
        t = x.segment<3>(0).cast<float>();

        m_monitor->endTimer(iterName);
    }

    m_monitor->endTimer("Tracking [2]");

    Vector2f bb_min(numeric_limits<float>::max(), numeric_limits<float>::max());
    Vector2f bb_max(- numeric_limits<float>::max(), - numeric_limits<float>::max());

    for (const Vector3f & v : controlModelPoints)
    {
        Vector2f p = m_camera->project((R * v + t).eval());
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
    if (E > 2.0f)
    {
        m_poseFilter.reset(m_resetCameraPose);
    }
    else
    {
        m_poseFilter.next(_x2pose(x));
    }

    return E;
}
