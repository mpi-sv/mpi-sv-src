/*-
 * Copyright (c) 2011 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


/*
   Double Linked Queue extension
 */

#ifndef _DLQ_RPC_H
#define _DLQ_RPC_H

#include <addr.h>
#include <dlq.h>


#define DLQ_findRPQrpc(dlq, item, keyAddr, keyTag, type, mode, success)    \
{\
  /*fprintf(stdout, "DLQ_findRPQrpc: wanted Rank = %d Tag = %x\n", (keyAddr)->Rank, keyTag); fflush(stdout); */ \
  *(success) = FALSE;                                 \
  if (!DLQ_isEmpty(dlq)) {                                      \
    DLQ_Link_t   lprev = &((dlq)->Queue);                     \
    DLQ_Link_t   link  = (dlq)->Queue.Next;                   \
    do {                                      \
      /*fprintf(stdout, "Link = 0x%x\n", link); fflush(stdout);*/ \
      /*fprintf(stdout, "RQST_matchRpc: [%d  %x  %x] with link [%d  %x  %x]\n", \
               (keyAddr)->Rank, (keyTag), (mode), \
               ((Rqst_t)link)->Status.Src.Rank, ((Rqst_t)link)->Status.Tag, ((Rqst_t)link)->Hdr.Mode); \
               fflush(stdout); */ \
      if (RQST_matchRpc((Rqst_t)link, (keyAddr), (keyTag), (mode))) { \
        *(item)    = (Rqst_t)link;                              \
        *(success) = TRUE;                          \
        /*fprintf(stdout, "DLQ_findRPQrpc: ¡Found!\n"); fflush(stdout); */  \
        break;                                                   \
      }                                                          \
      link = link->Next;                                         \
    } while (link != lprev);                                      \
  }\
}


#define DLQ_findMBXrpc(dlq, item, keyAddr, keyTag, type, mode, success)    \
{                                                           \
  /*fprintf(stdout, "DLQ_findMBXrpc: wanted Rank = %d Tag = %x\n", (keyAddr)->Rank, (keyTag)); fflush(stdout);*/ \
  *(success) = FALSE;                                 \
  if (!DLQ_isEmpty(dlq)) {                                      \
    DLQ_Link_t         lprev  = &((dlq)->Queue);                   \
    DLQ_Link_t         link   = (dlq)->Queue.Next;                \
    do {    \
      /*fprintf(stdout, "RPC_DLQ_findMBXrpc: Link = 0x%x\n", link); fflush(stdout); */\
	  /*fprintf(stdout, "MBX_matchRpc (%p): [%d  %x  %x] with link [%d  %x  %x]\n", \
              self(), (keyAddr)->Rank, (keyTag), (mode), \
              ((Header_t)link)->Src.Rank, ((Header_t)link)->Tag, ((Header_t)link)->Mode); \
      fflush(stdout); */\
      if ( \
             (ADDR_match( &((Header_t)link)->Src, (keyAddr)            )      )   && \
             (TAG_MATCH ( keyTag,                 ((Header_t)link)->Tag)      )   && \
             ((mode) & (((Header_t)link)->Mode)) \
         ) { \
        *(item)    = (Rqst_t)link;     \
        *(success) = TRUE;             \
        /*fprintf(stdout, "DLQ_findMBX: ¡Encontrada!\n"); fflush(stdout);*/   \
        break;                                                   \
      }                                                          \
      link = link->Next;                     \
    } while (link != lprev);                                     \
  }                                                            \
  /*fprintf(stdout, "RPC_DLQ_findMBX: (end)\n");  fflush(stdout); */\
}


#define old_DLQ_findMBXrpc(dlq, item, keyAddr, keyTag, type, mode, success)    \
{                                                           \
/*fprintf(stdout, "RPC_DLQ_findMBX: wanted Rank = %x\n", (keyAddr)); fflush(stdout);*/ \
/*fprintf(stdout, "RPC_DLQ_findMBX: wanted Tag  = %x\n", (keyTag));  fflush(stdout); */\
*(success) = FALSE;                                 \
if (!DLQ_isEmpty(dlq)) {                                      \
DLQ_Link_t         lprev  = &((dlq)->Queue);                   \
DLQ_Link_t         link   = (dlq)->Queue.Next;                \
do {    \
/*fprintf(stdout, "RPC_DLQ_findMBXrpc: Link = 0x%x\n", link); fflush(stdout); */\
if ( \
( (ADDR_match( &((Header_t)link)->Src, (keyAddr)            )      )   && \
(TAG_MATCH ( keyTag,                 ((Header_t)link)->Tag)      )   && \
/*(HDR_isLocal((Header_t)link) ? 1 : ((mode) & (((Header_t)link)->Mode))) */\
((mode) & (((Header_t)link)->Mode)) \
) \
? 1 : 0 \
) { \
*(item)    = (Rqst_t)link;     \
*(success) = TRUE;             \
/*fprintf(stdout, "DLQ_findMBX: ¡Encontrada!\n"); fflush(stdout);*/   \
break;                                                   \
}                                                          \
link = link->Next;                     \
} while (link != lprev);                                     \
}                                                            \
}

#endif

