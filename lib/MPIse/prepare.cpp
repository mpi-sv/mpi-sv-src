
#include <klee/ExecutionState.h>
#include <klee/Expr.h>
#include "mpise/commlog.h"
#include "mpise/commlogManager.h"
#include <glog/logging.h>
#include <mpise/prepare.h>
using namespace std;

/*we need to do 4 things:
 * 1. count the energy(number of slaves, and repetitive patterns( i.e. recv(*) send(*.source) ) in Master-Slave Pattern.
 * 2. locate the special recv in slave for modeling, and assign the energy to it
 * 3. locate the special send in slave for modeling
 * 4. locate the task allocation send in master for modeling
 *    5. locate the schedule send in master, used for optimization
 *  */
void prepareMS(mpicommlog_ty mpicommlog, int& tasks, int& slaves) {
	int counter1 = 0; // the number of slaves.
	int counter2 = 0; // the number of repetitive patterns.
	bool doubleRecv=false;
	int temp3 = 0;
	for (int i = 0; i < mpicommlog.size(); i++) {
		for (int j = 0; j < mpicommlog[i].size(); j++) {
			CommLogItem *temp = mpicommlog[i][j];
			//LOG(INFO)<<"DAVIS"<<i<<"*****"<<j<<"*********"<<temp->recvOfMaster;
			if (temp->recvOfMaster == 1) { // every MS pattern has the first encountered special recv in master process
				std::set<task_ty> sends;//store all the sends used in the master process for MS pattern
				counter1 = temp->set_match_candidate.size(); // use its size as number of slaves
				slaves=counter1;
      //LOG(INFO)<<"davisssss:"<<counter1;
				counter2 = 1; // counter the repetitive patterns: following recv(*) that has the same static location with the first encountered one
				int labelIndex = j; // the index of the last repetitive recv in master
				// we use another strategy to count the repetitive patterns because of mandelbrot, i.e., we count the number of sends with
				// the same location of the nearest send below temp!
      bool foundSpecialSend=false;
		   CommLogItem *specialSend;
				for (int n = j + 1; n < mpicommlog[i].size(); n++) {
					LOG(INFO)<<"n is %d"<<n<<"line--is:---"<<mpicommlog[i][n]->lineNumber;
//					if (mpicommlog[i][n]->isRecv() && mpicommlog[i][n]->file == temp->file
//							&& mpicommlog[i][n]->lineNumber == temp->lineNumber) {
//						counter2++;
//						labelIndex = n;
//					}
       if(foundSpecialSend){
        if(mpicommlog[i][n]->isSend()&& mpicommlog[i][n]->file==specialSend->file&&
        		mpicommlog[i][n]->lineNumber==specialSend->lineNumber){
        	     counter2++;
        	     labelIndex=n;
                               }
                   }

					if (!foundSpecialSend && mpicommlog[i][n]->isSend()) {
						specialSend=mpicommlog[i][n];
						foundSpecialSend=true;
						if(n-temp->index>1)
							doubleRecv=true;
						labelIndex=n;
					}
				}
				tasks=counter2;
				//LOG(INFO)<<"davisssss:"<<counter2;
				//  we also need to label the send that notify slave to terminate, note that count1 is the number of slaves
				int counter0 = 0;
				int labelIndex2 = 0; // the index of the last notify send in master process
				for (labelIndex2 = labelIndex+1; labelIndex2 < mpicommlog[i].size() && counter0
						< counter1; labelIndex2++) {
					if (mpicommlog[i][labelIndex2]->mpi_op == 2
							|| mpicommlog[i][labelIndex2]->mpi_op == 5) {
						 counter0++;
					  mpicommlog[i][labelIndex2]->notifySend = true;
					  task_ty ty(i,labelIndex2);
					  sends.insert(ty);
					}
				}
				// mark the special n (n=slaves) recv in master, which are used to recv the remained results
				counter0 = 0;
				for (int n = labelIndex+1; n < mpicommlog[i].size() && counter0 < counter1; n++) {
					if (mpicommlog[i][n]->isRecv()) {
						if(doubleRecv)
							counter0=counter0+0.5;
						else
						counter0++;
						mpicommlog[i][n]->recvOfMaster = 0;
					}
				}

				//second, we mark the corresponding send in master process.
					int temp2 = counter1;// the nearest counter1 send above the first special recv in master
					temp3 = 0;//reset temp3 to zero for a new MS pattern.
					for (int k = temp->index - 1; k >= 0 && temp2 > 0; k--) { // the task allocation send in master process
						if (mpicommlog[i][k]->mpi_op == 2 || mpicommlog[i][k]->mpi_op == 5) {
							mpicommlog[i][k]->sendOfMaster = temp2;
							temp2--;
							task_ty ty(i,k);
							sends.insert(ty);
						}
					}
				// locate the repetitive send and possible following recv
					CommLogItem * temp22; //the first repetitive send
					CommLogItem * temp33=NULL; // the possible following recv in Master; in most cases, not exist
				for (int k = temp->index + 1; k < mpicommlog[temp->rank].size(); k++) {
					if (mpicommlog[temp->rank][k]->isSend()) { // found and break;
						temp22 = mpicommlog[temp->rank][k];
						break;
					}
					if (mpicommlog[temp->rank][k]->isRecv()) {
						mpicommlog[temp->rank][k]->recvFM = true; //the following recv
						temp33=mpicommlog[temp->rank][k];
					}
				}


				// collect all the repetitive send
				task_ty ty(i,temp22->index);
				sends.insert(ty);
//					assert(mpicommlog[i][temp->index+1]->isSend());
//					task_ty ty(i,temp->index+1);
//					sends.insert(ty);
					for (int k = temp22->index + 1; k < mpicommlog[i].size(); k++) {
					if (mpicommlog[i][k]->isSend()) { // collect the repetitive send
						if (mpicommlog[i][k]->file != temp22->file || mpicommlog[i][k]->lineNumber
								!= temp22->lineNumber)
							break;
						else {
							task_ty ty(i, k);
							sends.insert(ty);
						}
					}
					if (temp33 != NULL && mpicommlog[i][k]->isRecv()) { // label the filtered following recv
						if (mpicommlog[i][k]->file == temp33->file && mpicommlog[i][k]->lineNumber
								== temp33->lineNumber) {
							mpicommlog[i][k]->filtered = false;
							mpicommlog[i][k]->recvFM = true;
						}
					}
				}

				//third, we assgin the counter1 and counter2 to the corresponding recv in slave, and label the special send in Slave
				std::set<CommLogItem*>::iterator iter = temp->set_match_candidate.begin(); // temp is the first encountered recv in master
				while (iter != temp->set_match_candidate.end()) {
					// locate the special send in slave process
					mpicommlog[(*iter)->rank][(*iter)->index]->sendOfSlave = true;
					CommLogItem * temp44=NULL;
					// check whether the following is a following send, in this version, we only support one following
					if(mpicommlog[(*iter)->rank][(*iter)->index+1]->isSend() &&
							dyn_cast<klee::ConstantExpr> (mpicommlog[(*iter)->rank][(*iter)->index+1]->arguments[3])->getZExtValue()==temp->rank &&
							mpicommlog[(*iter)->rank][(*iter)->index+1]->file==mpicommlog[(*iter)->rank][(*iter)->index]->file&&
							mpicommlog[(*iter)->rank][(*iter)->index+1]->lineNumber!=mpicommlog[(*iter)->rank][(*iter)->index]->lineNumber){
						mpicommlog[(*iter)->rank][(*iter)->index+1]->sendFS=true;
						temp44=mpicommlog[(*iter)->rank][(*iter)->index+1];
					}
					//   mark the following send in slave that have the same static location
					for (int n = (*iter)->index + 2; n < mpicommlog[(*iter)->rank].size(); n++) {
						if (mpicommlog[(*iter)->rank][n]->mpi_op == 1 || mpicommlog[(*iter)->rank][n]->mpi_op == 2 || mpicommlog[(*iter)->rank][n]->mpi_op == 5) {
							// mark the repetitive send1
							if(mpicommlog[(*iter)->rank][n]->file == (*iter)->file && mpicommlog[(*iter)->rank][n]->lineNumber == (*iter)->lineNumber){
								mpicommlog[(*iter)->rank][n]->sendOfSlave = true;
								mpicommlog[(*iter)->rank][n]->filtered = true; // the following send should be filtered for modeling
							}
							// if possible, mark the repetitive send2
							if(temp44!=NULL && mpicommlog[(*iter)->rank][n]->file == temp44->file && mpicommlog[(*iter)->rank][n]->lineNumber == temp44->lineNumber){
								mpicommlog[(*iter)->rank][n]->sendFS = true;
							  mpicommlog[(*iter)->rank][n]->filtered = true; // the following send should be filtered for modeling
							}
						}
					}
					// label for the special recv(the nearest recv above the candidate which has the same source), and assign counter1 and counter2
//					for (int n = (*iter)->index; n >= 0; n--) {
//						int source = dyn_cast<klee::ConstantExpr> (
//								mpicommlog[(*iter)->rank][n]->arguments[3])->getZExtValue();
//						if (mpicommlog[(*iter)->rank][n]->isRecv() && source == i) {
//							mpicommlog[(*iter)->rank][n]->slaves = counter1;
//							mpicommlog[(*iter)->rank][n]->schedules = counter2;
//							mpicommlog[(*iter)->rank][n]->recvOfSlave = true;
//							CommLogItem*temp2 = mpicommlog[(*iter)->rank][n];
//							temp2->tasks=sends;
//							// we also label the following repetitive recv in slave process as filtered
//							for (int l = (*iter)->index + 1; l < mpicommlog[(*iter)->rank].size(); l++) {
//								if (mpicommlog[(*iter)->rank][l]->isRecv()
//										&& mpicommlog[(*iter)->rank][l]->file == temp2->file
//										&& mpicommlog[(*iter)->rank][l]->lineNumber
//												== temp2->lineNumber) {
//									mpicommlog[(*iter)->rank][l]->recvOfSlave = true;
//									mpicommlog[(*iter)->rank][l]->slaves = counter1;
//									mpicommlog[(*iter)->rank][l]->schedules = counter2;
//									mpicommlog[(*iter)->rank][l]->filtered = true;
//								}
//							}
//							break;
//						}
//					}

					// label the pre special recv nearest the slave send
					for (int n = (*iter)->index; n >= 0; n--) {
						int source = dyn_cast<klee::ConstantExpr> (
								mpicommlog[(*iter)->rank][n]->arguments[3])->getZExtValue();
						if (mpicommlog[(*iter)->rank][n]->isRecv() && source == i) {
							mpicommlog[(*iter)->rank][n]->slaves = counter1;
							mpicommlog[(*iter)->rank][n]->schedules = counter2;
							mpicommlog[(*iter)->rank][n]->recvOfSlave = true;
							CommLogItem*temp2 = mpicommlog[(*iter)->rank][n];
							temp2->tasks = sends;
							break;
						}
					}
					// label the post special recv nearest the slave send, and mark the ones having the same static location
					for (int n = (*iter)->index; n<mpicommlog[(*iter)->rank].size(); n++) {
						int source = dyn_cast<klee::ConstantExpr> (
								mpicommlog[(*iter)->rank][n]->arguments[3])->getZExtValue();
						if (mpicommlog[(*iter)->rank][n]->isRecv() && source == i) {
							mpicommlog[(*iter)->rank][n]->slaves = counter1;
							mpicommlog[(*iter)->rank][n]->schedules = counter2;
							mpicommlog[(*iter)->rank][n]->recvOfSlave = true;
							mpicommlog[(*iter)->rank][n]->filtered = true; // because we use the pre to model
							CommLogItem*temp2 = mpicommlog[(*iter)->rank][n];
							temp2->tasks = sends;
							// we also label the following repetitive recv in slave process as filtered
							for (int l = temp2->index + 1; l < mpicommlog[(*iter)->rank].size(); l++) {
								if (mpicommlog[(*iter)->rank][l]->isRecv()
										&& mpicommlog[(*iter)->rank][l]->file == temp2->file
										&& mpicommlog[(*iter)->rank][l]->lineNumber
												== temp2->lineNumber) {
									mpicommlog[(*iter)->rank][l]->recvOfSlave = true;
									mpicommlog[(*iter)->rank][l]->slaves = counter1;
									mpicommlog[(*iter)->rank][l]->schedules = counter2;
									mpicommlog[(*iter)->rank][l]->filtered = true;
								}
							}
							break;
						}
					}
					iter++;
				}

			}
			// mark the following send in master process
			if (temp->recvOfMaster >= 1)
				for (int k = temp->index + 1; k < mpicommlog[i].size(); k++) {
					if (mpicommlog[i][k]->mpi_op == 2 || mpicommlog[i][k]->mpi_op == 5) {
						mpicommlog[i][k]->sendOfMaster = counter1 + temp3 + 1;
						temp3++;
						break;
					}
				}
		}
	}
}


/* dump the collected information for debugging*/
void dumpMS(mpicommlog_ty mpicommlog){
	for (int i = 0; i < mpicommlog.size(); i++) {
		for (int j = 0; j < mpicommlog[i].size(); j++) {
			CommLogItem *temp = mpicommlog[i][j];
			if(temp->recvOfMaster>0){
				LOG(INFO)<<"recvOfMaster:"<<temp->rank<<"---"<<temp->index<<"---filtered info:"<<(temp->filtered?1:0)
						<<"label info:"<<temp->recvOfMaster;
			}
			if(temp->recvOfMaster==0){
				LOG(INFO)<<"last n recvOfMaster:"<<temp->rank<<"---"<<temp->index<<"---filtered info:"<<(temp->filtered?1:0)
						<<"label info:"<<temp->recvOfMaster;
			}
			if(temp->recvOfSlave){
				LOG(INFO)<<"recvOfSlave:"<<temp->rank<<"---"<<temp->index << "---slaves:"<<temp->slaves<<
						"schedules:"<<temp->schedules <<"---filtered info:"<<(temp->filtered?1:0);
			}
			if(temp->sendOfMaster>0){
				LOG(INFO)<<"sendOfMaster:"<<temp->rank<<"---"<<temp->index<<"---filtered info:"<<(temp->filtered?1:0)
						<< "---id of send:"<<temp->sendOfMaster;
			}
			if(temp->sendOfSlave){
				LOG(INFO)<<"sendOfSlave:"<<temp->rank<<"---"<<temp->index<<"---filtered info:"<<(temp->filtered?1:0);
			}
			if(temp->notifySend){
				LOG(INFO)<<"notifySend:"<<temp->rank<<"---"<<temp->index<<"---filtered info:"<<(temp->filtered?1:0) <<"sendOfMaster:"<<temp->sendOfMaster;
			}
			if(temp->recvFM){
					LOG(INFO)<<"recvFM:"<<temp->rank<<"---"<<temp->index<<"---filtered info:"<<(temp->filtered?1:0);
				}
			if(temp->sendFS){
							LOG(INFO)<<"sendFS:"<<temp->rank<<"---"<<temp->index<<"---filtered info:"<<(temp->filtered?1:0);
						}
		}
}
}


