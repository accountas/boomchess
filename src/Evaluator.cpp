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

int Evaluator::pieceSquareTable(Board &board) {
    int totalPstBonus = 0;
    int totalSafeSquareBonus = 0;

    for (int piece : PieceTypes) {
        for (int color : {WHITE, BLACK}) {
            int mul = color == WHITE ? 1 : -1;
            for (int i = 0; i < board.pieceCounts[color][piece]; i++) {
                int piecePos = board.pieces[color][piece][i];

                //piece square tables
                int pstBonus = lookupSquareBonus(piecePos, piece, color);
                totalPstBonus += pstBonus * mul;

                //safe square bonus
                if (piece != KING && piece != PAWN) {
                    int explosionValue = getExplosionScore(board, piecePos) * (-mul);
                    int pieceValue = EvalParams::PieceWeights[piece];

                    //if this piece can`t be taken without loosing material then give bonus
                    if (pstBonus > 0 && explosionValue >= pieceValue) {
                        totalSafeSquareBonus += pstBonus * 100 / EvalParams::SAFE_SQUARE_BONUS * mul;
                    }
                }
            }
        }
    }

    return totalPstBonus + totalSafeSquareBonus;
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

int Evaluator::lookupSquareBonus(int idx, int piece, int color) {
    int file = Board::indexToFile(idx);
    int rank = Board::indexToRank(idx);
    int lookUpIdx = color == WHITE ? (7 - rank) * 8 + file : rank * 8 + file;
    return EvalParams::PieceSquareTables[piece][lookUpIdx];
}

int Evaluator::getExplosionScore(const Board &board, int idx) {
    int score = 0;
    for (int direction : explosionDirections) {
        int victimIdx = idx + direction;
        if (Board::inBounds(victimIdx) && !board.isEmpty(idx) && board[idx].type() != PAWN) {
            score += EvalParams::PieceWeights[board[victimIdx].type()] * board[idx].color() == WHITE ? 1 : -1;
        }
    }
    return score;
}


