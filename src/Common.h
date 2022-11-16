#ifndef BOOMCHESS_ENUMS_H
#define BOOMCHESS_ENUMS_H

#include <array>

const int MAX_DEPTH = 128;

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
    BLACK_FLAG = 1 << 6,
};

namespace MoveFlags {
    //has capture been made
    const int CAPTURE = 1;

    //has pawn made a double moveColor (for en passant)
    const int DOUBLE_PAWN = 1 << 1;

    //if captured en passant, also need to add CAPTURE flag
    const int EN_PASSANT_CAPTURE = 1 << 2;

    //promotion types
    const int KNIGHT_PROMOTION = 1 << 3;
    const int BISHOP_PROMOTION = 1 << 4;
    const int ROOK_PROMOTION = 1 << 5;
    const int QUEEN_PROMOTION = 1 << 6;
    const int PROMOTION_SUBMASK = KNIGHT_PROMOTION | BISHOP_PROMOTION | ROOK_PROMOTION | QUEEN_PROMOTION;

    //castling
    const int CASTLE_RIGHT = 1 << 7;
    const int CASTLE_LEFT = 1 << 8;
    const int CASTLE_SUBMASK = CASTLE_RIGHT | CASTLE_LEFT;
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
