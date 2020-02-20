#include <config.h>
#include "f_util.h"


/* Definition of variables declared as extern in .h */
MPI_Status MPI_FORTRAN_STATUS_IGNORE;
MPI_Status mpi_fortran_status_ignore;
MPI_Status mpi_fortran_status_ignore_;
MPI_Status mpi_fortran_status_ignore__;

MPI_Status MPI_FORTRAN_STATUSES_IGNORE;
MPI_Status mpi_fortran_statuses_ignore;
MPI_Status mpi_fortran_statuses_ignore_;
MPI_Status mpi_fortran_statuses_ignore__;


/* Public utility functions */
void status_c2f (MPI_Status *c_status, MPI_Fint *f_status) {

  int i;

  if (IS_FORTRAN_STATUS_IGNORE(f_status))  return;
  if (c_status == MPI_STATUS_IGNORE)       return;

  for (i = 0; i < sizeof(MPI_Status) / sizeof(int); i++) {
    f_status[i] = ((int *)c_status)[i];
  }

}


void request_c2f (MPI_Request c_req, MPI_Fint *f_req) {

  if (c_req == NULL) {
	*f_req = -1;
    return;
  }
  
  *f_req = (MPI_Fint)PCS_rqstGetByAddr(c_req);

}


void request_f2c (MPI_Fint *f_req, MPI_Request *c_req) {

  *c_req = PCS_rqstGetByIndex(*f_req);

}



void comm_c2f (MPI_Comm c_comm, MPI_Fint *f_comm) {
  
  *f_comm = (MPI_Fint)PCS_commGetByAddr(c_comm);
  
}


void comm_f2c (MPI_Fint *f_comm, MPI_Comm *c_comm) {
  
  *c_comm = PCS_commGetByIndex(*f_comm);
  
}



void dtype_c2f (MPI_Datatype c_dtype, MPI_Fint *f_dtype) {

  *f_dtype = PCS_dtypeGetByAddr(c_dtype);

}


void dtype_f2c (MPI_Fint *f_dtype, MPI_Datatype *c_dtype) {
  
  *c_dtype = PCS_dtypeGetByIndex(*f_dtype);
  
}


