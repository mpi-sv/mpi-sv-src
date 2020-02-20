//
// Created by zbchen on 12/27/19.
//

#ifndef MPISV_LBT_H
#define MPISV_LBT_H

#include "LtlGraph.h"

extern Automata *translateLTL(char *fileStr);

extern Ltl *parserLTL(char *ltlStr);

#endif //MPISV_LBT_H
