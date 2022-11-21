//
// Created by marty on 2022-11-17.
//

#include <iostream>
#include <cassert>
#include "Search.h"
#include "Metrics.h"

SearchResult Search::search(int depth) {
    numChecked = 0;
    cacheHits = 0;

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
    int alphaStart = alpha;
    numChecked++;

    //get eval from TT
    auto hash = board.zobristKey.value;

//    if (tTable.at(hash).zobristKey == hash && tTable.at(hash).depth >= depthLeft) {
//        cacheHits++;
//        auto entry = tTable.at(hash);
//        if (entry.bound == EXACT) {
//            return entry.value;
//        } else if (entry.bound == LOWER_BOUND && entry.value > alpha) {
//            alpha = entry.value;
//        } else if (entry.bound == UPPER_BOUND && entry.value < beta) {
//            beta = entry.value;
//        }
//
//        if (alpha >= beta) {
//            return entry.value;
//        }
//    }

    if (depthLeft <= 0) {
        return evaluator.evaluateRelative(board);
    }

    generator.increaseDepth();
    generator.generateMoves(board);

    int movesChecked = 0;

    //null move heuristic
//    if (!board.isInCheck() && !board.madeNullMove) {
//        board.makeMove(Move(1, 1, MoveFlags::NULL_MOVE));
//        board.madeNullMove = true;
//        int nullEval = -alphaBeta(depthLeft - 2, -beta, -beta + 1);
//        board.unmakeMove();
//        board.madeNullMove = false;
//        if (nullEval >= beta) {
//            generator.decreaseDepth();
//            return nullEval;
//        }
//    }

    if(board.zobristKey.value != hash){
        printf("%s \n", board.toString().c_str());
    }

    //other moves
    int bestMove = 0;
    int value = EVAL_MIN;
    for (int i = 0; i < generator.size(); i++) {
        auto before = board.zobristKey.value;
        board.makeMove(generator.getSorted(i, board));

        if (!board.isLegal()) {
            board.unmakeMove();
            continue;
        }

        movesChecked++;
        int eval = -alphaBeta(depthLeft - 1, -beta, -alpha);
        board.unmakeMove();

        auto after = board.zobristKey.value;
        if(before != after){
            printf("whoa\n");
        }

        if (eval > value) {
            bestMove = i;
            value = eval;
            if(eval > alpha){
                alpha = value;
            }
        }
        if(alpha >= beta){
            break;
        }

    }

    if (movesChecked == 0) {
        generator.decreaseDepth();
        return evaluator.evaluateRelative(board);
    }

    if(hash != board.zobristKey.value){
        printf("nooo\n");
        system("pause");
        exit(0);
    }

    if (tTable.at(hash).depth <= depthLeft && !board.madeNullMove) {
        SearchEntry ttEntry;
        ttEntry.value = value;
        ttEntry.bestMove = generator[bestMove];
        ttEntry.depth = depthLeft;
        ttEntry.zobristKey = hash;
        if (value <= alphaStart) {
            ttEntry.bound = UPPER_BOUND;
        } else if (value >= beta) {
            ttEntry.bound = LOWER_BOUND;
        } else {
            ttEntry.bound = EXACT;
        }
        tTable.store(hash, ttEntry);
    }

    generator.decreaseDepth();
    return value;
}
