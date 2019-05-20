#ifndef FINALSCENE_H
#define FINALSCENE_H

#include <list>

#include "animationscene.h"

#include "gl/gl_mesh.h"

#include "game/birdmesh.h"
#include "game/birdobject.h"

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

    std::list<BirdObjectPtr> m_birds;

    void _createTunnelGrid();
    void _createBirds(GL_ViewRenderer * view, int number);
    void _drawBirds(GL_ViewRenderer * view, const QMatrix4x4 & viewMatrix);
};

#endif // FINALSCENE_H
