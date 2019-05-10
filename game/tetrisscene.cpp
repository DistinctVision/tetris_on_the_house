#include "tetrisscene.h"

#include "objectedgestracker.h"

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
    m_house = GL_WorldObjectPtr::create(GL_MeshPtr::create(GL_Mesh::createCube(QVector3D(11.0f, 18.0f, 6.0f))),
                                        view->createMaterial(MaterialType::ContourFallOff));
    QMatrix4x4 transfrom;
    transfrom.translate(0.0f, 9.0f, 0.0f);
    m_house->setWorldMatrix(transfrom);
    m_house->material()->setValue("border_size", 0.01f);
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
