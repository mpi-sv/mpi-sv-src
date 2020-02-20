#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "iostream"
#include <fstream>
#include <string>
#include <list>
#include <utility>
#include "mpise/commlog.h"
using namespace std;
using namespace llvm;
using namespace klee;

struct Location {
	int PID;
	int index;
};

typedef pair<Location, Location> pair1;
typedef list<pair1> returnlist;

/*
 * through analyze the trace of counter-example, return a list of wildcard matching pair
 * */
returnlist* find(string &str);

/*store the file infor to a string str*/
int getFullFile(ifstream &ifs, string &str);
int getResult();
/*
 * through analyze the trace of counter-example, return a list of wildcard matching pair
 * the information of wild card is ax_ax_ax_ax
 * */
returnlist* get_wildcard();

/* dump the information of wild card is ax_ax_ax_ax to the return string */
string get_state_mark(returnlist* lst,std::map<mpi_rank_t, std::vector<CommLogItem *> > & mpicommlog);

/*display the matching from the counter-example  trace*/
void display_wildcard(returnlist * l);

inline bool operator<(Location l1, Location l2){
	return l1.PID< l2.PID || (l1.PID==l2.PID && l1.index < l2.index);
}

inline bool operator==(Location l1, Location l2){
	return l1.PID==l2.PID && l1.index==l2.index;
}
#endif
