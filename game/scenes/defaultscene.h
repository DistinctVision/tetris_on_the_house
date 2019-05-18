#ifndef DEFAULTSCENE_H
#define DEFAULTSCENE_H

#include "animationscene.h"

class DefaultScene:
        public AnimationScene
{
public:
    DefaultScene(int duration);

    void init(GL_ViewRenderer * view) override;
    void destroy(GL_ViewRenderer * view) override;
    void draw(GL_ViewRenderer * view) override;

private:
    GL_ShaderMaterialPtr m_material;
};

#endif // DEFAULTSCENE_H
