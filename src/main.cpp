#include <iostream>
#include <sstream>
#include "Driver.h"
#include "Config.h"
#include "Timer.h"

int main(int argc, char** argv) {
    // config options
    int evalDepth = 8;
    std::string inputFile;
    Config::transpositionTableSize = 64;

    // read params
    std::string input;
    getline(std::cin, input);

    // split to words
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(input);
    while (std::getline(token_stream, token, ' ')) {
        tokens.push_back(token);
    }

    // Parse tokens
    for(int i = 0; i < tokens.size(); i += 2){
        auto key = tokens[i];
        auto value = tokens[i + 1];

        if(key == "evalDepth"){
            evalDepth = std::stoi(value);
        }
        if(key == "inputFile"){
            inputFile = value;
        }
        if(key == "hce" && value == "FULL"){
            Config::hceType = Config::HceType::FULL;
        }
        if(key == "hce" && value == "SIMPLE"){
            Config::hceType = Config::HceType::SIMPLE;
        }
    }

    int start = 0;
    int skip = 0;
    int linesRead = 0;
    int fensProcessed = 0;

    // files
    std::ifstream stream(inputFile);
    std::ofstream output("evaluated_" + std::to_string(start) + ".csv");

    // Go through input data
    Timer timer;
    timer.start();
    std::string fen;
    Search search;
    while(std::getline(stream, fen)){
        linesRead += 1;

        if(linesRead - 1 < start || (skip > 0 && (linesRead - start - 1) % skip != 0)){
            continue;
        }
        if(fen.empty()){
            continue;
        }

        //search position
        search.resetCache();
        search.setBoard(Board::fromFen(fen));
        search.startSearch({evalDepth, -1, -1});

        //output results
        output << fen;
        for(int i = 0; i < evalDepth; i++){
            int j = std::min(evalDepth, i);
            output << "," << search.evals[j].first;
            output << "," << Board::moveToString(search.evals[j].second);
            output << "," << search.evals[j].second.flags;
        }
        output << "," << Metric<NODES_SEARCHED>::get() + Metric<Q_NODES_SEARCHED>::get() - Metric<LEAF_NODES_SEARCHED>::get();
        output << "\n";

        fensProcessed += 1;
        if(fensProcessed % 10 == 0){
            std::cout << start << ": Evaluated " << fensProcessed << " fens | " << fensProcessed / timer.getSecondsFromStart() << " fens/s"<< std::endl;
        }
    }

}
