#ifndef ANIMATIONSCENE_H
#define ANIMATIONSCENE_H

#include <QSharedPointer>

#include <QMatrix4x4>

#include "gl/gl_scene.h"

class HouseObject;
class TextureReceiver;

class AnimationScene:
        public GL_Scene
{
public:
    AnimationScene(int duration);

    int duration() const;
    int currentTime() const;
    float timeState() const;

    void resetTime();

    bool animationIsFinished() const;

    TextureReceiver * textureReceiver() const;
    void setTextureReceiver(TextureReceiver * textureReceiver);

    QMatrix4x4 viewMatrix() const;
    void setViewMatrix(const QMatrix4x4 & viewMatrix);

    QSharedPointer<HouseObject> house() const;
    void setHouse(const QSharedPointer<HouseObject> & house);

    void init(GL_ViewRenderer * view) override;
    void destroy(GL_ViewRenderer * view) override;

    void drawAndPlay(GL_ViewRenderer * view);

private:
    int m_duration;
    int m_currentTime;

    QSharedPointer<HouseObject> m_house;
    TextureReceiver * m_textureReceiver;
    QMatrix4x4 m_viewMatrix;
};

#endif // ANIMATIONSCENE_H
