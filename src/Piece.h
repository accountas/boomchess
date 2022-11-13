//
// Created by marty on 2022-11-05.
//

#ifndef BOOMCHESS_PIECE_H
#define BOOMCHESS_PIECE_H

#include <cstdint>
#include "Common.h"

class Piece {
 public:
    int piece;
    int pieceListLocation;

    Piece(uint8_t piece, uint8_t pieceListLocation);
    Piece() : piece(PieceType::EMPTY), pieceListLocation(127) {};

    int type() const {
        return piece & ~BLACK_FLAG;
    }
    char toChar() const;
};

#endif //BOOMCHESS_PIECE_H
