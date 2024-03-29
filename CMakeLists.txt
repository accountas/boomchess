cmake_minimum_required(VERSION 3.10)
project(BoomChess)

set(CMAKE_CXX_STANDARD 17)


set(COMMON_SOURCES src/main.cpp src/Driver.cpp src/Driver.h src/Board.cpp src/Board.h src/Piece.cpp src/Piece.h src/Common.h src/Move.h src/MoveGenerator.cpp src/MoveGenerator.h src/FenParsing.cpp src/Search.cpp src/Search.h src/Evaluator.cpp src/Evaluator.h src/Timer.h src/ZobristKey.cpp src/ZobristKey.h src/TranspositionTable.h src/Metrics.h src/Perft.cpp src/UCI.cpp src/UCI.h src/nnue.h src/nnue.cpp src/Config.h src/Config.cpp)

add_executable(BoomChess ${COMMON_SOURCES})

MATH(EXPR stack_size "32 * 1024 * 1024") # 32 Mb
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--stack,${stack_size}")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -pthread ") #-fsanitize=address,signed-integer-overflow
else ()
    MATH(EXPR stack_size "32 * 1024 * 1024") # 32 Mb
    set(CMAKE_EXE_LINKER_FLAGS "-Wl,--stack,${stack_size}")
    set(CMAKE_CXX_FLAGS "-pthread -static-libstdc++ -static-libgcc -static -O3 -mavx2 -flto")
endif ()

