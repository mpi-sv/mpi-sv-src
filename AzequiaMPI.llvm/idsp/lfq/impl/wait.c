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
#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
  #include <string.h>
  #include <stdio.h>
  #include <errno.h>
  #include <stdlib.h>
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
#include <rpq.h>
#include <lfq.h>


/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
#define self()        ((Thr_t)pthread_getspecific(key))


/*----------------------------------------------------------------*
 *   Declaration of external functions                            *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Implementation of public interface                           *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    waitone                                                         |
    |    Block the invoking thread until                                 |
    |    1) The request                                                  |
    |       a)  is satisfied                                             |
    |       b)  times out                                                |
    |    2) It is killed                                                 |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst     (Input)                                              |
    |        Request                                                     |
    |    o status   (Output)                                             |
    |        Status of the satisfied request                             |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 0 : On success                                                |
    |    < 0 : On other case                                             |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int AZQ_waitone(Rqst_t *rqst, RQST_Status *status)  
{
  /*fprintf(stdout, "waitone(%p). Rqst %p (state %x) BEGIN\n",  self(), *(rqst), (*rqst)->State); fflush(stdout);*/

  RQST_setWaiting(*(rqst)); 
  switch((*rqst)->State) { 
    case RQST_PENDING     :
    case RQST_PENDING + 1 :
    case RQST_FEEDING - 1 :
    case RQST_FEEDING     :   
	  if(RQST_isRecv(*rqst) && AZQ_getFromMBX(*rqst)) {
		RPQ_remove(&((Thr_t)((*rqst)->Owner))->RecvPendReg, *rqst);
		/*fprintf(stdout, "waitone(%p). Rqst %p removed for RPQ\n",  self(), *rqst); fflush(stdout);*/
	  }
	  while ((*rqst)->State != RQST_SATISFIED) 
		AZQ_progress((*(rqst))->Owner); 
	case RQST_SATISFIED: 
	  if((status) != AZQ_STATUS_IGNORE) { 
		RQST_getStatus(*(rqst), (status)); 
	  } 
	  if (RQST_isPersistent(*(rqst)))  { RQST_setInactive(*(rqst)); } 
	  break; 
    case RQST_INACTIVE:  
	  if((status) != AZQ_STATUS_IGNORE)  
		RQST_setEmptyStatus((status)); 
	  break;
    case RQST_CANCELLED: 
	  if((status) != AZQ_STATUS_IGNORE) 
		RQST_getStatus(*(rqst), (status)); 
	  if (RQST_isPersistent(*(rqst)))  {   
        //fprintf(stdout, "waitone(%p). Rqst: %p CANCELLED and persistent\n", self(), *(rqst)); fflush(stdout); 
		while ((*(rqst))->PersistentWait) {  
		  AZQ_progress((Thr_t)((*(rqst))->Owner)); 
		} 
		RQST_setInactive(*(rqst));  
	  } 
	  break; 
    default: 
	  /*fprintf(stdout, "waitone(%p). Rqst %p with state %x \n",  self(), *(rqst), (*rqst)->State); fflush(stdout); */
	  panic("waitone"); 
  } 
  RQST_unsetWaiting(*(rqst)); 

  /*fprintf(stdout, "waitone(%p). Rqst: %p with state %x   END\n", self(), *(rqst), (*(rqst))->State); fflush(stdout); */

  return(AZQ_SUCCESS); 
} 


