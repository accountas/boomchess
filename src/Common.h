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

/**
 * Board Evaluation constants
 */
const std::array<int, 7> PieceWeights = {
    0, 100, 300, 300, 500, 900, EVAL_MAX
};

const std::array<std::array<int, 64>, 7> PieceSquareTables = {{
                                                                  {},
                                                                  //pawn
                                                                  {
                                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                                      78, 83, 86, 73, 102, 82, 85, 90,
                                                                      7, 29, 21, 44, 40, 31, 44, 7,
                                                                      -17, 16, -2, 15, 14, 0, 15, -13,
                                                                      -26, 3, 10, 9, 6, 1, 0, -23,
                                                                      -22, 9, 5, -11, -10, -2, 3, -19,
                                                                      -31, 8, -7, -37, -36, -14, 3, -31,
                                                                      0, 0, 0, 0, 0, 0, 0, 0
                                                                  },

                                                                  //knight
                                                                  {
                                                                      -66, -53, -75, -75, -10, -55, -58, -70,
                                                                      -3, -6, 100, -36, 4, 62, -4, -14,
                                                                      10, 67, 1, 74, 73, 27, 62, -2,
                                                                      24, 24, 45, 37, 33, 41, 25, 17,
                                                                      -1, 5, 31, 21, 22, 35, 2, 0,
                                                                      -18, 10, 13, 22, 18, 15, 11, -14,
                                                                      -23, -15, 2, 0, 2, 0, -23, -20,
                                                                      -74, -23, -26, -24, -19, -35, -22, -69
                                                                  },
                                                                  //bishop
                                                                  {
                                                                      -59, -78, -82, -76, -23, -107, -37, -50,
                                                                      -11, 20, 35, -42, -39, 31, 2, -22,
                                                                      -9, 39, -32, 41, 52, -10, 28, -14,
                                                                      25, 17, 20, 34, 26, 25, 15, 10,
                                                                      13, 10, 17, 23, 17, 16, 0, 7,
                                                                      14, 25, 24, 15, 8, 25, 20, 15,
                                                                      19, 20, 11, 6, 7, 6, 20, 16,
                                                                      -7, 2, -15, -12, -14, -15, -10, -10
                                                                  },
                                                                  //rook
                                                                  {
                                                                      35, 29, 33, 4, 37, 33, 56, 50,
                                                                      55, 29, 56, 67, 55, 62, 34, 60,
                                                                      19, 35, 28, 33, 45, 27, 25, 15,
                                                                      0, 5, 16, 13, 18, -4, -9, -6,
                                                                      -28, -35, -16, -21, -13, -29, -46, -30,
                                                                      -42, -28, -42, -25, -25, -35, -26, -46,
                                                                      -53, -38, -31, -26, -29, -43, -44, -53,
                                                                      -30, -24, -18, 5, -2, -18, -31, -32
                                                                  },
                                                                  //queen
                                                                  {
                                                                      6, 1, -8, -104, 69, 24, 88, 26,
                                                                      14, 32, 60, -10, 20, 76, 57, 24,
                                                                      -2, 43, 32, 60, 72, 63, 43, 2,
                                                                      1, -16, 22, 17, 25, 20, -13, -6,
                                                                      -14, -15, -2, -5, -1, -10, -20, -22,
                                                                      -30, -6, -13, -11, -16, -11, -16, -27,
                                                                      -36, -18, 0, -19, -15, -15, -21, -38,
                                                                      -39, -30, -31, -13, -31, -36, -34, -42
                                                                  },
                                                                  //king
                                                                  {
                                                                      4, 54, 47, -99, -99, 60, 83, -62,
                                                                      -32, 10, 55, 56, 56, 55, 10, 3,
                                                                      -62, 12, -57, 44, -67, 28, 37, -31,
                                                                      -55, 50, 11, -4, -19, 13, 0, -49,
                                                                      -55, -43, -52, -28, -51, -47, -8, -50,
                                                                      -47, -42, -43, -79, -64, -32, -29, -32,
                                                                      -4, 3, -14, -50, -57, -18, 13, 4,
                                                                      17, 30, -3, -14, 6, -1, 40, 18
                                                                  },
                                                              }};

#endif //BOOMCHESS_ENUMS_H
