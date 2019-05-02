#ifndef GL_VIEW_H
#define GL_VIEW_H

#include <QObject>
#include <QOpenGLFunctions>
#include <QQuickItem>
#include <QQuickItem>

class GL_Scene;
class GL_ViewRenderer;

class GL_View:
        public QQuickItem
{
    Q_OBJECT
public:
    GL_View();
    ~GL_View();

    GL_Scene * scene() const;
    void setScene(GL_Scene * scene);

public slots:
    void sync();
    void cleanup();

private slots:
    void handleWindowChanged(QQuickWindow *win);

signals:
    void sceneChanged();

private:
    GL_ViewRenderer * m_renderer;
    GL_Scene * m_scene;
};

class GL_ViewRenderer:
        public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    GL_ViewRenderer(GL_View * parent);

public slots:
    void paint();

private:
    GL_View * m_parent;
};

#endif // GL_VIEW_H
