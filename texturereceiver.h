#ifndef TEXTURERECEIVER_H
#define TEXTURERECEIVER_H

#include <QObject>
#include <QSize>

#include <QtOpenGL/QGL>

class TextureReceiver: public QObject
{
    Q_OBJECT

public:
    TextureReceiver();

    GLuint textureId() const;
    QSize textureSize() const;
    void setTextureId(GLuint textureId, const QSize & textureSize);

signals:
    void textureChanged();

private:
    GLuint m_textureId;
    QSize m_textureSize;
};

#endif // TEXTURERECEIVER_H
