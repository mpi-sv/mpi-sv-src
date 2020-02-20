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
(                                                                    \
  DLQ_create((mbox), (cmp_fxn))                                      \
)


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
(                                                                    \
  DLQ_destroy((mbox))                                                \
)


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
(                                                                    \
  DLQ_put((mbox), (hdr))                                             \
)


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
(                                                                    \
  DLQ_delete((mbox), (hdr))                                          \
)


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
#define MBX_lookUp(mbox, hdr, src, tag)                                    \
(                                                                          \
  DLQ_find((mbox), ((void **)hdr), (src), (tag), RQST_ANY, 0)              \
)


/*----------------------------------------------------------------*
 *   Declaration of public interface                              *
 *----------------------------------------------------------------*/
extern int   mbx_match     (void *elem, Addr_t addr, Tag_t tag, int type, int mode);

extern int   MBX_get       (DLQ_t  mbox, Header_t *hdr, Addr_t src, Tag_t tag, int type);

#endif

