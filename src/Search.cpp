//
// Created by marty on 2022-11-17.
//

#include <iomanip>
#include <thread>
#include "Search.h"
#include "Metrics.h"
#include "UCI.h"
#include "Timer.h"

void Search::startSearch(const SearchParams &params) {
    if (!canSearch) {
        canSearch = true;
        std::thread([&, params]() { rootSearch(params); }).detach();

        if (params.timeLimit > 0) {
            std::thread([&, params]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(params.timeLimit));
                killSearch();
            }).detach();
        }
    }
}

void Search::killSearch() {
    canSearch = false;
}

void Search::rootSearch(const SearchParams &params) {
    auto boardStart = board;

    Move bestMove(0, 0, MoveFlags::NULL_MOVE);
    int bestEval = EVAL_MIN;

    Timer timer;
    timer.start();
    Metric<NODES_SEARCHED>::set(0);
    Metric<Q_NODES_SEARCHED>::set(0);
    Metric<LEAF_NODES_SEARCHED>::set(0);

    generator.setDepth(0);

    int depthStart;
#ifdef USE_ID
    depthStart = 0;
#else
    depthStart = params.depthLimit - 1;
#endif

    int currentDepth;
    for (currentDepth = depthStart; currentDepth < params.depthLimit; currentDepth++) {
        generator.ageHistory(0);
        generator.ageHistory(1);
        generator.generateMoves(board);
        if (currentDepth > 0) {
            generator.sortTT(bestMove);
        }

        int alpha = -1e9;
        int beta = 1e9;

        for (int i = 0; i < generator.size(); i++) {
            auto move = generator.getSorted(i, board);
            board.makeMove(move);

            if (!board.isLegal()) {
                board.unmakeMove();
                continue;
            }

            if (bestMove.flags & MoveFlags::NULL_MOVE) {
                bestMove = generator[i];
            }

            int eval = -alphaBeta(currentDepth, -beta, -alpha);
            board.unmakeMove();

            if (!canSearch) {
                goto end;
            }
            if (i == 0 || eval > bestEval) {
                bestEval = eval;
                bestMove = move;
            }
            if(eval > alpha){
                alpha = eval;
            }
            if (bestEval == EVAL_MAX) {
                goto end;
            }

        }
        UCI::sendInfo(currentDepth + 1, bestEval, bestMove, timer.getSecondsFromStart());
    }

    end:
    //if finnish earlier than move time limit, wait till clock stops
    while (params.timeLimit > 0 && canSearch) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    UCI::sendInfo(currentDepth + 1, bestEval, bestMove, timer.getSecondsFromStart());


    board = boardStart;
    canSearch = false;
    UCI::sendResult(bestMove);
}

int Search::alphaBeta(int depthLeft, int alpha, int beta) {
    if (!canSearch) {
        return 0;
    }
    if (board.isRepetition()) {
        return 0;
    }

    int alphaStart = alpha;
    Metric<NODES_SEARCHED>::inc();
    Move ttMove(0, 0, MoveFlags::NULL_MOVE);

#ifdef USE_TT
    //lookup transposition table
    auto hash = board.zobristKey.value;
    if (tTable.at(hash).zobristKey == hash) {
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
#endif

    //base case
    if (depthLeft <= 0) {
        Metric<LEAF_NODES_SEARCHED>::inc();
        return quiescence(alpha, beta);
    }

    //some helpers
    bool isPV = beta - alpha != 1;

    //null move heuristic
#ifdef USE_NULL_MOVE
    if (!board.isKingCaptured()
        && !board.madeNullMove
        && depthLeft > NULL_MOVE_R
        && !isPV
        && !board.onlyPawns()
        && !board.isInCheck()) {

        board.makeMove(Move(1, 1, MoveFlags::NULL_MOVE));
        board.madeNullMove = true;
        int nullEval = -alphaBeta(depthLeft - NULL_MOVE_R - 1, -beta, -beta + 1);
        board.unmakeMove();
        board.madeNullMove = false;
        if (nullEval >= beta) {
            return nullEval;
        }
    } else if (board.madeNullMove) {
        board.madeNullMove = false;
    }
#endif

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
        auto move = generator.getSorted(i, board);
        board.makeMove(move);

        if (!board.isLegal()) {
            board.unmakeMove();
            continue;
        }

        movesChecked++;
        int eval;
#ifdef USE_PVS
        if (i == 0) {
            eval = -alphaBeta(depthLeft - 1, -beta, -alpha);
        } else {
            eval = -alphaBeta(depthLeft - 1, -alpha - 1, -alpha);
            if (alpha < eval && eval < beta) {
                eval = -alphaBeta(depthLeft - 1, -beta, -alpha);
            }
        }
#else
        eval = -alphaBeta(depthLeft - 1, -beta, -alpha);
#endif
        board.unmakeMove();

        if (eval > value) {
            bestMove = i;
            value = eval;
            if (eval > alpha) {
                generator.updateHistory(0, move, depthLeft);
                alpha = value;
            }
        }
#ifndef NO_AB
        if (alpha >= beta) {
            generator.updateHistory(board.moveColor, move, depthLeft);
            generator.markKiller(i);
            break;
        }
#endif
    }


    //no legal moves, tie or lost
    if (movesChecked == 0) {
        generator.decreaseDepth();
        return board.isInCheck() || board.isKingCaptured() ? EVAL_MIN : 0;
    }

#ifdef USE_TT
    //save move to TT
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

    if (tTable.at(hash).zobristKey == 0) {
        Metric<TT_ENTRIES>::inc();
    }
    tTable.store(hash, ttEntry);
#endif

    generator.decreaseDepth();
    return value;
}

int Search::quiescence(int alpha, int beta) {
    Metric<Q_NODES_SEARCHED>::inc();

#ifndef USE_QSEARCH
    return evaluator.evaluateRelative(board);
#endif

    if (!canSearch) {
        return 0;
    }

    bool inCheck = board.isInCheck();

    //Standing Pat
    int bestScore = evaluator.evaluateRelative(board);
    if (bestScore > alpha) {
        alpha = bestScore;
    }
    if (bestScore >= beta) {
        return bestScore;
    }

    generator.increaseDepth();
    generator.generateMoves(board);

    for (int i = 0; i < generator.size(); i++) {
        auto move = generator.getSorted(i, board);

        if (!inCheck && !generator.isGoodCapture(i)) {
            break;
        }

        board.makeMove(move);

        if (!board.isLegal()) {
            board.unmakeMove();
            continue;
        }

        int score = -quiescence(-beta, -alpha);
        board.unmakeMove();

        if (score >= beta) {
            generator.decreaseDepth();
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }
        if(score > bestScore){
            bestScore = score;
        }
    }

    generator.decreaseDepth();
    return bestScore;
}



