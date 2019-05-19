#ifndef FINALSCENE_H
#define FINALSCENE_H

#include "animationscene.h"

#include "gl/gl_mesh.h"

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
    GL_ShaderMaterialPtr m_materialForDoors;
    GL_ShaderMaterialPtr m_materialColor;
    GL_ShaderMaterialPtr m_materialTunnel;

    GL_MeshPtr m_meshTunnelGrid;

    void _createTunnelGrid();
};

#endif // FINALSCENE_H
