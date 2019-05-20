#ifndef BIRDMESH_H
#define BIRDMESH_H

#include <QOpenGLBuffer>

#include "gl/gl_mesh.h"

#include <QSharedPointer>
#include <QVector>
#include <QVector2D>
#include <QVector3D>

class BirdMesh
{
public:
    BirdMesh();

    bool load(const QString & path,
              const QString & path1,
              const QString & path2);

private:
    struct _ModelData
    {
        QVector<QVector3D> vertices;
        QVector<GLuint> indices;
    };

    GL_MeshPtr m_meshBird;

    QOpenGLBuffer m_bufferVertices1;
    QOpenGLBuffer m_bufferNormals1;

    QOpenGLBuffer m_bufferVertices2;
    QOpenGLBuffer m_bufferNormals2;

    bool _loadObject(_ModelData & data, const QString & path) const;
};

using BirdMeshPtr = QSharedPointer<BirdMesh>;

#endif // BIRDMESH_H
