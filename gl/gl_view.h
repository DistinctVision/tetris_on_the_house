#ifndef GL_VIEW_H
#define GL_VIEW_H

#include <QObject>
#include <QOpenGLFunctions>
#include <QQuickItem>
#include <QVector2D>

#include <QPair>
#include <QList>
#include <QVariantList>
#include <QMap>
#include <QMatrix4x4>

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
    Q_PROPERTY(QVector2D focalLength READ focalLength WRITE setFocalLength NOTIFY focalLengthChanged)
    Q_PROPERTY(QVector2D opticalCenter READ opticalCenter WRITE setOpticalCenter NOTIFY opticalCenterChanged)
    Q_PROPERTY(float nearPlane READ nearPlane WRITE setNearPlane NOTIFY nearPlaneChanged)
    Q_PROPERTY(float farPlane READ farPlane WRITE setFarPlane NOTIFY farPlaneChanged)
    Q_PROPERTY(int fillFrameMode READ fillFrameMode WRITE setFillFrameMode NOTIFY fillFrameModeChanged)

public:
    GL_View();
    ~GL_View();

    QList<QObject*> scenes() const;
    void setScenes(const QList<QObject*> & scenes);

    GL_ShaderMaterialPtr createMaterial(MaterialType::Enum type) const;

    QVector2D focalLength() const;
    void setFocalLength(const QVector2D & focalLength);

    QVector2D opticalCenter() const;
    void setOpticalCenter(const QVector2D & opticalCenter);

    float nearPlane() const;
    void setNearPlane(float nearPlane);

    float farPlane() const;
    void setFarPlane(float farPlane);

    QSize inputeFrameSize() const;
    void setInputeFrameSize(const QSize & inputeFrameSize);

    FillMode::Enum fillFrameMode() const;
    void setFillFrameMode(FillMode::Enum fillFrameMode);

public slots:
    void sync();
    void cleanup();

private slots:
    void handleWindowChanged(QQuickWindow *win);

signals:
    void scenesChanged();
    void focalLengthChanged();
    void opticalCenterChanged();
    void nearPlaneChanged();
    void farPlaneChanged();
    void inputFrameSizeChanged();
    void fillFrameModeChanged();

private:
    friend class GL_ViewRenderer;

    GL_ViewRenderer * m_renderer;
    QList<QPair<GL_Scene*, bool>> m_scenes;
    QVector2D m_focalLength;
    QVector2D m_opticalCenter;
    float m_nearPlane;
    float m_farPlane;
    QSize m_inputeFrameSize;
    FillMode::Enum m_fillFrameMode;
};

class GL_ViewRenderer:
        public QObject, public QOpenGLFunctions
{
    Q_OBJECT
public:
    GL_ViewRenderer(GL_View * parent);

    GL_ShaderMaterialPtr createMaterial(MaterialType::Enum type) const;

    QSize viewportSize() const;

    QMatrix4x4 projectionMatrix() const;

private slots:
    void _draw();

private:
    GL_View * m_parent;

    GLuint m_emptyTextureId;
    QMap<int, GL_ShaderMaterialPtr> m_shaderMaterials;

    QMatrix4x4 m_projectionMatrix;

    void _initEmptyTexture();
    void _loadShaders();
    QMatrix4x4 _computeProjectionMatrix() const;
};

#endif // GL_VIEW_H
