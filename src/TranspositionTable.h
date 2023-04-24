#pragma once

#include <cstdint>
#include <iostream>
#include "Move.h"
#include "Config.h"
#include "Metrics.h"
#include "Common.h"

template<typename T>
class TranspositionTable {
 public:
    int N;
    TranspositionTable() {
        N = getNumEntriesNeeded();
        Metric<TT_ENTRIES>::set(N);
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
    void resize() {
        delete[] entries;
        N = getNumEntriesNeeded();
        entries = new T[N];
        Metric<TT_ENTRIES>::set(N);
    }
 private:
    int getNumEntriesNeeded(){
        uint64_t numEntries = Config::transpositionTableSize * 1000000LL / sizeof(T);
        return (1 << (64 - __builtin_clzll(numEntries))); //make it into the smallest power of two for fast mod
    }
    T *entries;
};

struct SearchEntry {
    uint64_t zobristKey = 0;
    int depth = 0;
    int value = 0;
    int bound = 0;
    Move bestMove;
};