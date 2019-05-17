#include "gamestartscene.h"
#include "houseobject.h"
#include "texturereceiver.h"

#include <QVector3D>

using namespace std;

GameStartScene::GameStartScene(int duration):
    AnimationScene(duration)
{
}

void GameStartScene::init(GL_ViewRenderer * view)
{
    AnimationScene::init(view);
    m_material = view->createMaterial(MaterialType::ScreenMorph_glowEdges);
}

void GameStartScene::destroy(GL_ViewRenderer * view)
{
    Q_UNUSED(view);
    m_material.reset();
    AnimationScene::destroy(view);
}

void GameStartScene::draw(GL_ViewRenderer * view)
{
    float time = this->timeState();

    QVector3D color_a(1.0f, time, 1.0f);
    QVector3D color_b(0.0f, 0.0f, 0.0f);

    m_material->setValue("matrixMVP", view->projectionMatrix() * viewMatrix());
    m_material->setValue("matrixView2FrameUV", house()->matrixView2FrameUV(view, textureReceiver()->textureSize()));
    m_material->setTexture("screen_texture", textureReceiver()->textureId());
    m_material->setValue("color_a", color_a);
    m_material->setValue("color_b", color_b);
    float timeEdge = min(max(0.5f - fabs(time * 2.0f - 1.0f), 0.0f), 1.0f);
    timeEdge *= timeEdge;
    m_material->setValue("edges_size", timeEdge * 0.25f);
    house()->meshForward()->draw(view, *m_material);
}
