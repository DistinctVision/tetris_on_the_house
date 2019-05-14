#include "tetrisscene.h"

#include "objectedgestracker.h"
#include "texturereceiver.h"
#include "tetrisgame.h"

using namespace Eigen;

TetrisScene::TetrisScene():
    m_tracker(nullptr),
    m_textureReceiver(nullptr)
{
    m_game = std::make_shared<TetrisGame>(Vector2i(8, 18));
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
    m_house = HouseObjectPtr::create(view, Vector3i(10, 18, 5),
                                           Vector3f(11.0f, 18.5f, 6.0f),
                                           Vector3f(0.0f, 0.0f, 0.5f),
                                           Vector3f(0.0f, 0.5f, 0.5f));
    m_meshBlock = GL_MeshPtr::create(GL_Mesh::createQuad(QVector2D(1.0f, 1.0f)));
    m_materialBlock = view->createMaterial(MaterialType::ContourFallOff);
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
        qCritical() << Q_FUNC_INFO << ": Object edges tracker is not set!";
        return;
    }
    if (!m_textureReceiver)
    {
        qCritical() << Q_FUNC_INFO << ": texture receiver is not set!";
        return;
    }
    m_game->step();
    view->glEnable(GL_BLEND);
    view->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_house->draw(view, m_tracker->viewMatrix(),
                  m_textureReceiver->textureId(), m_textureReceiver->textureSize());
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
    Vector3f fieldSize = m_house->size() - (m_house->borderFirst() + m_house->borderSecond());
    Vector3i n_fieldSize = m_house->n_size();
    Vector2f blockSize(fieldSize.x() / static_cast<float>(n_fieldSize.x()),
                       fieldSize.y() / static_cast<float>(n_fieldSize.y()));
    QMatrix4x4 projViewMatrix = projMatrix * viewMatrix;
    QMatrix4x4 worldMatrix;
    worldMatrix(0, 0) = worldMatrix(1, 1) = std::min(blockSize.x(), blockSize.y()) * 0.9f;
    worldMatrix(0, 3) = - m_house->size().z() * 0.5f;
    m_game->for_each_blocks([&, this] (const Vector2i & p) {
        worldMatrix(0, 3) = p.x() * blockSize.x() - fieldSize.x() * 0.5f;
        worldMatrix(1, 3) = p.y() * blockSize.y() - fieldSize.y() * 0.5f;
        m_materialBlock->setValue("matrixMVP", projViewMatrix * worldMatrix);
        m_meshBlock->draw(view, *m_materialBlock);
    });

    if (m_game->currentFigureState() > 0.0f)
    {
        float x = (m_game->figurePos().x() - TetrisGame::figureAnchor.x()) * blockSize.x() - fieldSize.x() * 0.5f;
        float y = (m_game->figurePos().y() - TetrisGame::figureAnchor.y()) * blockSize.y() - fieldSize.y() * 0.5f;
        if (m_game->currentFigureState() < 1.0f)
        {
            float t = 1.0f - m_game->currentFigureState();
            t *= t;
            t *= t;
            y -= (1.0f - t) * 100.0f;
        }
        for (int i = 0; i < TetrisGame::Figure::RowsAtCompileTime; ++i)
        {
            worldMatrix(1, 3) = y + blockSize.y() * i;
            for (int j = 0; j < TetrisGame::Figure::ColsAtCompileTime; ++j)
            {
                worldMatrix(0, 3) = x + blockSize.x() * j;
                m_materialBlock->setValue("matrixMVP", projViewMatrix * worldMatrix);
                m_meshBlock->draw(view, *m_materialBlock);
            }
        }
    }
}
