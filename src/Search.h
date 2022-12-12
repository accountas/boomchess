//
// Created by marty on 2022-11-17.
//

#ifndef BOOMCHESS_SRC_SEARCH_H_
#define BOOMCHESS_SRC_SEARCH_H_

#include "Board.h"
#include "MoveGenerator.h"
#include "Evaluator.h"
#include "TranspositionTable.h"

struct SearchResult;

class Search {
 public:
    Search() : board(Board::fromFen(DEFAULT_FEN)) {}

    int numChecked = 0;
    int cacheHits = 0;

    void search(int depth);
    void setBoard(const Board &b) {
        board = b;
    }

 private:
    Board board;
    MoveGenerator generator;
    Evaluator evaluator;

    TranspositionTable<SearchEntry, TT_SIZE> tTable {};

    int alphaBeta(int depthLeft, int alpha, int beta, bool isPV);
};

struct SearchResult {
    Move bestMove;
    int evaluation;
    SearchResult(const Move &best_move, int evaluation) : bestMove(best_move), evaluation(evaluation) {}
};

#endif //BOOMCHESS_SRC_SEARCH_H_
