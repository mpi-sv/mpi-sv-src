/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _AZQ_H_
#define _AZQ_H_


/*----------------------------------------------------------------*
 *   Declaration of public constants                              *
 *----------------------------------------------------------------*/
#define AZQ_VERSION       "2.1"
#define AZQ_REVISION      "7"

/*----------------------------------------------------------------*
 *   Declaration of public interface                              *
 *----------------------------------------------------------------*/
#include <grp.h>
#include <rpc.h>
#include <com.h>
#include <thr.h>
#include <opr.h>
#include <azq_types.h>


extern int  AZQ_init  (REG_Entry *oprTable, int oprNr);

#endif
