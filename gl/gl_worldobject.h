#ifndef GL_WORLDOBJECT_H
#define GL_WORLDOBJECT_H

#include <QSize>
#include <QSharedPointer>
#include <QMatrix4x4>

#include <Eigen/Eigen>

#include "gl_mesh.h"
#include "gl_shadermaterial.h"

#include "gl_view.h"

class GL_WorldObject
{
public:
    GL_WorldObject(const GL_MeshPtr & mesh, const GL_ShaderMaterialPtr & materialPtr);

    GL_MeshPtr mesh() const;
    void setMesh(const GL_MeshPtr & mesh);

    GL_ShaderMaterialPtr material() const;
    void setMaterial(const GL_ShaderMaterialPtr & material);

    QMatrix4x4 worldMatrix() const;
    void setWorldMatrix(const QMatrix4x4 & worldMatrix);

    void draw(GL_ViewRenderer * view, const QMatrix4x4 & viewMatrix = QMatrix4x4());

private:
    GL_MeshPtr m_mesh;
    GL_ShaderMaterialPtr m_material;
    QMatrix4x4 m_worldMatrix;
};

using GL_WorldObjectPtr = QSharedPointer<GL_WorldObject>;
#endif // GL_WORLDOBJECT_H
