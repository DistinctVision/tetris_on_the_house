#ifndef GL_SHADERMATERIAL_H
#define GL_SHADERMATERIAL_H

#include <QSharedPointer>
#include <QVariantMap>
#include <QOpenGLShaderProgram>

class GL_ShaderMaterial
{
public:
    GL_ShaderMaterial(const QSharedPointer<QOpenGLShaderProgram> & program,
                      const QVariantMap & values);

    void setValue(const QString & name, const QVariant & value);
    QVariant value(const QString & name) const;

    int attributeLocation(const QString & name) const;
    void enableAttribute(int location) const;
    void disableAttribute(int location) const;
    void setAttributeBuffer(int location, GLenum type, int offset, int tupleSize, int stride = 0) const;

    void bind() const;
    void release() const;

private:
    QSharedPointer<QOpenGLShaderProgram> m_program;
    QVariantMap m_values;

    bool _setUniformValue(const QString & key, const QVariant & value) const;
};

using GL_ShaderMaterialPtr = QSharedPointer<GL_ShaderMaterial>;

#endif // GL_SHADERMATERIAL_H
