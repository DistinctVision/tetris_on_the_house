#include "finalscene.h"

#include <cmath>
#include <QtMath>

#include "texturereceiver.h"

using namespace std;

FinalScene::FinalScene(int duration):
    AnimationScene(duration)
{
}

void FinalScene::init(GL_ViewRenderer * view)
{
    AnimationScene::init(view);
    m_materialHouse = view->createMaterial(MaterialType::ScreenMorph_default);
    m_materialTransform = view->createMaterial(MaterialType::ScreenMorph_transform);
}

void FinalScene::destroy(GL_ViewRenderer * view)
{
    m_materialHouse.reset();
    m_materialTransform.reset();
    AnimationScene::destroy(view);
}

void FinalScene::draw(GL_ViewRenderer * view)
{
    float time = this->timeState();

    HouseObjectPtr house = this->house();
    QMatrix4x4 matrixVP = view->projectionMatrix() * viewMatrix();

    QVector3D color_a(1.0f, min(max((1.0f - time) / 0.1f, 0.0f), 1.0f), 1.0f);
    QVector3D color_b(0.0f, 0.0f, house->activityLevel() * 0.25f);
    view->glDisable(GL_BLEND);
    {
        m_materialHouse->setValue("matrixMVP", matrixVP);
        m_materialHouse->setValue("matrixView2FrameUV",
                                  house->matrixView2FrameUV(view, textureReceiver()->textureSize()));
        m_materialHouse->setTexture("screen_texture", textureReceiver()->textureId());
        m_materialHouse->setValue("color_a", color_a);
        m_materialHouse->setValue("color_b", color_b);
        house->meshHouse_wo_doors()->draw(view, *m_materialHouse);
    }

    float time_for_doors = (time - 0.1f) / 0.2f;
    if ((time_for_doors >= 0.0f) && (time_for_doors <= 1.0f))
    {
        const float delta_x = 14.0f;
        const float delta_z = 10.0f;

        QMatrix4x4 worldMatrixForLeftDoor;
        QMatrix4x4 worldMatrixForRightDoor;

        if (time_for_doors < 0.5f)
        {
            float t = (time_for_doors * 2.0f);
            t *= t;
            worldMatrixForLeftDoor(2, 3) = delta_z * t;
            worldMatrixForRightDoor(2, 3) = delta_z * t;
        }
        else
        {
            float t = ((time_for_doors - 0.5f) * 2.0f);
            t *= t;
            worldMatrixForLeftDoor(0, 3) = - t * delta_x;
            worldMatrixForLeftDoor(2, 3) = delta_z;
            worldMatrixForRightDoor(0, 3) = t * delta_x;
            worldMatrixForRightDoor(2, 3) = delta_z;
        }
        m_materialTransform->setValue("matrixView2FrameUV",
                                      house->matrixView2FrameUV(view, textureReceiver()->textureSize()));
        m_materialTransform->setTexture("screen_texture", textureReceiver()->textureId());
        m_materialTransform->setValue("color_a", color_a);
        m_materialTransform->setValue("color_b", color_b);
        m_materialTransform->setValue("matrixMVP", matrixVP);
        m_materialTransform->setValue("matrixMVP_transform", matrixVP * worldMatrixForLeftDoor);
        house->meshLeftDoor()->draw(view, *m_materialTransform);
        m_materialTransform->setValue("matrixMVP_transform", matrixVP * worldMatrixForRightDoor);
        house->meshRightDoor()->draw(view, *m_materialTransform);
    }

    if (time < 0.1f)
    {
        view->glEnable(GL_BLEND);
        float time_grid = min(max(time / 0.1f, 0.0f), 1.0f);
        GL_ShaderMaterialPtr materialGrid = house->materialGrid();
        QMatrix4x4 worldGridMatrix;
        worldGridMatrix(2, 2) = 1.0f - time_grid;
        worldGridMatrix(2, 3) = house->grid_end().z() * time_grid;
        materialGrid->setValue("matrixMVP", matrixVP * worldGridMatrix);
        materialGrid->setValue("mainColor", QColor(255, 255, 255, static_cast<int>(100 * (1.0f - time_grid))));
        house->meshGrid()->draw(view, *materialGrid);
    }
}
