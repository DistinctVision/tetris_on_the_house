#include "gl_debugimagescene.h"
#include <cassert>

#include <Eigen/Eigen>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include "debugimageobject.h"

using namespace Eigen;

GL_DebugImageScene::GL_DebugImageScene():
    m_debugImageObject(nullptr),
    m_debugTextureId(0),
    m_screenQuad(nullptr),
    m_fillMode(FillMode::PreserveAspectCrop)
{
}

GL_DebugImageScene::~GL_DebugImageScene()
{
    assert(m_debugTextureId == 0);
}

DebugImageObject * GL_DebugImageScene::debugImageObject() const
{
    return m_debugImageObject;
}

void GL_DebugImageScene::setDebugImageObject(QObject * debugImageObject)
{
    DebugImageObject * d_object = dynamic_cast<DebugImageObject*>(debugImageObject);
    if (d_object != debugImageObject)
        qFatal((QString(Q_FUNC_INFO) + "Wrong debug image object").toStdString().c_str());
    if (d_object == m_debugImageObject)
        return;
    m_debugImageObject = d_object;
    emit debugImageObjectChanged();
}

FillMode::Enum GL_DebugImageScene::fillMode() const
{
    return m_fillMode;
}

void GL_DebugImageScene::setFillMode(FillMode::Enum mode)
{
    if (m_fillMode == mode)
        return;
    m_fillMode = mode;
    emit fillModeChanged();
}

void GL_DebugImageScene::init(GL_ViewRenderer * view)
{
    if (m_debugTextureId == 0)
    {
        view->glGenTextures(1, &m_debugTextureId);
        m_debugTextureSize = QSize(-1, -1);
        m_screenQuad = GL_ScreenObjectPtr::create(GL_MeshPtr::create(GL_Mesh::createQuad()),
                                                  view->createMaterial(MaterialType::MT_Texture));
        m_screenQuad->material()->setTexture("main_texture", m_debugTextureId);
    }
}

void GL_DebugImageScene::destroy(GL_ViewRenderer * view)
{
    if (m_debugTextureId != 0)
    {
        m_screenQuad.reset();
        view->glDeleteTextures(1, &m_debugTextureId);
        m_debugTextureId = 0;
        m_debugTextureSize = QSize(-1, -1);
    }
}

void GL_DebugImageScene::draw(GL_ViewRenderer * view)
{
    cv::Mat image = (m_debugImageObject != nullptr) ? m_debugImageObject->debugImage() : cv::Mat();
    if (image.empty())
        return;
    if (image.type() != CV_8UC4)
    {
        switch (image.channels()) {
        case 1:
            cv::cvtColor(image, image, cv::COLOR_GRAY2RGBA);
            break;
        case 3:
            cv::cvtColor(image, image, cv::COLOR_RGB2RGBA);
            break;
        default:
            qFatal((QString(Q_FUNC_INFO) + ": wrong image format").toStdString().c_str());
            return;
        }
    }
    view->glBindTexture(GL_TEXTURE_2D, m_debugTextureId);
    if ((image.cols != m_debugTextureSize.width()) || (image.rows != m_debugTextureSize.height()))
    {
        m_debugTextureSize = QSize(image.cols, image.rows);
        view->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, m_debugTextureSize.width(), m_debugTextureSize.height(), 0,
                           GL_RGBA, GL_UNSIGNED_BYTE, image.data);
    }
    else
    {
        view->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_debugTextureSize.width(), m_debugTextureSize.height(),
                              GL_RGBA, GL_UNSIGNED_BYTE, image.data);
    }

    view->glDisable(GL_DEPTH_TEST);
    view->glDepthMask(GL_FALSE);
    m_screenQuad->setOrigin(Vector2f(0.0f, 0.0f));
    m_screenQuad->setSize(Vector2f(image.cols, image.rows));
    m_screenQuad->setFillMode(m_fillMode);
    m_screenQuad->draw(view);
}
