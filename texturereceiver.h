#ifndef TEXTURERECEIVER_H
#define TEXTURERECEIVER_H

#include <QObject>
#include <QSize>
#include <QMatrix4x4>

#include <QtOpenGL/QGL>

class TextureReceiver: public QObject
{
    Q_OBJECT

public:
    TextureReceiver();

    static QMatrix4x4 getRotationImageMatrix(int orientation);

    GLuint textureId() const;
    QSize textureSize() const;
    int orientation() const;
    void setTextureId(GLuint textureId, const QSize & textureSize, int orientation);

signals:
    void textureChanged();

private:
    GLuint m_textureId;
    QSize m_textureSize;
    int m_orientation;
};

#endif // TEXTURERECEIVER_H
