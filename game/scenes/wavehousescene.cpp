#include "wavehousescene.h"

#include <QtMath>

#include "texturereceiver.h"

WaveHouseScene::WaveHouseScene(int duration,
                               const QVector3D & wave_origin,
                               float wave_timeScale,
                               float wave_distanceStep,
                               float wave_distanceStepScale,
                               float wave_scale):
    AnimationScene(duration),
    m_wave_origin(wave_origin),
    m_wave_timeScale(wave_timeScale),
    m_wave_distanceStep(wave_distanceStep),
    m_wave_distanceStepScale(wave_distanceStepScale),
    m_wave_scale(wave_scale)
{
}

void WaveHouseScene::init(GL_ViewRenderer * view)
{
    AnimationScene::init(view);
    m_material = view->createMaterial(MaterialType::ScreenMorph_wave);
}

void WaveHouseScene::destroy(GL_ViewRenderer * view)
{
    m_material.reset();
    AnimationScene::destroy(view);
}

void WaveHouseScene::draw(GL_ViewRenderer * view)
{
    float time = this->timeState();

    HouseObjectPtr house = this->house();
    QMatrix4x4 matrixVP = view->projectionMatrix() * viewMatrix();

    view->glDisable(GL_BLEND);
    {
        QVector3D color_a(1.0f, 1.0f, 1.0f);
        QVector3D color_b(0.0f, 0.0f, house->activityLevel() * 0.25f);
        m_material->setValue("matrixMVP", matrixVP);
        m_material->setValue("wave_time", time);
        m_material->setValue("wave_timeScale", m_wave_timeScale);
        m_material->setValue("wave_origin", m_wave_origin);
        m_material->setValue("wave_distanceStep", m_wave_distanceStep);
        m_material->setValue("wave_distanceStepScale", m_wave_distanceStepScale);
        m_material->setValue("wave_scale", m_wave_scale * sin(time * static_cast<float>(M_PI)));
        m_material->setValue("matrixView2FrameUV", house->matrixView2FrameUV(view,
                                                                             textureReceiver()->textureSize(),
                                                                             textureReceiver()->orientation()));
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
