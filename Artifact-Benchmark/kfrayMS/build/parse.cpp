#include <fstream>
#include <string>
#include <iostream>
using namespace std;

int main(){
  string files[6]={"kfray_muts1.log","kfray_muts1.opt.log","kfray_muts2.log","kfray_muts2.opt.log","kfray_muts3.log","kfray_muts3.opt.log"
  };
  ofstream output;
  output.open("result.txt",ios::app);
  int i=0;
  while(i!=6){
  string str=files[i];
  ifstream in(str.c_str());
  string line;
  bool to=false;
  if(in){
     output<<"----------------------------------result of "<<str<<"------------------------------------"<<endl;
     while(getline(in,line)){
        if(line.find("Resulting")==0){
          output<<line<<endl;
           if(to)
             break;
        }
        if(line.find("KLEE: HaltTimer")==0){
          output<<line<<endl;
          to=true;
        }
        if(!to&&line.find("real")==0){
          output<<line<<endl;
          break;
        }
    }
  }
  else{
  cout<<"no such file!"<<endl;
  }
  i++;
  }
  output.close();
return 0;
}
