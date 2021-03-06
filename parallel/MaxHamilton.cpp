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

void MaxHamilton::bestPathToArray(int * buffer){
    int n=0;
    while(!bestPath.empty()){
       buffer[n] = bestPath.top();
       bestPath.pop();
       n++;
    }
    cout << "Best path of process " << rank << ": ";
    for(int i=0; i<n; i++){
       cout << buffer[i] << " ";
    }
    cout << endl;
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
        for(int i=0; i<g->size; i++) {
        if(i%numProcessors != rank) continue;
       //if(rank != 1) continue;
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
	        cout << "Process " << rank << " is starting from root " << current.to << endl;
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
        if(!isFinished) waitForWork();
    }
    color = 'W';

    if (rank != 0) {
        MPI_Status status;
        MPI_Request request;
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
    }


	MPI_Status status;
	char w = 'W';
	cout << "Main process has finished working, waiting for white token" << endl;
	MPI_Send (&w, 1, MPI_CHAR, ((rank+1) % numProcessors), MSG_TOKEN, MPI_COMM_WORLD);

	do {
		int flag;
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
		if (flag){
		    checkMessage(status);
		}
	} while (token != 'W' || isFinished);

	cout << "Main process received white token, waiting for results" << endl;
 	char blank = ' ';

	for (int i = 1; i < numProcessors; i++)
	{
	    MPI_Send (&blank, 1, MPI_CHAR, i, MSG_FINISH, MPI_COMM_WORLD);
	}

    int * bestPath = new int[bestLength];
    bestPathToArray(bestPath);

	for (int i = 1; i < numProcessors; i++)
	{
	    int length;
	    MPI_Recv(&length, 1, MPI_INT, i, MSG_TOKEN, MPI_COMM_WORLD, &status);
	    int * result = new int[length];
	    MPI_Recv(result, length, MPI_INT, i, MSG_TOKEN, MPI_COMM_WORLD, &status);

		cout << "Received best path of " << i << ": ";
		for(int i=0; i<length; i++){
		cout << result[i] << " ";
		}
		cout << endl;

	    if(length > bestLength) {
            bestLength = length;
            bestPath = result;
	    } else {
            delete[] result;
		}
	    cout << "Main process received result of length " << length << " from " << status.MPI_SOURCE << endl;
	}

    // done, print the longest circle
    //cout << "-----------------------------" << endl;
    //cout << "Number of stack pops: " << numOperations << endl;
    //cout << "-----------------------------" << endl;
    cout << "Process no. " << rank << " finished: Longest hamiltonian subgraph: " << endl;

    for(int i=0; i<bestLength; i++){
       cout << bestPath[i] << " ";
    }
    delete [] bestPath;
    cout << endl;
}

void MaxHamilton::waitForWork(){
    cout << "Process " << rank << " asking " << askForWork << " for work, waiting..." << endl;
    char blank = ' ';
    MPI_Send (&blank, 1, MPI_CHAR, askForWork, MSG_WORK_REQUEST, MPI_COMM_WORLD);
    int w = -1;
    MPI_Status status;
    int stackSize = 0;
    do {
    	int flag;
	    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
	    if (flag){
            if(status.MPI_TAG == MSG_WORK_SIZE) {
                MPI_Recv(&stackSize, 1, MPI_INT, status.MPI_SOURCE, MSG_WORK_SIZE, MPI_COMM_WORLD, &status);
                continue;
            }
            if(status.MPI_TAG == MSG_WORK_SENT) break;
            checkMessage(status);
	    }
    } while(true);


    if(stackSize == 0) {
        MPI_Recv(&w, 1, MPI_INT, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD, &status);
        cout << "Process " << rank << " received no work from " << askForWork << endl;
        askForWork = (askForWork + 1) % numProcessors;
    } else {
        int structSize = stackSize + g->size;
        int * stack = new int[stackSize];
        int * workStruct = new int[structSize];
        int position=0;
        cout << "Process " << rank << " is receiving work of size " << structSize << " from " << askForWork << ": "  << endl;
        MPI_Recv(workStruct, structSize, MPI_INT, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD, &status);
        for(int i=0; i<stackSize; i++){
            stack[i] = workStruct[i];
        }
        for(int i=0; i<g->size; i++){
            g->prev[i] = workStruct[i+stackSize];
        }
        delete [] workStruct;
            s.clear();
        for(int i=0; i<stackSize; i+=2){
            edge e;
            e.from = stack[i];
            e.to = stack[i+1];
            s.push_front(e);
        }
        for(int i=0; i<stackSize; i+=2){
            edge e;
            cout << e.from << "->" << e.to << " ";
            e.from = stack[i];
            e.to = stack[i+1];
            s.push_front(e);
        }
        cout << endl;
        delete [] stack;
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
                                    int zero = 0;
 		                            MPI_Isend (&zero, 1, MPI_INT, status.MPI_SOURCE , MSG_WORK_SENT, MPI_COMM_WORLD, &request);
		                         }
		                         else
		                         {
                                    int structSize = w->stackSize + g->size;
                                    int * workStruct = new int[structSize];
                                    int position=0;
                                    MPI_Isend (&(w->stackSize), 1, MPI_INT, status.MPI_SOURCE, MSG_WORK_SIZE, MPI_COMM_WORLD, &request);
                                    cout << "Process " << rank << " sending work of size " << structSize << " to " << status.MPI_SOURCE << endl;
                                    for(int i=0; i<w->stackSize; i++){
                                        workStruct[i] = w->stack[i];
                                    }
                                    for(int i=0; i<g->size; i++){
                                        workStruct[i+w->stackSize] = g->prev[i];
                                    }
                                    //MPI_Pack(w->stack, w->stackSize, MPI_INT, workStruct, structSize, &position, MPI_COMM_WORLD);
                                    //MPI_Pack(g->prev, g->size, MPI_INT, workStruct, structSize, &position, MPI_COMM_WORLD);
                                    MPI_Isend (workStruct, structSize, MPI_INT, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD, &request);
                                    delete [] workStruct;
                                    color = 'B';
		                         }
                                 delete w;
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
                                int * bestPath = new int[bestLength];
                                bestPathToArray(bestPath);
                                cout << "Process " << rank << " was told to finish, sending path of length " << bestLength << endl;
                                MPI_Send (&bestLength, 1, MPI_INT, 0, MSG_TOKEN, MPI_COMM_WORLD);
                                MPI_Send (bestPath, bestLength, MPI_INT, 0, MSG_TOKEN, MPI_COMM_WORLD);
                                delete [] bestPath;
                                MPI_Finalize();
                                exit (0);
                           }
                           else
                           {
                                cout << "Main process was told to finish" << endl;
                                isFinished = true;// nejaky procesor nasel kruznici delky trivialni horni mezi
                           }
                           break;
         default : std::cout << "Neznamy typ zpravy" << endl; break;
      }
}

work* MaxHamilton::getSharableWork(){
   if(!s.empty() && s.back().from == -1){
      edge firstRoot = s.back();
      s.pop_back();
      edge secondRoot = s.back();
      s.push_back(firstRoot);
      if(secondRoot.from != -1) return NULL;
      
      work* workUnit = new work;
      int toShare = 1;//s.size()/2;
      workUnit->stackSize = toShare*2;
      workUnit->stack = new int[toShare*2];
      for(int i = 0; i<toShare; i++){
	      edge rootEdge = s.back();
	      s.pop_back();
	      workUnit->stack[i*2] = rootEdge.from;
	      workUnit->stack[i*2+1] = rootEdge.to;
	}
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

    char blank = ' ';
    if (bestLength == g->size && !foundLimit)
    {
        cout << "Process " << rank << " found best possible length, sending finish message" << endl;
        foundLimit = true;
        MPI_Send(&blank, 1, MPI_CHAR, 0, MSG_FINISH, MPI_COMM_WORLD);
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
