#include "tetrisscene.h"

#include "objectedgestracker.h"
#include "texturereceiver.h"
#include "tetrisgame.h"

using namespace Eigen;

TetrisScene::TetrisScene():
    m_tracker(nullptr),
    m_textureReceiver(nullptr),
    m_glowBuffer(nullptr),
    m_tempGlowBuffer(nullptr)
{
    m_game = std::make_shared<TetrisGame>(Vector2i(10, 30));
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
    emit textureReceiverChanged();
}

void TetrisScene::init(GL_ViewRenderer * view)
{
    m_house = HouseObjectPtr::create(view, Vector3i(10, 15, 5),
                                           Vector3f(20.0f, 30.0f, 3.0f),
                                           Vector3f(0.0f, 0.0f, 0.0f),
                                           Vector3f(0.0f, 0.5f, 0.0f));
    m_meshBlock = GL_MeshPtr::create(GL_Mesh::createQuad(QVector2D(1.0f, 1.0f),
                                                         QVector2D(0.0f, 0.0f), true));
    m_materialBlock = view->createMaterial(MaterialType::ContourFallOff);
}

void TetrisScene::destroy(GL_ViewRenderer * view)
{
    Q_UNUSED(view);
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
    if (m_game->step() == TetrisGame::EventType::Lose)
    {
        m_game->reset();
    }
    view->glEnable(GL_BLEND);
    view->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    QMatrix4x4 viewMatrix = m_tracker->viewMatrix();
    //m_house->draw(view, viewMatrix, m_textureReceiver->textureId(), m_textureReceiver->textureSize());
    _drawBlocks(view, view->projectionMatrix(), viewMatrix);
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

void TetrisScene::_drawBlocks(GL_ViewRenderer * view, const QMatrix4x4 & projMatrix, const QMatrix4x4 & viewMatrix)
{
    const float offset = 0.05f;

    std::pair<Vector3f, Vector3f> borders(m_house->borderFirst(), m_house->borderSecond());

    Vector3f fieldSize = m_house->size() - (borders.first + borders.second);
    Vector3i n_fieldSize = m_house->n_size();
    Vector2f blockSize(fieldSize.x() / static_cast<float>(n_fieldSize.x()),
                       fieldSize.y() / static_cast<float>(n_fieldSize.y()));
    QMatrix4x4 projViewMatrix = projMatrix * viewMatrix;
    QMatrix4x4 worldMatrix;
    worldMatrix(0, 0) = worldMatrix(1, 1) = std::min(blockSize.x(), blockSize.y()) * 0.9f;
    worldMatrix(2, 3) = - m_house->size().z() * 0.5f - offset;
    m_game->for_each_blocks([&, this] (const Vector2i & p) {
        worldMatrix(0, 3) = (p.x() + 1) * blockSize.x() - fieldSize.x() * 0.5f;
        worldMatrix(1, 3) = p.y() * blockSize.y() + borders.first.y();
        m_materialBlock->setValue("matrixMVP", projViewMatrix * worldMatrix);
        m_meshBlock->draw(view, *m_materialBlock);
    });

    if (m_game->currentFigureState() > 0.0f)
    {
        float x = (m_game->figurePos().x() - TetrisGame::figureAnchor.x() + 1) * blockSize.x() - fieldSize.x() * 0.5f;
        float y = (m_game->figurePos().y() - TetrisGame::figureAnchor.y()) * blockSize.y() + borders.first.y();
        if (m_game->currentFigureState() < 1.0f)
        {
            float t = 1.0f - m_game->currentFigureState();
            t *= t;
            t *= t;
            y += t * 30.0f;
        }
        TetrisGame::Figure figure = m_game->currentFigure();
        for (int i = 0; i < TetrisGame::Figure::RowsAtCompileTime; ++i)
        {
            worldMatrix(1, 3) = y + blockSize.y() * i;
            for (int j = 0; j < TetrisGame::Figure::ColsAtCompileTime; ++j)
            {
                if (figure(i, j) > 0)
                {
                    worldMatrix(0, 3) = x + blockSize.x() * j;
                    m_materialBlock->setValue("matrixMVP", projViewMatrix * worldMatrix);
                    m_meshBlock->draw(view, *m_materialBlock);
                }
            }
        }
    }
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
