#include <iomanip>
#include <thread>
#include "Search.h"
#include "Metrics.h"
#include "UCI.h"
#include "Timer.h"

void Search::startSearch(const SearchParams &params) {
    if(searchActive){
        return;
    }

    searchActive = true;
    searchStarted = Timer::getMillis();
    searchParams = params;
    rootSearch();
//    std::thread([&]() { rootSearch(); }).detach();
}

void Search::rootSearch() {
    auto boardStart = board;

    Move bestMove(0, 0, MoveFlags::NULL_MOVE);
    int bestEval = EVAL_MIN;

    Timer timer;
    timer.start();

    Metric<NODES_SEARCHED>::set(0);
    Metric<Q_NODES_SEARCHED>::set(0);
    Metric<LEAF_NODES_SEARCHED>::set(0);

    generator.setDepth(0);

    int currentDepth;
    for (currentDepth = 0; currentDepth < searchParams.depthLimit; currentDepth++) {
        generator.ageHistory(WHITE);
        generator.ageHistory(BLACK);
        generator.clearKillers();
        generator.generateMoves(board);
        generator.sortTT(bestMove);


        int alpha = -1e9;
        int beta = 1e9;

        for (int i = 0; i < generator.size(); i++) {
            auto move = generator.getSorted(i, board);

            if (!board.tryMakeMove(move)) continue;

            if (bestMove.flags & MoveFlags::NULL_MOVE) {
                bestMove = generator[i];
            }

            int eval = -alphaBeta(currentDepth, -beta, -alpha);

            board.unmakeMove();

            if (!canSearch()) {
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
        evals[currentDepth] = {bestEval, bestMove};
        UCI::sendInfo(currentDepth + 1, bestEval, bestMove, timer.getSecondsFromStart());
    }

    end:

    depthReached = currentDepth;
    evals[currentDepth] = {bestEval, bestMove};
    UCI::sendInfo(currentDepth + 1, bestEval, bestMove, timer.getSecondsFromStart());

    board = boardStart;
    searchActive = false;
    UCI::sendResult(bestMove);
}

int Search::alphaBeta(int depthLeft, int alpha, int beta) {
    if (!canSearch()) {
        return 0;
    }
    if (board.isRepetition()) {
        return 0;
    }

    int alphaStart = alpha;
    Metric<NODES_SEARCHED>::inc();
    Move ttMove(0, 0, MoveFlags::NULL_MOVE);

    // lookup transposition table
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

    //base case
    if (depthLeft <= 0) {
        Metric<LEAF_NODES_SEARCHED>::inc();
        return quiescence(alpha, beta);
    }

    //some helpers
    bool isPV = beta - alpha != 1;

    //null move heuristic
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

    generator.increaseDepth();
    generator.generateMoves(board);

    //if we have move from TT sort it to first position
    if (!(ttMove.flags & MoveFlags::NULL_MOVE)) {
        generator.sortTT(ttMove);
    }

    //iterate over possible moves
    int legalMovesFound = 0;
    int bestMoveIdx = 0;
    int value = EVAL_MIN;

    for (int i = 0; i < generator.size(); i++) {
        auto move = generator.getSorted(i, board);

        if(!board.tryMakeMove(move)) continue;

        legalMovesFound++;

        // Principal variation search
        int eval;
        if (i == 0) {
            eval = -alphaBeta(depthLeft - 1, -beta, -alpha);
        } else {
            eval = -alphaBeta(depthLeft - 1, -alpha - 1, -alpha);
            if (alpha < eval && eval < beta) {
                eval = -alphaBeta(depthLeft - 1, -beta, -alpha);
            }
        }

        board.unmakeMove();

        if (eval > value) {
            bestMoveIdx = i;
            value = eval;
            if (eval > alpha) {
                alpha = value;
            }
        }

        //beta cutoff
        if (alpha >= beta) {
            generator.updateHistory(board.moveColor, move, depthLeft);
            generator.markKiller(i);
            break;
        }
    }

    //no legal moves, tie or lost
    if (legalMovesFound == 0) {
        generator.decreaseDepth();
        return board.isInCheck() || board.isKingCaptured() ? EVAL_MIN : 0;
    }

    //save move to TT
    SearchEntry ttEntry;
    ttEntry.value = value;
    ttEntry.bestMove = generator[bestMoveIdx];
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
        Metric<TT_WRITTEN>::inc();
    }
    tTable.store(hash, ttEntry);

    generator.decreaseDepth();
    return value;
}

int Search::quiescence(int alpha, int beta) {
    Metric<Q_NODES_SEARCHED>::inc();

    if (!canSearch()) {
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

        if(!board.tryMakeMove(move)) continue;

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

bool Search::canSearch() {
    if (!searchActive) {
        return false;
    }

    long long nodesSearched = Metric<NODES_SEARCHED>::get()
        - Metric<LEAF_NODES_SEARCHED>::get()
        + Metric<Q_NODES_SEARCHED>::get();

    //if over the node limit
    if(searchParams.nodeLimit > 0 && nodesSearched >= searchParams.nodeLimit){
        searchActive = false;
        return false;
    }

    //if over the time limit, (only check every 2^14 nodes for smaller overhead)
    if(searchParams.timeLimit > 0
        && (nodesSearched & ((1 << 14) - 1)) == 0
        && Timer::getMillis() - searchStarted >= searchParams.timeLimit){
        searchActive = false;
        return false;
    }

    return true;
}

void Search::resetCache() {
    Metric<TT_WRITTEN>::set(0);
    tTable.clear();
    generator.clearHistory();
    generator.clearKillers();
}
void Search::killSearch() {
    searchActive = false;
}
