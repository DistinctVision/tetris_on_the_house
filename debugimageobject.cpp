#include "debugimageobject.h"

DebugImageObject::DebugImageObject():
    m_debugEnabled(false)
{
}

bool DebugImageObject::debugEnabled() const
{
    return m_debugEnabled;
}

void DebugImageObject::setDebugEnabled(bool debugEnabled)
{
    if (debugEnabled == m_debugEnabled)
        return;
    m_debugEnabled = debugEnabled;
    emit debugEnabledChangled();
}
