//
// Created by marty on 2022-11-09.
//

#ifndef BOOMCHESS_MOVE_H_
#define BOOMCHESS_MOVE_H_

#include <cstdint>

class Move {
 public:
    int from;
    int to;
    int flags;

    Move(int from, int to, int flags) : from(from), to(to), flags(flags) {}
    Move(int from, int to) : from(from), to(to), flags(0) {}
    Move() : from(-1), to(-1), flags(0) {}
    bool same(const Move &other) const {
        return from == other.from && to == other.to;
    }

};
#endif //BOOMCHESS_MOVE_H_
