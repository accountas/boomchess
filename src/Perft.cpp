#include <string>
#include <chrono>
#include <cassert>
#include "Driver.h"

void Driver::perftTest() {
    assert(perft(5, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", false) == 4864979);
    assert(perft(5, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", false) == 619830);
    assert(perft(4, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", false) == 3492097);
    assert(perft(4, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", false) == 1776042);
    assert(perft(4, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", false) == 3766789);
}
int Driver::perft(int depth, const std::string &fen, bool divide = false) {
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

    return total;
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
                std::cout << Board::moveToString(generator[i]) << ": " << childCount << std::endl;
            }
            nodes += childCount;

            //just to be consistent with stock-fish perft
            if (generator[i].flags & MoveFlags::PAWN_MOVE
                && generator[i].flags & MoveFlags::CAPTURE
                && (
                    Board::indexToRank(generator[i].to) == 0
                        || Board::indexToRank(generator[i].to) == 7)
                ) {
                nodes += childCount * 3;
            }
        }
        board.unmakeMove();
    }

    generator.decreaseDepth();
    return nodes;
}