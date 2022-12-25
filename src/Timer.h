//
// Created by marty on 2022-11-19.
//

#ifndef BOOMCHESS_SRC_TIMER_H_
#define BOOMCHESS_SRC_TIMER_H_

#include <chrono>

class Timer {
 public:
    void start() {
        startTs = std::chrono::high_resolution_clock::now();
    }
    void end() {
        endTs = std::chrono::high_resolution_clock::now();
    }
    double getSecondsFromStart() {
        auto now = std::chrono::high_resolution_clock::now();
        auto delta = now - startTs;
        auto nanos = (double) std::chrono::duration_cast<std::chrono::nanoseconds>(delta).count();
        return nanos / 1e9;
    }
    double getSeconds() {
        auto delta = endTs - startTs;
        auto nanos = (double) std::chrono::duration_cast<std::chrono::nanoseconds>(delta).count();
        return nanos / 1e9;
    }

    template<class F>
    void measure(F &&f) {
        start();
        f();
        end();
    }

 private:
    std::chrono::high_resolution_clock::time_point startTs, endTs;
};
#endif //BOOMCHESS_SRC_TIMER_H_
