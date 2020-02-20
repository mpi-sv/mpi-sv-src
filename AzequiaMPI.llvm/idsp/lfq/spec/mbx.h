/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */
#ifndef  _MBX_H
#define  _MBX_H

/*----------------------------------------------------------------*
 *   Declaration of types used by this module                     *
 *----------------------------------------------------------------*/
#include <addr.h>
#include <rqst.h>
#include <dlq.h>


/*----------------------------------------------------------------*
 *   Definition of public interface                               *
 *----------------------------------------------------------------*/


      /*________________________________________________________________
     /                                                                  \
    |    MBX_create                                                      |
    |                                                                    |
    |    Create a message mailbox queue                                  |
    |                                                                    |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o mbox (Input)                                                  |
    |        The target mailbox to create                                |
    |    o cmp_fxn (Input)                                               |
    |        Upcall function for comparing messages                      |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define MBX_create(mbox, cmp_fxn)                                    \
{                                                                    \
  DLQ_create((mbox), (cmp_fxn))                                      \
}


      /*________________________________________________________________
     /                                                                  \
    |    MBX_destroy                                                     |
    |                                                                    |
    |    Free a message mailbox queue                                    |
    |                                                                    |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o mbox (Input)                                                  |
    |        The target mailbox to free                                  |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define MBX_destroy(mbox)                                            \
{                                                                    \
  DLQ_destroy((mbox))                                                \
}


      /*________________________________________________________________
     /                                                                  \
    |    MBX_put                                                         |
    |                                                                    |
    |    Enqueue arrived message in the target mailbox                   |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o mbox (Input)                                                  |
    |        The target thread mailbox                                   |
    |    o hdr (Input)                                                   |
    |        The descriptor of the arrived message                       |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define MBX_put(mbox, hdr)                                           \
{                                                                    \
  DLQ_put((mbox), (hdr))                                            \
}


      /*________________________________________________________________
     /                                                                  \
    |    MBX_remove                                                      |
    |                                                                    |
    |    Remove a message from its mailbox                               |
    |                                                                    |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o mbox (Input)                                                  |
    |        The target mailbox the message is enqueued in               |
    |    o hdr (Input)                                                   |
    |        The descriptor of the message to be removed                 |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define MBX_remove(mbox, hdr)                                        \
{                                                                    \
  DLQ_delete((mbox), (hdr))                                          \
}


      /*________________________________________________________________
     /                                                                  \
    |    MBX_lookUp                                                      |
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
#define MBX_lookUp(mbox, hdr, keyRank, keyTag, mode, success)                  \
{                                                                 \
  DLQ_findMBX((mbox), (hdr), (keyRank), (keyTag), (mode), (success));  \
} 


      /*________________________________________________________________
     /                                                                  \
    |    mbx_match                                                       |
    |                                                                    |
    |      For find in the queue of request pending, it needs to install |
    |    this funtion in DLQ                                             |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o elem     (Input)                                              |
    |        Request to compare to                                       |
    |    o addr     (Input)                                              |
    |        Expected source address                                     |
    |    o tag      (Input)                                              |
    |        Expected tag                                                |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 1 : if found                                                  |
    |    = 0 : Not found                                                 |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
/*
#define mbx_match(elem, keyRank, keyTag, type, mode) \
{ \
  RANK_match(((Header_t)(elem))->Src.Rank, (keyRank)) \
  && \
  TAG_MATCH((keyTag), ((Header_t)(elem))->Tag) \
  && \
  ( \
      (!(((Header_t)(elem))->Mode & MODE_REMOTE)) ? \
                                                1 \
                                                : \
                                                ((type) & RQST_MASK) & (((Header_t)(elem))->Mode) \
  )    \
}
*/
#define mbx_match(elem, keyRank, keyTag, type, mode) \
( \
  RANK_match(((Header_t)(elem))->Src.Rank, (keyRank)) \
  && \
  TAG_MATCH((keyTag), ((Header_t)(elem))->Tag) \
)


      /*________________________________________________________________
     /                                                                  \
    |    MBX_get                                                         |
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
#define MBX_get(mbox, hdr, keyRank, keyTag, type, success)                  \
{                                                                           \
  DLQ_findMBX((mbox), (hdr), (keyRank), (keyTag), (type), 0, (success));    \
  if(*(success))                                                            \
    DLQ_delete((mbox), *(hdr));                                             \
} 



      /*________________________________________________________________
     /                                                                  \
    |    MBX_print                                                      |
    |                                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#ifdef __MBX_DEBUG
#define  MBX_print(hdr) { \
  fprintf(stdout, "\tMBX - Header: \n"); \
  fprintf(stdout, "\t\tSrc:    [0x%x]\n", (hdr)->Src.Rank); \
  fprintf(stdout, "\t\tTag:    %d\n", (hdr)->Tag); \
  fprintf(stdout, "\t\tMode:   0x%x\n", (hdr)->Mode); \
  fprintf(stdout, "\t\tCount:  %d\n", (hdr)->PayloadSize); \
  fprintf(stdout, "\t\tSource THR  %p  RQST  %p: \n", (hdr)->SrcThr, (hdr)->SrcRqst); \
  if (!((hdr)->Mode & MODE_REMOTE)) \
    fprintf(stdout, "\t\tRqst state:   0x%x\n", ((Rqst_t)((hdr)->SrcRqst))->State); \
}
#endif

#endif
