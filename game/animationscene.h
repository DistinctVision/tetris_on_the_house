#ifndef ANIMATIONSCENE_H
#define ANIMATIONSCENE_H

#include "gl/gl_scene.h"

class AnimationScene:
        public GL_Scene
{
public:
    AnimationScene(int duration);

    int duration() const;
    int currentTime() const;

    void resetTime();

    bool animationIsFinished() const;

    void drawAndPlay(GL_ViewRenderer * view);

private:
    int m_duration;
    int m_currentTime;
};

#endif // ANIMATIONSCENE_H
