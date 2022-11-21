//
// Created by marty on 2022-11-21.
//

#ifndef BOOMCHESS_SRC_METRICS_H_
#define BOOMCHESS_SRC_METRICS_H_

#include <unordered_map>
#include <string>

template<typename T = int>
class Metrics {
 public:
    static void inc(std::string name){
        metrics[name]++;
    }
    static void dec(std::string name){
        metrics[name]++;
    }
    static T get(std::string name){
        return metrics[name];
    }
 private:
    static std::unordered_map<std::string, T> metrics;
};
#endif //BOOMCHESS_SRC_METRICS_H_
