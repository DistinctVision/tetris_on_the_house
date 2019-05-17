#ifndef CHANGECOLORSSCENE_H
#define CHANGECOLORSSCENE_H

#include "animationscene.h"

class ChangeColorsScene:
        public AnimationScene
{
public:
    ChangeColorsScene(int duration, int numberCycles);

    int numberCyrcles() const;
    void setNumberCyrcles(int numberCyrcles);

    void init(GL_ViewRenderer * view) override;
    void destroy(GL_ViewRenderer * view) override;
    void draw(GL_ViewRenderer * view) override;

private:
    GL_ShaderMaterialPtr m_material;

    int m_numberCyrcles;
};

#endif // CHANGECOLORSSCENE_H
