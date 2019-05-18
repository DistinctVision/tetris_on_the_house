#ifndef GLOWEDGESSCENE_H
#define GLOWEDGESSCENE_H

#include <QVector3D>

#include "animationscene.h"

#include "gl/gl_mesh.h"

class GlowEdgesScene:
        public AnimationScene
{
public:
    GlowEdgesScene(int duration,
                   const QVector3D & planeDir);

    void init(GL_ViewRenderer * view) override;
    void destroy(GL_ViewRenderer * view) override;
    void draw(GL_ViewRenderer * view) override;

private:
    GL_ShaderMaterialPtr m_material;
    GL_MeshPtr m_backwardMesh;

    QVector3D m_planeDir;

    void _createBackwardMesh();
};

#endif // GLOWEDGESSCENE_H
