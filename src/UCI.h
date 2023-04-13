#pragma once

#include <vector>
#include "Board.h"

class UCI {
 public:
    static Board parsePosition(const std::vector<std::string> &tokens);
    static SearchParams parseGo(const std::vector<std::string> &tokens);
    static void sendInfo(int depth, int eval, const Move &best, double time);
    static void sendResult(const Move &bestMove);
    static void setQuiet(bool value);
 private:
    inline static bool isQuiet = false;
};
