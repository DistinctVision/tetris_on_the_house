#include "gl_mesh.h"

#include <QVector>

#include "gl_shadermaterial.h"

GL_Mesh::GL_Mesh():
    m_vertexBuffer(QOpenGLBuffer::VertexBuffer),
    m_textureCoordsBuffer(QOpenGLBuffer::VertexBuffer),
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
    QVector<QVector2D> textureCoords = {
        QVector2D(0.0f, 0.0),
        QVector2D(1.0f, 0.0f),
        QVector2D(1.0f, 1.0f),
        QVector2D(0.0f, 1.0f)
    };
    QVector<int> indices = {
        0, 1, 2,
        0, 2, 3
    };
    mesh.m_vertexBuffer.bind();
    mesh.m_vertexBuffer.allocate(vertices.data(), static_cast<int>(sizeof(QVector3D)) * vertices.size());
    mesh.m_textureCoordsBuffer.bind();
    mesh.m_textureCoordsBuffer.allocate(textureCoords.data(), static_cast<int>(sizeof(QVector2D)) * textureCoords.size());
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
    QVector<QVector3D> vertices;
    QVector<QVector2D> textureCoords;
    QVector<int> indices;
    vertices.reserve(4 * 6);
    textureCoords.resize(vertices.size());
    indices.reserve(6 * 2 * 3);

    QVector3D axisX(1.0f, 0.0f, 0.0f),
              axisY(0.0f, 1.0f, 0.0f),
              axisZ(0.0f, 0.0f, 1.0f);

    auto addSide = [&] ()
    {
        int i_o = static_cast<int>(vertices.size());
        vertices.push_back((- halfSize.x()) * axisX + (- halfSize.y()) * axisY + halfSize.z() * axisZ);
        vertices.push_back((halfSize.x()) * axisX + (- halfSize.y()) * axisY + halfSize.z() * axisZ);
        vertices.push_back((halfSize.x()) * axisX + (halfSize.y()) * axisY + halfSize.z() * axisZ);
        vertices.push_back((- halfSize.x()) * axisX + (halfSize.y()) * axisY + halfSize.z() * axisZ);
        textureCoords.push_back(QVector2D(0.0f, 0.0f));
        textureCoords.push_back(QVector2D(1.0f, 0.0f));
        textureCoords.push_back(QVector2D(1.0f, 1.0f));
        textureCoords.push_back(QVector2D(0.0f, 1.0f));
        indices.append({ i_o + 0, i_o + 1, i_o + 2 });
        indices.append({ i_o + 0, i_o + 2, i_o + 3 });
    };

    for (int k = 0; k < 4; ++k)
    {
        addSide();
        std::swap(axisZ, axisY);
        axisZ = - axisZ;
    }

    axisX = QVector3D(0.0f, 0.0f, 1.0f);
    axisY = QVector3D(0.0f, 1.0f, 0.0f);
    axisZ = QVector3D(-1.0f, 0.0f, 0.0f);
    addSide();

    axisX = - axisX;
    axisZ = - axisZ;
    addSide();

    QVector<QVector3D> normals = computeNormals(vertices, indices);

    mesh.m_indicesBuffer.bind();
    mesh.m_indicesBuffer.allocate(vertices.data(), static_cast<int>(sizeof(int)) * indices.size());
    mesh.m_normalsBuffer.bind();
    mesh.m_normalsBuffer.allocate(normals.data(), static_cast<int>(sizeof(QVector3D)) * normals.size());
    mesh.m_normalsBuffer.release();
    return mesh;
}

void GL_Mesh::draw(QOpenGLFunctions * gl, const GL_ShaderMaterial & shaderMaterial)
{
    shaderMaterial.bind();

    m_indicesBuffer.bind();

    int vertexLocation = shaderMaterial.attributeLocation("vertex_position");
    int textureCoordLocation = shaderMaterial.attributeLocation("vertex_textureCoord");
    int normalLocation = shaderMaterial.attributeLocation("vertex_normal");

    if (vertexLocation >= 0)
    {
        shaderMaterial.enableAttribute(vertexLocation);
        m_vertexBuffer.bind();
        shaderMaterial.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(QVector3D));
    }
    if (textureCoordLocation >= 0)
    {
        shaderMaterial.enableAttribute(textureCoordLocation);
        m_textureCoordsBuffer.bind();
        shaderMaterial.setAttributeBuffer(textureCoordLocation, GL_FLOAT, 0, 2, sizeof(QVector2D));
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
    if (textureCoordLocation >= 0)
        shaderMaterial.disableAttribute(textureCoordLocation);
    if (normalLocation >= 0)
        shaderMaterial.disableAttribute(normalLocation);

    m_vertexBuffer.release();
    m_indicesBuffer.release();

    shaderMaterial.release();
}
