#ifndef GAMESTARTSCENE_H
#define GAMESTARTSCENE_H

#include <memory>

#include <QMatrix4x4>

#include "gl/gl_shadermaterial.h"
#include "animationscene.h"

class HouseObject;
class TextureReceiver;

class GameStartScene:
        public AnimationScene
{
public:
    GameStartScene(int duration);

    void init(GL_ViewRenderer * view) override;
    void destroy(GL_ViewRenderer * view) override;
    void draw(GL_ViewRenderer * view) override;

private:
    GL_ShaderMaterialPtr m_material;
};

#endif // GAMESTARTSCENE_H
