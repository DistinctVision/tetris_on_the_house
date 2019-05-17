#ifndef HOUSEOBJECT_H
#define HOUSEOBJECT_H

#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>
#include <QSharedPointer>

#include <Eigen/Eigen>

#include "gl/gl_view.h"
#include "gl/gl_mesh.h"
#include "gl/gl_shadermaterial.h"
#include "gl/gl_screenobject.h"

class TetrisGame;

class HouseObject
{
public:
    HouseObject(GL_ViewRenderer * view,
                const Eigen::Vector3i & grid_n_size,
                const Eigen::Vector3f & grid_begin,
                const Eigen::Vector3f & grid_end);

    Eigen::Vector3i grid_n_size() const;
    Eigen::Vector3f grid_begin() const;
    Eigen::Vector3f grid_end() const;

    float activityLevel() const;
    void setActivityLevel(float activityLevel);

    GL_MeshPtr meshForward() const;
    GL_MeshPtr meshGrid() const;
    GL_ShaderMaterialPtr materialGrid() const;
    GL_MeshPtr meshBlock() const;
    GL_ShaderMaterialPtr materialBlock() const;

    QMatrix4x4 matrixView2FrameUV(GL_ViewRenderer * view, const QSize & frameTextureSize) const;

    void drawBlocks(GL_ViewRenderer * view, const TetrisGame * game, const QMatrix4x4 & viewMatrix);

private:
    struct _FloorInfo
    {
        float y;
        QVector<GLuint> i_sides[4];
    };

    Eigen::Vector3i m_grid_n_size;
    Eigen::Vector3f m_grid_begin;
    Eigen::Vector3f m_grid_end;

    float m_activityLevel;

    GL_MeshPtr m_meshForward;
    GL_MeshPtr m_meshGrid;
    GL_ShaderMaterialPtr m_materialGrid;
    GL_MeshPtr m_meshBlock;
    GL_ShaderMaterialPtr m_materialBlock;

    GL_ScreenObjectPtr m_screenTempObject;

    void _createMeshForward();
    void _createMeshGrid(float border);
};

using HouseObjectPtr = QSharedPointer<HouseObject>;

#endif // HOUSEOBJECT_H
