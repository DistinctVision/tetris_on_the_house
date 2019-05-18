#include "defaultscene.h"

#include "texturereceiver.h"

DefaultScene::DefaultScene(int duration):
    AnimationScene(duration)
{
}

void DefaultScene::init(GL_ViewRenderer * view)
{
    AnimationScene::init(view);
    m_material = view->createMaterial(MaterialType::ScreenMorph_default);
}

void DefaultScene::destroy(GL_ViewRenderer * view)
{
    m_material.reset();
    AnimationScene::destroy(view);
}

void DefaultScene::draw(GL_ViewRenderer * view)
{
    HouseObjectPtr house = this->house();
    QMatrix4x4 matrixVP = view->projectionMatrix() * viewMatrix();

    view->glDisable(GL_BLEND);
    {
        QVector3D color_a(1.0f, 1.0f, 1.0f);
        QVector3D color_b(0.0f, 0.0f, house->activityLevel() * 0.25f);
        m_material->setValue("matrixMVP", matrixVP);
        m_material->setValue("matrixView2FrameUV", house->matrixView2FrameUV(view, textureReceiver()->textureSize()));
        m_material->setTexture("screen_texture", textureReceiver()->textureId());
        m_material->setValue("color_a", color_a);
        m_material->setValue("color_b", color_b);
        house->meshForward()->draw(view, *m_material);
    }

    view->glEnable(GL_BLEND);
    GL_ShaderMaterialPtr materialGrid = house->materialGrid();
    materialGrid->setValue("matrixMVP", matrixVP);
    materialGrid->setValue("mainColor", QColor(255, 255, 255, 100));
    house->meshGrid()->draw(view, *materialGrid);
}
