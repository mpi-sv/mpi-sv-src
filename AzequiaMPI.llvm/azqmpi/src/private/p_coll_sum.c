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
 *  mpi_sum()
 */
void mpi_sum (void *invec, void *inoutvec, int *len, Mpi_P_Datatype *datatype) {

	int i;


  switch(dtypeGetBasicSignature(datatype)) {

	  case MPI_P_CHAR: {
			char *a = (char *)invec;
			char *b = (char *)inoutvec;

			for (i = 0; i < *len; i++)       b[i] = a[i] + b[i];

			break;
		}

	  case MPI_P_SHORT: {
			short *a = (short *)invec;
			short *b = (short *)inoutvec;

			for (i = 0; i < *len; i++)       b[i] = a[i] + b[i];

			break;
		}

		case MPI_P_INT: {
			int *a = (int *)invec;
			int *b = (int *)inoutvec;

			for (i = 0; i < *len; i++)       b[i] = a[i] + b[i];

			break;
		}

    case MPI_P_LONG: {
			long *a = (long *)invec;
			long *b = (long *)inoutvec;

			for (i = 0; i < *len; i++)       b[i] = a[i] + b[i];

			break;
		}

    case MPI_P_UNSIGNED_SHORT: {
			unsigned short *a = (unsigned short *)invec;
			unsigned short *b = (unsigned short *)inoutvec;

			for (i = 0; i < *len; i++)       b[i] = a[i] + b[i];

			break;
		}

    case MPI_P_UNSIGNED: {
			unsigned *a = (unsigned *)invec;
			unsigned *b = (unsigned *)inoutvec;

			for (i = 0; i < *len; i++)       b[i] = a[i] + b[i];

			break;
		}

    case MPI_P_UNSIGNED_LONG: {
			unsigned long *a = (unsigned long *)invec;
			unsigned long *b = (unsigned long *)inoutvec;

			for (i = 0; i < *len; i++)       b[i] = a[i] + b[i];

			break;
		}

    case MPI_P_FLOAT: {
			float *a = (float *)invec;
			float *b = (float *)inoutvec;

			for (i = 0; i < *len; i++)       b[i] = a[i] + b[i];

			break;
		}

		case MPI_P_DOUBLE: {
			double *a = (double *)invec;
			double *b = (double *)inoutvec;

			for (i = 0; i < *len; i++)       b[i] = a[i] + b[i];

			break;
		}

    case MPI_P_LONG_DOUBLE: {
			long double *a = (long double *)invec;
			long double *b = (long double *)inoutvec;

			for (i = 0; i < *len; i++)       b[i] = a[i] + b[i];

			break;
		}

	  case MPI_P_COMPLEX: {
			struct mpi_complex *a = (struct mpi_complex *)invec;
			struct mpi_complex *b = (struct mpi_complex *)inoutvec;

      for (i = 0; i < *len; i++) {
        b[i].r = a[i].r + b[i].r;
        b[i].i = a[i].i + b[i].i;
      }

			break;
		}

	  case MPI_P_DOUBLE_COMPLEX: {
			struct mpi_double_complex *a = (struct mpi_double_complex *)invec;
			struct mpi_double_complex *b = (struct mpi_double_complex *)inoutvec;

      for (i = 0; i < *len; i++) {
        b[i].r = a[i].r + b[i].r;
        b[i].i = a[i].i + b[i].i;
      }

			break;
		}

		default:
			return;
	}
}
