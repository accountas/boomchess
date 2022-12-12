//
// Created by marty on 2022-11-05.
//

#include <iostream>
#include <chrono>
#include <iomanip>
#include <cassert>
#include "Driver.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "Search.h"
#include "Timer.h"

void Driver::start() {
    using namespace std;

    Timer timer;
    string fen;

    while (true) {
        getline(std::cin, fen);
        if (fen == "q") break;
        Search search;
        search.setBoard(Board::fromFen(fen));

        timer.start();
        search.search(10);
        timer.end();

//        perft(5, fen, false);
//        perftTT(5, fen, false);
    }
}
void Driver::perft(int depth, const std::string &fen, bool divide = false) {
    auto board = Board::fromFen(fen);
    auto generator = MoveGenerator();

    //run and time perft
    auto start = std::chrono::high_resolution_clock::now();
    int total = perft(depth, 0, divide, board, generator);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "===== Perft results for " << fen << " =====" << std::endl;
    std::cout << "perft(" << depth << ") finished in: " << duration / 1000.0 << " s. ";
    std::cout << std::fixed << "speed: " << ((float) total / duration) * 1000 << " nodes/s" << std::endl;
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
            if (divide && depth == 0) {
                std::cout << Board::indexToString(generator[i].from);
                std::cout << Board::indexToString(generator[i].to);
                std::cout << ": " << childCount << std::endl;
            }
            nodes += childCount;
        }
        board.unmakeMove();
    }

    generator.decreaseDepth();
    return nodes;
}

void Driver::perftTT(int depth, const std::string &fen, bool divide = false) {
    auto board = Board::fromFen(fen);
    auto generator = MoveGenerator();
    int hits = 0;
    TranspositionTable<PerftEntry, TT_SIZE> tTable;

    //run and time perft
    auto start = std::chrono::high_resolution_clock::now();
    int total = perftTT(depth, 0, divide, board, generator, tTable, hits);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "===== PerftTT results for " << fen << " =====" << std::endl;
    std::cout << "perft(" << depth << ") finished in: " << duration / 1000.0 << " s. ";
    std::cout << "cache hits: " << hits << std::endl;
    std::cout << std::fixed << "speed: " << ((float) total / duration) * 1000 << " nodes/s" << std::endl;
    std::cout << "total number of nodes: " << total << std::endl;
}

int Driver::perftTT(int maxDepth,
                    int depth,
                    bool divide,
                    Board &board,
                    MoveGenerator &generator,
                    TranspositionTable<PerftEntry, TT_SIZE> &tTable,
                    int &cacheHits) {
    if (depth == maxDepth) {
        return 1;
    }

    auto hash = board.zobristKey.value;
    int tValue = -1;
    if (tTable[hash].hash == hash && tTable[hash].depth == depth) {
        cacheHits += tTable[hash].count;
        return tTable[hash].count;
    }

    int nodes = 0;
    generator.increaseDepth();
    generator.generateMoves(board);

    for (int i = 0; i < generator.size(); i++) {
        auto before = board.zobristKey.value;
        board.makeMove(generator[i]);

        if (board.isLegal()) {
            int childCount = perftTT(maxDepth, depth + 1, divide, board, generator, tTable, cacheHits);
            if (divide && depth == 0) {
                std::cout << Board::indexToString(generator[i].from);
                std::cout << Board::indexToString(generator[i].to);
                std::cout << ": " << childCount << std::endl;
            }
            nodes += childCount;
        }
        board.unmakeMove();
        auto after = board.zobristKey.value;
        if (before != after) {
            std::cout << Board::moveToString(generator[i]) << std::endl;
            std::cout << "??\n";
        }
    }

    generator.decreaseDepth();

    PerftEntry entry{
        hash, depth, nodes
    };
    tTable.store(hash, entry);

    return nodes;
}


