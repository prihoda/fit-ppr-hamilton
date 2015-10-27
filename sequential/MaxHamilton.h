//
// Created by david on 18.10.15.
//

#ifndef PPR_SEQUENTIAL_HAMILTON_H
#define PPR_SEQUENTIAL_HAMILTON_H


#include <stack>
#include "Graph.h"

struct edge {
    int from;
    int to;
};


class MaxHamilton {
protected:
    stack<edge> s;
    Graph* g;
    int root;
    stack<int> bestPath;
    int bestLength;

public:

    MaxHamilton(Graph* graph);
    ~MaxHamilton();
    void max();
    void visit(edge current);
    void setBestPath(int nodeAtEnd);
};

#endif //PPR_SEQUENTIAL_HAMILTON_H
