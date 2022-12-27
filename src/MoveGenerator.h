//
// Created by marty on 2022-11-09.
//

#ifndef BOOMCHESS_SRC_MOVEGENERATOR_H_
#define BOOMCHESS_SRC_MOVEGENERATOR_H_

#include <array>
#include <list>
#include "Move.h"
#include "Board.h"

class MoveGenerator {

 public:
    std::array<std::array<Move, 300>, MAX_DEPTH> moves{};
    std::array<std::array<int, 300>, MAX_DEPTH> captureScore{}; //mvvlva
    std::array<int, MAX_DEPTH> n{};
    std::array<int, MAX_DEPTH> nSorted{};

    void generateMoves(const Board &board, int piece = -1);

    int size() {
        return n[curDepth];
    }
    void sortTT(const Move &move);

    void increaseDepth() {
        curDepth++;
    }
    void decreaseDepth() {
        curDepth--;
    }
    void setDepth(int depth) {
        curDepth = depth;
    }
    Move &getSorted(int idx, const Board &board) {
        sortTill(idx, board);
        return (*this)[idx];
    }
    Move &operator[](int idx) {
        return moves[curDepth][idx];
    }
    void markKiller(int idx);
    void updateHistory(const Move &move, int depth);
    void ageHistory();
    bool isGoodCapture(int idx);

 private:
    int curDepth = 0;
    void sortTill(int idx, const Board &board);
    void generatePawnMoves(const Board &board);
    void generateBishopMoves(const Board &board);
    void generateKnightMoves(const Board &board);
    void generateRookMoves(const Board &board);
    void generateQueenMoves(const Board &board);
    void generateKingMoves(const Board &board);
    void generateSlidingMoves(const Board &board, int startingSquare, int direction);
    void generateCastle(const Board &board, int kingSquare, int castleDirection);
    void calculateLatestCaptureScore(const Board &board);
    void addMove(int from, int to, int flags = 0) {
        moves[curDepth][n[curDepth]++] = Move(from, to, flags);
    }

    std::array<std::array<Move, 2>, MAX_DEPTH> killers{};
    std::array<std::array<int, 128>, 128> historyTable{};
};

#endif //BOOMCHESS_SRC_MOVEGENERATOR_H_
