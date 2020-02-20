/// @file lbt.C
/// The main program of lbt

/*
 *  Copyright � 2001 Heikki Tauriainen <Heikki.Tauriainen@hut.fi>
 *  Copyright � 2001 Marko M�kel� <msmakela@tcs.hut.fi>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/** @mainpage LBT: LTL to B�chi Translator
 * @htmlonly
 * Please visit also the
 * <a href="http://www.tcs.hut.fi/maria/tools/lbt/">LBT home page</a>.
 * @endhtmlonly
 */

#include "ltl/lbt.h"
#include <stdio.h>
#include <iostream>
#include <map>
#include <fstream>
#include <string>
#include <stdlib.h>
#include "ltl/Property.h"

FILE * file;
char * global_ltl_str;
int globa_ltl_str_index = 0;

char getFileChar() {
    char c;
    fread(&c, sizeof(char), 1, file);
//    printf("%c\n", c);
    return c;
}

char getStrChar() {
    int len = strlen(global_ltl_str);
    if (globa_ltl_str_index < len) {
        char c = global_ltl_str[globa_ltl_str_index];
        globa_ltl_str_index++;
        return c;
    }
    return EOF;
}

/** Read an LTL formula from standard input
 * @param negation	flag: negate the formula
 * @return		the parsed formula, or NULL on error
 */
static class Ltl* readFormula(bool negation) {
    int ch;
    class Ltl *left, *right;
    loop:
    while ((ch = getStrChar()) == ' ' || ch == '\n' || ch == '\r' ||
           ch == '\t' || ch == '\v' || ch == '\f') ;
    switch (ch) {
        case 't':
        case 'f':
            return &LtlConstant::construct((ch == 'f') == negation);
        case 'p':
            /// we need to scan a integer
            char str[3];
            ch = getStrChar();
            if (!(ch >= '0' && ch <= '9')) {
                fputs("error in proposition number\n", stderr);
                return 0;
            }
            str[0] = ch;

            ch = getStrChar();
            if (!(ch >= '0' && ch <= '9')) {
                globa_ltl_str_index--;
                return &LtlAtom::construct((unsigned int)(str[0]-'0'), negation);
            }

            str[1] = ch;
            str[2] = '\0';

            return &LtlAtom::construct(atoi(str), negation);
        case '!':
            negation = !negation;
            goto loop;
        case '&':
        case '|':
            if ((left = readFormula(negation)) && (right = readFormula(negation)))
                return &LtlJunct::construct((ch == '&') != negation, *left, *right);
            return 0;
        case 'i':
            if ((left = readFormula(!negation)) && (right = readFormula(negation)))
                return &LtlJunct::construct(negation, *left, *right);
            return 0;
        case 'e':
        case '^' :
            if ((left = readFormula(false)) && (right = readFormula(false)))
                return &LtlIff::construct((ch == 'e') != negation, *left, *right);
            return 0;
        case 'X':
            return (left = readFormula(negation))
                   ? &LtlFuture::construct(LtlFuture::next, *left)
                   : 0;
        case 'F':
        case 'G':
            return (left = readFormula(negation))
                   ? &LtlFuture::construct((ch == 'F') == negation
                                           ? LtlFuture::globally
                                           : LtlFuture::finally, *left)
                   : 0;
        case 'U':
        case 'V':
            if ((left = readFormula(negation)) && (right = readFormula(negation)))
                return &LtlUntil::construct((ch == 'U') == negation, *left, *right);
        case EOF:
            fputs("unexpected end of file while parsing formula\n", stderr);
            return 0;
        default:
            fprintf(stderr, "unknown character 0x%02x\n", ch);
            return 0;
    }
}

/** Display the gates for arcs leaving a specific B�chi automaton state
 * @param gba		the generalized B�chi automaton
 * @param state		the state whose successor arcs are to be displayed
 */
static void
printGates(const class LtlGraph &gba, unsigned state) {
    for (LtlGraph::const_iterator n = gba.begin(); n != gba.end(); n++) {
        if (n->second.m_incoming.find(state) ==
            const_cast<std::set<unsigned> &>(n->second.m_incoming).end())
            continue;
        const class BitVector &propositions = n->second.m_atomic;
        printf("%u ", n->first);
        /** Previously encountered gate expression */
        const class LtlAtom *gate = 0;

        for (unsigned i = propositions.nonzero(); i;) {
            if (gate)
                printf(gate->isNegated() ? "& ! p%u " : "& p%u ",
                       gate->getValue());
            gate = &static_cast<const class LtlAtom &>(Ltl::fetch(i - 1));
            if (i) i = propositions.findNext(i);
        }
        if (gate)
            printf(gate->isNegated() ? "! p%u\n" : "p%u\n", gate->getValue());
        else
            puts("t");
    }
    puts("-1");
}

/** Translate a formula and output the automaton to standard output
 * @param formula	the formula to be translated
 */
static void
translateFormula(const class Ltl &formula) {
    /** The generalized Buchi automaton */
    class LtlGraph gba(formula);
    /** acceptance set number and proposition */
    typedef std::pair<unsigned, const class Ltl *> acceptance_set_t;
    typedef std::map<const class Ltl *, acceptance_set_t> acceptance_map_t;
    acceptance_map_t acceptance_sets;

    /** number of states */
    unsigned numStates = 0;
    /** iterator to states */
    LtlGraph::const_iterator s;

    // construct the acceptance sets
    for (s = gba.begin(); s != gba.end(); s++) {
        numStates++;
        const class BitVector &temporal = s->second.m_old;
        for (unsigned i = temporal.nonzero(); i;) {
            const class Ltl &f = Ltl::fetch(i - 1);
            switch (f.getKind()) {
                case Ltl::Until:
                    if (!static_cast<const class LtlUntil &>(f).isRelease())
                        acceptance_sets.insert(std::pair<const class Ltl *, acceptance_set_t>
                                                       (&f, acceptance_set_t
                                                               (acceptance_sets.size(),
                                                                &static_cast<const class LtlUntil &>
                                                                (f).getRight())));
                    break;
                case Ltl::Future:
                    if (static_cast<const class LtlFuture &>(f).getOp() ==
                        LtlFuture::finally)
                        acceptance_sets.insert(std::pair<const class Ltl *, acceptance_set_t>
                                                       (&f, acceptance_set_t
                                                               (acceptance_sets.size(),
                                                                &static_cast<const class LtlFuture &>
                                                                (f).getFormula())));
                    break;
                default:
                    break;
            }
            if (i) i = temporal.findNext(i);
        }
    }

    if (numStates) {
        printf("%u %u", numStates + 1, acceptance_sets.size());
        puts("\n0 1 -1");
        printGates(gba, 0);
    } else
        puts("0 0");

    for (s = gba.begin(); s != gba.end(); s++) {
        printf("%u 0", s->first);
        const class BitVector &temporal = s->second.m_old;

        // determine and display the acceptance sets the state belongs to
        for (acceptance_map_t::iterator a = acceptance_sets.begin();
             a != acceptance_sets.end(); a++) {
            /** flag: does the state belong to the acceptance set? */
            bool accepting = true;

            for (unsigned i = temporal.nonzero(); i;) {
                const class Ltl *f = &Ltl::fetch(i - 1);
                if (f == a->second.second) {
                    accepting = true;
                    break;
                } else if (f == a->first)
                    accepting = false;
                if (i) i = temporal.findNext(i);
            }

            if (accepting)
                printf(" %u", a->second.first);
        }

        puts(" -1");
        printGates(gba, s->first);
    }
}

Ltl *parserLTL(char *ltlStr) {
    globa_ltl_str_index = 0;
    global_ltl_str = ltlStr;
    /// parser the LTL from ltlStr
    Ltl * ltl = readFormula(false);
    return ltl;
}

Automata *translateLTL(char *ltlStr) {
    globa_ltl_str_index = 0;
    global_ltl_str = ltlStr;
    /// parser the LTL from ltlStr
    const Ltl * ltl = readFormula(false);
    /** The generalized Buchi automaton */
    class Automata *result = new Automata(*ltl);
    class Automata &gba = *result;

    /// set the initial state
    gba.initial_state = 0;

    gba.states.insert(gba.initial_state);

    /// add all the states first
    for (auto s = gba.begin(); s != gba.end(); s++) {
//        std::cout << s->first << "\n";
        gba.states.insert(s->first);
    }

    /// add all the final states
    /** acceptance set number and proposition */
    typedef std::pair<unsigned, const class Ltl *> acceptance_set_t;
    typedef std::map<const class Ltl *, acceptance_set_t> acceptance_map_t;
    acceptance_map_t acceptance_sets;

    /** number of states */
    unsigned numStates = 0;
    /** iterator to states */
    LtlGraph::const_iterator s;

    // construct the acceptance sets
    for (s = gba.begin(); s != gba.end(); s++) {
        numStates++;
        const class BitVector &temporal = s->second.m_old;
        for (unsigned i = temporal.nonzero(); i;) {
            const class Ltl &f = Ltl::fetch(i - 1);
            switch (f.getKind()) {
                case Ltl::Until:
                    if (!static_cast<const class LtlUntil &>(f).isRelease())
                        acceptance_sets.insert(std::pair<const class Ltl *, acceptance_set_t>
                                                       (&f, acceptance_set_t
                                                               (acceptance_sets.size(),
                                                                &static_cast<const class LtlUntil &>
                                                                (f).getRight())));
                    else if (static_cast<const class LtlUntil &>(f).isRelease()) {
                        acceptance_sets.insert(std::pair<const class Ltl *, acceptance_set_t>
                                                       (&f, acceptance_set_t
                                                               (acceptance_sets.size(),
                                                                &static_cast<const class LtlUntil &>
                                                                (f).getLeft())));
                    }
                    break;
                case Ltl::Future:
                    if (static_cast<const class LtlFuture &>(f).getOp() ==
                        LtlFuture::finally)
                        acceptance_sets.insert(std::pair<const class Ltl *, acceptance_set_t>
                                                       (&f, acceptance_set_t
                                                               (acceptance_sets.size(),
                                                                &static_cast<const class LtlFuture &>
                                                                (f).getFormula())));
                    break;
                default:
                    break;
            }
            if (i) i = temporal.findNext(i);
        }
    }

    /// add final states

    for (s = gba.begin(); s != gba.end(); s++) {
        const class BitVector &temporal = s->second.m_old;

        // determine and display the acceptance sets the state belongs to
        for (acceptance_map_t::iterator a = acceptance_sets.begin();
             a != acceptance_sets.end(); a++) {
            /** flag: does the state belong to the acceptance set? */
            bool accepting = true;

            for (unsigned i = temporal.nonzero(); i;) {
                const class Ltl *f = &Ltl::fetch(i - 1);
                if (f == a->second.second) {
                    accepting = true;
                    break;
                } else if (f == a->first)
                    accepting = false;
                if (i) i = temporal.findNext(i);
            }

            if (accepting) {
                gba.finalStates.insert(s->first);
            }
        }
    }

//    std::cout << gba.states.size() << "\n";
//    std::cout << gba.finalStates.size() << "\n";



    /// add all the transitions

    for (auto s : gba.states) {//= gba.begin(); s != gba.end(); s++) {
        int state = s;

        /// add the transition
        for (LtlGraph::const_iterator n = gba.begin(); n != gba.end(); n++) {
            if (n->second.m_incoming.find(state) ==
                const_cast<std::set<unsigned> &>(n->second.m_incoming).end())
                continue;
            const class BitVector &propositions = n->second.m_atomic;

            /// create the transition

            Transition *newTran = new Transition();
            newTran->source = state;
            newTran->target = n->first;

            const class LtlAtom *gate = 0;

            for (unsigned i = propositions.nonzero(); i;) {
                gate = &static_cast<const class LtlAtom &>(Ltl::fetch(i - 1));
                if (gate)
                    newTran->transition_atoms.insert(gate);
                if (i) i = propositions.findNext(i);
            }

            gba.transitions.insert(newTran);
        }

    }

    return &gba;
}

/*

int main(int argc, char *argv[]) {
    LTL_Property ltl_prop;

    // Information
    string filePath = "/home/weijiang/Github/lbt/property.txt";
    ifstream file;
    file.open(filePath, ios::in);
    if (!file.is_open())
        return 0;
    string strLine;
    int prop_flag = 0;
    while (getline(file, strLine)) {
        //        cout<<strLine <<endl;
        if (strLine.empty()) {
            prop_flag = 1;
            continue;
        }
        if (prop_flag == 0) {
            //  Atom information
            ltl_prop.Atom_Imp[strLine.substr(strLine.find("=") + 2)] = strLine.substr(0, strLine.find("=") - 1);
        } else {
            // Property Information
            string Prop_Imp = strLine;
            char *testLTL = (char *)Prop_Imp.data();
            ltl_prop.Automaton = translateLTL(testLTL);
        }
    }

//    cout << "Automaton: \n";
//    ltl_prop.Automaton->dump();
//    cout << "======================\n";


    int current_state = ltl_prop.Automaton->initial_state;
    string current_Props = "";
    set<unsigned int> result;
    set <int> current_atoms;
    while (current_Props!="quit"){
        result.clear();
        current_atoms.clear();
        cout << "current state: " << current_state << endl;
        cout << "next operation: \t";
        std::cin >> current_Props;
        if (ltl_prop.getAtom(current_Props) == "This atom is not defined."){
            cout << "This atom is not defined. Please provide a new atom.\n";
            cout << "----------" << endl;
            continue;
        }
        else{
            cout << "This operation is corresponded to the atom " << ltl_prop.getAtom(current_Props) << endl;
            current_atoms.insert(atoi(ltl_prop.getAtom(current_Props).substr(1).c_str()));
            ltl_prop.Automaton->transite(current_state, current_atoms, result);
            for (auto s : result) {
                cout << "candidated state: " << s;
                if (ltl_prop.Automaton->finalStates.find(s)!=ltl_prop.Automaton->finalStates.end()){
                    cout << "(accpet)\n";
                }
                else{
                    cout << "(reject)\n";
                }
            }
            cout << "Which one?" << endl;
            cin >> current_state;
            if (ltl_prop.Automaton->finalStates.find(current_state)!=ltl_prop.Automaton->finalStates.end()){
                cout << "congratulation!";
                break;
            }
            cout << "----------" << endl;
        }
    }
    return 0;
}*/






/*
std::set<Transition *> trans;
ltl_prop.Automaton->getTransition(0, trans);
for (auto t : trans) {
cout << t->source << endl;
cout << t->target << endl;
for (auto a : t->transition_atoms) {
std::cout << a->isNegated() << ": p" << a->getValue() << "\n";
}
}
 */
//    cout << ltl_prop.getAtom("send(1,2)") << endl;
//    cout << ltl_prop.getAtom("recv(2,3)") << endl;
//    cout << ltl_prop.getAtom("send(2,2)") << endl;