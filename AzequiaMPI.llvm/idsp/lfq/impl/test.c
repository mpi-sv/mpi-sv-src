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


/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
static const
       char  *e_names[8] = { /* This order has to be consistent with com.h */
                             /*  0 */ "COM_E_OK",
                             /*  1 */ "COM_E_EXHAUST",
                             /*  2 */ "COM_E_INTEGRITY",
                             /*  3 */ "COM_E_TIMEOUT",
                             /*  4 */ "COM_E_INTERFACE",
                             /*  5 */ "COM_E_SYSTEM",
                             /*  6 */ "COM_E_SIGNALED",
                             /*  7 */ "COM_E_DEADPART"
                           };
#define self()        ((Thr_t)pthread_getspecific(key))


/*----------------------------------------------------------------*
 *   Declaration of external functions                            *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Implementation of public interface                           *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    test                                                            |
    |    Test if an async request has finished                           |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst     (Input)                                              |
    |        The tested asynchronous request previously done             |
    |    o flag (Output)                                                 |
    |        TRUE  if the request is done                                |
    |        FALSE if the request not done                               |
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

int test   (Rqst_t *rqst, int *flag, RQST_Status *status)
{
  int       excpn;
  static
  char     *where       = "test";

  DBG_PRNT((stdout, "test(%p). Rqst: %p with state %x BEGIN\n", self(), *rqst, (*rqst)->State));

  /* 2. Test the request state */
  switch((*rqst)->State) {
    case RQST_PENDING:
    case RQST_PENDING + 1 :
    case RQST_FEEDING - 1 :
	case RQST_FEEDING:
	  *flag = FALSE;
	  if(RQST_isRecv(*(rqst))) {
		if(AZQ_getFromMBX(*(rqst)))
		  RPQ_remove(&(*rqst)->Owner->RecvPendReg, (*rqst));
		AZQ_progress((*rqst)->Owner);
	  }
	  break;
    case RQST_SATISFIED:
	  *flag = TRUE;
	  if(status != AZQ_STATUS_IGNORE) /* No hace falta, porque va dentro de RQST_getStatus */
		RQST_getStatus(*rqst, status);
	  if (RQST_isPersistent(*rqst))  { RQST_setInactive  (*rqst); }
	  break;
    case RQST_CANCELLED:
	  *flag = TRUE;
	  if (RQST_isPersistent(*rqst))  {
		if((*rqst)->PersistentWait)
		  *flag = FALSE;
		else
		  RQST_setInactive(*rqst);
	  }
	  break;
    default:
	  printf("test(%p). Rqst: %p with state %x\n", self(), *rqst, (*rqst)->State); fflush(stdout);
	  excpn = AZQ_E_INTEGRITY;
	  goto exception;
  }
  if(status != AZQ_STATUS_IGNORE)
    RQST_getStatus(*rqst, status);
  
  DBG_PRNT((stdout, "test(%p). Rqst: %p with state %x END\n", self(), *rqst, (*rqst)->State));

  return AZQ_SUCCESS; 
  
exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}

