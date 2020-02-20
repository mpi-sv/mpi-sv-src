#include <config.h>
/* This definitions must be before including proto.h */
#if defined(F77_NAME_LOWER_USCORE)
#define mpi_waitall__ mpi_waitall_
#elif defined(F77_NAME_LOWER_2USCORE)
/* as is */
#else 
#error "Naming Fortran functions not supported"
#endif


#include "f_proto.h"
#include "f_util.h"



void mpi_waitall__ (MPI_Fint *count, MPI_Fint *array_of_requests, MPI_Fint *array_of_statuses, MPI_Fint *ierr) {
  
  Mpi_P_Status    *c_status;
  MPI_Request     *c_req;
  int              i;
  
  
  if (IS_FORTRAN_STATUSES_IGNORE(array_of_statuses)) {
	
    c_status = MPI_STATUSES_IGNORE;
	
  } else {
	
    if (NULL == (c_status = malloc (*count * sizeof(Mpi_P_Status)))) {
      *ierr = MPI_ERR_INTERN;
      return;
    }
		
  }
    
  
  c_req = malloc (*count * sizeof(MPI_Request));
  for (i = 0; i < *count; i++) {
	request_f2c(&array_of_requests[i], &c_req[i]);
  }
  
  *ierr = MPI_Waitall (*count, c_req, c_status);
  
  if (*ierr == MPI_SUCCESS) {
	
    for (i = 0; i < *count; i++) {
      request_c2f(c_req[i], &array_of_requests[i]);
	}
	 
	for (i = 0; i < *count; i++) {
	  if ((!IS_FORTRAN_STATUSES_IGNORE(array_of_statuses)) &&
	      (!IS_FORTRAN_STATUS_IGNORE(array_of_statuses[i])))   {
        status_c2f(&c_status[i], &array_of_statuses[i * (sizeof(MPI_Status) / sizeof(int))]);
	  }
	}
	
  }
  
  if (c_status) free(c_status);
  
}

