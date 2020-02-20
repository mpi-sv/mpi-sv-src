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
#include <mbx.h>

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
#include <thr_dptr.h>


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   MBX public interface                                         *
 *----------------------------------------------------------------*/
#ifdef __MBX_DEBUG
void mbx_print (Header_t hdr) {
  fprintf(stdout, "\tMBX - Header: \n");
  fprintf(stdout, "\t\tSrc:    [0x%x,0x%x]\n", hdr->Src.Group, hdr->Src.Rank);
  fprintf(stdout, "\t\tTag:    %d\n", hdr->Tag);
  fprintf(stdout, "\t\tMode:   0x%x\n", hdr->Mode);
  fprintf(stdout, "\t\tCount:  %d\n", hdr->PayloadSize);
  fprintf(stdout, "\t\tSource THR  %p  RQST  %p: \n", hdr->SrcThr, hdr->SrcRqst);
  if (!(hdr->Mode & MODE_REMOTE))
    fprintf(stdout, "\t\tRqst state:   0x%x\n", ((Rqst_t)(hdr->SrcRqst))->State);
}
#endif

