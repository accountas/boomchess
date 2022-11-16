//
// Created by marty on 2022-11-09.
//

#include "MoveGeneration.h"

void MoveGeneration::generateMoves(const Board &board) {
    n[curDepth] = 0;

    //no legal moves if king ded
    if(board.pieceCounts[board.moveColor][KING] == 0){
        return;
    }

    generatePawnMoves(board);
    generateQueenMoves(board);
    generateBishopMoves(board);
    generateRookMoves(board);
    generateKnightMoves(board);
    generateKingMoves(board);
}

void MoveGeneration::generatePawnMoves(const Board &board) {
    int forward = board.moveColor == Color::WHITE ? Direction::UP : Direction::DOWN;
    int startingRank = board.moveColor == Color::WHITE ? 1 : 6;
    int promotionRank = board.moveColor == Color::WHITE ? 6 : 1;

    for (int i = 0; i < board.pieceCounts[board.moveColor][PieceType::PAWN]; i++) {
        int square = board.pieces[board.moveColor][PieceType::PAWN][i];

        //forward or promote
        bool forwardClear = board.isEmpty(square + forward);
        if (forwardClear && Board::indexToRank(square) == promotionRank) {
            addMove(square, square + forward, MoveFlags::BISHOP_PROMOTION);
            addMove(square, square + forward, MoveFlags::KNIGHT_PROMOTION);
            addMove(square, square + forward, MoveFlags::ROOK_PROMOTION);
            addMove(square, square + forward, MoveFlags::QUEEN_PROMOTION);
        } else if (forwardClear) {
            addMove(square, square + forward);
        }

        //first double move
        if (Board::indexToRank(square) == startingRank
            && forwardClear
            && board.isEmpty(square + forward * 2)) {
            addMove(square, square + forward * 2, MoveFlags::DOUBLE_PAWN);
        }

        //attack moves
        int attackRight = square + forward + Direction::RIGHT;
        if (Board::inBounds(attackRight)
            && !board.isEmpty(attackRight)
            && board.isEnemy(attackRight)) {

            if (Board::indexToRank(square) == promotionRank) {
                addMove(square, attackRight, MoveFlags::BISHOP_PROMOTION | MoveFlags::CAPTURE);
                addMove(square, attackRight, MoveFlags::KNIGHT_PROMOTION | MoveFlags::CAPTURE);
                addMove(square, attackRight, MoveFlags::ROOK_PROMOTION | MoveFlags::CAPTURE);
                addMove(square, attackRight, MoveFlags::QUEEN_PROMOTION | MoveFlags::CAPTURE);
            } else {
                addMove(square, attackRight, MoveFlags::CAPTURE);
            }

        }

        int attackLeft = square + forward + Direction::LEFT;
        if (Board::inBounds(attackLeft)
            && !board.isEmpty(attackLeft)
            && board.isEnemy(attackLeft)) {

            if (Board::indexToRank(square) == promotionRank) {
                addMove(square, attackLeft, MoveFlags::BISHOP_PROMOTION | MoveFlags::CAPTURE);
                addMove(square, attackLeft, MoveFlags::KNIGHT_PROMOTION | MoveFlags::CAPTURE);
                addMove(square, attackLeft, MoveFlags::ROOK_PROMOTION | MoveFlags::CAPTURE);
                addMove(square, attackLeft, MoveFlags::QUEEN_PROMOTION | MoveFlags::CAPTURE);
            } else {
                addMove(square, attackLeft, MoveFlags::CAPTURE);
            }
        }

        //en passant
        if (board.enPassantSquare != -1
            && square + Direction::LEFT == board.enPassantSquare
            && board.isEnemy(square + Direction::LEFT)
            && board.isEmpty(square + Direction::LEFT + forward)) {
            addMove(square, square + Direction::LEFT + forward, MoveFlags::CAPTURE | MoveFlags::EN_PASSANT_CAPTURE);
        }
        if (board.enPassantSquare != -1
            && square + Direction::RIGHT == board.enPassantSquare
            && board.isEnemy(square + Direction::RIGHT)
            && board.isEmpty(square + Direction::RIGHT + forward)) {
            addMove(square, square + Direction::RIGHT + forward, MoveFlags::CAPTURE | MoveFlags::EN_PASSANT_CAPTURE);
        }

    }
}
void MoveGeneration::generateKingMoves(const Board &board) {
    int square = board.pieces[board.moveColor][PieceType::KING][0];

    //move king (Kings can't capture)
    for (int direction : allDirections) {
        int newSquare = square + direction;
        if (Board::inBounds(newSquare) && board.isEmpty(newSquare)) {
            addMove(square, newSquare);
        }
    }

    //castling king side
    if (board.castlingRights[board.moveColor] & CastlingRight::KING_SIDE) {
        generateCastle(board, square, Direction::RIGHT);
    }

    //castling queen side
    if (board.castlingRights[board.moveColor] & CastlingRight::QUEEN_SIDE) {
        generateCastle(board, square, Direction::LEFT);
    }
}

void MoveGeneration::generateCastle(const Board &board, int kingSquare, int castleDirection) {
    bool canCastle = true;

    canCastle &= !board.isAttacked(kingSquare);

    canCastle &= board.isEmpty(kingSquare + castleDirection);
    canCastle &= !board.isAttacked(kingSquare + castleDirection);

    canCastle &= board.isEmpty(kingSquare + castleDirection * 2);
    canCastle &= !board.isAttacked(kingSquare + castleDirection * 2);

    if(castleDirection == Direction::LEFT)
        canCastle &= board.isEmpty(kingSquare + castleDirection * 3);

    if (canCastle) {
        int flag = castleDirection == Direction::RIGHT ? MoveFlags::CASTLE_RIGHT : MoveFlags::CASTLE_LEFT;
        addMove(kingSquare, kingSquare + castleDirection * 2, flag);
    }
}

void MoveGeneration::generateKnightMoves(const Board &board) {
    for (int i = 0; i < board.pieceCounts[board.moveColor][PieceType::KNIGHT]; i++) {
        int square = board.pieces[board.moveColor][PieceType::KNIGHT][i];

        for (int direction : knightDirections) {
            int newSquare = square + direction;
            if (Board::inBounds(newSquare)) {
                if (board.isEmpty(newSquare)) {
                    addMove(square, newSquare);
                } else if (board.isEnemy(newSquare)) {
                    addMove(square, newSquare, MoveFlags::CAPTURE);
                }
            }
        }
    }
}

void MoveGeneration::generateBishopMoves(const Board &board) {
    for (int i = 0; i < board.pieceCounts[board.moveColor][PieceType::BISHOP]; i++) {
        int square = board.pieces[board.moveColor][PieceType::BISHOP][i];

        for (int direction : diagonalDirections) {
            generateSlidingMoves(board, square, direction);
        }
    }
}

void MoveGeneration::generateRookMoves(const Board &board) {
    for (int i = 0; i < board.pieceCounts[board.moveColor][PieceType::ROOK]; i++) {
        int square = board.pieces[board.moveColor][PieceType::ROOK][i];

        for (int direction : straightDirections) {
            generateSlidingMoves(board, square, direction);
        }
    }
}

void MoveGeneration::generateQueenMoves(const Board &board) {
    for (int i = 0; i < board.pieceCounts[board.moveColor][PieceType::QUEEN]; i++) {
        int square = board.pieces[board.moveColor][PieceType::QUEEN][i];

        for (int direction : allDirections) {
            generateSlidingMoves(board, square, direction);
        }
    }
}

void MoveGeneration::generateSlidingMoves(const Board &board, int startingSquare, int direction) {
    int currentSquare = startingSquare;
    while (true) {
        currentSquare += direction;
        if (!Board::inBounds(currentSquare)) {
            break;
        } else if (board.isEmpty(currentSquare)) {
            addMove(startingSquare, currentSquare);
        } else if (board.isEnemy(currentSquare)) {
            addMove(startingSquare, currentSquare, MoveFlags::CAPTURE);
            break;
        } else {
            break;
        }
    }
}

