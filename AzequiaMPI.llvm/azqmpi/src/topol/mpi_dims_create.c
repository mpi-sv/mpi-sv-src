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
   |  Date:    Sept 24, 2008                                               |
   |                                                                       |
   |  Description:  MPI_Dims_create function taken from MPICH2 v1.2.7.     |
   |                                                                       |
   |                                                                       |
   |_______________________________________________________________________| */

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <mpi.h>

#include <env.h>
#include <errhnd.h>
#include <check.h>

#include <p_config.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Dims_create
#define MPI_Dims_create  PMPI_Dims_create
#endif

  /*----------------------------------------------------------------/
 /         Definition of constants used by this module             /
/----------------------------------------------------------------*/
#define MAX_FACTORS 10

#define NUM_PRIMES 168
  static int primes[NUM_PRIMES] =
	   {2,    3,    5,    7,   11,   13,   17,   19,   23,   29,
	   31,   37,   41,   43,   47,   53,   59,   61,   67,   71,
	   73,   79,   83,   89,   97,  101,  103,  107,  109,  113,
	  127,  131,  137,  139,  149,  151,  157,  163,  167,  173,
	  179,  181,  191,  193,  197,  199,  211,  223,  227,  229,
	  233,  239,  241,  251,  257,  263,  269,  271,  277,  281,
	  283,  293,  307,  311,  313,  317,  331,  337,  347,  349,
	  353,  359,  367,  373,  379,  383,  389,  397,  401,  409,
	  419,  421,  431,  433,  439,  443,  449,  457,  461,  463,
	  467,  479,  487,  491,  499,  503,  509,  521,  523,  541,
	  547,  557,  563,  569,  571,  577,  587,  593,  599,  601,
	  607,  613,  617,  619,  631,  641,  643,  647,  653,  659,
	  661,  673,  677,  683,  691,  701,  709,  719,  727,  733,
	  739,  743,  751,  757,  761,  769,  773,  787,  797,  809,
	  811,  821,  823,  827,  829,  839,  853,  857,  859,  863,
	  877,  881,  883,  887,  907,  911,  919,  929,  937,  941,
	  947,  953,  967,  971,  977,  983,  991,  997};

  /*----------------------------------------------------------------/
 /            Definition of types used by this module              /
/----------------------------------------------------------------*/
typedef struct Factors {
  int val;
  int cnt;
} Factors;

  /*----------------------------------------------------------------/
 /        Declaration of functions used by this module             /
/----------------------------------------------------------------*/
PRIVATE int factor        (int n, Factors factors[], int *ndivisors);
PRIVATE int chooseFactors (int nfactors, Factors factors[], int nnodes, int needed, int chosen[]);

  /*----------------------------------------------------------------/
 /        Definition of functions used by this module              /
/----------------------------------------------------------------*/
PRIVATE int factor (int n, Factors factors[], int *ndivisors) {

  int n_tmp, n_root;
  int i, nfactors=0, nall=0;
  int cnt;

    /* Start from an approximate of the square root of n, by first finding
       the power of 2 at least as large as n.  The approximate root is then
       2 to the 1/2 this power */
  n_tmp  = n;
  n_root = 0;
  while (n_tmp) {
    n_root ++;
	  n_tmp >>= 1;
  }
  n_root = 1 << (n_root / 2);

  /* Find the prime number that less than that value and try dividing
     out the primes.  */
  for (i=0; i<NUM_PRIMES; i++) {
	  if (primes[i] > n_root) break;
  }

    /* For each prime, divide out as many as possible */
  for (;i>=0;i--) {
	  cnt = 0;
	  while ( (n %  primes[i]) == 0) {
	    cnt ++;
	    n = n / primes[i];
	  }
	  if (cnt > 0) {
	    /* --BEGIN ERROR HANDLING-- */
	    if (nfactors + 1 == MAX_FACTORS) {
		  /* Time to panic.  This should not happen, since the
		     smallest number that could exceed this would
		     be the product of the first 10 primes that are
		     greater than one, which is 6469693230 */
		    return nfactors;
	    }
	    /* --END ERROR HANDLING-- */
	    factors[nfactors].val = primes[i];
	    factors[nfactors++].cnt = cnt;
	    nall += cnt;
	  }
  }
  /* If nfactors == 0, n was a prime, so return that */
  if (nfactors == 0) {
	  nfactors = 1;
    nall = 1;
    factors[0].val = n;
    factors[0].cnt = 1;
  } else if (n > 1) {
	  /* We need one more factor (a single prime > n_root) */
	  factors[nfactors].val   = n;
	  factors[nfactors++].cnt = 1;
	  nall++;
  }
  *ndivisors = nall;
  return nfactors;
}


PRIVATE int chooseFactors (int nfactors, Factors factors[],
				   int nnodes, int needed, int chosen[])  {

  int i, j;

  /* Initialize the chosen factors to all 1 */
  for (i=0; i<needed; i++) {
    chosen[i] = 1;
  }
  /* For each of the available factors, there are factors[].cnt
     copies of each of the unique factors.  These are assigned in
     modified round robin fashion to each of the "chosen" entries.
     The modification is to combine with the smallest current term
     Note that the factors are in decreasing order. */

  i = 0;  /* We don't reset the count so that the factors are
	       distributed among the dimensions */
  for (j=0; j<nfactors; j++) {
	  int cnt = factors[j].cnt;
	  int val = factors[j].val;
	  /* For each of the factors that we add, try to keep the
	     entries balanced. */
	  while (cnt--) {
	    /* Find the smallest current entry */
	    int ii, cMin, iMin;
	    iMin = 0;
	    cMin = chosen[0];
	    for (ii=1; ii<needed; ii++) {
		    if (chosen[ii] < cMin) {
		      cMin = chosen[ii];
		      iMin = ii;
		    }
	    }
	    if (chosen[i] > iMin) i = iMin;

	    chosen[i] *= val;
	    i++;
	    if (i >= needed) i = 0;
	  }
  }

  /* Second, sort the chosen array in non-increasing order.  Use
     a simple bubble sort because the number of elements is always small */
  for (i=0; i<needed-1; i++) {
    for (j=i+1; j<needed; j++) {
	    if (chosen[j] > chosen[i]) {
		    int tmp = chosen[i];
		    chosen[i] = chosen[j];
		    chosen[j] = tmp;
	    }
	  }
  }

  return 0;
}



/**
 *  MPI_Dims_create
 */
int MPI_Dims_create (int nnodes, int ndims, int *dims) {

  int      mpi_errno;
  Factors  factors[MAX_FACTORS];
  int      chosen[MAX_CART_DIMS];
  int      i, j;
  int      dims_needed, dims_product, nfactors, ndivisors = 0;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Dims_create (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_ndims(ndims))                      goto mpi_exception;
  if (mpi_errno = check_count(nnodes))                     goto mpi_exception;
  if (dims == NULL)                                       {mpi_errno = MPI_ERR_ARG;  goto mpi_exception;}
  if (mpi_errno = check_dims(ndims, dims))                 goto mpi_exception;
#endif

  /* Find the number of unspecified dimensions in dims and the product
     of the positive values in dims */
  dims_needed  = 0;
  dims_product = 1;
  for (i = 0; i < ndims; i++) {
	  if (dims[i] == 0) dims_needed++;
	  else dims_product *= dims[i];
  }

  /* Can we factor nnodes by dims_product? */
  if ((nnodes / dims_product ) * dims_product != nnodes )
                                                          {mpi_errno = MPI_ERR_DIMS; goto mpi_exception;}

  if (!dims_needed) {
  	/* Special case - all dimensions provided */
	  return MPI_SUCCESS;
  }

  if (dims_needed > MAX_CART_DIMS)                        {mpi_errno = MPI_ERR_DIMS; goto mpi_exception;}

  nnodes /= dims_product;

  /* Now, factor nnodes into dims_needed components.  We'd like these
     to match the underlying machine topology as much as possible.  In the
     absence of information about the machine topology, we can try to
     make the factors a close to each other as possible.

     The MPICH 1 version used donated code that was quite sophisticated
     and complex.  However, since it didn't take the system topology
     into account, it was more sophisticated that was perhaps warranted.
     In addition, useful values of nnodes for most MPI programs will be
     of the order 10-10000, and powers of two will be common.
  */

  /* Get the factors */
  nfactors = factor(nnodes, factors, &ndivisors);

  /* Divide into 3 major cases:
     1. Fewer divisors than needed dimensions.  Just use all of the
        factors up, setting the remaining dimensions to 1
     2. Only one distinct factor (typically 2) but with greater
        multiplicity.  Give each dimension a nearly equal size
     3. Other.  There are enough factors to divide among the dimensions.
        This is done in an ad hoc fashion
  */

  /* The MPI spec requires that the values that are set be in nonincreasing
     order (MPI-1, section 6.5).  */

  /* Distribute the factors among the dimensions */
  if (ndivisors <= dims_needed) {
    /* Just use the factors as needed.  */
	  chooseFactors( nfactors, factors, nnodes, dims_needed, chosen );
	  j = 0;
	  for (i=0; i<ndims; i++) {
	    if (dims[i] == 0) {
		    dims[i] = chosen[j++];
	    }
	  }
  } else {
	  /* We must combine some of the factors */
	  /* This is what the fancy code is for in the MPICH-1 code.
	     If the number of distinct factors is 1 (e.g., a power of 2),
	     then this code can be much simpler */
	  /* NOT DONE */
	  /* FIXME */
	  if (nfactors == 1) {
	    /* Special case for k**n, such as powers of 2 */
	    int factor = factors[0].val;
	    int cnt    = factors[0].cnt; /* Numver of factors left */
	    int cnteach = ( cnt + dims_needed - 1 ) / dims_needed;
	    int factor_each;

	    factor_each = factor;
	    for (i=1; i<cnteach; i++) factor_each *= factor;

	    for (i=0; i<ndims; i++) {
		    if (dims[i] == 0) {

		      if (cnt > cnteach) {
			      dims[i] = factor_each;
			      cnt -= cnteach;
		      } else if (cnt > 0) {
			      factor_each = factor;
			      for (j=1; j<cnt; j++)
			        factor_each *= factor;
			      dims[i] = factor_each;
			      cnt = 0;
		      } else {
			      dims[i] = 1;
		      }

		    }
	    }
	  } else {
	    /* Here is the general case.  */
	    chooseFactors( nfactors, factors, nnodes, dims_needed, chosen );
	    j = 0;
	    for (i=0; i<ndims; i++) {
		    if (dims[i] == 0) {
		      dims[i] = chosen[j++];
		    }
	    }
	  }
  }

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Dims_create (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Dims_create");
}

