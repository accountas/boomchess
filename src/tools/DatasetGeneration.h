#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <random>
#include "../Board.h"
#include "../Common.h"
#include "../Search.h"


class DatasetGenerator {

    struct DataGenParams {
        int minNodes = 5000;
        int maxNodes = 15000;
        int maxPly = 150;
        int evalDepth = 5;
        int count = 1000;
        float saveRate = 0.01;
        std::string outputFileName = "dataset.csv";
    };

    public:
        void generate(const std::vector<std::string> &tokens){
            auto params = parseParams(tokens);
            outputFile.open(params.outputFileName);
            while(positionsGenerated < params.count){
                exploreNewGame(params);
            }
        }

    private:
        int positionsGenerated = 0;
        std::mt19937 randomGenerator{1};
        std::ofstream outputFile;

        void exploreNewGame(const DataGenParams &params){
            auto board = Board::fromFen(DEFAULT_FEN);
            auto search = Search();
            auto distribution = std::uniform_int_distribution<>(params.minNodes, params.maxNodes);

            for(int move = 0; move < params.maxPly; move++){
                int nodesToExplore = distribution(randomGenerator);
                auto explorationParams = SearchParams{-1, -1, nodesToExplore};


                search.setBoard(board);
                search.resetCache();
                auto [moveToExplore, eval]  = search.findBestMove(explorationParams);

                if(moveToExplore.flags & MoveFlags::NULL_MOVE){
                    break; //end of the game
                }

                board.makeMove(moveToExplore);

            }
        }

        void addToDataset(const Board &board, const DataGenParams &params){
            auto distribution = std::uniform_real_distribution<>(0, 1);
            if(distribution(randomGenerator) >= params.saveRate){
                return;
            }

            auto boardCopy = board;
            auto search = Search();
            auto searchParams = SearchParams{params.evalDepth, -1, -1};

            search.setBoard(boardCopy);
            auto [bestMove, eval]  = search.findBestMove(searchParams);

            outputFile << board.toFen() << ",";
            outputFile << eval << ",";
            outputFile << Board::moveToString(bestMove) << ",";
            outputFile << Metric<NODES_SEARCHED>::get() << Metric<Q_NODES_SEARCHED>::get() << Metric<LEAF_NODES_SEARCHED>::get();
            outputFile << std::endl;
        }

        DataGenParams parseParams(const std::vector<std::string> &tokens){
            auto params = DataGenParams();

            for(int i = 1; i < tokens.size(); i++){
                if(tokens[i] == "minNodes"){
                    params.minNodes = std::stoi(tokens[i + 1]);
                }
                else if (tokens[i] == "maxNodes"){
                    params.maxNodes = std::stoi(tokens[i + 1]);
                }
                else if (tokens[i] == "maxPly"){
                    params.maxPly = std::stoi(tokens[i + 1]);
                }
                else if (tokens[i] == "evalDepth"){
                    params.evalDepth = std::stoi(tokens[i + 1]);
                }
                else if (tokens[i] == "count"){
                    params.count = std::stoi(tokens[i + 1]);
                }
                else if (tokens[i] == "file"){
                    params.outputFileName = tokens[i + 1];
                }
                else if (tokens[i] == "saveRate"){
                    params.saveRate = std::stof(tokens[i + 1]);
                } 
                else {
                    std::cout << "Unrecognised token: " << tokens[i] << std::endl;
                    exit(1);
                }
            }

            return params;
        }

}