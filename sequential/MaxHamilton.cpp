//
// Created by david on 18.10.15.
//

#include "MaxHamilton.h"


void MaxHamilton::max() {
    g->reset();
    s.push(0);
    g->discovered[0] = true;
    while(!s.empty()) {
        int node = s.top();
        s.pop();
        visit(node);
    }
}

void MaxHamilton::visit(int node) {
    cout << "Visiting " << node << endl;
    for(int i=0; i<g->size; i++) {
        if(g->adjacent[node][i] && !g->discovered[i]) {
            g->discovered[i] = true;
            cout << "  Adding " << i << endl;
            s.push(i);
        }
    }
}

MaxHamilton::MaxHamilton(Graph* graph) {
    g = graph;
}
