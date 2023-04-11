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
    std::pair<Move, int>  findBestMove(const SearchParams &params);
    void resetCache();
    void killSearch();

 private:
    Board board;
    MoveGenerator generator;
    Evaluator evaluator;
    SearchParams searchParams{};
    long long searchStarted = 0;
    bool searchActive = false;

    TranspositionTable<SearchEntry, TT_SIZE> tTable{};
    int quiescence(int alpha, int beta);
    int alphaBeta(int depthLeft, int alpha, int beta);
    std::pair<Move, int>  rootSearch();
    [[nodiscard]] bool canSearch();
};