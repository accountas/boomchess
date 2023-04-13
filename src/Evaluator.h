#pragma once

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
    static int getExplosionScore(const Board &board, int idx);
    static int lookupSquareBonus(int idx, int piece, int color);
    int mobilityBonus(Board &board);
    int evalPieces(Board &board, int phase);
    static int kingSafety(Board &board);
    static int getKingDistanceFactor(const Board &board, int phase);
    int getPhase(const Board &board);
    static int interpolateScore(int midgame, int endgame, int phase);
};
