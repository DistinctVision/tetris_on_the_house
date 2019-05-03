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

void TestCubeScene::setObjectEdgesTracker(QObject * tracker)
{
    ObjectEdgesTracker * d_object = dynamic_cast<ObjectEdgesTracker*>(tracker);
    if (d_object != tracker)
        qFatal((QString(Q_FUNC_INFO) + "Wrong object edges tracker").toStdString().c_str());
    if (d_object == m_tracker)
        return;
    m_tracker = d_object;
    emit objectEdgesTrackerChanged();
}

void TestCubeScene::init(GL_ViewRenderer * view)
{
    m_cube = GL_WorldObjectPtr::create(GL_MeshPtr::create(GL_Mesh::createCube()),
                                       view->createMaterial(MaterialType::MT_Texture));
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
    m_cube->draw(view, m_tracker->viewMatrix());
}
