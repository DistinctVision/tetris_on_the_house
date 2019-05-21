#include "birdobject.h"

#include <QtMath>
#include <cmath>
#include <random>
#include <chrono>

#include "gl/gl_view.h"

using namespace std;
using namespace std::chrono;

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
    m_orientation = _getRotation(targetPoint);

    int time = rand() % 100;
    while (time > 0)
    {
        _updateAnimation();
        --time;
    }
}

void BirdObject::updateStep()
{
    QVector3D dir = m_orientation.rotatedVector(QVector3D(1.0f, 0.0f, 0.0f));
    m_position += dir * m_velocity;

    m_orientation = (m_angularVelocity * m_orientation).normalized();
    m_angularVelocity = QQuaternion::slerp(m_angularVelocity, _getRotation(m_targetPoint) * m_orientation.conjugated(), 0.001f);

    _updateAnimation();
}

void BirdObject::draw(GL_ViewRenderer * view, const QMatrix4x4 & viewMatrix)
{
    QMatrix4x4 matrixRotation;
    matrixRotation.rotate(m_orientation);
    QMatrix4x4 matrixTranslation;
    matrixTranslation.translate(m_position);
    QMatrix4x4 matrixScale;
    matrixScale.scale(m_localScale);
    QMatrix4x4 matrixWorld = matrixTranslation * matrixRotation * matrixScale;

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

QQuaternion BirdObject::_getRotation(const QVector3D & targetPoint) const
{
    QVector3D localPoint = targetPoint - m_position;
    float yAngle = atan2(localPoint.z(), localPoint.x()) * static_cast<float>(180.0 / M_PI);
    QQuaternion q1 = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, - yAngle);

    localPoint = q1.conjugated().rotatedVector(localPoint);
    float xAngle = atan2(localPoint.y(), localPoint.x()) * static_cast<float>(180.0 / M_PI);
    QQuaternion q2 = QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, xAngle) * q1;

    return q2;
}

void BirdObject::_updateAnimation()
{
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
    else
    {
        if (m_vertexIndexA == 1)
        {
            m_angularVelocity = m_angularVelocity * QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, 0.03f);
        }
        else if (m_vertexIndexA == 2)
        {
            m_angularVelocity = m_angularVelocity * QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, -0.03f);
        }
    }
}
