#include <iostream>
#include <iomanip>
#include <algorithm>
#include "Driver.h"
#include "Board.h"
#include "Search.h"
#include "UCI.h"
#include "tools/DatasetGeneration.h"
#include <sstream>
#include <bitset>
#include <mpi.h>

void Driver::start() {
    std::string input;

    std::cout << "Started:" << std::endl;

    while (true) {
        getline(std::cin, input);
        std::cout << input << std::endl;
        auto tokens = tokenizeString(input, ' ');

        if (tokens[0] == "uci") {
            uciMode();
            break;
        }
        else if (tokens[0] == "perft") {
            std::string fen = tokens[1];
            for (int i = 2; i <= 6; i++)
                fen += " " + tokens[i];

            int depth = std::stoi(tokens[7]);
            bool divide = std::count(tokens.begin(), tokens.end(), "div");
            perft(depth, fen, divide);

        }
        else if(tokens[0] == "genpos"){
            DatasetGenerator generator;
            generator.generatePositions(tokens);
        }
        else if(tokens[0] == "evaluatepos"){
            DatasetGenerator generator;
            generator.evaluatePositions(tokens);
        }
        else if(tokens[0] == "mpievaluate"){
            int rank, size;
    
            MPI_Comm_rank(MPI_COMM_WORLD, &rank);
            MPI_Comm_size(MPI_COMM_WORLD, &size);
            
            int start = rank;
            int skip = size;

            tokens.push_back("start");
            tokens.push_back(std::to_string(rank));

            tokens.push_back("skip");
            tokens.push_back(std::to_string(size));

            tokens.push_back("outputfile");
            tokens.push_back("evaluated_" + std::to_string(rank) + ".csv");
            
            std::cout << "Hello from process: "  << rank << " / " << size << std::endl;

            DatasetGenerator generator;
            generator.evaluatePositions(tokens);

            MPI_Finalize();
        }
        else if (tokens[0] == "test") {
            perftTest();
        }
        else if (tokens[0] == "q") {
            break;
        }
        else {
            std::cout << "unknown Token" << std::endl;
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


