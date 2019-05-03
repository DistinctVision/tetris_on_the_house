#include "gl_screenobject.h"
#include <QMatrix4x4>

using namespace Eigen;

GL_ScreenObject::GL_ScreenObject(const GL_MeshPtr & mesh, const GL_ShaderMaterialPtr & material):
    m_mesh(mesh),
    m_material(material),
    m_origin(Vector2f::Zero()),
    m_size(Vector2f::Ones()),
    m_fillMode(FillMode::PreserveAspectCrop)
{
}

GL_MeshPtr GL_ScreenObject::mesh() const
{
    return m_mesh;
}

void GL_ScreenObject::setMesh(const GL_MeshPtr & mesh)
{
    m_mesh = mesh;
}

GL_ShaderMaterialPtr GL_ScreenObject::material() const
{
    return m_material;
}

void GL_ScreenObject::setMaterial(const GL_ShaderMaterialPtr & material)
{
    m_material = material;
}

FillMode::Enum GL_ScreenObject::fillMode() const
{
    return m_fillMode;
}

void GL_ScreenObject::setFillMode(FillMode::Enum fillMode)
{
    m_fillMode = fillMode;
}


Vector2f GL_ScreenObject::origin() const
{
    return m_origin;
}

void GL_ScreenObject::setOrigin(const Vector2f & origin)
{
    m_origin = origin;
}

Vector2f GL_ScreenObject::size() const
{
    return m_size;
}

void GL_ScreenObject::setSize(const Vector2f & size)
{
    m_size = size;
}

QMatrix4x4 GL_ScreenObject::getMatrixMVP(const QSize & viewportSize) const
{
    if ((viewportSize.width() == 0) || (viewportSize.height() == 0))
        return QMatrix4x4();

    Vector2f origin = m_origin;
    Vector2f size = m_size;

    switch (m_fillMode)
    {
    case FillMode::Stretch:
    {
        origin = Vector2f(0.0f, 0.0f);
        size = Vector2f(viewportSize.width(), viewportSize.height());
    } break;
    case FillMode::PreserveAspectFit:
    {
        float viewAspect = viewportSize.height() / static_cast<float>(viewportSize.width());

        float width = size.y() * viewAspect, height = static_cast<float>(viewportSize.height());
        if (width < static_cast<float>(viewportSize.width()))
        {
            width = static_cast<float>(viewportSize.width());
            height = width / viewAspect;
        }
        size.x() = width;
        size.y() = height;
        origin.x() = (width - viewportSize.width()) * 0.5f;
        origin.y() = (height - viewportSize.height()) * 0.5f;
    } break;
    case FillMode::PreserveAspectCrop:
    {
        float viewAspect = viewportSize.height() / static_cast<float>(viewportSize.width());

        float width = size.y() * viewAspect, height = static_cast<float>(viewportSize.height());
        if (width > static_cast<float>(viewportSize.width()))
        {
            width = static_cast<float>(viewportSize.width());
            height = width / viewAspect;
        }
        size.x() = width;
        size.y() = height;
        origin.x() = (viewportSize.width() - width) * 0.5f;
        origin.y() = (viewportSize.height() - height) * 0.5f;
    } break;
    default:
        break;
    }
    QMatrix4x4 imageTransform;
    imageTransform(0, 0) = size.x();
    imageTransform(0, 3) = origin.x();
    imageTransform(1, 1) = size.y();
    imageTransform(1, 3) = origin.y();
    QMatrix4x4 orto;
    orto.ortho(0.0f, viewportSize.width(), viewportSize.height(), 0.0f, 0.0f, 1.0f);
    return orto * imageTransform;
}

void GL_ScreenObject::draw(GL_ViewRenderer * view)
{
    if (m_material->containsValue("matrixMVP"))
    {
        m_material->setValue("matrixMVP", getMatrixMVP(view->viewportSize()));
    }
    m_mesh->draw(view, *m_material);
}
