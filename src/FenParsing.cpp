#include <sstream>
#include <tuple>
#include "Board.h"
#include "EvalParms.h"

Board Board::fromFen(const std::string &fen) {
    std::istringstream ss(fen);

    std::string fenPieces;
    std::string activeSide;
    std::string castlingRights;
    std::string enPassantTarget;
    int halfMoves;
    int totalMoves;

    ss >> fenPieces >> activeSide >> castlingRights >> enPassantTarget >> halfMoves >> totalMoves;

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

    return {boardArray, pieceArray, pieceCountArray, move, castling, enPassantSquare, halfMoves, totalMoves};
}

std::string Board::toFen(){
    std::string fen = "";

    //build pieces
    for(int rank = 7; rank >= 0; rank--){
        int empty = 0;
        for(int file = 0; file < 8; file++){
            int index = positionToIndex(file, rank);

            if(isEmpty(index)){
                empty++;
                continue;
            }

            if(empty > 0){
                fen += std::to_string(empty);
                empty = 0;
            }

            char pieceChar = board[index].toChar();
            fen.push_back(pieceChar);
        }
        if(empty > 0){
            fen += std::to_string(empty);
        }
        if(rank >= 1){
            fen += "/";
        }
    }

    fen += " ";

    //active side
    if(moveColor == WHITE){
        fen += "w";
    } else {
        fen += "b";
    }

    fen += " ";

    //castling rights
    std::string rightsString = "";
    if(castlingRights[WHITE] & CastlingRight::KING_SIDE)
        rightsString += "K";
    if(castlingRights[WHITE] & CastlingRight::QUEEN_SIDE)
        rightsString += "Q";
    if(castlingRights[BLACK] & CastlingRight::KING_SIDE)
        rightsString += "k";
    if(castlingRights[BLACK] & CastlingRight::QUEEN_SIDE)
        rightsString += "q";
    if(rightsString == ""){
        rightsString = "-";
    }
    fen += rightsString;

    fen += " ";

    //en passant target
    if(enPassantSquare == -1){
        fen += "-";
    } else {
        fen += indexToString(enPassantSquare);
    }

    fen += " ";

    //half moves
    fen += std::to_string(numHalfMoves);

    fen += " ";

    //full moves
    fen += std::to_string(totalMoves);

    return fen;
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

        int piece;
        if (c == 'p' || c == 'P') piece = PieceType::PAWN;
        if (c == 'n' || c == 'N') piece = PieceType::KNIGHT;
        if (c == 'b' || c == 'B') piece = PieceType::BISHOP;
        if (c == 'r' || c == 'R') piece = PieceType::ROOK;
        if (c == 'q' || c == 'Q') piece = PieceType::QUEEN;
        if (c == 'k' || c == 'K') piece = PieceType::KING;

        if (std::islower(c)) {
            piece |= PieceType::BLACK_FLAG;
        }

        int boardLocation = positionToIndex(curFile, curRank);
        int color = (bool) (piece & PieceType::BLACK_FLAG);
        int pieceType = piece & ~PieceType::BLACK_FLAG;
        int pieceListIndex = pieceCounts[color][pieceType];

        board[boardLocation] = Piece(piece, pieceListIndex);
        pieces[color][pieceType][pieceListIndex] = boardLocation;
        pieceCounts[color][pieceType]++;

        curFile += 1;
    }
    return std::make_tuple(board, pieces, pieceCounts);
}


