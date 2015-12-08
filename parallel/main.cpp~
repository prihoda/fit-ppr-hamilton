#include <iostream>
#include "Graph.h"
#include "MaxHamilton.h"
#include <time.h>
#include <fstream>
#include <mpi.h>

using namespace std;

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank;
    int processors;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &processors);

    ifstream file;
    file.open("../parallel/data/graf.txt", ios::in);
    Graph* graph = new Graph();
    //cout << "Input graph: " << endl;
    file >> *graph;
    //cout << "Graph: " << endl << *graph << endl;
    clock_t start, end;
    start = clock();

    file.close();
    MaxHamilton algorithm(graph, processors, rank);

    algorithm.max();
    delete graph;

    end = clock();
    cout << "Time required for execution: " << (double)(end-start)/CLOCKS_PER_SEC << " seconds." << "\n\n";
    MPI_Finalize();

    return 0;
}
