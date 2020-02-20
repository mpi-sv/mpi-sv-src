//
// Created by zbchen on 12/29/19.
//

#include <stdlib.h>
#include "ltl/Property.h"
#include <boost/algorithm/string.hpp>

// initialize the current_states;
//int LTL_Property::initialization_Current_States(){
//    Current_States.insert(Automaton->initial_state);
//    return 0;
//};

std::map<std::string, std::string> sensitiveEventFunctionMap;

void LTL_Property::initialize(string fileStr) {

    /// read from file
    /// we suppose that LTL property is in one line and there is no '=' in the line
    /// for the atoms, we require the form should be
    /// p<1-9><0-9>* = <op>(<number>, <number>)
    /// e.g.,
    /// p1 = send(1, 2)
    /// p2 = recv(3, 4)
    ifstream file;
    file.open(fileStr.c_str(), ios::in);
    string strLine;
    while (getline(file, strLine)) {
        if (strLine.empty()) {
            continue;
        }
        //  Atom information            atoi(str.c_str()): string->int

        /// trim the line first
        boost::trim(strLine);

        if (strLine.size() > 0) {
            if (strLine.find("=") != string::npos) {
                string op = strLine.substr(strLine.find("=") + 1);
                boost::trim(op);  /// trim
                boost::replace_all(op, " ", ""); /// replace empty space
                string indexStr = strLine.substr(1, strLine.find("=") - 1);
                boost::trim(indexStr);
                int index = atoi(indexStr.c_str());
                Atom_Imp[op] = index;
            } else {
                /// it is the property str
                ltlStr = strLine;
            }
        }
    }

    /// parse the automata
    if (ltlStr.size() > 0) {
        /// first do the negation (we monitor the negation of the verification property)
        negLTLStr = "! " + ltlStr;
        this->ltl = parserLTL((char *)ltlStr.c_str());
        this->negltl = parserLTL((char *)negLTLStr.c_str());
        this->Automaton = translateLTL((char *)negLTLStr.c_str());

        fixAutomtata();

        //this->Automaton->dump();
    }

}

void LTL_Property::fixAutomtata() {
    /// this is pretty hacky ( TODO: to be refined later)
    if (ltlStr.find("U") != ltlStr.npos || ltlStr.find("V") != ltlStr.npos) {
        set<int> allAxioms;
        for (auto pair : Atom_Imp) {
            allAxioms.insert(pair.second);
        }
        int newState = Automaton->m_next;
        set<Transition *> addedTrans;
        /// we need to add some transitions
        for (auto s : Automaton->states) {
            /// only consider non-final states
            if (Automaton->finalStates.find(s) == Automaton->finalStates.end()) {
                set<Transition *> trans;
                Automaton->getTransition(s, trans);
                set<int> existingTrans;
                for (auto t : trans) {
                    for (auto a : t->transition_atoms) {
                        if (a->isNegated() == false) {
                            existingTrans.insert(a->getValue());
                        }
                    }
                }

                for (auto a : allAxioms) {
                    if (existingTrans.find(a) == existingTrans.end()) {
                        Transition *newT = new Transition();
                        newT->source = s;
                        newT->target = newState;
                        newT->transition_atoms.insert((LtlAtom *)(&LtlAtom::construct(a, false)));
                        addedTrans.insert(newT);
                    }
                }

            }
        }
        if (addedTrans.size() > 0) {
            Automaton->states.insert(newState);
            Automaton->transitions.insert(addedTrans.begin(), addedTrans.end());
        }
    }
}

//// initialize the proposition->atom map.
//void LTL_Property::Initialization_Atom_Imp(string &filePath){
//}


// (proposition) send(1,2) -> 0 (atom)
int LTL_Property::get_value(string &atom_key) {
    map<string, int>::iterator it;
    it = Atom_Imp.find(atom_key);
    if (it == Atom_Imp.end()) {
        return -1;
    }
    else{
        return it->second;
    }
}

// (proposition) send(1,2) -> [-1,0,2,3] (atom set)
set <int> LTL_Property::get_Atoms(int atom_value) {
    set<int> Atoms;
    map<string, int>::iterator it;
    for (it = Atom_Imp.begin(); it != Atom_Imp.end(); ++it) {
        if (it->second != atom_value) {
            Atoms.insert(-it->second);
        }
        else{
            Atoms.insert(it->second);
        }
    }
    return Atoms;
}

/**
 * Generate the mapping recursively
 * @param map
 * @param result
 */
void getMap(std::map<int, std::set<std::string> > &map, set<std::map<unsigned, string > > &result) {
    if (map.size() == 1) {
        for (auto s : map) {
            for (auto str : map[s.first]){
                std::map<unsigned, string > single;
                single[s.first] = str;
                result.insert(single);
            }
        }
    } else {
        std::map<int, std::set<std::string> >::iterator first = map.begin();
        std::map<int, std::set<std::string> > restMap ;
        std::map<int, std::set<std::string> >::iterator second = ++map.begin();
        while(second != map.end()) {
            restMap.insert(*second);
            second++;
        }
        getMap(restMap, result);
        set<std::map<unsigned, string > > realResult;
        for (auto s : first->second) {
            std::map<unsigned, string > tmp;
            tmp[first->first] = s;
            for (auto subMap : result) {
                tmp.insert(subMap.begin(), subMap.end());
            }
            realResult.insert(tmp);
        }
        result = realResult;
    }
}

/**
 * We use recursion to print
 * @param ltl
 * @param map
 * @return
 */
string printString(const Ltl &ltlInput, std::map<unsigned, string > &map) {
    string result = "";
    const Ltl *ltl = &ltlInput;
    Ltl::Kind kind = ltl->getKind();
    if (kind == Ltl::Atom) {
        LtlAtom *atom = (LtlAtom *) ltl;
        result = "\"" + map[atom->getValue()] + "\"";
        if (atom->isNegated()) {
            result = "! " + result;
        }
    } else if (kind == Ltl::Constant) {
        LtlConstant *constant = (LtlConstant *)ltl;
        result = constant?"true":"false";
    } else if (kind == Ltl::Junct) {
        LtlJunct *junct = (LtlJunct *)ltl;
        string left = printString(junct->getLeft(), map);
        string right = printString(junct->getRight(), map);
        if (junct->getCon()) {
            result = "(" + left + " && " + right + " )";
        } else {
            result = "(" + left + " || " + right + " )";
        }
    } else if (kind == Ltl::Iff) {
        LtlIff *iff = (LtlIff *)ltl;
        string left = printString(iff->getLeft(), map);
        string right = printString(iff->getRight(), map);
        if (iff->getIff()) {
            result = "(" + left + " <-> " + right + " )";
        } else {
            result = "(" + left + " xor " + right + " )";
        }
    } else if (kind == Ltl::Future) {
        LtlFuture *future = (LtlFuture *)ltl;
        string subString = printString(future->getFormula(), map);
        switch (future->getOp()) {
            case LtlFuture::next:
                result = "(X " + subString + " )";
                break;
            case LtlFuture::globally:
                result = "([] " + subString + " )";
                break;
            case LtlFuture::finally:
                result = "(<> " + subString + " )";
                break;
        }
    } else if (kind == Ltl:: Until) {
        LtlUntil *util = (LtlUntil *)ltl;
        string left = printString(util->getLeft(), map);
        string right = printString(util->getRight(), map);
        if (util->isRelease()) {
            result = "(" + left + " R " + right + " )";
        } else {
            result = "(" + left + " U " + right + " )";
        }
    }
    return result;
}

/**
 * Generate the LTL property for PAT using the map
 * @param map
 * @return
 */
void LTL_Property::getLTLProperty(std::map<std::string, std::set<std::string> > &map, set<string> &ltlSet) {
    set<std::map<unsigned, string > > setOfMaps;

    std::map<int, std::set<std::string> > idMap;

    for (auto pair : Atom_Imp) {
       if ( map.find(pair.first) != map.end()) {
           idMap[pair.second] = map[pair.first];
       }
    }

    getMap(idMap, setOfMaps);

    for (auto s : setOfMaps) {
        for (auto a : s) {
           std::cout << a.first << "->" << a.second << ":";
        }
        std::cout<< "\n";
        ltlSet.insert(printString((const Ltl &)*ltl, s));
    }
}


