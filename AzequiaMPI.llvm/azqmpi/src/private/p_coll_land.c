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
#include <p_collops.h>

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <azq_types.h>

#include <p_config.h>
#include <p_dtype.h>

  /*-------------------------------------------------------/
 /   Implementation of collective predefined operations   /
/-------------------------------------------------------*/
/*
 *  mpi_land()
 */
void mpi_land (void *invec, void *inoutvec, int *len, Mpi_P_Datatype *datatype) {

	int i;


	switch(dtypeGetBasicSignature(datatype)) {

	  case MPI_P_SHORT: {
			short *a = (short *)invec;
			short *b = (short *)inoutvec;

			for (i = 0; i < *len; i++)       b[i] = a[i] && b[i];

			break;
		}

		case MPI_P_INT: {
			int *a = (int *)invec;
			int *b = (int *)inoutvec;

			for (i = 0; i < *len; i++)       b[i] = a[i] && b[i];

			break;
		}

    case MPI_P_LONG: {
			long *a = (long *)invec;
			long *b = (long *)inoutvec;

			for (i = 0; i < *len; i++)       b[i] = a[i] && b[i];

			break;
		}

    case MPI_P_UNSIGNED_SHORT: {
			unsigned short *a = (unsigned short *)invec;
			unsigned short *b = (unsigned short *)inoutvec;

			for (i = 0; i < *len; i++)       b[i] = a[i] && b[i];

			break;
		}

    case MPI_P_UNSIGNED: {
			unsigned *a = (unsigned *)invec;
			unsigned *b = (unsigned *)inoutvec;

			for (i = 0; i < *len; i++)       b[i] = a[i] && b[i];

			break;
		}

    case MPI_P_UNSIGNED_LONG: {
			unsigned long *a = (unsigned long *)invec;
			unsigned long *b = (unsigned long *)inoutvec;

			for (i = 0; i < *len; i++)       b[i] = a[i] && b[i];

			break;
		}

		default:
			return;
	}
}

