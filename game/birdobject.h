#ifndef BIRDOBJECT_H
#define BIRDOBJECT_H

#include <QVector3D>

#include "birdmesh.h"

class BirdObject
{
public:
    enum class StateType
    {
        Fly,
        Gliding
    };

    BirdObject(const BirdMeshPtr & meshBird);

private:
    BirdMeshPtr m_meshBird;

    StateType m_state;
    float m_timeState;
    float m_timeSpeed;

    QVector3D m_position;
    QVector3D m_dir;
    float m_velocity;

    QVector3D m_targetPoint;
};

#endif // BIRDOBJECT_H
