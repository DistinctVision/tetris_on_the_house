#include "texturereceiver.h"

TextureReceiver::TextureReceiver():
    m_textureId(0),
    m_textureSize(-1, -1),
    m_orientation(0)
{
}

QMatrix4x4 TextureReceiver::getRotationImageMatrix(int orientation)
{
    orientation = ((orientation / 90) % 4) * 90;
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
    return rotation;
}
GLuint TextureReceiver::textureId() const
{
    return m_textureId;
}

QSize TextureReceiver::textureSize() const
{
    return m_textureSize;
}

int TextureReceiver::orientation() const
{
    return m_orientation;
}

void TextureReceiver::setTextureId(GLuint textureId, const QSize & textureSize, int orientation)
{
    if (m_textureId == textureId)
        return;
    m_textureId = textureId;
    m_textureSize = textureSize;
    m_orientation = ((orientation / 90) % 4) * 90;
    emit textureChanged();
}
