//
// Created by marty on 2022-11-05.
//

#include <iostream>
#include <chrono>
#include "Driver.h"
#include "Board.h"
#include "MoveGeneration.h"
using namespace std;

void Driver::start() {

//    auto defaultFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";



    perft(5, "rn2kb1r/1pp1p2p/p2q1pp1/3P4/2P3b1/4PN2/PP3PPP/R2QKB1R b KQkq - 0 1", true);

    system("pause");
}

void Driver::perft(int depth, string fen, bool divide) {
    auto board = Board::fromFen(fen);
    auto generator = MoveGeneration();

    //run and time perft
    auto start = std::chrono::high_resolution_clock::now();
    int total = perft(depth, 0, divide, board, generator);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "===== Perft results for " << fen << " =====" << std::endl;
    std::cout << "perft(" << depth << ") finished in: " << duration / 1000.0 << " s. ";
    std::cout << fixed << "speed: " << ((float)total / duration) * 1000 << " nodes/s" << endl;
    std::cout << "total number of nodes: " << total << endl;
}


int Driver::perft(int maxDepth, int depth, bool divide, Board &board, MoveGeneration &generator) {
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
                cout << Board::indexToString(generator[i].from);
                cout << Board::indexToString(generator[i].to);
                cout << ": " << childCount << endl;
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


