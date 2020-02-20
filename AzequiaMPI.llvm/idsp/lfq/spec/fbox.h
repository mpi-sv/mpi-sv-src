/*-
 * Copyright (c) 2009-2010 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _FBOX_H_
#define _FBOX_H_


/*----------------------------------------------------------------*
 *   Declaration of types used by this module                     *
 *----------------------------------------------------------------*/
#include <config.h>
#include <addr.h>
#include <util.h>


/*----------------------------------------------------------------*
 *   Exported constants                                           *
 *----------------------------------------------------------------*/
//#define FBOX_BUF_MAX 1024
//#define FBOX_MAX       16  


/*----------------------------------------------------------------*
 *   Exported types                                               *
 *----------------------------------------------------------------*/

/* FastBox */
struct fastBox {
  int   GlobalRank;      /* Global rank of Rank sending here */  
  int   Turn;
  int   SeqNr;
  int   Tag;
  char  Payload[FBOX_BUF_MAX];
  char  PayloadSize;
};
typedef struct fastBox fastBox__, *fastBox_t__;


/*----------------------------------------------------------------*
 *   Declaration of public interface implemented by this module   *
 *----------------------------------------------------------------*/
/*
#define RQST_feedFromFBox(rqst, fBox) \
{ \
  MEMCPY((rqst)->BuffInPtr, (fBox)->Payload, (fBox)->PayloadSize); \
  (rqst)->Status.Count    = (fBox)->PayloadSize; \
  (rqst)->Status.Src.Rank = (fBox)->GlobalRank; \
  (rqst)->Status.Tag      = (fBox)->Tag; \
}
*/
/*
//#define FBOX_init(&newthr->FastBox)
#define FBOX_init(fastBox) \
{ \
  int i; \
  fprintf(stdout, "\nFBOX_init: Begin\n"); fflush(stdout); \
  for(i = 0; i < FBOX_MAX; i++) { \
    fprintf(stdout, "FBOX_init: De rango local %d a rango global\n", i); fflush(stdout); \
    if(0 > ((fastBox)[i].GlobalRank = GRP_localRank2globalRank(i))) {\
      fprintf(stdout, "FBOX_init: Error\n"); fflush(stdout); \
      exit(1); \
    } \
  } \
  fprintf(stdout, "FBOX_init: End\n"); fflush(stdout); \
}
*/

#endif
