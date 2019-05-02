#include "gl_mesh.h"

#include <QVector>

#include "gl_shadermaterial.h"

GL_Mesh::GL_Mesh():
    m_vertexBuffer(QOpenGLBuffer::VertexBuffer),
    m_normalsBuffer(QOpenGLBuffer::VertexBuffer),
    m_indicesBuffer(QOpenGLBuffer::IndexBuffer)
{
    m_vertexBuffer.create();
    m_normalsBuffer.create();
    m_indicesBuffer.create();
}

QVector<QVector3D> GL_Mesh::computeNormals(const QVector<QVector3D> & vertices, const QVector<int> & indices)
{
    QVector<QVector3D> normals(vertices.size());
    normals.fill(QVector3D(0.0, 0.0, 0.0));
    for (int i = 0; i < indices.size(); i += 3)
    {
        QVector3D n = QVector3D::crossProduct((vertices[indices[i + 2]] - vertices[indices[i + 1]]),
                                              (vertices[indices[i + 0]] - vertices[indices[i + 1]]));
        n.normalize();
        normals[indices[i + 0]] += n;
        normals[indices[i + 1]] += n;
        normals[indices[i + 2]] += n;
    }
    for (QVector3D & n : normals)
        n.normalize();
    return normals;
}

GL_Mesh GL_Mesh::createQuad(const QVector2D & size)
{
    QVector2D halfSize = size * 0.5f;

    GL_Mesh mesh;
    QVector<QVector3D> vertices = {
        QVector3D(- halfSize.x(), - halfSize.y(), 0.0f),
        QVector3D(halfSize.x(), - halfSize.y(), 0.0f),
        QVector3D(halfSize.x(), halfSize.y(), 0.0f),
        QVector3D(- halfSize.x(), halfSize.y(), 0.0f)
    };
    mesh.m_vertexBuffer.bind();
    mesh.m_vertexBuffer.allocate(vertices.data(), static_cast<int>(sizeof(QVector3D)) * vertices.size());
    QVector<int> indices = {
        0, 1, 2,
        0, 2, 3
    };
    mesh.m_indicesBuffer.bind();
    mesh.m_indicesBuffer.allocate(vertices.data(), static_cast<int>(sizeof(int)) * indices.size());
    QVector<QVector3D> normals = computeNormals(vertices, indices);
    mesh.m_normalsBuffer.bind();
    mesh.m_normalsBuffer.allocate(normals.data(), static_cast<int>(sizeof(QVector3D)) * normals.size());
    mesh.m_normalsBuffer.release();
    return mesh;
}

GL_Mesh GL_Mesh::createCube(const QVector3D & size)
{
    QVector3D halfSize = size * 0.5f;

    GL_Mesh mesh;
    QVector<QVector3D> vertices = {
        QVector3D(- halfSize.x(), - halfSize.y(), halfSize.z()),
        QVector3D(halfSize.x(), - halfSize.y(), halfSize.z()),
        QVector3D(halfSize.x(), halfSize.y(), halfSize.z()),
        QVector3D(- halfSize.x(), halfSize.y(), halfSize.z()),
        QVector3D(- halfSize.x(), - halfSize.y(), - halfSize.z()),
        QVector3D(halfSize.x(), - halfSize.y(), - halfSize.z()),
        QVector3D(halfSize.x(), halfSize.y(), - halfSize.z()),
        QVector3D(- halfSize.x(), halfSize.y(), - halfSize.z())
    };
    mesh.m_vertexBuffer.bind();
    mesh.m_vertexBuffer.allocate(vertices.data(), static_cast<int>(sizeof(QVector3D)) * vertices.size());
    QVector<int> indices = {
        0, 1, 2,
        0, 2, 3,
        7, 6, 5,
        7, 5, 4,
        1, 5, 6,
        1, 6, 2,
        0, 3, 7,
        0, 7, 4,
        2, 6, 7,
        2, 7, 3,
        0, 4, 5,
        0, 5, 1
    };
    mesh.m_indicesBuffer.bind();
    mesh.m_indicesBuffer.allocate(vertices.data(), static_cast<int>(sizeof(int)) * indices.size());
    QVector<QVector3D> normals = computeNormals(vertices, indices);
    mesh.m_normalsBuffer.bind();
    mesh.m_normalsBuffer.allocate(normals.data(), static_cast<int>(sizeof(QVector3D)) * normals.size());
    mesh.m_normalsBuffer.release();
    return mesh;
}

void GL_Mesh::draw(QOpenGLFunctions * gl, const GL_ShaderMaterial & shaderMaterial)
{
    shaderMaterial.bind();

    m_indicesBuffer.bind();

    int vertexLocation = shaderMaterial.attributeLocation("vertex");
    int normalLocation = shaderMaterial.attributeLocation("normal");

    if (vertexLocation >= 0)
    {
        shaderMaterial.enableAttribute(vertexLocation);
        m_vertexBuffer.bind();
        shaderMaterial.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(QVector3D));
    }
    if (normalLocation >= 0)
    {
        shaderMaterial.enableAttribute(normalLocation);
        m_normalsBuffer.bind();
        shaderMaterial.setAttributeBuffer(normalLocation, GL_FLOAT, 0, 3, sizeof(QVector3D));
    }

    gl->glDrawElements(GL_TRIANGLES, m_indicesBuffer.size() / static_cast<int>(sizeof(int)), GL_INT, nullptr);

    if (vertexLocation >= 0)
        shaderMaterial.disableAttribute(vertexLocation);
    if (normalLocation >= 0)
        shaderMaterial.disableAttribute(normalLocation);

    m_vertexBuffer.release();
    m_indicesBuffer.release();

    shaderMaterial.release();
}
