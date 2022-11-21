//
// Created by marty on 2022-11-21.
//

#ifndef BOOMCHESS_SRC_TRANSPOSITIONTABLE_H_
#define BOOMCHESS_SRC_TRANSPOSITIONTABLE_H_

#include <cstdint>
#include <iostream>
#include "Move.h"

template<typename T, int N>
class TranspositionTable {
 public:
    TranspositionTable() {
        entries = new T[N];
    }
    ~TranspositionTable() {
        delete[] entries;
    }
    T &at(uint64_t key) {
        return entries[key & (N - 1)];
    }
    T &operator[](uint64_t key) {
        return entries[key & (N - 1)];
    }
    void store(uint64_t key, T value) {
        entries[key & (N - 1)] = value;
    }
 private:
    T *entries;
};

struct SearchEntry {
    uint64_t zobristKey = 0;
    int depth = 0;
    int value = 0;
    int bound = 0;
    Move bestMove;
};

#endif //BOOMCHESS_SRC_TRANSPOSITIONTABLE_H_
