//
//  LTL.h
//  Property-Analysis
//
//  Created by WJ-Hong on 19/12/22.
//  Copyright © 2019 weijiang.Hong. All rights reserved.
//

#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include "lbt.h"
using namespace std;


extern std::map<std::string, std::string> sensitiveEventFunctionMap;

class LTL_Property {
public:
    map<string, int> Atom_Imp;
    string negLTLStr;
    string ltlStr;
    Automata *Automaton;
    Ltl * negltl;
    Ltl * ltl;

public:

    // initialize the current_states;
    // int initialization_Current_States();
    // initialize the proposition->atom map.
    // void Initialization_Atom_Imp(string &filePath);
    void initialize(string fileStr);
    // (proposition) send(1,2) -> 0 (atom)
    int get_value(string &atom_key);
    // (proposition) send(1,2) -> [-1,0,2,3] (atom set)
    set <int> get_Atoms(int atom_value);

    void fixAutomtata();

    void getLTLProperty(std::map<std::string, std::set<std::string> > &map, set<string> &ltlSet);
};
