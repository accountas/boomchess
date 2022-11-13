//
// Created by marty on 2022-11-09.
//

#ifndef BOOMCHESS_SRC_MOVEGENERATION_H_
#define BOOMCHESS_SRC_MOVEGENERATION_H_

#include <array>
#include "Move.h"
#include "Board.h"


class MoveGeneration {
 public:
    std::array<Move, 300> moves;
    int n;

    MoveGeneration() : n(0) {};
    void generateMoves(const Board &board);

 private:
    void generatePawnMoves(const Board &board);
    void generateBishopMoves(const Board &board);
    void generateKnightMoves(const Board &board);
    void generateRookMoves(const Board &board);
    void generateQueenMoves(const Board &board);
    void generateKingMoves(const Board &board);
    void generateSlidingMoves(const Board &board, int startingSquare, int direction);
    void generateCastle(const Board &board, int kingSquare, int castleDirection);

    void addMove(int from, int to, int flags = 0) {
        moves[n++] = Move(from, to, flags);
    }
};

#endif //BOOMCHESS_SRC_MOVEGENERATION_H_
