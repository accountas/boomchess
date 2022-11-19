//
// Created by marty on 2022-11-05.
//

#ifndef BOOMCHESS_DRIVER_H
#define BOOMCHESS_DRIVER_H

#include "Board.h"
#include "MoveGenerator.h"

class Driver {
public:
    void start();
private:
    void perft(int depth, std::string fen, bool divide = false);
    int perft(int maxDepth, int depth, bool divide, Board &board, MoveGenerator &generator);
};


#endif //BOOMCHESS_DRIVER_H
