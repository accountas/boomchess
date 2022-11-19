//
// Created by marty on 2022-11-05.
//

#include <iostream>
#include <chrono>
#include "Driver.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "Search.h"
#include "Timer.h"


void Driver::start() {
    using namespace std;

    std::string fen;
    Timer timer;

    while(true){
        getline(std::cin, fen);
        if(fen == "q") break;
        Search search;
        search.setBoard(Board::fromFen(fen));

        timer.start();
        auto result = search.search(8);
        timer.end();

        cout << "nodes checked: " << search.numChecked << "; nps = " << fixed << search.numChecked / timer.getSeconds() << endl;
        cout << result.evaluation << " " << Board::moveToString(result.bestMove) << endl;
    }
}
void Driver::perft(int depth, std::string fen, bool divide) {
    auto board = Board::fromFen(fen);
    auto generator = MoveGenerator();

    //run and time perft
    auto start = std::chrono::high_resolution_clock::now();
    int total = perft(depth, 0, divide, board, generator);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "===== Perft results for " << fen << " =====" << std::endl;
    std::cout << "perft(" << depth << ") finished in: " << duration / 1000.0 << " s. ";
    std::cout << std::fixed << "speed: " << ((float)total / duration) * 1000 << " nodes/s" << std::endl;
    std::cout << "total number of nodes: " << total << std::endl;
}


int Driver::perft(int maxDepth, int depth, bool divide, Board &board, MoveGenerator &generator) {
    if (depth == maxDepth) {
       return 1;
    }

    int nodes = 0;
    generator.increaseDepth();
    generator.generateMoves(board);

    for (int i = 0; i < generator.size(); i++) {
        board.makeMove(generator[i]);

        if (board.isLegal()) {
            int childCount = perft(maxDepth, depth + 1, divide, board, generator);
            if(divide && depth == 0){
                std::cout << Board::indexToString(generator[i].from);
                std::cout << Board::indexToString(generator[i].to);
                std::cout << ": " << childCount << std::endl;
            }
//            if(divide && depth == 1){
//                cout << " " << Board::indexToString(generator[i].from);
//                cout << Board::indexToString(generator[i].to);
//                cout << ": " << childCount << endl;
//            }
            nodes += childCount;
        }
        board.unmakeMove();
    }

    generator.decreaseDepth();
    return nodes;
}


