#ifndef GL_SCREENOBJECT_H
#define GL_SCREENOBJECT_H

#include <QSize>
#include <QSharedPointer>

#include <Eigen/Eigen>

#include "gl_mesh.h"
#include "gl_shadermaterial.h"

#include "gl_view.h"

class GL_ScreenObject
{
public:
    GL_ScreenObject(const GL_MeshPtr & mesh, const GL_ShaderMaterialPtr & material);

    GL_MeshPtr mesh() const;
    void setMesh(const GL_MeshPtr & mesh);

    GL_ShaderMaterialPtr material() const;
    void setMaterial(const GL_ShaderMaterialPtr & material);

    FillMode::Enum fillMode() const;
    void setFillMode(FillMode::Enum fillMode);

    Eigen::Vector2f origin() const;
    void setOrigin(const Eigen::Vector2f & origin);

    Eigen::Vector2f size() const;
    void setSize(const Eigen::Vector2f & size);

    QMatrix4x4 getMatrixMVP(const QSize & viewportSize) const;

    void draw(GL_ViewRenderer * view);

private:
    GL_MeshPtr m_mesh;
    GL_ShaderMaterialPtr m_material;

    Eigen::Vector2f m_origin;
    Eigen::Vector2f m_size;

    FillMode::Enum m_fillMode;
};

using GL_ScreenObjectPtr = QSharedPointer<GL_ScreenObject>;

#endif // GL_SCREENOBJECT_H
