#include "posefilter.h"

using namespace Eigen;

PoseFilter::Motion::Motion()
{
    reset();
}

PoseFilter::Motion::Motion(const Vector3d & linear,
                           const Quaterniond & angular):
    linear(linear),
    angular(angular)
{
}

void PoseFilter::Motion::reset()
{
    linear.setZero();
    angular.setIdentity();
}

PoseFilter::Pose::Pose()
{
    reset();
}

PoseFilter::Pose::Pose(const Vector3d & position,
                       const Quaterniond & rotation):
    position(position),
    rotation(rotation)
{
}

void PoseFilter::Pose::reset()
{
    position.setZero();
    rotation.setIdentity();
}

PoseFilter::Motion PoseFilter::Pose::getMotion(const PoseFilter::Pose & next) const
{
    return Motion(next.position - position, next.rotation * rotation.conjugate());
}

PoseFilter::Pose PoseFilter::Pose::getNext(const PoseFilter::Motion & motion) const
{
    return Pose(position + motion.linear, motion.angular * rotation);
}

PoseFilter::Pose PoseFilter::Pose::mix(const PoseFilter::Pose & pose, double k_position, double k_angular) const
{
    return Pose(position * (1.0 - k_position) + pose.position * k_position,
                rotation.slerp(k_angular, pose.rotation));
}

PoseFilter::PoseFilter():
    m_positionWeights(100.0, 1.0),
    m_rotationWeights(100.0, 1.0)
{
    reset();
}

Vector2d PoseFilter::positionWeights() const
{
    return m_positionWeights;
}

void PoseFilter::setPositionWeights(const Vector2d & positionWeights)
{
    m_positionWeights = positionWeights;
}

Vector2d PoseFilter::rotationWeights() const
{
    return m_rotationWeights;
}

void PoseFilter::setRotationWeights(const Vector2d & rotationWeights)
{
    m_rotationWeights = rotationWeights;
}

void PoseFilter::reset()
{
    m_currentStep = 0;
    m_pose.reset();
    m_motion.reset();
}

void PoseFilter::reset(const Pose & pose)
{
    m_currentStep = 1;
    m_pose = pose;
    m_motion.reset();
}

const PoseFilter::Pose & PoseFilter::currentPose() const
{
    return m_pose;
}

const PoseFilter::Motion & PoseFilter::currentMotion() const
{
    return m_motion;
}

const PoseFilter::Pose & PoseFilter::next(const PoseFilter::Pose & rawPose)
{
    if (m_currentStep == 0)
    {
        m_pose = rawPose;
        ++m_currentStep;
    }
    else if (m_currentStep == 1)
    {
        m_motion = m_pose.getMotion(rawPose);
        m_pose = rawPose;
        ++m_currentStep;
    }
    else
    {
        Pose simulated_next = m_pose.getNext(m_motion);
        Pose next = rawPose.mix(simulated_next,
                                m_positionWeights.y() / (m_positionWeights.x() + m_positionWeights.y()),
                                m_rotationWeights.y() / (m_rotationWeights.x() + m_rotationWeights.y()));
        m_motion = m_pose.getMotion(next);
        m_pose = next;
    }
    return m_pose;
}

int PoseFilter::currentStep() const
{
    return m_currentStep;
}
