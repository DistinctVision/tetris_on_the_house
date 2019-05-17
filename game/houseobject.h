#ifndef HOUSEOBJECT_H
#define HOUSEOBJECT_H

#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QSharedPointer>

#include <Eigen/Eigen>

#include "gl/gl_view.h"
#include "gl/gl_mesh.h"
#include "gl/gl_shadermaterial.h"
#include "gl/gl_screenobject.h"

class HouseObject
{
public:
    HouseObject(const Eigen::Vector3i & n_size,
                const Eigen::Vector3f & size,
                const Eigen::Vector3f & borderFirst,
                const Eigen::Vector3f & borderSecond);

    GL_MeshPtr meshForward() const;

    QMatrix4x4 matrixView2FrameUV(GL_ViewRenderer * view, const QSize & frameTextureSize) const;

    Eigen::Vector3f size() const;
    Eigen::Vector3i n_size() const;
    Eigen::Vector3f borderFirst() const;
    Eigen::Vector3f borderSecond() const;

    float activityLevel() const;
    void setActivityLevel(float activityLevel);

private:
    struct _FloorInfo
    {
        float y;
        QVector<GLuint> i_sides[4];
    };

    Eigen::Vector3i m_n_size;
    Eigen::Vector3f m_size;
    Eigen::Vector3f m_borderFirst;
    Eigen::Vector3f m_borderSecond;

    float m_activityLevel;

    GL_MeshPtr m_meshForward;

    GL_ScreenObjectPtr m_screenTempObject;

    void _createMeshForward();
    void _createMeshBackward();
};

using HouseObjectPtr = QSharedPointer<HouseObject>;

#endif // HOUSEOBJECT_H
