#include "testcubescene.h"
#include <QDebug>

#include "objectedgestracker.h"

TestCubeScene::TestCubeScene():
    m_tracker(nullptr)
{
}

ObjectEdgesTracker * TestCubeScene::objectEdgesTracker() const
{
    return m_tracker;
}

void TestCubeScene::setObjectEdgesTracker(ObjectEdgesTracker * tracker)
{
    if (tracker == m_tracker)
        return;
    m_tracker = tracker;
    emit objectEdgesTrackerChanged();
}

void TestCubeScene::init(GL_ViewRenderer * view)
{
    m_cube = GL_WorldObjectPtr::create(GL_MeshPtr::create(GL_Mesh::createCubikRubik()),
                                       view->createMaterial(MaterialType::ContourFallOff));
}

void TestCubeScene::destroy(GL_ViewRenderer * view)
{
    Q_UNUSED(view);
    m_cube.reset();
}

void TestCubeScene::draw(GL_ViewRenderer * view)
{
    if (!m_tracker)
    {
        qCritical() << Q_FUNC_INFO << "Object edges tracker is not set!";
        return;
    }
    view->glEnable(GL_BLEND);
    view->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_cube->draw(view, m_tracker->viewMatrix());
}
