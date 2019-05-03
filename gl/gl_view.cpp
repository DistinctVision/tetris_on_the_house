#include "gl_view.h"
#include <QOpenGLShaderProgram>
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

GL_ShaderMaterialPtr GL_View::createMaterial(GL_View::MaterialType type) const
{
    if (!m_renderer)
        return GL_ShaderMaterialPtr();
    return m_renderer->createMaterial(type);
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
    _loadShaders();
}

GL_ShaderMaterialPtr GL_ViewRenderer::createMaterial(GL_View::MaterialType type) const
{
    auto it = m_shaderMaterials.find(type);
    if (it == m_shaderMaterials.cend())
    {
        qFatal("Coudn't find shader material");
    }
    return GL_ShaderMaterialPtr::create(it.value());
}

void GL_ViewRenderer::_loadShaders()
{
    QSharedPointer<QOpenGLShaderProgram> programPtr = QSharedPointer<QOpenGLShaderProgram>::create();
    programPtr->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/texture.vsh");
    programPtr->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/texture.fsh");
    programPtr->link();
    m_shaderMaterials.insert(GL_View::MT_Texture, GL_ShaderMaterialPtr::create(programPtr, QVariantMap {
                                                                                                { "matrixMVP", QMatrix4x4() },
                                                                                                { "main_texture", 0 }
                                                                                            }));
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
    {
        if (!s.first->enabled())
            continue;
        s.first->draw();
    }

    window->resetOpenGLState();
}
