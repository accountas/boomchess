#ifndef BOOMCHESS_BOARD_H
#define BOOMCHESS_BOARD_H

#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include "Piece.h"
#include "Common.h"

typedef std::array<Piece, 128> BoardArray;
typedef std::array<std::array<std::array<int, 10>, 7>, 2> PieceArray;
typedef std::array<std::array<int, 7>, 2> PieceCountArray;

class Board {
 public:
    //Piece representation
    BoardArray board;
    PieceArray pieces;
    PieceCountArray pieceCounts;

    //Game state
    Color move;
    std::array<int, 2> castlingRights;
    int enPassantSquare;
    int numHalfMoves; //for 50 move rule

    Board(const BoardArray &board,
          const PieceArray &pieces,
          const PieceCountArray &piece_counts,
          Color move,
          const std::array<int, 2> &castling_rights,
          int en_passant_square,
          int num_half_moves);

    static Board fromFen(const std::string &fen);

    bool isAttacked(int idx) const;
    int castRay(int startingSquare, int direction) const;
    std::string toString() const;

    bool isCheck() const {
        return isAttacked(pieces[move][KING][0]);
    }
    bool isEnemy(int idx) const {
        return !isEmpty(idx) && (board[idx].piece & PieceType::BLACK_FLAG) != move;
    }
    bool isEmpty(int idx) const {
        return board[idx].piece == PieceType::EMPTY;
    }
    static bool inBounds(int idx) {
        return !(idx & 0x88);
    }
    static int positionToIndex(int file, int rank) {
        return (rank << 4) + file;
    }
    static int indexToFile(int index) {
        return index & 0b111;
    }
    static int indexToRank(int index) {
        return (index & 0b1110000) >> 4;
    }
    Piece &operator[](int index) {
        return board[index];
    }

 private:
    static std::tuple<BoardArray, PieceArray, PieceCountArray> extractPiecesFromFen(const std::string &fen);
};

#endif //BOOMCHESS_BOARD_H
