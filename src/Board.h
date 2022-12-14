#ifndef BOOMCHESS_BOARD_H
#define BOOMCHESS_BOARD_H

#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <stack>
#include "Piece.h"
#include "Common.h"
#include "Move.h"
#include "ZobristKey.h"

typedef std::array<Piece, 128> BoardArray;
typedef std::array<std::array<std::array<int, 10>, 7>, 2> PieceArray;
typedef std::array<std::array<int, 7>, 2> PieceCountArray;

class Board {
 public:
    Board(const BoardArray &board,
          const PieceArray &pieces,
          const PieceCountArray &piece_counts,
          int move,
          const std::array<int, 2> &castling_rights,
          int en_passant_square,
          int num_half_moves);

    //Piece representation
    BoardArray board;
    PieceArray pieces;
    PieceCountArray pieceCounts;

    //Game state
    int moveColor;
    int enPassantSquare;
    int numHalfMoves;
    bool madeNullMove = false;
    std::array<int, 2> castlingRights;

    //hash
    ZobristKey zobristKey{};

    static Board fromFen(const std::string &fen);

    [[maybe_unused]] std::string toString() const;

    void makeMove(const Move &move);
    void unmakeMove();
    void addPiece(int idx, Piece piece);
    void removePiece(int idx, bool capture = false);
    void movePiece(int from, int to);

    bool isAttacked(int idx) const;
    bool isLegal() const;
    bool isRepetition() const;
    bool kingsTouch() const {
        int whiteKing = pieces[WHITE][KING][0];
        int blackKing = pieces[BLACK][KING][0];
        return attackDirection[KING][0x77 + whiteKing - blackKing] != 0;
    }
    bool isKingCaptured() const {
        return pieceCounts[moveColor][KING] == 0;
    };
    bool isInCheck() const {
        return !isKingCaptured() && !kingsTouch() && isAttacked(pieces[moveColor][KING][0]);
    }
    bool isEnemy(int idx) const {
        return !isEmpty(idx) && board[idx].color() != moveColor;
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
    static int stringToIndex(const std::string &square) {
        return positionToIndex(square[0] - 'a', square[1] - '0' - 1);
    }
    [[maybe_unused]] static Move stringToMove(const std::string &move, int flags) {
        int from = stringToIndex(move.substr(0, 2));
        int to = stringToIndex(move.substr(2));
        return {from, to, flags};
    }
    static std::string moveToString(const Move &move);
    static std::string indexToString(int idx) {
        char file = 'a' + indexToFile(idx);
        char rank = '1' + indexToRank(idx);
        return {file, rank};
    }
    Piece const &operator[](int index) {
        return board[index];
    }
    Piece operator[](int index) const {
        return board[index];
    }
    [[maybe_unused]] std::string getMovesMade() {
        std::string result;
        for (const auto &move : moveHistory) {
            result += moveToString(move.move);
            result += " ";
        }
        return result;
    }

 private:
    struct MoveInfo {
        MoveInfo(const Move &move,
                 int num_captured,
                 int prev_en_passant,
                 int prev_num_half_moves,
                 const std::array<int, 2> &previous_castling_rights,
                 uint64_t zobristKey);
        Move move;
        int numCaptured{};
        int prevEnPassant{};
        int prevNumHalfMoves{};
        uint64_t zobristKey{};
        std::array<int, 2> previousCastlingRights{};
    };

    //fen parsing
    static std::tuple<BoardArray, PieceArray, PieceCountArray> extractPiecesFromFen(const std::string &fen);

    //incremental update
    std::vector<MoveInfo> moveHistory;
    std::vector<std::pair<int, Piece>> captureHistory;

    //move gen util stuff
    int castRay(int startingSquare, int direction, int destination) const;
    void setEnPassantSquare(int square);
    void setCastlingRights(int color, int rights);
    void flipMoveColor();
    bool isAttacked(int idx, bool inverseColor) const;

    //attacker check utils
    std::array<std::array<int, 256>, 7> attackDirection{};

    void precalculateAttackTable();
};

#endif //BOOMCHESS_BOARD_H
