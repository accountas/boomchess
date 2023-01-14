//
// Created by marty on 2022-11-05.
//

#include <string>
#include <unordered_map>
#include "Piece.h"
#include "EvalParms.h"

char Piece::toChar() const {
    std::unordered_map<int, std::string> pieceString = {
        {PieceType::EMPTY, "."},
        {PieceType::PAWN, "P"},
        {PieceType::ROOK, "R"},
        {PieceType::BISHOP, "B"},
        {PieceType::KNIGHT, "N"},
        {PieceType::QUEEN, "Q"},
        {PieceType::KING, "K"},
        {PieceType::PAWN | PieceType::BLACK_FLAG, "p"},
        {PieceType::ROOK | PieceType::BLACK_FLAG, "r"},
        {PieceType::BISHOP | PieceType::BLACK_FLAG, "b"},
        {PieceType::KNIGHT | PieceType::BLACK_FLAG, "n"},
        {PieceType::QUEEN | PieceType::BLACK_FLAG, "q"},
        {PieceType::KING | PieceType::BLACK_FLAG, "k"},
    };

    return pieceString[piece][0];
}

Piece::Piece(int piece,
             int pieceListLocation)
    : piece(piece),
      pieceListLocation(pieceListLocation) {
    value = 0;
    int type = piece & (~BLACK_FLAG);
    if (type != PAWN) {
        value = EvalParams::PieceWeights[type];
    }
}
