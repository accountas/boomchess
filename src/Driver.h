//
// Created by marty on 2022-11-05.
//

#ifndef BOOMCHESS_DRIVER_H
#define BOOMCHESS_DRIVER_H

#include "Board.h"
#include "MoveGenerator.h"
#include "TranspositionTable.h"
#include "Search.h"

class Driver {
 public:
    void start();

 private:
    int perft(int depth, const std::string &fen, bool divide);
    int perft(int maxDepth, int depth, bool divide, Board &board, MoveGenerator &generator);
    std::vector<std::string> tokenizeString(const std::string &s, char delimiter);
    void uciMode();
    void perftTest();
};

#endif //BOOMCHESS_DRIVER_H
