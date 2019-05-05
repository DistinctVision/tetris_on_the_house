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

using namespace std;
using namespace std::chrono;
using namespace Eigen;

ObjectEdgesTracker::ObjectEdgesTracker():
    m_controlPixelDistance(15.0f),
    m_cannyThresholdA(50.0),
    m_cannyThresholdB(100.0),
    m_model(ObjectModel::createCubikRubik())
{
    m_monitor = make_shared<PerformanceMonitor>();
    m_R = Matrix3d::Identity();
    m_t = Vector3d(0.0, 0.0, 5.0);
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
    m_camera = camera;
}

QMatrix4x4 ObjectEdgesTracker::viewMatrix() const
{
    QMatrix4x4 M;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
            M(i, j) = static_cast<float>(m_R(i, j));
        M(i, 3) = static_cast<float>(m_t(i));
    }
    return M;
}

void ObjectEdgesTracker::compute(cv::Mat image)
{
    assert(image.channels() == 1);
    assert(m_camera);

    m_monitor->start();

    m_monitor->startTimer("Canny");
    cv::Mat edges;
    cv::Canny(image, edges, m_cannyThresholdA, m_cannyThresholdB);
    m_monitor->endTimer("Canny");

    m_monitor->startTimer("Inverting");
    cv::bitwise_not(edges, edges);
    m_monitor->endTimer("Inverting");

    m_monitor->startTimer("Distance transfrom");
    cv::Mat distancesMap;
    cv::distanceTransform(edges, distancesMap, cv::DIST_L2, 3);
    m_monitor->endTimer("Distance transfrom");

    m_monitor->startTimer("Tracking");
    double E = _tracking(distancesMap);
    m_monitor->endTimer("Tracking");

    qDebug() << "Error =" << E;

    if (debugEnabled())
    {
        m_monitor->startTimer("Debug");
        cv::cvtColor(edges, edges, cv::COLOR_GRAY2BGR);
        m_model.draw(edges, m_camera, m_R, m_t);

        m_monitor->endTimer("Debug");
    }
    m_monitor->end();
    qDebug().noquote() << QString::fromStdString(m_monitor->report());
    m_debugImage = edges;
}

cv::Mat ObjectEdgesTracker::debugImage() const
{
    return m_debugImage;
}

double ObjectEdgesTracker::_tracking(const cv::Mat & distancesMap)
{
    mt19937 rnd_gen;
    uniform_int_distribution<int> rnd(0, 1000);
    rnd_gen.seed(static_cast<unsigned int>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()));

    auto getRandomUnitVector = [&] () -> Vector3d
    {
        return Vector3d((rnd(rnd_gen) % 2000) / 1000.0 - 1.0,
                        (rnd(rnd_gen) % 2000) / 1000.0 - 1.0,
                        (rnd(rnd_gen) % 2000) / 1000.0 - 1.0).normalized();
    };
    auto getRandomUnitScalar = [&] () -> double
    {
        return ((rnd(rnd_gen) % 2000) / 1e3 - 1.0) * 1e-3;
    };

    static const double r_delta_angle = 10.0 * (M_PI / 180.0);
    static const double r_delta_image = 35.0;

    Vector2d focalLength = m_camera->focalLength().cast<double>();

    Vectors3d controlModelPoints;
    double E = numeric_limits<double>::max();

    Matrix<double, 6, 1> x;
    x.segment<3>(0) = m_t;
    x.segment<3>(3) = ln_rotationMatrix(m_R);

    for (int i = 0; i < 5; ++i)
    {
        controlModelPoints = m_model.getControlPoints(m_camera, m_controlPixelDistance,
                                                      m_R, m_t);
        if (controlModelPoints.size() < 4)
        {
            E = numeric_limits<double>::max();
            break;
        }

        Matrix<double, 6, 1> x1 = x;
        double E1 = optimize_pose(x1,
                                  QThreadPool::globalInstance(), QThread::idealThreadCount(),
                                  distancesMap, m_camera, controlModelPoints, 60.0f, 6);

        for (int j = 0; j < 0; ++j)
        {
            Matrix<double, 6, 1> x2 = x;
            x2.segment<3>(0) += getRandomUnitVector() * getRandomUnitScalar() * r_delta_image * (x.z() / focalLength.x());
            x2.segment<3>(3) += getRandomUnitVector() * getRandomUnitScalar() * r_delta_angle;

            controlModelPoints = m_model.getControlPoints(m_camera, m_controlPixelDistance,
                                                          exp_rotationMatrix(x2.segment<3>(3)), x2.segment<3>(0));
            if (controlModelPoints.size() < 4)
            {
                continue;
            }

            double E2 = optimize_pose(x2,
                                      QThreadPool::globalInstance(), QThread::idealThreadCount(),
                                      distancesMap, m_camera, controlModelPoints, 60.0f, 6);
            if (E2 < E1)
            {
                E1 = E2;
                x1 = x2;
            }
        }

        if (E1 < E)
        {
            E = E1;
            x = x1;
        }
        m_t = x.segment<3>(0);
        m_R = exp_rotationMatrix(x.segment<3>(3));
    }

    Vector2f bb_min(numeric_limits<float>::max(), numeric_limits<float>::max());
    Vector2f bb_max(- numeric_limits<float>::max(), - numeric_limits<float>::max());

    for (const Vector3d & v : controlModelPoints)
    {
        Vector2f p = m_camera->project((m_R * v + m_t).cast<float>());
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
        E = numeric_limits<double>::max();
    if (E > 2.5)
    {
        m_R = Matrix3d::Identity();
        m_t = Vector3d(0.0, 0.0, 5.0);
    }

    return E;
}
