#include "gl_view.h"
#include <QQuickWindow>
#include <QDebug>

#include "gl_scene.h"

GL_View::GL_View():
    m_renderer(nullptr)
{
    connect(this, &QQuickItem::windowChanged, this, &GL_View::handleWindowChanged);
}

GL_View::~GL_View()
{
    cleanup();
}

QList<QObject*> GL_View::scenes() const
{
    QList<QObject*> l;
    for (const QPair<GL_Scene*, bool> & s : m_scenes)
        l.push_back(s.first);
    return l;
}

void GL_View::setScenes(const QList<QObject*> & scenes)
{
    for (const QPair<GL_Scene*, bool> & s : m_scenes)
    {
        if (s.second)
            s.first->destroy();
    }
    m_scenes.clear();
    for (QObject * o : scenes)
    {
        GL_Scene * scene = dynamic_cast<GL_Scene*>(o);
        if (scene == nullptr)
        {
            qFatal((QString(Q_FUNC_INFO) + ":Inavlid scene").toStdString().c_str());
            continue;
        }
        m_scenes.push_back(qMakePair(scene, false));
    }
    emit scenesChanged();
}

void GL_View::sync()
{
    if (!m_renderer)
    {
        m_renderer = new GL_ViewRenderer(this);
        connect(window(), &QQuickWindow::beforeRendering, m_renderer, &GL_ViewRenderer::paint, Qt::DirectConnection);
    }
}

void GL_View::cleanup()
{
    if (m_renderer)
    {
        for (QPair<GL_Scene*, bool> & s : m_scenes)
        {
            if (!s.second)
                continue;
            s.first->destroy();
            s.second = false;
        }
        delete m_renderer;
        m_renderer = nullptr;
    }
}

void GL_View::handleWindowChanged(QQuickWindow * win)
{
    if (win)
    {
        connect(win, &QQuickWindow::beforeSynchronizing, this, &GL_View::sync, Qt::DirectConnection);
        connect(win, &QQuickWindow::sceneGraphInvalidated, this, &GL_View::cleanup, Qt::DirectConnection);
        win->setClearBeforeRendering(false);
    }
}

GL_ViewRenderer::GL_ViewRenderer(GL_View * parent):
    m_parent(parent)
{
    initializeOpenGLFunctions();
}

void GL_ViewRenderer::paint()
{
    QQuickWindow * window = m_parent->window();

    QSize viewportSize = window->size();
    glViewport(0, 0, viewportSize.width(), viewportSize.height());

    glEnable(GL_DEPTH_TEST);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    QList<QPair<GL_Scene*, bool>> & scenes = m_parent->m_scenes;
    for (QPair<GL_Scene*, bool> & s : scenes)
    {
        if (!s.second)
        {
            s.first->init();
            s.second = true;
        }
    }
    for (QPair<GL_Scene*, bool> & s : scenes)
        s.first->draw();

    window->resetOpenGLState();
}
