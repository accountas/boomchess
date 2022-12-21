cmake_minimum_required(VERSION 3.10)
project(BoomChess)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -static-libstdc++ -static-libgcc -static")
add_executable(BC_paper_w src/main.cpp src/Driver.cpp src/Driver.h src/Board.cpp src/Board.h src/Piece.cpp src/Piece.h src/Common.h src/Move.h src/MoveGenerator.cpp src/MoveGenerator.h src/FenParsing.cpp src/Search.cpp src/Search.h src/Evaluator.cpp src/Evaluator.h src/Timer.h src/ZobristKey.cpp src/ZobristKey.h src/TranspositionTable.cpp src/TranspositionTable.h src/Metrics.h src/Perft.cpp src/UCI.cpp src/UCI.h)

