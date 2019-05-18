#ifndef TRANSFROMHOUSESCENE_H
#define TRANSFROMHOUSESCENE_H

#include <QMatrix4x4>

#include "animationscene.h"

class TransfromHouseScene:
        public AnimationScene
{
public:
    TransfromHouseScene(int duration, float deltaZ);

    void init(GL_ViewRenderer * view) override;
    void destroy(GL_ViewRenderer * view) override;
    void draw(GL_ViewRenderer * view) override;

private:
    GL_ShaderMaterialPtr m_material;
    float m_deltaZ;
};

#endif // TRANSFROMHOUSESCENE_H
