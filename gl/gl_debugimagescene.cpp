#include "gl_debugimagescene.h"
#include <cassert>

#include "framehandler.h"

GL_DebugImageScene::GL_DebugImageScene():
    m_debugTextureId(0),
    m_frameHandler(nullptr)
{
}

GL_DebugImageScene::~GL_DebugImageScene()
{
    assert(m_debugTextureId == 0);
}

FrameHandler * GL_DebugImageScene::frameHandler() const
{
    return m_frameHandler;
}

void GL_DebugImageScene::setFrameHandler(FrameHandler * frameHandler)
{
    m_frameHandler = frameHandler;
    emit frameHandlerChanged();
}

void GL_DebugImageScene::init(QOpenGLFunctions * gl)
{
    if (m_debugTextureId == 0)
    {
        gl->glGenTextures(1, &m_debugTextureId);
    }
}

void GL_DebugImageScene::destroy(QOpenGLFunctions * gl)
{
    if (m_debugTextureId != 0)
    {
        gl->glDeleteTextures(1, &m_debugTextureId);
        m_debugTextureId = 0;
    }
}

void GL_DebugImageScene::draw(GL_ViewRenderer * view)
{
    if (m_frameHandler == nullptr)
        return;
}
