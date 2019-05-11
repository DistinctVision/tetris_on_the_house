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

private:
    struct _FloorInfo
    {
        float y;
        QVector<GLuint> i_sides[4];
    };

    GL_MeshPtr m_mesh;
    GL_ShaderMaterialPtr m_material;

    Eigen::Vector3i m_n_size;
    Eigen::Vector3f m_size;
    Eigen::Vector3f m_borderFirst;
    Eigen::Vector3f m_borderSecond;

    QVector<_FloorInfo> m_floorInfos;

    GL_ScreenObjectPtr m_screenTempObject;

    void _createMesh();
};

using HouseObjectPtr = QSharedPointer<HouseObject>;

#endif // HOUSEOBJECT_H
