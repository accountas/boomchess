//
// Created by marty on 2022-11-05.
//

#ifndef BOOMCHESS_DRIVER_H
#define BOOMCHESS_DRIVER_H

#include "Board.h"
#include "MoveGenerator.h"
#include "TranspositionTable.h"

class Driver {
public:
    void start();
private:

    void perft(int depth, const std::string &fen, bool divide);
    int perft(int maxDepth, int depth, bool divide, Board &board, MoveGenerator &generator);
    void perftTT(int depth, const std::string &fen, bool divide);


    struct PerftEntry {
        uint64_t hash = 0;
        int depth = 0;
        int count = 0;
    };
    int perftTT(int maxDepth,
                int depth,
                bool divide,
                Board &board,
                MoveGenerator &generator,
                TranspositionTable<PerftEntry, TT_SIZE> &tTable,
                int &cacheHits);
};



#endif //BOOMCHESS_DRIVER_H
