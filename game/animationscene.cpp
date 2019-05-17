#include "animationscene.h"
#include <cmath>

AnimationScene::AnimationScene(int duration):
    m_duration(duration),
    m_currentTime(0)
{
}

int AnimationScene::duration() const
{
    return m_duration;
}

int AnimationScene::currentTime() const
{
    return m_currentTime;
}

void AnimationScene::resetTime()
{
    m_currentTime = 0;
}

bool AnimationScene::animationIsFinished() const
{
    return (m_currentTime == m_duration);
}

void AnimationScene::drawAndPlay(GL_ViewRenderer * view)
{
    draw(view);
    m_currentTime = std::min(m_currentTime + 1, m_duration);
}
