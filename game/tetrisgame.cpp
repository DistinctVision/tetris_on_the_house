#include "tetrisgame.h"

#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace Eigen;

const Vector2i TetrisGame::figureAnchor = Vector2i(2, 2);

TetrisGame::TetrisGame(const Vector2i & size):
    m_field(size.x(), size.y() + Figure::ColsAtCompileTime),
    m_rnd(0, 10000)
{
    _generateFigureSet();
    reset();
}

Vector2i TetrisGame::size() const
{
    return Vector2i(m_field.cols(), m_field.rows());
}

void TetrisGame::reset()
{
    m_field.setZero();
    m_currentAction.type = ActionType::FigureEntry;
    m_userAction.type = ActionType::NoAction;
    m_currentFigurePos.setZero();
    m_currentFigure.setZero();
    _updateRandom();
    m_nextFigure = _createRandomFigure();
}

void TetrisGame::_updateRandom()
{
    m_rnd_gen.seed(static_cast<unsigned int>(duration_cast<milliseconds>(
                                                 system_clock::now().time_since_epoch()).count()));
}

TetrisGame::Figure TetrisGame::_createRandomFigure() const
{
    Figure figure = m_figureSet[m_rnd(m_rnd_gen) % m_figureSet.size()];
    size_t n = m_rnd(m_rnd_gen) % 3;
    for (size_t i = 0; i < n; ++i)
    {
        figure = _rotated(figure);
    }
    return figure;
}

bool TetrisGame::_checkAction(TetrisGame::ActionType type)
{
    switch (type)
    {
    case ActionType::NoAction:
    case ActionType::FigureEntry: {
        m_currentFigure = m_nextFigure;
        m_nextFigure = _createRandomFigure();
        pair<Vector2i, Vector2i> bb = _getFigureBoundedBox(m_nextFigure);
        int begin_x = bb.first.x();
        int end_x = m_field.cols() - (bb.second.x());
        m_currentFigurePos.x() = begin_x + static_cast<int>(m_rnd(m_rnd_gen)) % (end_x - begin_x);
        m_currentFigurePos.y() = bb.first.y();
    } return true;
    case ActionType::LinesRemoval:
        return !m_fullLines.empty();
    case ActionType::FigureFall:
        return !_checkIntersect(m_currentFigure,
                                Vector2i(m_currentFigurePos.x(), m_currentFigurePos.y() - 1));
    case ActionType::MoveFigureLeft:
        return !_checkIntersect(m_currentFigure,
                                Vector2i(m_currentFigurePos.x() - 1, m_currentFigurePos.y()));
    case ActionType::MoveFigureRight:
        return !_checkIntersect(m_currentFigure,
                                Vector2i(m_currentFigurePos.x() + 1, m_currentFigurePos.y()));
    case ActionType::MoveFigureDown:
        return !_checkIntersect(m_currentFigure,
                                Vector2i(m_currentFigurePos.x(), m_currentFigurePos.y() - 1));
    case ActionType::RotateFigure:
        return !_checkIntersect(_rotated(m_currentFigure), m_currentFigurePos);
    default:
        break;
    }
    return false;
}

void TetrisGame::_generateFigureSet()
{
    Figure F;
    F << 0, 1, 1, 0, 0,
         0, 1, 1, 0, 0,
         0, 0, 0, 0, 0,
         0, 0, 0, 0, 0,
         0, 0, 0, 0, 0;
    m_figureSet.push_back(F);
    F << 0, 0, 1, 0, 0,
         0, 0, 1, 0, 0,
         0, 0, 1, 0, 0,
         0, 0, 1, 0, 0,
         0, 0, 0, 0, 0;
    m_figureSet.push_back(F);
    F << 0, 0, 0, 0, 0,
         0, 0, 1, 0, 0,
         0, 0, 1, 0, 0,
         0, 1, 1, 0, 0,
         0, 0, 0, 0, 0;
    m_figureSet.push_back(F);
    F << 0, 0, 0, 0, 0,
         0, 0, 1, 0, 0,
         0, 0, 1, 0, 0,
         0, 0, 1, 1, 0,
         0, 0, 0, 0, 0;
    m_figureSet.push_back(F);
    F << 0, 0, 0, 0, 0,
         0, 1, 0, 0, 0,
         0, 1, 1, 0, 0,
         0, 0, 1, 0, 0,
         0, 0, 0, 0, 0;
    m_figureSet.push_back(F);
    F << 0, 0, 0, 0, 0,
         0, 0, 1, 0, 0,
         0, 1, 1, 0, 0,
         0, 1, 0, 0, 0,
         0, 0, 0, 0, 0;
    m_figureSet.push_back(F);
    F << 0, 0, 0, 0, 0,
         0, 0, 1, 0, 0,
         0, 1, 1, 1, 0,
         0, 0, 0, 0, 0,
         0, 0, 0, 0, 0;
    m_figureSet.push_back(F);
}

bool TetrisGame::_doAction(const Action & action)
{
    switch (action.type)
    {
    case ActionType::NoAction:
        return false;
    case ActionType::FigureEntry:
    {
        m_currentFigure = ;
        return true;
    }
    case ActionType::LinesRemoval:
    {

    }
    case ActionType::FigureFall:
        return !_checkIntersect(m_currentFigure,
                                Vector2i(m_currentFigurePos.x(), m_currentFigurePos.y() - 1));
    case ActionType::MoveFigureLeft:
        return !_checkIntersect(m_currentFigure,
                                Vector2i(m_currentFigurePos.x() - 1, m_currentFigurePos.y()));
    case ActionType::MoveFigureRight:
        return !_checkIntersect(m_currentFigure,
                                Vector2i(m_currentFigurePos.x() + 1, m_currentFigurePos.y()));
    case ActionType::MoveFigureDown:
        return !_checkIntersect(m_currentFigure,
                                Vector2i(m_currentFigurePos.x(), m_currentFigurePos.y() - 1));
    case ActionType::RotateFigure:
        return !_checkIntersect(_rotated(m_currentFigure), m_currentFigurePos);
    default:
        break;
    }
    return false;
}

TetrisGame::Figure TetrisGame::_rotated(const TetrisGame::Figure & figure) const
{
    Figure rFigure;
    for (int i = 0; i < figure.rows(); ++i)
    {
        int y = i - figureAnchor.y();
        for (int j = 0; j < figure.cols(); ++j)
        {
            int x = j - figureAnchor.x();
            rFigure(i, j) = figure(figureAnchor.y() + x, figureAnchor.x() - y);
        }
    }
    return rFigure;
}

std::pair<Vector2i, Vector2i> TetrisGame::_getFigureBoundedBox(const TetrisGame::Figure & figure) const
{
    std::pair<Vector2i, Vector2i> bb(Vector2i(m_field.cols(), m_field.rows()),
                                     Vector2i(0, 0));
    for (int i = 0; i < figure.rows(); ++i)
    {
        for (int j = 0; j < figure.cols(); ++j)
        {
            if (figure(i, j) > 0)
            {
                if (j > bb.first.x())
                    bb.first.x() = j;
                if (i > bb.first.y())
                    bb.first.y() = i;
                if (j < bb.second.x())
                    bb.second.x() = j;
                if (i < bb.second.y())
                    bb.second.y() = i;
            }
        }
    }
    return bb;
}

bool TetrisGame::_checkIntersect(const TetrisGame::Figure & figure, const Vector2i & pos) const
{
    Vector2i begin = pos - figureAnchor;
    for (int i = 0; i < figure.rows(); ++i)
    {
        for (int j = 0; j < figure.cols(); ++j)
        {
            if (figure(i, j) > 0)
            {
                int y = begin.y() + i;
                if ((y < 0) || (y > m_field.rows()))
                    return true;
                int x = begin.x() + j;
                if ((x < 0) || (x > m_field.cols()))
                    return true;
                if (m_field(y, x) > 0)
                    return true;
            }
        }
    }
    return false;
}
