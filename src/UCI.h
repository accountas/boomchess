//
// Created by marty on 2022-12-13.
//

#ifndef BOOMCHESS_SRC_UCI_H_
#define BOOMCHESS_SRC_UCI_H_

#include <vector>
#include "Board.h"

class UCI {
 public:
    static Board parsePosition(const std::vector<std::string> &tokens);
    static SearchParams parseGo(const std::vector<std::string> &tokens);
    static void sendInfo(int depth, int eval, const Move &best);
    static void sendResult(const Move &bestMove);
};

#endif //BOOMCHESS_SRC_UCI_H_
