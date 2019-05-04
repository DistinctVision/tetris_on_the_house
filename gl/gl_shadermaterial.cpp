#include "gl_shadermaterial.h"
#include <QMapIterator>
#include <QDebug>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix3x3>
#include <QMatrix4x4>
#include <QColor>

GL_ShaderMaterial::GL_ShaderMaterial(const QSharedPointer<QOpenGLShaderProgram> & program,
                                     const QVariantMap & values,
                                     const QMap<QString, GLuint> & textures):
    m_program(program),
    m_values(values),
    m_textures(textures)
{
}

bool GL_ShaderMaterial::containsValue(const QString & name) const
{
    return m_values.contains(name);
}

QVariant GL_ShaderMaterial::value(const QString & name) const
{
    auto it = m_values.find(name);
    if (it == m_values.cend())
    {
        qFatal(QString("Coudn't find value : %1").arg(name).toStdString().c_str());
    }
    return it.value();
}

void GL_ShaderMaterial::setValue(const QString & name, const QVariant & value)
{
    auto it = m_values.find(name);
    if (it == m_values.end())
    {
        qFatal(QString("Coudn't find value : %1").arg(name).toStdString().c_str());
    }
    if (it.value().type() != value.type())
    {
        qFatal(QString("Mismath value : %1").arg(name).toStdString().c_str());
    }
    it.value() = value;
}

bool GL_ShaderMaterial::containsTexture(const QString & name) const
{
    return m_textures.contains(name);
}

GLuint GL_ShaderMaterial::texture(const QString & name) const
{
    auto it = m_textures.find(name);
    if (it == m_textures.cend())
    {
        qFatal(QString("Coudn't find texture : %1").arg(name).toStdString().c_str());
    }
    return it.value();
}

void GL_ShaderMaterial::setTexture(const QString & name, GLuint textureId)
{
    auto it = m_textures.find(name);
    if (it == m_textures.end())
    {
        qFatal(QString("Coudn't find texture : %1").arg(name).toStdString().c_str());
    }
    it.value() = textureId;
}

void GL_ShaderMaterial::bind(QOpenGLFunctions * gl) const
{
    m_program->bind();

    {
        QMapIterator<QString, QVariant> it(m_values);
        while (it.hasNext())
        {
            it.next();
            if (!_setUniformValue(it.key(), it.value()))
            {
                qFatal(QString("Coudn't set uniform value: %1").arg(it.key()).toStdString().c_str());
            }
        }
    }

    {
        int index = 0;
        QMapIterator<QString, GLuint> it(m_textures);
        while (it.hasNext())
        {
            it.next();
            if (!_setUniformValue(it.key(), index))
            {
                qFatal(QString("Coudn't set texture: %1").arg(it.key()).toStdString().c_str());
            }
            gl->glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + index));
            gl->glBindTexture(GL_TEXTURE_2D, it.value());
        }
    }
}

void GL_ShaderMaterial::release(QOpenGLFunctions * gl) const
{
    Q_UNUSED(gl);
    m_program->release();
}

int GL_ShaderMaterial::attributeLocation(const QString & name) const
{
    return m_program->attributeLocation(name.toStdString().c_str());
}

void GL_ShaderMaterial::enableAttribute(int location) const
{
    m_program->enableAttributeArray(location);
}

void GL_ShaderMaterial::disableAttribute(int location) const
{
    m_program->disableAttributeArray(location);
}

void GL_ShaderMaterial::setAttributeBuffer(int location, GLenum type, int offset, int tupleSize, int stride) const
{
    m_program->enableAttributeArray(location);
    m_program->setAttributeBuffer(location, type, offset, tupleSize, stride);
}

bool GL_ShaderMaterial::_setUniformValue(const QString & name, const QVariant & value) const
{
    int location = m_program->uniformLocation(name.toStdString().c_str());
    if (location < 0)
        return false;

    switch (value.type()) {
    case QVariant::Int:
        m_program->setUniformValue(location, value.toInt());
        break;
    case QVariant::Double:
        m_program->setUniformValue(location, value.toFloat());
        break;
    case QVariant::Vector2D:
        m_program->setUniformValue(location, value.value<QVector2D>());
        break;
    case QVariant::Vector3D:
        m_program->setUniformValue(location, value.value<QVector3D>());
        break;
    case QVariant::Vector4D:
        m_program->setUniformValue(location, value.value<QVector4D>());
        break;
    case QVariant::Matrix4x4:
        m_program->setUniformValue(location, value.value<QMatrix4x4>());
        break;
    case QVariant::Color: {
        QColor color = value.value<QColor>();
        m_program->setUniformValue(location, QVector4D(static_cast<float>(color.redF()),
                                                       static_cast<float>(color.greenF()),
                                                       static_cast<float>(color.blueF()),
                                                       static_cast<float>(color.alphaF())));
    } break;
    default:
        return false;
    }
    return true;
}
