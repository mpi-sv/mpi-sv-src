/*-
 * Copyright (c) 2009-2010 Universidad de Extremadura
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
#include <rpq.h>
#include <rqst.h>
#include <util.h>
#include <com.h>


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/
#ifdef USE_FASTBOXES
void AZQ_FBX_lookUp(Thr_t me, fastBox_t *fbx, int keyRank, int keyTag, int *fbxSuccess)
{ 
  fastBox_t fBox; 
  
  *fbxSuccess = 0; 
  if(keyRank != ADDR_RNK_ANY) { 
    /* 1. Map keyRank to the fastBox in which looking up by keyTag */ 
    Thr_t srcThr     = ((me)->GrpInfo)[keyRank].Thr; 
    int srcLocalRank = srcThr->LocalRank; 
    fBox             = &(me)->FastBox[srcLocalRank]; 
	
    if( fBox->Turn == TURN_RECV ) { 
      /* 2. Test the tag */ 
      if(TAG_MATCH((keyTag), fBox->Tag)) { 
        if(fBox->SeqNr == (1 + (me)->LastRecvSeqNr[srcLocalRank])) { 
          *fbx = fBox;
          *fbxSuccess = 1; 
          return; 
        } 
      } 
    } 
  } else { 
    int lRank; 
    /* Sweep all the fast boxes */ 
    for(lRank = 0; lRank < (me)->LocalGroupSize; lRank++) { 
      fBox = &(me)->FastBox[lRank]; 
      if( fBox->Turn == TURN_RECV ) { 
        if(TAG_MATCH(keyTag, fBox->Tag)) { 
          if(fBox->SeqNr == (1 + (me)->LastRecvSeqNr[lRank])) { 
            *fbx = fBox;
            *fbxSuccess = 1; 
            return; 
          } 
        } 
      } 
    } 
  } 
  return;
} 
#endif

/*----------------------------------------------------------------*
 *   Implementation of public interface                           *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    AZQ_probe                                                       |
    |    Test if given src address & tag, a matching whole message       |
    |    (all chunks) is found at thread maibox, else block.             |
    |                                                                    |
    |    RETURNS: 0 if OK                                                |
    |    PARAMETERS:                                                     |
    |    o keyRank (Input)                                               |
    |        The tested message src address                              |
    |    o tag (Input)                                                   |
    |        Tested message tag                                          |
    |    o status (Output)                                               |
    |        Status of pending receive communication                     |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int AZQ_probe (const int keyRank, const Tag_t keyTag, RQST_Status *status) 
{
  Thr_t     me      = self();
  Rqst     *rqst;
  Header_t  hdr;
  int       success;
  
  DBG_PRNT((stdout, "\nprobe(%p): From [%d]. Tag %d. \n", self(), keyRank, keyTag));
  //fprintf(stdout, "AZQ_probe(%p): From [%d]. Tag %d. BEGIN\n", self(), keyRank, keyTag);
  
#ifdef USE_FASTBOXES
  /* 1. Find a message in fastBoxes */
  fastBox_t fBox; 
  AZQ_FBX_lookUp(me, &fBox, keyRank, keyTag, &success); 
  if(success) {  
    if(status != AZQ_STATUS_IGNORE) {                    
      (status)->Count    = (fBox)->MessageSize; 
      (status)->Src.Rank = (fBox)->SenderGlobalRank; 
      (status)->Tag      = (fBox)->Tag; 
    }
	
    DBG_PRNT((stdout, "probe(%p): Found in Fastbox from [%d]. Tag %d. Count %d\n", self(),
			  status->Src.Rank, status->Tag, status->Count));
    goto retorno;
  }
#endif
  
  AZQ_progress(me);
  rqst = &me->SyncRqst;
  RQST_initProbeRecv(rqst,  keyRank, keyTag, NULL, 0, me);
  if(AZQ_testFromMBX(rqst, status)) {
    goto retorno;
  }
  
  DBG_PRNT((stdout, "probe(%p): Registering %p on RPQ %p\n", me, rqst, &me->RecvPendReg));
  RPQ_put(&me->RecvPendReg, rqst);
  waitone(&rqst, status);

retorno:
  //fprintf(stdout, "AZQ_probe(%p): END\n", self());
  DBG_PRNT((stdout, "probe(%p): END\n", me));
  return AZQ_SUCCESS;
}



