//
// Created by marty on 2022-11-17.
//

#ifndef BOOMCHESS_SRC_SEARCH_H_
#define BOOMCHESS_SRC_SEARCH_H_

#include "Board.h"
#include "MoveGenerator.h"
#include "Evaluator.h"
#include "TranspositionTable.h"
#include "Metrics.h"

class Search {
 public:
    Search() : board(Board::fromFen(DEFAULT_FEN)) {}

    void setBoard(const Board &b) {
        board = b;
    }
    void startSearch(const SearchParams &params);

    void killSearch();

    void resetCache() {
        Metric<TT_ENTRIES>::set(0);
        tTable.clear();
        generator.clearHistory();
    }

 private:
    Board board;
    MoveGenerator generator;
    Evaluator evaluator;

    TranspositionTable<SearchEntry, TT_SIZE> tTable{};

    bool canSearch = false;

    void rootSearch(const SearchParams &params);

    int quiescence(int alpha, int beta);
    int alphaBeta(int depthLeft, int alpha, int beta);
};

#endif //BOOMCHESS_SRC_SEARCH_H_
