#ifndef TETRISSCENE_H
#define TETRISSCENE_H

#include "gl/gl_scene.h"
#include "gl/gl_screenobject.h"
#include "gl/gl_worldobject.h"
#include "houseobject.h"

class ObjectEdgesTracker;
class TextureReceiver;

class TetrisScene:
        public GL_Scene
{
    Q_OBJECT

    Q_PROPERTY(ObjectEdgesTracker* objectEdgesTracker READ objectEdgesTracker WRITE setObjectEdgesTracker
               NOTIFY objectEdgesTrackerChanged)
    Q_PROPERTY(TextureReceiver* textureReceiver READ textureReceiver WRITE setTextureReceiver
               NOTIFY textureReceiverChanged)
public:
    TetrisScene();

    ObjectEdgesTracker * objectEdgesTracker() const;
    void setObjectEdgesTracker(ObjectEdgesTracker * objectEdgesTracker);

    TextureReceiver * textureReceiver() const;
    void setTextureReceiver(TextureReceiver * textureReceiver);

    void init(GL_ViewRenderer * view);
    void destroy(GL_ViewRenderer * view);
    void draw(GL_ViewRenderer * view);

signals:
    void objectEdgesTrackerChanged();
    void textureReceiverChanged();

private:
    HouseObjectPtr m_house;
    ObjectEdgesTracker * m_tracker;
    TextureReceiver * m_textureReceiver;
};

#endif // TETRISSCENE_H
