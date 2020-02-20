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


#ifndef ERRHND_H
#define ERRHND_H

#include <config.h>
#include <mpi.h>

extern void  FatalErrors   (MPI_Comm *comm, int *errcode, char *where);
extern void  ReturnErrors  (MPI_Comm *comm, int *errcode, char *where);


#endif
