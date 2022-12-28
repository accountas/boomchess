//
// Created by marty on 2022-11-09.
//

#include "MoveGenerator.h"
#include "Common.h"
#include "EvalParms.h"
#include <tuple>

void MoveGenerator::generateMoves(const Board &board, int piece) {
    n[curDepth] = 0;
    nSorted[curDepth] = 0;

    //no legal moves if king ded
    if (board.pieceCounts[board.moveColor][KING] == 0) {
        return;
    }

    if (piece == -1 || piece == PAWN) {
        generatePawnMoves(board);
    }
    if (piece == -1 || piece == KNIGHT) {
        generateKnightMoves(board);
    }
    if (piece == -1 || piece == BISHOP) {
        generateBishopMoves(board);
    }
    if (piece == -1 || piece == QUEEN) {
        generateQueenMoves(board);
    }
    if (piece == -1 || piece == KING) {
        generateKingMoves(board);
    }
    if (piece == -1 || piece == ROOK) {
        generateRookMoves(board);
    }
}

void MoveGenerator::generatePawnMoves(const Board &board) {
    int forward = board.moveColor == Color::WHITE ? Direction::UP : Direction::DOWN;
    int startingRank = board.moveColor == Color::WHITE ? 1 : 6;
    int promotionRank = board.moveColor == Color::WHITE ? 6 : 1;

    for (int i = 0; i < board.pieceCounts[board.moveColor][PieceType::PAWN]; i++) {
        int square = board.pieces[board.moveColor][PieceType::PAWN][i];

        //forward or promote
        bool forwardClear = board.isEmpty(square + forward);
        if (forwardClear && Board::indexToRank(square) == promotionRank) {
            addMove(square, square + forward, MoveFlags::BISHOP_PROMOTION | MoveFlags::PAWN_MOVE);
            addMove(square, square + forward, MoveFlags::KNIGHT_PROMOTION | MoveFlags::PAWN_MOVE);
            addMove(square, square + forward, MoveFlags::ROOK_PROMOTION | MoveFlags::PAWN_MOVE);
            addMove(square, square + forward, MoveFlags::QUEEN_PROMOTION | MoveFlags::PAWN_MOVE);
        } else if (forwardClear) {
            addMove(square, square + forward);
        }

        //first double move
        if (Board::indexToRank(square) == startingRank
            && forwardClear
            && board.isEmpty(square + forward * 2)) {
            addMove(square, square + forward * 2, MoveFlags::DOUBLE_PAWN | MoveFlags::PAWN_MOVE);
        }

        //attack moves
        int attackRight = square + forward + Direction::RIGHT;
        if (Board::inBounds(attackRight)
            && !board.isEmpty(attackRight)
            && board.isEnemy(attackRight)) {
            addMove(square, attackRight, MoveFlags::CAPTURE | MoveFlags::PAWN_MOVE);
            calculateLatestCaptureScore(board);

        }

        int attackLeft = square + forward + Direction::LEFT;
        if (Board::inBounds(attackLeft)
            && !board.isEmpty(attackLeft)
            && board.isEnemy(attackLeft)) {
            addMove(square, attackLeft, MoveFlags::CAPTURE | MoveFlags::PAWN_MOVE);
            calculateLatestCaptureScore(board);

        }

        //en passant
        if (board.enPassantSquare != -1
            && square + Direction::LEFT == board.enPassantSquare
            && board.isEnemy(square + Direction::LEFT)
            && board.isEmpty(square + Direction::LEFT + forward)) {
            addMove(square,
                    square + Direction::LEFT + forward,
                    MoveFlags::CAPTURE | MoveFlags::EN_PASSANT_CAPTURE | MoveFlags::PAWN_MOVE);
            calculateLatestCaptureScore(board);
        }
        if (board.enPassantSquare != -1
            && square + Direction::RIGHT == board.enPassantSquare
            && board.isEnemy(square + Direction::RIGHT)
            && board.isEmpty(square + Direction::RIGHT + forward)) {
            addMove(square,
                    square + Direction::RIGHT + forward,
                    MoveFlags::CAPTURE | MoveFlags::EN_PASSANT_CAPTURE | MoveFlags::PAWN_MOVE);
            calculateLatestCaptureScore(board);
        }

    }
}
void MoveGenerator::generateKingMoves(const Board &board) {
    int square = board.pieces[board.moveColor][PieceType::KING][0];

    //move king (Kings can't capture)
    for (int direction : allDirections) {
        int newSquare = square + direction;
        if (Board::inBounds(newSquare) && board.isEmpty(newSquare)) {
            addMove(square, newSquare);
        }
    }

    //castling king side
    if (!fast && board.castlingRights[board.moveColor] & CastlingRight::KING_SIDE) {
        generateCastle(board, square, Direction::RIGHT);
    }

    //castling queen side
    if (!fast && board.castlingRights[board.moveColor] & CastlingRight::QUEEN_SIDE) {
        generateCastle(board, square, Direction::LEFT);
    }
}

void MoveGenerator::generateCastle(const Board &board, int kingSquare, int castleDirection) {
    bool canCastle = board.isEmpty(kingSquare + castleDirection)
        && board.isEmpty(kingSquare + castleDirection * 2)
        && (castleDirection == Direction::RIGHT || board.isEmpty(kingSquare + castleDirection * 3))
        && !board.isAttacked(kingSquare)
        && !board.isAttacked(kingSquare + castleDirection)
        && !board.isAttacked(kingSquare + castleDirection * 2);

    if (canCastle) {
        int flag = castleDirection == Direction::RIGHT ? MoveFlags::CASTLE_RIGHT : MoveFlags::CASTLE_LEFT;
        addMove(kingSquare, kingSquare + castleDirection * 2, flag);
    }
}

void MoveGenerator::generateKnightMoves(const Board &board) {
    for (int i = 0; i < board.pieceCounts[board.moveColor][PieceType::KNIGHT]; i++) {
        int square = board.pieces[board.moveColor][PieceType::KNIGHT][i];

        for (int direction : knightDirections) {
            int newSquare = square + direction;
            if (Board::inBounds(newSquare)) {
                if (board.isEmpty(newSquare)) {
                    addMove(square, newSquare);
                } else if (board.isEnemy(newSquare)) {
                    addMove(square, newSquare, MoveFlags::CAPTURE);
                    calculateLatestCaptureScore(board);
                }
            }
        }
    }
}

void MoveGenerator::generateBishopMoves(const Board &board) {
    for (int i = 0; i < board.pieceCounts[board.moveColor][PieceType::BISHOP]; i++) {
        int square = board.pieces[board.moveColor][PieceType::BISHOP][i];

        for (int direction : diagonalDirections) {
            generateSlidingMoves(board, square, direction);
        }
    }
}

void MoveGenerator::generateRookMoves(const Board &board) {
    for (int i = 0; i < board.pieceCounts[board.moveColor][PieceType::ROOK]; i++) {
        int square = board.pieces[board.moveColor][PieceType::ROOK][i];

        for (int direction : straightDirections) {
            generateSlidingMoves(board, square, direction);
        }
    }
}

void MoveGenerator::generateQueenMoves(const Board &board) {
    for (int i = 0; i < board.pieceCounts[board.moveColor][PieceType::QUEEN]; i++) {
        int square = board.pieces[board.moveColor][PieceType::QUEEN][i];

        for (int direction : allDirections) {
            generateSlidingMoves(board, square, direction);
        }
    }
}

void MoveGenerator::generateSlidingMoves(const Board &board, int startingSquare, int direction) {
    int currentSquare = startingSquare;
    while (true) {
        currentSquare += direction;
        if (!Board::inBounds(currentSquare)) {
            break;
        } else if (board.isEmpty(currentSquare)) {
            addMove(startingSquare, currentSquare);
        } else if (board.isEnemy(currentSquare)) {
            addMove(startingSquare, currentSquare, MoveFlags::CAPTURE);
            calculateLatestCaptureScore(board);
            break;
        } else {
            break;
        }
    }
}

void MoveGenerator::sortTill(int idx, const Board &board) {
    if (idx < nSorted[curDepth]) {
        return;
    }

    for (int i = nSorted[curDepth]; i <= idx; i++) {
        // ordering:
        // 1. winning/equal captures
        // 2. killers
        // 3. history

        int64_t bestMoveScore = 0;
        int bestMove = i;

        for (int j = i; j < n[curDepth]; j++) {
            auto move = (*this)[j];
            int64_t curMoveScore = 0;

            const int killerOffset = (MAX_HISTORY_TABLE_VAL + 1);
            const int mvvOffset = killerOffset * (KILLER_MOVES_N + 1);

            //assign MVV-LVA score, we want score to be > 0 for equal captures and < 0 for loosing captures
            if (move.flags & MoveFlags::CAPTURE) {
                int score = captureScore[curDepth][j];
                if (score >= 0) score++;
                curMoveScore += mvvOffset * score;
            }

            //  treat promotions as winning captures for ordering
            if (move.flags & MoveFlags::PROMOTION_SUBMASK) {
                if (move.flags & MoveFlags::QUEEN_PROMOTION)
                    curMoveScore += mvvOffset * (EvalParams::PieceWeights[QUEEN] - EvalParams::PieceWeights[PAWN]);
                else if (move.flags & MoveFlags::ROOK_PROMOTION)
                    curMoveScore += mvvOffset * -10000;
                else if (move.flags & MoveFlags::BISHOP_PROMOTION)
                    curMoveScore += mvvOffset * -20000;
                else if (move.flags & MoveFlags::KNIGHT_PROMOTION)
                    curMoveScore += mvvOffset * (EvalParams::PieceWeights[KNIGHT] - EvalParams::PieceWeights[PAWN]);
            }


            //assign killer heuristic score
            if (!(move.flags & MoveFlags::CAPTURE)) {
                int killerId = KILLER_MOVES_N;
                for (const Move &killer : killers[curDepth]) {
                    if (killer.same(move)) {
                        curMoveScore += killerOffset * killerId;
                        break;
                    }
                    killerId--;
                }
            }

            //assign history score
            curMoveScore += historyTable[move.from][move.to];

            if (curMoveScore > bestMoveScore) {
                bestMove = j;
                bestMoveScore = curMoveScore;
            }
        }

        std::swap(moves[curDepth][i], moves[curDepth][bestMove]);
        std::swap(captureScore[curDepth][i], captureScore[curDepth][bestMove]);
    }

    nSorted[curDepth] = idx + 1;
}

void MoveGenerator::calculateLatestCaptureScore(const Board &board) {
    if (fast) return;

    auto move = moves[curDepth][size() - 1];
    int score = -EvalParams::PieceWeights[board[move.from].type()];
    for (int direction : explosionDirections) {
        int captureIdx = move.to + direction;
        if (Board::inBounds(captureIdx) && !board.isEmpty(captureIdx)) {
            bool friendly = board[captureIdx].color() == board[move.from].color();
            score += EvalParams::PieceWeights[board[captureIdx].type()] * (friendly ? -1 : 1);
        }
    }
    captureScore[curDepth][size() - 1] = score;
}

void MoveGenerator::sortTT(const Move &move) {
    for (int i = 0; i < n[curDepth]; i++) {
        if (moves[curDepth][i].from == move.from && moves[curDepth][i].to == move.to) {
            std::swap(moves[curDepth][0], moves[curDepth][i]);
            std::swap(captureScore[curDepth][0], captureScore[curDepth][i]);
            nSorted[curDepth] = 1;
            break;
        }
    }
}

void MoveGenerator::markKiller(int idx) {
    auto move = moves[curDepth][idx];
    if (!killers[curDepth][0].same(move)) {
        for (int i = KILLER_MOVES_N - 1; i > 0; i--) {
            killers[curDepth][i] = killers[curDepth][i - 1];
        }
        killers[curDepth][0] = move;
    }
}

bool MoveGenerator::isGoodCapture(int idx) {
    auto move = moves[curDepth][idx];
    return move.flags & MoveFlags::CAPTURE && captureScore[curDepth][idx] >= 0;
}

void MoveGenerator::updateHistory(const Move &move, int depth) {
    historyTable[move.from][move.to] += depth * depth;
    if (historyTable[move.from][move.to] > MAX_HISTORY_TABLE_VAL)
        ageHistory();
}

void MoveGenerator::ageHistory() {
    for (auto &i : historyTable) {
        for (auto &j : i) {
            j >>= 1;
        }
    }
}


