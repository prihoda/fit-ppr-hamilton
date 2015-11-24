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
    int numOperations;
    stack<int> bestPath;
    int bestLength;
    bool foundLimit;
    void maxFromRoot(int fromRoot);
    void visit(edge current);
    void setBestPath(int nodeAtEnd);

    int neighbours(int m);

public:

    MaxHamilton(Graph* graph);
    ~MaxHamilton();
    void max();
};

#endif //PPR_SEQUENTIAL_HAMILTON_H
