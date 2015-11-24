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
    for(int i=rank; i<g->size; i+=numProcessors) {
        if (neighbours(i) > 1){//(!foundLimit) && (
	// prepare first edge (beginning is marked with -1)
	    edge rootEdge;
	    rootEdge.from = -1;
	    rootEdge.to = i;
            s.push_front(rootEdge);
        }
    }

    // push root edge to stack and start
    //s.push(rootEdge);
    //cout << "Starting at " << root << endl;

    // keep removing edges from stack
    while (!s.empty()) {
        edge current = s.front();
        s.pop_front();
        if(current.from == -1){
            // reset graph variables
            g->reset();
            root = current.to;
        }
        if ((numOperations % CHECK_MSG_AMOUNT)==0){
	   int flag;
	   MPI_Status status;
	   MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
	   if (flag){
              checkMessage(status);
	   }
	}

        visit(current);
        numOperations++;
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
void MaxHamilton::waitForWork(){
    int flag;
    MPI_Status status;
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
    if (flag)
    {
      //prisla zprava, je treba ji obslouzit
      //v promenne status je tag (status.MPI_TAG), cislo odesilatele (status.MPI_SOURCE)
      //a pripadne cislo chyby (status.MPI_ERROR)
      switch (status.MPI_TAG)
      {
         case MSG_WORK_SENT : // prisel rozdeleny zasobnik, prijmout
                              // deserializovat a spustit vypocet
                              break;
         case MSG_WORK_NOWORK : // odmitnuti zadosti o praci
                                // zkusit jiny proces
                                // a nebo se prepnout do pasivniho stavu a cekat na token
                                break
         default : chyba("neznamy typ zpravy"); break;
      }
      checkMessage(status);
    }
    
}
void MaxHamilton::checkMessage(MPI_Status status){
{
      //prisla zprava, je treba ji obslouzit
      //v promenne status je tag (status.MPI_TAG), cislo odesilatele (status.MPI_SOURCE)
      //a pripadne cislo chyby (status.MPI_ERROR)
      switch (status.MPI_TAG)
      {
         case MSG_WORK_REQUEST : // zadost o praci, prijmout a dopovedet
                                 // zaslat rozdeleny zasobnik a nebo odmitnuti MSG_WORK_NOWORK
                                 break;
         case MSG_TOKEN : //ukoncovaci token, prijmout a nasledne preposlat
                          // - bily nebo cerny v zavislosti na stavu procesu
                          break;
         case MSG_FINISH : //konec vypoctu - proces 0 pomoci tokenu zjistil, ze jiz nikdo nema praci
                           //a rozeslal zpravu ukoncujici vypocet
                           //mam-li reseni, odeslu procesu 0
                           //nasledne ukoncim spoji cinnost
                           //jestlize se meri cas, nezapomen zavolat koncovou barieru MPI_Barrier (MPI_COMM_WORLD)
                           MPI_Finalize();
                           exit (0);
                           break;
         default : chyba("neznamy typ zpravy"); break;
      }
}
work* MaxHamilton::getSharableWork(){
   if(s.back().from == -1){
      edge rootEdge = s.back();
      s.pop_back();
      return rootEdge;
   }
   
   return NULL;
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

MaxHamilton::MaxHamilton(Graph *graph, int numProcessors, int rank) {
    g = graph;
    this.numProcessors = numProcessors;
    this.rank = rank;
}


MaxHamilton::~MaxHamilton() {
}
