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
    MPI_Comm_rank(MPI_COMM_WORLD, &askForWork);
    askForWork = (askForWork + 1) % numProcessors;
    color = 'W';
    token = 'N';

    // remove old best path
    bestLength = 0;
    while (!bestPath.empty())
        bestPath.pop();

    // start at each node
    cout << "Process no. " << rank << " starting from nodes { ";
    for(int i=rank; i<g->size; i+=numProcessors) {
        if (neighbours(i) > 1){//(!foundLimit) && (
            cout << i << " ";
	// prepare first edge (beginning is marked with -1)
	    edge rootEdge;
	    rootEdge.from = -1;
	    rootEdge.to = i;
            s.push_front(rootEdge);
        }
    }
    cout << "}" << endl;

    // push root edge to stack and start
    //s.push(rootEdge);
    //cout << "Starting at " << root << endl;

    // keep removing edges from stack

    while (askForWork != rank && !isFinished) {
	    while (!s.empty() && !isFinished) {
		edge current = s.front();
		s.pop_front();
		if(current.from == -1){
		    // reset graph variables
		    g->reset();
		    root = current.to;
		}

		visit(current);
		numOperations++;

		if ((numOperations % CHECK_MSG_AMOUNT)==0){
		    int flag;
		    MPI_Status status;
		    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
		    if (flag){
		        checkMessage(status);
		    }
		}
	    }
            waitForWork();
	    
    }


    color = 'W';

    // uz nema praci
    if (rank == 0)
    {
	MPI_Status status;
        char w = 'W';
        cout << "Main process has finished working, waiting for white token" << endl;
	MPI_Send (&w, 1, MPI_CHAR, ((rank+1) % numProcessors), MSG_TOKEN, MPI_COMM_WORLD);

	do {
        // posle bileho peska
		int flag;
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
		if (flag){
		    checkMessage(status);
		}
        } while (token != 'W' || isFinished);

        cout << "Main process received white token, waiting for results" << endl;

        // procesor 0 ceka na odpovedi vsech procesoru
	for (int i = 1; i < numProcessors; i++)
        {
	    MPI_Send (NULL, 0, MPI_CHAR, i, MSG_FINISH, MPI_COMM_WORLD);
        }
        for (int i = 1; i < numProcessors; i++)
        {
	    int w;
	    MPI_Recv(&w, 1, MPI_INT, status.MPI_SOURCE, MSG_TOKEN, MPI_COMM_WORLD, &status);
            cout << "Main process received result " << w << endl;
            // prijme ypravu s nejlepsi cestou procesoru i
            // porovna se svou nejlepsi kruznici
            // pokud je dosla kruznice vetsi nez dosud nejlepsi, prepis
        }
    }
    else
    {
	MPI_Status status;
	MPI_Request request;
        // pokud ma procesor peska, posle ho (rank+1 % numProcessors) procesoru
        cout << "Process " << rank << " finishing naturally" << endl;
        while(true){
		int flag;
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
		if (flag){
		    checkMessage(status);
		}
		if (token != 'N')
		{
        	    cout << "Process " << rank << " resending token " << token << endl;
		    MPI_Isend (&token, 1, MPI_CHAR, ((rank+1) % numProcessors), MSG_TOKEN, MPI_COMM_WORLD, &request);
		    token = 'N';
		}
	}
        // ceka a kontroluje prichozi zpravy
    }

    // done, print the longest circle
    //cout << "-----------------------------" << endl;
    //cout << "Number of stack pops: " << numOperations << endl;
    //cout << "-----------------------------" << endl;
    cout << "Process no. " << rank << " finished: Longest hamiltonian subgraph: " << endl;

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
    cout << "Processor " << rank << " asking " << askForWork << " for work, waiting..." << endl;
    MPI_Send (NULL, 0, MPI_CHAR, askForWork, MSG_WORK_REQUEST, MPI_COMM_WORLD);
    int w = -1;
    MPI_Status status;
    do {
	    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
	    if (flag){
                if(status.MPI_TAG == MSG_WORK_SENT) break;
		checkMessage(status);
	    }
    } while(true);

    MPI_Recv(&w, 1, MPI_INT, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD, &status);

    if(w == -1) {
	cout << "Processor " << rank << " received no work from " << askForWork << endl;
        askForWork = (askForWork + 1) % numProcessors;
    } else { 
	cout << "Processor " << rank << " received work '" << w <<"' from " << askForWork << endl;
    }

}
void MaxHamilton::checkMessage(MPI_Status status){
      //prisla zprava, je treba ji obslouzit
      //v promenne status je tag (status.MPI_TAG), cislo odesilatele (status.MPI_SOURCE)
      //a pripadne cislo chyby (status.MPI_ERROR)
      char t = ' ';
      MPI_Recv(&t, 1, MPI_CHAR, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
      cout << "Process " << rank << " received message " << status.MPI_TAG << ": '" << t << "' from " << status.MPI_SOURCE << endl;
      switch (status.MPI_TAG)
      {
         case MSG_WORK_REQUEST : {// zadost o praci, prijmout a dopovedet
		                         // zaslat rozdeleny zasobnik a nebo odmitnuti MSG_WORK_NOWORK
					MPI_Request request;
		                         work* w = getSharableWork();
		                         if (w == NULL)
		                         {
	cout << "Process " << rank << " sending no work message to " << status.MPI_SOURCE << endl;
                                             
		                            MPI_Isend (NULL, 0, MPI_INT, status.MPI_SOURCE , MSG_WORK_SENT, MPI_COMM_WORLD, &request);
		                         }
		                         else
		                         {
	cout << "Process " << rank << " sending work to " << status.MPI_SOURCE << endl;
					    int msg = 5;
		                            MPI_Isend (&msg, 1, MPI_INT, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD, &request);
					    color = 'B';
		                         }
                                 }
break;
         case MSG_TOKEN : //ukoncovaci token, prijmout a nasledne preposlat
                          // - bily nebo cerny v zavislosti na stavu procesu
                          token = t;
                          if (color == 'B') token = 'B';
                          if (rank == 0 && token == 'B'){
				char w = 'W';
				token = 'N';
				MPI_Send (&w, 1, MPI_CHAR, ((rank+1) % numProcessors), MSG_TOKEN, MPI_COMM_WORLD);
			  }
                          break;
         case MSG_FINISH : //konec vypoctu - proces 0 pomoci tokenu zjistil, ze jiz nikdo nema praci
                           //a rozeslal zpravu ukoncujici vypocet
                           //mam-li reseni, odeslu procesu 0
                           //nasledne ukoncim spoji cinnost
                           //jestlize se meri cas, nezapomen zavolat koncovou barieru MPI_Barrier (MPI_COMM_WORLD)
                           if (rank != 0)
                           {
                               // posle sve nejlepsi reseni procesoru 0
                               //MPI_Send (bestPath, bestLength, MPI_INT, 0, MSG_TOKEN, MPI_COMM_WORLD);
                                int done = 666;
                                cout << "Process " << rank << " was told to finish, sending " << done << endl;
				MPI_Send (&done, 1, MPI_INT, 0, MSG_TOKEN, MPI_COMM_WORLD);
		                   MPI_Finalize();
		                   exit (0);
                           }
                           else
                           {
                                isFinished = true;// nejaky procesor nasel kruznici delky trivialni horni mezi

                           }
                           break;
         default : std::cout << "neznamy typ zpravy" << endl; break;
      }
}

work* MaxHamilton::getSharableWork(){
   if(s.back().from == -1){
      edge rootEdge = s.back();
      s.pop_back();
      work* workUnit = new work;
      workUnit->root = rootEdge;

      return workUnit;
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
        s.push_front(nextEdge);
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
    //cout << "---- Setting best path (length " << length << "): ";
    bestLength = length;
    current = nodeAtEnd;
    while (current != -1) {
        //cout << current << " ";
        bestPath.push(current);
        current = g->prev[current];
    }

    if (bestLength == g->size)
    {
        foundLimit = true;
    }

    //cout << endl;
}

MaxHamilton::MaxHamilton(Graph *graph, int numProcessors, int rank) {
    g = graph;
    this->numProcessors = numProcessors;
    this->rank = rank;
}


MaxHamilton::~MaxHamilton() {
}
