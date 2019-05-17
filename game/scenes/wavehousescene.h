#ifndef WAVEHOUSESCENE_H
#define WAVEHOUSESCENE_H

#include "animationscene.h"

class WaveHouseScene:
        public AnimationScene
{
public:
    WaveHouseScene(int duration, float wave_timeScale, float wave_scale);

    float wave_timeScale() const;
    void setWave_timeScale(float wave_timeScale);

    float wave_scale() const;
    void setWave_scale(float wave_scale);

    void init(GL_ViewRenderer * view) override;
    void destroy(GL_ViewRenderer * view) override;
    void draw(GL_ViewRenderer * view) override;

private:
    GL_ShaderMaterialPtr m_material;
    float m_wave_timeScale;
    float m_wave_scale;
};

#endif // WAVEHOUSESCENE_H
