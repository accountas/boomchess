#include <iostream>
#include "Driver.h"
#include <mpi.h>
#include <sstream>
#include "tools/DatasetGeneration.h"

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::cout << "Hello from process: " << rank + 1 << "/" << size << std::endl;

    // Read input
    std::string input;
    if(rank == 0){
        std::getline(std::cin, input);
    }
    
    //share input
    int inputSize = input.size();
    MPI_Bcast(&inputSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank != 0) input.resize(inputSize);
    MPI_Bcast(const_cast<char*>(input.data()), inputSize, MPI_CHAR, 0, MPI_COMM_WORLD);

    std::cout << rank << ": " << input << std::endl;

    // Tokenize command
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(input);
     while (std::getline(token_stream, token, ' ')) {
        tokens.push_back(token);
    }

    // Update tokens
    tokens.push_back("start");
    tokens.push_back(std::to_string(rank));

    tokens.push_back("skip");
    tokens.push_back(std::to_string(size));

    tokens.push_back("outputfile");
    tokens.push_back("evaluated_" + std::to_string(rank) + ".csv");

    // Run the tasks
    DatasetGenerator generator;
    generator.evaluatePositions(tokens);

    MPI_Finalize();

    // Driver driver = Driver();
    // driver.start();
}
