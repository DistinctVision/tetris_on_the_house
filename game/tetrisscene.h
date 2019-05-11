#ifndef TETRISSCENE_H
#define TETRISSCENE_H

#include "gl/gl_scene.h"
#include "gl/gl_worldobject.h"
#include "houseobject.h"

class ObjectEdgesTracker;

class TetrisScene:
        public GL_Scene
{
    Q_OBJECT

    Q_PROPERTY(ObjectEdgesTracker* objectEdgesTracker READ objectEdgesTracker WRITE setObjectEdgesTracker
               NOTIFY objectEdgesTrackerChanged)
public:
    TetrisScene();

    ObjectEdgesTracker * objectEdgesTracker() const;
    void setObjectEdgesTracker(ObjectEdgesTracker * objectEdgesTracker);

    void init(GL_ViewRenderer * view);
    void destroy(GL_ViewRenderer * view);
    void draw(GL_ViewRenderer * view);

signals:
    void objectEdgesTrackerChanged();

private:
    HouseObjectPtr m_house;
    ObjectEdgesTracker * m_tracker;
};

#endif // TETRISSCENE_H
