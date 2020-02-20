#include "iostream"
#include <fstream>
#include <list>
#include <map>
#include <vector>
#include <string>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include "mpise/commlog.h"
#include "klee/ExecutionState.h"


struct special_Irecv {
	int index;
	int PID;
	CommLogItem * irecv;
};

typedef enum {
	Seq, Parallel
} CSP_operator;


/*used to describe the property that the event P[rank1][index1] happens before P[rank2][index2]*/
struct temporalProperty {
	int rank1;
	int index1;
	int rank2;
	int index2;
};

//mode :guard->action->dom->Skip+CSP_operator
struct cspNode {
	bool hasguard;
	bool hasdom;
	bool haswait;
	bool hasHbChan;
	int guard;
	int dom;
	int wait;
	int hbChan;
	std::vector<std::string> action;
	CSP_operator op;
};

//Process = cspNode1 cspNode2 ...
class CSP_Process {
public:
	std::list<cspNode> actionlist;
	int numOfIsend; //used to new Channel size=1
	int numOfSend; //used to new Channel size=0;
public:
	CSP_Process();
	CSP_Process(std::list<cspNode> templist,int num1, int num2);
	void print(FILE *fp, int i);
};


/*
 * find the nearest isomorphic Isend/ssend behind this Isend
 * used to model the Non-overtaken rules.
 * return the index of this nearest Isend/ssend
 * else return -1
 * */
int findSendBehind(CommLogItem * comm, std::vector<CommLogItem *> vec);


/*
 * find the indices of Irecv/recv, which must happen after the current Irecv.
 * comm: the current irecv; vec: the operation list, dependingSet: result set, used to store the indices
 * case 1: current is Irecv(*), find all Irecv(i), until found Irecv(*)/recv(i)/recv(*)/wait(req),  return size >= 1.
 * note that only add the first, if irecv(i); ... irecv(j) && i==j
 * case 2: current is Irecv(i), find Irecv(i)/Irecv(*)/recv(i)/recv(*)/   until found Irecv(i)/recv(i)/wait(req),   return size >=1
 * case 3: no found, return size==0
 * */
void findRecvBehind(CommLogItem * comm, std::vector<CommLogItem *> vec, std::vector<int>& dependingSet );


/*
 * get the corresponding irecv/isend of wait
 * */
CommLogItem * getWaitTarget(CommLogItem*comm, std::vector<CommLogItem *> vec);


/*get the channel index for ssend/isend
 * type: 0 for Isend, 1 for Ssend
 * */
int get_chanIndex(int index, int type, std::vector<CommLogItem *> vec);


/*
 * invoke PAT to check the csp model
 * */
int callPAT(char* filename);

/*
 * generate the csp model for the global mpi communication
 *  MSPattern: whether we need to consider the Master-Slave pattern
 *  temp: describe the temporal property we want to verify
 *  opt indicates whether we use the optimization for MS modeling
 *  tasks is the number of scheduled tasks
 *    slaves is the number of slaves
 *    opt2 : a label that whether we remove the modeling of master-slave pattern
 * */
void generate_csp(ExecutionState *state, mpicommlog_ty mpicommlog, bool MSPattern, temporalProperty temp, bool opt, int tasks, int slaves, bool opt2);

/*
 * Background: we believe that the smallest index of a possible matching
 * must be bigger than the last barrier before the issuance of the wild.
 * return 0 if there is no barrier before the issuance of wild, otherwise return the number of barriers
 * */
int get_upperBound(CommLogItem * wild, mpicommlog_ty mpicommlog);

/*
 * Background: we believe that the biggest index of a possible matching
 * must be smaller than the first barrier after the issuance of the wait of the wild
 * return 0 if there is no barrier after the wait of the wild
 * otherwise return how many barriers before issuance of wait of wildcard receive
 * note that this only works for irecv
 * */
int get_lowerBound(CommLogItem * wild, mpicommlog_ty mpicommlog);

/*
 * get the candidates set for a wildcard recv/irecv
 * compared with blindly get the static matching, we use upper and lower bound to refine the scope.
 * in this function, we also want to ensure the channel reading order of those candidates in the same process. Using Sij!j and Sij?j where i is rank, j is index.
 * */
void get_refinedCandidateSet(CommLogItem * wild, mpicommlog_ty mpicommlog, set<CommLogItem *> * result, int upper, int lower);

/*
 get the number of wildcard before this irecv
 * */
int hasPrevwild( CommLogItem* item  , mpicommlog_ty mpicommlog);

/*
 *get the candidates set of a deterministic irecv/recv
 * it is recv/irecv, index is from its dynamic matching send
 * */
void  getCandidates(CommLogItem* it,int index,  int count, mpicommlog_ty mpicommlog, set<CommLogItem*> & result );

/*check whether a Schan has already added to the extern list*/
bool containSchan(list<Location> tempList, Location temp);

/* generate a log file for transformation, used by weijiang*/
void print2(FILE *fp, mpicommlog_ty mpicommlog);

