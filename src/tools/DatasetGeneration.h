#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <random>
#include <unordered_set>
#include "../Board.h"
#include "../Common.h"
#include "../Search.h"
#include "../Timer.h"
#include "../UCI.h"

class DatasetGenerator {

    struct DataGenParams {
        int minNodes = 2000;
        int maxNodes = 15000;
        int maxPly = 150;
        int evalDepth = 8;
        int count = 30;
        int totalRandomPly = 6;
        int skip = 0;
        int start = 0;
        float saveRate = 0.20;
        std::string outputFileName;
        std::string inputFileName;
    };

 public:
    void generatePositions(const std::vector<std::string> &tokens) {
        UCI::setQuiet(true);
        timer.start();

        auto params = parseParams(tokens);
        outputFile.open(params.outputFileName);
        while (positionsGenerated < params.count) {
            exploreNewGame(params);
        }

        UCI::setQuiet(false);
    }

    void evaluatePositions(const std::vector<std::string> &tokens){
        UCI::setQuiet(true);
        timer.start();

        auto params = parseParams(tokens);
        positionsFile.open(params.inputFileName);
        outputFile.open(params.outputFileName);

        std::string fen;

        int linesRead = 0;
        int fensProcessed = 0;
        Search search;

        while (std::getline(positionsFile, fen)){
            linesRead++;

            if(linesRead - 1 < params.start || (params.skip > 0 && (linesRead - params.start - 1) % params.skip != 0)){
                continue;
            }

            if(fen.empty()){
                continue;
            }

            search.resetCache();
            search.setBoard(Board::fromFen(fen));
            auto evals = search.getEvalPerDepth(SearchParams{params.evalDepth, -1, -1});

            outputFile << fen;
            for(int i = 0; i < params.evalDepth; i++){
                outputFile << "," << evals[i];
            }
            outputFile << "," << Metric<NODES_SEARCHED>::get() + Metric<Q_NODES_SEARCHED>::get() - Metric<LEAF_NODES_SEARCHED>::get();
            outputFile << std::endl;;

            fensProcessed += 1;

            if(fensProcessed % 10 == 0){
                std::cout << params.start << ": Evaluated " << fensProcessed << " fens | " << fensProcessed / timer.getSecondsFromStart() << " fens/s" << std::endl;
            }
        }
        UCI::setQuiet(false);
    }


 private:
    int positionsGenerated = 0;
    std::mt19937 randomGenerator{1};
    std::ofstream outputFile;
    std::ifstream positionsFile;
    std::unordered_set<uint64_t> positionSet;
    Timer timer;


    void exploreNewGame(const DataGenParams &params) {
        auto board = Board::fromFen(DEFAULT_FEN);
        auto search = Search();
        auto generator = MoveGenerator();
        auto distribution = std::uniform_int_distribution<>(params.minNodes, params.maxNodes);
        Move moveToExplore(0, 0, MoveFlags::NULL_MOVE);

        for (int move = 0; move < params.maxPly; move++) {
            if(move < params.totalRandomPly){
                generator.generateMoves(board);
                std::vector<Move> legalMoves;
                for(int i = 0; i < generator.size(); i++){
                    board.makeMove(generator[i]);
                    if(board.isLegal()){
                        legalMoves.push_back(generator[i]);
                    }
                    board.unmakeMove();
                }
                if(legalMoves.empty()){
                    break;
                }
                int moveToMake = std::uniform_int_distribution<>(0, (int)legalMoves.size() - 1)(randomGenerator);
                moveToExplore = legalMoves[moveToMake];
            } else {
                int nodesToExplore = distribution(randomGenerator);
                auto explorationParams = SearchParams{20, -1, nodesToExplore};

                search.setBoard(board);
                search.resetCache();
                moveToExplore = search.findBestMove(explorationParams).first;
            }

            if (moveToExplore.flags & MoveFlags::NULL_MOVE) {
                break; //end of the game
            }

            board.makeMove(moveToExplore);
            addToDataset(board, params);
        }
    }

    void addToDataset(const Board &board, const DataGenParams &params) {
        auto distribution = std::uniform_real_distribution<>(0, 1);
        if (distribution(randomGenerator) >= params.saveRate) {
            return;
        }
        if(positionSet.count(board.zobristKey.value)){
            return;
        }

        positionSet.insert(board.zobristKey.value);
        positionsGenerated += 1;

        outputFile << board.toFen() << std::endl;

        if(positionsGenerated % 5 == 0){
            std::cout << "Generated: " << positionsGenerated << " | " << (positionsGenerated / timer.getSecondsFromStart()) << " fen / s" << std::endl;
        }
    }

    DataGenParams parseParams(const std::vector<std::string> &tokens) {
        auto params = DataGenParams();

        for (int i = 1; i < tokens.size(); i += 2) {
            if (tokens[i] == "minNodes") {
                params.minNodes = std::stoi(tokens[i + 1]);
            } else if (tokens[i] == "maxNodes") {
                params.maxNodes = std::stoi(tokens[i + 1]);
            } else if (tokens[i] == "maxPly") {
                params.maxPly = std::stoi(tokens[i + 1]);
            } else if (tokens[i] == "evalDepth") {
                params.evalDepth = std::stoi(tokens[i + 1]);
            } else if (tokens[i] == "count") {
                params.count = std::stoi(tokens[i + 1]);
            } else if (tokens[i] == "outputfile") {
                params.outputFileName = tokens[i + 1];
            } else if(tokens[i] == "inputfile") {
                params.inputFileName = tokens[i + 1];
            } else if(tokens[i] == "skip"){
                params.skip = std::stoi(tokens[i + 1]);
            } else if(tokens[i] == "start"){
                params.start = std::stoi(tokens[i + 1]);
            } else if (tokens[i] == "saveRate") {
                params.saveRate = std::stof(tokens[i + 1]);
            } else if (tokens[i] == "seed") {
                randomGenerator.seed(std::stoi(tokens[i + 1]));
            } else {
                std::cout << "Unrecognised token: " << tokens[i] << std::endl;
                exit(1);
            }
        }
        return params;
    }

};