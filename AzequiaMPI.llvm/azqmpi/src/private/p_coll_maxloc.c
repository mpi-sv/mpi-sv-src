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
 /   Definition of macros                                 /
/-------------------------------------------------------*/
#define MIN_VALUE(a,b) (((a)>(b))?(b):(a))

  /*-------------------------------------------------------/
 /   Definition of types                                  /
/-------------------------------------------------------*/
/* Datatypes struct for maxloc */

/* MPI_2INT */
struct maxloc_2int {
  int    Value;
  int    Loc;
};

/* MPI_SHORT_INT */
struct maxloc_short_int {
  short  Value;
  int    Loc;
};

/* MPI_LONG_INT */
struct maxloc_long_int {
  long   Value;
  int    Loc;
};

/* MPI_FLOAT_INT */
struct maxloc_float_int {
  float  Value;
  int    Loc;
};

/* MPI_DOUBLE_INT */
struct maxloc_double_int {
  double Value;
  int    Loc;
};

/* MPI_LONG_DOUBLE_INT */
struct maxloc_long_double_int {
  long double Value;
  int         Loc;
};

  /*-------------------------------------------------------/
 /   Implementation of collective predefined operations   /
/-------------------------------------------------------*/
/*
 *  mpi_maxloc()
 */
void mpi_maxloc (void *invec, void *inoutvec, int *len, Mpi_P_Datatype *datatype) {

	int i;


	switch(dtypeGetBasicSignature(datatype)) {

	  case MPI_P_2INT: {
			struct maxloc_2int *a = (struct maxloc_2int *)invec;
			struct maxloc_2int *b = (struct maxloc_2int *)inoutvec;

      for (i = 0; i < *len; i++) {
        if (a[i].Value == b[i].Value) {
          b[i].Loc = MIN_VALUE(a[i].Loc, b[i].Loc);
        } else if (a[i].Value > b[i].Value) {
          b[i].Value = a[i].Value;
          b[i].Loc   = a[i].Loc;
        }
      }

			break;
		}

	  case MPI_P_SHORT_INT: {
			struct maxloc_short_int *a = (struct maxloc_short_int *)invec;
			struct maxloc_short_int *b = (struct maxloc_short_int *)inoutvec;

      for (i = 0; i < *len; i++) {
        if (a[i].Value == b[i].Value) {
          b[i].Loc = MIN_VALUE(a[i].Loc, b[i].Loc);
        } else if (a[i].Value > b[i].Value) {
          b[i].Value = a[i].Value;
          b[i].Loc   = a[i].Loc;
        }
      }

			break;
		}

	  case MPI_P_LONG_INT: {
			struct maxloc_long_int *a = (struct maxloc_long_int *)invec;
			struct maxloc_long_int *b = (struct maxloc_long_int *)inoutvec;

      for (i = 0; i < *len; i++) {
        if (a[i].Value == b[i].Value) {
          b[i].Loc = MIN_VALUE(a[i].Loc, b[i].Loc);
        } else if (a[i].Value > b[i].Value) {
          b[i].Value = a[i].Value;
          b[i].Loc   = a[i].Loc;
        }
      }


			break;
		}

	  case MPI_P_FLOAT_INT: {
			struct maxloc_float_int *a = (struct maxloc_float_int *)invec;
			struct maxloc_float_int *b = (struct maxloc_float_int *)inoutvec;

      for (i = 0; i < *len; i++) {
        if (a[i].Value == b[i].Value) {
          b[i].Loc = MIN_VALUE(a[i].Loc, b[i].Loc);
        } else if (a[i].Value > b[i].Value) {
          b[i].Value = a[i].Value;
          b[i].Loc   = a[i].Loc;
        }
      }

			break;
		}

	  case MPI_P_DOUBLE_INT: {
			struct maxloc_double_int *a = (struct maxloc_double_int *)invec;
			struct maxloc_double_int *b = (struct maxloc_double_int *)inoutvec;

      for (i = 0; i < *len; i++) {
        if (a[i].Value == b[i].Value) {
          b[i].Loc = MIN_VALUE(a[i].Loc, b[i].Loc);
        } else if (a[i].Value > b[i].Value) {
          b[i].Value = a[i].Value;
          b[i].Loc   = a[i].Loc;
        }
      }

			break;
		}

	  case MPI_P_LONG_DOUBLE_INT: {
			struct maxloc_long_double_int *a = (struct maxloc_long_double_int *)invec;
			struct maxloc_long_double_int *b = (struct maxloc_long_double_int *)inoutvec;

      for (i = 0; i < *len; i++) {
        if (a[i].Value == b[i].Value) {
          b[i].Loc = MIN_VALUE(a[i].Loc, b[i].Loc);
        } else if (a[i].Value > b[i].Value) {
          b[i].Value = a[i].Value;
          b[i].Loc   = a[i].Loc;
        }
      }

			break;
		}


		default:
			return;

	}
}
