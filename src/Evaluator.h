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
    MoveGenerator generator{true};

    int getWinState(Board &board);
    static int materialAdvantage(Board &board);
    static int pieceSquareTable(Board &board);
    static int getExplosionScore(const Board &board, int idx);
    static int lookupSquareBonus(int idx, int piece, int color);
    int mobilityBonus(Board &board);
};

#endif //BOOMCHESS_SRC_EVALUATOR_H_
