#ifndef GL_SCENE_H
#define GL_SCENE_H

#include <QObject>

class GL_Scene:
        public QObject
{
    Q_OBJECT

public:
    GL_Scene();
    virtual ~GL_Scene() {}

    virtual void init() = 0;
    virtual void draw() = 0;
    virtual void destroy() = 0;
};

#endif // GL_SCENE_H
