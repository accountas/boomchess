cmake_minimum_required(VERSION 3.23)
project(BoomChess)

set(CMAKE_CXX_STANDARD 20)

add_executable(BoomChess src/main.cpp src/Driver.cpp src/Driver.h src/Board.cpp src/Board.h src/Piece.cpp src/Piece.h src/Common.h src/Move.h src/MoveGeneration.cpp src/MoveGeneration.h src/FenParsing.cpp src/Game.cpp src/Game.h)
