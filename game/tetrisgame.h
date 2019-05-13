#ifndef TETRISGAME_H
#define TETRISGAME_H

#include <vector>
#include <array>
#include <set>
#include <random>

#include <Eigen/Eigen>

class TetrisGame
{
public:
    using Figure = Eigen::Matrix<int, 5, 5>;
    static const Eigen::Vector2i figureAnchor;

    TetrisGame(const Eigen::Vector2i & fieldSize);

    Eigen::Vector2i fieldSize() const;

    void reset();

    enum class ActionType
    {
        NoAction,
        FigureEntry,
        LinesRemoval,
        FigureFall,
        MoveFigureLeft,
        MoveFigureRight,
        MoveFigureDown,
        RotateFigure
    };

    struct Action
    {
        ActionType type;
        int currentTime;
        int timer;
    };

private:
    std::vector<Figure, Eigen::aligned_allocator<Figure>> m_figureSet;
    std::array<Eigen::Vector2i, Figure::ColsAtCompileTime * Figure::RowsAtCompileTime> m_rotateFigureInd;

    Eigen::Vector2i m_fieldSize;
    Eigen::MatrixXi m_field;
    Eigen::MatrixXi m_moveField;

    Action m_userAction;
    Action m_currentAction;
    Figure m_currentFigure;
    Figure m_nextFigure;
    Eigen::Vector2i m_figurePos_current;
    Eigen::Vector2i m_figurePos_next;

    std::mt19937 m_rnd_gen;
    std::uniform_int_distribution<std::size_t> m_rnd;

    void _updateRandom();
    Figure _createRandomFigure() const;

    void _generateFigureSet();
    bool _beginAction(ActionType actionType);
    bool _endAction(const Action & action);

    Figure _rotated(const Figure & figure) const;
    std::pair<Eigen::Vector2i, Eigen::Vector2i> _getFigureBoundedBox(const Figure & figure) const;
    bool _checkIntersect(const Figure & figure, const Eigen::Vector2i & pos) const;

    std::vector<int> _findFullLines() const;
    void _fillMoveFieldForLinesRemoval(const std::vector<int> & lines) const;
};

#endif // TETRISGAME_H
