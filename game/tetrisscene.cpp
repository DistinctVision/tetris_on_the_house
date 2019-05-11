#include "tetrisscene.h"

#include "objectedgestracker.h"

using namespace Eigen;

TetrisScene::TetrisScene():
    m_tracker(nullptr)
{
}

ObjectEdgesTracker * TetrisScene::objectEdgesTracker() const
{
    return m_tracker;
}

void TetrisScene::setObjectEdgesTracker(ObjectEdgesTracker * objectEdgesTracker)
{
    if (m_tracker == objectEdgesTracker)
        return;
    m_tracker = objectEdgesTracker;
    emit objectEdgesTrackerChanged();
}

void TetrisScene::init(GL_ViewRenderer * view)
{
    m_house = HouseObjectPtr::create(view, Vector3i(10, 18, 5),
                                           Vector3f(11.0f, 18.5f, 6.0f),
                                           Vector3f(0.5f, 0.0f, 0.5f),
                                           Vector3f(0.5f, 0.5f, 0.5f));
}

void TetrisScene::destroy(GL_ViewRenderer * view)
{
    Q_UNUSED(view);
    m_house.reset();
}

void TetrisScene::draw(GL_ViewRenderer * view)
{
    if (!m_tracker)
    {
        qCritical() << Q_FUNC_INFO << "Object edges tracker is not set!";
        return;
    }
    view->glEnable(GL_BLEND);
    view->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_house->draw(view, m_tracker->viewMatrix());
}
