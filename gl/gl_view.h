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

class GL_View:
        public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QList<QObject*> scenes READ scenes WRITE setScenes NOTIFY scenesChanged)
public:
    enum MaterialType
    {
        MT_Texture
    };

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
        public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    GL_ViewRenderer(GL_View * parent);

    GL_ShaderMaterialPtr createMaterial(GL_View::MaterialType type) const;

public slots:
    void paint();

private:
    GL_View * m_parent;

    QMap<int, GL_ShaderMaterialPtr> m_shaderMaterials;

    void _loadShaders();
};

#endif // GL_VIEW_H
