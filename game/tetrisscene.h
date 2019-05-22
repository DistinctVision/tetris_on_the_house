#ifndef TETRISSCENE_H
#define TETRISSCENE_H

#include <random>

#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLFramebufferObject>

#include "gl/gl_scene.h"
#include "gl/gl_worldobject.h"
#include "gl/gl_mesh.h"
#include "gl/gl_shadermaterial.h"
#include "houseobject.h"
#include "scenes/animationscene.h"

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
    Q_PROPERTY(bool started READ started NOTIFY startedChanged)
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

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    bool started() const;

signals:
    void objectEdgesTrackerChanged();
    void textureReceiverChanged();
    void startedChanged();

private:
    bool m_started;

    ObjectEdgesTracker * m_tracker;
    TextureReceiver * m_textureReceiver;

    QSharedPointer<AnimationScene> m_startScene;
    QSharedPointer<AnimationScene> m_currentScene;

    HouseObjectPtr m_house;

    QSharedPointer<TetrisGame> m_game;
    int m_numberRemovalLines;
    float m_linesLevel;

    QOpenGLFramebufferObject * m_glowBuffer;
    QOpenGLFramebufferObject * m_tempGlowBuffer;

    mutable std::mt19937 m_rnd_gen;
    mutable std::uniform_int_distribution<int> m_rnd;

    QSharedPointer<AnimationScene> _createRandomScene() const;

    void _drawGlow(GL_ViewRenderer * view);
};

#endif // TETRISSCENE_H
