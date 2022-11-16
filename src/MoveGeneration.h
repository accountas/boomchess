//
// Created by marty on 2022-11-09.
//

#ifndef BOOMCHESS_SRC_MOVEGENERATION_H_
#define BOOMCHESS_SRC_MOVEGENERATION_H_

#include <array>
#include <list>
#include "Move.h"
#include "Board.h"

class MoveGeneration {
 public:
    std::array<std::array<Move, 300>, MAX_DEPTH> moves {};
    std::array<int, MAX_DEPTH> n {};

    void generateMoves(const Board &board);

    int size(){
        return n[curDepth];
    }
    void increaseDepth(){
        curDepth++;
    }
    void decreaseDepth(){
        curDepth--;
    }
    void setDepth(int depth){
        curDepth = depth;
    }
    Move &operator[](int idx){
        return moves[curDepth][idx];
    }

 private:
    int curDepth = 0;
    void generatePawnMoves(const Board &board);
    void generateBishopMoves(const Board &board);
    void generateKnightMoves(const Board &board);
    void generateRookMoves(const Board &board);
    void generateQueenMoves(const Board &board);
    void generateKingMoves(const Board &board);
    void generateSlidingMoves(const Board &board, int startingSquare, int direction);
    void generateCastle(const Board &board, int kingSquare, int castleDirection);
    void addMove(int from, int to, int flags = 0) {
        moves[curDepth][n[curDepth]++] = Move(from, to, flags);
    }
};

#endif //BOOMCHESS_SRC_MOVEGENERATION_H_
