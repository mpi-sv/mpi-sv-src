#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <utility>
#include <pwd.h>
#include <glog/logging.h>
#include "mpise/analysis.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <llvm/Support/CommandLine.h>

using namespace std;

/*
 * through analyze the model checking result, return a list of wildcard matching pair
 * */
returnlist* find(string &str) {
	char currch;
	int num = 0;
	int length = 0;
	int a[4];
	int i = 0;
	list<pair1> *relist = new list<pair1> ();
	pair1 * result;
	string::iterator it = str.begin();
	while (it != str.end()) {
		if (*it == 'a') {
			bool label = false;
			num = 0;
			string::iterator scan = it;
			string::iterator ori = it;
			scan++;
			while (*(scan) >= '0' && *(scan) <= '9') {
				length++;
				scan++;
			}
			while (length > 0) {
				ori++;
				num = num * 10 + *(ori) - '0';
				length--;
				label = true;
			}
			if (label) {
				a[i] = num;
				i++;
			}
			if (i == 4) {
				i = 0;
				result = new pair1();
				result->first.PID = a[0];
				result->first.index = a[1];
				result->second.PID = a[2];
				result->second.index = a[3];
				relist->push_back(*result);
			}
		}
		it++;
	}
	return relist;
}

/*store the file infor to a string str*/
int getFullFile(ifstream &ifs, string &str) {
	char currch;
	str.erase();
	while (!ifs.eof()) {
		ifs.get(currch);
		if (ifs.bad()) //there is an unrecoverable error.
			return -1;
		if (currch == '\0')
			break;
		else
			str += currch;
	}
	return str.length();
}

/*display the matching from the counter-example  trace*/
void display_wildcard(returnlist * l) {
	returnlist::iterator it_vec = l->begin();
	while (it_vec != l->end()) {
		std::cout << "PID of Rcev: " << ((*it_vec).first).PID << endl;
		std::cout << "index of Rcev: " << ((*it_vec).first).index << endl;
		std::cout << "PID of Send: " << ((*it_vec).second).PID << endl;
		std::cout << "index of Send: " << ((*it_vec).second).index << endl;
		it_vec++;
	}
}


extern cl::opt<std::string> ModelGenerationPath;
extern int global_model_counter;

// return 0 if deadlock free; 1 for deadlock
int getResult(){
	int pid=getpid();
		string pwd=getpwuid(getuid())->pw_dir;
		if (ModelGenerationPath.getValue().empty() == false) {
			pwd = ModelGenerationPath.getValue();
		}
		char temp[10];
		sprintf(temp,"%d",pid);

		char count[10];
		sprintf(count, "%d", global_model_counter);

		string filename2=pwd+"/PAT_Result"+temp+ "_" + count + ".txt";
		int len=strlen(filename2.c_str());
		char filename[len+1];
		strcpy(filename,filename2.c_str());
		ifstream tfile(filename, ios::binary);
		LOG(INFO)<<filename;
		if (!tfile.is_open())
			LOG(FATAL) << "Error opening file " << filename;
		string trace("");
		string str="NOT";
		int t=-1;
		while (!tfile.eof()) {
			getFullFile(tfile, trace);
			LOG(INFO) << trace <<endl;
			if(trace.find(str)==string::npos)
				{t=0; break;}
			else
				{t=1; break;}
		}
		global_model_counter++;
		return t;
}

/*
 * through analyze the trace of counter-example, return a list of wildcard matching pair
 * the information of wild card is ax_ax_ax_ax
 * */
returnlist* get_wildcard() {
	//int len = strlen(getpwuid(getuid())->pw_dir) + strlen("/PAT_Result.txt") + 1;
	//char filename[len];
	//snprintf(filename, len, "%s%s", getpwuid(getuid())->pw_dir, "/PAT_Result.txt");
	int pid=getpid();
	string pwd=getpwuid(getuid())->pw_dir;
	if (ModelGenerationPath.getValue().empty() == false) {
		pwd = ModelGenerationPath.getValue();
	}
	char temp[10];
	sprintf(temp,"%d",pid);

	char count[10];
	sprintf(count, "%d", global_model_counter);

	string filename2=pwd+"/PAT_Result"+temp+ "_" + count + ".txt";
	int len=strlen(filename2.c_str());
	char filename[len+1];
	strcpy(filename,filename2.c_str());
	ifstream tfile(filename, ios::binary);
	LOG(INFO)<<filename;
	if (!tfile.is_open())
		LOG(FATAL) << "Error opening file " << filename;
	string trace("");
	returnlist *t = NULL;
	while (!tfile.eof()) {
		getFullFile(tfile, trace);
		t = find(trace);
	}
	display_wildcard(t);

	global_model_counter++;
	return t;
}

/* dump the information of wild card is ax_ax_ax_ax to the return string */
string get_state_mark(returnlist* rl,
		std::map<mpi_rank_t, std::vector<CommLogItem *> > & mpicommlog) {
	string res;
	CommLogItem * item;
	if (rl == NULL || mpicommlog.empty()) {
		res = "";
		return res;
	}
	/*use foreach to enumerate all the elements in the returnlist
	 * then we will return the infomation of the matching*/
	BOOST_FOREACH(pair1& p, *rl) {
		item = mpicommlog[p.first.PID][p.first.index];
		if(item->wildcard_rewrite || item->isWildcard(true)) {
			res.append(boost::lexical_cast<string>(p.first.PID));
			res.append(boost::lexical_cast<string>(","));
			res.append(boost::lexical_cast<string>(p.first.index));
			res.append(boost::lexical_cast<string>(","));
			res.append(boost::lexical_cast<string>(p.second.PID));
			res.append(boost::lexical_cast<string>(","));
			res.append(boost::lexical_cast<string>(p.second.index));
			res.append(boost::lexical_cast<string>(";"));
		}
	}
	return res;
}
