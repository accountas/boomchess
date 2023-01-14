//
// Created by marty on 2022-12-13.
//


#include <iostream>
#include <algorithm>
#include <iomanip>

#include "UCI.h"
#include "MoveGenerator.h"
#include "Board.h"
#include "Metrics.h"

Board UCI::parsePosition(const std::vector<std::string> &tokens) {
    auto getBoard = [&] {
        if (tokens[1] == "startpos") {
            return Board::fromFen(DEFAULT_FEN);
        }
        if (tokens[1] == "fen") {
            std::string fen = tokens[2];
            for (int i = 3; i <= 7; i++)
                fen += " " + tokens[i];
            return Board::fromFen(fen);
        }
    };

    Board board = getBoard();

    auto movesToken = std::find(tokens.begin(), tokens.end(), "moves");
    if (movesToken != tokens.end()) {
        MoveGenerator moveGenerator;
        for (auto move = std::next(movesToken); move != tokens.end(); move++) {
            //check the move with the generator, we need it to assign move flags
            //hacky, but we also win moves pseudo-legality check
            moveGenerator.generateMoves(board);
            bool foundLegal = false;
            for (int j = 0; j < moveGenerator.size() && !foundLegal; j++) {
                if (Board::moveToString(moveGenerator[j]) == *move) {
                    board.makeMove(moveGenerator[j]);
                    foundLegal = true;
                }
            }
            if (!foundLegal) {
                std::cout << "Requested an illegal move: " << *move << std::endl;
            }
        }
    }

    return board;
}

SearchParams UCI::parseGo(const std::vector<std::string> &tokens) {
    SearchParams params{};

    auto depthToken = std::find(tokens.begin(), tokens.end(), "depth");
    auto moveTimeToken = std::find(tokens.begin(), tokens.end(), "movetime");
Tune     auto nodesToken = std::find(tokens.begin(), tokens.end(), "nodes");

    if(nodesToken == tokens.end()){
        params.nodeLimit = 0;
    } else {
        params.nodeLimit = std::stoi(*std::next(nodesToken));
    }
    if (moveTimeToken == tokens.end()) {
        params.timeLimit = 0;
    } else {
        params.timeLimit = std::stoi(*std::next(moveTimeToken));
    }

    if (depthToken == tokens.end()) {
        params.depthLimit = MAX_DEPTH;
    } else {
        params.depthLimit = std::stoi(*std::next(depthToken));
    }

    return params;
}

void UCI::sendInfo(int depth, int eval, const Move &best, double time) {
#ifdef SEND_UCI_INFO
    auto nodesSearched =
        Metric<NODES_SEARCHED>::get() + Metric<Q_NODES_SEARCHED>::get() - Metric<LEAF_NODES_SEARCHED>::get();
    std::cout << "info ";
    std::cout << "depth " << depth << " ";
    std::cout << "score cp " << eval << " ";
    std::cout << "pv " << Board::moveToString(best) << " ";
    std::cout << "nodes " << nodesSearched << " ";
    std::cout << "nps " << (int) (nodesSearched / time) << " ";
    std::cout << "time " << (int) (time * 1000) << " ";
    std::cout << "hashfull " << (1000 * Metric<TT_ENTRIES>::get() / TT_SIZE) << " ";
    std::cout << "qwidth " << Metric<TT_ENTRIES>::get() << " ";
    std::cout << std::endl;
#endif
}
void UCI::sendResult(const Move &bestMove) {
#ifdef SEND_UCI_INFO
    std::cout << "bestmove " << Board::moveToString(bestMove) << std::endl;
#endif
}
