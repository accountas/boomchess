//
// Created by marty on 2022-11-17.
//

#include <iostream>
#include <cassert>
#include <iomanip>
#include "Search.h"
#include "Metrics.h"
#include "Timer.h"

void Search::search(int depth) {
    Timer timer;
    //iterative deepening
    Move bestMove(0, 0, MoveFlags::NULL_MOVE);
    int bestEval = EVAL_MIN;
    for (int currentDepth = 0; currentDepth < depth; currentDepth++) {
        timer.start();
        numChecked = 0;
        cacheHits = 0;
        generator.generateMoves(board);
        if (currentDepth > 0) {
            generator.sortTT(bestMove);
        }
        for (int i = 0; i < generator.size(); i++) {
            auto move = generator.getSorted(i, board);
            board.makeMove(move);
            if (board.isLegal()) {
                int eval = -alphaBeta(currentDepth, -1e9, 1e9);
                if (eval > bestEval) {
                    bestEval = eval;
                    bestMove = move;
                }
            }
            board.unmakeMove();
        }
        timer.end();
        std::cout << "Depth: " << std::setw(2) << currentDepth + 1 << " | ";
        std::cout << "Eval: " << std::setprecision(2) << std::setw(5) << bestEval / 100.0 << " | ";
        std::cout << "Best Move: " << Board::moveToString(bestMove) << " | ";
        std::cout << std::setprecision(4);
        std::cout << "Duration: " << std::setw(8) << std::fixed << timer.getSeconds() << " | ";
        std::cout << "NPS: " << std::setw(8) << std::fixed << numChecked / timer.getSeconds() << "\n";
    }
}

int Search::alphaBeta(int depthLeft, int alpha, int beta) {
    int alphaStart = alpha;
    numChecked++;

    //lookup transposition table
    auto hash = board.zobristKey.value;
    Move ttMove(0, 0, MoveFlags::NULL_MOVE);
    if (tTable.at(hash).zobristKey == hash) {
        cacheHits++;
        auto entry = tTable.at(hash);

        if (entry.depth >= depthLeft) {
            if (entry.bound == EXACT) {
                return entry.value;
            } else if (entry.bound == LOWER_BOUND && entry.value > alpha) {
                alpha = entry.value;
            } else if (entry.bound == UPPER_BOUND && entry.value < beta) {
                beta = entry.value;
            }

            if (alpha >= beta) {
                return entry.value;
            }
        }

        ttMove = entry.bestMove;
    }

    //base case
    if (depthLeft <= 0) {
        return evaluator.evaluateRelative(board);
    }

    //null move heuristic
    if (!board.isInCheck() && !board.isKingCaptured() && !board.madeNullMove) {
        board.makeMove(Move(1, 1, MoveFlags::NULL_MOVE));
        board.madeNullMove = true;
        int nullEval = -alphaBeta(depthLeft - 2, -beta, -beta + 1);
        board.unmakeMove();
        board.madeNullMove = false;
        if (nullEval >= beta) {
            return nullEval;
        }
    }

    generator.increaseDepth();
    generator.generateMoves(board);

    //if we have move from TT sort it to first position
    if (!(ttMove.flags & MoveFlags::NULL_MOVE)) {
        generator.sortTT(ttMove);
    }

    //iterate over possible moves
    int movesChecked = 0;
    int bestMove = 0;
    int value = EVAL_MIN;
    for (int i = 0; i < generator.size(); i++) {
        board.makeMove(generator.getSorted(i, board));

        if (!board.isLegal()) {
            board.unmakeMove();
            continue;
        }

        movesChecked++;
        int eval = -alphaBeta(depthLeft - 1, -beta, -alpha);
        board.unmakeMove();

        if (eval > value) {
            bestMove = i;
            value = eval;
            if (eval > alpha) {
                alpha = value;
            }
        }
        if (alpha >= beta) {
            generator.markKiller(i);
            break;
        }
    }

    //no legal moves, tie or lost
    if (movesChecked == 0) {
        generator.decreaseDepth();
        return evaluator.evaluateRelative(board);
    }

    //save move to TT
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


