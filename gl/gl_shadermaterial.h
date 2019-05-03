#ifndef GL_SHADERMATERIAL_H
#define GL_SHADERMATERIAL_H

#include <QSharedPointer>
#include <QVariantMap>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QVector>

class GL_ShaderMaterial
{
public:
    GL_ShaderMaterial(const QSharedPointer<QOpenGLShaderProgram> & program,
                      const QVariantMap & values,
                      const QMap<QString, GLuint> & textures = {});

    bool containsValue(const QString & name) const;
    QVariant value(const QString & name) const;
    void setValue(const QString & name, const QVariant & value);

    bool containsTexture(const QString & name) const;
    GLuint texture(const QString & name) const;
    void setTexture(const QString & name, GLuint textureId);

    int attributeLocation(const QString & name) const;
    void enableAttribute(int location) const;
    void disableAttribute(int location) const;
    void setAttributeBuffer(int location, GLenum type, int offset, int tupleSize, int stride = 0) const;

    void bind(QOpenGLFunctions * gl) const;
    void release(QOpenGLFunctions * gl) const;

private:
    QSharedPointer<QOpenGLShaderProgram> m_program;
    QVariantMap m_values;
    QMap<QString, GLuint> m_textures;

    bool _setUniformValue(const QString & key, const QVariant & value) const;
};

using GL_ShaderMaterialPtr = QSharedPointer<GL_ShaderMaterial>;

#endif // GL_SHADERMATERIAL_H
