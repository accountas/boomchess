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
    generator.generateMoves(board);

    int winState = getWinState(board);

    if (winState == WinState::LOST) {
        return board.moveColor == WHITE ? EVAL_MIN : EVAL_MAX;
    }
    if (winState == WinState::TIE) {
        return 0;
    }

    int score = 0;
    score += materialAdvantage(board);
    score += evalPieces(board);
    score += mobilityBonus(board);
    score += kingSafety(board);
    return score;
}

int Evaluator::mobilityBonus(Board &board) {
    int currentPlayerMoves = generator.size();

    board.makeMove(Move(0, 0, MoveFlags::NULL_MOVE));
    generator.generateMoves(board);
    board.unmakeMove();

    int otherPlayerMoves = generator.size();

    int delta;
    if (board.moveColor == WHITE) {
        delta = currentPlayerMoves - otherPlayerMoves;
    } else {
        delta = otherPlayerMoves - currentPlayerMoves;
    }

    return delta * EvalParams::MOBILITY_WEIGHT;
}

int Evaluator::materialAdvantage(Board &board) {
    int score = 0;
    for (int piece : PieceTypes) {
        score += EvalParams::PieceWeights[piece] * (board.pieceCounts[WHITE][piece] - board.pieceCounts[BLACK][piece]);
    }
    return score;
}

int Evaluator::evalPieces(Board &board) {
    int totalPstBonus = 0;
    int totalSafeSquareBonus = 0;
    int unsafeSquarePenalty = 0;
    int passedPawnBonus = 0;
    short pawnRanks[2][10] = {};

    for (int piece : PieceTypes) {
        for (int color : {WHITE, BLACK}) {
            int mul = color == WHITE ? 1 : -1;
            for (int i = 0; i < board.pieceCounts[color][piece]; i++) {
                int piecePos = board.pieces[color][piece][i];

                //piece square tables
                int pstBonus = lookupSquareBonus(piecePos, piece, color);
                totalPstBonus += pstBonus * mul;

                //safe square bonus
//                if (piece != KING && piece != PAWN) {
//                    int explosionValue = getExplosionScore(board, piecePos) * mul;
//                    int pieceValue = EvalParams::PieceWeights[piece];
//
//                    //if this piece can`t be taken without loosing material then give bonus
//                    if (explosionValue >= pieceValue) {
//                        totalSafeSquareBonus += (abs(pstBonus) * EvalParams::SAFE_SQUARE_BONUS / 100) * mul;
//                    }
//                }

                //update pawn table
                if (piece == PAWN) {
                    pawnRanks[color][Board::indexToFile(piecePos) + 1] = (short) (Board::indexToRank(piecePos) + 1);
                }
            }
        }
    }

    //calculate passed pawn bonuses
    for (int i = 1; i <= 8; i++) {
        bool whiteExists = pawnRanks[WHITE][i] != 0;
        bool blackExists = pawnRanks[BLACK][i] != 0;

        if ((whiteExists ^ blackExists) == false) {
            continue;
        }

        int color = whiteExists ? WHITE : BLACK;
        int mul = whiteExists ? 1 : -1;
        int rank = pawnRanks[color][i] * mul;
        int leftRank = pawnRanks[color ^ 1][i - 1] * mul;
        int rightRank = pawnRanks[color ^ 1][i + 1] * mul;
        int relativeRank = color == WHITE ? rank * mul : 9 - rank * mul;

        if ((leftRank == 0 || leftRank <= rank) && (rightRank == 0 || rightRank <= rank)) {
            passedPawnBonus += EvalParams::PASSED_PAWN_BONUS[relativeRank - 1];
        }
    }

    return totalPstBonus + totalSafeSquareBonus + unsafeSquarePenalty + passedPawnBonus;
}

int Evaluator::kingSafety(Board &board) {
    int attackedSquares[2] = {};
    int touchingSquares[2] = {};

    for (int i = 0; i <= 1; i++) {
        int color = board.moveColor;
        int kingPos = board.pieces[color][KING][0];
        for (int dir : explosionDirections) {
            int idx = kingPos + dir;
            if (Board::inBounds(kingPos + dir)) {
                bool touchesOwn = !board.isEmpty(idx) && board[idx].color() == color;
                bool isAttacked = board.isAttacked(idx);

                touchingSquares[color] += touchesOwn;
                attackedSquares[color] += isAttacked;
                attackedSquares[color] += isAttacked && touchesOwn;
            }
        }
        if (i == 0) {
            board.makeMove(Move(0, 0, MoveFlags::NULL_MOVE));
        } else {
            board.unmakeMove();
        }
    }

    int attackEval = (attackedSquares[BLACK] - attackedSquares[WHITE]) * EvalParams::ATTACKED_KING_SQUARE_BONUS;
    int touchEval = (touchingSquares[BLACK] - touchingSquares[WHITE]) * EvalParams::KING_TOUCH_PENALTY;
    return attackEval + touchEval;
}


int Evaluator::getWinState(Board &board) {
    if (board.isKingCaptured())
        return WinState::LOST;

    if (board.numHalfMoves >= 100) {
        return WinState::TIE;
    }

    bool hasLegalMoves = false;
    for (int i = 0; i < generator.size() && !hasLegalMoves; i++) {
        board.makeMove(generator[i]);
        hasLegalMoves |= board.isLegal();
        board.unmakeMove();
    }

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
        if (Board::inBounds(victimIdx) && !board.isEmpty(victimIdx) && board[victimIdx].type() != PAWN) {
            score += EvalParams::PieceWeights[board[victimIdx].type()] * (board[victimIdx].color() == WHITE ? -1 : 1);
        }
    }
    return score;
}


