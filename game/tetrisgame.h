#ifndef TETRISGAME_H
#define TETRISGAME_H

#include <vector>
#include <array>
#include <set>
#include <random>
#include <functional>

#include <Eigen/Eigen>

class TetrisGame
{
public:
    static const int removalLinesTime = 5;
    static const int userActionTime = 2;
    static const int stepTime = 3;
    static const int newFigureTime = 10;

    using Figure = Eigen::Matrix<int, 5, 5>;
    static const Eigen::Vector2i figureAnchor;

    enum class EventType
    {
        RemovingLines,
        NewFigure,
        WaitStep,
        Lose,
        FigureInserting,
        FigureFalling
    };

    TetrisGame(const Eigen::Vector2i & fieldSize);

    Eigen::Vector2i fieldSize() const;

    Figure currentFigure() const;
    Figure nextFigure() const;
    Eigen::Vector2i figurePos() const;

    float currentFigureState() const;

    void for_each_blocks(const std::function<void(const Eigen::Vector2i &)> & lambda) const;

    void reset();

    EventType step();

    bool moveFigureLeft();
    bool moveFigureRight();
    bool moveFigureDown();
    bool rotateFigure();

private:
    std::vector<Figure, Eigen::aligned_allocator<Figure>> m_figureSet;
    std::array<Eigen::Vector2i, Figure::ColsAtCompileTime * Figure::RowsAtCompileTime> m_rotateFigureInd;

    Eigen::Vector2i m_fieldSize;
    Eigen::MatrixXi m_field;

    Figure m_currentFigure;
    Figure m_nextFigure;
    Eigen::Vector2i m_figurePos;

    mutable std::mt19937 m_rnd_gen;
    std::uniform_int_distribution<std::size_t> m_rnd;

    std::vector<int> m_linesForRemoval;
    int m_numberRemovedLines;

    int m_removalLinesTimer;
    int m_newFigureTimer;
    int m_userActionTimer;
    int m_stepTimer;

    void _updateRandom();
    Figure _createRandomFigure() const;
    Eigen::Vector2i _createRandomStartPos(const Figure & figure) const;

    void _generateFigureSet();

    Figure _rotated(const Figure & figure) const;
    std::pair<Eigen::Vector2i, Eigen::Vector2i> _getFigureBoundedBox(const Figure & figure) const;
    bool _checkIntersect(const Figure & figure, const Eigen::Vector2i & pos) const;
    bool _insertCurrentFigure();
    void _removeLine(int y);
};

#endif // TETRISGAME_H
