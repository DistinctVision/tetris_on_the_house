#include "gl_debugimagescene.h"
#include <cassert>

#include "framehandler.h"

GL_DebugImageScene::GL_DebugImageScene():
    m_gl(nullptr),
    m_debugTextureId(0),
    m_frameHandler(nullptr)
{
}

GL_DebugImageScene::~GL_DebugImageScene()
{
    destroy();
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

void GL_DebugImageScene::init()
{
    if (!m_gl)
    {
        m_gl = new QOpenGLFunctions();
        m_gl->initializeOpenGLFunctions();

        m_gl->glGenTextures(1, &m_debugTextureId);
    }
}

void GL_DebugImageScene::destroy()
{
    if (m_gl)
    {
        if (m_debugTextureId != 0)
        {
            m_gl->glDeleteTextures(1, &m_debugTextureId);
            m_debugTextureId = 0;
        }

        delete m_gl;
        m_gl = nullptr;
    }
}

void GL_DebugImageScene::draw()
{
    assert(m_gl);
    if (m_frameHandler == nullptr)
        return;
}
