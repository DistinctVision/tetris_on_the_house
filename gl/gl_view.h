#ifndef GL_VIEW_H
#define GL_VIEW_H

#include <QObject>
#include <QOpenGLFunctions>
#include <QQuickItem>

#include <QPair>
#include <QList>
#include <QVariantList>
#include <QMap>

#include "gl_shadermaterial.h"

class GL_Scene;
class GL_ViewRenderer;

struct MaterialType
{
    Q_GADGET
public:
    enum Enum
    {
        MT_Texture
    };
    Q_ENUM(Enum)
};

struct FillMode
{
    Q_GADGET
public:
    enum Enum
    {
        NoFill,
        Stretch,
        PreserveAspectFit,
        PreserveAspectCrop
    };
    Q_ENUM(Enum)
};

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

    GL_ShaderMaterialPtr createMaterial(MaterialType type) const;

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
        public QObject, public QOpenGLFunctions
{
    Q_OBJECT
public:
    GL_ViewRenderer(GL_View * parent);

    GL_ShaderMaterialPtr createMaterial(MaterialType::Enum type) const;

    QSize viewportSize() const;

private slots:
    void _draw();

private:
    GL_View * m_parent;

    GLuint m_emptyTextureId;
    QMap<int, GL_ShaderMaterialPtr> m_shaderMaterials;

    void _initEmptyTexture();
    void _loadShaders();
};

#endif // GL_VIEW_H
