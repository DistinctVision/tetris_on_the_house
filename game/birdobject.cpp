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
    m_targetPoint(targetPoint)
{
    m_material = view->createMaterial(MaterialType::Morph_fallOff);
    QVector3D delta = targetPoint - position;
    m_orientation = QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 1.0f, 0.0), atan2(delta.z(), delta.x()) * static_cast<float>(180.0 / M_PI));
}

void BirdObject::updateStep()
{
    QVector3D dir = m_orientation.rotatedVector(QVector3D(0.0f, 0.0f, 1.0f));
    m_position += dir * m_velocity;
}

void BirdObject::draw(GL_ViewRenderer * view, const QMatrix4x4 & viewMatrix)
{
    QMatrix4x4 matrixWorld;
    matrixWorld.rotate(m_orientation);
    matrixWorld.translate(m_position);

    QMatrix4x4 matrixViewWorld = viewMatrix * matrixWorld;
    m_material->setValue("matrixMVP", view->projectionMatrix());
    m_material->setValue("matrixMV", matrixViewWorld);

    m_mesh->mesh()->draw(view, *m_material, { { m_mesh->bufferVertices1(), "vertex_morph_position", GL_FLOAT, 0, 3, sizeof(QVector3D) },
                                              { m_mesh->bufferNormals1(), "vertex_morph_normal", GL_FLOAT, 0, 3, sizeof(QVector3D) }});
}


