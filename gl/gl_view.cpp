#include "gl_view.h"
#include <QFile>
#include <QOpenGLShaderProgram>
#include <QQuickWindow>
#include <QDebug>

#include "gl_scene.h"

GL_View::GL_View():
    m_renderer(nullptr),
    m_focalLength(1.2f, 1.2f),
    m_opticalCenter(0.5f, 0.5f),
    m_nearPlane(0.1f),
    m_farPlane(1000.0f),
    m_inputeFrameSize(-1, -1),
    m_fillFrameMode(FillMode::PreserveAspectCrop),
    m_orderRender(1),
    m_viewportSize(-1, -1)
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

GL_ShaderMaterialPtr GL_View::createMaterial(MaterialType::Enum type) const
{
    if (!m_renderer)
        return GL_ShaderMaterialPtr();
    return m_renderer->createMaterial(type);
}

QVector2D GL_View::focalLength() const
{
    return m_focalLength;
}

void GL_View::setFocalLength(const QVector2D & focalLength)
{
    if (m_focalLength == focalLength)
        return;
    m_focalLength = focalLength;
    emit focalLengthChanged();
}

QVector2D GL_View::opticalCenter() const
{
    return m_opticalCenter;
}

void GL_View::setOpticalCenter(const QVector2D & opticalCenter)
{
    if (m_opticalCenter == opticalCenter)
        return;
    m_opticalCenter = opticalCenter;
    emit opticalCenterChanged();
}

float GL_View::nearPlane() const
{
    return m_nearPlane;
}

void GL_View::setNearPlane(float nearPlane)
{
    if (m_nearPlane == nearPlane)
        return;
    m_nearPlane = nearPlane;
    emit nearPlaneChanged();
}

float GL_View::farPlane() const
{
    return m_farPlane;
}

void GL_View::setFarPlane(float farPlane)
{
    if (m_farPlane == farPlane)
        return;
    m_farPlane = farPlane;
    emit farPlaneChanged();
}

QSize GL_View::inputFrameSize() const
{
    return m_inputeFrameSize;
}

void GL_View::setInputFrameSize(const QSize & inputFrameSize)
{
    if (m_inputeFrameSize == inputFrameSize)
        return;
    m_inputeFrameSize = inputFrameSize;
    emit inputFrameSizeChanged();
}

FillMode::Enum GL_View::fillFrameMode() const
{
    return m_fillFrameMode;
}

void GL_View::setFillFrameMode(FillMode::Enum fillFrameMode)
{
    if (fillFrameMode == m_fillFrameMode)
        return;
    m_fillFrameMode = fillFrameMode;
    emit fillFrameModeChanged();
}

void GL_View::setFillFrameMode(int fillFrameMode)
{
    setFillFrameMode(static_cast<FillMode::Enum>(fillFrameMode));
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

QSize GL_View::viewportSize() const
{
    return m_viewportSize;
}

void GL_View::_setViewportSize(const QSize & viewportSize)
{
    if (viewportSize == m_viewportSize)
        return;
    m_viewportSize = viewportSize;
    emit viewportSizeChanged();
}

int GL_View::orderRender() const
{
    return m_orderRender;
}

void GL_View::setOrderRender(int orderRender)
{
    if (m_orderRender == orderRender)
        return;
    m_orderRender = orderRender;
    emit orderRenderChanged();
}

GL_ViewRenderer::GL_ViewRenderer(GL_View * parent):
    m_parent(parent)
{
    connect(m_parent->window(), &QQuickWindow::beforeRendering, this,
            &GL_ViewRenderer::_beforeSlotDraw, Qt::DirectConnection);
    connect(m_parent->window(), &QQuickWindow::afterRendering, this,
            &GL_ViewRenderer::_afterSlotDraw, Qt::DirectConnection);
    initializeOpenGLFunctions();
    _initEmptyTexture();
    _loadShaders();
    if (m_parent->viewportSize() != viewportSize())
        m_parent->_setViewportSize(viewportSize());
}

GL_View * GL_ViewRenderer::parent() const
{
    return m_parent;
}

GL_ShaderMaterialPtr GL_ViewRenderer::createMaterial(MaterialType::Enum type) const
{
    auto it = m_shaderMaterials.find(type);
    if (it == m_shaderMaterials.cend())
    {
        qFatal("Coudn't find shader material");
    }
    return GL_ShaderMaterialPtr::create(*it.value());
}

QSize GL_ViewRenderer::viewportSize() const
{
    return m_parent->window()->size();
}

GLuint GL_ViewRenderer::emptyTextureId() const
{
    return m_emptyTextureId;
}

QMatrix4x4 GL_ViewRenderer::projectionMatrix() const
{
    return m_projectionMatrix;
}

QMatrix4x4 GL_ViewRenderer::_computeProjectionMatrix() const
{
    QVector2D focalLength = m_parent->focalLength();
    QVector2D opticalCenter = m_parent->opticalCenter();
    float nearPlane = m_parent->nearPlane(), farPlane = m_parent->farPlane();

    QSize viewportSize = this->viewportSize();
    QSize inputFrameSize = m_parent->inputFrameSize();
    if ((inputFrameSize.width() <= 0) || (inputFrameSize.height() <= 0))
        inputFrameSize = viewportSize;
    QMatrix4x4 M;
    float clip = farPlane - nearPlane;
    M(0, 0) = focalLength.x() * inputFrameSize.width();
    M(0, 1) = 0.0f;
    M(0, 2) = opticalCenter.x() * inputFrameSize.width();
    M(0, 3) = 0.0f;
    M(1, 0) = 0.0f;
    M(1, 1) = focalLength.y() * inputFrameSize.width();
    M(1, 2) = opticalCenter.y() * inputFrameSize.height();
    M(1, 3) = 0.0f;
    M(2, 0) = 0.0f;
    M(2, 1) = 0.0f;
    M(2, 2) = (nearPlane + farPlane) / clip;
    M(2, 3) = - (2.0f * nearPlane * farPlane) / clip;
    M(3, 0) = 0.0f;
    M(3, 1) = 0.0f;
    M(3, 2) = 1.0f;
    M(3, 3) = 0.0f;

    QVector2D origin(0.0f, 0.0f);
    QVector2D size(static_cast<float>(inputFrameSize.width()),
                   static_cast<float>(inputFrameSize.height()));

    switch (m_parent->fillFrameMode())
    {
    case FillMode::Stretch:
    {
        origin = QVector2D(0.0f, 0.0f);
        size = QVector2D(static_cast<float>(viewportSize.width()),
                         static_cast<float>(viewportSize.height()));
    } break;
    case FillMode::PreserveAspectFit:
    {
        float aspect = size.x() / size.y();

        float width = viewportSize.height() * aspect, height = static_cast<float>(viewportSize.height());
        if (width > static_cast<float>(viewportSize.width()))
        {
            width = static_cast<float>(viewportSize.width());
            height = viewportSize.width() / aspect;
        }
        size.setX(width);
        size.setY(height);
        origin.setX((viewportSize.width() - width) * 0.5f);
        origin.setY((viewportSize.height() - height) * 0.5f);
    } break;
    case FillMode::PreserveAspectCrop:
    {
        float aspect = size.x() / size.y();

        float width = viewportSize.height() * aspect, height = static_cast<float>(viewportSize.height());
        if (width < static_cast<float>(viewportSize.width()))
        {
            width = static_cast<float>(viewportSize.width());
            height = viewportSize.width() / aspect;
        }
        size.setX(width);
        size.setY(height);
        origin.setX((viewportSize.width() - width) * 0.5f);
        origin.setY((viewportSize.height() - height) * 0.5f);
    } break;
    default:
        break;
    }
    size.setX(size.x() / static_cast<float>(inputFrameSize.width()));
    size.setY(size.y() / static_cast<float>(inputFrameSize.height()));
    QMatrix4x4 imageTransform;
    imageTransform(0, 0) = size.x();
    imageTransform(0, 3) = origin.x();
    imageTransform(1, 1) = size.y();
    imageTransform(1, 3) = origin.y();
    QMatrix4x4 viewportTransform;
    viewportTransform(0, 0) = 2.0f / static_cast<float>(viewportSize.width());
    viewportTransform(0, 3) = - 1.0f;
    viewportTransform(1, 1) = - 2.0f / static_cast<float>(viewportSize.height());
    viewportTransform(1, 3) = 1.0f;

    return viewportTransform * imageTransform * M;
}

void GL_ViewRenderer::_initEmptyTexture()
{
    const uchar bytes[] = { 255, 255, 255, 255 };
    glGenTextures(1, &m_emptyTextureId);
    glBindTexture(GL_TEXTURE_2D, m_emptyTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
}

QSharedPointer<QOpenGLShaderProgram> GL_ViewRenderer::loadShader(const QString & vertexPath,
                                                                 const QString & fragmentPath)
{
    QString vertexSource, fragmentSource;
    {
        QFile file(vertexPath);
        if (!file.open(QFile::ReadOnly | QFile::Text))
        {
            qFatal(QString("Coudn't open file: %1").arg(vertexPath).toStdString().c_str());
        }
        vertexSource = file.readAll();
    }
    {
        QFile file(fragmentPath);
        if (!file.open(QFile::ReadOnly | QFile::Text))
        {
            qFatal(QString("Coudn't open file: %1").arg(fragmentPath).toStdString().c_str());
        }
        fragmentSource = file.readAll();
    }
    if (QOpenGLContext::currentContext()->isOpenGLES())
    {
        vertexSource.push_front(QByteArrayLiteral("#version 300 es\n"));
        fragmentSource.push_front(QByteArrayLiteral("#version 300 es\n"));
    }
    else
    {
        vertexSource.push_front(QByteArrayLiteral("#version 330\n"));
        fragmentSource.push_front(QByteArrayLiteral("#version 330\n"));
    }
    QSharedPointer<QOpenGLShaderProgram> program = QSharedPointer<QOpenGLShaderProgram>::create();
    if (!program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexSource))
    {
        qFatal(program->log().toStdString().c_str());
    }
    if (!program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSource))
    {
        qFatal(program->log().toStdString().c_str());
    }
    if (!program->link())
    {
        qFatal(program->log().toStdString().c_str());
    }
    return program;
}

void GL_ViewRenderer::_loadShaders()
{
    QSharedPointer<QOpenGLShaderProgram> programPtr = loadShader(":/shaders/color.vsh",
                                                                  ":/shaders/color.fsh");
    m_shaderMaterials.insert(MaterialType::Color,
                             GL_ShaderMaterialPtr::create(programPtr,
                                                          QVariantMap {
                                                              { "matrixMVP", QMatrix4x4() },
                                                              { "mainColor", QColor(255, 0, 0, 255) }
                                                          }));
    programPtr = loadShader(":/shaders/texture.vsh",
                             ":/shaders/texture.fsh");
    m_shaderMaterials.insert(MaterialType::Texture,
                             GL_ShaderMaterialPtr::create(programPtr,
                                                          QVariantMap {
                                                              { "matrixMVP", QMatrix4x4() }
                                                          },
                                                          QMap<QString, GLuint> {
                                                              { "main_texture", m_emptyTextureId }
                                                          }));
    programPtr = loadShader(":/shaders/contour_falloff.vsh",
                             ":/shaders/contour_falloff.fsh");
    m_shaderMaterials.insert(MaterialType::ContourFallOff,
                             GL_ShaderMaterialPtr::create(programPtr,
                                                          QVariantMap {
                                                              { "matrixMVP", QMatrix4x4() },
                                                              { "matrixMV", QMatrix4x4() },
                                                              { "mainColor", QColor(255, 215, 0, 200) },
                                                              { "border_size", 0.1f },
                                                              { "border_color", QColor(255, 255, 255, 255) }
                                                          }));
    programPtr = loadShader(":/shaders/screen_morph/default.vsh",
                             ":/shaders/screen_morph/default.fsh");
    m_shaderMaterials.insert(MaterialType::ScreenMorph_default,
                             GL_ShaderMaterialPtr::create(programPtr,
                                                          QVariantMap {
                                                              { "matrixMVP", QMatrix4x4() },
                                                              { "matrixView2FrameUV", QMatrix4x4() },
                                                              { "color_a", QVector3D(1.0f, 1.0f, 1.0f) },
                                                              { "color_b", QVector3D(0.0f, 0.0f, 0.0f) }
                                                          },
                                                          QMap<QString, GLuint> {
                                                              { "screen_texture", m_emptyTextureId }
                                                          }));
    programPtr = loadShader(":/shaders/screen_morph/glow_edges.vsh",
                             ":/shaders/screen_morph/glow_edges.fsh");
    m_shaderMaterials.insert(MaterialType::ScreenMorph_glowEdges,
                             GL_ShaderMaterialPtr::create(programPtr,
                                                          QVariantMap {
                                                              { "matrixMVP", QMatrix4x4() },
                                                              { "matrixView2FrameUV", QMatrix4x4() },
                                                              { "color_a", QVector3D(1.0f, 1.0f, 1.0f) },
                                                              { "color_b", QVector3D(0.0f, 0.0f, 0.0f) },
                                                              { "edges_size", 0.1f },
                                                              { "edges_color", QColor(255, 255, 255, 255) }
                                                          },
                                                          QMap<QString, GLuint> {
                                                              { "screen_texture", m_emptyTextureId }
                                                          }));
    programPtr = loadShader(":/shaders/screen_morph/wave.vsh",
                             ":/shaders/screen_morph/default.fsh");
    m_shaderMaterials.insert(MaterialType::ScreenMorph_wave,
                             GL_ShaderMaterialPtr::create(programPtr,
                                                          QVariantMap {
                                                              { "matrixMVP", QMatrix4x4() },
                                                              { "wave_threshold", 0.05f },
                                                              { "wave_time", 0.0f },
                                                              { "wave_timeScale", 1.0f },
                                                              { "wave_origin", QVector3D(0.0f, 0.0f, 0.0f) },
                                                              { "wave_distanceStep", 10.0f },
                                                              { "wave_distanceStepScale", 2.0f },
                                                              { "wave_scale", 5.0f },
                                                              { "matrixView2FrameUV", QMatrix4x4() },
                                                              { "color_a", QVector3D(1.0f, 1.0f, 1.0f) },
                                                              { "color_b", QVector3D(0.0f, 0.0f, 0.0f) }
                                                          },
                                                          QMap<QString, GLuint> {
                                                              { "screen_texture", m_emptyTextureId }
                                                          }));
    programPtr = loadShader(":/shaders/screen_morph/transform.vsh",
                             ":/shaders/screen_morph/default.fsh");
    m_shaderMaterials.insert(MaterialType::ScreenMorph_transform,
                             GL_ShaderMaterialPtr::create(programPtr,
                                                          QVariantMap {
                                                              { "matrixMVP", QMatrix4x4() },
                                                              { "matrixMVP_transform", QMatrix4x4() },
                                                              { "matrixView2FrameUV", QMatrix4x4() },
                                                              { "color_a", QVector3D(1.0f, 1.0f, 1.0f) },
                                                              { "color_b", QVector3D(0.0f, 0.0f, 0.0f) }
                                                          },
                                                          QMap<QString, GLuint> {
                                                              { "screen_texture", m_emptyTextureId }
                                                          }));
    programPtr = loadShader(":/shaders/screen_morph/glow_edges_on_plane.vsh",
                             ":/shaders/screen_morph/glow_edges_on_plane.fsh");
    m_shaderMaterials.insert(MaterialType::ScreenMorph_glowEdgesOnPlane,
                             GL_ShaderMaterialPtr::create(programPtr,
                                                          QVariantMap {
                                                              { "matrixMVP", QMatrix4x4() },
                                                              { "matrixView2FrameUV", QMatrix4x4() },
                                                              { "color_a", QVector3D(1.0f, 1.0f, 1.0f) },
                                                              { "color_b", QVector3D(0.0f, 0.0f, 0.0f) },
                                                              { "edges_size", 0.1f },
                                                              { "edges_power", 2.0f },
                                                              { "edges_color", QColor(255, 255, 255, 255) },
                                                              { "plane", QVector4D(1.0, 0.0, 0.0, 0.0) },
                                                              { "plane_edge_distance", 5.0f },
                                                              { "max_edges_distance", 10.0f }
                                                          },
                                                          QMap<QString, GLuint> {
                                                              { "screen_texture", m_emptyTextureId }
                                                          }));
    programPtr = loadShader(":/shaders/screen_morph/glow_edges_transform.vsh",
                             ":/shaders/screen_morph/glow_edges.fsh");
    m_shaderMaterials.insert(MaterialType::ScreenMorph_glowEdges_transform,
                             GL_ShaderMaterialPtr::create(programPtr,
                                                          QVariantMap {
                                                              { "matrixMVP", QMatrix4x4() },
                                                              { "matrixMVP_transform", QMatrix4x4() },
                                                              { "matrixView2FrameUV", QMatrix4x4() },
                                                              { "color_a", QVector3D(1.0f, 1.0f, 1.0f) },
                                                              { "color_b", QVector3D(0.0f, 0.0f, 0.0f) },
                                                              { "edges_size", 0.1f },
                                                              { "edges_color", QColor(255, 255, 255, 255) }
                                                          },
                                                          QMap<QString, GLuint> {
                                                              { "screen_texture", m_emptyTextureId }
                                                          }));
    programPtr = loadShader(":/shaders/tunnel.vsh",
                             ":/shaders/tunnel.fsh");
    m_shaderMaterials.insert(MaterialType::Tunnel,
                             GL_ShaderMaterialPtr::create(programPtr,
                                                          QVariantMap {
                                                              { "matrixMVP", QMatrix4x4() },
                                                              { "colorA", QColor(255, 255, 255, 255) },
                                                              { "colorB", QColor(0, 0, 0, 255) },
                                                              { "plane_z", 0.0f },
                                                              { "plane_delta", 30.0f },
                                                              { "begin_z", 0.0f }
                                                          }));
    programPtr = loadShader(":/shaders/morph_falloff.vsh",
                             ":/shaders/morph_falloff.fsh");
    m_shaderMaterials.insert(MaterialType::Morph_fallOff,
                             GL_ShaderMaterialPtr::create(programPtr,
                                                          QVariantMap {
                                                              { "matrixMVP", QMatrix4x4() },
                                                              { "matrixMV", QMatrix4x4() },
                                                              { "time", 0.0f },
                                                              { "mainColor", QColor(255, 255, 255, 255) },
                                                              { "fallOffColor", QColor(0, 0, 0, 255) }
                                                          }));
    programPtr = loadShader(":/shaders/table.vsh",
                             ":/shaders/table.fsh");
    m_shaderMaterials.insert(MaterialType::Table,
                             GL_ShaderMaterialPtr::create(programPtr,
                                                          QVariantMap {
                                                              { "matrixMVP", QMatrix4x4() },
                                                              { "tableColor", QColor(0, 255, 0, 255) },
                                                              { "tableSize", 19 },
                                                              { "tableEdgeColor", QColor(0, 255, 0, 0) },
                                                              { "tableEdgeSize", 0.1f },
                                                              { "level", 5.0f }
                                                          }));
}

void GL_ViewRenderer::_beforeSlotDraw()
{
    if (m_parent->orderRender() <= 0)
        _draw();
}

void GL_ViewRenderer::_afterSlotDraw()
{
    if (m_parent->orderRender() > 0)
        _draw();
}

void GL_ViewRenderer::_draw()
{
    if (m_parent->viewportSize() != viewportSize())
        m_parent->_setViewportSize(viewportSize());

    m_projectionMatrix = _computeProjectionMatrix();

    QSize viewportSize = this->viewportSize();
    glViewport(0, 0, viewportSize.width(), viewportSize.height());

    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glClearDepthf(1.0f);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    QList<QPair<GL_Scene*, bool>> & scenes = m_parent->m_scenes;
    for (QPair<GL_Scene*, bool> & s : scenes)
    {
        if (s.first->enabled() && !s.second)
        {
            s.first->init(this);
            s.second = true;
        }
    }
    for (QPair<GL_Scene*, bool> & s : scenes)
    {
        if (!s.first->enabled() || (!s.second))
            continue;
        s.first->draw(this);
    }

    glDisable(GL_CULL_FACE);
    m_parent->window()->resetOpenGLState();
}
