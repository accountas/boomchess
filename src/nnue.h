#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include <string>
#include <immintrin.h>
#include <fstream>
#include <iostream>
#include "Common.h"

namespace NNUE {
    constexpr int INPUT_SIZE = 768;
    constexpr int L1_SIZE = 128;
    constexpr int L2_SIZE = 32;
    constexpr int Q_FACTOR = 256;
    using QT = int16_t;
    using QTO = int32_t;

    /**
     * LINEAR LAYER
     */
    template<int INPUT_N, int OUTPUT_N>
    class LinearLayer {
     public:
        std::array<QTO, OUTPUT_N> bias{};
        std::array<std::array<QT, OUTPUT_N>, INPUT_N> weights{}; //[IN][OUT]
        void load(std::ifstream &fileStream, int weightScale, int biasScale) {
            for (int i = 0; i < OUTPUT_N; i++) {
                for (int j = 0; j < INPUT_N; j++) {
                    double w;
                    fileStream >> w;

                    weights[j][i] = static_cast<QT>(w * weightScale);
                }
            }
            for (int i = 0; i < OUTPUT_N; i++) {
                double w;
                fileStream >> w;

                bias[i] = static_cast<QTO>(w * biasScale);
            }
        }
    };

    /**
     * ACCUMULATOR
     */
    template<int MAX_PLY, int WIDTH>
    class NnueAccumulator {
     public:
        explicit NnueAccumulator(LinearLayer<INPUT_SIZE, L1_SIZE> *layer_1) : layer_1(layer_1) {};

        template<bool REMOVE_PIECE>
        void stageChange(int square0x88, int piece, int color) {
            int square = (square0x88 + (square0x88 & 7)) >> 1;
            int featureWhite = square * 12 + (piece - 1) * 2 + color;
            int featureBlack = (square ^ 56) * 12 + (piece - 1) * 2 + (1 - color);
            auto update = std::make_pair(featureWhite, featureBlack);

            if (!REMOVE_PIECE) {
                stagingNewFeatures[newIdx++] = update;
            } else {
                stagingDelFeatures[delIdx++] = update;
            }
        }

        void init(){
            for(int i = 0; i < L1_SIZE; i++){
                accumulator[ply][WHITE][i] = static_cast<QT>(layer_1->bias[i]);
                accumulator[ply][BLACK][i] = static_cast<QT>(layer_1->bias[i]);
            }
        }
        void applyStagedChanges(bool copy = true) {
            if (copy)
                for (int color : {WHITE, BLACK}) {
                    std::copy(accumulator[ply - 1][color].begin(),
                              accumulator[ply - 1][color].end(),
                              accumulator[ply][color].begin());
                }

            for (int f = 0; f < newIdx; f++) {
                auto &feature = stagingNewFeatures[f];
                for (int i = 0; i < WIDTH; i++) {

                    accumulator[ply][WHITE][i] += layer_1->weights[feature.first][i];
                    accumulator[ply][BLACK][i] += layer_1->weights[feature.second][i];
                }
            }

            for (int f = 0; f < delIdx; f++) {
                auto &feature = stagingDelFeatures[f];
                for (int i = 0; i < WIDTH; i++) {
                    accumulator[ply][WHITE][i] -= layer_1->weights[feature.first][i];
                    accumulator[ply][BLACK][i] -= layer_1->weights[feature.second][i];
                }
            }

            delIdx = 0;
            newIdx = 0;
        }

        void increaseDepth() { ply++; }
        void decreaseDepth() { ply--; }
        void setDepth(int depth) { ply = depth; }

        std::array<QT, WIDTH> &operator[](int stm) {
            return accumulator[ply][stm];
        }
        int ply = 0;
     private:

        int newIdx = 0;
        int delIdx = 0;

        LinearLayer<INPUT_SIZE, L1_SIZE> *layer_1;
        std::array<std::array<std::array<QT, WIDTH>, 2>, MAX_PLY> accumulator{};
        std::array<std::pair<int, int>, 64> stagingNewFeatures{};
        std::array<std::pair<int, int>, 64> stagingDelFeatures{};
    };

    /**
     * NNUE
     */
    class NNUE {
     public:
        NNUE() : accumulator(&layer_1) {}
        void loadNetwork(const std::string &fileName);
        int evaluate(int sideToMove);

        NnueAccumulator<MAX_DEPTH, L1_SIZE> accumulator;

     private:
        LinearLayer<INPUT_SIZE, L1_SIZE> layer_1{};
        LinearLayer<L1_SIZE * 2, L2_SIZE> layer_2{};
        LinearLayer<L2_SIZE, 1> layer_3{};

        template<int N>
        void applyClippedReLU(std::array<QT, N> &input);

        template<int N, int M>
        void applyLinear(const LinearLayer<N, M> &layer, std::array<QT, N> &input, std::array<QT, M> &output);
    };

}
