#pragma once
#include <string>

namespace Config {
    // Path to nnue network (if given disables HCE)
    extern std::string nnuePath;

    // Transposition table size in megabytes
    extern int transpositionTableSize;

    // Hand-crafted evaluation function to use
    enum class HceType {
        FULL,
        SIMPLE
    };
    extern HceType hceType;
}