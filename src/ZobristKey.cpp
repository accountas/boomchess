//
// Created by marty on 2022-11-19.
//

#include <random>
#include <iostream>
#include "ZobristKey.h"
#include "Timer.h"

void ZobristKey::generateNumbers() {
    std::random_device r;
    std::default_random_engine e(r());
    std::uniform_int_distribution<uint64_t> uniform_dist(0, UINT64_MAX);

    for (int i = 0; i < pieceNumbers.size(); i++) {
        for (int j = 0; j < pieceNumbers[i].size(); j++) {
            for (int k = 0; k < pieceNumbers[i][j].size(); k++) {
                pieceNumbers[i][j][k] = uniform_dist(e);
            }
        }
    }

    for (auto &i : moveColorNumbers) {
        i = uniform_dist(e);
    }

    for (auto &i : enPassantFileNumbers) {
        i = uniform_dist(e);
    }

    for (int i = 0; i < castlingRightNumbers.size(); i++) {
        for (int j = 0; j < castlingRightNumbers[i].size(); j++) {
            castlingRightNumbers[i][j] = uniform_dist(e);
        }
    }
}
