#ifndef GL_VIEW_H
#define GL_VIEW_H

#include <QObject>
#include <QOpenGLFunctions>
#include <QQuickItem>
#include <QQuickItem>

#include <QPair>
#include <QList>
#include <QVariantList>

class GL_Scene;
class GL_ViewRenderer;

class GL_View:
        public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QList<QObject*> scenes READ scenes WRITE setScenes NOTIFY scenesChanged)
public:
    GL_View();
    ~GL_View();

    QList<QObject*> scenes() const;
    void setScenes(const QList<QObject*> & scenes);

public slots:
    void sync();
    void cleanup();

private slots:
    void handleWindowChanged(QQuickWindow *win);

signals:
    void scenesChanged();

private:
    friend class GL_ViewRenderer;

    GL_ViewRenderer * m_renderer;
    QList<QPair<GL_Scene*, bool>> m_scenes;
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
