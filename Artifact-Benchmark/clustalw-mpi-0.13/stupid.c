
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "clustalw.h"
#include "mpi.h"

static void padd(sint k);
static void pdel(sint k);
static void palign(void);
static lint prfscore(sint n, sint m);
static sint gap_penalty1(sint i, sint j, sint k);
static sint open_penalty1(sint i, sint j);
static sint ext_penalty1(sint i, sint j);
static sint gap_penalty2(sint i, sint j, sint k);
static sint open_penalty2(sint i, sint j);
static sint ext_penalty2(sint i, sint j);

static lint *HH, *DD, *RR, *SS;
static lint *gS;

static  sint print_ptr;
static  sint last_print;
static lint *displ;
static sint max_aa;
static sint max_aln_length;
static sint gap_pos1, gap_pos2;
static sint **profile1, **profile2;
static Boolean endgappenalties;
static sint prf_length1, prf_length2;
static int reverse_rank;

static lint mypdiff(sint A, sint B, sint M, sint N, sint go1, sint go2);

/*
 * Using MPI to computer the preverse_pass() upto this level
 * of the recursive mypdiff() calls.
 */
static int plevel;
#define MYPDIFFLEVEL 3

/*******************************************************************
 * Input from prfalign():
 * a,b,c,d,go1,go2,print_ptr,last_print,displ,max_aln_length,max_aa,
 * gap_pos1,gap_pos2,profile1,profile2,pendgappenalties,
 * prf_length1,prf_length2;
 *
 * Output to prfalign():
 * score, print_ptr, last_print, displ;
 *
 *******************************************************************/
void stupid(int a, int b, int c, int d, sint go1, sint go2, 
	      lint *pscore, sint *pprint_ptr, sint *plast_print, lint **pdispl,
	      int pmax_aln_length, sint pmax_aa,
	      sint pgap_pos1, sint pgap_pos2,
	      sint **pprofile1, sint **pprofile2,
	      Boolean pendgappenalties,
	      sint pprf_length1, sint pprf_length2,
	      int preverse_rank){

    lint score;

    print_ptr = *pprint_ptr;
    last_print = *plast_print;
    max_aa = pmax_aa;
    gap_pos1 = pgap_pos1;
    gap_pos2 = pgap_pos2;
    profile1 = pprofile1;
    profile2 = pprofile2;
    pendgappenalties = endgappenalties;
    prf_length1 = pprf_length1;
    prf_length2 = pprf_length2;
    max_aln_length = pmax_aln_length;
    reverse_rank = preverse_rank;

    plevel = 0;

    HH = (lint *) ckalloc((max_aln_length + 1) * sizeof(lint));
    DD = (lint *) ckalloc((max_aln_length + 1) * sizeof(lint));
    RR = (lint *) ckalloc((max_aln_length + 1) * sizeof(lint));
    SS = (lint *) ckalloc((max_aln_length + 1) * sizeof(lint));
    gS = (lint *) ckalloc((max_aln_length + 1) * sizeof(lint));
    displ = (sint *) ckalloc((max_aln_length + 1) * sizeof(sint));

    score = mypdiff(a, b, c, d, go1, go2);

    HH = ckfree((void *) HH);
    DD = ckfree((void *) DD);
    RR = ckfree((void *) RR);
    SS = ckfree((void *) SS);
    gS = ckfree((void *) gS);

     *pprint_ptr = print_ptr;
     *plast_print = last_print;
     *pscore = score;
     *pdispl = displ;

    return ;
}


static lint mypdiff(sint A, sint B, sint M, sint N, sint go1, sint go2)
{
    sint midi, midj, type;
    lint midh;
    lint t, tl, g, h;
    sint i, j;
    lint hh, f, e, s;
    int mybsize,position;
    char *mpi_buffer;
    MPI_Status status;

    /* remember the number of times that mypdiff() have been called */
    plevel++;

/* Boundary cases: M <= 1 or N == 0 */

/* if sequence B is empty....                                            */

	if (N <= 0) {

/* if sequence A is not empty....                                        */

	    if (M > 0) {

/* delete residues A[1] to A[M]                                          */

		pdel(M);
	    }
	    return (-gap_penalty1(A, B, M));
	}

/* if sequence A is empty....                                            */

	if (M <= 1) {
	    if (M <= 0) {

/* insert residues B[1] to B[N]                                          */

		padd(N);
		return (-gap_penalty2(A, B, N));
	    }

/* if sequence A has just one residue....                                */

	    if (go1 == 0)
		midh = -gap_penalty1(A + 1, B + 1, N);
	    else
		midh =
		    -gap_penalty2(A + 1, B, 1) - gap_penalty1(A + 1, B + 1,
							      N);
	    midj = 0;
	    for (j = 1; j <= N; j++) {
		hh = -gap_penalty1(A, B + 1, j - 1) + prfscore(A + 1,
							       B + j)
		    - gap_penalty1(A + 1, B + j + 1, N - j);
		if (hh > midh) {
		    midh = hh;
		    midj = j;
		}
	    }

	    if (midj == 0) {
		padd(N);
		pdel(1);
	    } else {
		if (midj > 1)
		    padd(midj - 1);
		palign();
		if (midj < N)
		    padd(N - midj);
	    }
	    return midh;
	}


/* Divide sequence A in half: midi */

	midi = M / 2;

/* In a forward phase, calculate all HH[j] and HH[j] */

	HH[0] = 0.0;
	t = -open_penalty1(A, B + 1);
	tl = -ext_penalty1(A, B + 1);
	for (j = 1; j <= N; j++) {
	    HH[j] = t = t + tl;
	    DD[j] = t - open_penalty2(A + 1, B + j);
	}

	if (go1 == 0)
	    t = 0;
	else
	    t = -open_penalty2(A + 1, B);
	tl = -ext_penalty2(A + 1, B);


#ifdef OLD_CODES
	for (i = 1; i <= midi; i++) {
	    s = HH[0];
	    HH[0] = hh = t = t + tl;
	    f = t - open_penalty1(A + i, B + 1);

	    for (j = 1; j <= N; j++) {
		g = open_penalty1(A + i, B + j);
		h = ext_penalty1(A + i, B + j);
		if ((hh = hh - g - h) > (f = f - h))
		    f = hh;
		g = open_penalty2(A + i, B + j);
		h = ext_penalty2(A + i, B + j);
		if ((hh = HH[j] - g - h) > (e = DD[j] - h))
		    e = hh;
		hh = s + prfscore(A + i, B + j);
		if (f > hh)
		    hh = f;
		if (e > hh)
		    hh = e;

		s = HH[j];
		HH[j] = hh;
		DD[j] = e;

	    }
	}

	DD[0] = HH[0];

/* In a reverse phase, calculate all RR[j] and SS[j] */

	RR[N] = 0.0;
	tl = 0.0;
	for (j = N - 1; j >= 0; j--) {
	    g = -open_penalty1(A + M, B + j + 1);
	    tl -= ext_penalty1(A + M, B + j + 1);
	    RR[j] = g + tl;
	    SS[j] = RR[j] - open_penalty2(A + M, B + j);
	    gS[j] = open_penalty2(A + M, B + j);
	}

	tl = 0.0;
	for (i = M - 1; i >= midi; i--) {
	    s = RR[N];
	    if (go2 == 0)
		g = 0;
	    else
		g = -open_penalty2(A + i + 1, B + N);
	    tl -= ext_penalty2(A + i + 1, B + N);
	    RR[N] = hh = g + tl;
	    t = open_penalty1(A + i, B + N);
	    f = RR[N] - t;

	    for (j = N - 1; j >= 0; j--) {
		g = open_penalty1(A + i, B + j + 1);
		h = ext_penalty1(A + i, B + j + 1);
		if ((hh = hh - g - h) > (f = f - h - g + t))
		    f = hh;
		t = g;
		g = open_penalty2(A + i + 1, B + j);
		h = ext_penalty2(A + i + 1, B + j);
		hh = RR[j] - g - h;
		if (i == (M - 1)) {
		    e = SS[j] - h;
		} else {
		    e = SS[j] - h - g + open_penalty2(A + i + 2, B + j);
		    gS[j] = g;
		}
		if (hh > e)
		    e = hh;
		hh = s + prfscore(A + i + 1, B + j + 1);
		if (f > hh)
		    hh = f;
		if (e > hh)
		    hh = e;

		s = RR[j];
		RR[j] = hh;
		SS[j] = e;

	    }
	}
#endif


	/*
	 * "reverse_rank" is the rank of the MPI process that is going to
	 * execute preverse_pass().
	 */

	    if (reverse_rank) {


		/* determine buffer size */

		mybsize = 0;
		mybsize += 9 * sizeof(int) + sizeof(Boolean);
		for (i = A; i < A + M + 2; i++)
		    mybsize += (LENCOL + 2) * sizeof(sint);
		for (i = B; i < B + N + 2; i++)
		    mybsize += (LENCOL + 2) * sizeof(sint);

		MPI_Send(&mybsize, 1, MPI_INT, reverse_rank, PREVERSE_TAG,
			 MPI_COMM_WORLD);
		mpi_buffer = (char *) malloc(mybsize * sizeof(char));
		assert(mpi_buffer);

		position = 0;

		MPI_Pack(&prf_length1, 1, MPI_INT, mpi_buffer, mybsize,
			 &position, MPI_COMM_WORLD);
		MPI_Pack(&prf_length2, 1, MPI_INT, mpi_buffer, mybsize,
			 &position, MPI_COMM_WORLD);
		MPI_Pack(&endgappenalties, 1, MPI_CHAR, mpi_buffer,
			 mybsize, &position, MPI_COMM_WORLD);
		MPI_Pack(&max_aa, 1, MPI_INT, mpi_buffer, mybsize,
			 &position, MPI_COMM_WORLD);

		MPI_Pack(&midi, 1, MPI_INT, mpi_buffer, mybsize, &position,
			 MPI_COMM_WORLD);
		MPI_Pack(&go2, 1, MPI_INT, mpi_buffer, mybsize, &position,
			 MPI_COMM_WORLD);
		MPI_Pack(&A, 1, MPI_INT, mpi_buffer, mybsize, &position,
			 MPI_COMM_WORLD);
		MPI_Pack(&B, 1, MPI_INT, mpi_buffer, mybsize, &position,
			 MPI_COMM_WORLD);
		MPI_Pack(&M, 1, MPI_INT, mpi_buffer, mybsize, &position,
			 MPI_COMM_WORLD);
		MPI_Pack(&N, 1, MPI_INT, mpi_buffer, mybsize, &position,
			 MPI_COMM_WORLD);

		for (i = A; i < A + M + 2; i++)
		    MPI_Pack(profile1[i], (LENCOL + 2), MPI_INT,
			     mpi_buffer, mybsize, &position,
			     MPI_COMM_WORLD);
		for (i = B; i < B + N + 2; i++)
		    MPI_Pack(profile2[i], (LENCOL + 2), MPI_INT,
			     mpi_buffer, mybsize, &position,
			     MPI_COMM_WORLD);

		MPI_Send(mpi_buffer, mybsize, MPI_PACKED, reverse_rank,
			 MY_DATA_TAG, MPI_COMM_WORLD);
		free(mpi_buffer);


		/* in the meantime, we will be computing pforward_pass() */
		pforward_pass(midi, t, tl, A, B, N, HH, DD);
		DD[0] = HH[0];




#ifdef DEBUG
	{
	    int rank;
	    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	    fprintf(stderr,"DEBUG: my rank=%d, line 415, rank %d is doing preverse_pass()\n", 
		    rank,reverse_rank);
	    fflush(stderr);
	}
#endif



		/* preverse_pass(midi, A, B, M, N, go2, RR, SS, gS); */


		/* receiving RR[], SS[], gS[] */

		MPI_Recv(&mybsize, 1, MPI_INT, reverse_rank, MY_RESULT_TAG,
			 MPI_COMM_WORLD, &status);

		mpi_buffer = (char *) malloc(mybsize * sizeof(char));
		assert(mpi_buffer);

		position = 0;

		MPI_Recv(mpi_buffer, mybsize, MPI_PACKED, reverse_rank,
			 MY_RESULT_TAG, MPI_COMM_WORLD, &status);


		MPI_Unpack(mpi_buffer, mybsize, &position,
			   RR, (max_aln_length + 1), MPI_INT,
			   MPI_COMM_WORLD);
		MPI_Unpack(mpi_buffer, mybsize, &position, SS,
			   (max_aln_length + 1), MPI_INT, MPI_COMM_WORLD);
		MPI_Unpack(mpi_buffer, mybsize, &position, gS,
			   (max_aln_length + 1), MPI_INT, MPI_COMM_WORLD);

		free(mpi_buffer);

		if (plevel>= (MYPDIFFLEVEL))
		   reverse_rank = 0;

	    } else {

		/* single process */

		pforward_pass(midi, t, tl, A, B, N, HH, DD);
		DD[0] = HH[0];

		preverse_pass(midi, A, B, M, N, go2, RR, SS, gS);

	    }












	SS[N] = RR[N];
	gS[N] = open_penalty2(A + midi + 1, B + N);

/* find midj, such that HH[j]+RR[j] or DD[j]+SS[j]+gap is the maximum */

	midh = HH[0] + RR[0];
	midj = 0;
	type = 1;
	for (j = 0; j <= N; j++) {
	    hh = HH[j] + RR[j];
	    if (hh >= midh)
		if (hh > midh || (HH[j] != DD[j] && RR[j] == SS[j])) {
		    midh = hh;
		    midj = j;
		}
	}

	for (j = N; j >= 0; j--) {
	    hh = DD[j] + SS[j] + gS[j];
	    if (hh > midh) {
		midh = hh;
		midj = j;
		type = 2;
	    }
	}


/* Conquer recursively around midpoint                                   */


    if (type == 1) {		/* Type 1 gaps  */
	mypdiff(A, B, midi, midj, go1, 1);
	mypdiff(A + midi, B + midj, M - midi, N - midj, 1, go2);
    } else {
	mypdiff(A, B, midi - 1, midj, go1, 0);
	pdel(2);
	mypdiff(A + midi + 1, B + midj, M - midi - 1, N - midj, 0, go2);
    }

    return midh;		/* Return the score of the best alignment */
}

static void pdel(sint k)
{
    if (last_print < 0)
	last_print = displ[print_ptr - 1] -= k;
    else
	last_print = displ[print_ptr++] = -(k);
}

static void padd(sint k)
{

    if (last_print < 0) {
	displ[print_ptr - 1] = k;
	displ[print_ptr++] = last_print;
    } else
	last_print = displ[print_ptr++] = k;
}

static void palign(void)
{
    displ[print_ptr++] = last_print = 0;
}



static lint prfscore(sint n, sint m)
{
    sint ix;
    lint score;

    score = 0.0;
    for (ix = 0; ix <= max_aa; ix++) {
	score += (profile1[n][ix] * profile2[m][ix]);
    }
    score += (profile1[n][gap_pos1] * profile2[m][gap_pos1]);
    score += (profile1[n][gap_pos2] * profile2[m][gap_pos2]);
    return (score / 10);

}

/* calculate the score for opening a gap at residues A[i] and B[j]       */

static sint open_penalty1(sint i, sint j)
{
    sint g;

    if (!endgappenalties && (i == 0 || i == prf_length1))
	return (0);

    g = profile2[j][GAPCOL] + profile1[i][GAPCOL];
    return (g);
}

/* calculate the score for extending an existing gap at A[i] and B[j]    */

static sint ext_penalty1(sint i, sint j)
{
    sint h;

    if (!endgappenalties && (i == 0 || i == prf_length1))
	return (0);

    h = profile2[j][LENCOL];
    return (h);
}

/* calculate the score for a gap of length k, at residues A[i] and B[j]  */

static sint gap_penalty1(sint i, sint j, sint k)
{
    sint ix;
    sint gp;
    sint g, h = 0;

    if (k <= 0)
	return (0);
    if (!endgappenalties && (i == 0 || i == prf_length1))
	return (0);

    g = profile2[j][GAPCOL] + profile1[i][GAPCOL];
    for (ix = 0; ix < k && ix + j < prf_length2; ix++)
	h = profile2[ix + j][LENCOL];

    gp = g + h * k;
    return (gp);
}

/* calculate the score for opening a gap at residues A[i] and B[j]       */

static sint open_penalty2(sint i, sint j)
{
    sint g;

    if (!endgappenalties && (j == 0 || j == prf_length2))
	return (0);

    g = profile1[i][GAPCOL] + profile2[j][GAPCOL];
    return (g);
}

/* calculate the score for extending an existing gap at A[i] and B[j]    */

static sint ext_penalty2(sint i, sint j)
{
    sint h;

    if (!endgappenalties && (j == 0 || j == prf_length2))
	return (0);

    h = profile1[i][LENCOL];
    return (h);
}

/* calculate the score for a gap of length k, at residues A[i] and B[j]  */

static sint gap_penalty2(sint i, sint j, sint k)
{
    sint ix;
    sint gp;
    sint g, h = 0;

    if (k <= 0)
	return (0);
    if (!endgappenalties && (j == 0 || j == prf_length2))
	return (0);

    g = profile1[i][GAPCOL] + profile2[j][GAPCOL];
    for (ix = 0; ix < k && ix + i < prf_length1; ix++)
	h = profile1[ix + i][LENCOL];

    gp = g + h * k;
    return (gp);
}
