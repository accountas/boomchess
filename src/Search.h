#pragma once

#include "Board.h"
#include "MoveGenerator.h"
#include "Evaluator.h"
#include "TranspositionTable.h"
#include "Metrics.h"

class Search {
 public:
    Search() : board(Board::fromFen(DEFAULT_FEN)) {}

    void setBoard(const Board &b) {board = b;}
    void startSearch(const SearchParams &params);
    void resetCache();
    void killSearch();

    std::array<std::pair<int, Move>, 32> evals;
    int depthReached = 0;

    TranspositionTable<SearchEntry> tTable{};

 private:
    Board board;
    MoveGenerator generator;
    Evaluator evaluator;
    SearchParams searchParams{};
    long long searchStarted = 0;
    bool searchActive = false;

    int quiescence(int alpha, int beta);
    int alphaBeta(int depthLeft, int alpha, int beta);
    void rootSearch();
    [[nodiscard]] bool canSearch();
};