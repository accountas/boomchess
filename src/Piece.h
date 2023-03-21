#pragma once

#include <cstdint>
#include "Common.h"

class Piece {
 public:
    int piece;
    int pieceListLocation;
    int value;

    Piece(int piece, int pieceListLocation);
    Piece() : piece(PieceType::EMPTY), pieceListLocation(127), value(0) {};

    int type() const {
        return piece & (~BLACK_FLAG);
    }
    int color() const {
        return bool(piece & BLACK_FLAG);
    };
    char toChar() const;
};
