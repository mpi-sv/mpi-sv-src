/*-
 * Copyright (c) 2009-2011 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   implemented by this module                                   *
 *----------------------------------------------------------------*/
#include <com.h>

/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#include <config.h>

#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
  #include <string.h>
  #include <stdio.h>
  #include <errno.h>
#endif

#include <azq_types.h>
#include <xpn.h>
#include <addr.h>
#include <inet.h>
#include <thr.h>
#include <thr_dptr.h>
#include <mbx.h>
#include <rqst.h>
#include <util.h>
#include <com.h>


/*----------------------------------------------------------------*
 *   Declaration of external functions                            *
 *----------------------------------------------------------------*/
#ifdef USE_FASTBOXES
void AZQ_FBX_lookUp(Thr_t me, fastBox_t *fbx, int keyRank, int keyTag, int *fbxSuccess);
#endif


/*----------------------------------------------------------------*
 *   Implementation of public interface                           *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    aprobe                                                          |
    |    Test if given src address & tag, a matching whole message       |
    |    (all chunks) is found at thread maibox                          |
    |                                                                    |
    |    RETURNS: 0 if OK                                                |
    |    PARAMETERS:                                                     |
    |    o keySrc (Input)                                                |
    |        The tested message src address                              |
    |    o keyTag (Input)                                                |
    |        Tested message tag                                          |
    |    o flag (Output)                                                 |
    |        TRUE  if the whole hdr was received                         |
    |        FALSE if the whole hdr was not received                     |
    |    o status (Output)                                               |
    |        Status of pending receive communication                     |
    |                                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int aprobe(const int keyRank, const Tag_t keyTag, int *flag, RQST_Status *status) 
{
  Thr_t     me     = self();
  Header_t  hdr;
  Rqst      rqst;
  int       success;
  
  DBG_PRNT((stdout, "aprobe(%p): From [%d]. Tag %d\n", self(), keyRank, keyTag));
  
  *flag = FALSE;
  
#ifdef USE_FASTBOXES
  fastBox_t fBox; 
  AZQ_FBX_lookUp(me, &fBox, keyRank, keyTag, &success); 
  if(success) {  
    *flag = TRUE;
    if(status != AZQ_STATUS_IGNORE) {
      (status)->Count    = (fBox)->MessageSize; 
      (status)->Src.Rank = (fBox)->SenderGlobalRank; 
      (status)->Tag      = (fBox)->Tag; 
    }
    DBG_PRNT((stdout, "aprobe(%p): Found in FBX from [%d]. Tag %d. Count %d\n", self(),
			  status->Src.Rank, status->Tag, status->Count));
    goto retorno;
  }
#endif
  
  
  /* 2. Find a message in MailBox */
  AZQ_progress(me);
  RQST_initProbeRecv(&rqst,  keyRank, keyTag, NULL, 0, me);
  if(AZQ_testFromMBX(&rqst, status)) {
    *flag = TRUE;
  }
  
retorno:
  DBG_PRNT((stdout, "aprobe(%p): End\n", self()));
  return AZQ_SUCCESS;
}

