#ifndef PINHOLECAMERA_H
#define PINHOLECAMERA_H

#include <QVector2D>

#include <Eigen/Eigen>

class PinholeCamera
{
public:
    PinholeCamera(const Eigen::Vector2i & imageSize,
                  const Eigen::Vector2f & pixelFocalLength = { 640.0f, 640.0f },
                  const Eigen::Vector2f & pixelOpticalCenter = { 320.0f, 240.0f });

    Eigen::Vector2i imageSize() const;
    Eigen::Vector2f pixelFocalLength() const;
    Eigen::Vector2f pixelOpticalCenter() const;
    QVector2D focalLength() const;
    QVector2D opticalCenter() const;

    Eigen::Vector2f project(const Eigen::Vector2f & view) const;
    Eigen::Vector2f project(const Eigen::Vector3f & v) const;
    bool imagePointInView(const Eigen::Vector2f & imagePoint) const;
    Eigen::Vector2f project(const Eigen::Vector2f & view, bool & inViewFlag) const;
    Eigen::Vector2f project(const Eigen::Vector3f & v, bool & inViewFlag) const;

    Eigen::Vector3f unproject(const Eigen::Vector2f & imagePoint) const;
    Eigen::Vector2f unprojectToView(const Eigen::Vector2f & imagePoint) const;

private:
    Eigen::Vector2i m_imageSize;
    Eigen::Vector2f m_pixelFocalLength;
    Eigen::Vector2f m_pixelOpticalCenter;
};

#endif // PINHOLECAMERA_H
