#include <iostream>
#include "Driver.h"

void toFenTests();

int main() {

    Driver driver = Driver();
    driver.start();
}

void toFenTests(){

    std::vector<std::string> fens = {
        "4q2B/p3Pbp1/pr3N2/8/4P3/7P/n1Q5/1B1K2k1 w - - 0 1",
        "2b1r2r/1p2Nk2/2Bp4/8/2p5/P1P2N2/3QK2p/6R1 w - - 0 1",
        "8/R5P1/7N/P3b1rp/1K2k1p1/2p3p1/1BP1P1P1/8 w - - 0 1",
        "3b3k/P3P3/n2Q4/6p1/N2K1B2/P4r2/6Rp/2R1r3 w - - 0 1",
        "k7/2B2p2/q4N2/1P2NKp1/1P3QP1/8/4P1pb/7n w - - 0 1",
        "8/5kqr/2P5/P3b1n1/3QpPp1/n3K1pp/5B2/8 w - - 0 1",
        "q7/3r2Pk/7p/p1K1B2p/PN3p2/2P2P2/4P3/5r2 w - - 0 1",
        "8/p2b1P1N/3P3p/1R4pq/3N1b1n/7P/K4p2/7k w - - 0 1",
        "4q2B/p3Pb2/pr3Np1/8/1n2P2P/2Q5/8/1B1K2k1 w - - 2 3"
    };

    for(auto fen : fens){
        auto board = Board::fromFen(fen);
        auto genFen = board.toFen();
        assert(fen == genFen);
    }
}   
