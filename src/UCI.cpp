#include <iostream>
#include <algorithm>
#include <iomanip>

#include "UCI.h"
#include "MoveGenerator.h"
#include "Board.h"
#include "Metrics.h"
#include "Config.h"

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

        std::cout << "Unrecognized position" << std::endl;
        exit(1);
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
    auto nodesToken = std::find(tokens.begin(), tokens.end(), "nodes");

    if (nodesToken == tokens.end()) {
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
        params.depthLimit = MAX_DEPTH - 25;
    } else {
        params.depthLimit = std::stoi(*std::next(depthToken));
    }

    return params;
}

void UCI::sendInfo(int depth, int eval, const Move &best, double time) {
    auto nodesSearched =
        Metric<NODES_SEARCHED>::get() + Metric<Q_NODES_SEARCHED>::get() - Metric<LEAF_NODES_SEARCHED>::get();
    std::cout << "info ";
    std::cout << "depth " << depth << " ";
    std::cout << "score cp " << eval << " ";
    std::cout << "pv " << Board::moveToString(best) << " ";
    std::cout << "nodes " << nodesSearched << " ";
    std::cout << "nps " << (int) (nodesSearched / time) << " ";
    std::cout << "time " << (int) (time * 1000) << " ";
    std::cout << "hashfull " << (1000 * Metric<TT_WRITTEN>::get() / Metric<TT_ENTRIES>::get()) << " ";
    std::cout << std::endl;
}

void UCI::sendResult(const Move &bestMove) {
    std::cout << "bestmove " << Board::moveToString(bestMove) << std::endl;
}

void UCI::sendOptions() {
    // Tell GUI that playing atomic chess
    std::cout << "option name UCI_Variant type combo default atomic var atomic" << std::endl;

    // Hash size
    std::cout << "option name Hash type spin default " << Config::transpositionTableSize << " min 1 max 1024"
              << std::endl;

    // Evaluation type
    std::cout << "option name EvalType type combo default FULL var FULL var SIMPLE" << std::endl;

    // NNUE path
    std::cout << "option name NNUEPath type string default <empty>" << std::endl;
}

void UCI::setOption(
    const std::vector<std::string> &tokens
) {
    if (tokens.size() != 5 || tokens[1] != "name" || tokens[3] != "value") {
        std::cout << "Unrecognized setoption command";
        return;
    }

    const auto &option = tokens[2];
    const auto &value = tokens[4];

    if (option == "Hash") {
        Config::transpositionTableSize = std::stoi(value);
    } else if (option == "NNUEPath") {
        Config::nnuePath = value == "<empty>" ? "" : value;
    } else if (option == "EvalType") {
        if (value == "FULL") Config::hceType = Config::HceType::FULL;
        else if (value == "SIMPLE") Config::hceType = Config::HceType::SIMPLE;
        else { std::cout << "Unrecognized value for EvalType" << std::endl; }
    } else if(option == "UCI_Variant"){
        return;
    } else {
        std::cout << "Unrecognized Option name!" << std::endl;
    }
}
void UCI::uciOk() {
    std::cout << "uciok" << std::endl;
}
void UCI::engineInfo() {
    std::cout << "id name BoomChess" << std::endl;
    std::cout << "id author Martynas Cibulskis" << std::endl;
}
