#include "tetrisgame.h"

#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace Eigen;

const Vector2i TetrisGame::figureAnchor = Vector2i(2, 2);

TetrisGame::TetrisGame(const Vector2i & fieldSize):
    m_fieldSize(fieldSize),
    m_field(fieldSize.x(), fieldSize.y() + Figure::ColsAtCompileTime),
    m_moveField(m_field.rows(), m_field.cols()),
    m_rnd(0, 10000)
{
    _generateFigureSet();
    reset();
}

Vector2i TetrisGame::fieldSize() const
{
    return m_fieldSize;
}

void TetrisGame::reset()
{
    m_field.setZero();
    m_moveField.setZero();
    m_currentAction.type = ActionType::FigureEntry;
    m_userAction.type = ActionType::NoAction;
    m_figurePos_current.setZero();
    m_figurePos_next = m_figurePos_current;
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

void TetrisGame::_generateFigureSet()
{
    Figure F;
    F << 0, 0, 0, 0, 0,
         0, 1, 1, 0, 0,
         0, 1, 1, 0, 0,
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

    size_t k = 0;
    for (int i = 0; i < Figure::RowsAtCompileTime; ++i)
    {
        int y = i - figureAnchor.y();
        for (int j = 0; j < Figure::ColsAtCompileTime; ++j, ++k)
        {
            int x = j - figureAnchor.x();
            m_rotateFigureInd[k] = Vector2i(figureAnchor.x() - y, figureAnchor.y() + x);
        }
    }
}

bool TetrisGame::_beginAction(ActionType actionType)
{
    switch (actionType)
    {
    case ActionType::NoAction:
        return true;
    case ActionType::FigureEntry:
    {
        m_currentFigure = m_nextFigure;
        m_nextFigure = _createRandomFigure();
        pair<Vector2i, Vector2i> bb = _getFigureBoundedBox(m_nextFigure);
        int begin_x = bb.first.x();
        int end_x = m_fieldSize.x() - (bb.second.x());
        m_figurePos_current.x() = begin_x + static_cast<int>(m_rnd(m_rnd_gen)) % (end_x - begin_x);
        m_figurePos_current.y() = bb.first.y() + m_fieldSize.y();
        m_figurePos_next = m_figurePos_current;
    } return true;
    case ActionType::LinesRemoval:
    {

    } return true;
    case ActionType::FigureFall:
    {
        Vector2i nextPos = m_figurePos_current + Vector2i(0, 1);
        if (!_checkIntersect(m_currentFigure, nextPos))
            return false;
        do {
            m_figurePos_next = nextPos;
            nextPos = nextPos + Vector2i(0, 1);
        } while (!_checkIntersect(m_currentFigure, nextPos));
    } return true;
    case ActionType::MoveFigureLeft:
    {
        Vector2i nextPos = m_figurePos_current + Vector2i(-1, 0);
        if (!_checkIntersect(m_currentFigure, nextPos))
            return false;
        m_figurePos_next = nextPos;
    } return true;
    case ActionType::MoveFigureRight:
    {
        Vector2i nextPos = m_figurePos_current + Vector2i(1, 0);
        if (!_checkIntersect(m_currentFigure, nextPos))
            return false;
        m_figurePos_next = nextPos;
    } return true;
    case ActionType::MoveFigureDown:
    {
        Vector2i nextPos = m_figurePos_current + Vector2i(0, 1);
        if (!_checkIntersect(m_currentFigure, nextPos))
            return false;
        m_figurePos_next = nextPos;
    } return true;
    case ActionType::RotateFigure:
        return !_checkIntersect(_rotated(m_currentFigure), m_figurePos_current);
    default:
        break;
    }
    return false;
}

TetrisGame::Figure TetrisGame::_rotated(const TetrisGame::Figure & figure) const
{
    std::size_t k = 0;
    Figure rFigure;
    for (int i = 0; i < Figure::RowsAtCompileTime; ++i)
    {
        for (int j = 0; j < figure.cols(); ++j, ++k)
        {
            const Vector2i & p = m_rotateFigureInd[k];
            rFigure(i, j) = figure(p.y(), p.x());
        }
    }
    return rFigure;
}

std::pair<Vector2i, Vector2i> TetrisGame::_getFigureBoundedBox(const TetrisGame::Figure & figure) const
{
    std::pair<Vector2i, Vector2i> bb(Vector2i(m_fieldSize),
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
                if ((y < 0) || (y > m_fieldSize.y()))
                    return true;
                int x = begin.x() + j;
                if ((x < 0) || (x > m_fieldSize.x()))
                    return true;
                if (m_field(y, x) > 0)
                    return true;
            }
        }
    }
    return false;
}

vector<int> TetrisGame::_findFullLines() const
{
    std::vector<int> lines;
    lines.reserve(Figure::RowsAtCompileTime);
    int begin_y = m_figurePos_current.y() - figureAnchor.y();
    if (begin_y < 0)
        begin_y = 0;
    int end_y = m_figurePos_current.y() + Figure::RowsAtCompileTime - figureAnchor.y();
    if (end_y > m_fieldSize.y())
        end_y = m_fieldSize.y();
    for (int y = end_y - 1; y >= begin_y; --y)
    {
        bool fullLine = true;
        for (int x = 0; x < Figure::ColsAtCompileTime; ++x)
        {
            if (m_field(y, x) <= 0)
            {
                fullLine = false;
                break;
            }
        }
        if (fullLine)
            lines.push_back(y);
    }
    return lines;
}

void TetrisGame::_fillMoveFieldForLinesRemoval(const vector<int> & lines) const
{

}
