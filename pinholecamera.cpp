#include "pinholecamera.h"
#include <limits>
#include <climits>

using namespace Eigen;

PinholeCamera::PinholeCamera(const Vector2i & imageSize,
                             const Vector2f & focalLength,
                             const Vector2f & opticalCenter):
    m_imageSize(imageSize),
    m_focalLength(focalLength),
    m_opticalCenter(opticalCenter)
{
}

Vector2i PinholeCamera::imageSize() const
{
    return m_imageSize;
}

Vector2f PinholeCamera::focalLength() const
{
    return m_focalLength;
}

Vector2f PinholeCamera::opticalCenter() const
{
    return m_opticalCenter;
}

Vector2f PinholeCamera::project(const Vector2f & view) const
{
    return Vector2f(view.x() * m_focalLength.x() + m_opticalCenter.x(),
                    view.y() * m_focalLength.y() + m_opticalCenter.y());
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
    Vector2f d = imagePoint - m_opticalCenter;
    return Vector3f(d.x() / m_focalLength.x(), d.y() / m_focalLength.y(), 1.0f);
}

Vector2f PinholeCamera::unprojectToView(const Vector2f & imagePoint) const
{
    Vector2f d = imagePoint - m_opticalCenter;
    return Vector2f(d.x() / m_focalLength.x(), d.y() / m_focalLength.y());
}
