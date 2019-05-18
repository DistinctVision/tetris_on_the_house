#ifndef WAVEHOUSESCENE_H
#define WAVEHOUSESCENE_H

#include <QVector3D>

#include "animationscene.h"

class WaveHouseScene:
        public AnimationScene
{
public:
    WaveHouseScene(int duration,
                   const QVector3D & wave_origin,
                   float wave_timeScale,
                   float wave_distanceStep,
                   float wave_distanceStepScale,
                   float wave_scale);

    void init(GL_ViewRenderer * view) override;
    void destroy(GL_ViewRenderer * view) override;
    void draw(GL_ViewRenderer * view) override;

private:
    GL_ShaderMaterialPtr m_material;
    QVector3D m_wave_origin;
    float m_wave_timeScale;
    float m_wave_distanceStep;
    float m_wave_distanceStepScale;
    float m_wave_scale;
};

#endif // WAVEHOUSESCENE_H
