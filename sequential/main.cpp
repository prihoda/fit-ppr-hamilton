#include <iostream>
#include "Graph.h"
#include "MaxHamilton.h"

using namespace std;



int main() {
    Graph* graph = new Graph();
    cout << "Input graph: " << endl;
    cin >> *graph;
    cout << "Graph: " << endl << *graph << endl;

    clock_t start, end;
    start = clock();

    MaxHamilton algorithm(graph);
    algorithm.max();


    end = clock();
    cout << "Time required for execution: " << (double)(end-start)/CLOCKS_PER_SEC << " seconds." << "\n\n";

    delete graph;
    return 0;
}
