//
// Created by david on 18.10.15.
//

#ifndef PPR_SEQUENTIAL_GRAPH_H
#define PPR_SEQUENTIAL_GRAPH_H

#include <iostream>
using namespace std;

class Graph {
public:
    bool** adjacent;
    bool*discovered;
    int size;

    ~Graph();
    void reset();
    friend ostream &operator<<( ostream &output, const Graph &G );
    friend istream &operator>>( istream  &input, Graph &G );

    Graph();
};

#endif //PPR_SEQUENTIAL_GRAPH_H
