#include <iostream>
#include "Graph.h"
#include "MaxHamilton.h"

using namespace std;



int main() {
    Graph* graph = new Graph();
    cout << "Input graph: " << endl;
    cin >> *graph;
    cout << "Graph: " << endl << *graph << endl;
    MaxHamilton algorithm(graph);
    algorithm.max();
    delete graph;
    return 0;
}