#include <tuple>
#include "Board.h"

Board::Board(const BoardArray &board,
             const PieceArray &pieces,
             const PieceCountArray &piece_counts,
             Color move,
             const std::array<int, 2> &castling_rights,
             int en_passant_square,
             int num_half_moves)
    : board(board),
      pieces(pieces),
      pieceCounts(piece_counts),
      move(move),
      castlingRights(castling_rights),
      enPassantSquare(en_passant_square),
      numHalfMoves(num_half_moves) {}

bool Board::isAttacked(int idx) const {
    //queen and rook attacks
    for (int direction : straightDirections) {
        int hit = castRay(idx, direction);
        if (isEnemy(hit)) {
            if (board[hit].type() == QUEEN || board[hit].type() == ROOK) return true;
        }
    }

    //queen and bishop attacks
    for (int direction : diagonalDirections) {
        int hit = castRay(idx, direction);
        if (isEnemy(hit)) {
            if (board[hit].type() == QUEEN || board[hit].type() == BISHOP) return true;
        }
    }

    //knight attacks
    for (int direction : knightDirections) {
        int hit = idx + direction;
        if (inBounds(hit) && isEnemy(hit) && board[hit].type() == KNIGHT) {
            return true;
        }
    }

    //pawn attacks
    int forward = move == WHITE ? Direction::UP : Direction::DOWN;
    int pawnRight = idx + forward + Direction::RIGHT;
    int pawnLeft = idx + forward + Direction::LEFT;
    if (Board::inBounds(pawnRight) && isEnemy(pawnRight) &&  board[pawnRight].type() == PAWN)
        return true;

    if (Board::inBounds(pawnLeft) && isEnemy(pawnRight) &&  board[pawnRight].type() == PAWN)
        return true;

    return false;
}

int Board::castRay(int startingSquare, int direction) const {
    int currentSquare = startingSquare + direction;
    if(Board::inBounds(currentSquare)){
        while (inBounds(currentSquare + direction) && isEmpty(currentSquare)){
            currentSquare += direction;
        }
        return currentSquare;
    } else {
        return currentSquare - direction;
    }
}

std::string Board::toString() const {
    std::string result;

    for (int i = 7; i >= 0; i--) {
        result += "(";
        result += (char) ('0' + i + 1);
        result += ")  ";

        for (int j = 0; j < 8; j++) {
            result += board[positionToIndex(j, i)].toChar();
            result += " ";
        }
        result += "\n";
    }
    result += "\n    (A B C D E F G H)\n";

    return result;
}
