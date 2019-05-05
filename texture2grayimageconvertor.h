#ifndef TEXTURE2GRAYIMAGECONVERTOR_H
#define TEXTURE2GRAYIMAGECONVERTOR_H

#include <QSize>
#include <QSharedPointer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>

#include <opencv2/core.hpp>

#include "gl/gl_mesh.h"
#include "gl/gl_shadermaterial.h"

class Texture2GrayImageConvertor
{
public:
    Texture2GrayImageConvertor();

    cv::Mat read(QOpenGLFunctions * gl,
                 GLuint textureId,
                 const QSize & textureSize,
                 const QSize & maxImageSize);

private:
    GL_MeshPtr m_quad;
    GL_ShaderMaterialPtr m_material;
    QSharedPointer<QOpenGLFramebufferObject> m_fbo;

    QSize _getImageSize(const QSize & textureSize,
                        QSize maxImageSize) const;
};

#endif // TEXTURE2GRAYIMAGECONVERTOR_H
