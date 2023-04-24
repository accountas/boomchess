#include "nnue.h"

namespace NNUE {

    void NNUE::loadNetwork(const std::string &fileName) {
        std::ifstream fileStream(fileName);
        layer_1.load(fileStream, Q_FACTOR, Q_FACTOR);
        layer_2.load(fileStream, Q_FACTOR, Q_FACTOR * Q_FACTOR);
        layer_3.load(fileStream, Q_FACTOR, Q_FACTOR * Q_FACTOR);

        accumulator.init();
    }

    int NNUE::evaluate(int sideToMove) {
        // join the outputs of accumulators
        static std::array<QT, L1_SIZE * 2> accumulators;

        for (int i = 0; i < L1_SIZE; i++) {
            accumulators[i] = accumulator[sideToMove][i];
            accumulators[i + L1_SIZE] = accumulator[1 - sideToMove][i];
        }

        applyClippedReLU<L1_SIZE * 2>(accumulators);

        //apply second hidden layer
        static std::array<QT, L2_SIZE> outputLayer2;

        applyLinear<L1_SIZE * 2, L2_SIZE>(layer_2, accumulators, outputLayer2);
        applyClippedReLU<L2_SIZE>(outputLayer2);

        //apply final layer
        static std::array<QT, 1> outputLayer3;
        applyLinear<L2_SIZE, 1>(layer_3, outputLayer2, outputLayer3);

        return static_cast<int>(outputLayer3[0]) * 100 / Q_FACTOR;
    }

    template<int N>
    void NNUE::applyClippedReLU(std::array<QT, N> &input) {
        for (int i = 0; i < N; i++) {
            input[i] = std::min(std::max(input[i], static_cast<QT>(0)), static_cast<QT>(Q_FACTOR));
        }
    }

    template<int N, int M>
    void NNUE::applyLinear(const LinearLayer<N, M> &layer, std::array<QT, N> &input, std::array<QT, M> &output) {
        static std::array<QTO, M> outputBig;

        std::copy(layer.bias.begin(), layer.bias.end(), outputBig.begin());

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                outputBig[j] +=  input[i] * layer.weights[i][j];
            }
        }

        for (int i = 0; i < M; i++) {
            output[i] = outputBig[i] / Q_FACTOR;
        }
    }

}
