#include "gamestartscene.h"

#include "game/houseobject.h"
#include "texturereceiver.h"

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

    HouseObjectPtr house = this->house();
    QMatrix4x4 matrixVP = view->projectionMatrix() * viewMatrix();

    view->glDisable(GL_BLEND);
    {
        QVector3D color_a(1.0f, time, 1.0f);
        QVector3D color_b(0.0f, 0.0f, house->activityLevel() * 0.25f);
        m_material->setValue("matrixMVP", matrixVP);
        m_material->setValue("matrixView2FrameUV", house->matrixView2FrameUV(view,
                                                                             textureReceiver()->textureSize(),
                                                                             textureReceiver()->orientation()));
        m_material->setTexture("screen_texture", textureReceiver()->textureId());
        m_material->setValue("color_a", color_a);
        m_material->setValue("color_b", color_b);
        float time_edge = min(max(0.5f - fabs(time * 2.0f - 1.0f), 0.0f), 1.0f);
        time_edge *= time_edge;
        m_material->setValue("edges_size", time_edge * 0.25f);
        house->meshHouse()->draw(view, *m_material);
    }

    view->glEnable(GL_BLEND);
    {
        float time_grid = min(max((time - 0.5f) * 2.0f, 0.0f), 1.0f);
        GL_ShaderMaterialPtr materialGrid = house->materialGrid();
        QMatrix4x4 worldGridMatrix;
        worldGridMatrix(2, 2) = time_grid;
        worldGridMatrix(2, 3) = house->grid_end().z() * (1.0f - time_grid);
        materialGrid->setValue("matrixMVP", matrixVP * worldGridMatrix);
        materialGrid->setValue("mainColor", QColor(255, 255, 255, static_cast<int>(100 * time_grid)));
        house->meshGrid()->draw(view, *materialGrid);
    }
}
