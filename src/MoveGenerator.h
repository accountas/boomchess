#pragma once

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

    explicit MoveGenerator(bool fast) : fast(fast) {}
    MoveGenerator() : fast(false) {}

    void generateMoves(const Board &board);

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
    void setCountOnly(bool flag){
        countOnly = flag;
    }
    void markKiller(int idx);
    void updateHistory(int sideToMove, const Move &move, int depth);
    void clearHistory();
    void ageHistory(int side);
    bool isGoodCapture(int idx);
    void clearKillers();
 private:
    bool fast = false;
    int curDepth = 0;
    int countOnly = false;
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
    void addMove(int from, int to, int flags = 0);

    std::array<std::array<Move, 2>, MAX_DEPTH> killers{};
    std::array<std::array<std::array<int, 128>, 128>, 2> historyTable{};
};