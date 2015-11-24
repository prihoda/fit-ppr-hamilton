#include <iostream>
#include "Graph.h"
#include "MaxHamilton.h"
#include <time.h>
#include <fstream>

using namespace std;



int main() {
    ifstream file;
    file.open("input.txt", ios::in);
    Graph* graph = new Graph();
    cout << "Input graph: " << endl;
    file >> *graph;
    cout << "Graph: " << endl << *graph << endl;

    clock_t start, end;
    start = clock();

    MaxHamilton algorithm(graph);
    algorithm.max();
    delete graph;

    file.close();

    end = clock();
    cout << "Time required for execution: " << (double)(end-start)/CLOCKS_PER_SEC << " seconds." << "\n\n";

    return 0;
}
