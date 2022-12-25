//
// Created by marty on 2022-11-09.
//

#include <sstream>
#include <tuple>
#include "Board.h"

Board Board::fromFen(const std::string &fen) {
    std::istringstream ss(fen);

    std::string fenPieces;
    std::string activeSide;
    std::string castlingRights;
    std::string enPassantTarget;
    int32_t halfMoves;

    ss >> fenPieces >> activeSide >> castlingRights >> enPassantTarget >> halfMoves;

    Color move = activeSide == "b" ? Color::BLACK : Color::WHITE;

    int enPassantSquare = -1;
    if (enPassantTarget != "-") {
        int file = enPassantTarget[0] - 'a';
        int rank = enPassantTarget[1] - '0';
        enPassantSquare = positionToIndex(file, rank);
    }

    std::array<int, 2> castling{NO_CASTLE, NO_CASTLE};
    if (castlingRights.find('k') != std::string::npos)
        castling[Color::BLACK] |= CastlingRight::KING_SIDE;
    if (castlingRights.find('q') != std::string::npos)
        castling[Color::BLACK] |= CastlingRight::QUEEN_SIDE;
    if (castlingRights.find('K') != std::string::npos)
        castling[Color::WHITE] |= CastlingRight::KING_SIDE;
    if (castlingRights.find('Q') != std::string::npos)
        castling[Color::WHITE] |= CastlingRight::QUEEN_SIDE;

    auto temp = extractPiecesFromFen(fenPieces);
    BoardArray boardArray = std::get<0>(temp);
    PieceArray pieceArray = std::get<1>(temp);
    PieceCountArray pieceCountArray = std::get<2>(temp);

    return {boardArray, pieceArray, pieceCountArray, move, castling, enPassantSquare, halfMoves};
}

std::tuple<BoardArray, PieceArray, PieceCountArray> Board::extractPiecesFromFen(const std::string &fen) {
    BoardArray board = {};
    PieceArray pieces = {};
    PieceCountArray pieceCounts = {};

    int curRank = 7;
    int curFile = 0;

    for (const char &c : fen) {
        if (c == '/') {
            curRank--;
            curFile = 0;
            continue;
        }

        if (std::isdigit(c)) {
            curFile += (c - '0');
            continue;
        }

        Piece piece;
        if (c == 'p' || c == 'P') piece.piece = PieceType::PAWN;
        if (c == 'n' || c == 'N') piece.piece = PieceType::KNIGHT;
        if (c == 'b' || c == 'B') piece.piece = PieceType::BISHOP;
        if (c == 'r' || c == 'R') piece.piece = PieceType::ROOK;
        if (c == 'q' || c == 'Q') piece.piece = PieceType::QUEEN;
        if (c == 'k' || c == 'K') piece.piece = PieceType::KING;

        if (std::islower(c)) {
            piece.piece |= PieceType::BLACK_FLAG;
        }

        int boardLocation = positionToIndex(curFile, curRank);
        int color = (bool) (piece.piece & PieceType::BLACK_FLAG);
        int pieceType = piece.piece & ~PieceType::BLACK_FLAG;
        int pieceListIndex = pieceCounts[color][pieceType];

        piece.pieceListLocation = pieceListIndex;
        board[boardLocation] = piece;
        pieces[color][pieceType][pieceListIndex] = boardLocation;
        pieceCounts[color][pieceType]++;

        curFile += 1;
    }
    return std::make_tuple(board, pieces, pieceCounts);
}


