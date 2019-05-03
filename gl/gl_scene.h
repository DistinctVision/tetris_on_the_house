#ifndef GL_SCENE_H
#define GL_SCENE_H

#include <QObject>
#include <QOpenGLFunctions>

class GL_ViewRenderer;

class GL_Scene:
        public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
public:
    GL_Scene();
    virtual ~GL_Scene();

    bool enabled() const;
    void setEnabled(bool enabled);

    virtual void init(GL_ViewRenderer * view) = 0;
    virtual void draw(GL_ViewRenderer * view) = 0;
    virtual void destroy(GL_ViewRenderer * view) = 0;

signals:
    void enabledChanged();

private:
    bool m_enabled;
};

#endif // GL_SCENE_H
