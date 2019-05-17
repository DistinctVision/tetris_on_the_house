#include "tetrisscene.h"

#include "objectedgestracker.h"
#include "texturereceiver.h"
#include "tetrisgame.h"

#include "animationscene.h"
#include "gamestartscene.h"

using namespace Eigen;

TetrisScene::TetrisScene():
    m_tracker(nullptr),
    m_textureReceiver(nullptr),
    m_houseColorK_a(1.0f, 1.0f, 1.0f),
    m_houseColorK_b(0.0f, 0.0f, 0.0f),
    m_glowBuffer(nullptr),
    m_tempGlowBuffer(nullptr)
{
    m_game = QSharedPointer<TetrisGame>::create(Vector2i(8, 19));
    m_startScene = QSharedPointer<GameStartScene>::create(120);
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

TextureReceiver * TetrisScene::textureReceiver() const
{
    return m_textureReceiver;
}

void TetrisScene::setTextureReceiver(TextureReceiver * textureReceiver)
{
    if (m_textureReceiver == textureReceiver)
        return;
    m_textureReceiver = textureReceiver;
    m_startScene->setTextureReceiver(m_textureReceiver);
    emit textureReceiverChanged();
}

void TetrisScene::init(GL_ViewRenderer * view)
{
    float k_floor = 2.7f;

    m_house = HouseObjectPtr::create(view,
                                     Vector3i(8, 19, 1),
                                     Vector3f(-11.0f, 0.0f, -4.0f),
                                     Vector3f(11.0f, 19.0f * k_floor, 4.0f));
    m_houseDefaultMaterial = view->createMaterial(MaterialType::ScreenMorph_glowEdges);
    m_houseColorK_a = QVector3D(1.0f, 1.0f, 1.0f);
    m_houseColorK_b = QVector3D(0.0f, 0.0f, 0.0f);

    m_startScene->init(view);
    m_startScene->setHouse(m_house);
}

void TetrisScene::destroy(GL_ViewRenderer * view)
{
    Q_UNUSED(view);
    m_startScene->destroy(view);
    m_house.reset();
    m_houseDefaultMaterial.reset();
    if (m_glowBuffer != nullptr)
    {
        delete m_glowBuffer;
        m_glowBuffer = nullptr;
        delete m_tempGlowBuffer;
        m_tempGlowBuffer = nullptr;
    }
}

void TetrisScene::draw(GL_ViewRenderer * view)
{
    if (!m_tracker)
    {
        qCritical() << Q_FUNC_INFO << ": Object edges tracker is not set!";
        return;
    }
    if (!m_textureReceiver)
    {
        qCritical() << Q_FUNC_INFO << ": texture receiver is not set!";
        return;
    }
    if (m_game->step() == TetrisGame::EventType::Lose)
    {
        m_game->reset();
    }

    m_house->setActivityLevel(std::max(m_house->activityLevel() - 0.01f, 0.0f));

    QMatrix4x4 viewMatrix = m_tracker->viewMatrix();

    m_startScene->setViewMatrix(viewMatrix);
    m_startScene->drawAndPlay(view);
    if (m_startScene->animationIsFinished())
        m_startScene->resetTime();

    view->glEnable(GL_BLEND);

    m_house->drawBlocks(view, m_game.get(), viewMatrix);
}

bool TetrisScene::moveFigureLeft()
{
    return m_game->moveFigureLeft();
}

bool TetrisScene::moveFigureRight()
{
    return m_game->moveFigureRight();
}

bool TetrisScene::moveFigureDown()
{
    return m_game->moveFigureDown();
}

bool TetrisScene::rotateFigure()
{
    return m_game->rotateFigure();
}

void TetrisScene::_drawGlow(GL_ViewRenderer * view)
{
    QSize viewportSize = view->viewportSize();
    if ((m_glowBuffer == nullptr) ||
            (m_glowBuffer->width() < viewportSize.width()) ||
            (m_glowBuffer->height() << viewportSize.height()))
    {
        if (m_glowBuffer != nullptr)
            delete m_glowBuffer;
        m_glowBuffer = new QOpenGLFramebufferObject(viewportSize);
    }
    if ((m_tempGlowBuffer == nullptr) ||
            (m_tempGlowBuffer->width() < viewportSize.width()) ||
            (m_tempGlowBuffer->height() << viewportSize.height()))
    {
        if (m_tempGlowBuffer != nullptr)
            delete m_tempGlowBuffer;
        m_tempGlowBuffer = new QOpenGLFramebufferObject(viewportSize);
    }
}
