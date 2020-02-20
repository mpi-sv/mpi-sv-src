#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include "clustalw.h"
#include "mpi.h"

/*
 *       Prototypes
 */

/*
 *       Global Variables
 */

extern double **tmat;
extern Boolean no_weights;
extern sint debug;
extern sint max_aa;
extern sint nseqs;
extern sint profile1_nseqs;
extern sint nsets;
extern sint **sets;
extern sint divergence_cutoff;
extern sint *seq_weight;
extern sint output_order, *output_index;
extern Boolean distance_tree;
extern char seqname[];
extern sint *seqlen_array;
extern char **seq_array;

#define MIN_N_SEQ_PDIFF 4

/* The followings are for prfalign_mpi_progressive() */
extern float gap_open, gap_extend;
extern float transition_weight;
extern sint gap_pos1, gap_pos2;
extern Boolean neg_matrix;
extern sint mat_avscore;
extern short usermatseries[MAXMAT][NUMRES][NUMRES];
extern Boolean user_series;
extern UserMatSeries matseries;

static void determine_parallelizable_sets(sint *** sets, int nseqs,
					  int ***set1, int ***set2);
static void kbl_debug_printsets(int nseqs, int which_set, int **set1,
				int **set2);

static int key_compare(const void *e1, const void *e2);
static int verify_dependency(int set, int **set1, int **set2,
			     sint ** sets, int nseqs, short *finished,
			     int *new2old);

static void myqsort(int **a, int lo, int hi, int *b);

sint malign_mpi_progressive(sint istart, char *phylip_name)
{				/* full progressive alignment */
    static sint *aligned;
    static sint *group;
    static sint ix;

    sint *maxid, max, sum;
    sint *tree_weight;
    sint i, j, set, iseq = 0;
    sint status, entries;
    lint score = 0;
    double wtime1, wtime2;
 printf("WNBA%d\n",nseqs);
   /**** Kuobin's debugging codes start ********/
    int **set1, **set2;
    int *dset1 = NULL, *dset2 = NULL;
    int myidx, myidx2;
    int mybsize;		/* the size of the MPI send/recv buffer */
    char *mpi_buffer;
    int position, np, work, from_where, which_set;
    int *dest;  /* array storing the MPI ranks of available processes */

    /* finished[1..nseqs]: finished[i] indicates that whether sets[i] has
     *                     been "prfalign-ed" or not.  */
    short *finished;
    int count;
    int ret;
    int *myentries;		/* to replace "entries" */

    int *new2old;  /* for the use of myqsort(): new2old[0..nsets]  */
    int **dsets;
    int reverse_rank;
   /**** Kuobin's debugging codes end ********/

    info("Start of Multiple Alignment");

/* get the phylogenetic tree from *.ph */

    if (nseqs >= 2) {
	status = read_tree(phylip_name, (sint) 0, nseqs);
	printf("WNBA%d\n",status );
	if (status == 0)
	    return ((sint) 0);
    }

/* calculate sequence weights according to branch lengths of the tree -
   weights in global variable seq_weight normalised to sum to 100 */

    calc_seq_weights((sint) 0, nseqs, seq_weight);

/* recalculate tmat matrix as percent similarity matrix */

    status = calc_similarities(nseqs);
    if (status == 0)
	return ((sint) 0);

/* for each sequence, find the most closely related sequence */

    maxid = (sint *) ckalloc((nseqs + 1) * sizeof(sint));
    for (i = 1; i <= nseqs; i++) {
	maxid[i] = -1;
	for (j = 1; j <= nseqs; j++)
	    if (j != i && maxid[i] < tmat[i][j])
		maxid[i] = tmat[i][j];
    }

/* group the sequences according to their relative divergence */
    printf("WNBA%d\n",istart );
    if (istart == 0) {

	sets = (sint **) ckalloc((nseqs + 1) * sizeof(sint *));
	for (i = 0; i <= nseqs; i++)
	    sets[i] = (sint *) ckalloc((nseqs + 1) * sizeof(sint));

	finished = (short *) ckalloc((nseqs + 1) * sizeof(short));

	set1 = (sint **) ckalloc((nseqs + 1) * sizeof(sint *));
	for (i = 0; i <= nseqs; i++)
	    set1[i] = (sint *) ckalloc((nseqs + 1) * sizeof(sint));
	set2 = (sint **) ckalloc((nseqs + 1) * sizeof(sint *));
	for (i = 0; i <= nseqs; i++)
	    set2[i] = (sint *) ckalloc((nseqs + 1) * sizeof(sint));

	create_sets((sint) 0, nseqs);
	info("There are %d groups", (pint) nsets);

/* clear the memory used for the phylogenetic tree */

	if (nseqs >= 2)
	    clear_tree(NULL);

/* start the multiple alignments.........  */

	info("Aligning...");

	/** Measuring wall time by MPI_Wtime() **/
	wtime1 = MPI_Wtime();

/* first pass, align closely related sequences first.... */

	ix = 0;
	aligned = (sint *) ckalloc((nseqs + 1) * sizeof(sint));
	for (i = 0; i <= nseqs; i++)
	    aligned[i] = 0;

	myentries = (int *) ckalloc((nseqs + 1) * sizeof(int));
	for (i = 0; i <= nseqs; i++)
	    myentries[i] = 0;

	new2old = (int *) ckalloc((nsets + 1) * sizeof(int));
	for (i = 0; i <= nsets ; i++)
	    new2old[i] = i;


	/*
	 * Determine set1[] and set2[]: 
	 *
	 * for example: if sets[3]={3,0,1,0,2,2,0} then
	 *                 set1 = {1,2}
	 *                 set2 = {2,4,5}
	 */

	for (set = 1; set <= nsets; ++set) {

	    int idx1 = 0;
	    int idx2 = 0;

	    myentries[set] = 0;

	    /* number of non-zero elements in sets[set] */
	    count = 0;

	    for (i = 1; i <= nseqs; i++) {

		int tmpv = 0;	/* a simple temp variable */

		if ((tmpv = sets[set][i]) != 0) {
		    count++;

		    if (tmpv == 1)
			set1[set][++idx1] = i;
		    else if (tmpv == 2)
			set2[set][++idx2] = i;
		    else
			fprintf(stderr,
				"Error: something wrong with sets[%d]\n",
				set);

		    if (maxid[i] > divergence_cutoff) {

			myentries[set]++;

			if (aligned[i] == 0) {

			    if (output_order == INPUT) {
				++ix;
				output_index[i] = i;
			    } else {
				output_index[++ix] = i;
			    }

			    aligned[i] = 1;
			}
		    }
		}
	    }
	    sets[set][0] = count;
	    set1[set][0] = idx1;
	    set2[set][0] = idx2;
	}

	/*
	 * Now I am going to sort sets[i] (i=1..nseqs) according to sets[i][0].
	 */

	/* Initially I was using the stdlib's qsort().
	   qsort((sets + 1), nsets, sizeof(sint **), key_compare);
	*/
	    printf("AAAA!!!\n");
         myqsort(sets, 1, nsets, new2old);
 printf("AAAA222!!!\n");
	 /* ****************************************************************
	  * TEMPORARY TEMPORARY 
	  *
	  * Since prf_init() sometimes overwrites the contents
	  * of sets[i][....] (in the case of "Delayed...."),
	  * this would cause trouble for verify_dependency().
	  *
	  * Temporarily here we make a duplicate copy of sets[][].
	  * ****************************************************************/
	 dsets = (int **)calloc((nseqs+1), sizeof(int *));
	 assert(dsets);
	 for (i=0;i<(nseqs+1);i++) {
	     dsets[i]=(int *)calloc((nseqs+1),sizeof(int));
	     assert(dsets[i]);
	 }

	 for (i=1;i<=nsets;i++)
	     for (j=0;j<(nseqs+1);j++)
	     dsets[i][j]=sets[i][j];


	/*
	 * Main working loop .....
	 */

	MPI_Comm_size(MPI_COMM_WORLD, &np);
	work = 1;

	/*
	 * Note: dest[0] holds the number of available processes so far.
	 *       dest[1], dest[2] are the ranks of those available processes.
	 */
	dest = (int *)malloc((np)*sizeof(int));
	assert(dest);

	dest[0]=(np-1);
	for (i=1;i<np;i++)
	    dest[i]=i;


	work = nsets;
	for (set = 1; set <= nsets; ++set) {

	    int ok_or_not;

	    if (sets[set][0] == 0) {
                work--;
		continue;
	    }

	    if (myentries[new2old[set]]<=0) {
 printf("WU4!!!\n");
		score = 0;
		finished[set]=1;
		work--;

		/* Perhaps I could print message like "Delayed ...." right here. */

		    printf ("\nGroup %2d:                     Delayed", new2old[set]);
		    kbl_debug_printsets(nseqs, new2old[set], set1, set2);

		/* go straight to the next sets[] */
		continue;
	    }

	    if (sets[set][0] == 2) {
 printf("WU3!!!\n");
		/* go straight to MPI if there is available MPI process */

		if (dest[0]) {

		    /* while we still have available MPI processes */
		    /* 
		     * (ret == 1): normal alignment
		     * (ret == 0): this set is skipped, no MPI involved.
		     * (ret == -1): some unknown error
		     */

		    ret = prf_init(sets[set], aligned, set, dest[1], 0); // YU:15-16
		    if (ret == 1) {
		       dest[0]--;
		       memmove((dest+1),(dest+2),dest[0]*sizeof(int)); 
		    }
		    else if (ret == 0) {
			finished[set] = 1;
			work--;
		        printf ("\nGroup %2d:                     Delayed", new2old[set]);
		        kbl_debug_printsets(nseqs, new2old[set], set1, set2);
		    }

		} else {
 printf("WU1!!!\n");
		    /* need to wait for a MPI process to return its result */

		    /* update the related seq_array[][] and seqlen_array[] */
		    score = prf_update(&from_where, &reverse_rank, &which_set);
		    /*
		       info("From rank %d, set %d, Score:%d",from_where, 
		       which_set, (int)score);
		     */

		    finished[which_set] = 1;
		    work--;

		    printf
			("\nGroup %2d: Sequences:%4d      Score: %d (from rank %d)",
			 new2old[which_set], sets[which_set][0], (int) score,
			 from_where);
		    kbl_debug_printsets(nseqs, new2old[which_set], set1, set2);

		    ret = prf_init(sets[set], aligned, set, from_where, 0);

		    if (ret == 0) {

			/*
			 * Although sets[set] is skipped by prf_init(),
			 * we will assume it has been aligned for load-balancing
			 * purpose.
			 */
			finished[set] = 1;
			work--;
		        printf ("\nGroup %2d:                     Delayed", new2old[set]);
		        kbl_debug_printsets(nseqs, new2old[set], set1, set2);

			/*
			 * The "from_where" variable must be remembered, since
			 * the next prf_init() is going to MPI_Send() to 
			 * "from_where".
			 */
			dest[0]++;
			dest[dest[0]]=from_where;
		    }

		}

	    } else {
 printf("WU2!!!\n");
		/* verify the time dependency */

		ok_or_not =
		    verify_dependency(set, set1, set2, sets, nseqs,
				      finished, new2old);

		/*
		 * TEMPORARY!!!!!!!!!!!!
		 * If not ok, we'll wait until it becomes ok.
		 * TEMPORARY!!!!!!!!!!!!
		 */

		while (ok_or_not == 0) {

		    /* need to wait for a MPI process to return its result */

		    /* update the related seq_array[][] and seqlen_array[] */
		    score = prf_update(&from_where, &reverse_rank, &which_set);
		    /*
		       info("From rank %d, set %d, Score:%d",from_where, 
		       which_set, (int)score);
		     */

		    finished[which_set] = 1;
		    work--;

		    printf
			("\nGroup %2d: Sequences:%4d      Score: %d (from rank %d)",
			 new2old[which_set], sets[which_set][0], (int) score,
			 from_where);
		    kbl_debug_printsets(nseqs, new2old[which_set], set1, set2);

		    /* Now we have one more available MPI process */
		    if (reverse_rank) {
		       dest[dest[0]+1]=from_where;
		       dest[dest[0]+2]=reverse_rank;
		       dest[0] += 2;
		    } else{
		       dest[dest[0]+1]=from_where;
		       dest[0] += 1;
		    }

		    ok_or_not =
			verify_dependency(set, set1, set2, sets, nseqs,
					  finished, new2old);

		    /**************** DEBUG DEBUG *********************
		    fprintf(stdout,"\n......  in while loop ...\n");
		    fflush(stdout);
		    **************** DEBUG DEBUG *********************/

		}

		/* send to MPI */

		if (dest[0]>=2) {

		    /* 
		     *
		     * Note: Temporarily assign the next available MPI process
		     *       to prf_init as the "reverse_rank". This process
		     *       will be doing preverse_pass().
		     */
		    if (sets[set][0]>= MIN_N_SEQ_PDIFF)
		       ret = prf_init(sets[set], aligned, set, dest[1], dest[2]);
		    else
		       ret = prf_init(sets[set], aligned, set, dest[1], 0);

		    if (ret == 1) {

		       if (sets[set][0]>= MIN_N_SEQ_PDIFF) {
		          dest[0] -= 2;
		          memmove((dest+1),(dest+3),dest[0]*sizeof(int));
		       } else{
		          dest[0] -= 1;
		          memmove((dest+1),(dest+2),dest[0]*sizeof(int));
		       }

		    } else if (ret==0) {

			finished[set] = 1;
			work--;
		        printf ("\nGroup %2d:                     Delayed", new2old[set]);
		        kbl_debug_printsets(nseqs, new2old[set], set1, set2);

		    }

		} else {

		    /* need to wait for a MPI process to return its result */

		    do {

		       score = prf_update(&from_where, &reverse_rank, &which_set);
		       finished[which_set] = 1;
		       work--;

		       printf
			("\nGroup %2d: Sequences:%4d      Score: %d (from rank %d)",
			 new2old[which_set], sets[which_set][0], (int) score,
			 from_where);
		       kbl_debug_printsets(nseqs, new2old[which_set], set1, set2);

		       dest[dest[0]+1] = from_where;
		       if (reverse_rank){
		          dest[dest[0]+2] = reverse_rank;
		          dest[0] +=1;
		       }
		       dest[0] +=1;

		    } while (dest[0]<2);

		    if (sets[set][0]>= MIN_N_SEQ_PDIFF)
		       ret = prf_init(sets[set], aligned, set, dest[1], dest[2]);
		    else
		       ret = prf_init(sets[set], aligned, set, dest[1], 0);

		    if (ret == 1) {

		       if (sets[set][0]>= MIN_N_SEQ_PDIFF) {
		          dest[0] -= 2;
		          memmove((dest+1),(dest+3),dest[0]*sizeof(int));
		       } else{
		          dest[0] -= 1;
		          memmove((dest+1),(dest+2),dest[0]*sizeof(int));
		       }

		    } else if (ret==0) {
			finished[set] = 1;
			work--;
		        printf ("\nGroup %2d:                     Delayed", new2old[set]);
		        kbl_debug_printsets(nseqs, new2old[set], set1, set2);
		    }
		}
	    }
	}

	/*
	 * Waiting for the remaining MPI processes to return.
	 */
	for (i = 0; i < work; i++) {
	    score = prf_update(&from_where, &reverse_rank, &which_set); //YU: op:21--22
	    /*
	       info("From rank %d, set %d, Score:%d",from_where,
	       which_set, (int)score);
	     */
	    finished[which_set] = 1;
	    printf
		("\nGroup %2d: Sequences:%4d      Score: %d (from rank %d)",
		 new2old[which_set], sets[which_set][0], (int) score, from_where);
	    kbl_debug_printsets(nseqs, new2old[which_set], set1, set2);

	}
	for (i = 0; i <= nseqs; i++)
	    sets[i] = ckfree((void *) sets[i]);
	sets = ckfree(sets);


	for (i = 0; i <= nseqs; i++)
	    free(dsets[i]);
	free(dsets);

	for (i = 0; i <= nseqs; i++) {
	    free(set1[i]);
	    free(set2[i]);
	}
	free(set1);
	free(set2);

	free(dest);

	finished = ckfree((void *) finished);

    } else {
/* clear the memory used for the phylogenetic tree */

	if (nseqs >= 2)
	    clear_tree(NULL);

	aligned = (sint *) ckalloc((nseqs + 1) * sizeof(sint));
	ix = 0;
	for (i = 1; i <= istart + 1; i++) {
	    aligned[i] = 1;
	    ++ix;
	    output_index[i] = i;
	}
	for (i = istart + 2; i <= nseqs; i++)
	    aligned[i] = 0;
    }

/* second pass - align remaining, more divergent sequences..... */

/* if not all sequences were aligned, for each unaligned sequence,
   find it's closest pair amongst the aligned sequences.  */

    group = (sint *) ckalloc((nseqs + 1) * sizeof(sint));
    tree_weight = (sint *) ckalloc((nseqs) * sizeof(sint));
    for (i = 0; i < nseqs; i++)
	tree_weight[i] = seq_weight[i];

/* if we haven't aligned any sequences, in the first pass - align the
two most closely related sequences now */
    if (ix == 0) {
	max = -1;
	iseq = 0;
	for (i = 1; i <= nseqs; i++) {
	    for (j = i + 1; j <= nseqs; j++) {
		if (max < tmat[i][j]) {
		    max = tmat[i][j];
		    iseq = i;
		}
	    }
	}
	aligned[iseq] = 1;
	if (output_order == INPUT) {
	    ++ix;
	    output_index[iseq] = iseq;
	} else
	    output_index[++ix] = iseq;
    }

    while (ix < nseqs) {
	for (i = 1; i <= nseqs; i++) {
	    if (aligned[i] == 0) {
		maxid[i] = -1;
		for (j = 1; j <= nseqs; j++)
		    if ((maxid[i] < tmat[i][j]) && (aligned[j] != 0))
			maxid[i] = tmat[i][j];
	    }
	}
/* find the most closely related sequence to those already aligned */

	max = -1;
	iseq = 0;
	for (i = 1; i <= nseqs; i++) {
	    if ((aligned[i] == 0) && (maxid[i] > max)) {
		max = maxid[i];
		iseq = i;
	    }
	}

/* align this sequence to the existing alignment */
/* weight sequences with percent identity with profile*/
/* OR...., multiply sequence weights from tree by percent identity with new sequence */
	if (no_weights == FALSE) {
	    for (j = 0; j < nseqs; j++)
		if (aligned[j + 1] != 0)
		    seq_weight[j] = tree_weight[j] * tmat[j + 1][iseq];
/*
  Normalise the weights, such that the sum of the weights = INT_SCALE_FACTOR
*/

	    sum = 0;
	    for (j = 0; j < nseqs; j++)
		if (aligned[j + 1] != 0)
		    sum += seq_weight[j];
	    if (sum == 0) {
		for (j = 0; j < nseqs; j++)
		    seq_weight[j] = 1;
		sum = j;
	    }
	    for (j = 0; j < nseqs; j++)
		if (aligned[j + 1] != 0) {
		    seq_weight[j] =
			(seq_weight[j] * INT_SCALE_FACTOR) / sum;
		    if (seq_weight[j] < 1)
			seq_weight[j] = 1;
		}
	}

	entries = 0;
	for (j = 1; j <= nseqs; j++)
	    if (aligned[j] != 0) {
		group[j] = 1;
		entries++;
	    } else if (iseq == j) {
		group[j] = 2;
		entries++;
	    }
	aligned[iseq] = 1;

	/*
	score = prfalign(group, aligned);
	*/

	/* Trying to use pdiff() version of prfalign() */
	/*
	fprintf(stderr,"DEBUG: calling prfalign_mpi_pdiff()...\n");
	fflush(stderr);
	*/
	 printf("CBA%d---%d\n",group,aligned);
	score = prfalign_mpi_pdiff(group, aligned);


   /**** Kuobin's debugging codes start ********/
	if (dset1 != NULL) {
	    memset(dset1, 0, (nseqs + 1));
	} else {
	    dset1 = (int *) calloc((nseqs + 1), sizeof(int));
	}

	if (dset2 != NULL) {
	    memset(dset2, 0, (nseqs + 1));
	} else {
	    dset2 = (int *) calloc((nseqs + 1), sizeof(int));
	}

	myidx = 0;
	myidx2 = 0;
	for (i = 1; i <= nseqs; i++) {
	    if (group[i] == 1) {
		dset1[myidx] = i;
		myidx++;
	    } else if (group[i] == 2) {
		dset2[myidx2] = i;
		myidx2++;
	    }
	}
   /**** Kuobin's debugging codes end ********/

	info("Sequence:%d     Score:%d", (pint) iseq, (pint) score);

   /**** Kuobin's debugging codes start ********/
	printf("   was aligning: (");
	myidx = 0;
	myidx2 = 0;
	while (dset1[myidx]) {
	    printf("%d ", dset1[myidx]);
	    myidx++;
	}
	printf(") and (");
	while (dset2[myidx2]) {
	    printf("%d ", dset2[myidx2]);
	    myidx2++;
	}
	printf(")");
   /**** Kuobin's debugging codes end ********/

	if (output_order == INPUT) {
	    ++ix;
	    output_index[iseq] = iseq;
	} else
	    output_index[++ix] = iseq;
    }

    group = ckfree((void *) group);
    aligned = ckfree((void *) aligned);

    myentries = ckfree((void *) myentries);
    new2old = ckfree((void *) new2old);

    maxid = ckfree((void *) maxid);
    tree_weight = ckfree((void *) tree_weight);

    aln_score();

/* make the rest (output stuff) into routine clustal_out in file amenu.c */

	/** Measuring wall time by MPI_Wtime() **/
    wtime2 = MPI_Wtime();

    fprintf(stderr, "\nDEBUG: malign time = %5.3f sec\n", wtime2 - wtime1);
    fflush(stderr);

    return (nseqs);
}

/*
 * This function will save the number of non-zero elements of each
 * sets[i] array in * sets[i][0], i = 0 to nseqs.
 *
 * In addition, set1[i] (i = 1 to nseqs) is a one dimensional array
 * storing the sequence number that has '1' in sets[i];
 * similarly, set2[i] (i = 1 to nseqs) is a one dimensional array
 * storing the sequence number that has '2' in sets[i].
 */
static void determine_parallelizable_sets(sint *** sets, int nseqs,
					  int ***set1, int ***set2)
{
    sint **tmp, **psets;
    int i, j;
    int myidx, myidx2;

    *set1 = (int **) calloc((nseqs + 1), sizeof(int *));
    assert(*set1);
    for (i = 0; i <= nseqs; i++) {
	(*set1)[i] = (int *) calloc((nseqs + 1), sizeof(int));
	assert((*set1)[i]);
    }

    *set2 = (int **) calloc((nseqs + 1), sizeof(int *));
    assert(*set2);
    for (i = 0; i <= nseqs; i++) {
	(*set2)[i] = (int *) calloc((nseqs + 1), sizeof(int));
	assert((*set2)[i]);
    }

    tmp = (sint **) calloc((nseqs + 1), sizeof(sint *));
    assert(tmp);
    for (i = 0; i <= (nseqs); i++) {
	tmp[i] = (sint *) calloc((nseqs + 1), sizeof(sint));
	assert(tmp[i]);
    }

    psets = *sets;

    /* To store the number of non-zero elements of each psets[i] into 
     * psets[i][0].
     */
    for (i = 1; i <= (nseqs); i++) {

	myidx = 0;
	myidx2 = 0;

	for (j = 1; j <= (nseqs); j++)
	    if (psets[i][j]) {

		tmp[i][0]++;
		tmp[i][j] = psets[i][j];

		if (psets[i][j] == 1) {
		    (*set1)[i][myidx] = j;
		    myidx++;
		} else if (psets[i][j] == 2) {
		    (*set2)[i][myidx2] = j;
		    myidx2++;
		}
	    }
    }

    for (i = 0; i <= (nseqs); i++)
	free(psets[i]);
    free(psets);

    *sets = tmp;

    return;
}

/*
 * Print the sequences involved in the current sets[which_set][].
 */
static void kbl_debug_printsets(int nseqs, int which_set, int **set1,
				int **set2)
{

    int i;

    printf("   was aligning: (");

    for (i = 1; i <= set1[which_set][0]; i++)
	printf("%d ", set1[which_set][i]);
    printf(") and (");

    for (i = 1; i <= set2[which_set][0]; i++)
	printf("%d ", set2[which_set][i]);
    printf(")");

    return;
}

static int key_compare(const void *e1, const void *e2)
{
    int v1, v2;

    v1 = *(int *) (*((int *) e1));
    v2 = *(int *) (*((int *) e2));

    return (v1 < v2) ? -1 : (v1 > v2) ? 1 : 0;
}

/*
 * Return 1 if sets[set] is safe to be executed by slave
 * MPI process; else return 0.
 */
static int verify_dependency(int set, int **set1, int **set2,
			     sint ** sets, int nseqs, short *finished,
			     int *new2old)
{
    int i, j;
    int length1, length2;	/* number of sequences in the first and the second set */
    int flag1, flag2;

    flag1 = 0;
    flag2 = 0;

    length1 = set1[new2old[set]][0];
    length2 = set2[new2old[set]][0];

    if (length1 > 1) {

	for (i = 1; i <= nseqs; i++) {
	    if (sets[i][0] == length1) {

		for (j = 1; j <= length1; j++) {
		    if (sets[i][set1[new2old[set]][j]] == 0)
			break;
		}

		if (j == (length1 + 1))
		    if (finished[i]) {
			flag1 = 1;
			break;
		    }
	    }
	}

    } else

	flag1 = 1;

    if (length2 > 1) {
	for (i = 1; i <= nseqs; i++) {

	    if (sets[i][0] == length2) {

		for (j = 1; j <= length2; j++) {
		    if (sets[i][set2[new2old[set]][j]] == 0)
			break;
		}

		if (j == (length2 + 1))
		    if (finished[i]) {
			flag2 = 1;
			break;
		    }
	    }
	}

    } else

	flag2 = 1;

    if (flag1 * flag2)
	return 1;
    else
	return 0;
}

/*
 * Sort sets[1], sets[2], ... sets[nsets] according
 * to sets[1][0], sets[2][0], ... sets[nsets][0].
 *
 * Note: b[i]=j means, in the sorted array, the ith
 *       element corresponds to the jth element in
 *       the un-sorted array. 
 */
static void myqsort(int **a, int lo, int hi, int *b)
{
    int i, j;	
    int x;
    int *tmp;
    int t;
    i = lo;
    j = hi;

    /* the middle element */
    x= a[(lo+hi)/2][0];

    do {
	while (a[i][0] < x)
	    i++;
	while (a[j][0] > x)
	    j--;

	if (i <= j) {

	    /*
	     * remember the exchange
	     */
	    t=b[i];
	    b[i]=b[j];
	    b[j]=t;

	    /* exchange a[i] and a[j] */
	    tmp = a[i];
	    a[i] = a[j];
	    a[j] = tmp;
	    i++;
	    j--;

	}

    } while (i <= j);

    if (lo < j)
	myqsort(a, lo, j, b);

    if (i < hi)
	myqsort(a, i, hi, b);

    return;
}
