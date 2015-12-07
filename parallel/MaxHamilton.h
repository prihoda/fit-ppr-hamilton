//
// Created by david on 18.10.15.
//

#ifndef PPR_SEQUENTIAL_HAMILTON_H
#define PPR_SEQUENTIAL_HAMILTON_H

#define CHECK_MSG_AMOUNT  5

#define MSG_WORK_REQUEST 1000
#define MSG_WORK_SENT    1001
#define MSG_WORK_NOWORK  1002
#define MSG_TOKEN        1003
#define MSG_FINISH       1004

#include <cstdlib>
#include <deque>
#include <stack>
#include <mpi.h>
#include "Graph.h"

struct edge {
    int from;
    int to;
};
struct work {
    edge root;
    int*prev;
};


class MaxHamilton {
protected:
    deque<edge> s;
    Graph* g;
    int root;
    int numOperations;
    int numProcessors;
    int rank;
    stack<int> bestPath;
    int bestLength;
    bool foundLimit;
    void visit(edge current);
    work* getSharableWork();
    void setBestPath(int nodeAtEnd);
    void checkMessage(MPI_Status status);
    void waitForWork();
    int neighbours(int m);
    int askToWork;
    char color;
    char token;

public:

    MaxHamilton(Graph* graph, int numProcessors, int rank);
    ~MaxHamilton();
    void max();
};

#endif //PPR_SEQUENTIAL_HAMILTON_H
