#ifndef FINALSCENE_H
#define FINALSCENE_H

#include "animationscene.h"

class FinalScene:
        public AnimationScene
{
public:
    FinalScene(int duration);

    void init(GL_ViewRenderer * view) override;
    void destroy(GL_ViewRenderer * view) override;
    void draw(GL_ViewRenderer * view) override;

private:
    GL_ShaderMaterialPtr m_materialHouse;
    GL_ShaderMaterialPtr m_materialTransform;
};

#endif // FINALSCENE_H
