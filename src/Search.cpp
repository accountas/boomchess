//
// Created by marty on 2022-11-17.
//

#include <iostream>
#include "Search.h"

SearchResult Search::search(int depth) {
    numChecked = 0;

    int bestEval = EVAL_MIN;
    Move bestMove;

    generator.increaseDepth();
    generator.generateMoves(board);
    for (int i = 0; i < generator.size(); i++) {
        auto move = generator.getSorted(i, board);

        board.makeMove(move);
        if (board.isLegal()) {
            int eval = -alphaBeta(depth - 1, -1e9, 1e9);
            if (eval > bestEval) {
                bestEval = eval;
                bestMove = move;
            }
        }
        board.unmakeMove();
    }

    return {bestMove, bestEval};
}

int Search::alphaBeta(int depthLeft, int alpha, int beta) {
    numChecked++;
    if (depthLeft <= 0) {
        return evaluator.evaluateRelative(board);
    }

    generator.increaseDepth();
    generator.generateMoves(board);

    int movesChecked = 0;

    //null move heuristic
    if(!board.isInCheck() && !board.madeNullMove){
        board.makeMove(Move(0, 0, 0));
        board.madeNullMove = true;
        int nullEval = -alphaBeta(depthLeft - 2, -beta, -beta + 1);
        board.unmakeMove();
        board.madeNullMove = false;
        if(nullEval >= beta){
            generator.decreaseDepth();
            return nullEval;
        }
    }


    //other moves
    for (int i = 0; i < generator.size(); i++) {
        board.makeMove(generator.getSorted(i, board));

        if (!board.isLegal()) {
            board.unmakeMove();
            continue;
        }

        movesChecked++;
        int eval = -alphaBeta(depthLeft - 1, -beta, -alpha);
        board.unmakeMove();

        if (eval >= beta) {
            alpha = beta;
            break;
        }
        if (eval > alpha) {
            alpha = eval;
        }
    }

    if (movesChecked == 0) {
        generator.decreaseDepth();
        return evaluator.evaluateRelative(board);
    }

    generator.decreaseDepth();
    return alpha;
}
