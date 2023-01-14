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

    void rootSearch();

    void killSearch();

    void resetCache() {
        Metric<TT_ENTRIES>::set(0);
        tTable.clear();
        generator.clearHistory();
        generator.clearKillers();
    }

    bool canSearch() {
        if (!searchActive) {
            return false;
        }
        if (searchParams.nodeLimit < Metric<NODES_SEARCHED>::get() - Metric<LEAF_NODES_SEARCHED>::get() + Metric<Q_NODES_SEARCHED>::get()) {
            return false;
        }
        return true;
    }
    bool searchActive = false;

 private:
    Board board;
    MoveGenerator generator;
    Evaluator evaluator;
    SearchParams searchParams;

    TranspositionTable<SearchEntry, TT_SIZE> tTable{};
    int quiescence(int alpha, int beta);
    int alphaBeta(int depthLeft, int alpha, int beta);
};

#endif //BOOMCHESS_SRC_SEARCH_H_
