//
// Created by david on 18.10.15.
//

#ifndef PPR_SEQUENTIAL_HAMILTON_H
#define PPR_SEQUENTIAL_HAMILTON_H


#include <stack>
#include "Graph.h"

class MaxHamilton {
protected:
    stack<int> s;
    Graph* g;
public:
    MaxHamilton(Graph* graph);
    void max();
    void visit(int node);
};


#endif //PPR_SEQUENTIAL_HAMILTON_H
