#include "texturereceiver.h"

TextureReceiver::TextureReceiver():
    m_textureId(0)
{
}

GLuint TextureReceiver::textureId() const
{
    return m_textureId;
}

QSize TextureReceiver::textureSize() const
{
    return m_textureSize;
}

void TextureReceiver::setTextureId(GLuint textureId, const QSize & textureSize)
{
    if (m_textureId == textureId)
        return;
    m_textureId = textureId;
    m_textureSize = textureSize;
    emit textureChanged();
}
