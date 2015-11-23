//
// Created by david on 18.10.15.
//
#include <iostream>
#include "Graph.h"

using namespace std;


Graph::Graph() {

}


Graph::~Graph() {
    for(int i=0; i<size; i++){
        delete[] adjacent[i];
    }
    delete[] adjacent;
    delete[] prev;
}

// write graph matrix to output
ostream &operator<<( ostream &output, const Graph &G )
{

    output << G.size << endl;
    for(int i=0; i<G.size; i++){
        for(int j=0; j<G.size;j++){
            output << (G.adjacent[i][j] ? '1' : '0');
        }
        output << endl;
    }
    return output;
}

// read the graph from input
istream &operator>>( istream  &input, Graph &G )
{
    input >> G.size;
    G.adjacent = new bool*[G.size];
    G.prev = new int[G.size];
    for(int i=0; i<G.size; i++){
        char ch;
        G.adjacent[i] = new bool[G.size];
        for(int j=0; j<G.size; j++){
            input >> ch;
            G.adjacent[i][j] = ch == '1';
        }
    }
    return input;
}

void Graph::reset() {
    for(int i=0; i<size; i++){
        prev[i] = -1;
    }
}

// find nodeToFind backtracking from nodeAtEnd
bool Graph::isOnPath(int nodeAtEnd, int nodeToFind) {
    int current = nodeAtEnd;
    while (current != -1) {
        if (current == nodeToFind)
            return true;
        current = prev[current];
    }
    return false;
}

// print path backwards starting at nodeAtEnd
void Graph::printPath(int nodeAtEnd) {
    int current = nodeAtEnd;
    while (current != -1) {
        cout << current << " ";
        current = prev[current];
    }
    cout << endl;
}
