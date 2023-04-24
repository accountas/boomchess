#include <iostream>
#include <iomanip>
#include <algorithm>
#include "Driver.h"
#include "Board.h"
#include "Search.h"
#include "UCI.h"
#include "Config.h"

void Driver::start() {
    std::string input;

    while (true) {
        getline(std::cin, input);
        auto tokens = tokenizeString(input, ' ');

        if (tokens[0] == "uci") {
            uciMode();
            break;
        }
        if (tokens[0] == "perft") {
            std::string fen = tokens[1];
            for (int i = 2; i <= 6; i++)
                fen += " " + tokens[i];

            int depth = std::stoi(tokens[7]);
            bool divide = std::count(tokens.begin(), tokens.end(), "div");
            perft(depth, fen, divide);

        }
        if (tokens[0] == "test") {
            perftTest();
        }
        if (tokens[0] == "q") {
            break;
        }
    }
}

void Driver::uciMode() {
    UCI::engineInfo();
    UCI::sendOptions();
    UCI::uciOk();

    Search search;
    Board board = Board::fromFen(DEFAULT_FEN);
    NNUE::NNUE nnue;

    std::string input;
    while (true) {
        getline(std::cin, input);
        auto tokens = tokenizeString(input, ' ');

        if (tokens.empty()) continue;

        if (tokens[0] == "isready") {
            std::cout << "readyok" << std::endl;
        } else if (tokens[0] == "ucinewgame") {
            search.resetCache();
        } else if (tokens[0] == "position") {
            board = UCI::parsePosition(tokens);
        } else if (tokens[0] == "go") {
            SearchParams params = UCI::parseGo(tokens);
            if(!Config::nnuePath.empty()){
                board.nnue = &nnue;
                initNnueFromBoard(board, nnue);
            }
            search.setBoard(board);
            search.startSearch(params);
        } else if (tokens[0] == "stop") {
            search.killSearch();
        } else if (tokens[0] == "quit") {
            search.killSearch();
            break;
        } else if (tokens[0] == "setoption") {
            UCI::setOption(tokens);
            // TODO: Ugly ifs
            if (tokens[2] == "NNUEPath"){
                nnue.loadNetwork(Config::nnuePath);
            }
            if (tokens[2] == "Hash"){
                search.tTable.resize();
            }
        } else {
            std::cout << "Unknown Input: " << input << std::endl;
        }
    }
}

void Driver::initNnueFromBoard(Board &board, NNUE::NNUE &nnue) {
    nnue.accumulator.setDepth(0);
    nnue.accumulator.init();
    for (int color : {WHITE, BLACK}) {
        for (int piece : PieceTypes) {
            for (int i = 0; i < board.pieceCounts[color][piece]; i++) {
                int pos = board.pieces[color][piece][i];
                nnue.accumulator.stageChange<false>(pos, piece, color);
            }
        }
    }
    nnue.accumulator.applyStagedChanges(false);
}

std::vector<std::string> Driver::tokenizeString(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(s);
    while (std::getline(token_stream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


