cmake_minimum_required(VERSION 3.10)
project(BoomChess)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")


add_executable(BoomChess src/main.cpp src/Driver.cpp src/Driver.h src/Board.cpp src/Board.h src/Piece.cpp src/Piece.h src/Common.h src/Move.h src/MoveGeneration.cpp src/MoveGeneration.h src/FenParsing.cpp)
