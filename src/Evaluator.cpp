//
// Created by marty on 2022-11-17.
//

#include "Evaluator.h"
#include "Common.h"
#include "EvalParms.h"
Evaluator::Evaluator() {

}

int Evaluator::evaluateRelative(Board &board) {
    return evaluate(board) * (board.moveColor == WHITE ? 1 : -1);
}
int Evaluator::evaluate(Board &board) {
    int winState = getWinState(board);

    if (winState == WinState::LOST) {
        return board.moveColor == WHITE ? EVAL_MIN : EVAL_MAX;
    }
    if (winState == WinState::TIE) {
        return 0;
    }

    int score = 0;
    score += materialAdvantage(board);
    score += pieceSquareTable(board);

    return score;
}

int Evaluator::materialAdvantage(Board &board) {
    int score = 0;
    for (int piece : PieceTypes) {
        score += EvalParams::PieceWeights[piece] * (board.pieceCounts[WHITE][piece] - board.pieceCounts[BLACK][piece]);
    }
    return score;
}

//TODO: make this lazy maybe
int Evaluator::pieceSquareTable(Board &board) {
    int score = 0;

    //white
    for (int piece : PieceTypes) {
        for (int i = 0; i < board.pieceCounts[WHITE][piece]; i++) {
            int piecePos = board.pieces[WHITE][piece][i];
            int file = Board::indexToFile(piecePos);
            int rank = Board::indexToRank(piecePos);
            int lookUpIdx = (7 - rank) * 8 + file;
            score += EvalParams::PieceSquareTables[piece][lookUpIdx];
        }
    }

    //black
    for (int piece : PieceTypes) {
        for (int i = 0; i < board.pieceCounts[BLACK][piece]; i++) {
            int piecePos = board.pieces[BLACK][piece][i];
            int file = Board::indexToFile(piecePos);
            int rank = Board::indexToRank(piecePos);
            int lookUpIdx = rank * 8 + file;
            score -= EvalParams::PieceSquareTables[piece][lookUpIdx];
        }
    }

    return score;
}

int Evaluator::getWinState(Board &board) {
    if (board.isKingCaptured())
        return WinState::LOST;

    if (board.numHalfMoves >= 100) {
        return WinState::TIE;
    }

    bool hasLegalMoves = false;

    auto checkMoves = [&](int piece) {
        if(!hasLegalMoves){
            generator.generateMoves(board, piece);
            for (int i = 0; i < generator.size() && !hasLegalMoves; i++) {
                board.makeMove(generator[i]);
                hasLegalMoves |= board.isLegal();
                board.unmakeMove();
            }
        }

    };

    //do this in parts to exit quicker
    checkMoves(KING);
    checkMoves(PAWN);
    checkMoves(KNIGHT);
    checkMoves(BISHOP);
    checkMoves(ROOK);
    checkMoves(QUEEN);

    if (!hasLegalMoves) {
        return board.isInCheck() ? WinState::LOST : WinState::TIE;
    }

    return WinState::NORMAL;
}


