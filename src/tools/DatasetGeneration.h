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

class DatasetGenerator {

    struct DataGenParams {
        int minNodes = 2000;
        int maxNodes = 15000;
        int maxPly = 150;
        int evalDepth = 8;
        int count = 30;
        int totalRandomPly = 4;
        float saveRate = 0.04;
        std::string outputFileName = "dataset.csv";
    };

 public:
    void generate(const std::vector<std::string> &tokens) {
        UCI::setQuiet(true);
        timer.start();

        auto params = parseParams(tokens);
        outputFile.open(params.outputFileName);
        while (positionsGenerated < params.count) {
            exploreNewGame(params);
        }

        UCI::setQuiet(false);
    }

 private:
    int positionsGenerated = 0;
    std::mt19937 randomGenerator{1};
    std::ofstream outputFile;
    std::unordered_set<uint64_t> positionSet;
    Timer timer;


    void exploreNewGame(const DataGenParams &params) {
        auto board = Board::fromFen(DEFAULT_FEN);
        auto search = Search();
        auto generator = MoveGenerator();
        auto distribution = std::uniform_int_distribution<>(params.minNodes, params.maxNodes);
        Move moveToExplore;

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
                int moveToMake = std::uniform_int_distribution<>(0, (int)legalMoves.size())(randomGenerator);
                moveToExplore = generator[moveToMake];
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

        auto search = Search();
        auto searchParams = SearchParams{params.evalDepth, -1, -1};

        search.setBoard(board);
        auto [bestMove, eval] = search.findBestMove(searchParams);

        positionsGenerated += 1;

        outputFile << board.toFen() << ",";
        outputFile << eval << ",";
        outputFile << Board::moveToString(bestMove) << ",";
        outputFile << Metric<NODES_SEARCHED>::get() + Metric<Q_NODES_SEARCHED>::get() - Metric<LEAF_NODES_SEARCHED>::get() << ",";
        outputFile << std::endl;

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
            } else if (tokens[i] == "file") {
                params.outputFileName = tokens[i + 1];
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