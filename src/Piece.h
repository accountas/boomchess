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

    Piece(int piece, int pieceListLocation);
    Piece() : piece(PieceType::EMPTY), pieceListLocation(127) {};

    int type() const {
        return piece & (~BLACK_FLAG);
    }
    int color() const {
        return bool(piece & BLACK_FLAG);
    };
    char toChar() const;
};

#endif //BOOMCHESS_PIECE_H
