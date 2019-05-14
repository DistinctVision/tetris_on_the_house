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
    HouseObject(GL_ViewRenderer * view,
                const Eigen::Vector3i & n_size,
                const Eigen::Vector3f & size,
                const Eigen::Vector3f & borderFirst,
                const Eigen::Vector3f & borderSecond);

    void draw(GL_ViewRenderer * view, const QMatrix4x4 & viewMatrix,
              GLuint frameTextureId, const QSize & frameTextureSize);

    Eigen::Vector3f size() const;
    Eigen::Vector3i n_size() const;
    Eigen::Vector3f borderFirst() const;
    Eigen::Vector3f borderSecond() const;

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

    GL_MeshPtr m_meshForward;
    GL_ShaderMaterialPtr m_materialForward;
    QVector<_FloorInfo> m_floorInfos;
    QVector<QVector2D> m_houseTextureCoords;

    GL_MeshPtr m_meshBackward;
    GL_ShaderMaterialPtr m_materialBackward;

    GL_ScreenObjectPtr m_screenTempObject;

    void _createMeshForward();
    void _createMeshBackward();

    void _moveFloors(float dy);
};

using HouseObjectPtr = QSharedPointer<HouseObject>;

#endif // HOUSEOBJECT_H
