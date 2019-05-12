#ifndef TETRISGAME_H
#define TETRISGAME_H

#include <vector>
#include <set>
#include <random>

#include <Eigen/Eigen>

class TetrisGame
{
public:
    using Figure = Eigen::Matrix<int, 5, 5>;
    static const Eigen::Vector2i figureAnchor;

    TetrisGame(const Eigen::Vector2i & size);

    Eigen::Vector2i size() const;

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

    Eigen::MatrixXi m_field;
    std::set<int> m_fullLines;

    Action m_userAction;
    Action m_currentAction;
    Figure m_currentFigure;
    Figure m_nextFigure;
    Eigen::Vector2i m_currentFigurePos;

    std::mt19937 m_rnd_gen;
    std::uniform_int_distribution<std::size_t> m_rnd;

    void _updateRandom();
    Figure _createRandomFigure() const;

    bool _checkAction(ActionType type);

    void _generateFigureSet();
    bool _doAction(const Action & action);

    Figure _rotated(const Figure & figure) const;
    std::pair<Eigen::Vector2i, Eigen::Vector2i> _getFigureBoundedBox(const Figure & figure) const;
    bool _checkIntersect(const Figure & figure, const Eigen::Vector2i & pos) const;
};

#endif // TETRISGAME_H
