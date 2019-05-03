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
        {
            assert(m_renderer);
            s.first->destroy(m_renderer);
        }
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
            s.first->destroy(m_renderer);
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
    connect(m_parent->window(), &QQuickWindow::beforeRendering, this, &GL_ViewRenderer::_draw, Qt::DirectConnection);
    initializeOpenGLFunctions();
    _initEmptyTexture();
    _loadShaders();
}

GL_ShaderMaterialPtr GL_ViewRenderer::createMaterial(MaterialType::Enum type) const
{
    auto it = m_shaderMaterials.find(type);
    if (it == m_shaderMaterials.cend())
    {
        qFatal("Coudn't find shader material");
    }
    return GL_ShaderMaterialPtr::create(it.value());
}

QSize GL_ViewRenderer::viewportSize() const
{
    return m_parent->window()->size();
}

void GL_ViewRenderer::_initEmptyTexture()
{
    const uchar bytes[] = { 255, 255, 255, 255 };
    glGenTextures(1, &m_emptyTextureId);
    glBindTexture(GL_TEXTURE_2D, m_emptyTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void GL_ViewRenderer::_loadShaders()
{
    QSharedPointer<QOpenGLShaderProgram> programPtr = QSharedPointer<QOpenGLShaderProgram>::create();
    programPtr->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/texture.vsh");
    programPtr->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/texture.fsh");
    programPtr->link();
    m_shaderMaterials.insert(MaterialType::MT_Texture,
                             GL_ShaderMaterialPtr::create(programPtr,
                                                          QVariantMap {
                                                              { "matrixMVP", QMatrix4x4() }
                                                          },
                                                          QMap<QString, GLuint> {
                                                              { "main_texture", m_emptyTextureId }
                                                          }));
}

void GL_ViewRenderer::_draw()
{
    QQuickWindow * window = m_parent->window();

    QSize viewportSize = this->viewportSize();
    glViewport(0, 0, viewportSize.width(), viewportSize.height());

    glEnable(GL_DEPTH_TEST);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    QList<QPair<GL_Scene*, bool>> & scenes = m_parent->m_scenes;
    for (QPair<GL_Scene*, bool> & s : scenes)
    {
        if (!s.second)
        {
            s.first->init(this);
            s.second = true;
        }
    }
    for (QPair<GL_Scene*, bool> & s : scenes)
    {
        if (!s.first->enabled())
            continue;
        s.first->draw(this);
    }

    window->resetOpenGLState();
}
