//
// Created by david on 18.10.15.
//

#include "MaxHamilton.h"

using namespace std;

int MaxHamilton::neighbours(int m)
{
    int n =0;
    for (int i=0; i<g->size; i++)
    {
        if (g->adjacent[m][i])
        {
            n++;
        }
    }
    return n;
}


void MaxHamilton::max() {

    numOperations = 0;
    foundLimit = false;

    // remove old best path
    bestLength = 0;
    while (!bestPath.empty())
        bestPath.pop();

    // start at each node
    for(int i=0; i<g->size; i++) {
        if ((!foundLimit) && (neighbours(i) > 1)){
            maxFromRoot(i);
        }
    }

    // done, print the longest circle
    cout << "-----------------------------" << endl;
    cout << "Number of stack pops: " << numOperations << endl;
    cout << "-----------------------------" << endl;
    cout << "Longest hamiltonian subgraph: " << endl;

    if (bestPath.empty())
        cout << "Not found.";

    while (!bestPath.empty()) {
        cout << bestPath.top() << " ";
        bestPath.pop();
    }
    cout << endl;
}

void MaxHamilton::maxFromRoot(int fromRoot) {
    // reset graph variables
    g->reset();

    root = fromRoot;

    // prepare first edge (beginning is marked with -1)
    edge rootEdge;
    rootEdge.from = -1;
    rootEdge.to = root;

    // push root edge to stack and start
    s.push(rootEdge);
    //cout << "Starting at " << root << endl;

    // keep removing edges from stack
    while (!s.empty()) {
        edge current = s.top();
        s.pop();
        visit(current);
        numOperations++;
    }

}

void MaxHamilton::visit(edge currentEdge) {

    // update the current path (just connect the current edge to the existing path)
    g->prev[currentEdge.to] = currentEdge.from;

    //cout << " Visiting " << currentEdge.from << "->" << currentEdge.to;
    //cout << ", path: ";
    //g->printPath(currentEdge.to);

    // loop through all neighbors
    for (int next = g->size - 1; next >= 0; next--) {
        if (!g->adjacent[currentEdge.to][next]) continue;
        //cout << "   Opening " << next << ": ";

        // came back to root in more than two steps
        if (next == root && g->prev[currentEdge.to] != root) {
            //cout << "Found cycle: ";
            //g->printPath(currentEdge.to);
            setBestPath(currentEdge.to);
            continue;
        }

        // an already visited neighbor
        if (g->isOnPath(currentEdge.to, next)) {
            //cout << "Already visited " << endl;
            continue;
        }

        // add the edge to the stack
        edge nextEdge;
        nextEdge.from = currentEdge.to;
        nextEdge.to = next;
        //cout << "Adding " << nextEdge.from << "->" << nextEdge.to << endl;
        s.push(nextEdge);
    }
}

void MaxHamilton::setBestPath(int nodeAtEnd) {
    int length = 0;

    // calculate path length
    int current = nodeAtEnd;
    while (current != -1) {
        length++;
        current = g->prev[current];
    }
    if (length <= bestLength) return;


    // remove old best path
    while (!bestPath.empty())
        bestPath.pop();

    // save best path
    cout << "---- Setting best path (length " << length << "): ";
    bestLength = length;
    current = nodeAtEnd;
    while (current != -1) {
        cout << current << " ";
        bestPath.push(current);
        current = g->prev[current];
    }

    if (bestLength == g->size)
    {
        foundLimit = true;
    }

    cout << endl;
}

MaxHamilton::MaxHamilton(Graph *graph) {
    g = graph;
}


MaxHamilton::~MaxHamilton() {
}
