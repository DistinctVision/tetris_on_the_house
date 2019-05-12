#ifndef GL_MESH_H
#define GL_MESH_H

#include <QSharedPointer>

#include <QOpenGLFunctions>
#include <QOpenGLBuffer>

#include <QVector>
#include <QVector2D>
#include <QVector3D>

QString glErrorToQString(GLenum error);

void gl_assert(QOpenGLFunctions * gl);

class GL_ShaderMaterial;

class GL_Mesh
{
public:
    static QVector<QVector3D> computeNormals(const QVector<QVector3D> & vertices,
                                             const QVector<GLuint> & indices);

    static GL_Mesh createQuad(const QVector2D & size = QVector2D(1.0f, 1.0f));
    static GL_Mesh createCube(const QVector3D & size = QVector3D(1.0f, 1.0f, 1.0f));
    static GL_Mesh createCubikRubik(float border = 0.075f);
    static GL_Mesh createMesh(const QVector<QVector3D> & vertices,
                              const QVector<QVector2D> & textureCoords,
                              const QVector<GLuint> & indices);

    void updateVertices(const QVector<QVector3D> & vertices, bool updateNormals = false);
    void updateTextureCoords(const QVector<QVector2D> & textureCoords);

    void draw(QOpenGLFunctions * gl, const GL_ShaderMaterial & shaderMaterial);

private:
    GL_Mesh();

    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_textureCoordsBuffer;
    QOpenGLBuffer m_normalsBuffer;
    QOpenGLBuffer m_indicesBuffer;
    GLsizei m_numberElements;
};

using GL_MeshPtr = QSharedPointer<GL_Mesh>;

#endif // GL_MESH_H
