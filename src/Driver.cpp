//
// Created by marty on 2022-11-05.
//

#include <iostream>
#include "Driver.h"
#include "Board.h"
#include "MoveGeneration.h"

void Driver::start() {
    using namespace std;

    auto board = Board::fromFen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1");
    cout << board.toString() << endl;
    auto generator = MoveGeneration();
    generator.generateMoves(board);

    for(int i = 0; i < generator.n; i++){
        if(!(generator.moves[i].flags & (MoveFlags::CASTLE_RIGHT | MoveFlags::CASTLE_LEFT))){
            continue;
        }

        std::cout << "Move: " << i << std::endl;
        std::swap(board[generator.moves[i].from], board[generator.moves[i].to]);
        std::cout << board.toString() << "\n";
        std::swap(board[generator.moves[i].to], board[generator.moves[i].from]);
    }

    for(int i = 7; i >= 0; i--){
        for(int j = 0; j < 8; j++){
            cout << board.isAttacked(Board::positionToIndex(j, i)) << " ";
        }
        cout << endl;
    }

//    system("pause");
}
