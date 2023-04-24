#include <tuple>
#include <functional>
#include "Board.h"
#include "nnue.h"

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
      numHalfMoves(num_half_moves) {

    //set initial hash
    for (int color : std::array<int, 2>{WHITE, BLACK}) {
        for (int piece : PieceTypes) {
            for (int i = 0; i < pieceCounts[color][piece]; i++) {
                int idx = pieces[color][piece][i];
                zobristKey.flipPiece(idx, board[idx]);
            }
        }
    }
    zobristKey.setMoveColor(move);
    zobristKey.flipCastlingRights(WHITE, castling_rights[WHITE]);
    zobristKey.flipCastlingRights(BLACK, castling_rights[BLACK]);
    setEnPassantSquare(en_passant_square);

    moveHistory.reserve(256);
    captureHistory.reserve(64);

    precalculateAttackTable();
}
void Board::precalculateAttackTable() {
    for (int distance = 1; distance <= 7; distance++) {
        for (int direction : straightDirections) {
            attackDirection[QUEEN][0x77 + direction * distance] = direction;
            attackDirection[ROOK][0x77 + direction * distance] = direction;
        }
        for (int direction : diagonalDirections) {
            attackDirection[QUEEN][0x77 + direction * distance] = direction;
            attackDirection[BISHOP][0x77 + direction * distance] = direction;
        }
    }
    for (int direction : knightDirections) {
        attackDirection[KNIGHT][0x77 + direction] = direction;
    }
    for (int direction : allDirections) {
        attackDirection[KING][0x77 + direction] = direction;
    }
}

void Board::makeMove(const Move &move) {
    auto moveInfo = MoveInfo(move,
                             0,
                             enPassantSquare,
                             numHalfMoves,
                             castlingRights,
                             0);

    //update castling rights
    if (board[move.from].type() == KING) {
        setCastlingRights(moveColor, NO_CASTLE);
    }
    if (indexToFile(move.from) == 0 && board[move.from].type() == ROOK) {
        int newRights = castlingRights[moveColor] & ~CastlingRight::QUEEN_SIDE;
        setCastlingRights(moveColor, newRights);
    }
    if (indexToFile(move.from) == 7 && board[move.from].type() == ROOK) {
        int newRights = castlingRights[moveColor] & ~CastlingRight::KING_SIDE;
        setCastlingRights(moveColor, newRights);
    }

    //for 50 move rule
    if (board[move.from].type() == PAWN || move.flags & MoveFlags::CAPTURE) {
        numHalfMoves = 0;
    } else {
        numHalfMoves += 1;
    }

    //track potential en passant
    if (move.flags & MoveFlags::DOUBLE_PAWN) {
        setEnPassantSquare(move.to);
    } else {
        setEnPassantSquare(-1);
    }

    //perform capture
    if (move.flags & MoveFlags::CAPTURE) {
        moveInfo.numCaptured = 2;

        int capturedIdx = !(move.flags & MoveFlags::EN_PASSANT_CAPTURE)
                          ? move.to
                          : move.to + (moveColor == WHITE ? Direction::DOWN : Direction::UP);

        captureHistory.emplace_back(move.from, board[move.from]);
        captureHistory.emplace_back(capturedIdx, board[capturedIdx]);

        removePiece(capturedIdx, true, false);
        removePiece(move.from, false, false);

        for (int direction : explosionDirections) {
            int idx = move.to + direction;
            if (Board::inBounds(idx) && !isEmpty(idx) && board[idx].type() != PieceType::PAWN) {
                moveInfo.numCaptured++;
                captureHistory.emplace_back(idx, board[idx]);
                removePiece(idx, true, false);
            }
        }
    }

    //promotion
    if (move.flags & MoveFlags::PROMOTION_SUBMASK) {
        if (move.flags & MoveFlags::CAPTURE) {
            removePiece(move.to, false, false);
        } else {
            removePiece(move.from, false, false);
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
        addPiece(move.to, newPiece, false);
    }

    //castling
    if (move.flags & MoveFlags::CASTLE_SUBMASK) {
        movePiece(move.from, move.to, false);
        if (move.flags & MoveFlags::CASTLE_LEFT) {
            movePiece(positionToIndex(0, indexToRank(move.to)), move.to + Direction::RIGHT, false);
        }
        if (move.flags & MoveFlags::CASTLE_RIGHT) {
            movePiece(positionToIndex(7, indexToRank(move.to)), move.to + Direction::LEFT, false);
        }
        setCastlingRights(moveColor, NO_CASTLE);
    }

    //quiet move
    if ((move.flags & ~(MoveFlags::DOUBLE_PAWN | MoveFlags::PAWN_MOVE)) == 0 && !(move.flags & MoveFlags::NULL_MOVE)) {
        movePiece(move.from, move.to, false);
    }

    //for undo
    moveInfo.zobristKey = zobristKey.value;
    moveHistory.push_back(moveInfo);

    //change player color
    flipMoveColor();

    if(nnue){
        nnue->accumulator.increaseDepth();
        nnue->accumulator.applyStagedChanges();
    }

}

void Board::unmakeMove() {
    auto lastMoveUndo = moveHistory.back();
    moveHistory.pop_back();

    //restore game state
    flipMoveColor();
    setCastlingRights(WHITE, lastMoveUndo.previousCastlingRights[WHITE]);
    setCastlingRights(BLACK, lastMoveUndo.previousCastlingRights[BLACK]);
    numHalfMoves = lastMoveUndo.prevNumHalfMoves;
    setEnPassantSquare(lastMoveUndo.prevEnPassant);

    auto move = lastMoveUndo.move;

    //restore captures
    if (move.flags & MoveFlags::CAPTURE) {
        for (int i = 0; i < lastMoveUndo.numCaptured; i++) {
            auto pieceToRestore = captureHistory.back();
            captureHistory.pop_back();
            addPiece(pieceToRestore.first, pieceToRestore.second, true);
        }
    }

    //restore promotion
    if (move.flags & MoveFlags::PROMOTION_SUBMASK) {
        if (move.flags & MoveFlags::CAPTURE) {
            removePiece(move.from, false, true);
        } else {
            removePiece(move.to, false, true);
        }

        int colorFlag = moveColor == WHITE ? 0 : BLACK_FLAG;
        auto newPiece = Piece(PAWN | colorFlag, -1);
        addPiece(move.from, newPiece, true);
    }

    //restore castling
    if (move.flags & MoveFlags::CASTLE_SUBMASK) {
        movePiece(move.to, move.from, true);
        if (move.flags & MoveFlags::CASTLE_LEFT) {
            movePiece(move.to + Direction::RIGHT, positionToIndex(0, indexToRank(move.to)), true);
        }
        if (move.flags & MoveFlags::CASTLE_RIGHT) {
            movePiece(move.to + Direction::LEFT, positionToIndex(7, indexToRank(move.to)), true);
        }
    }

    //restore quiet move
    if ((move.flags & ~(MoveFlags::DOUBLE_PAWN | MoveFlags::PAWN_MOVE)) == 0 && !(move.flags & MoveFlags::NULL_MOVE)) {
        movePiece(move.to, move.from, true);
    }

    if(nnue){
        nnue->accumulator.decreaseDepth();
    }

}

void Board::movePiece(int from, int to, bool unmake) {
    auto piece = board[from];
    zobristKey.flipPiece(from, piece);

    board[from] = Piece();
    board[to] = piece;
    pieces[piece.color()][piece.type()][piece.pieceListLocation] = to;

    zobristKey.flipPiece(to, piece);

    if(!unmake && nnue){
        nnue->accumulator.stageChange<false>(to, piece.type(), piece.color());
        nnue->accumulator.stageChange<true>(from, piece.type(), piece.color());
    }
}

void Board::addPiece(int idx, Piece piece, bool unmake) {
    int currentCount = pieceCounts[piece.color()][piece.type()];

    piece.pieceListLocation = currentCount;
    pieces[piece.color()][piece.type()][currentCount] = idx;
    pieceCounts[piece.color()][piece.type()]++;

    board[idx] = piece;
    zobristKey.flipPiece(idx, piece);

    if(!unmake && nnue){
        nnue->accumulator.stageChange<false>(idx, piece.type(), piece.color());
    }
}

void Board::removePiece(int idx, bool capture, bool unmake) {
    Piece piece = board[idx];
    zobristKey.flipPiece(idx, piece);

    //update castling rights if rook
    if (capture && piece.type() == ROOK) {
        if (idx == positionToIndex(0, 0))
            setCastlingRights(WHITE, castlingRights[WHITE] & ~CastlingRight::QUEEN_SIDE);
        if (idx == positionToIndex(0, 7))
            setCastlingRights(BLACK, castlingRights[BLACK] & ~CastlingRight::QUEEN_SIDE);
        if (idx == positionToIndex(7, 0))
            setCastlingRights(WHITE, castlingRights[WHITE] & ~CastlingRight::KING_SIDE);
        if (idx == positionToIndex(7, 7))
            setCastlingRights(BLACK, castlingRights[BLACK] & ~CastlingRight::KING_SIDE);
    }

    //remove from piece list by replacing with last element
    int lastElementIdx = pieceCounts[piece.color()][piece.type()] - 1;
    if (lastElementIdx > 0) {
        int lastElementValue = pieces[piece.color()][piece.type()][lastElementIdx];
        pieces[piece.color()][piece.type()][piece.pieceListLocation] = lastElementValue;
        board[lastElementValue].pieceListLocation = piece.pieceListLocation;
    }

    pieceCounts[piece.color()][piece.type()]--;
    board[idx] = Piece();

    if(!unmake && nnue){
        nnue->accumulator.stageChange<true>(idx, piece.type(), piece.color());
    }
}

void Board::setEnPassantSquare(int square) {
    if (enPassantSquare != -1)
        zobristKey.flipEnPassantFile(indexToFile(enPassantSquare));
    if (square != -1)
        zobristKey.flipEnPassantFile(indexToFile(square));

    enPassantSquare = square;
}
void Board::setCastlingRights(int color, int rights) {
    zobristKey.flipCastlingRights(color, castlingRights[color]);
    zobristKey.flipCastlingRights(color, rights);

    castlingRights[color] = rights;
}
void Board::flipMoveColor() {
    zobristKey.flipMoveColor();
    moveColor = !moveColor;
}

bool Board::isLegal() const {
    bool tookEnemyKing = pieceCounts[moveColor][KING] == 0;
    bool tookOurKing = pieceCounts[!moveColor][KING] == 0;
    bool inCheck = isAttacked(pieces[!moveColor][KING][0], true) && !kingsTouch();
    return (!tookOurKing) && (tookEnemyKing || !inCheck);
}

bool Board::isAttacked(int idx) const {
    return isAttacked(idx, false);
}

bool Board::isAttacked(int idx, bool inverseColor) const {
    int enemyColor = moveColor ^ inverseColor ^ 1;

    //pawn attacks
    for (int direction : {Direction::RIGHT, Direction::LEFT}) {
        int pawnPos = idx + direction + ((enemyColor == BLACK) ? Direction::UP : Direction::DOWN);
        if (Board::inBounds(pawnPos)
            && board[pawnPos].type() == PAWN
            && board[pawnPos].color() == enemyColor)
            return true;
    }

    //other pieces
    for (int piece : PieceTypes) {
        if (piece == PAWN || piece == KING)
            continue;

        for (int i = 0; i < pieceCounts[enemyColor][piece]; i++) {
            int enemyPos = pieces[enemyColor][piece][i];
            int enemyAttackDir = attackDirection[piece][0x77 + idx - enemyPos];

            if (enemyAttackDir == 0) {
                continue;
            }

            if (piece == KNIGHT || castRay(enemyPos, enemyAttackDir, idx) == idx) {
                return true;
            }
        }
    }

    return false;
}

int Board::castRay(int startingSquare, int direction, int destination) const {
    int currentSquare = startingSquare + direction;
    if (Board::inBounds(currentSquare)) {
        while (inBounds(currentSquare + direction) && isEmpty(currentSquare) && currentSquare != destination) {
            currentSquare += direction;
        }
        return currentSquare;
    } else {
        return currentSquare - direction;
    }
}

[[maybe_unused]] std::string Board::toString() const {
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
bool Board::isRepetition() const {
    uint64_t lastHash = moveHistory.back().zobristKey;
    int seen = 0;
    for (auto it = moveHistory.rbegin(); it != moveHistory.rend(); it = next(it)) {
        auto move = *it;
        if (move.move.flags & MoveFlags::NULL_MOVE) {
            continue;
        }
        if (move.move.flags & MoveFlags::NON_REPEATABLE_MASK) {
            return false;
        }
        if (lastHash == move.zobristKey) {
            if (++seen == 3) return true;
        }
    }
    return false;
}
std::string Board::moveToString(const Move &move) {
    auto result = indexToString(move.from) + indexToString(move.to);
    if (move.flags & MoveFlags::QUEEN_PROMOTION) result += 'q';
    if (move.flags & MoveFlags::BISHOP_PROMOTION) result += 'b';
    if (move.flags & MoveFlags::ROOK_PROMOTION) result += 'r';
    if (move.flags & MoveFlags::KNIGHT_PROMOTION) result += 'n';

    //for uci compatability, engine does not treat last rank pawn captures as promotions
    if (move.flags & MoveFlags::CAPTURE
        && move.flags & MoveFlags::PAWN_MOVE
        && (indexToRank(move.to) == 7 || indexToRank(move.to) == 0)) {
        result += 'q';
    }

    return result;
}
bool Board::tryMakeMove(const Move &move) {
    makeMove(move);
    if(!isLegal()){
        unmakeMove();
        return false;
    }
    return true;
}
bool Board::kingsTouch() const {
    int whiteKing = pieces[WHITE][KING][0];
    int blackKing = pieces[BLACK][KING][0];
    return attackDirection[KING][0x77 + whiteKing - blackKing] != 0;
}
bool Board::isKingCaptured() const {
    return pieceCounts[moveColor][KING] == 0;
}
bool Board::onlyPawns() const {
    for(int piece : {KNIGHT, BISHOP, ROOK, QUEEN}){
        if(pieceCounts[moveColor][piece] > 0) return false;
    }
    return true;
}
bool Board::isInCheck() const {
    return !isKingCaptured() && !kingsTouch() && isAttacked(pieces[moveColor][KING][0]);
}
bool Board::isEnemy(int idx) const {
    return !isEmpty(idx) && board[idx].color() != moveColor;
}
bool Board::isEmpty(int idx) const {
    return board[idx].piece == PieceType::EMPTY;
}
bool Board::inBounds(int idx) {
    return !(idx & 0x88);
}
int Board::positionToIndex(int file, int rank) {
    return (rank << 4) + file;
}
int Board::indexToFile(int index) {
    return index & 0b111;
}
int Board::indexToRank(int index) {
    return (index & 0b1110000) >> 4;
}
int Board::stringToIndex(const std::string &square) {
    return positionToIndex(square[0] - 'a', square[1] - '0' - 1);
}
Move Board::stringToMove(const std::string &move, int flags) {
    int from = stringToIndex(move.substr(0, 2));
    int to = stringToIndex(move.substr(2));
    return {from, to, flags};
}
std::string Board::indexToString(int idx) {
    char file = 'a' + indexToFile(idx);
    char rank = '1' + indexToRank(idx);
    return {file, rank};
}

Board::MoveInfo::MoveInfo(const Move &move,
                          int num_captured,
                          int prev_en_passant,
                          int prev_num_half_moves,
                          const std::array<int, 2> &previous_castling_rights,
                          uint64_t zobristKey)
    : move(move),
      numCaptured(num_captured),
      prevEnPassant(prev_en_passant),
      prevNumHalfMoves(prev_num_half_moves),
      previousCastlingRights(previous_castling_rights),
      zobristKey(zobristKey) {}
