#include "texture2grayimageconvertor.h"

#include <QtMath>
#include <QMap>

#include <opencv2/imgproc.hpp>

#include "gl/gl_view.h"

Texture2GrayImageConvertor::Texture2GrayImageConvertor()
{
    QSharedPointer<QOpenGLShaderProgram> programPtr = GL_ViewRenderer::loadShader(":/shaders/texture.vsh",
                                                                                  ":/shaders/texture.fsh");
    m_materialColor = GL_ShaderMaterialPtr::create(programPtr,
                                                   QVariantMap({ { "matrixMVP", _getMatrixMVP(0, false) } }),
                                                   QMap<QString, GLuint>({ { "main_texture", 0 } }));
    programPtr = GL_ViewRenderer::loadShader(":/shaders/color.vsh",
                                             ":/shaders/packed_gray_image.fsh");
    m_materialPackedGray = GL_ShaderMaterialPtr::create(programPtr,
                                                        QVariantMap({ { "matrixMVP", _getMatrixMVP(0, false) },
                                                                      { "offsets[0]", 0 },
                                                                      { "offsets[1]", 1 },
                                                                      { "offsets[2]", 2 },
                                                                      { "offsets[3]", 3 } }),
                                                        QMap<QString, GLuint>({ { "main_texture", 0 } }));
    m_quad = GL_MeshPtr::create(GL_Mesh::createQuad());
}

cv::Mat Texture2GrayImageConvertor::read(QOpenGLFunctions * gl,
                                         GLuint textureId,
                                         QSize textureSize,
                                         const QSize & maxImageSize,
                                         int orientation, bool flipHorizontally)
{
    orientation = ((orientation / 90) % 4) * 90;
    if ((orientation == 90) || (orientation == 270))
        textureSize = QSize(textureSize.height(), textureSize.width());
    QSize imageSize = _getImageSize(textureSize, maxImageSize);
    if (!m_fboColor ||
            (m_fboColor->width() < imageSize.width()) ||
            (m_fboColor->height() < imageSize.height()))
    {
        m_fboColor = QSharedPointer<QOpenGLFramebufferObject>::create(imageSize);
        m_fboPackedGray = QSharedPointer<QOpenGLFramebufferObject>::create(imageSize.width() / 4,
                                                                           imageSize.height());
    }
    m_materialColor->setTexture("main_texture", textureId);
    m_materialColor->setValue("matrixMVP", _getMatrixMVP(orientation, flipHorizontally));
    m_fboColor->bind();
    gl->glViewport(0, 0, imageSize.width(), imageSize.height());
    gl->glDisable(GL_BLEND);
    gl->glDisable(GL_DEPTH_TEST);
    gl->glDepthMask(GL_FALSE);
    m_quad->draw(gl, *m_materialColor);
    m_fboPackedGray->bind();
    gl->glViewport(0, 0, imageSize.width() / 4, imageSize.height());
    gl->glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT);
    m_materialPackedGray->setTexture("main_texture", m_fboColor->texture());
    m_materialPackedGray->setValue("offsets[0]", 0);
    m_materialPackedGray->setValue("offsets[1]", 1);
    m_materialPackedGray->setValue("offsets[2]", 2);
    m_materialPackedGray->setValue("offsets[3]", 3);
    m_quad->draw(gl, *m_materialPackedGray);
    cv::Mat image(imageSize.height(), imageSize.width(), CV_8UC1);
    gl->glReadPixels(0, 0, imageSize.width() / 4, imageSize.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.data);
    //cv::Mat image(imageSize.height(), imageSize.width(), CV_8UC4);
    //gl->glReadPixels(0, 0, imageSize.width(), imageSize.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.data);
    //cv::cvtColor(image, image, cv::COLOR_RGBA2GRAY);
    gl->glDisable(GL_BLEND);
    m_fboPackedGray->release();
    return image;
}

QSize Texture2GrayImageConvertor::_getImageSize(const QSize & textureSize,
                                                QSize maxImageSize) const
{
    maxImageSize.setWidth((maxImageSize.width() / 4) * 4);
    if ((textureSize.width() < maxImageSize.width()) && (textureSize.height() < maxImageSize.height()))
    {
        if (textureSize.width() % 4 == 0)
            return textureSize;
        QSize imageSize;
        imageSize.setWidth((textureSize.width() / 4) * 4);
        float scale = imageSize.width() / static_cast<float>(textureSize.width());
        imageSize.setHeight(static_cast<int>(textureSize.height() * scale));
        return imageSize;
    }
    float aspect = textureSize.height() / static_cast<float>(textureSize.width());
    QSize imageSize(maxImageSize.width(), static_cast<int>(maxImageSize.width() * aspect));
    if (imageSize.height() > maxImageSize.height())
    {
        imageSize = QSize(static_cast<int>(maxImageSize.height() / aspect), maxImageSize.height());
        int width = (imageSize.width() / 4) * 4;
        float scale = width / static_cast<float>(imageSize.width());
        imageSize.setWidth(width);
        imageSize.setHeight(static_cast<int>(imageSize.height() * scale));
    }
    return imageSize;
}

QMatrix4x4 Texture2GrayImageConvertor::_getMatrixMVP(int orientation, bool flipHorizontally) const
{
    float signHorizontally = flipHorizontally ? -1.0f : 1.0f;

    QMatrix4x4 matrixMVP;
    matrixMVP(0, 0) = 2.0f * signHorizontally;
    matrixMVP(0, 3) = - 1.0f * signHorizontally;
    matrixMVP(1, 1) = - 2.0f;
    matrixMVP(1, 3) = 1.0f;
    matrixMVP(2, 2) = 1.0f;
    matrixMVP(2, 3) = 0.0f;
    matrixMVP(3, 3) = 1.0f;

    QMatrix4x4 rotation;
    switch (orientation) {
    case 90:
        rotation(0, 0) = 0.0f;
        rotation(0, 1) = -1.0f;
        rotation(0, 3) = 1.0f;
        rotation(1, 0) = 1.0f;
        rotation(1, 1) = 0.0f;
        rotation(1, 3) = 0.0f;
        break;
    case 180:
        rotation(0, 0) = -1.0f;
        rotation(0, 1) = 0.0f;
        rotation(0, 3) = 1.0f;
        rotation(1, 0) = 0.0f;
        rotation(1, 1) = -1.0f;
        rotation(1, 3) = 1.0f;
        break;
    case 270:
        rotation(0, 0) = 0.0f;
        rotation(0, 1) = 1.0f;
        rotation(0, 3) = 0.0f;
        rotation(1, 0) = -1.0f;
        rotation(1, 1) = 0.0f;
        rotation(1, 3) = 1.0f;
        break;
    }
    return matrixMVP * rotation;
}
