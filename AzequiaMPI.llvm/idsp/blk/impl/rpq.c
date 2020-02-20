/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   implemented by this module                                   *
 *----------------------------------------------------------------*/
#include <rpq.h>

/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
#endif
#include <azq_types.h>
#include <addr_hddn.h>
#include <thr_dptr.h>


#ifdef __RPQ_DEBUG
#define DBG_PRNT(pmsg)  fprintf pmsg
#else
#define DBG_PRNT(pmsg)
#endif

/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/
#ifdef __RPQ_DEBUG

extern void rqst_print (Rqst_t rqst);

void rpq_print (Header_t hdr) {
  fprintf(stdout, "\tMBX - Header: \n");
  fprintf(stdout, "\t\tSrc:    [%d,%d]\n", hdr->Src.Group, hdr->Src.Rank);
  fprintf(stdout, "\t\tTag:    %d\n", hdr->Tag);
  fprintf(stdout, "\t\tMode:   0x%x\n", hdr->Mode);
  fprintf(stdout, "\t\tCount:  %d\n", hdr->PayloadSize);
  fprintf(stdout, "\t\tSource THR  0x%x  RQST  0x%x: \n", hdr->SrcThr, hdr->SrcRqst);
  fprintf(stdout, "\t\tRqst state:   0x%x\n", ((Rqst_t)(hdr->SrcRqst))->State);
}
#endif

/*----------------------------------------------------------------*
 *   Rquest Pending Queue public interface                        *
 *----------------------------------------------------------------*/


      /*________________________________________________________________
     /                                                                  \
    |    rpq_match                                                       |
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
int rpq_match (void *elem, Addr_t addr, Tag_t tag, int type, int mode) {

#ifdef __RPQ_DEBUG
  rqst_print((Rqst_t)elem);
#endif

  return (RQST_match((Rqst_t) elem, addr, tag, mode));
}
