#include "tetrisgame.h"

#include <chrono>
#include <algorithm>

using namespace std;
using namespace std::chrono;
using namespace Eigen;

const Vector2i TetrisGame::figureAnchor = Vector2i(2, 2);

TetrisGame::TetrisGame(const Vector2i & fieldSize):
    m_fieldSize(fieldSize),
    m_field(fieldSize.y() + Figure::RowsAtCompileTime, fieldSize.x()),
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
    _updateRandom();
    m_currentFigure = _createRandomFigure();
    m_nextFigure = _createRandomFigure();
    m_figurePos = _createRandomStartPos(m_currentFigure);
    m_newFigureTimer = newFigureTime;
    m_userActionTimer = 0;
    m_stepTimer = 0;
    m_removalLinesTimer = 0;
    m_linesForRemoval.clear();
    m_numberRemovedLines = 0;
}

TetrisGame::Figure TetrisGame::currentFigure() const
{
    return m_currentFigure;
}

TetrisGame::Figure TetrisGame::nextFigure() const
{
    return m_nextFigure;
}

Vector2i TetrisGame::figurePos() const
{
    return m_figurePos;
}

float TetrisGame::currentFigureState() const
{
    if (m_removalLinesTimer > 0)
        return 0.0f;
    if (m_newFigureTimer > 0)
        return 1.0f - m_newFigureTimer / static_cast<float>(newFigureTime);
    return 1.0f;
}

void TetrisGame::for_each_blocks(const std::function<void (const Vector2i &)> & lambda) const
{
    for (int i = 0; i < m_fieldSize.y(); ++i)
    {
        for (int j = 0; j < m_fieldSize.x(); ++j)
        {
            if (m_field(i, j) > 0)
            {
                lambda(Vector2i(j, i));
            }
        }
    }
}

TetrisGame::EventType TetrisGame::step()
{
    if (!m_linesForRemoval.empty())
    {
        if (m_removalLinesTimer > 0)
        {
            int end_x = static_cast<int>(ceil((1.0f - (m_removalLinesTimer /
                                               static_cast<float>(removalLinesTime))) *
                                              m_fieldSize.x()));
            for (int y : m_linesForRemoval)
            {
                for (int x = 0; x < end_x; ++x)
                    m_field(y, x) = 0;
            }
            --m_removalLinesTimer;
        }
        else
        {
            sort(m_linesForRemoval.begin(), m_linesForRemoval.end(), greater<int>());
            for (int y: m_linesForRemoval)
            {
                _removeLine(y);
                ++m_numberRemovedLines;
            }
            m_linesForRemoval.clear();
            m_stepTimer = stepTime;
        }
        return EventType::RemovingLines;
    }
    if (m_userActionTimer > 0)
        --m_userActionTimer;
    if (m_newFigureTimer > 0)
    {
        --m_newFigureTimer;
        if (m_newFigureTimer <= 0)
            m_stepTimer = stepTime;
        return EventType::NewFigure;
    }
    if (m_stepTimer > 0)
    {
        --m_stepTimer;
        if (m_stepTimer > 0)
            return EventType::WaitStep;
    }
    if (_checkIntersect(m_currentFigure, Vector2i(m_figurePos.x(), m_figurePos.y() - 1)))
    {
        if (!_insertCurrentFigure())
        {
            return EventType::Lose;
        }
        m_currentFigure = m_nextFigure;
        m_figurePos = _createRandomStartPos(m_currentFigure);
        m_nextFigure = _createRandomFigure();
        m_newFigureTimer = newFigureTime;
        return EventType::FigureInserting;
    }
    --m_figurePos.y();
    m_stepTimer = stepTime;
    return EventType::FigureFalling;
}

bool TetrisGame::moveFigureLeft()
{
    if (m_userActionTimer > 0)
        return false;
    if (m_stepTimer <= 0)
        return false;
    Vector2i pos(m_figurePos.x() - 1, m_figurePos.y());
    if (_checkIntersect(m_currentFigure, pos))
        return false;
    m_figurePos = pos;
    m_userActionTimer = userActionTime;
    return true;
}

bool TetrisGame::moveFigureRight()
{
    if (m_userActionTimer > 0)
        return false;
    if (m_stepTimer <= 0)
        return false;
    Vector2i pos(m_figurePos.x() + 1, m_figurePos.y());
    if (_checkIntersect(m_currentFigure, pos))
        return false;
    m_figurePos = pos;
    m_userActionTimer = userActionTime;
    return true;
}

bool TetrisGame::moveFigureDown()
{
    if (m_userActionTimer > 0)
        return false;
    if (m_stepTimer <= 0)
        return false;
    Vector2i pos(m_figurePos.x(), m_figurePos.y() - 1);
    if (_checkIntersect(m_currentFigure, pos))
        return false;
    do {
        m_figurePos = pos;
        pos = Vector2i(m_figurePos.x(), m_figurePos.y() - 1);
    } while (!_checkIntersect(m_currentFigure, pos));
    m_userActionTimer = userActionTime;
    return true;
}

bool TetrisGame::rotateFigure()
{
    if (m_userActionTimer > 0)
        return false;
    if (m_stepTimer <= 0)
        return false;
    Figure f = _rotated(m_currentFigure);
    if (_checkIntersect(f, m_figurePos))
        return false;
    m_currentFigure = f;
    m_userActionTimer = userActionTime;
    return true;
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

Vector2i TetrisGame::_createRandomStartPos(const TetrisGame::Figure & figure) const
{
    pair<Vector2i, Vector2i> bb = _getFigureBoundedBox(figure);
    int begin_x = - bb.first.x(), end_x = m_fieldSize.x() - 1 - bb.second.x();
    Vector2i pos(begin_x + m_rnd(m_rnd_gen) % (end_x - begin_x),
                 m_fieldSize.y() - bb.first.y());
    return pos;
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
    std::pair<Vector2i, Vector2i> bb(Vector2i(figure.cols(), figure.rows()),
                                     Vector2i(0, 0));
    for (int i = 0; i < figure.rows(); ++i)
    {
        for (int j = 0; j < figure.cols(); ++j)
        {
            if (figure(i, j) > 0)
            {
                if (j < bb.first.x())
                    bb.first.x() = j;
                if (i < bb.first.y())
                    bb.first.y() = i;
                if (j > bb.second.x())
                    bb.second.x() = j;
                if (i > bb.second.y())
                    bb.second.y() = i;
            }
        }
    }
    bb.first -= figureAnchor;
    bb.second -= figureAnchor;
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
                if ((y < 0))
                    return true;
                int x = begin.x() + j;
                if ((x < 0) || (x >= m_fieldSize.x()))
                    return true;
                if (m_field(y, x) > 0)
                    return true;
            }
        }
    }
    return false;
}

bool TetrisGame::_insertCurrentFigure()
{
    pair<Vector2i, Vector2i> bb = _getFigureBoundedBox(m_currentFigure);
    bb.first += m_figurePos;
    bb.second += m_figurePos;
    if (bb.second.y() >= m_fieldSize.y())
        return false;
    for (int i = 0; i < Figure::RowsAtCompileTime; ++i)
    {
        int y = i - figureAnchor.y() + m_figurePos.y();
        for (int j = 0; j < Figure::ColsAtCompileTime; ++j)
        {
            if (m_currentFigure(i, j) > 0)
            {
                int x = j - figureAnchor.x() + m_figurePos.x();
                m_field(y, x) = 1;
            }
        }
    }
    m_linesForRemoval.clear();
    for (int y = bb.first.y(); y <= bb.second.y(); ++y)
    {
        bool fullLineFlag = true;
        for (int x = 0; x < m_fieldSize.x(); ++x)
        {
            if (m_field(y, x) <= 0)
            {
                fullLineFlag = false;
                break;
            }
        }
        if (fullLineFlag)
            m_linesForRemoval.push_back(y);
    }
    if (!m_linesForRemoval.empty())
        m_removalLinesTimer = removalLinesTime;
    return true;
}

void TetrisGame::_removeLine(int y)
{
    for (int i = y; i < m_fieldSize.y(); ++i)
        m_field.row(i) = m_field.row(i + 1);
}
