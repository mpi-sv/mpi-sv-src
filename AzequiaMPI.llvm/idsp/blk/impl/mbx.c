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
#include <addr_hddn.h>
#include <thr_dptr.h>


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/
#ifdef __MBX_DEBUG
void mbx_print (Header_t hdr) {
  fprintf(stdout, "\tMBX - Header: \n");
  fprintf(stdout, "\t\tSrc:    [%d,%d]\n", hdr->Src.Group, hdr->Src.Rank);
  fprintf(stdout, "\t\tTag:    %d\n", hdr->Tag);
  fprintf(stdout, "\t\tMode:   0x%x\n", hdr->Mode);
  fprintf(stdout, "\t\tCount:  %d\n", hdr->PayloadSize);
  fprintf(stdout, "\t\tSource THR  %p  RQST  %p: \n", hdr->SrcThr, hdr->SrcRqst);
  if (!(hdr->Mode & MODE_REMOTE))
    fprintf(stdout, "\t\tRqst state:   0x%x\n", ((Rqst_t)(hdr->SrcRqst))->State);
}
#endif

/*----------------------------------------------------------------*
 *   MBX public interface                                         *
 *----------------------------------------------------------------*/

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
int mbx_match (void *elem, Addr_t addr, Tag_t tag, int type, int mode) {

  Header_t hdr = (Header_t)elem;

#ifdef __MBX_DEBUG
  mbx_print(hdr);
#endif

//  if ((ADDR_match(&hdr->Src, addr)) && (TAG_MATCH(tag, hdr->Tag)) && ((type & RQST_ANY) ? 1 : hdr->Mode & type))
//    return TRUE;

  if ((ADDR_match(&hdr->Src, addr)) &&
      (TAG_MATCH(tag, hdr->Tag))    &&
      ((hdr->Mode & MODE_REMOTE) ? ((type & RQST_MASK) & (hdr->Mode)) : 1))
    return TRUE;

  return FALSE;
}


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
int MBX_get(DLQ_t mbox, Header_t *hdr, Addr_t src, Tag_t tag, int type) {

  if (DLQ_find(mbox, (void **)hdr, src, tag, type, 0)) {
    DLQ_delete(mbox, *hdr);
    return (TRUE);
  }

  *hdr = NULL;

  return(FALSE);
}


