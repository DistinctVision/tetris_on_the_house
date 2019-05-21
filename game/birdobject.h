#ifndef BIRDOBJECT_H
#define BIRDOBJECT_H

#include <QSharedPointer>
#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>

#include "birdmesh.h"
#include "gl/gl_shadermaterial.h"

class GL_ViewRenderer;

class BirdObject
{
public:
    enum class StateType
    {
        Fly,
        Gliding
    };

    BirdObject(GL_ViewRenderer * view,
               const BirdMeshPtr & mesh,
               const QVector3D & position,
               float velocity,
               const QVector3D & targetPoint);

    void updateStep();

    void draw(GL_ViewRenderer * view, const QMatrix4x4 & viewMatrix);

private:
    const float m_localScale = 2.5f;

    BirdMeshPtr m_mesh;
    GL_ShaderMaterialPtr m_material;

    QVector3D m_position;
    QQuaternion m_orientation;
    QQuaternion m_angularVelocity;

    float m_velocity;

    QVector3D m_targetPoint;

    StateType m_state;
    int m_vertexIndexA;
    int m_vertexIndexB;
    float m_timeState;
    float m_timeSpeed;

    QQuaternion _getRotation(const QVector3D & targetPoint) const;
};

using BirdObjectPtr = QSharedPointer<BirdObject>;

#endif // BIRDOBJECT_H
