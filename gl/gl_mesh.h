#ifndef GL_MESH_H
#define GL_MESH_H

#include <QOpenGLFunctions>
#include <QOpenGLBuffer>

#include <QVector>
#include <QVector2D>
#include <QVector3D>

class GL_ShaderMaterial;

class GL_Mesh
{
public:
    static QVector<QVector3D> computeNormals(const QVector<QVector3D> & vertices,
                                             const QVector<int> & indices);

    static GL_Mesh createQuad(const QVector2D & size);
    static GL_Mesh createCube(const QVector3D & size);

    void draw(QOpenGLFunctions * gl, const GL_ShaderMaterial & shaderMaterial);

private:
    GL_Mesh();

    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_normalsBuffer;
    QOpenGLBuffer m_indicesBuffer;

};

#endif // GL_MESH_H
