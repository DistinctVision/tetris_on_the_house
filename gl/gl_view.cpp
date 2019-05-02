#include "gl_view.h"
#include <QQuickWindow>

#include "gl_scene.h"

GL_View::GL_View():
    m_renderer(nullptr),
    m_scene(nullptr)
{
    connect(this, &QQuickItem::windowChanged, this, &GL_View::handleWindowChanged);
}

GL_View::~GL_View()
{
    //cleanup();
}

GL_Scene * GL_View::scene() const
{
    return m_scene;
}

void GL_View::setScene(GL_Scene * scene)
{
    m_scene = scene;
    emit sceneChanged();
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
        if (m_scene)
            m_scene->destroy();
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

    GL_Scene * scene = m_parent->scene();
    if (scene)
    {
        scene->draw();
    }
    window->resetOpenGLState();
}
