#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <stack>
#include <memory>
#include "Piece.h"
#include "Common.h"
#include "Move.h"
#include "ZobristKey.h"
#include "nnue.h"

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

    // Piece representation
    BoardArray board;
    PieceArray pieces;
    PieceCountArray pieceCounts;

    // Game state
    int moveColor;
    int enPassantSquare;
    int numHalfMoves;
    bool madeNullMove = false;
    std::array<int, 2> castlingRights;

    // hash
    ZobristKey zobristKey{};

    // NNUE
    NNUE::NNUE *nnue = nullptr;

    // fen parsing stuff
    static Board fromFen(const std::string &fen);
    [[nodiscard]] std::string toFen() const;

    // move making
    void makeMove(const Move &move);
    bool tryMakeMove(const Move &move);
    void unmakeMove();

    // state checks
    [[nodiscard]] bool isAttacked(int idx) const;
    [[nodiscard]] bool isLegal() const;
    [[nodiscard]] bool isRepetition() const;
    [[nodiscard]] bool kingsTouch() const;
    [[nodiscard]] bool isKingCaptured() const;
    [[nodiscard]] bool onlyPawns() const;
    [[nodiscard]] bool isInCheck() const;

    // Square checks
    [[nodiscard]] bool isEnemy(int idx) const;
    [[nodiscard]] bool isEmpty(int idx) const;
    static bool inBounds(int idx);

    // conversions
    [[nodiscard]] std::string toString() const;
    static Move stringToMove(const std::string &move, int flags);
    static std::string moveToString(const Move &move);
    static int positionToIndex(int file, int rank);
    static int indexToFile(int index);
    static int indexToRank(int index);
    static std::string indexToString(int idx);
    static int stringToIndex(const std::string &square);

    // Operators
    Piece const &operator[](int index) {
        return board[index];
    }
    Piece operator[](int index) const {
        return board[index];
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

    // used in move making
    void addPiece(int idx, Piece piece, bool unmake);
    void removePiece(int idx, bool capture, bool unmake);
    void movePiece(int from, int to, bool unmake);

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