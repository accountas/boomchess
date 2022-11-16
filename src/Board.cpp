#include <tuple>
#include <functional>
#include <iostream>
#include <cassert>
#include "Board.h"

Board::Board(const BoardArray &board,
             const PieceArray &pieces,
             const PieceCountArray &piece_counts,
             int move,
             const std::array<int, 2> &castling_rights,
             int en_passant_square,
             int num_half_moves)
    : board(board),
      pieces(pieces),
      pieceCounts(piece_counts),
      moveColor(move),
      castlingRights(castling_rights),
      enPassantSquare(en_passant_square),
      numHalfMoves(num_half_moves) {}

void Board::makeMove(const Move &move) {
    auto moveInfo = MoveInfo(move,
                             0,
                             enPassantSquare,
                             numHalfMoves,
                             castlingRights);

    //update castling rights
    if (board[move.from].type() == KING) {
        castlingRights[moveColor] = NO_CASTLE;
    }
    if (indexToFile(move.from) == 0 && board[move.from].type() == ROOK) {
        castlingRights[moveColor] &= ~CastlingRight::QUEEN_SIDE;
    }
    if (indexToFile(move.from) == 7 && board[move.from].type() == ROOK) {
        castlingRights[moveColor] &= ~CastlingRight::KING_SIDE;
    }

    //for 50 move rule
    if (board[move.from].type() == PAWN || move.flags & MoveFlags::CAPTURE) {
        numHalfMoves = 0;
    } else {
        numHalfMoves += 1;
    }

    //track potential en passant
    if (move.flags & MoveFlags::DOUBLE_PAWN) {
        enPassantSquare = move.to;
    } else {
        enPassantSquare = -1;
    }

    //perform capture
    if (move.flags & MoveFlags::CAPTURE) {
        moveInfo.numCaptured = 2;

        int capturedIdx = !(move.flags & MoveFlags::EN_PASSANT_CAPTURE)
                          ? move.to
                          : move.to + (moveColor == WHITE ? Direction::DOWN : Direction::UP);

        captureHistory.push({move.from, board[move.from]});
        captureHistory.push({capturedIdx, board[capturedIdx]});

        removePiece(capturedIdx, true);
        removePiece(move.from);

        for (int direction : explosionDirections) {
            int idx = move.to + direction;
            if (Board::inBounds(idx) && !isEmpty(idx) && board[idx].type() != PieceType::PAWN) {
                moveInfo.numCaptured++;
                captureHistory.push({idx, board[idx]});
                removePiece(idx, true);
            }
        }
    }

    //promotion
    if (move.flags & MoveFlags::PROMOTION_SUBMASK) {
        if (move.flags & MoveFlags::CAPTURE) {
            removePiece(move.to);
        } else {
            removePiece(move.from);
        }

        int pieceType;
        if (move.flags & MoveFlags::KNIGHT_PROMOTION)
            pieceType = KNIGHT;
        if (move.flags & MoveFlags::BISHOP_PROMOTION)
            pieceType = BISHOP;
        if (move.flags & MoveFlags::QUEEN_PROMOTION)
            pieceType = QUEEN;
        if (move.flags & MoveFlags::ROOK_PROMOTION)
            pieceType = ROOK;
        if (moveColor == BLACK) {
            pieceType |= BLACK_FLAG;
        }
        auto newPiece = Piece(pieceType, -1);
        addPiece(move.to, newPiece);
    }

    //castling
    if (move.flags & MoveFlags::CASTLE_SUBMASK) {
        movePiece(move.from, move.to);
        if (move.flags & MoveFlags::CASTLE_LEFT) {
            movePiece(positionToIndex(0, indexToRank(move.to)), move.to + Direction::RIGHT);
        }
        if (move.flags & MoveFlags::CASTLE_RIGHT) {
            movePiece(positionToIndex(7, indexToRank(move.to)), move.to + Direction::LEFT);
        }
        castlingRights[moveColor] = NO_CASTLE;
    }

    //quiet move
    if ((move.flags & (~MoveFlags::DOUBLE_PAWN)) == 0) {
        movePiece(move.from, move.to);
    }

    //for undo
    moveHistory.push(moveInfo);

    //change player color
    moveColor = !moveColor;
}

void Board::unmakeMove() {
    auto lastMoveUndo = moveHistory.top();
    moveHistory.pop();

    //restore game state
    moveColor = !moveColor;
    castlingRights = lastMoveUndo.previousCastlingRights;
    numHalfMoves = lastMoveUndo.prevNumHalfMoves;
    enPassantSquare = lastMoveUndo.prevEnPassant;

    auto move = lastMoveUndo.move;

    //restore captures
    if (move.flags & MoveFlags::CAPTURE) {
        for (int i = 0; i < lastMoveUndo.numCaptured; i++) {
            auto pieceToRestore = captureHistory.top();
            captureHistory.pop();
            addPiece(pieceToRestore.first, pieceToRestore.second);
        }
    }

    //restore promotion
    if (move.flags & MoveFlags::PROMOTION_SUBMASK) {
        if (move.flags & MoveFlags::CAPTURE) {
            removePiece(move.from);
        } else {
            removePiece(move.to);
        }

        int colorFlag = moveColor == WHITE ? 0 : BLACK_FLAG;
        auto newPiece = Piece(PAWN | colorFlag, -1);
        addPiece(move.from, newPiece);
    }

    //restore castling
    if (move.flags & MoveFlags::CASTLE_SUBMASK) {
        movePiece(move.to, move.from);
        if (move.flags & MoveFlags::CASTLE_LEFT) {
            movePiece(move.to + Direction::RIGHT, positionToIndex(0, indexToRank(move.to)));
        }
        if (move.flags & MoveFlags::CASTLE_RIGHT) {
            movePiece(move.to + Direction::LEFT, positionToIndex(7, indexToRank(move.to)));
        }
    }

    //restore quiet move
    if ((move.flags & (~MoveFlags::DOUBLE_PAWN)) == 0) {
        movePiece(move.to, move.from);
    }

}

void Board::movePiece(int from, int to) {
    auto piece = board[from];
    board[from].piece = PieceType::EMPTY;
    board[to] = piece;
    pieces[piece.color()][piece.type()][piece.pieceListLocation] = to;
}

void Board::addPiece(int idx, Piece piece) {
    int currentCount = pieceCounts[piece.color()][piece.type()];

    piece.pieceListLocation = currentCount;
    pieces[piece.color()][piece.type()][currentCount] = idx;
    pieceCounts[piece.color()][piece.type()]++;

    board[idx] = piece;
}

void Board::removePiece(int idx, bool capture) {
    Piece piece = board[idx];

    //update castling rights if rook
    if(capture && piece.type() == ROOK){
        if(idx == positionToIndex(0, 0)) castlingRights[WHITE] &= ~CastlingRight::QUEEN_SIDE;
        if(idx == positionToIndex(0, 7)) castlingRights[BLACK] &= ~CastlingRight::QUEEN_SIDE;
        if(idx == positionToIndex(7, 0)) castlingRights[WHITE] &= ~CastlingRight::KING_SIDE;
        if(idx == positionToIndex(7, 7)) castlingRights[BLACK] &= ~CastlingRight::KING_SIDE;
    }

    //remove from piece list by replacing with last element
    int lastElementIdx = pieceCounts[piece.color()][piece.type()] - 1;
    if (lastElementIdx > 0) {
        int lastElementValue = pieces[piece.color()][piece.type()][lastElementIdx];
        pieces[piece.color()][piece.type()][piece.pieceListLocation] = lastElementValue;
        board[lastElementValue].pieceListLocation = piece.pieceListLocation;
    }

    pieceCounts[piece.color()][piece.type()]--;
    board[idx].piece = EMPTY;
}

bool Board::isLegal() const {
    bool tookEnemyKing = pieceCounts[moveColor][KING] == 0;
    bool tookOurKing = pieceCounts[!moveColor][KING] == 0;
    bool inCheck = isAttacked(pieces[!moveColor][KING][0], &Board::isFriendly, true);
    return tookEnemyKing || (!tookOurKing && !inCheck);
}
bool Board::isAttacked(int idx) const {
    return isAttacked(idx, &Board::isEnemy);
}
bool Board::isAttacked(int idx, bool (Board::*isEnemyFn)(int) const, bool inverseColor) const {
    //queen and rook attacks
    for (int direction : straightDirections) {
        int hit = castRay(idx, direction);
        if ((this->*isEnemyFn)(hit)) {
            if (board[hit].type() == QUEEN || board[hit].type() == ROOK) return true;
        }
    }

    //queen and bishop attacks
    for (int direction : diagonalDirections) {
        int hit = castRay(idx, direction);
        if ((this->*isEnemyFn)(hit)) {
            if (board[hit].type() == QUEEN || board[hit].type() == BISHOP) return true;
        }
    }

    //knight attacks
    for (int direction : knightDirections) {
        int hit = idx + direction;
        if (inBounds(hit) && (this->*isEnemyFn)(hit) && board[hit].type() == KNIGHT) {
            return true;
        }
    }

    //pawn attacks
    int forward = ((moveColor == WHITE) ^ inverseColor) ? Direction::UP : Direction::DOWN;
    int pawnRight = idx + forward + Direction::RIGHT;
    int pawnLeft = idx + forward + Direction::LEFT;
    if (Board::inBounds(pawnRight) && (this->*isEnemyFn)(pawnRight) && board[pawnRight].type() == PAWN)
        return true;

    if (Board::inBounds(pawnLeft) && (this->*isEnemyFn)(pawnLeft) && board[pawnLeft].type() == PAWN)
        return true;

    return false;
}

int Board::castRay(int startingSquare, int direction) const {
    int currentSquare = startingSquare + direction;
    if (Board::inBounds(currentSquare)) {
        while (inBounds(currentSquare + direction) && isEmpty(currentSquare)) {
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

Board::MoveInfo::MoveInfo(const Move &move,
                          int num_captured,
                          int prev_en_passant,
                          int prev_num_half_moves,
                          const std::array<int, 2> &previous_castling_rights)
    : move(move),
      numCaptured(num_captured),
      prevEnPassant(prev_en_passant),
      prevNumHalfMoves(prev_num_half_moves),
      previousCastlingRights(previous_castling_rights) {}
