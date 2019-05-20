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
    BirdMeshPtr m_mesh;
    GL_ShaderMaterialPtr m_material;

    StateType m_state;
    float m_timeState;
    float m_timeSpeed;

    QVector3D m_position;
    QQuaternion m_orientation;

    float m_velocity;

    QVector3D m_targetPoint;
};

using BirdObjectPtr = QSharedPointer<BirdObject>;

#endif // BIRDOBJECT_H
