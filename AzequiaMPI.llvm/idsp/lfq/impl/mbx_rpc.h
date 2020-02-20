/*-
 * Copyright (c) 2011 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */
#ifndef  _MBX_RPC_H
#define  _MBX_RPC_H

/*----------------------------------------------------------------*
 *   Declaration of types used by this module                     *
 *----------------------------------------------------------------*/
#include <addr.h>
#include <mbx.h>
#include <rqst.h>
#include <dlq.h>


/*----------------------------------------------------------------*
 *   Definition of public interface                               *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    MBX_lookUpRpc                                                   |
    |    RETURNS: Message from "src" with "tag" tag.                     |
    |                                                                    |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o mbox (Input)                                                  |
    |                                                                    |
    |    o src  (Input / Output)                                         |
    |        First search key                                            |
    |    o tag  (Input)                                                  |
    |        Second search key                                           |
    |    o outtag  (Output)                                              |
    |         received tag if wildcard in request                        |
    |    o count  (Output)                                               |
    |         Size of the message (for later buffer allocation purposes) |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define MBX_lookUpRpc(mbox, hdr, keyAddr, keyTag, success)                     \
{                                                                           \
  DLQ_findMBXrpc((mbox), (hdr), (keyAddr), (keyTag), 0, (mode), (success));  \
} 


      /*________________________________________________________________
     /                                                                  \
    |    MBX_getRpc                                                      |
    |                                                                    |
    |    Dequeue from the mailbox "mbox" a message descriptor            |
    |    that matches the "src" and "tag" search keys                    |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o mbox (Input)                                                  |
    |        The mailbox                                                 |
    |    o hdr  (Output)                                                 |
    |        The descriptor of the message (if found)                    |
    |    o src  (Input)                                                  |
    |        First search key                                            |
    |    o tag  (Input)                                                  |
    |        Second search key                                           |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define MBX_getRpc(mbox, hdr, keyAddr, keyTag, mode, success)                  \
{                                                                    \
  DLQ_findMBXrpc((mbox), (hdr), (keyAddr), (keyTag), 0, (mode), (success));        \
  if(*(success))                                                    \
    DLQ_delete((mbox), *(hdr));                                      \
} 


#endif
