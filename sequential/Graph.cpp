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
    delete[] discovered;
}

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

istream &operator>>( istream  &input, Graph &G )
{
    input >> G.size;
    G.adjacent = new bool*[G.size];
    G.discovered = new bool[G.size];
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
        discovered[i] = false;
    }
}
