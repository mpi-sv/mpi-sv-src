#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <vector>
#include <string>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include "mpise/generate.h"
#include "mpise/solution.h"
#include "mpise/analysis.h"
//----added
#include "mpise/commlog.h"
#include "klee/ExecutionState.h"
#include "klee/klee.h"
#include "klee/Executor.h"
#include <llvm/Support/CommandLine.h>

using namespace llvm;
using namespace klee;
using namespace std;

extern cl::opt<std::string> LTLFile;
cl::opt<std::string>
        ModelGenerationPath("model-generation-path",
                            cl::desc("Path to store the exported models"));


CSP_Process::CSP_Process() : numOfIsend(0), numOfSend(0) {
}

CSP_Process::CSP_Process(std::list<cspNode> templist, int num1, int num2) :
        numOfIsend(num1), numOfSend(num2) {
    std::list<cspNode>::iterator it;
    actionlist = *(new std::list<cspNode>());
    for (it = templist.begin(); it != templist.end(); it++) {
        actionlist.push_back(*it);
    }
}

/*
 * find the nearest isomorphic Isend/ssend behind this Isend
 * return the index of this nearest Isend/ssend
 * else return -1
 * */
int findSendBehind(CommLogItem *comm, std::vector<CommLogItem *> vec) {
    std::vector<CommLogItem *>::iterator it_vec = vec.begin();
    while (it_vec != vec.end()) {
        if (((*it_vec)->mpi_op == COMM_ISEND || (*it_vec)->mpi_op == COMM_SSEND)
            && (*it_vec)->index > comm->index && dyn_cast<klee::ConstantExpr>(
                (*it_vec)->arguments[3])->getZExtValue() == dyn_cast<klee::ConstantExpr>(
                comm->arguments[3])->getZExtValue()) {
            return (*it_vec)->index;
        }
        it_vec++;
    }
    return -1;
}

/*
 * YHB
 * find the indices of Irecv/recv, which must happen after the current Irecv.
 * comm: the current irecv; vec: the operation list, dependingSet: result set, used to store the indices
 * case 1: current is Irecv(*), find all Irecv(i), until found Irecv(*)/recv(i)/recv(*)/wait(req),  return size >= 1. note that only add the first, if irecv(i); ... irecv(j) && i==j
 * case 2: current is Irecv(i), find Irecv(i)/Irecv(*)/recv(i)/recv(*)/   until found Irecv(i)/recv(i)/wait(req),   return size >=1
 * case 3: no found, return size==0
 * */
void findRecvBehind(CommLogItem *comm, std::vector<CommLogItem *> vec, std::vector<int> &dependingSet) {
    //comm is a wildcard receive itself, then we will find the nearest recv
    if (comm->wildcard_rewrite) {
        std::set<int> added;
        std::vector<CommLogItem *>::iterator it_vec = vec.begin();
        while (it_vec != vec.end()) {
            if ((*it_vec)->wildcard_rewrite && (*it_vec)->index > comm->index) {
                dependingSet.push_back((*it_vec)->index);
                break;
            }
            if ((*it_vec)->mpi_op == COMM_WAIT && dyn_cast<
                    klee::ConstantExpr>(comm->arguments[6])->getZExtValue() == dyn_cast<
                    klee::ConstantExpr>((*it_vec)->arguments[0])->getZExtValue()) {
                break;
            }
            if ((*it_vec)->index > comm->index && (*it_vec)->mpi_op == COMM_RECV) {
                dependingSet.push_back((*it_vec)->index);
                break;
            }
            if ((*it_vec)->index > comm->index && (*it_vec)->mpi_op == COMM_IRECV &&
                added.find((*it_vec)->index) != added.end()) {
                dependingSet.push_back((*it_vec)->index);
                added.insert((*it_vec)->index);
            }
            it_vec++;
        }
    } else {
        std::vector<CommLogItem *>::iterator it_vec = vec.begin();
        while (it_vec != vec.end()) {
            if ((*it_vec)->index > comm->index && ((*it_vec)->mpi_op == COMM_IRECV || (*it_vec)->mpi_op == COMM_RECV)
                && (*it_vec)->index > comm->index && dyn_cast<klee::ConstantExpr>(
                    (*it_vec)->arguments[3])->getZExtValue() == dyn_cast<klee::ConstantExpr>(
                    comm->arguments[3])->getZExtValue()) {
                dependingSet.push_back((*it_vec)->index);
                break;
            }
            if ((*it_vec)->wildcard_rewrite &&
                (*it_vec)->index > comm->index) { // conditional happens-before : Irecv(1)  .... Irecv(*)/recv(*)
                dependingSet.push_back((*it_vec)->index);
            }
            if ((*it_vec)->mpi_op == COMM_WAIT && dyn_cast<
                    klee::ConstantExpr>(comm->arguments[6])->getZExtValue() == dyn_cast<
                    klee::ConstantExpr>((*it_vec)->arguments[0])->getZExtValue()) {
                break;
            }
            it_vec++;
        }
    }
}

/*
 * get the corresponding irecv/isend of wait
 * */
CommLogItem *getWaitTarget(CommLogItem *comm, std::vector<CommLogItem *> vec) {
    for (int index = comm->index - 1; index >= 0; index--) {
        if (((vec[index])->mpi_op == COMM_IRECV || (vec[index])->mpi_op == COMM_ISEND) && dyn_cast<
                klee::ConstantExpr>((vec[index])->arguments[6])->getZExtValue() == dyn_cast<
                klee::ConstantExpr>(comm->arguments[0])->getZExtValue()) {
            return vec[index];
        }
    }
}


/*get the channel index for ssend/isend
 * type: 0 for Isend, 1 for Ssend
 * */
int get_chanIndex(int index, int type, std::vector<CommLogItem *> vec) {
    int chanIndex = 0;
    std::vector<CommLogItem *>::iterator it_vec = vec.begin();
    for (int i = 0; i < index; i++) {
        if (type == 0) {
            if ((*it_vec)->mpi_op == COMM_ISEND) {
                chanIndex++;
            }
        } else {
            if ((*it_vec)->mpi_op == COMM_SSEND) {
                chanIndex++;
            }
        }
        it_vec++;
    }
    return chanIndex;
}

//dump process i to a given file
void CSP_Process::print(FILE *fp, int i) {
    int count = 0; // num of ( == num of )
    char str_temp2[25];
    sprintf(str_temp2, "%d", i); //Pi=.....
    fprintf(fp, "%s", "P");
    fprintf(fp, "%s", str_temp2);
    fprintf(fp, "%s", "=");
    std::list<cspNode>::iterator it = this->actionlist.begin();
    while (it != this->actionlist.end()) // print the actionlist
    {
        std::vector<std::string>::iterator label = (it->action).begin();
        //deal one node
        fprintf(fp, "%s", "(");
        if (it->hasguard) {
            LOG(INFO) << "has guard" << endl;
            sprintf(str_temp2, "%d", it->guard);
            fprintf(fp, "%s", "ac");
            fprintf(fp, "%s", str_temp2);
            fprintf(fp, "%s", "->");
        }
        while (label != (it->action).end()) {
            fprintf(fp, "%s", (char *) ((*label).c_str()));
            label++;
        }
        if (it->hasdom) {
            LOG(INFO) << "has dom" << endl;
            sprintf(str_temp2, "%d", it->dom);
            fprintf(fp, "%s", "->");
            fprintf(fp, "%s", "ac");
            fprintf(fp, "%s", str_temp2);
            fprintf(fp, "%s", "->Skip");
        }
        fprintf(fp, "%s", ")");

        // special operations for the last
        std::list<cspNode>::iterator temp_test = it;
        std::list<cspNode>::iterator ltest = ++it;
        if (ltest != this->actionlist.end()) {
            switch (temp_test->op) {
                case Seq:
                    //fprintf(fp, "%s", ";");
                    fprintf(fp, "%s", ";(");
                    count++;
                    break;
                case Parallel:
                    fprintf(fp, "%s", "||");
                    break;
            }
        }
    }
    while (count > 0) {
        fprintf(fp, "%s", ")");
        count--;
    }
    if ((this->actionlist).size() == 0) // the opt2 can cause this situation
        fprintf(fp, "%s", "Skip;\n");
    else
        fprintf(fp, "%s", ";\n");
}

//dump process i to a given file for transformation
void print2(FILE *fp, mpicommlog_ty mpicommlog) {

    char str_temp2[25];
    char des[10];
    char messageTag[10];
    char handlerStr[20];
    int process_num = mpicommlog.size();
    for (int i = 0; i < process_num; i++) {
        sprintf(str_temp2, "%d", i); //Pi=.....
        fprintf(fp, "%s", "P");
        fprintf(fp, "%s", str_temp2);
        fprintf(fp, "%s", "=");
        std::vector<CommLogItem *>::iterator it = mpicommlog[i].begin();
        while (it != mpicommlog[i].end()) {
            switch ((*it)->mpi_op) {
                case (COMM_BARR): {
                    fprintf(fp, "%s", "Barrier;");
                    break;
                }
                case (COMM_SSEND): {
                    fprintf(fp, "%s", "Ssend(");
                    int dest = dyn_cast<klee::ConstantExpr>((*it)->arguments[3])->getSExtValue();
                    int send_tag = dyn_cast<klee::ConstantExpr>((*it)->arguments[4])->getSExtValue();
                    sprintf(des, "%d", dest);
                    sprintf(messageTag, "%d", send_tag);
                    fprintf(fp, "%s", des);
                    fprintf(fp, "%s", ",");
                    fprintf(fp, "%s", messageTag);
                    fprintf(fp, "%s", ");");
                    break;
                }
                case (COMM_ISEND): {
                    fprintf(fp, "%s", "ISend(");
                    int dest = dyn_cast<klee::ConstantExpr>((*it)->arguments[3])->getSExtValue();
                    int send_tag = dyn_cast<klee::ConstantExpr>((*it)->arguments[4])->getSExtValue();
                    int handler = dyn_cast<klee::ConstantExpr>((*it)->arguments[6])->getSExtValue();
                    sprintf(des, "%d", dest);
                    sprintf(messageTag, "%d", send_tag);
                    sprintf(handlerStr, "%d", handler);
                    fprintf(fp, "%s", des);
                    fprintf(fp, "%s", ",");
                    fprintf(fp, "%s", messageTag);
                    fprintf(fp, "%s", ",");
                    fprintf(fp, "%s", handlerStr);
                    fprintf(fp, "%s", ");");
                    break;
                }
                case (COMM_SEND): {
                    fprintf(fp, "%s", "Send(");
                    int dest = dyn_cast<klee::ConstantExpr>((*it)->arguments[3])->getSExtValue();
                    int send_tag = dyn_cast<klee::ConstantExpr>((*it)->arguments[4])->getSExtValue();
                    sprintf(des, "%d", dest);
                    sprintf(messageTag, "%d", send_tag);
                    fprintf(fp, "%s", des);
                    fprintf(fp, "%s", ",");
                    fprintf(fp, "%s", messageTag);
                    fprintf(fp, "%s", ");");
                    break;
                }
                case (COMM_WAIT): {
                    fprintf(fp, "%s", "Wait(");
                    int tag = dyn_cast<klee::ConstantExpr>((*it)->arguments[0])->getSExtValue();
                    sprintf(des, "%d", tag);
                    fprintf(fp, "%s", des);
                    fprintf(fp, "%s", ");");
                    break;
                }

                case (COMM_RECV): {
                    fprintf(fp, "%s", "Recv(");
                    int src = dyn_cast<klee::ConstantExpr>((*it)->arguments[3])->getSExtValue();
                    int recv_tag = dyn_cast<klee::ConstantExpr>((*it)->arguments[4])->getSExtValue();
                    sprintf(messageTag, "%d", recv_tag);
                    if ((*it)->isWildcard(true)) {
                        fprintf(fp, "%s", "*,");
                        if (recv_tag == ANY_TAG)
                            fprintf(fp, "%s", "*);");
                        else {
                            fprintf(fp, "%s", messageTag);
                            fprintf(fp, "%s", ");");
                        }
                    } else {
                        sprintf(des, "%d", src);
                        fprintf(fp, "%s", des);
                        fprintf(fp, "%s", ",");
                        if (recv_tag == ANY_TAG)
                            fprintf(fp, "%s", "*);");
                        else {
                            fprintf(fp, "%s", messageTag);
                            fprintf(fp, "%s", ");");
                        }
                    }
                    break;
                }
                case (COMM_IRECV): {
                    fprintf(fp, "%s", "IRecv(");
                    int src = dyn_cast<klee::ConstantExpr>((*it)->arguments[3])->getSExtValue();
                    int recv_tag = dyn_cast<klee::ConstantExpr>((*it)->arguments[4])->getSExtValue();
                    if ((*it)->isWildcard(true)) {
                        fprintf(fp, "%s", "*,");
                        if (recv_tag == ANY_TAG)
                            fprintf(fp, "%s", "*,");
                        else {
                            fprintf(fp, "%s", messageTag);
                            fprintf(fp, "%s", ",");
                        }
                    } else {
                        sprintf(des, "%d", src);
                        fprintf(fp, "%s", des);
                        fprintf(fp, "%s", ",");
                        if (recv_tag == ANY_TAG)
                            fprintf(fp, "%s", "*,");
                        else {
                            fprintf(fp, "%s", messageTag);
                            fprintf(fp, "%s", ",");
                        }
                    }
                    int tag = dyn_cast<klee::ConstantExpr>((*it)->arguments[6])->getZExtValue();
                    sprintf(des, "%d", tag);
                    fprintf(fp, "%s", des);
                    fprintf(fp, "%s", ");");
                    break;
                }
            }
            it++;
        }
        fprintf(fp, "%s", "\n");
    }
}

int global_model_counter = 1;

/*invoke PAT to verify the csp model stored */
int callPAT(char *filename) {
    char command[128];
    char buf[1024];
    //sprintf(command, "%s", "mono ~/pat/PAT3.Console.exe -csp ~/CSP_Model.csp ~/PAT_Result.txt");
    string pwd = getpwuid(getuid())->pw_dir;
    int pid = getpid();
    char temp[10];
    sprintf(temp, "%d", pid);

    if (ModelGenerationPath.getValue().empty() == false) {
        pwd = ModelGenerationPath.getValue();
    }

    string name1 = pwd + "/PAT_Result";
    //string  name2="PAT3.Console.exe -csp ";
    string name2 = "mono ~/pat/PAT3.Console.exe -csp ";
    char count[10];
    sprintf(count, "%d", global_model_counter);
    string result = name1 + temp + "_" + count + ".txt";

    string comd = name2 + filename + " " + result;
    strcpy(command, comd.c_str());


//sprintf(command, "%s", "PAT3.Console.exe -csp ~/CSP_Model.csp ~/PAT_Result.txt");
    // llvm::errs() << "----------------start------------------------------\n";
    //sprintf(command, "%s", "mono ~/pat/PAT3.Console.exe -csp ~/CSP_Model.csp ~/PAT_Result.txt");
    //"/usr/local/share/PAT331/\"./PAT3.Console.exe\" -csp /home/mpiklee/test_new.csp /home/mpiklee/result_new.txt");
    FILE *fd;
    if ((fd = popen(command, "r")) == NULL) {
        perror("popen failed!");
        return -1;
    }
    if (fgets(buf, sizeof(buf), fd) == NULL) {
        printf("No output is given by PAT3\n");
    } else {
        //printf("PAT3 output: %s\n", buf);
    }
    if (pclose(fd) == -1) {
        perror("pclose failed!");
        return -2;
    }
    //invoke the command to extract the cost of model checking,e.g. time, memory
//	int pid2=getpid();
//	char temp2[10];
//	sprintf(temp2,"%d",pid2);
//	string name11="~/extract ";
//	sprintf(count, "%d", global_model_counter);
//	string Extracting=name11+temp2 + " " + count;
//	int len=strlen(Extracting.c_str())+1;
//	char commandExtract[len+1];
//	LOG(INFO)<<Extracting;
//	strcpy( commandExtract,Extracting.c_str() );
//	system(commandExtract);
    //llvm::errs() << "----------------end------------------------------\n";
    return 0;
}

/*
 * Background: we believe that the smallest index of a possible matching
 * must be bigger than the last barrier before the issuance of the wild.
 * return 0 if there is no barrier before the issuance of wild, otherwise return the number of barriers
 * */
int get_upperBound(CommLogItem *wild, mpicommlog_ty mpicommlog) {
    //we want to find how many barriers issue before the issuance of wildcard receive
    int counter = 0;
    std::vector<CommLogItem *>::iterator it_vec = mpicommlog[wild->rank].begin();
    while (it_vec != mpicommlog[wild->rank].end() && (*it_vec)->index < wild->index) {
        if ((*it_vec)->mpi_op == COMM_BARR)
            counter++;
        it_vec++;
    }
    return counter;
}

/*
 * Background: we believe that the biggest index of a possible matching
 * must be smaller than the first barrier after the issuance of the wait of the wild
 * return 0 if there is no barrier after the wait of the wild
 * otherwise return how many barriers before issuance of wait of wildcard receive
 * note that this only works for irecv
 * */
int get_lowerBound(CommLogItem *wild, mpicommlog_ty mpicommlog) {
    int counter = 0;
    std::vector<CommLogItem *>::iterator it_vec = mpicommlog[wild->rank].begin();
    while (it_vec != mpicommlog[wild->rank].end()) {
        if ((*it_vec)->mpi_op == COMM_WAIT
            && dyn_cast<klee::ConstantExpr>((*it_vec)->arguments[0])->getZExtValue()
               == dyn_cast<klee::ConstantExpr>(wild->arguments[6])->getZExtValue()) {
            break;
        }
        if ((*it_vec)->mpi_op == COMM_BARR)
            counter++;
        it_vec++;
    }
    return counter;
}

/*
 * get the candidates set for a wildcard recv/irecv
 * compared with blindly get the static matching, we use upper and lower bound to refine the scope.
 * in this function, we also want to ensure the channel reading order of those candidates in the same process. Using Sij!j and Sij?j where i is rank, j is index.
 * */
void get_refinedCandidateSet(CommLogItem *wild, mpicommlog_ty mpicommlog, vector<CommLogItem *> *result, int lower) {
    int size = mpicommlog.size();
    int findJ = -1;
    for (int j = 0; j < size; j++) {
        int count = 0;
        findJ = -1;
        if (j != wild->rank) { // scan all the other processes
            //first we need to count how many recv(i) or Irecv(i) before this one
            int m = 0;
            int n = 0; // m count Irecv(i)/recv(i) , while n count Irecv(*)/recv(*) before the current recv/irecv
            for (int i = 0; i < wild->index; i++) {
                if ((mpicommlog[wild->rank][i])->mpi_op == COMM_RECV
                    || (mpicommlog[wild->rank][i])->mpi_op == COMM_IRECV) {
                    if (!(mpicommlog[wild->rank][i])->isWildcard(true) &&
                        !(mpicommlog[wild->rank][i])->recvFM &&
                        !(mpicommlog[wild->rank][i])->recvOfMaster == 0
                        && dyn_cast<klee::ConstantExpr>(
                            (mpicommlog[wild->rank][i])->arguments[3])->getZExtValue() == j
                        && (dyn_cast<klee::ConstantExpr>(
                            (mpicommlog[wild->rank][i])->arguments[4])->getZExtValue()
                            == dyn_cast<klee::ConstantExpr>(wild->arguments[4])->getZExtValue()
                            || dyn_cast<klee::ConstantExpr>(
                            (mpicommlog[wild->rank][i])->arguments[4])->getZExtValue()
                               == ANY_TAG || dyn_cast<klee::ConstantExpr>(
                            wild->arguments[4])->getZExtValue() == ANY_TAG)) //ANY_TAG
                        m++;
                    //second we need to count how many recv(*) or Irecv(*) before this one
                    if ((mpicommlog[wild->rank][i])->isWildcard(true)
                        && (dyn_cast<klee::ConstantExpr>(
                            (mpicommlog[wild->rank][i])->arguments[4])->getZExtValue()
                            == dyn_cast<klee::ConstantExpr>(wild->arguments[4])->getZExtValue()
                            || dyn_cast<klee::ConstantExpr>(
                            (mpicommlog[wild->rank][i])->arguments[4])->getZExtValue()
                               == ANY_TAG || dyn_cast<klee::ConstantExpr>(
                            wild->arguments[4])->getZExtValue() == ANY_TAG))
                        n++;
                    if ((mpicommlog[wild->rank][i])->recvFM || ((mpicommlog[wild->rank][i])->recvOfMaster == 0 &&
                                                                !(mpicommlog[wild->rank][i])->isWildcard(true)))
                        n++;
                }
            }
            int counter = 0;
            std::vector<CommLogItem *>::iterator it_vec = mpicommlog[j].begin();
            while (it_vec != mpicommlog[j].end()) {
                if ((*it_vec)->mpi_op == COMM_BARR) {
                    count++;
                    if (count == lower)
                        break;
                }
                if (/*count>=upper &&*/ ((*it_vec)->mpi_op == COMM_SSEND || (*it_vec)->mpi_op == COMM_ISEND)
                                        && dyn_cast<klee::ConstantExpr>((*it_vec)->arguments[3])->getZExtValue() ==
                                           ((unsigned long) wild->rank) &&
                                        (dyn_cast<klee::ConstantExpr>((*it_vec)->arguments[4])->getZExtValue() ==
                                         dyn_cast<klee::ConstantExpr>(wild->arguments[4])->getZExtValue()
                                         ||
                                         dyn_cast<klee::ConstantExpr>(wild->arguments[4])->getZExtValue() == ANY_TAG)) {
                    counter++;
                    if (counter > m && counter <= m + n + 1) {
                        /*we only need to ensure the reading order of Isend, not Ssend*/
                        if ((*it_vec)->mpi_op == COMM_ISEND) {
                            if (findJ >= 0) {
                                (*it_vec)->NB = findJ; // means it need guard for index findJ is added
                                mpicommlog[j][findJ]->NA = true; // means index findJ need dom
                            }
                            findJ = get_chanIndex((*it_vec)->index, 0, mpicommlog[j]);
                        }
                        (*result).push_back(*it_vec);
                    }
                }
                it_vec++;
            }
        }
    }
}

/*
 get the number of wildcard before this irecv
 * */
int hasPrevwild(CommLogItem *item, mpicommlog_ty mpicommlog) {
    int count = 0;
    int rankId = item->rank;
    std::vector<CommLogItem *>::iterator it = mpicommlog[rankId].begin();
    while (it != mpicommlog[rankId].end()) {
        if ((*it)->isWildcard(true) && (*it)->index < item->index) {
            count++;
        } else {
            if ((*it)->index >= item->index) {
                break;
            }
        }
        it++;
    }
    return count;
}

/*
 *get the candidates set of a deterministic irecv/recv
 * it is recv/irecv, index is from its dynamic matching send
 * */
void getCandidates(CommLogItem *it, int index, int count, mpicommlog_ty mpicommlog, vector<CommLogItem *> &result) {
    int rankId = dyn_cast<klee::ConstantExpr>(it->arguments[3])->getZExtValue();
    int counter1 = count;
    int indexOfSend = index;
    vector<CommLogItem *> *temp = new vector<CommLogItem *>();
    // first: find the matching before current match
    //while(it_vec!= mpicommlog[rankId].begin() && counter1!=0){
    if (index != 0) {
        for (indexOfSend = index - 1; indexOfSend >= 0 && counter1 >= 0; indexOfSend--) {
            if (mpicommlog[rankId][indexOfSend]->mpi_op == COMM_ISEND
                || mpicommlog[rankId][indexOfSend]->mpi_op == COMM_SSEND) {
                LOG(INFO)
                        << dyn_cast<klee::ConstantExpr>(mpicommlog[rankId][indexOfSend]->arguments[4])->getZExtValue();
                LOG(INFO) << dyn_cast<klee::ConstantExpr>(it->arguments[4])->getZExtValue();
                if ((long) dyn_cast<klee::ConstantExpr>(mpicommlog[rankId][indexOfSend]->arguments[3])->getZExtValue()
                    == it->rank && dyn_cast<klee::ConstantExpr>(
                        mpicommlog[rankId][indexOfSend]->arguments[4])->getZExtValue() == dyn_cast<
                        klee::ConstantExpr>(it->arguments[4])->getZExtValue()) {
                    counter1--;
                    result.push_back(mpicommlog[rankId][indexOfSend]);
                }
            }
        }
    }
    vector<CommLogItem *>::iterator it_V = (*temp).end();
    while (it_V != (*temp).begin()) {
        it_V--;
        result.push_back(*(it_V));
    }
    delete temp;
    // also need to add the current one.
    result.push_back(mpicommlog[rankId][index]);
    // second: find the matching behind current match
    if (index != mpicommlog[rankId].size() - 2) {
        for (indexOfSend = index + 1; indexOfSend < mpicommlog[rankId].size() && count >= 0; indexOfSend++) {
            if (mpicommlog[rankId][indexOfSend]->mpi_op == COMM_ISEND
                || mpicommlog[rankId][indexOfSend]->mpi_op == COMM_SSEND) {
                if ((long) dyn_cast<klee::ConstantExpr>(mpicommlog[rankId][indexOfSend]->arguments[3])->getZExtValue()
                    == it->rank && dyn_cast<klee::ConstantExpr>(
                        mpicommlog[rankId][indexOfSend]->arguments[4])->getZExtValue() == dyn_cast<
                        klee::ConstantExpr>(it->arguments[4])->getZExtValue()) {
                    count--;
                    result.push_back(mpicommlog[rankId][indexOfSend]);
                }
            }
        }
    }
    LOG(INFO) << "size :" << result.size();
    // third, we need to ensure the reading order of these candidates
    if (result.size() > 1) {
        int *p = new int[result.size()]; //we sort the index
        vector<CommLogItem *>::iterator it = result.begin();
        int i = 0;
        //INITIALZATION
        while (it != result.end()) {
            p[i] = (*it)->index;
            i++;
            it++;
        }
        //SORTING
        for (int j = 0; j < result.size(); j++)
            for (int l = 0; l < result.size() - j; l++) {
                if (p[l] > p[l + 1]) // swap
                {
                    int temp = p[l];
                    p[l] = p[l + 1];
                    p[l + 1] = temp;
                }
            }
        // LABELING
        for (int j = 0; j < result.size(); j++) {
            if (j == 0)
                mpicommlog[rankId][p[j]]->NA = true;
            else {
                if (j != result.size() - 1) {
                    mpicommlog[rankId][p[j]]->NA = true;
                    mpicommlog[rankId][p[j]]->NB = get_chanIndex(p[j - 1], 0, mpicommlog[rankId]);
                } else
                    mpicommlog[rankId][p[j]]->NB = get_chanIndex(p[j - 1], 0, mpicommlog[rankId]);
            }
        }
    }
}

/*check whether a Schan has already added to the extern list*/
bool containSchan(list<Location> tempList, Location temp) {
    list<Location>::iterator it = tempList.begin();
    while (it != tempList.end()) {
        if ((*it).PID == temp.PID && (*it).index == temp.index) {
            return true;
        }
        it++;
    }
    return false;
}


/*
 * current is an Isend, return true if it has a wait, otherwise return false
 * */
bool testIsend(CommLogItem *current, mpicommlog_ty mpicommlog) {
    int rank = current->rank;
    int index = current->index;
    bool haswait = false;
    for (; index < mpicommlog[rank].size(); index++) {
        if (mpicommlog[rank][index]->mpi_op == COMM_WAIT &&
            dyn_cast<klee::ConstantExpr>(current->arguments[6])->getZExtValue()
            == dyn_cast<klee::ConstantExpr>(mpicommlog[rank][index]->arguments[0])->getZExtValue()) {
            haswait = true;
            break;
        }
    }
    return haswait;
}


/*YU
 * mpicommlog_ty is a map <rankId, vector<mpiops>>
 * input the global communication.
 * output the corresponding csp model
 * Four kinds of channel.
 * C-->size 0, model Ssend;
 * D-->size 1, model Isend;
 * H-->size 1, model happens-before between Irecv/recv;
 * F-->size 1, model the wait status of Isend
 * if there exists Master-Slave Pattern(indicated by the second parameter) in the communication, we need to extern 4 types:
1. 5 slave/notify channels, 10 task channels, and 5 label variables. These auxiliary types can be reused for multiple MS patterns
last 3 arguments are used for MS optimization.
2.  opt2 means we skip the modeling of master-slave pattern.
 * */

std::map<std::string, std::set<std::string> > opChanOpMap;

extern cl::opt<std::string> LTLFile;

/**
 * Add send-recv to channel action mappings
 */
void insertOpChanOp(CommLogItem &recvItem, CommLogItem &sendItem, char *pid, char *cid) {
    if (LTLFile.size() <= 0) return;
    stringstream recvEvent, sendEvent, channelRecvStream, channelSendStream;

    if (sendItem.mpi_op == COMM_ISEND) {
        sendEvent << "Isend(";
        channelSendStream << "D";
    } else if (sendItem.mpi_op == COMM_SSEND) {
        sendEvent << "Ssend(";
        channelSendStream << "C";
    }

    if (recvItem.mpi_op == COMM_RECV) {
        recvEvent << "Recv(";
    } else if (recvItem.mpi_op == COMM_IRECV) {
        recvEvent << "Irecv(";
    }


    /// generate the mapping relation
    recvEvent << recvItem.rank << "," << sendItem.rank << ")";
    sendEvent << sendItem.rank << "," << recvItem.rank << ")";
    channelSendStream << pid << "_" << cid;
    /// copy channel name to recv channel
    channelRecvStream << channelSendStream.str();
    channelSendStream << "!" << cid;
    channelRecvStream << "?" << cid;

    /// add them to the map
    opChanOpMap[recvEvent.str()].insert(channelRecvStream.str());
    opChanOpMap[sendEvent.str()].insert(channelSendStream.str());
}

void generate_csp(ExecutionState *state, mpicommlog_ty mpicommlog, bool MSPattern, temporalProperty temp, bool opt,
                  int tasks, int slaves, bool opt2) {
    int Rflag = -1; // 1 means as first, 2 for second, which are used as temporal property checking for recvs!
    int dcounter = 0;// used for the double recev MS pattern
    //char firstEvent[20]; // used for temporal property;
    std::vector<std::string> firstEvent; // used for temporal property;
    std::vector<std::string> secondEvent; // used for temporal property;
    //char secondEvent[20]; //used for temporal property;

    list<int *> *result = new list<int *>();
    if (opt) { // compute the solutions for optimizing the modeling of master-slave
        g(tasks, slaves, result);
    }

    int lastNRecv = 0; // a counter used for modeling the optimization of MS pattern.

    bool needRecursion = false; // a label for handling MSPattern.
    list<cspNode> *recursionP = new list<cspNode>(); // used for storing the recursion processes.

    LOG(INFO) << "Genetate_csp begin" << endl;
    int Process_num = mpicommlog.size();
    int *counterPP = new int[Process_num]; // used in Master-Slave Pattern, counterPP[i] means the number of encoutered slave patterns.
    for (int i = 0; i < Process_num; i++)
        counterPP[i] = 0;

    /*a global integer used to initialize the guard/dom of Isend(ac_hap); guard fo Ssend(ac_hap); wait of Irecv(wait_hap).   It increase by one*/
    int hap = 0;
    CSP_Process *pro = new CSP_Process[Process_num];
    cspNode *temp_Node; //csp operation of current mpi event
    std::list<cspNode> *temp_list; // the event list of this process, used to construct a CSP_Process
    bool need_deal_B = false; // if a process has no barrier ,while other processes has barrier, then we need to add 'b' to the event list of this process.
    /* used to store the send and recv info of  a wildcard receive, since we need to get the rewrite info of the wildcard receive from a couter-example */
    char Send_Id[10];
    char Send_Index[10];
    char Recv_Id[10];
    char Recv_Index[10];
    /*used to indicate whether Process[i] has a barrier, otherwise we need to declare 'B' as its synchronization event*/
    bool *hasBarrier = new bool[Process_num];
    for (int j = 0; j < Process_num; j++)
        hasBarrier[j] = false;
    for (int i = 0; i < Process_num; i++) {
        if (i == 1)
            LOG(INFO) << "GOt it";
        /*used to count the number total used C and D channels, since we need to extern these channels for the model*/
        int numOfSend = 0;
        int numOfIsend = 0;
        temp_list = new std::list<cspNode>();
        /* For Isend/Ssend, findGuard[j]=k means location j must happens after some operation in location i,  whose dom is ack!
         *  However, for  a Irecv(*)/Irecv(i), it  may depend on many Irecv(i)/Irecv(*),  so find[j]={i,...}, where j is the index of current Irecv, i is the index of irecvs must happen before this one */
        list<int> *findGuard = new list<int>[mpicommlog[i].size()];
        for (int j = 0; j < mpicommlog[i].size(); j++) {
            std::list<int> *tempList = new list<int>();
            findGuard[j] = *tempList;
        }
        /* findWait[j]=k means index j is a Irecv, and we use k to represent its wait, because we use another chan reading for Isend */
        int *findWait = new int[mpicommlog[i].size()];
        for (int j = 0; j < mpicommlog[i].size(); j++)
            findWait[j] = -1;
        std::vector<CommLogItem *>::iterator it = mpicommlog[i].begin();
        while (it != mpicommlog[i].end()) {
            char NumofChan[10]; //use to j of Cij/Dij
            char ProcessID[10];//use to i of Cij/Dij
            switch ((*it)->mpi_op) {
                case (COMM_BARR): {
                    temp_Node = new cspNode();
                    temp_Node->hasdom = false;
                    temp_Node->hasguard = false;
                    temp_Node->haswait = false;
                    temp_Node->hasHbChan = false;
                    temp_Node->action.push_back("B->Skip");
                    temp_Node->op = Seq;
                    temp_list->push_back(*temp_Node);
                    it++;
                    hasBarrier[i] = (hasBarrier[i] == false) ? true : false;
                    need_deal_B = (need_deal_B == false) ? true : false;
                    break;
                }
                case (COMM_SSEND): {
                    // if opt2 is enabled, we skip the modeling of master-slave pattern
                    if (((*it)->sendOfSlave || (*it)->sendFS || (*it)->notifySend || (*it)->sendOfMaster) && opt2) {
                        it++;
                        break;
                    }

                    if ((*it)->sendOfSlave ||
                        (*it)->sendFS) { // special handling for MSPattern, model it together with its binding recv
                        if (!(*it)->filtered && !opt && !opt2)
                            numOfSend++;
                        it++;
                        break;
                    }
                    temp_Node = new cspNode();
                    temp_Node->hasdom = false;
                    temp_Node->hasguard = false;
                    temp_Node->haswait = false;
                    temp_Node->hasHbChan = false;
                    sprintf(NumofChan, "%d", get_chanIndex((*it)->index, 1, mpicommlog[i]));
                    sprintf(ProcessID, "%d", i);
                    temp_Node->action.push_back("C");
                    temp_Node->action.push_back(ProcessID);
                    temp_Node->action.push_back("_");
                    temp_Node->action.push_back(NumofChan);
                    temp_Node->action.push_back("!");
                    temp_Node->action.push_back(NumofChan);
                    temp_Node->action.push_back("->Skip");
                    temp_Node->op = Seq;
                    temp_list->push_back(*temp_Node);
                    numOfSend++;
                    it++;
                    break;
                }
                case (COMM_RECV): {
                    if ((*it)->recvOfMaster == 0 && (*it)->isWildcard(true))
                        dcounter++;
                    if (Rflag > 0)
                        Rflag = 0;
                    // added for verify temproal property between send operations
                    if ((*it)->rank == temp.rank1 && (*it)->index == temp.index1) {
                        Rflag = 1;
                    }
                    if ((*it)->rank == temp.rank2 && (*it)->index == temp.index2) {
                        Rflag = 2;
                    }
                    if ((*it)->filtered) { //used in MSPattern
                        it++;
                        break;
                    }

                    // if opt2 is enabled, we do not model the master-slave pattern
                    if (((*it)->recvOfMaster >= 0 || (*it)->recvOfSlave >= 0) && opt2) {
                        it++;
                        break;
                    }

                    if ((*it)->recvOfMaster > 1 && opt) { // for the optimization mode, filter the following ones.
                        it++;
                        break;
                    }

                    /*an indicator: whether it's the pattern: recv(*)/recv(i) which has different H channels for different candidates*/
                    bool flagMulHb = true;
                    temp_Node = new cspNode();
                    temp_Node->hasdom = false;
                    temp_Node->hasguard = false;
                    temp_Node->haswait = false;
                    temp_Node->hasHbChan = false;
                    if (findGuard[(*it)->index].size() ==
                        0) //find the depending information, which are computed on the fly!
                        flagMulHb = false; // no need for H channel reading
                    /*two cases:
                     * recv(i): size of its findGuard can only be 0 or 1(a Irecv(*) or a Irecv(i)), if 1, we can use a whole guard
                     * recv(*): if its size is 1 and the corresponding depended is a wildcard, we can use a whole guard. May depend on many Irecv(i),Irecv(j)...
                     * */
                    if (findGuard[(*it)->index].size() == 1 && (!(*it)->isWildcard(true) ||
                                                                mpicommlog[(*it)->rank][findGuard[(*it)->index].front()]->isWildcard(
                                                                        true))) {
                        char idChan[25], indexChan[25];
                        sprintf(idChan, "%d", (int) (*it)->rank);
                        sprintf(indexChan, "%d", findGuard[(*it)->index].front());
                        temp_Node->action.push_back("(H");
                        temp_Node->action.push_back(idChan);
                        temp_Node->action.push_back("_");
                        temp_Node->action.push_back(indexChan);
                        temp_Node->action.push_back("?");
                        temp_Node->action.push_back(indexChan);
                        temp_Node->action.push_back("->Skip);");
                        flagMulHb = false;
                    } else {
                        /*considering that if recv(*)'s findGuard has wildcard, it must only have one, we use a whole guard, and remove the first*/
                        if (findGuard[(*it)->index].size() > 1) {
                            std::list<int>::iterator it_L = findGuard[(*it)->index].begin();
                            while (it_L != findGuard[(*it)->index].end()) {
                                /*find the sole wildcard, generate a whole channel, and remove it*/
                                if (mpicommlog[(*it)->rank][*it_L]->isWildcard(true)) {
                                    char idChan[25], indexChan[25];
                                    sprintf(idChan, "%d", (int) (*it)->rank);
                                    sprintf(indexChan, "%d", findGuard[(*it)->index].front());
                                    temp_Node->action.push_back("(H");
                                    temp_Node->action.push_back(idChan);
                                    temp_Node->action.push_back("_");
                                    temp_Node->action.push_back(indexChan);
                                    temp_Node->action.push_back("?");
                                    temp_Node->action.push_back(indexChan);
                                    temp_Node->action.push_back("->Skip);");
                                    findGuard[(*it)->index].remove(*it_L);
                                    break;
                                }
                                it_L++;
                            }
                        }
                    }
                    /*using our static method to get all candidates, and model recv(*)*/
                    bool firstEnter = true;
                    int hasPrev = hasPrevwild(*it, mpicommlog); // also need to handle  Recv(*), Recv(2);
                    if ((*it)->isWildcard(true) || hasPrev > 0) {
                        vector<CommLogItem *> *matching_set = new vector<CommLogItem *>();
                        //get_CandidateSet((*it), mpicommlog, matching_set);
                        // we also need to consider the second receive in double receive MS.
                        if ((*it)->isWildcard(true) || (*it)->recvFM || ((*it)->recvOfMaster == 0 && !(*it)->isWildcard(
                                true))) { // for the recvFM, we need to treat it as a wildcard recv
                            int upper = get_upperBound((*it), mpicommlog);
                            // for recv, lower=upper+1;
                            //get_refinedCandidateSet((*it), mpicommlog, matching_set, upper, upper + 1);
                            get_refinedCandidateSet((*it), mpicommlog, matching_set, upper + 1);
                        } else {
                            std::set<CommLogItem *>::iterator it_candidate2 =
                                    (*it)->set_match_candidate.begin();
                            LOG(INFO) << "size: " << (*it)->set_match_candidate.size();
                            assert((*it)->set_match_candidate.size() != 0);
                            int tempIndex = (*it_candidate2)->index;
                            getCandidates(*it, tempIndex, hasPrev, mpicommlog, *matching_set);
                        }
                        // first, for the Recv in master, we need to filter the repetitive slave send!
                        if ((*it)->recvOfMaster >= 0 || (*it)->recvFM) {
                            std::vector<CommLogItem *>::iterator itm = (*matching_set).begin();
                            for (itm; itm != (*matching_set).end();) {
                                // LOG(INFO)<<(*itm)->rank<<"********"<<(*itm)->index;
                                if ((*itm)->filtered)
                                    itm = (*matching_set).erase(itm);
                                else
                                    itm++;
                            }
                        }
                        if ((*it)->recvOfMaster >= 0 && opt) { // modeling the optimization
                            if ((*it)->recvOfMaster == 1) {
                                int size = result->size(); // the number of solutions
                                list<int *>::iterator l;
                                int counter = 0;
                                for (l = (*result).begin(); l != (*result).end(); ++l) {
                                    int *p = *l;
                                    string temp;
                                    for (int k = 0; k < slaves; k++) {
                                        int value = p[k];
                                        for (int l = 0; l < p[k]; l++) {
                                            char id[10];
                                            sprintf(id, "%d", k + 1);
                                            temp = temp + "D" + id + "?" + id + "->S" + id + "!0->";
                                        }
                                    }
                                    temp = temp + "Skip";
                                    if (counter != size - 1)
                                        temp = temp + "[]";
                                    counter++;
                                    temp_Node->action.push_back(temp);
                                }
                            } else { // modeling the last n receives.
                                string temp = "(";
                                char id[10];
                                sprintf(id, "%d", lastNRecv + 1);
                                temp = temp + "D" + id + "?" + id + "->Skip";
                                temp = temp + ")";
                                lastNRecv++;
                                temp_Node->action.push_back(temp);
                            }

                        } else {
                            std::vector<CommLogItem *>::iterator it_candidate = (*matching_set).begin();
                            while (it_candidate != (*matching_set).end()) {
                                int rankId;
                                int indexTar = -1;
                                //generate special hb channel for special Isend
                                if ((*it)->isWildcard(true) && flagMulHb) {
                                    std::list<int>::iterator it_T = findGuard[(*it)->index].begin();
                                    while (it_T != findGuard[(*it)->index].end()) {
                                        if (dyn_cast<klee::ConstantExpr>(
                                                mpicommlog[(*it)->rank][(*it_T)]->arguments[3])->getZExtValue() ==
                                            (*it_candidate)->rank) {//candidate Isend is from the same src of hb irecv(i). that is i==src!
                                            indexTar = (*it_T);
                                            break;
                                        }
                                        it_T++;
                                    }
                                }
                                if ((*it_candidate)->mpi_op == COMM_ISEND) {
                                    rankId = (*it_candidate)->rank;
                                    sprintf(NumofChan, "%d",
                                            get_chanIndex((*it_candidate)->index, 0, mpicommlog[rankId]));
                                    sprintf(ProcessID, "%d", rankId);
                                    if (firstEnter) {
                                        if (indexTar != -1) {
                                            char idChan[25], indexChan[25];
                                            sprintf(idChan, "%d", (int) (*it)->rank);
                                            sprintf(indexChan, "%d", indexTar);
                                            temp_Node->action.push_back("([!call(cempty,D");
                                            temp_Node->action.push_back(ProcessID);
                                            temp_Node->action.push_back("_");
                                            temp_Node->action.push_back(NumofChan);
                                            temp_Node->action.push_back(")");
                                            if ((*it)->recvFM) { // special handler for recvFM
                                                char labelId[10];
                                                temp_Node->action.push_back("&& label");
                                                int varId = mpicommlog[(*it)->rank][(*it)->index - 1]->recvOfMaster;
                                                sprintf(labelId, "%d", varId);
                                                temp_Node->action.push_back(labelId);
                                                temp_Node->action.push_back("==");
                                                temp_Node->action.push_back(ProcessID); // label the slave id
                                            }

                                            temp_Node->action.push_back("]H");
                                            temp_Node->action.push_back(idChan);
                                            temp_Node->action.push_back("_");
                                            temp_Node->action.push_back(indexChan);
                                            temp_Node->action.push_back("?");
                                            temp_Node->action.push_back(indexChan);
                                            temp_Node->action.push_back("->D");
                                        } else {
                                            if ((*it)->recvFM && !opt) { // special handler for recvFM
                                                char labelId[10];
                                                temp_Node->action.push_back("([label");
                                                int varId = mpicommlog[(*it)->rank][(*it)->index - 1]->recvOfMaster;
                                                sprintf(labelId, "%d", varId);
                                                temp_Node->action.push_back(labelId);
                                                temp_Node->action.push_back("==");
                                                temp_Node->action.push_back(ProcessID); // label the slave id
                                                temp_Node->action.push_back("]D"); // label the slave id
                                            } else {
                                                temp_Node->action.push_back("(D");
                                            }

                                        }
                                        firstEnter = false;
                                    } else {
                                        char aa[10];
                                        char bb[10];
                                        if (indexTar != -1) {
                                            // need both H and order of Isend
                                            if ((*it_candidate)->NB >= 0) {
                                                sprintf(aa, "%d", (*it_candidate)->rank);
                                                sprintf(bb, "%d", (*it_candidate)->NB);
                                                temp_Node->action.push_back("[!call(cempty,D");
                                                temp_Node->action.push_back(ProcessID);
                                                temp_Node->action.push_back("_");
                                                temp_Node->action.push_back(NumofChan);
                                                temp_Node->action.push_back(")&&call(cempty,D");
                                                temp_Node->action.push_back(aa);
                                                temp_Node->action.push_back("_");
                                                temp_Node->action.push_back(bb);
                                                temp_Node->action.push_back(")");
                                                if ((*it)->recvFM && !opt) { // special handler for recvFM
                                                    char labelId[10];
                                                    temp_Node->action.push_back("&& label");
                                                    int varId = mpicommlog[(*it)->rank][(*it)->index - 1]->recvOfMaster;
                                                    sprintf(labelId, "%d", varId);
                                                    temp_Node->action.push_back(labelId);
                                                    temp_Node->action.push_back("==");
                                                    temp_Node->action.push_back(ProcessID); // label the slave id
                                                }
                                                temp_Node->action.push_back("]");
                                                (*it_candidate)->NB = -1; //we need to reset in order to affect it in other candidate set!
                                            } else {
                                                char idChan[10], indexChan[10];
                                                sprintf(idChan, "%d", (int) (*it)->rank);
                                                sprintf(indexChan, "%d", indexTar);
                                                temp_Node->action.push_back("[!call(cempty,D");
                                                temp_Node->action.push_back(ProcessID);
                                                temp_Node->action.push_back("_");
                                                temp_Node->action.push_back(NumofChan);
                                                temp_Node->action.push_back(")");
                                                if ((*it)->recvFM && !opt) { // special handler for recvFM
                                                    char labelId[10];
                                                    temp_Node->action.push_back("&& label");
                                                    int varId = mpicommlog[(*it)->rank][(*it)->index - 1]->recvOfMaster;
                                                    sprintf(labelId, "%d", varId);
                                                    temp_Node->action.push_back(labelId);
                                                    temp_Node->action.push_back("==");
                                                    temp_Node->action.push_back(ProcessID); // label the slave id
                                                }
                                                temp_Node->action.push_back("]");
                                            }
                                            char idChan[10], indexChan[10];
                                            sprintf(idChan, "%d", (int) (*it)->rank);
                                            sprintf(indexChan, "%d", indexTar);
                                            temp_Node->action.push_back("H");
                                            temp_Node->action.push_back(idChan);
                                            temp_Node->action.push_back("_");
                                            temp_Node->action.push_back(indexChan);
                                            temp_Node->action.push_back("?");
                                            temp_Node->action.push_back(indexChan);
                                            temp_Node->action.push_back("->");
                                        } else {
                                            if ((*it_candidate)->NB >= 0) {
                                                sprintf(aa, "%d", (*it_candidate)->rank);
                                                sprintf(bb, "%d", (*it_candidate)->NB);
                                                temp_Node->action.push_back("[call(cempty,D");
                                                temp_Node->action.push_back(aa);
                                                temp_Node->action.push_back("_");
                                                temp_Node->action.push_back(bb);
                                                temp_Node->action.push_back(")");
                                                if ((*it)->recvFM && !opt) { // special handler for recvFM
                                                    char labelId[10];
                                                    temp_Node->action.push_back("&& label");
                                                    int varId = mpicommlog[(*it)->rank][(*it)->index - 1]->recvOfMaster;
                                                    sprintf(labelId, "%d", varId);
                                                    temp_Node->action.push_back(labelId);
                                                    temp_Node->action.push_back("==");
                                                    temp_Node->action.push_back(ProcessID); // label the slave id
                                                }
                                                temp_Node->action.push_back("]");
                                                (*it_candidate)->NB = -1; //we need to reset in order to affect it in other candidate set!
                                            } else {
                                                if ((*it)->recvFM && !opt) { // special handler for recvFM
                                                    char labelId[10];
                                                    temp_Node->action.push_back(" [label");
                                                    int varId = mpicommlog[(*it)->rank][(*it)->index - 1]->recvOfMaster;
                                                    sprintf(labelId, "%d", varId);
                                                    temp_Node->action.push_back(labelId);
                                                    temp_Node->action.push_back("==");
                                                    temp_Node->action.push_back(ProcessID); // label the slave id
                                                    temp_Node->action.push_back("]");
                                                }
                                            }
                                        }
                                        temp_Node->action.push_back("D");
                                    }
                                }
                                if ((*it_candidate)->mpi_op == COMM_SSEND) {
                                    rankId = (*it_candidate)->rank;
                                    sprintf(NumofChan, "%d",
                                            get_chanIndex((*it_candidate)->index, 1, mpicommlog[rankId]));
                                    sprintf(ProcessID, "%d", rankId);
                                    if (firstEnter) {
                                        if (indexTar != -1) {
                                            char idChan[25], indexChan[25];
                                            sprintf(idChan, "%d", (int) (*it)->rank);
                                            sprintf(indexChan, "%d", indexTar);
                                            temp_Node->action.push_back("(H");
                                            temp_Node->action.push_back(idChan);
                                            temp_Node->action.push_back("_");
                                            temp_Node->action.push_back(indexChan);
                                            temp_Node->action.push_back("?");
                                            temp_Node->action.push_back(indexChan);
                                            temp_Node->action.push_back("->Skip);");
                                            temp_Node->action.push_back("C");
                                        } else {
                                            temp_Node->action.push_back("(");
                                            if ((*it)->recvOfMaster == 0 && !(*it)->isWildcard(true) && !opt) {
                                                char labelId[10];
                                                temp_Node->action.push_back(" [lab");
                                                sprintf(labelId, "%d", dcounter);
                                                temp_Node->action.push_back(labelId);
                                                temp_Node->action.push_back("==");
                                                temp_Node->action.push_back(ProcessID); // label the slave id
                                                temp_Node->action.push_back("]");
                                            }
                                            temp_Node->action.push_back("C");
                                        }

                                        firstEnter = false;
                                    } else {
                                        if (indexTar != -1) {
                                            char idChan[10], indexChan[10];
                                            sprintf(idChan, "%d", (int) (*it)->rank);
                                            sprintf(indexChan, "%d", indexTar);
                                            temp_Node->action.push_back("H");
                                            temp_Node->action.push_back(idChan);
                                            temp_Node->action.push_back("_");
                                            temp_Node->action.push_back(indexChan);
                                            temp_Node->action.push_back("?");
                                            temp_Node->action.push_back(indexChan);
                                            temp_Node->action.push_back("->");
                                        }

                                        if ((*it)->recvOfMaster == 0 && !(*it)->isWildcard(true) && !opt) {
                                            char labelId[10];
                                            temp_Node->action.push_back(" [lab");
                                            sprintf(labelId, "%d", dcounter);
                                            temp_Node->action.push_back(labelId);
                                            temp_Node->action.push_back("==");
                                            temp_Node->action.push_back(ProcessID); // label the slave id
                                            temp_Node->action.push_back("]");
                                        }

                                        temp_Node->action.push_back("C");
                                    }
                                }
                                temp_Node->action.push_back(ProcessID);
                                temp_Node->action.push_back("_");
                                temp_Node->action.push_back(NumofChan);
                                temp_Node->action.push_back("?");
                                temp_Node->action.push_back(NumofChan);

                                /// added by zhenbang to get the mapping information
                                insertOpChanOp(**it, **it_candidate, ProcessID, NumofChan);

                                //add the wild card info
//												temp_Node->action.push_back("->");
//												sprintf(Recv_Index, "%d", (*it)->index);
//												sprintf(Recv_Id, "%d", i);
//												sprintf(Send_Index, "%d", (*it_candidate)->index);
//												sprintf(Send_Id, "%d", rankId);
//												temp_Node->action.push_back("a");
//												temp_Node->action.push_back(Recv_Id);
//												temp_Node->action.push_back("_a");
//												temp_Node->action.push_back(Recv_Index);
//												temp_Node->action.push_back("_a");
//												temp_Node->action.push_back(Send_Id);
//												temp_Node->action.push_back("_a");
//												temp_Node->action.push_back(Send_Index);

                                //in Master-Slave Pattern, we need to handle the label assignment!
                                if ((*it)->recvOfMaster > 0 && (*it_candidate)->sendOfSlave && !opt) {
                                    char labelId[10];
                                    temp_Node->action.push_back("->{label");
                                    int varId = (*it)->recvOfMaster;
                                    sprintf(labelId, "%d", varId);
                                    temp_Node->action.push_back(labelId);
                                    temp_Node->action.push_back("=");
                                    temp_Node->action.push_back(ProcessID); // label the slave id
                                    temp_Node->action.push_back("}");
                                }
                                // to support the double receive MS pattern
                                if ((*it)->recvOfMaster == 0 && (*it)->isWildcard(true) && !opt) {
                                    char labelId[10];
                                    temp_Node->action.push_back("->{lab");
                                    sprintf(labelId, "%d", dcounter);
                                    temp_Node->action.push_back(labelId);
                                    temp_Node->action.push_back("=");
                                    temp_Node->action.push_back(ProcessID); // label the slave id
                                    temp_Node->action.push_back("}");
                                }
                                temp_Node->action.push_back("->Skip");
                                it_candidate++;
                                if (it_candidate != (*matching_set).end()) {
                                    temp_Node->action.push_back("[]");
                                }
                                    //						else
                                    //							temp_Node->action.push_back(")}");
                                else {
                                    if (Rflag == 1)
                                        temp_Node->action.push_back(");fevent->Skip"); // used for temporal checking
                                    if (Rflag == 2)
                                        temp_Node->action.push_back(");sevent->Skip"); // used for temporal checking
                                    else
                                        temp_Node->action.push_back(")");
                                }
                            }
                        }

                    }//----------------------------end-----------------------------------
                    else { // a deterministic matching, get from DSE
                        std::set<CommLogItem *>::iterator it_candidate =
                                (*it)->set_match_candidate.begin();
                        LOG(INFO) << "SIZE of RECV: " << (*it)->set_match_candidate.size();

//------------------------------special handling begin, all in the if scope---------------------------------------------------
// for recv in slave in MSPattern, we need to collect all send task in master, and also model the corresponding send in slave together

                        /*modeling the ms in optimized manner*/
                        if ((*it)->recvOfSlave && opt) {
                            // --------------------build the recursion process ---------------------------
                            // first, get the channel of the notification send
                            int indexNS = -1;
                            set<task_ty>::iterator itm = (*it)->tasks.begin();
                            for (itm; itm != (*it)->tasks.end();) {
                                int
                                        dest =
                                        dyn_cast<klee::ConstantExpr>(
                                                mpicommlog[(*itm).first][(*itm).second]->arguments[3])->getZExtValue();
                                if (dest == (*it)->rank
                                    && mpicommlog[(*itm).first][(*itm).second]->notifySend) {
                                    indexNS = get_chanIndex((*itm).second, 0, mpicommlog[(*itm).first]);
                                    break;
                                }
                                itm++;
                            }
                            char ind[10];
                            char rId[10];
                            sprintf(ind, "%d", indexNS);
                            string aa = "D0_";
                            aa = aa + ind + "?0" + "->Skip";
                            sprintf(rId, "%d", (*it)->rank);
                            string temp;
                            temp = temp + "P" + rId + "0=S" + rId + "?0->D" + rId + "!" + rId + "->P" + rId + "0[]" +
                                   aa + ";";
                            cspNode *temp_Node2 = new cspNode();
                            temp_Node2->action.push_back(temp);
                            recursionP->push_back(*temp_Node2);
                            // --------------------build the recursion process end---------------------------

                            string temp2;
                            temp2 = temp2 + "P" + rId + "0";
                            temp_Node->action.push_back(temp2);
                            temp_Node->op = Seq;
                            temp_list->push_back(*temp_Node);
                            it++;
                            break;
                        }


                        if ((*it)->recvOfSlave && !opt) {
                            char recursionRank[10];
                            string aa = "P";
                            sprintf(recursionRank, "%d", (*it)->rank);
                            aa = aa + recursionRank;
                            sprintf(recursionRank, "%d", counterPP[(*it)->rank]++);
                            aa = aa + recursionRank;
                            temp_Node->action.push_back(aa);
                            temp_Node->op = Seq;
                            temp_list->push_back(*temp_Node);
//-------------------generate the recursion process, and store it in recursionP-------------------------
                            cspNode *temp_Node2 = new cspNode();
                            aa = "P";
                            sprintf(recursionRank, "%d", (*it)->rank);
                            aa += recursionRank;
                            sprintf(recursionRank, "%d", counterPP[(*it)->rank] - 1);
                            aa = aa + recursionRank + "=";
                            temp_Node2->action.push_back(aa);
                            int source = dyn_cast<klee::ConstantExpr>(
                                    mpicommlog[(*it)->rank][(*it)->index]->arguments[3])->getZExtValue();
//--------------------collect all master send: 1) allocation send 2) repetitive send 3) notify send(contained itself)-----------------------------
//						for (int j = 0; j < mpicommlog[source].size(); j++) {
//							if (mpicommlog[source][j]->sendOfMaster
//									&& mpicommlog[source][j]->isSend()) {
//								if (dyn_cast<klee::ConstantExpr> (
//										mpicommlog[source][j]->arguments[3])->getZExtValue()
//										== (*it)->rank) {
//									(*it)->set_match_candidate.insert(mpicommlog[source][j]);
//								} else {
//									if (mpicommlog[source][j]->sendOfMaster > (*it)->slaves)
//										(*it)->set_match_candidate.insert(mpicommlog[source][j]);
//								}
//							}
//						}
// ------------------------build the recursion process------------------
                            //first we need to filter the false matching stored in tasks
                            set<task_ty>::iterator itm = (*it)->tasks.begin();
                            for (itm; itm != (*it)->tasks.end();) {
                                int dest = dyn_cast<klee::ConstantExpr>(
                                        mpicommlog[(*itm).first][(*itm).second]->arguments[3])->getZExtValue();
                                if (dest != (*it)->rank &&
                                    mpicommlog[(*itm).first][(*itm).second]->sendOfMaster <= (*it)->slaves)
                                    ((*it)->tasks).erase(itm++);
                                else
                                    itm++;
                            }
                            // build recursion process for the remained tasks
                            set<task_ty>::iterator iter_ty = (*it)->tasks.begin();
                            while (iter_ty != (*it)->tasks.end()) {
                                int rankId;
                                int tag = -1; // a label for handing the check [labeli==1] for repetitive pattern tasks
                                int dest = dyn_cast<klee::ConstantExpr>(
                                        mpicommlog[(*iter_ty).first][(*iter_ty).second]->arguments[3])->getZExtValue();
                                if (mpicommlog[(*iter_ty).first][(*iter_ty).second]->mpi_op == COMM_ISEND) {
                                    rankId = (*iter_ty).first;
                                    sprintf(NumofChan, "%d", get_chanIndex((*iter_ty).second, 0, mpicommlog[rankId]));
                                    sprintf(ProcessID, "%d", rankId);
                                    /* the choice that must have conditions*/
                                    if ((mpicommlog[(*iter_ty).first][(*iter_ty).second]->sendOfMaster > 0 ||
                                         mpicommlog[(*iter_ty).first][(*iter_ty).second]->notifySend)
                                        && (*it)->recvOfSlave) {
                                        tag = mpicommlog[(*iter_ty).first][(*iter_ty).second]->sendOfMaster -
                                              (*it)->slaves;
                                        if (tag >
                                            0) { // means we need use atomic blocking, because these tasks are allocated dynamically!
                                            sprintf(recursionRank, "%d", tag);
                                            aa = "[label";
                                            aa = aa + recursionRank + "==";
                                            sprintf(recursionRank, "%d", (*it)->rank);
                                            aa = aa + recursionRank + "]D";
                                            temp_Node2->action.push_back(aa);
                                        } else {
                                            if (mpicommlog[(*iter_ty).first][(*iter_ty).second]->notifySend) { //for notify send.
                                                aa = "[";
                                                int count = (*it)->schedules;//the number of labels.
                                                for (; count > 0; count--) {
                                                    sprintf(recursionRank, "%d", count);
                                                    char rid[10];
                                                    int identifier = (*it)->rank;
                                                    sprintf(rid, "%d", identifier);
                                                    aa = aa + "label" + recursionRank + "!=" + rid + "&&";
                                                }
                                                // add the check of the allocation send
                                                sprintf(recursionRank, "%d",
                                                        mpicommlog[(*iter_ty).first][(*iter_ty).second]->rank);
                                                aa = aa + "call(cempty,D" + recursionRank + "_";
                                                // find the index of allocation send
                                                int tempIndex = -1;
                                                set<task_ty>::iterator iter_ty2 = (*it)->tasks.begin();
                                                while (iter_ty2 != (*it)->tasks.end()) {
                                                    int target = -1;
                                                    if (mpicommlog[(*iter_ty2).first][(*iter_ty2).second]->isSend()) {
                                                        target = dyn_cast<klee::ConstantExpr>(
                                                                mpicommlog[(*iter_ty2).first][(*iter_ty2).second]->arguments[3])->getZExtValue();
                                                    }
                                                    if (mpicommlog[(*iter_ty2).first][(*iter_ty2).second]->sendOfMaster <=
                                                        (*it)->slaves
                                                        &&
                                                        !mpicommlog[(*iter_ty2).first][(*iter_ty2).second]->notifySend &&
                                                        target == (*it)->rank) {
                                                        tempIndex = (*iter_ty2).second;
                                                        break;
                                                    }
                                                    iter_ty2++;
                                                }
                                                sprintf(recursionRank, "%d",
                                                        get_chanIndex(tempIndex, 0, mpicommlog[(iter_ty2)->first]));
                                                aa = aa + recursionRank + ")]D";
                                                temp_Node2->action.push_back(aa);
                                            } else
                                                temp_Node2->action.push_back("D");
                                        }
                                    } else
                                        temp_Node2->action.push_back("D");
                                }
                                if ((*it_candidate)->mpi_op == COMM_SSEND) {
                                    rankId = (*iter_ty).first;
                                    sprintf(NumofChan, "%d", get_chanIndex((*iter_ty).second, 1, mpicommlog[rankId]));
                                    sprintf(ProcessID, "%d", rankId);
                                    temp_Node2->action.push_back("C");
                                }
                                aa = ProcessID;
                                aa = aa + "_" + NumofChan + "?" + NumofChan + "->";
                                sprintf(Recv_Index, "%d", (*it)->index);
                                sprintf(Recv_Id, "%d", i);
                                sprintf(Send_Index, "%d", (*iter_ty).second);
                                sprintf(Send_Id, "%d", rankId);
                                aa = aa + "a" + Recv_Id + "_a" + Recv_Index + "_a" + Send_Id + "_a" + Send_Index;
                                temp_Node2->action.push_back(aa);

                                // for MSPattern, we need to encode the corresponding send together!
                                aa = "";
                                if (mpicommlog[(*iter_ty).first][(*iter_ty).second]->sendOfMaster >= 0) {
                                    assert(mpicommlog[(*it)->rank][(*it)->index +
                                                                   1]->isSend()); // we believe the following is the binding send!
                                    char ChanId[10];
                                    char curRank[10];
                                    char tagId[10];

                                    sprintf(curRank, "%d", (*it)->rank);
                                    sprintf(tagId, "%d", tag);
                                    //reset the label for repetitive send
                                    if (mpicommlog[(*iter_ty).first][(*iter_ty).second]->sendOfMaster > (*it)->slaves) {
                                        aa = aa + "->{label" + tagId + "=0}";
                                    }
                                    int situation = -1;
                                    if (mpicommlog[(*it)->rank][(*it)->index + 1]->mpi_op == 2
                                        ||
                                        mpicommlog[(*it)->rank][(*it)->index + 1]->mpi_op == 5) { // for ISend and Send
                                        sprintf(ChanId, "%d",
                                                get_chanIndex((*it)->index + 1, 0, mpicommlog[(*it)->rank]));
                                        aa = aa + "->D" + curRank + "_" + ChanId + "!" + ChanId;
                                        situation = 1;
                                    }
                                    if (mpicommlog[(*it)->rank][(*it)->index + 1]->mpi_op == 1) { // for ISend and Send
                                        sprintf(ChanId, "%d",
                                                get_chanIndex((*it)->index + 1, 1, mpicommlog[(*it)->rank]));
                                        aa = aa + "->C" + curRank + "_" + ChanId + "!" + ChanId;
                                        situation = 2;
                                    }

                                    temp_Node2->action.push_back(aa);
                                    // chech whether the slave is a double send pattern: recv->send->send, if so, we also need to model it here
                                    int target1 = dyn_cast<klee::ConstantExpr>(
                                            mpicommlog[(*it)->rank][(*it)->index + 1]->arguments[3])->getZExtValue();
                                    if (mpicommlog[(*it)->rank][(*it)->index + 1]->isSend() &&
                                        mpicommlog[(*it)->rank][(*it)->index + 2]->isSend() &&
                                        target1 == dyn_cast<klee::ConstantExpr>(mpicommlog[(*it)->rank][(*it)->index +
                                                                                                        2]->arguments[3])->getZExtValue()) {

                                        if (situation == 1) {
                                            sprintf(ChanId, "%d",
                                                    get_chanIndex((*it)->index + 2, 0, mpicommlog[(*it)->rank]));
                                            sprintf(curRank, "%d", (*it)->rank);
                                            aa = "->D";
                                        }
                                        if (situation == 2) {
                                            sprintf(ChanId, "%d",
                                                    get_chanIndex((*it)->index + 2, 1, mpicommlog[(*it)->rank]));
                                            sprintf(curRank, "%d", (*it)->rank);
                                            aa = "->C";
                                        }
                                        aa = aa + curRank + "_" + ChanId + "!" + ChanId;
                                        temp_Node2->action.push_back(aa);
                                    }
                                }
                                aa = "->Skip";
                                if (mpicommlog[(*iter_ty).first][(*iter_ty).second]->sendOfMaster >=
                                    0) { // model the recursion
                                    sprintf(recursionRank, "%d", (*it)->rank);
                                    aa = aa + ";P" + recursionRank;
                                    sprintf(recursionRank, "%d", counterPP[(*it)->rank] - 1);
                                    aa = aa + recursionRank;
                                }
                                temp_Node2->action.push_back(aa);
                                iter_ty++;
                                if (iter_ty != (*it)->tasks.end()) {
                                    temp_Node2->action.push_back("[]");
                                } else
                                    temp_Node2->action.push_back(";");
                            }
                            recursionP->push_back((*temp_Node2));

                            it++;
                            break; //temporaly
                        }

                        if ((*it)->recvOfMaster == 0 &&
                            !opt) {  //the second receive of the double receive in MS pattern, we model it together with the first one
                            it++;
                            break;
                        }

//--------------------------------------------special handling end----------------------------------------------------------

                        it_candidate = (*it)->set_match_candidate.begin();
                        while (it_candidate != (*it)->set_match_candidate.end()) {
                            int rankId;
                            if ((*it_candidate)->mpi_op == COMM_ISEND) {
                                rankId = (*it_candidate)->rank;
                                sprintf(NumofChan, "%d",
                                        get_chanIndex((*it_candidate)->index, 0, mpicommlog[rankId]));
                                sprintf(ProcessID, "%d", rankId);
                                temp_Node->action.push_back("D");
                            }
                            if ((*it_candidate)->mpi_op == COMM_SSEND) {
                                rankId = (*it_candidate)->rank;
                                sprintf(NumofChan, "%d",
                                        get_chanIndex((*it_candidate)->index, 1, mpicommlog[rankId]));
                                sprintf(ProcessID, "%d", rankId);
                                temp_Node->action.push_back("C");
                            }

                            temp_Node->action.push_back(ProcessID);
                            temp_Node->action.push_back("_");
                            temp_Node->action.push_back(NumofChan);
                            temp_Node->action.push_back("?");
                            temp_Node->action.push_back(NumofChan);

                            /// added by zhenbang to get the mapping information
                            insertOpChanOp(**it, **it_candidate, ProcessID, NumofChan);

                            //add the wild card info
//						temp_Node->action.push_back("->");
//						sprintf(Recv_Index, "%d", (*it)->index);
//						sprintf(Recv_Id, "%d", i);
//						sprintf(Send_Index, "%d", (*it_candidate)->index);
//						sprintf(Send_Id, "%d", rankId);
//						temp_Node->action.push_back("a");
//						temp_Node->action.push_back(Recv_Id);
//						temp_Node->action.push_back("_a");
//						temp_Node->action.push_back(Recv_Index);
//						temp_Node->action.push_back("_a");
//						temp_Node->action.push_back(Send_Id);
//						temp_Node->action.push_back("_a");
//						temp_Node->action.push_back(Send_Index);
                            //----handle the corresponding isend wait channel write
//						if ((*it_candidate)->mpi_op == COMM_ISEND && testIsend(*it_candidate,mpicommlog)) {
//							char id[25], index[25];
//							sprintf(id, "%d", (*it_candidate)->rank);
//							sprintf(index, "%d", (*it_candidate)->index);
//							temp_Node->action.push_back("->");
//							temp_Node->action.push_back("F");
//							temp_Node->action.push_back(id);
//							temp_Node->action.push_back("_");
//							temp_Node->action.push_back(index);
//							temp_Node->action.push_back("!");
//							temp_Node->action.push_back(index);
//						}
                            //------------------end
                            temp_Node->action.push_back("->Skip");
                            it_candidate++;
                            if (it_candidate != (*it)->set_match_candidate.end()) {
                                temp_Node->action.push_back("[]");
                            } else {
                                if (Rflag == 1)
                                    temp_Node->action.push_back(";fevent->Skip"); // used for temporal checking
                                if (Rflag == 2)
                                    temp_Node->action.push_back(";sevent->Skip"); // used for temporal checking
                                temp_Node->action.push_back("}");
                            }
                        }
                    }
                    temp_Node->op = Seq;
                    temp_list->push_back(*temp_Node);
                    it++;
                    break;
                }
                case (COMM_ISEND): {
                    if (((*it)->sendOfSlave || (*it)->sendFS || (*it)->notifySend || (*it)->sendOfMaster) && opt2) {
                        it++;
                        break;
                    }
                    if ((*it)->sendOfSlave ||
                        (*it)->sendFS) { // special handling for MSPattern, model it together with its binding recv
                        if (!(*it)->filtered && !opt)
                            numOfIsend++;
                        it++;
                        break;
                    }
                    if ((*it)->sendOfMaster > 0 && opt) {
                        temp_Node = new cspNode();
                        if ((*it)->sendOfMaster == 1) {// model the allocation once
                            string temp;
                            char kk[10];
                            for (int k = 1; k <= slaves; k++) {
                                sprintf(kk, "%d", k);
                                temp = temp + "S" + kk + "!0->";
                            }
                            temp = temp + "Skip";
                            temp_Node->action.push_back(temp);
                            it++;
                            numOfIsend++;
                        } else {
                            numOfIsend++;
                            it++;
                            break;
                        }
                        temp_Node->op = Seq;
                        temp_list->push_back(*temp_Node);
                        break;
                    }

                    // test happensbefore
                    bool flagMulHb = true; // a label to indicate whether it's the pattern: recv(*) which has many hb channels
                    temp_Node = new cspNode();
                    temp_Node->hasdom = false;
                    temp_Node->hasguard = false;
                    temp_Node->haswait = true;
                    temp_Node->hasHbChan = false;
                    temp_Node->wait = (*it)->index; // every Isend must have a wait, we need to generate a channel to represent it.
                    /*Isend has dom*/
                    if (findSendBehind(*it, mpicommlog[i]) != -1) {
                        char dom_event[10];
                        temp_Node->hasdom = true;
                        temp_Node->dom = hap;
                        sprintf(dom_event, "%d", hap);
                        findGuard[findSendBehind(*it, mpicommlog[i])].push_back(hap);
                        hap++;
                    }
                    /*Isend has guard*/
                    if (findGuard[(*it)->index].size() != 0) {
                        temp_Node->hasguard = true;
                        temp_Node->guard = findGuard[(*it)->index].front();
                    }
                    int rankId = (*it)->rank;
                    sprintf(NumofChan, "%d", get_chanIndex((*it)->index, 0, mpicommlog[rankId]));
                    sprintf(ProcessID, "%d", rankId);
                    temp_Node->action.push_back("D");
                    temp_Node->action.push_back(ProcessID);
                    temp_Node->action.push_back("_");
                    temp_Node->action.push_back(NumofChan);
                    temp_Node->action.push_back("!");
                    if (opt && (*it)->sendOfMaster)
                        temp_Node->action.push_back("0");
                    else
                        temp_Node->action.push_back(NumofChan);

                    // added for verify temproal property between send operations
                    if ((*it)->rank == temp.rank1 && (*it)->index == temp.index1) {
                        firstEvent.push_back("D");
                        firstEvent.push_back(ProcessID);
                        firstEvent.push_back("_");
                        firstEvent.push_back(NumofChan);
                        firstEvent.push_back("!");
                        firstEvent.push_back(NumofChan);
                    }
                    if ((*it)->rank == temp.rank2 && (*it)->index == temp.index2) {
                        secondEvent.push_back("D");
                        secondEvent.push_back(ProcessID);
                        secondEvent.push_back("_");
                        secondEvent.push_back(NumofChan);
                        secondEvent.push_back("!");
                        secondEvent.push_back(NumofChan);
                    }
//				if (!temp_Node->hasdom) {
//					temp_Node->action.push_back("->Skip");
//				}
                    temp_Node->action.push_back("->Skip");
                    //temp_Node->op = Parallel;
                    /*new modified*/
                    temp_Node->hasdom = false;
                    temp_Node->hasguard = false;
                    temp_Node->op = Seq;
                    temp_list->push_back(*temp_Node);
                    numOfIsend++;
                    it++;
                    break;
                }
                    /*another complicated case
                     * if there exists some Irecv(*) before Irecv(2), then we need to add more candidates for this Irecv(2) */
                case (COMM_IRECV): {
                    bool flagMulHb = true;
                    //bool commFirst = false;
                    temp_Node = new cspNode();
                    temp_Node->hasdom = false;
                    temp_Node->hasguard = false;
                    temp_Node->haswait = false;
                    temp_Node->hasHbChan = false;
                    if (findGuard[(*it)->index].size() == 0)
                        flagMulHb = false; // no hb channel reading
                    /*two cases:
                     * Irecv(i): size of its findGuard can only be 0 or 1(a Irecv(*) or a Irecv(i)), if 1, we can use a whole guard
                     * Irecv(*): if its size is 1 and the corresponding depended is a wildcard, we can use a whole guard
                     * */
                    if (findGuard[(*it)->index].size() == 1 && (!(*it)->isWildcard(true) ||
                                                                mpicommlog[(*it)->rank][findGuard[(*it)->index].front()]->isWildcard(
                                                                        true))) {
                        char idChan[10], indexChan[10];
                        sprintf(idChan, "%d", (int) (*it)->rank);
                        sprintf(indexChan, "%d", findGuard[(*it)->index].front());
                        temp_Node->action.push_back("(H");
                        temp_Node->action.push_back(idChan);
                        temp_Node->action.push_back("_");
                        temp_Node->action.push_back(indexChan);
                        temp_Node->action.push_back("?");
                        temp_Node->action.push_back(indexChan);
                        temp_Node->action.push_back("->Skip);");
                        flagMulHb = false;
                    } else {
                        /*considering that if irecv(*)'s findGuard has wildcard, it must only have one, we a use a whole guard, and remove the first*/
                        if (findGuard[(*it)->index].size() > 1) {
                            std::list<int>::iterator it_L = findGuard[(*it)->index].begin();
                            while (it_L != findGuard[(*it)->index].end()) {
                                /*find the sole wildcard, generate a whole channel, and remove it*/
                                if (mpicommlog[(*it)->rank][*it_L]->isWildcard(true)) {
                                    char idChan[25], indexChan[25];
                                    sprintf(idChan, "%d", (int) (*it)->rank);
                                    sprintf(indexChan, "%d", findGuard[(*it)->index].front());
                                    temp_Node->action.push_back("(H");
                                    temp_Node->action.push_back(idChan);
                                    temp_Node->action.push_back("_");
                                    temp_Node->action.push_back(indexChan);
                                    temp_Node->action.push_back("?");
                                    temp_Node->action.push_back(indexChan);
                                    temp_Node->action.push_back("->Skip);");
                                    findGuard[(*it)->index].remove(*it_L);
                                    break;
                                }
                                it_L++;
                            }
                        }
                    }
                    // -----------using our static matching method to get all the possible matchings and model this Irecv(*)
                    bool flag_first = true;
                    /*first we need to check whether there exists some irecv(*) before this one*/
                    int hasPrev = hasPrevwild(*it, mpicommlog);
                    if ((*it)->set_match_candidate.size() == 0) {
                        LOG(INFO) << "don't need to handle it since it has no dynamic matching!"
                                  << "\n";
                        break;
                    }
                    /*the case that we need static method to get all the candidates, even for a deterministic one, if hasPrev>1, it also need static matching!*/
                    if ((*it)->isWildcard(true) || hasPrev > 0) {
                        vector<CommLogItem *> *matching_set = new vector<CommLogItem *>();
                        if ((*it)->isWildcard(true)) {
                            int upper = get_upperBound((*it), mpicommlog);
                            int lower = get_lowerBound((*it), mpicommlog);
                            //get_refinedCandidateSet((*it), mpicommlog, matching_set, upper, lower+1);
                            get_refinedCandidateSet((*it), mpicommlog, matching_set, lower + 1);
                        } else {
                            std::set<CommLogItem *>::iterator it_candidate2 =
                                    (*it)->set_match_candidate.begin();
                            assert((*it)->set_match_candidate.size() != 0);
                            int tempIndex = (*it_candidate2)->index;
                            getCandidates(*it, tempIndex, hasPrev, mpicommlog, *matching_set);
                        }
                        std::vector<CommLogItem *>::iterator it_candidate = (*matching_set).begin();
                        while (it_candidate != (*matching_set).end()) {
                            int indexTar = -1;
                            //generate special H channel for special Isend, note that only for wildcard!
                            if ((*it)->isWildcard(true) && flagMulHb) {
                                std::list<int>::iterator it_T = findGuard[(*it)->index].begin();
                                while (it_T != findGuard[(*it)->index].end()) {
                                    if (dyn_cast<klee::ConstantExpr>(
                                            mpicommlog[(*it)->rank][(*it_T)]->arguments[3])->getZExtValue() ==
                                        (*it_candidate)->rank) {
                                        indexTar = (*it_T);
                                        break;
                                    }
                                    it_T++;
                                }
                            }
                            int rankId;
                            if ((*it_candidate)->mpi_op == COMM_ISEND) {
                                rankId = (*it_candidate)->rank;
                                sprintf(NumofChan, "%d", get_chanIndex((*it_candidate)->index, 0, mpicommlog[rankId]));
                                sprintf(ProcessID, "%d", rankId);
                                if (flag_first) {
                                    if (indexTar != -1) {
                                        char idChan[25], indexChan[25];
                                        sprintf(idChan, "%d", (int) (*it)->rank);
                                        sprintf(indexChan, "%d", indexTar);
                                        temp_Node->action.push_back("([!call(cempty,D");
                                        temp_Node->action.push_back(ProcessID);
                                        temp_Node->action.push_back("_");
                                        temp_Node->action.push_back(NumofChan);
                                        temp_Node->action.push_back(")]");
                                        temp_Node->action.push_back("H");
                                        temp_Node->action.push_back(idChan);
                                        temp_Node->action.push_back("_");
                                        temp_Node->action.push_back(indexChan);
                                        temp_Node->action.push_back("?");
                                        temp_Node->action.push_back(indexChan);
                                        temp_Node->action.push_back("->");
                                        temp_Node->action.push_back("D");
                                    } else
                                        temp_Node->action.push_back("(D");
                                    flag_first = false;
                                } else {
                                    if (indexTar != -1) {
                                        if ((*it_candidate)->NB >= 0) {
                                            char aa[10];
                                            char bb[10];
                                            sprintf(aa, "%d", (*it_candidate)->rank);
                                            sprintf(bb, "%d", (*it_candidate)->NB);
                                            temp_Node->action.push_back("[!call(cempty,D");
                                            temp_Node->action.push_back(ProcessID);
                                            temp_Node->action.push_back("_");
                                            temp_Node->action.push_back(NumofChan);
                                            temp_Node->action.push_back(")&&call(cempty,D");
                                            temp_Node->action.push_back(aa);
                                            temp_Node->action.push_back("_");
                                            temp_Node->action.push_back(bb);
                                            temp_Node->action.push_back(")]");
                                            (*it_candidate)->NB = -1; //we need to reset in order to affect it in other candidate set!
                                        } else {
                                            char idChan[10], indexChan[10];
                                            sprintf(idChan, "%d", (int) (*it)->rank);
                                            sprintf(indexChan, "%d", indexTar);
                                            temp_Node->action.push_back("[!call(cempty,D");
                                            temp_Node->action.push_back(ProcessID);
                                            temp_Node->action.push_back("_");
                                            temp_Node->action.push_back(NumofChan);
                                            temp_Node->action.push_back(")]");
                                            temp_Node->action.push_back("H");
                                            temp_Node->action.push_back(idChan);
                                            temp_Node->action.push_back("_");
                                            temp_Node->action.push_back(indexChan);
                                            temp_Node->action.push_back("?");
                                            temp_Node->action.push_back(indexChan);
                                            temp_Node->action.push_back("->");
                                        }
                                    } else {
                                        /*we need to ensure the order of candidates from same process*/
                                        if ((*it_candidate)->NB >= 0) {
                                            char aa[10];
                                            char bb[10];
                                            sprintf(aa, "%d", (*it_candidate)->rank);
                                            sprintf(bb, "%d", (*it_candidate)->NB);
                                            temp_Node->action.push_back("[call(cempty,D");
                                            temp_Node->action.push_back(aa);
                                            temp_Node->action.push_back("_");
                                            temp_Node->action.push_back(bb);
                                            temp_Node->action.push_back(")]");
                                            (*it_candidate)->NB = -1; //we need to reset in order to affect it in other candidate set!
                                        }
                                    }
                                    temp_Node->action.push_back("D");
                                }
                            }
                            if ((*it_candidate)->mpi_op == COMM_SSEND) {
                                rankId = (*it_candidate)->rank;
                                sprintf(NumofChan, "%d", get_chanIndex((*it_candidate)->index, 1, mpicommlog[rankId]));
                                sprintf(ProcessID, "%d", rankId);
                                if (flag_first) {
                                    if (indexTar != -1) {
                                        char idChan[25], indexChan[25];
                                        sprintf(idChan, "%d", (int) (*it)->rank);
                                        sprintf(indexChan, "%d", indexTar);
                                        temp_Node->action.push_back("([!call(cempty,D");
                                        temp_Node->action.push_back(ProcessID);
                                        temp_Node->action.push_back("_");
                                        temp_Node->action.push_back(NumofChan);
                                        temp_Node->action.push_back(")]");
                                        temp_Node->action.push_back("H");
                                        temp_Node->action.push_back(idChan);
                                        temp_Node->action.push_back("_");
                                        temp_Node->action.push_back(indexChan);
                                        temp_Node->action.push_back("?");
                                        temp_Node->action.push_back(indexChan);
                                        temp_Node->action.push_back("->Skip);");
                                        temp_Node->action.push_back("C");
                                    } else
                                        temp_Node->action.push_back("(C");
                                    flag_first = false;
                                } else {
                                    if (indexTar != -1) {
                                        char idChan[10], indexChan[10];
                                        sprintf(idChan, "%d", (int) (*it)->rank);
                                        sprintf(indexChan, "%d", indexTar);
                                        temp_Node->action.push_back("[!call(cempty,D");
                                        temp_Node->action.push_back(ProcessID);
                                        temp_Node->action.push_back("_");
                                        temp_Node->action.push_back(NumofChan);
                                        temp_Node->action.push_back(")]");
                                        temp_Node->action.push_back("H");
                                        temp_Node->action.push_back(idChan);
                                        temp_Node->action.push_back("_");
                                        temp_Node->action.push_back(indexChan);
                                        temp_Node->action.push_back("?");
                                        temp_Node->action.push_back(indexChan);
                                        temp_Node->action.push_back("->");
                                    }
                                    temp_Node->action.push_back("C");
                                }
                            }
                            temp_Node->action.push_back(ProcessID);
                            temp_Node->action.push_back("_");
                            temp_Node->action.push_back(NumofChan);
                            temp_Node->action.push_back("?");
                            temp_Node->action.push_back(NumofChan);

                            /// added by zhenbang to get the mapping information
                            insertOpChanOp(**it, **it_candidate, ProcessID, NumofChan);

                            //add the wildcard info for analyze the couter-example trace!
//						temp_Node->action.push_back("->");
//						sprintf(Recv_Index, "%d", (*it)->index);
//						sprintf(Recv_Id, "%d", i);
//						sprintf(Send_Index, "%d", (*it_candidate)->index);
//						sprintf(Send_Id, "%d", rankId);
//						temp_Node->action.push_back("a");
//						temp_Node->action.push_back(Recv_Id);
//						temp_Node->action.push_back("_a");
//						temp_Node->action.push_back(Recv_Index);
//						temp_Node->action.push_back("_a");
//						temp_Node->action.push_back(Send_Id);
//						temp_Node->action.push_back("_a");
//						temp_Node->action.push_back(Send_Index);
                            //----handle the corresponding isend wait channel write
//						if ((*it_candidate)->mpi_op == COMM_ISEND && testIsend(*it_candidate,mpicommlog)) {
//							char id[25], index[25];
//							sprintf(id, "%d", (*it_candidate)->rank);
//							sprintf(index, "%d", (*it_candidate)->index);
//							temp_Node->action.push_back("->");
//							temp_Node->action.push_back("F");
//							temp_Node->action.push_back(id);
//							temp_Node->action.push_back("_");
//							temp_Node->action.push_back(index);
//							temp_Node->action.push_back("!");
//							temp_Node->action.push_back(index);
//						}
                            temp_Node->action.push_back("->Skip");
                            it_candidate++;
                            if (it_candidate != (*matching_set).end()) {
                                temp_Node->action.push_back("[]");
                            } else {
                                temp_Node->action.push_back(")");
                            }
                        }
                    }
                        // the case that don't need the static matching method!
                    else { // this a deterministic matching, get it from DSE.
                        std::set<CommLogItem *>::iterator it_candidate =
                                (*it)->set_match_candidate.begin();
                        while (it_candidate != (*it)->set_match_candidate.end()) {
                            assert((*it)->set_match_candidate.size() == 1);// otherwise, an error happened!
                            int rankId;
                            if ((*it_candidate)->mpi_op == COMM_ISEND) {
                                rankId = (*it_candidate)->rank;
                                sprintf(NumofChan, "%d",
                                        get_chanIndex((*it_candidate)->index, 0, mpicommlog[rankId]));
                                sprintf(ProcessID, "%d", rankId);
                                temp_Node->action.push_back("D");
                            }
                            if ((*it_candidate)->mpi_op == COMM_SSEND) {
                                rankId = (*it_candidate)->rank;
                                sprintf(NumofChan, "%d",
                                        get_chanIndex((*it_candidate)->index, 1, mpicommlog[rankId]));
                                sprintf(ProcessID, "%d", rankId);
                                temp_Node->action.push_back("C");
                            }
                            temp_Node->action.push_back(ProcessID);
                            temp_Node->action.push_back("_");
                            temp_Node->action.push_back(NumofChan);
                            temp_Node->action.push_back("?");
                            temp_Node->action.push_back(NumofChan);

                            /// added by zhenbang to get the mapping information
                            insertOpChanOp(**it, **it_candidate, ProcessID, NumofChan);

                            //add the wild card info
                            temp_Node->action.push_back("->");
//						sprintf(Recv_Index, "%d", (*it)->index);
//						sprintf(Recv_Id, "%d", i);
//						sprintf(Send_Index, "%d", (*it_candidate)->index);
//						sprintf(Send_Id, "%d", rankId);
//						temp_Node->action.push_back("a");
//						temp_Node->action.push_back(Recv_Id);
//						temp_Node->action.push_back("_a");
//						temp_Node->action.push_back(Recv_Index);
//						temp_Node->action.push_back("_a");
//						temp_Node->action.push_back(Send_Id);
//						temp_Node->action.push_back("_a");
//						temp_Node->action.push_back(Send_Index);
                            temp_Node->action.push_back("->Skip");
                            it_candidate++;
                            if (it_candidate != (*it)->set_match_candidate.end()) {
                                temp_Node->action.push_back("[]");
                            }
                        }
                    }
                    // ------------------end  --------------------------------
                    //Irecv has dom, generate the dom channel read write of this Irecv
                    std::vector<int> *dependingSet = new vector<int>();
                    findRecvBehind(*it, mpicommlog[i], *dependingSet);
                    if (dependingSet->size() != 0) { // -->chan! --> ...
                        temp_Node->hasHbChan = true;
                        temp_Node->hbChan = (*it)->index;
                        std::vector<int>::iterator it_V = (*dependingSet).begin();
                        while (it_V != (*dependingSet).end()) {
                            findGuard[*it_V].push_back((*it)->index);
                            it_V++;
                        }
                        char idChan[25], indexChan[25];
                        sprintf(idChan, "%d", (*it)->rank);
                        sprintf(indexChan, "%d", (*it)->index);
                        temp_Node->action.push_back(";");
                        temp_Node->action.push_back("H");
                        temp_Node->action.push_back(idChan);
                        temp_Node->action.push_back("_");
                        temp_Node->action.push_back(indexChan);
                        temp_Node->action.push_back("!");
                        temp_Node->action.push_back(indexChan);
                        // ------------------handle the wait of Irecv
                        findWait[(*it)->index] = hap; // assign the wait of Isend
                        hap++;
                        char wait[25];
                        sprintf(wait, "%d", findWait[(*it)->index]);
                        temp_Node->action.push_back("->wait");
                        temp_Node->action.push_back(wait);
                        //-------------------end-----------------------
                        temp_Node->action.push_back("->Skip");
                    } else { // don't have dom we still need to generate a wait for irecv
                        findWait[(*it)->index] = hap; // assign the wait of Isend
                        hap++;
                        char wait[25];
                        sprintf(wait, "%d", findWait[(*it)->index]);
                        temp_Node->action.push_back(";wait");
                        temp_Node->action.push_back(wait);
                        //-------------------end-----------------------
                        temp_Node->action.push_back("->Skip");
                    }
                    temp_Node->op = Parallel;
                    temp_list->push_back(*temp_Node);
                    it++;
                    break;
                }
                case (COMM_WAIT): {
                    temp_Node = new cspNode();
                    temp_Node->hasdom = false;
                    temp_Node->hasguard = false;
                    temp_Node->haswait = false;
                    temp_Node->hasHbChan = false;
                    CommLogItem *temp = getWaitTarget(*it, mpicommlog[i]);
                    if (temp->mpi_op == COMM_ISEND) { // generate the corresponding channel read
                        char id[25], index[25];
                        it++;
                        break;
                    } else { // generate just a wait operation for Irecv
                        char wait[10];
                        sprintf(wait, "%d", findWait[temp->index]);
                        temp_Node->action.push_back("wait");
                        temp_Node->action.push_back(wait);
                        temp_Node->action.push_back("->Skip");
                    }
                    temp_Node->op = Seq;
                    temp_list->push_back(*temp_Node);
                    it++;
                    break;
                }
                default: {
                    LOG(INFO) << Process_num << "unmatched mpi operator!!!!!" << endl;
                    it++;
                    break;
                }
            }


        }
        stringstream os;
        for (auto l : *temp_list) {
            for (auto s : l.action) {
                os << s;
            }
            os << " ";
        }
        LOG(INFO) << os.str() << "\n";
        pro[i] = *(new CSP_Process(*temp_list, numOfIsend, numOfSend));
    }


    // now we need generate the declaration of channels
    char PID[25];
    char chanID[25];
    //int len = strlen(getpwuid(getuid())->pw_dir)+strlen("/test_new.csp") +1;
    int pid = getpid();
    string name = "/CSP_Model";
    string nameT = "/CSP_TF";
    string name2 = ".csp";
    string name3;
    string name3T;
    string path(getpwuid(getuid())->pw_dir);
    if (ModelGenerationPath.getValue().empty() == false) {
        path = ModelGenerationPath.getValue();
    }
    char ad[10];
    sprintf(ad, "%d_", pid);
    char model_count[10];
    sprintf(model_count, "%d", global_model_counter);
    //global_model_counter++;
    name3 = name + ad + model_count + name2;
    name3T = nameT + ad + model_count + name2;
    //int len = strlen(getpwuid(getuid())->pw_dir)+strlen("/CSP_Model.csp") +1;
    int len = path.size() + strlen(name3.c_str()) + 1;
    int len2 = path.size() + strlen(name3T.c_str()) + 1;
    char filename[len];
    char filename2[len2];
    //snprintf(filename, len, "%s%s", getpwuid(getuid())->pw_dir,"/test_new.csp");
    //snprintf(filename, len, "%s%s", getpwuid(getuid())->pw_dir,"/CSP_Model.csp");
    snprintf(filename, len, "%s%s", path.c_str(), name3.c_str());
    snprintf(filename2, len2, "%s%s", path.c_str(), name3T.c_str());
    FILE *fp = fopen(filename, "w+");
    if (fp != NULL) {
        LOG(INFO) << "----------has opened successfully-----a-" << endl;
    } else {
        LOG(FATAL) << "open file " << filename << "to write failed!" << endl;
    }
//	FILE *fp2 = fopen(filename2, "w+");
//	if(fp2!=NULL)
//	{
//		LOG(INFO)<<"----------has opened successfully-----a-"<<endl;
//	}
//	else{
//		LOG(FATAL)<<"open file "<< filename2 <<"to write failed!"<<endl;
//	}

    // in Master-Slave pattern, we declare the auxiliary channels.
    if (MSPattern && opt) {
        for (int i = 1; i <= slaves; i++) {
            sprintf(PID, "%d", i);
            fprintf(fp, "%s", "channel S");
            fprintf(fp, "%s", PID);
            fprintf(fp, "%s", " 1;\n");
            fprintf(fp, "%s", "channel D");
            fprintf(fp, "%s", PID);
            fprintf(fp, "%s", " 1;\n");
        }
//		for(int i=0;i<slaves;i++){
//			sprintf(PID, "%d", i);
//						fprintf(fp, "%s", "channel S");
//						fprintf(fp, "%s", PID);
//						fprintf(fp, "%s", " 1;\n");
//		}
    }
    if (MSPattern && !opt) {
        for (int i = 1; i <= tasks; i++) {
            sprintf(PID, "%d", i);
            fprintf(fp, "%s", "var label");
            fprintf(fp, "%s", PID);
            fprintf(fp, "%s", "=0;\n");
        }
        for (int i = 1; i <= dcounter; i++) {
            sprintf(PID, "%d", i);
            fprintf(fp, "%s", "var lab");
            fprintf(fp, "%s", PID);
            fprintf(fp, "%s", "=0;\n");
        }
    }


    for (int j = 0; j < Process_num; j++) {
        /*extern C channel*/
        for (int l = 0; l < pro[j].numOfSend; l++) {
            fprintf(fp, "%s", "channel C");
            sprintf(PID, "%d", j);
            sprintf(chanID, "%d", l);
            fprintf(fp, "%s", PID);
            fprintf(fp, "%s", "_");
            fprintf(fp, "%s", chanID);
            fprintf(fp, "%s", " 0;");
            fprintf(fp, "%s", "\n");
        }
        /*extern D channel*/
        for (int l = 0; l < pro[j].numOfIsend; l++) {
            fprintf(fp, "%s", "channel D");
            sprintf(PID, "%d", j);
            sprintf(chanID, "%d", l);
            fprintf(fp, "%s", PID);
            fprintf(fp, "%s", "_");
            fprintf(fp, "%s", chanID);
            fprintf(fp, "%s", " 1;");
            fprintf(fp, "%s", "\n");
        }
        /*extern H channel*/
        std::list<cspNode>::iterator it_temp = pro[j].actionlist.begin();
        while (it_temp != pro[j].actionlist.end()) {
            if ((*it_temp).hasHbChan) {
                fprintf(fp, "%s", "channel H");
                sprintf(PID, "%d", j);
                sprintf(chanID, "%d", (*it_temp).hbChan);
                fprintf(fp, "%s", PID);
                fprintf(fp, "%s", "_");
                fprintf(fp, "%s", chanID);
                fprintf(fp, "%s", " 1;");
                fprintf(fp, "%s", "\n");
            }
            it_temp++;
        }
    }
//	/*extern S channel*/
//   std::list<Location>::iterator it_S=listOfSChan->begin();
//   while(it_S!= listOfSChan->end()){
//		fprintf(fp, "%s", "channel S");
//		sprintf(PID, "%d", (*it_S).PID);
//		sprintf(chanID, "%d", (*it_S).index);
//		fprintf(fp, "%s", PID);
//		fprintf(fp, "%s", chanID);
//		fprintf(fp, "%s", " 1;");
//		fprintf(fp, "%s", "\n");
//		it_S++;
//   }
    /*extern F channel*/
//			int pN=mpicommlog.size();
//			for(int j=0;j<pN;j++){
//				std::vector<CommLogItem *>::iterator it_temp = mpicommlog[j].begin();
//		        while(it_temp!= mpicommlog[j].end()){
//		        	//extern the fcChan for Isend wait
//		        	//moreover, it still need to have a corresponding wait!
//		        	 if((*it_temp)->mpi_op==COMM_ISEND){
//		        		if(testIsend( *it_temp  , mpicommlog)){
//			     			fprintf(fp, "%s", "channel F");
//			     			sprintf(PID, "%d", j);
//			     			sprintf(chanID, "%d", (*it_temp)->index);
//			     			fprintf(fp, "%s", PID);
//			     			fprintf(fp, "%s", "_");
//			     			fprintf(fp, "%s", chanID);
//			     			fprintf(fp, "%s", " 1;");
//			     			fprintf(fp, "%s", "\n");
//		        		}
//		        	 }
//		        	 it_temp++;
//		        }
//			}
// ----------------------------------------channel declaration ended---------------------------------------------------------------------------------------
    // now we need to generate the definition of processes
    //print the sychronized events of the process needed
    if (need_deal_B) {
        char ID_Pro[10];
        for (int j = 0; j < Process_num; j++) {
            if (hasBarrier[j] == false) {
                fprintf(fp, "%s", "#alphabet P");
                sprintf(ID_Pro, "%d", j);
                fprintf(fp, "%d", ID_Pro);
                fprintf(fp, "%s", "{");
                fprintf(fp, "%s", "B");
                fprintf(fp, "%s", "}\n");
            }
        }
    }


    // if in Master-Slave Pattern, we need to define the recursion process in slave separately!
    list<cspNode>::iterator iter2 = recursionP->begin();
    while (iter2 != recursionP->end()) {
        std::vector<std::string>::iterator label = (iter2->action).begin();
        while (label != (iter2->action).end()) {
            fprintf(fp, "%s", (char *) ((*label).c_str()));
            label++;
        }
        iter2++;
        fprintf(fp, "%s", "\n");
    }
// -------------------------------------recursion definition end--------------------

// ------generate the file for transformation, used by weijinag-------------------------------

    //print2(fp2,mpicommlog);
    //fclose(fp2);

// -------------------------------ending --------------------------------------------


    for (int j = 0; j < Process_num; j++) {
        pro[j].print(fp, j);
    }
    fprintf(fp, "%s", "P=");
    for (int j = 0; j < Process_num; j++) {
        sprintf(PID, "%d", j);
        fprintf(fp, "%s", "P");
        fprintf(fp, "%s", PID);
        if (j != Process_num - 1) {
            fprintf(fp, "%s", "||");
        }
    }
    fprintf(fp, "%s", ";\n");
    // we need to verify temporal property
    if (temp.rank1 != -1) {
        if (Rflag >= 0) { // temporal property for recvs
            fprintf(fp, "%s", "#assert P |= G(! fevent U sevent); \n ");
        } else { // assert for temporal property between sends
            fprintf(fp, "%s", "#assert P |= G(! ");
            std::vector<std::string>::iterator label1 = (firstEvent).begin();
            while (label1 != (firstEvent).end()) {
                fprintf(fp, "%s", (char *) ((*label1).c_str()));
                label1++;
            }
            fprintf(fp, "%s", " U ");
            std::vector<std::string>::iterator label2 = (secondEvent).begin();
            while (label2 != (secondEvent).end()) {
                fprintf(fp, "%s", (char *) ((*label2).c_str()));
                label2++;
            }
            fprintf(fp, "%s", ");\n");
        }
    } else
        fprintf(fp, "%s", "#assert P deadlockfree;\n");

    /// if the LTL property is valid
//    if (state->executor->property) {
//        for (auto pair : opChanOpMap) {
//            LOG(INFO) << pair.first << ":";
//            for (auto v : pair.second) {
//                LOG(INFO) << v << "--";
//            }
//            LOG(INFO) << "\n";
//        }
//
//    }

    if ( LTLFile.size() > 0 && state->executor->property && opChanOpMap.size() > 0 ) {
        set<string> propertyStrSet;

        state->executor->property->getLTLProperty(opChanOpMap, propertyStrSet);

        for (auto s : propertyStrSet) {
            string assertStr = "#assert P |= " + s + " ; \n";
            LOG(INFO) << s << "\n";
            fprintf(fp, "%s", assertStr.c_str());
        }
    }

    fclose(fp);
    callPAT(filename);
}

