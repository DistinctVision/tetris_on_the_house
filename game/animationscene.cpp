#include "animationscene.h"
#include <cmath>

#include "houseobject.h"

AnimationScene::AnimationScene(int duration):
    m_duration(duration),
    m_currentTime(0),
    m_textureReceiver(nullptr)
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

float AnimationScene::timeState() const
{
    return m_currentTime / static_cast<float>(m_duration);
}

void AnimationScene::resetTime()
{
    m_currentTime = 0;
}

bool AnimationScene::animationIsFinished() const
{
    return (m_currentTime >= m_duration);
}

TextureReceiver * AnimationScene::textureReceiver() const
{
    return m_textureReceiver;
}

void AnimationScene::setTextureReceiver(TextureReceiver * textureReceiver)
{
    m_textureReceiver = textureReceiver;
}

QMatrix4x4 AnimationScene::viewMatrix() const
{
    return m_viewMatrix;
}

void AnimationScene::setViewMatrix(const QMatrix4x4 & viewMatrix)
{
    m_viewMatrix = viewMatrix;
}

QSharedPointer<HouseObject> AnimationScene::house() const
{
    return m_house;
}

void AnimationScene::setHouse(const QSharedPointer<HouseObject> & house)
{
    m_house = house;
}

void AnimationScene::init(GL_ViewRenderer * view)
{
    Q_UNUSED(view);
}

void AnimationScene::destroy(GL_ViewRenderer * view)
{
    Q_UNUSED(view);
    m_house.reset();
}

void AnimationScene::drawAndPlay(GL_ViewRenderer * view)
{
    draw(view);
    m_currentTime = std::min(m_currentTime + 1, m_duration);
}
