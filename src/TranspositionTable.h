#pragma once

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
    void clear() {
        for (int i = 0; i < N; i++) {
            entries[i] = T{};
        }
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