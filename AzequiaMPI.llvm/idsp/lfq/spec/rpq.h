/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef  _RPQ_H
#define  _RPQ_H

/*----------------------------------------------------------------*
 *   Declaration of types used by this module                     *
 *----------------------------------------------------------------*/
#include <addr.h>
#include <rqst.h>


/*----------------------------------------------------------------*
 *   Definition of public interface                               *
 *----------------------------------------------------------------*/


      /*________________________________________________________________
     /                                                                  \
    |    RPQ_put                                                         |
    |                                                                    |
    |    Register a request to wait completion in the thread register    |
    |    of pending requests                                             |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst        (Input)                                           |
    |        The request to register                                     |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define RPQ_put(rpq, rqst)                                           \
{                                                                    \
  DLQ_put((rpq), (rqst))                                             \
}




      /*________________________________________________________________
     /                                                                  \
    |    RPQ_get                                                         |
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
#define RPQ_get(rpq, rqst, addr, tag, mode, success)                    \
{                                                                       \
  DLQ_findRPQ((rpq), (rqst), (addr), (tag), 0, (mode), (success))          \
}



      /*________________________________________________________________
     /                                                                  \
    |    RPQ_remove                                                      |
    |                                                                    |
    |    Unregister a request from the thread register of pending        |
    |    requests                                                        |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst        (Input)                                           |
    |        The request to unregister                                   |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define RPQ_remove(rpq, rqst)                                        \
{                                                                    \
  DLQ_delete((rpq), (rqst))                                          \
}


/*----------------------------------------------------------------*
 *   Declaration of public interface                              *
 *----------------------------------------------------------------*/
extern int   rpq_match     (void *elem, Addr_t addr, Tag_t tag, int type, int mode);


#endif

