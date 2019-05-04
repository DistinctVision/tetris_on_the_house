#ifndef GL_DEBUGIMAGESCENE_H
#define GL_DEBUGIMAGESCENE_H

#include <QSize>
#include <QSharedPointer>
#include <QOpenGLFunctions>

#include "gl_screenobject.h"
#include "gl_scene.h"
#include "gl_view.h"

class DebugImageObject;

class GL_DebugImageScene:
        public GL_Scene
{
    Q_OBJECT

    Q_PROPERTY(DebugImageObject* debugImageObject READ debugImageObject WRITE setDebugImageObject NOTIFY debugImageObjectChanged)
    Q_PROPERTY(int fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged)
public:
    GL_DebugImageScene();
    ~GL_DebugImageScene() override;

    DebugImageObject * debugImageObject() const;
    void setDebugImageObject(DebugImageObject * debugImageObject);

    FillMode::Enum fillMode() const;
    void setFillMode(FillMode::Enum mode);
    void setFillMode(int mode);

    void init(GL_ViewRenderer * view) override;
    void destroy(GL_ViewRenderer * view) override;

    void draw(GL_ViewRenderer * view) override;

signals:
    void debugImageObjectChanged();
    void fillModeChanged();

private:
    DebugImageObject * m_debugImageObject;
    GLuint m_debugTextureId;
    QSize m_debugTextureSize;
    GL_ScreenObjectPtr m_screenQuad;
    FillMode::Enum m_fillMode;
};

#endif // GL_DEBUGIMAGESCENE_H
