#ifndef TETRISSCENE_H
#define TETRISSCENE_H

#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLFramebufferObject>

#include "gl/gl_scene.h"
#include "gl/gl_worldobject.h"
#include "gl/gl_mesh.h"
#include "gl/gl_shadermaterial.h"
#include "houseobject.h"

#include "animationscene.h"

class ObjectEdgesTracker;
class TextureReceiver;
class TetrisGame;

class TetrisScene:
        public GL_Scene
{
    Q_OBJECT

    Q_PROPERTY(ObjectEdgesTracker* objectEdgesTracker READ objectEdgesTracker WRITE setObjectEdgesTracker
               NOTIFY objectEdgesTrackerChanged)
    Q_PROPERTY(TextureReceiver* textureReceiver READ textureReceiver WRITE setTextureReceiver
               NOTIFY textureReceiverChanged)
public:
    TetrisScene();

    ObjectEdgesTracker * objectEdgesTracker() const;
    void setObjectEdgesTracker(ObjectEdgesTracker * objectEdgesTracker);

    TextureReceiver * textureReceiver() const;
    void setTextureReceiver(TextureReceiver * textureReceiver);

    void init(GL_ViewRenderer * view);
    void destroy(GL_ViewRenderer * view);
    void draw(GL_ViewRenderer * view);

    Q_INVOKABLE bool moveFigureLeft();
    Q_INVOKABLE bool moveFigureRight();
    Q_INVOKABLE bool moveFigureDown();
    Q_INVOKABLE bool rotateFigure();

signals:
    void objectEdgesTrackerChanged();
    void textureReceiverChanged();

private:
    ObjectEdgesTracker * m_tracker;
    TextureReceiver * m_textureReceiver;

    QSharedPointer<AnimationScene> m_startScene;

    HouseObjectPtr m_house;
    GL_ShaderMaterialPtr m_houseDefaultMaterial;
    QVector3D m_houseColorK_a;
    QVector3D m_houseColorK_b;
    GL_MeshPtr m_blockMesh;
    GL_ShaderMaterialPtr m_blockMaterial;

    QSharedPointer<TetrisGame> m_game;

    QOpenGLFramebufferObject * m_glowBuffer;
    QOpenGLFramebufferObject * m_tempGlowBuffer;

    void _drawBlocks(GL_ViewRenderer * view, const QMatrix4x4 & projMatrix, const QMatrix4x4 & viewMatrix);

    void _drawGlow(GL_ViewRenderer * view);
};

#endif // TETRISSCENE_H
