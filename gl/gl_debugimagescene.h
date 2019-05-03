#ifndef GL_DEBUGIMAGESCENE_H
#define GL_DEBUGIMAGESCENE_H

#include <QOpenGLFunctions>

#include "gl_scene.h"

class FrameHandler;

class GL_DebugImageScene:
        public GL_Scene
{
    Q_OBJECT

    Q_PROPERTY(QObject* frameHandler READ frameHandler WRITE setFrameHandler NOTIFY frameHandlerChanged)
public:
    GL_DebugImageScene();
    ~GL_DebugImageScene() override;

    FrameHandler * frameHandler() const;
    void setFrameHandler(FrameHandler * frameHandler);

    void init(QOpenGLFunctions * gl) override;
    void destroy(QOpenGLFunctions * gl) override;

    void draw(GL_ViewRenderer * view) override;

signals:
    void frameHandlerChanged();

private:
    GLuint m_debugTextureId;

    FrameHandler * m_frameHandler;
};

#endif // GL_DEBUGIMAGESCENE_H
