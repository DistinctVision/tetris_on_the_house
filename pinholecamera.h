#ifndef PINHOLECAMERA_H
#define PINHOLECAMERA_H

#include <Eigen/Eigen>

class PinholeCamera
{
public:
    PinholeCamera(const Eigen::Vector2i & imageSize,
                  const Eigen::Vector2f & focalLength = { 640.0f, 640.0f },
                  const Eigen::Vector2f & opticalCenter = { 320.0f, 240.0f });

    Eigen::Vector2i imageSize() const;
    Eigen::Vector2f focalLength() const;
    Eigen::Vector2f opticalCenter() const;

    Eigen::Vector2f project(const Eigen::Vector3f & v) const;
    bool imagePointInView(const Eigen::Vector2f & imagePoint) const;
    Eigen::Vector2f project(const Eigen::Vector3f & v, bool & inViewFlag) const;

    Eigen::Vector3f unproject(const Eigen::Vector2f & imagePoint) const;

private:
    Eigen::Vector2i m_imageSize;
    Eigen::Vector2f m_focalLength;
    Eigen::Vector2f m_opticalCenter;
};

#endif // PINHOLECAMERA_H
