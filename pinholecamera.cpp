#include "pinholecamera.h"
#include <limits>
#include <climits>

using namespace Eigen;

PinholeCamera::PinholeCamera(const Vector2i & imageSize,
                             const Vector2f & pixelFocalLength,
                             const Vector2f & pixelOpticalCenter):
    m_imageSize(imageSize),
    m_pixelFocalLength(pixelFocalLength),
    m_pixelOpticalCenter(pixelOpticalCenter)
{
}

Vector2i PinholeCamera::imageSize() const
{
    return m_imageSize;
}

Vector2f PinholeCamera::pixelFocalLength() const
{
    return m_pixelFocalLength;
}

Vector2f PinholeCamera::pixelOpticalCenter() const
{
    return m_pixelOpticalCenter;
}

QVector2D PinholeCamera::focalLength() const
{
    return QVector2D(m_pixelFocalLength.x() / static_cast<float>(m_imageSize.x()),
                     m_pixelFocalLength.y() / static_cast<float>(m_imageSize.x()));
}

QVector2D PinholeCamera::opticalCenter() const
{
    return QVector2D(m_pixelOpticalCenter.x() / static_cast<float>(m_imageSize.x()),
                     m_pixelOpticalCenter.y() / static_cast<float>(m_imageSize.y()));
}

Vector2f PinholeCamera::project(const Vector2f & view) const
{
    return Vector2f(view.x() * m_pixelFocalLength.x() + m_pixelOpticalCenter.x(),
                    view.y() * m_pixelFocalLength.y() + m_pixelOpticalCenter.y());
}

Vector2f PinholeCamera::project(const Vector3f & v) const
{
    return project((v.segment<2>(0) / v.z()).eval());
}

bool PinholeCamera::imagePointInView(const Vector2f & imagePoint) const
{
    return ((imagePoint.x() >= 0.0f) &&
            (imagePoint.y() >= 0.0f) &&
            (imagePoint.x() < (m_imageSize.x() - 1.0f)) &&
            (imagePoint.y() < (m_imageSize.y() - 1.0f)));
}

Vector2f PinholeCamera::project(const Vector3f & v, bool & inViewFlag) const
{
    if (v.z() < std::numeric_limits<float>::epsilon())
    {
        inViewFlag = false;
        return Vector2f::Zero();
    }
    return project((v.segment<2>(0) / v.z()).eval(), inViewFlag);
}

Vector2f PinholeCamera::project(const Vector2f & view, bool & inViewFlag) const
{
    Vector2f p = project(view);
    if (!imagePointInView(p))
    {
        inViewFlag = false;
        return Vector2f::Zero();
    }
    inViewFlag = true;
    return p;
}

Vector3f PinholeCamera::unproject(const Vector2f & imagePoint) const
{
    Vector2f d = imagePoint - m_pixelOpticalCenter;
    return Vector3f(d.x() / m_pixelFocalLength.x(), d.y() / m_pixelFocalLength.y(), 1.0f);
}

Vector2f PinholeCamera::unprojectToView(const Vector2f & imagePoint) const
{
    Vector2f d = imagePoint - m_pixelOpticalCenter;
    return Vector2f(d.x() / m_pixelFocalLength.x(), d.y() / m_pixelFocalLength.y());
}
