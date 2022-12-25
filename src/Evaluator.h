//
// Created by marty on 2022-11-17.
//

#ifndef BOOMCHESS_SRC_EVALUATOR_H_
#define BOOMCHESS_SRC_EVALUATOR_H_

#include "Board.h"
#include "MoveGenerator.h"
class Evaluator {
 public:
    Evaluator();
    int evaluate(Board &board);
    int evaluateRelative(Board &board);

 private:
    std::array<int, 10> pieceWeights{};
    MoveGenerator generator{};

    int getWinState(Board &board);
    int materialAdvantage(Board &board);
    int pieceSquareTable(Board &board);;
};

#endif //BOOMCHESS_SRC_EVALUATOR_H_
