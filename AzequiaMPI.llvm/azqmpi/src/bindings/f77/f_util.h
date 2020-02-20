#include <config.h>
#ifndef _F_UTIL_H
#define _F_UTIL_H

#include "p_status.h"


#define  MPI_FORTRAN_COMM_NULL  (-1)


typedef int MPI_Fint;


/* Extern variables defined at f_util.c */
extern Mpi_P_Status MPI_FORTRAN_STATUS_IGNORE;
extern Mpi_P_Status mpi_fortran_status_ignore;
extern Mpi_P_Status mpi_fortran_status_ignore_;
extern Mpi_P_Status mpi_fortran_status_ignore__;

extern Mpi_P_Status MPI_FORTRAN_STATUSES_IGNORE;
extern Mpi_P_Status mpi_fortran_statuses_ignore;
extern Mpi_P_Status mpi_fortran_statuses_ignore_;
extern Mpi_P_Status mpi_fortran_statuses_ignore__;


/* Public utility macros */
#define IS_FORTRAN_STATUS_IGNORE(status)                  \
  (status == (void *) &MPI_FORTRAN_STATUS_IGNORE   ||     \
   status == (void *) &mpi_fortran_status_ignore   ||     \
   status == (void *) &mpi_fortran_status_ignore_  ||     \
   status == (void *) &mpi_fortran_status_ignore__)

#define IS_FORTRAN_STATUSES_IGNORE(status)                \
  (status == (void *) &MPI_FORTRAN_STATUSES_IGNORE   ||   \
   status == (void *) &mpi_fortran_statuses_ignore   ||   \
   status == (void *) &mpi_fortran_statuses_ignore_  ||   \
   status == (void *) &mpi_fortran_statuses_ignore__)


/* Public utility functions */
extern  void  status_c2f  (MPI_Status *c_status, MPI_Fint *f_status);

extern  void  request_c2f (MPI_Request c_req, MPI_Fint *f_req);
extern  void  request_f2c (MPI_Fint *f_req,   MPI_Request *c_req);

extern  void  comm_c2f    (MPI_Comm c_comm, MPI_Fint *f_comm);
extern  void  comm_f2c    (MPI_Fint *f_comm, MPI_Comm *c_comm);

extern  void  dtype_c2f   (MPI_Datatype c_dtype, MPI_Fint *f_dtype);
extern  void  dtype_f2c   (MPI_Fint *f_dtype, MPI_Datatype *c_dtype);

#endif
