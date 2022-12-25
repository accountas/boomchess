#ifndef BOOMCHESS_ENUMS_H
#define BOOMCHESS_ENUMS_H

#include <array>

//engine params
const bool USE_METRICS = true;
const int MAX_DEPTH = 128;
const int EVAL_MAX = 1e5;
const int EVAL_MIN = -EVAL_MAX;
const int TT_SIZE = 1 << 25;
const int KILLER_MOVES_N = 2;
const int NULL_MOVE_R = 2;
const std::string DEFAULT_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

//disable by setting value to negative number
enum MetricTypes : int {
    NODES_SEARCHED,
    LEAF_NODES_SEARCHED,
    Q_NODES_SEARCHED,
    CACHE_HITS,
    PV_HITS,
    PV_MISSES,
    TT_ENTRIES
};

struct SearchParams {
    int depthLimit;
    int timeLimit;
};

/**
 * Common enums
 */
enum Color : int {
    WHITE = 0,
    BLACK = 1
};

enum CastlingRight : int {
    NO_CASTLE = 0,
    ALL_SIDES = 3,
    KING_SIDE = 1,
    QUEEN_SIDE = 2
};

enum PieceType : int {
    EMPTY = 0,
    PAWN = 1,
    KNIGHT = 2,
    BISHOP = 3,
    ROOK = 4,
    QUEEN = 5,
    KING = 6,

    //flags
    BLACK_FLAG = 1 << 6
};

const std::array<int, 6> PieceTypes = {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
};

enum SearchBound : int {
    UPPER_BOUND = 0,
    LOWER_BOUND = 1,
    EXACT = 2
};

namespace WinState {
    const int LOST = 0;
    const int TIE = 1;
    const int NORMAL = 2;
}


/**
 * Move Generation
 * */
namespace MoveFlags {
    //has capture been made
    const int CAPTURE = 1;

    //has pawn made a double moveColor (for en passant)
    const int DOUBLE_PAWN = 1 << 1;

    //was pawn moved, for three-fold speedup
    const int PAWN_MOVE = 1 << 2;

    //if captured en passant, also need to add CAPTURE flag
    const int EN_PASSANT_CAPTURE = 1 << 3;

    //promotion types
    const int KNIGHT_PROMOTION = 1 << 4;
    const int BISHOP_PROMOTION = 1 << 5;
    const int ROOK_PROMOTION = 1 << 6;
    const int QUEEN_PROMOTION = 1 << 7;
    const int PROMOTION_SUBMASK = KNIGHT_PROMOTION | BISHOP_PROMOTION | ROOK_PROMOTION | QUEEN_PROMOTION;

    //castling
    const int CASTLE_RIGHT = 1 << 8;
    const int CASTLE_LEFT = 1 << 9;
    const int CASTLE_SUBMASK = CASTLE_RIGHT | CASTLE_LEFT;

    //null move heuristic
    const int NULL_MOVE = 1 << 10;

    //moves that cant be repeated
    const int NON_REPEATABLE_MASK = CASTLE_SUBMASK | PAWN_MOVE | CAPTURE | PROMOTION_SUBMASK;
}

namespace Direction {
    const int UP = 16;
    const int DOWN = -16;
    const int RIGHT = 1;
    const int LEFT = -1;
}

const std::array<int, 8> knightDirections = {
    Direction::UP * 2 + Direction::RIGHT,
    Direction::UP * 2 + Direction::LEFT,
    Direction::DOWN * 2 + Direction::RIGHT,
    Direction::DOWN * 2 + Direction::LEFT,
    Direction::RIGHT * 2 + Direction::UP,
    Direction::RIGHT * 2 + Direction::DOWN,
    Direction::LEFT * 2 + Direction::UP,
    Direction::LEFT * 2 + Direction::DOWN
};

const std::array<int, 4> straightDirections = {
    Direction::UP,
    Direction::DOWN,
    Direction::RIGHT,
    Direction::LEFT,
};

const std::array<int, 4> diagonalDirections = {
    Direction::UP + Direction::RIGHT,
    Direction::UP + Direction::LEFT,
    Direction::DOWN + Direction::RIGHT,
    Direction::DOWN + Direction::LEFT,
};

const std::array<int, 8> allDirections = {
    Direction::UP,
    Direction::DOWN,
    Direction::RIGHT,
    Direction::LEFT,
    Direction::UP + Direction::RIGHT,
    Direction::UP + Direction::LEFT,
    Direction::DOWN + Direction::RIGHT,
    Direction::DOWN + Direction::LEFT,
};

//might want to play around with it
const std::array<int, 8> explosionDirections = allDirections;

#endif //BOOMCHESS_ENUMS_H
