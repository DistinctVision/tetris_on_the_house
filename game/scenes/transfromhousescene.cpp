#include "transfromhousescene.h"

#include <QtMath>

#include "texturereceiver.h"

TransfromHouseScene::TransfromHouseScene(int duration, float deltaZ):
    AnimationScene(duration),
    m_deltaZ(deltaZ)
{
}

void TransfromHouseScene::init(GL_ViewRenderer * view)
{
    AnimationScene::init(view);
    m_material = view->createMaterial(MaterialType::ScreenMorph_transform);
}

void TransfromHouseScene::destroy(GL_ViewRenderer * view)
{
    m_material.reset();
    AnimationScene::destroy(view);
}

void TransfromHouseScene::draw(GL_ViewRenderer * view)
{
    float time = this->timeState();

    HouseObjectPtr house = this->house();
    QMatrix4x4 matrixVP = view->projectionMatrix() * viewMatrix();

    QMatrix4x4 matrixTransform;
    {
        float t = fabs(sin(time * static_cast<float>(M_PI) * 2.0f));
        float k = (1.0f - t) * 1.0f + t * m_deltaZ;
        matrixTransform(2, 2) = k;
        matrixTransform(2, 3) = (1.0f - k) * 4.0f;
    }

    view->glDisable(GL_BLEND);
    {
        QVector3D color_a(1.0f, 1.0f, 1.0f);
        QVector3D color_b(0.0f, 0.0f, house->activityLevel() * 0.25f);
        m_material->setValue("matrixMVP", matrixVP);
        m_material->setValue("matrixMVP_transform", matrixVP * matrixTransform);
        m_material->setValue("matrixView2FrameUV", house->matrixView2FrameUV(view, textureReceiver()->textureSize()));
        m_material->setTexture("screen_texture", textureReceiver()->textureId());
        m_material->setValue("color_a", color_a);
        m_material->setValue("color_b", color_b);
        house->meshHouse()->draw(view, *m_material);
    }

    view->glEnable(GL_BLEND);
    GL_ShaderMaterialPtr materialGrid = house->materialGrid();
    materialGrid->setValue("matrixMVP", matrixVP);
    materialGrid->setValue("mainColor", QColor(255, 255, 255, 100));
    house->meshGrid()->draw(view, *materialGrid);
}
