#include "tetrisscene.h"

#include <cassert>
#include <chrono>

#include <QQuaternion>

#include "objectedgestracker.h"
#include "texturereceiver.h"
#include "tetrisgame.h"

#include "scenes/gamestartscene.h"
#include "scenes/changecolorsscene.h"
#include "scenes/wavehousescene.h"
#include "scenes/transfromhousescene.h"
#include "scenes/glowedgesscene.h"
#include "scenes/defaultscene.h"
#include "scenes/finalscene.h"

using namespace std;
using namespace std::chrono;
using namespace Eigen;

TetrisScene::TetrisScene():
    m_tracker(nullptr),
    m_textureReceiver(nullptr),
    m_glowBuffer(nullptr),
    m_tempGlowBuffer(nullptr),
    m_rnd(0, 10000)
{
    m_game = QSharedPointer<TetrisGame>::create(Vector2i(8, 19));
    m_numberRemovalLines = m_game->numberRemovedLines();
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
    if (m_currentScene)
        m_currentScene->setTextureReceiver(m_textureReceiver);
    emit textureReceiverChanged();
}

void TetrisScene::init(GL_ViewRenderer * view)
{
    float k_floor = 2.7f;

    m_house = HouseObjectPtr::create(view,
                                     Vector3i(8, 19, 1),
                                     Vector3f(-11.0f, 0.0f, -4.0f),
                                     Vector3f(11.0f, 19.0f * k_floor, 4.0f));

    m_startScene->init(view);
    m_startScene->setHouse(m_house);
}

void TetrisScene::destroy(GL_ViewRenderer * view)
{
    Q_UNUSED(view);
    m_startScene->destroy(view);
    if (m_currentScene)
    {
        m_currentScene->destroy(view);
        m_currentScene.reset();
    }
    m_house.reset();
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

    m_house->setActivityLevel(std::max(m_house->activityLevel() - 0.1f, 0.0f));

    QMatrix4x4 viewMatrix = m_tracker->viewMatrix();

    if (m_numberRemovalLines != m_game->numberRemovedLines())
    {
        m_numberRemovalLines = m_game->numberRemovedLines();
        m_house->setActivityLevel(1.0f);
    }

    if (!m_startScene->animationIsFinished() && false)
    {
        m_startScene->setViewMatrix(viewMatrix);
        m_startScene->drawAndPlay(view);

        view->glEnable(GL_BLEND);
        m_house->drawBlocks(view, m_game.get(), viewMatrix, 0.8f, m_startScene->timeState());
    }
    else
    {
        if (m_game->step() == TetrisGame::EventType::Lose)
        {
            m_game->reset();
        }

        if (!m_currentScene)
        {
            m_currentScene = _createRandomScene();
            m_currentScene->init(view);
            m_currentScene->setTextureReceiver(m_textureReceiver);
            m_currentScene->setHouse(m_house);
        }
        m_currentScene->setViewMatrix(viewMatrix);
        m_currentScene->drawAndPlay(view);
        if (m_currentScene->animationIsFinished())
        {
            m_currentScene->destroy(view);
            m_currentScene.reset();
        }

        view->glEnable(GL_BLEND);
        m_house->drawBlocks(view, m_game.get(), viewMatrix, 0.8f, 1.0f);
    }
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

QSharedPointer<AnimationScene> TetrisScene::_createRandomScene() const
{
    return QSharedPointer<FinalScene>::create(200);

    m_rnd_gen.seed(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    int index = m_rnd(m_rnd_gen) % 6;
    //index = 3;
    QSharedPointer<AnimationScene> scene;
    switch (index) {
    case 0: {
        scene = QSharedPointer<ChangeColorsScene>::create(50 + m_rnd(m_rnd_gen) % 50,
                                                          1 + m_rnd(m_rnd_gen) % 2);
    } break;
    case 1: {
        int duration = 100 + m_rnd(m_rnd_gen) % 100;
        QVector3D wave_origin(float(m_rnd(m_rnd_gen) % 30) - 15.0f,
                              float(m_rnd(m_rnd_gen) % 30),
                              0.0f);
        float wave_timeScale = float(m_rnd(m_rnd_gen) % 3 + 3);
        float wave_distanceStep = float(m_rnd(m_rnd_gen) % 10 + 20);
        float wave_distanceStepScale = float(m_rnd(m_rnd_gen) % 2 + 1);
        float wave_scale = float(m_rnd(m_rnd_gen) % 2 + 1);
        scene = QSharedPointer<WaveHouseScene>::create(duration, wave_origin,
                                                       wave_timeScale, wave_distanceStep,
                                                       wave_distanceStepScale, wave_scale);
    } break;
    case 2: {
        int duration = 60 + m_rnd(m_rnd_gen) % 60;
        float delta_z = float(m_rnd(m_rnd_gen) % 3 + 2) * 0.99f;
        scene = QSharedPointer<TransfromHouseScene>::create(duration, delta_z);
    } break;
    case 3: {
        int duration = 100 + m_rnd(m_rnd_gen) % 60;
        float pitch = m_rnd(m_rnd_gen) % 360;
        float yaw = m_rnd(m_rnd_gen) % 360;
        float roll = m_rnd(m_rnd_gen) % 360;
        QQuaternion q = QQuaternion::fromEulerAngles(pitch, yaw, roll);
        scene = QSharedPointer<GlowEdgesScene>::create(duration, q.rotatedVector(QVector3D(1.0f, 0.0f, 0.0f)));
    } break;
    case 4:
    case 5: {
        scene = QSharedPointer<DefaultScene>::create(50 + m_rnd(m_rnd_gen) % 50);
    } break;
    default:
        assert(false);
    }
    return scene;
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
