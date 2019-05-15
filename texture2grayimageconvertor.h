#ifndef TEXTURE2GRAYIMAGECONVERTOR_H
#define TEXTURE2GRAYIMAGECONVERTOR_H

#include <tuple>

#include <QSize>
#include <QSharedPointer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QMatrix4x4>

#include <opencv2/core.hpp>

#include "gl/gl_mesh.h"
#include "gl/gl_shadermaterial.h"

class Texture2GrayImageConvertor
{
public:
    Texture2GrayImageConvertor();

    cv::Mat read(QOpenGLFunctions * gl,
                 GLuint textureId,
                 QSize textureSize,
                 const QSize & maxImageSize,
                 int orientation = 0,
                 bool flipHorizontally = false);

    std::tuple<cv::Mat, QVector2D> read_cropped(QOpenGLFunctions * gl,
                                                GLuint textureId,
                                                QSize textureSize,
                                                const QSize & maxImageSize,
                                                float viewAspect,
                                                int orientation = 0,
                                                bool flipHorizontally = false);

private:
    GL_MeshPtr m_quad;
    GL_ShaderMaterialPtr m_materialColor;
    GL_ShaderMaterialPtr m_materialPackedGray;
    QSharedPointer<QOpenGLFramebufferObject> m_fboColor;
    QSharedPointer<QOpenGLFramebufferObject> m_fboPackedGray;

    QSize _getImageSize(const QSize & textureSize,
                        QSize maxImageSize, bool k4fixFlag) const;
    QMatrix4x4 _getRotation(int orientation) const;
    QMatrix4x4 _getViewMatrix(float scaleX, float scaleY) const;

    QMatrix4x4 _getMatrixMVP(int orientation, bool flipHorizontally) const;


};

#endif // TEXTURE2GRAYIMAGECONVERTOR_H
