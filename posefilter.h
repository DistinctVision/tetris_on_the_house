#ifndef POSEFILTER_H
#define POSEFILTER_H

#include <Eigen/Eigen>

class PoseFilter
{
public:
    struct Motion
    {
        Eigen::Vector3d linear;
        Eigen::Quaterniond angular;

        Motion();
        Motion(const Eigen::Vector3d & linear,
               const Eigen::Quaterniond & angular);
        void reset();
    };

    struct Pose
    {
        Eigen::Vector3d position;
        Eigen::Quaterniond rotation;

        Pose();
        Pose(const Eigen::Vector3d & position,
             const Eigen::Quaterniond & rotation);

        void reset();

        Motion getMotion(const Pose & next) const;
        Pose getNext(const Motion & motion) const;

        Pose mix(const Pose & pose, double k_position, double k_angular) const;
    };

    PoseFilter();

    Eigen::Vector2d positionWeights() const;
    void setPositionWeights(const Eigen::Vector2d & positionWeights);

    Eigen::Vector2d rotationWeights() const;
    void setRotationWeights(const Eigen::Vector2d & rotationWeights);

    void reset();
    void reset(const Pose & pose);

    const Pose & currentPose() const;
    const Motion & currentMotion() const;
    const Pose & next(const Pose & rawPose);

private:
    Eigen::Vector2d m_positionWeights;
    Eigen::Vector2d m_rotationWeights;

    Pose m_pose;
    Motion m_motion;
    int m_currentStep;
};

#endif // POSEFILTER_H
