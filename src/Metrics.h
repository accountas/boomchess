//
// Created by marty on 2022-11-21.
//

#ifndef BOOMCHESS_SRC_METRICS_H_
#define BOOMCHESS_SRC_METRICS_H_

#include <unordered_map>
#include <string>

template<int ID, typename T = int64_t>
class Metric {
 public:
    static void inc(T n = 1) {
        if (USE_METRICS && ID >= 0) {
            value += n;
        }
    }
    static void dec(T n = 1) {
        if (USE_METRICS && ID >= 0) {
            value -= n;
        }
    }
    static void set(T n = 1) {
        if (USE_METRICS && ID >= 0) {
            value = n;
        }
    }
    static T get() {
        return value;
    }
 private:
    static T value;
};

template<int ID, typename T>
T Metric<ID, T>::value = 0;

#endif //BOOMCHESS_SRC_METRICS_H_
