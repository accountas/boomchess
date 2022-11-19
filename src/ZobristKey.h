//
// Created by marty on 2022-11-19.
//

#ifndef BOOMCHESS_SRC_ZOBRISTKEY_H_
#define BOOMCHESS_SRC_ZOBRISTKEY_H_

#include <array>
#include <cstdint>
#include "Common.h"
#include "Piece.h"

class ZobristKey {

 public:
    ZobristKey() {
        generateNumbers();
    }

    uint64_t value = 0;

    void flipPiece(int idx, const Piece &piece) {
        value ^= pieceNumbers[piece.color()][piece.type()][idx];
    }
    void setMoveColor(int color) {
        value ^= moveColorNumbers[color];
    }
    void flipMoveColor() {
        value ^= moveColorNumbers[0];
        value ^= moveColorNumbers[1];
    }
    void flipEnPassantFile(int fileFrom) {
        value ^= enPassantFileNumbers[fileFrom];
    }
    void flipCastlingRights(int color, int rights) {
        value ^= castlingRightNumbers[color][rights];
    }

 private:
    std::array<std::array<std::array<uint64_t, 128>, 10>, 2> pieceNumbers{}; //[color][type][index]
    std::array<uint64_t, 2> moveColorNumbers{};
    std::array<uint64_t, 8> enPassantFileNumbers{};
    std::array<std::array<uint64_t, 4>, 2> castlingRightNumbers{}; //[color][right]
    void generateNumbers();
};

#endif //BOOMCHESS_SRC_ZOBRISTKEY_H_
