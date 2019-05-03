#include "gl_scene.h"

GL_Scene::GL_Scene():
    m_enabled(true)
{}

GL_Scene::~GL_Scene()
{}

bool GL_Scene::enabled() const
{
    return m_enabled;
}

void GL_Scene::setEnabled(bool enabled)
{
    m_enabled = enabled;
    emit enabledChanged();
}
