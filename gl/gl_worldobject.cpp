#include "gl_worldobject.h"

GL_WorldObject::GL_WorldObject(const GL_MeshPtr & mesh, const GL_ShaderMaterialPtr & material):
    m_mesh(mesh),
    m_material(material)
{}

GL_MeshPtr GL_WorldObject::mesh() const
{
    return m_mesh;
}

void GL_WorldObject::setMesh(const GL_MeshPtr & mesh)
{
    m_mesh = mesh;
}

GL_ShaderMaterialPtr GL_WorldObject::material() const
{
    return m_material;
}

void GL_WorldObject::setMaterial(const GL_ShaderMaterialPtr & material)
{
    m_material = material;
}

QMatrix4x4 GL_WorldObject::worldMatrix() const
{
    return m_worldMatrix;
}

void GL_WorldObject::setWorldMatrix(const QMatrix4x4 & worldMatrix)
{
    m_worldMatrix = worldMatrix;
}

void GL_WorldObject::draw(GL_ViewRenderer * view, const QMatrix4x4 & viewMatrix)
{
    if (m_material->containsValue("matrixMVP"))
    {
        m_material->setValue("matrixMVP", view->projectionMatrix() * viewMatrix * m_worldMatrix);
    }
    m_mesh->draw(view, *m_material);
}
