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

  /*----------------------------------------------------------------/
 /   Declaration of public functions implemented by this module    /
/----------------------------------------------------------------*/
#include <errhnd.h>

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#if defined (__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <string.h>
#endif

#include <com.h>

#include <mpi.h>
#include <p_errhnd.h>
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Error_string
#define MPI_Error_string  PMPI_Error_string
#endif

  /*----------------------------------------------------------------/
 /            Definition of data used by this module               /
/----------------------------------------------------------------*/
char *errStrings [MPI_ERR_LASTCODE] = {
  "No error",
  "",
  "Invalid buffer pointer",
  "Invalid count argument",
  "Invalid datatype argument",
  "Invalid message tag",
  "Invalid communicator",
  "Invalid rank",
  "Invalid request",
  "Invalid root",
  "Invalid group",
  "Invalid collective operation",
  "Invalid topology",
  "Illegal dimension argument",
  "Invalid argument",
  "Unknown error",
  "Message truncated",
  "", /* No default message for OTHER. Need code error */
  "Internal error",
  "Error code in Status",
  "Pending request (no error)"
};


  /*----------------------------------------------------------------/
 /   Declaration of private functions used by this module          /
/----------------------------------------------------------------*/

/*
 *  FatalErrors():
 *    Default MPI_ERRORS_ARE_FATAL error handler
 */
void FatalErrors (MPI_Comm *comm, int *errcode, char *where) {

  int rank;

  MPI_Comm_rank(*comm, &rank);

#ifdef VERBOSE_MODE
  fprintf(stderr, "\nMPI_ERRORS_ARE_FATAL handler. ");
  fprintf(stderr, "Error: %s\n", errStrings[*errcode]);
  fprintf(stderr, "Function: %s \t", where);
  fprintf(stderr, "Error code: %d\tError class: %d\n", *errcode, *errcode);
  fprintf(stderr, "Rank: %d\t", rank);
  fprintf(stderr, "Communicator: 0x%x (CommNR %d)\n\n", (unsigned int)*comm, commGetContext((Mpi_P_Comm *)*comm));
#endif

  MPI_Abort(*comm, *errcode);
}


/*
 *  ReturnErrors():
 *    Default MPI_ERRORS_RETURN error handler
 */
void ReturnErrors (MPI_Comm *comm, int *errcode, char *where) {

  int rank;

  MPI_Comm_rank(*comm, &rank);

#ifdef VERBOSE_MODE
  fprintf(stderr, "\nMPI_ERRORS_RETURN handler. ");
  fprintf(stderr, "Error: %s\n", errStrings[*errcode]);
  fprintf(stderr, "Function: %s \t", where);
  fprintf(stderr, "Error code: %d\tError class: %d\n", *errcode, *errcode);
  fprintf(stderr, "Rank: %d\t", rank);
  fprintf(stderr, "Communicator: 0x%x (CommNR %d)\n\n", (unsigned int)*comm, commGetContext((Mpi_P_Comm *)*comm));
#endif
}


  /*----------------------------------------------------------------/
 /       implmentation of Public exported interface                /
/----------------------------------------------------------------*/
/*
 *  MPI_Error_string
 */
int MPI_Error_string (int errorcode, char *string, int *resultlen) {

  strcpy(string, errStrings[errorcode]);
  *resultlen = strlen(string);

  return MPI_SUCCESS;
}

