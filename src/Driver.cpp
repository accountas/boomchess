#include <iostream>
#include <iomanip>
#include <algorithm>
#include "Driver.h"
#include "Board.h"
#include "Search.h"
#include "UCI.h"
#include <sstream>
#include <bitset>

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

    std::cout << "id name BoomChess" << std::endl;
    std::cout << "id author Martynas Cibulskis" << std::endl;
    std::cout << "option name UCI_Variant type combo default atomic var atomic" << std::endl;
    std::cout << "uciok" << std::endl;

    Search search;

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
            Board board = UCI::parsePosition(tokens);
            search.setBoard(board);
        } else if (tokens[0] == "go") {
            SearchParams params = UCI::parseGo(tokens);
            search.startSearch(params);
        } else if (tokens[0] == "stop") {
            search.killSearch();
        } else if (tokens[0] == "quit") {
            search.killSearch();
            break;
        } else if (tokens[0] == "setoption") {
            continue;
        } else {
            std::cout << "Unknown Input: " << input << std::endl;
        }
    }
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


