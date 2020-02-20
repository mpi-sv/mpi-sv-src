/* _________________________________________________________________________
   |                                                                       |
   |  Azequia (embedded) Message Passing Interface   ( AzequiaMPI )        |
   |                                                                       |
   |  Authors: DSP Systems Group                                           |
   |           http://gsd.unex.es                                          |
   |           University of Extremadura                                   |
   |           Caceres, Spain                                              |
   |           jarico@unex.es                                              |
   |                                                                       |
   |  Date:    Sept 22, 2008                                               |
   |                                                                       |
   |  Description:                                                         |
   |                                                                       |
   |                                                                       |
   |_______________________________________________________________________| */

#ifndef	COMMON_H
#define	COMMON_H

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <config.h>

#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
#endif

#include <mpi.h>

  /*-------------------------------------------------------/
 /           Declaration of public functions              /
/-------------------------------------------------------*/

/* Init */
extern int   common_init    (int *argc, char **argv[], int thread_support);

/* Communicators */
extern int   comm_create    (MPI_Comm intracomm, MPI_Group local_group, MPI_Group remote_group,
							 int context, int type, void *errhnd, 
							 MPI_Comm *newcomm);

extern int   comm_free      (MPI_Comm *comm);


#endif
