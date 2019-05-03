#ifndef TESTCUBESCENE_H
#define TESTCUBESCENE_H

#include "gl/gl_scene.h"
#include "gl/gl_worldobject.h"

class ObjectEdgesTracker;

class TestCubeScene:
        public GL_Scene
{
    Q_OBJECT

    Q_PROPERTY(QObject* objectEdgesTracker READ objectEdgesTracker WRITE setObjectEdgesTracker NOTIFY objectEdgesTrackerChanged)
public:
    TestCubeScene();

    ObjectEdgesTracker * objectEdgesTracker() const;
    void setObjectEdgesTracker(QObject * objectEdgesTracker);

    void init(GL_ViewRenderer * view);
    void destroy(GL_ViewRenderer * view);
    void draw(GL_ViewRenderer * view);

signals:
    void objectEdgesTrackerChanged();

private:
    GL_WorldObjectPtr m_cube;
    ObjectEdgesTracker * m_tracker;
};

#endif // TESTCUBESCENE_H
