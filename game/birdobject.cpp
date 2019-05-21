#include "birdobject.h"

#include <QtMath>
#include <cmath>

#include "gl/gl_view.h"

using namespace std;

BirdObject::BirdObject(GL_ViewRenderer * view,
                       const BirdMeshPtr & mesh,
                       const QVector3D & position,
                       float velocity,
                       const QVector3D & targetPoint):
    m_mesh(mesh),
    m_position(position),
    m_velocity(velocity),
    m_targetPoint(targetPoint),
    m_state(StateType::Fly),
    m_vertexIndexA(0),
    m_vertexIndexB(0),
    m_timeState(0.0f),
    m_timeSpeed(0.1f)
{
    m_material = view->createMaterial(MaterialType::Morph_fallOff);
    QVector3D delta = targetPoint - position;
    m_orientation = QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 1.0f, 0.0), - atan2(delta.z(), delta.x()) * static_cast<float>(180.0 / M_PI));
}

void BirdObject::updateStep()
{
    QVector3D dir = m_orientation.rotatedVector(QVector3D(0.0f, 0.0f, 1.0f));
    m_position += dir * m_velocity;

    if (m_vertexIndexA == m_vertexIndexB)
    {
        m_timeState = 1.0f;
    }
    else
    {
        m_timeState += m_timeSpeed;
    }

    if (m_timeState >= 1.0f)
    {
        switch (m_state) {
        case StateType::Gliding:
            m_vertexIndexA = m_vertexIndexB;
            m_vertexIndexB = 0;
            break;
        case StateType::Fly:
            if (m_vertexIndexB == 0)
            {
                if (m_vertexIndexA == 2)
                {
                    m_vertexIndexA = 0;
                    m_vertexIndexB = 1;
                }
                else
                {
                    m_vertexIndexA = 0;
                    m_vertexIndexB = 2;
                }
            }
            else if (m_vertexIndexB == 1)
            {
                m_vertexIndexA = 1;
                m_vertexIndexB = 0;
            }
            else
            {
                m_vertexIndexA = 2;
                m_vertexIndexB = 0;
            }
            break;
        }
        m_timeState = m_timeState - floor(m_timeState);
    }
}

void BirdObject::draw(GL_ViewRenderer * view, const QMatrix4x4 & viewMatrix)
{
    QMatrix4x4 matrixWorld;
    matrixWorld.rotate(m_orientation);
    matrixWorld.translate(m_position);
    QMatrix4x4 matrixScale;
    //matrixScale.scale(2.5f);
    matrixScale.scale(5.0f);
    matrixWorld = matrixWorld * matrixScale;

    QMatrix4x4 matrixViewWorld = viewMatrix * matrixWorld;
    m_material->setValue("matrixMVP", view->projectionMatrix() * matrixViewWorld);
    m_material->setValue("matrixMV", matrixViewWorld);

    GL_MeshPtr mesh;

    if (m_vertexIndexA == 0)
    {
        mesh = m_mesh->mesh();
    }
    else if (m_vertexIndexA == 1)
    {
        mesh = GL_MeshPtr::create(GL_Mesh::createMesh(m_mesh->verticesBuffer1(),
                                                      m_mesh->mesh()->textureCoordsBuffer(),
                                                      m_mesh->normalsBuffer1(),
                                                      m_mesh->mesh()->indicesBuffer(),
                                                      m_mesh->mesh()->numberElements()));
    }
    else
    {
        mesh = GL_MeshPtr::create(GL_Mesh::createMesh(m_mesh->verticesBuffer2(),
                                                      m_mesh->mesh()->textureCoordsBuffer(),
                                                      m_mesh->normalsBuffer2(),
                                                      m_mesh->mesh()->indicesBuffer(),
                                                      m_mesh->mesh()->numberElements()));
    }

    float time;

    QOpenGLBuffer vertices2, normals2;
    if (m_vertexIndexB == 0)
    {
        vertices2 = m_mesh->mesh()->verticesBuffer();
        normals2 = m_mesh->mesh()->normalsBuffer();
        time = pow(m_timeState, 1.5f);
    }
    else if (m_vertexIndexB == 1)
    {
        vertices2 = m_mesh->verticesBuffer1();
        normals2 = m_mesh->normalsBuffer1();
        time = pow(m_timeState, 1.5f);
    }
    else
    {
        vertices2 = m_mesh->verticesBuffer2();
        normals2 = m_mesh->normalsBuffer2();
        time = m_timeState * m_timeState;
        time = 1.0f - pow(1.0f - m_timeState, 1.5f);
    }

    m_material->setValue("time", time);
    mesh->draw(view, *m_material, { { vertices2, "vertex_morph_position", GL_FLOAT, 0, 3, sizeof(QVector3D) },
                                    { normals2, "vertex_morph_normal", GL_FLOAT, 0, 3, sizeof(QVector3D) } });
}


