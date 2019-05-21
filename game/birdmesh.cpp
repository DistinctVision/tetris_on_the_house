#include "birdmesh.h"

#include <QtDebug>

#include <QFile>
#include <QByteArray>

BirdMesh::BirdMesh()
{
}

bool BirdMesh::load(const QString & path,
                    const QString & path1,
                    const QString & path2)
{
    _ModelData data, data1, data2;
    if (!_loadObject(data, path))
    {
        qFatal(QString("Coudn't load %1").arg(path).toStdString().c_str());
        return false;
    }
    if (!_loadObject(data1, path1))
    {
        qFatal(QString("Coudn't load %1").arg(path1).toStdString().c_str());
        return false;
    }
    if (!_loadObject(data2, path2))
    {
        qFatal(QString("Coudn't load %1").arg(path2).toStdString().c_str());
        return false;
    }
    if ((data1.vertices.size() != data.vertices.size()) ||
            (data1.indices.size() != data.indices.size()) ||
            (data2.vertices.size() != data.vertices.size()) ||
            (data2.indices.size() != data.indices.size()))
    {
        qFatal(QString("Coudn't load %1").arg(path1).toStdString().c_str());
        return false;
    }

    QVector<QVector2D> texCoords(data.vertices.size(), QVector2D(0.0f, 0.0f));
    m_mesh = GL_MeshPtr::create(GL_Mesh::createMesh(data.vertices, texCoords, data.indices));

    QVector<QVector3D> normals1 = GL_Mesh::computeNormals(data1.vertices, data1.indices);
    QVector<QVector3D> normals2 = GL_Mesh::computeNormals(data2.vertices, data2.indices);

    m_bufferVertices1.create();
    m_bufferVertices1.bind();
    m_bufferVertices1.allocate(data1.vertices.data(), static_cast<int>(sizeof(QVector3D)) * data1.vertices.size());

    m_bufferNormals1.create();
    m_bufferNormals1.bind();
    m_bufferNormals1.allocate(normals1.data(), static_cast<int>(sizeof(QVector3D)) * normals1.size());

    m_bufferVertices2.create();
    m_bufferVertices2.bind();
    m_bufferVertices2.allocate(data2.vertices.data(), static_cast<int>(sizeof(QVector3D)) * data2.vertices.size());

    m_bufferNormals2.create();
    m_bufferNormals2.bind();
    m_bufferNormals2.allocate(normals2.data(), static_cast<int>(sizeof(QVector3D)) * normals2.size());

    return true;
}

GL_MeshPtr BirdMesh::mesh() const
{
    return m_mesh;
}

QOpenGLBuffer BirdMesh::verticesBuffer1() const
{
    return m_bufferVertices1;
}

QOpenGLBuffer BirdMesh::normalsBuffer1() const
{
    return m_bufferNormals1;
}

QOpenGLBuffer BirdMesh::verticesBuffer2() const
{
    return m_bufferVertices2;
}

QOpenGLBuffer BirdMesh::normalsBuffer2() const
{
    return m_bufferNormals2;
}

bool BirdMesh::_loadObject(BirdMesh::_ModelData & data, const QString & path) const
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    QStringList strList = QString::fromUtf8(file.readAll()).split('\n');

    data = _ModelData();

    QVector<QVector3D> vertices;

    for (const QString & str : strList)
    {
        QStringList l = str.split(" ");
        if (l.size() < 4)
            continue;
        if (l[0] == "v")
        {
            QVector3D v(l[1].toFloat(), l[2].toFloat(), l[3].toFloat());
            vertices.push_back(v);
        }
    }


    for (const QString & str : strList)
    {
        QStringList l = str.split(" ");
        if (l.size() < 4)
            continue;
        if (l[0] == "f")
        {
            GLuint v_o = static_cast<GLuint>(data.vertices.size());
            QVector<GLuint> f(l.size() - 1);
            for (int i = 0; i < f.size(); ++i)
            {
                QStringList fl = l[i+1].split("/");
                data.vertices.push_back(vertices[fl[0].toUInt() - 1]);
                f[i] = v_o + i;
            }
            while (f.size() >= 3)
            {
                data.indices.append({ f[0], f[1], f[2] });
                f.removeAt(1);
            }
        }
    }

    return true;
}
