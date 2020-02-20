/*-
 * Copyright (c) 2011 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef  _RPQ_RPC_H
#define  _RPQ_RPC_H

/*----------------------------------------------------------------*
 *   Declaration of types used by this module                     *
 *----------------------------------------------------------------*/
#include <addr.h>
#include <rqst.h>
#include <rpq.h>


/*----------------------------------------------------------------*
 *   Definition of public interface                               *
 *----------------------------------------------------------------*/


      /*________________________________________________________________
     /                                                                  \
    |    RPQ_getRpc                                                      |
    |                                                                    |
    |    Get a request from the thread pending request register          |
    |    by Address and tag (not remove)                                 |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o dstThr       (Input)                                          |
    |        Thread where put the request                                |
    |    o rqst         (Output)                                         |
    |        Request, if found                                           |
    |    o dstAddr, tag (Input)                                          |
    |        Search keys                                                 |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 1 : if rqst found                                             |
    |    = 0 : if not found                                              |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define RPQ_getRpc(rpq, rqst, addr, tag, mode, success)                    \
{                                                                       \
  DLQ_findRPQrpc((rpq), (rqst), (addr), (tag), 0, (mode), (success))          \
}


#endif

