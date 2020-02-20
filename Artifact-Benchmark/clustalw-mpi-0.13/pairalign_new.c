/* Change int h to int gh everywhere  DES June 1994 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "clustalw.h"
#include "mpi.h"

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

#define gap(k)  ((k) <= 0 ? 0 : g + gh * (k))
#define tbgap(k)  ((k) <= 0 ? 0 : tb + gh * (k))
#define tegap(k)  ((k) <= 0 ? 0 : te + gh * (k))

/*
 *   Global variables
 */
#ifdef MAC
#define pwint   short
#else
#define pwint   int
#endif
static sint int_scale;

extern double **tmat;
extern float pw_go_penalty;
extern float pw_ge_penalty;
extern float transition_weight;
extern sint nseqs;
extern sint max_aa;
extern sint gap_pos1, gap_pos2;
extern sint max_aln_length;
extern sint *seqlen_array;
extern sint debug;
extern sint mat_avscore;
extern short blosum30mt[], pam350mt[], idmat[], pw_usermat[],
    pw_userdnamat[];
extern short clustalvdnamt[], swgapdnamt[];
extern short gon250mt[];
extern short def_dna_xref[], def_aa_xref[], pw_dna_xref[], pw_aa_xref[];
extern Boolean dnaflag;
extern char **seq_array;
extern char *amino_acid_codes;
extern char pw_mtrxname[];
extern char pw_dnamtrxname[];

static float mm_score;
static sint print_ptr, last_print;
static sint *displ;
static pwint *HH, *DD, *RR, *SS;
static sint g, gh;
static sint seq1, seq2,rseq1, rseq2;
static sint matrix[NUMRES][NUMRES];
static pwint maxscore;
static sint sb1, sb2, se1, se2;

/*
*	Prototypes
*/
static void add(sint v);
static sint calc_score(sint iat, sint jat, sint v1, sint v2);
static float tracepath(sint tsb1, sint tsb2);
static void forward_pass(char *ia, char *ib, sint n, sint m);
static void reverse_pass(char *ia, char *ib);
static sint diff(sint A, sint B, sint M, sint N, sint tb, sint te);
static void del(sint k);




sint pairalign(sint istart, sint iend, sint jstart, sint jend)
{
    short *mat_xref;
    static sint si, sj, i,j;
    static sint n, m, len1, len2, rlen1, rlen2;
    static sint maxres;
    static short *matptr;
    static char c;
    static float gscale, ghscale;
    int pidx,pdest;

    short *xarray,*yarray;
    double *tarray;
    int slen;
    int num_siarray;
    int ave_load;

    MPI_Status status;
    int np,work,position,idx1,idx2;
    char *mpi_buffer; /* My Buffer SIZE */
    char *mystr;
    int mystrlen,mybsize,myres_size;
    double wtime1, wtime2;
    double wtime99, wtime100;

    displ = (sint *) ckalloc((2 * max_aln_length + 1) * sizeof(sint));
    HH = (pwint *) ckalloc((max_aln_length) * sizeof(pwint));
    DD = (pwint *) ckalloc((max_aln_length) * sizeof(pwint));
    RR = (pwint *) ckalloc((max_aln_length) * sizeof(pwint));
    SS = (pwint *) ckalloc((max_aln_length) * sizeof(pwint));


#ifdef MAC
    int_scale = 10;
#else
    int_scale = 100;
#endif
    gscale = ghscale = 1.0;
    if (dnaflag) {
	if (debug > 1)
	    fprintf(stdout, "matrix %s\n", pw_dnamtrxname);
	if (strcmp(pw_dnamtrxname, "iub") == 0) {
	    matptr = swgapdnamt;
	    mat_xref = def_dna_xref;
	} else if (strcmp(pw_dnamtrxname, "clustalw") == 0) {
	    matptr = clustalvdnamt;
	    mat_xref = def_dna_xref;
	    gscale = 0.6667;
	    ghscale = 0.751;
	} else {
	    matptr = pw_userdnamat;
	    mat_xref = pw_dna_xref;
	}
	maxres = get_matrix(matptr, mat_xref, matrix, TRUE, int_scale);
	if (maxres == 0)
	    return ((sint) - 1);

	matrix[0][4] = transition_weight * matrix[0][0];
	matrix[4][0] = transition_weight * matrix[0][0];
	matrix[2][11] = transition_weight * matrix[0][0];
	matrix[11][2] = transition_weight * matrix[0][0];
	matrix[2][12] = transition_weight * matrix[0][0];
	matrix[12][2] = transition_weight * matrix[0][0];
    } else {
	if (debug > 1)
	    fprintf(stdout, "matrix %s\n", pw_mtrxname);
	if (strcmp(pw_mtrxname, "blosum") == 0) {
	    matptr = blosum30mt;
	    mat_xref = def_aa_xref;
	} else if (strcmp(pw_mtrxname, "pam") == 0) {
	    matptr = pam350mt;
	    mat_xref = def_aa_xref;
	} else if (strcmp(pw_mtrxname, "gonnet") == 0) {
	    matptr = gon250mt;
	    int_scale /= 10;
	    mat_xref = def_aa_xref;
	} else if (strcmp(pw_mtrxname, "id") == 0) {
	    matptr = idmat;
	    mat_xref = def_aa_xref;
	} else {
	    matptr = pw_usermat;
	    mat_xref = pw_aa_xref;
	}

	maxres = get_matrix(matptr, mat_xref, matrix, TRUE, int_scale);
	if (maxres == 0)
	    return ((sint) - 1);
    }

    /* 
     * Determine the number of available MPI processes
     */
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    wtime1 = MPI_Wtime();

    if (np < 2) {

	    /* do pairwise alignment sequentially ... */

    for (si = MAX(0, istart); si < nseqs && si < iend; si++) {

	n = seqlen_array[si + 1];
	len1 = 0;
	for (i = 1; i <= n; i++) {
	    c = seq_array[si + 1][i];
	    if ((c != gap_pos1) && (c != gap_pos2))
		len1++;
	}

	for (sj = MAX(si + 1, jstart + 1); sj < nseqs && sj < jend; sj++) {

	    m = seqlen_array[sj + 1];
	    if (n == 0 || m == 0) {
		tmat[si + 1][sj + 1] = 1.0;
		tmat[sj + 1][si + 1] = 1.0;
		continue;
	    }
	    len2 = 0;
	    for (i = 1; i <= m; i++) {
		c = seq_array[sj + 1][i];
		if ((c != gap_pos1) && (c != gap_pos2))
		    len2++;
	    }

	    if (dnaflag) {
		g = 2 * (float) pw_go_penalty *int_scale * gscale;
		gh = pw_ge_penalty * int_scale * ghscale;
	    } else {
		if (mat_avscore <= 0)
		    g = 2 * (float) (pw_go_penalty +
				     log((double) (MIN(n, m)))) *
			int_scale;
		else
		    g = 2 * mat_avscore * (float) (pw_go_penalty +
						   log((double)
						       (MIN(n, m)))) *
			gscale;
		gh = pw_ge_penalty * int_scale;
	    }

	    if (debug > 1)
		fprintf(stdout, "go %d ge %d\n", (pint) g, (pint) gh);

/*
   align the sequences
*/
	    seq1 = si + 1;
	    seq2 = sj + 1;

	    forward_pass(&seq_array[seq1][0], &seq_array[seq2][0], n, m);

	    reverse_pass(&seq_array[seq1][0], &seq_array[seq2][0]);

	    last_print = 0;
	    print_ptr = 1;
/*
        sb1 = sb2 = 1;
        se1 = n-1;
        se2 = m-1;
*/

/* use Myers and Miller to align two sequences */

	    maxscore = diff(sb1 - 1, sb2 - 1, se1 - sb1 + 1, se2 - sb2 + 1,
			    (sint) 0, (sint) 0);

/* calculate percentage residue identity */

	    mm_score = tracepath(sb1, sb2);

	    if (len1 == 0 || len2 == 0)
		mm_score = 0;
	    else
		mm_score /= (float) MIN(len1, len2);

	    tmat[si + 1][sj + 1] =
		((float) 100.0 - mm_score) / (float) 100.0;
	    tmat[sj + 1][si + 1] =
		((float) 100.0 - mm_score) / (float) 100.0;

	    if (debug > 1) {
		fprintf(stdout,
			"Sequences (%d:%d) Aligned1. Score: %d CompScore:  %d\n",
			(pint) si + 1, (pint) sj + 1, (pint) mm_score,
			(pint) maxscore / (MIN(len1, len2) * 100));
	    } else {
		info("Sequences (%d:%d) Aligned. Score:  %d",
		     (pint) si + 1, (pint) sj + 1, (pint) mm_score);
	    }

	}
    }

    } else {

    /* do pairwise alignment parallelly ... */
    /* Frist, sending all sequences to slaves ... */

	mybsize = 0;
	mybsize += (9+(nseqs+1)+(NUMRES)*(NUMRES))*sizeof(int) + 26*sizeof(char);
	mybsize += sizeof(char)+2*sizeof(int)+4*sizeof(float);
	for (i=1;i<=nseqs;i++)
	   mybsize += (seqlen_array[i]+1);

	for (pdest = 1; pdest < np ; pdest++ ){
    		MPI_Send(&mybsize, 1, MPI_INT, pdest, PAIRWISE_TAG, MPI_COMM_WORLD);
	}

 	mpi_buffer = (char *)malloc(mybsize*sizeof(char)); assert(mpi_buffer);
        assert(mpi_buffer);

    	position = 0;
    	MPI_Pack(&nseqs, 1, MPI_INT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
    	MPI_Pack(&istart, 1, MPI_INT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
    	MPI_Pack(&iend, 1, MPI_INT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
    	MPI_Pack(&jstart, 1, MPI_INT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
    	MPI_Pack(&jend, 1, MPI_INT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);

    	MPI_Pack(&dnaflag, 1, MPI_CHAR, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
    	MPI_Pack(&pw_go_penalty, 1, MPI_FLOAT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
    	MPI_Pack(&pw_ge_penalty, 1, MPI_FLOAT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
    	MPI_Pack(&int_scale, 1, MPI_INT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
    	MPI_Pack(&gscale, 1, MPI_FLOAT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
    	MPI_Pack(&ghscale, 1, MPI_FLOAT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
    	MPI_Pack(&mat_avscore, 1, MPI_INT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);


	MPI_Pack(seqlen_array, (nseqs+1), MPI_INT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
	for (i=1;i<=nseqs;i++) 
	    MPI_Pack(seq_array[i], seqlen_array[i]+1, MPI_CHAR, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
	MPI_Pack(amino_acid_codes, 26, MPI_CHAR, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
        MPI_Pack(&max_aa, 1, MPI_INT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
	MPI_Pack(&gap_pos1, 1, MPI_INT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
        MPI_Pack(&gap_pos2, 1, MPI_INT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
	MPI_Pack(&max_aln_length, 1, MPI_INT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
        for (i=0;i<NUMRES;i++)
		     MPI_Pack(*(matrix+i), NUMRES, MPI_INT, mpi_buffer,
				 mybsize, &position, MPI_COMM_WORLD);


	wtime99 = MPI_Wtime();

	for (pdest = 1; pdest < np ; pdest++ ){
	        MPI_Send(mpi_buffer, mybsize, MPI_PACKED, pdest, MY_DATA_TAG, MPI_COMM_WORLD);
	}
	free(mpi_buffer);

	wtime100 = MPI_Wtime();

	fprintf(stderr,"DEBUG: it takes %8.6f sec to send data to all slaves.\n",
		wtime100-wtime99);
	fflush(stderr);

	/* OK, now wait until all slaves have finished ... */


#ifdef STATIC_SCHEDULING_PAIRALIGN

	/* the default is to use dynamic scheduling */

        num_siarray=0;
        for (si = MAX(0, istart); si < nseqs && si < iend; si++) {
           for (sj = MAX(si + 1, jstart + 1); sj < nseqs && sj < jend; sj++) {
    	   num_siarray++;
           }
        }
    
        MPI_Comm_size(MPI_COMM_WORLD, &np);

        ave_load = (num_siarray/(np-1))/PAIRALIGN_NCHUNK;
	if (ave_load==0)
	    ave_load = 1;

	pdest = 0;
	work = 1;
	while(pdest*ave_load < num_siarray)
	{
	     if (work<np) {

	        MPI_Send(&pdest, 1, MPI_INT, work, WHICH_INTERVAL, MPI_COMM_WORLD);

		work++;

	     }else{


               MPI_Recv(&myres_size, 1, MPI_INT, MPI_ANY_SOURCE, MY_BSIZE_TAG, MPI_COMM_WORLD, &status);
               mpi_buffer=(char *)malloc(myres_size*sizeof(char));
   	       assert(mpi_buffer);
   
               MPI_Recv(mpi_buffer, myres_size, MPI_PACKED, status.MPI_SOURCE, MY_RESULT_TAG, MPI_COMM_WORLD, &status); 
   
   	       position=0;
               MPI_Unpack(mpi_buffer, myres_size, &position, &slen, 1, MPI_INT, MPI_COMM_WORLD);
   
               xarray = (short *)calloc(slen,sizeof(short));
               assert(xarray);
               yarray = (short *)calloc(slen,sizeof(short));
               assert(yarray);
               tarray = (double *)calloc(slen,sizeof(double));
               assert(tarray);
               MPI_Unpack(mpi_buffer, myres_size, &position, xarray, slen, MPI_SHORT, MPI_COMM_WORLD);
               MPI_Unpack(mpi_buffer, myres_size, &position, yarray, slen, MPI_SHORT, MPI_COMM_WORLD);
               MPI_Unpack(mpi_buffer, myres_size, &position, tarray, slen, MPI_DOUBLE, MPI_COMM_WORLD);
   
   	       for (i=0;i<slen;i++) {
   	
                  if ((xarray[i]==0) || (yarray[i]==0))
   		   continue;
   	          tmat[xarray[i]][yarray[i]] = tarray[i];
   	          tmat[yarray[i]][xarray[i]] = tarray[i];
   	       }
   
   	       free(mpi_buffer);
   	       free(xarray);
   	       free(yarray);
   	       free(tarray);

	       MPI_Send(&pdest, 1, MPI_INT, status.MPI_SOURCE, WHICH_INTERVAL, MPI_COMM_WORLD);

	     }
	
	     pdest++;
	}

	while(work>1){

               MPI_Recv(&myres_size, 1, MPI_INT, MPI_ANY_SOURCE, MY_BSIZE_TAG, MPI_COMM_WORLD, &status);
               mpi_buffer=(char *)malloc(myres_size*sizeof(char));
   	       assert(mpi_buffer);
   
               MPI_Recv(mpi_buffer, myres_size, MPI_PACKED, status.MPI_SOURCE, MY_RESULT_TAG, MPI_COMM_WORLD, &status); 
   
   	       position=0;
               MPI_Unpack(mpi_buffer, myres_size, &position, &slen, 1, MPI_INT, MPI_COMM_WORLD);
   
               xarray = (short *)calloc(slen,sizeof(short));
               assert(xarray);
               yarray = (short *)calloc(slen,sizeof(short));
               assert(yarray);
               tarray = (double *)calloc(slen,sizeof(double));
               assert(tarray);
               MPI_Unpack(mpi_buffer, myres_size, &position, xarray, slen, MPI_SHORT, MPI_COMM_WORLD);
               MPI_Unpack(mpi_buffer, myres_size, &position, yarray, slen, MPI_SHORT, MPI_COMM_WORLD);
               MPI_Unpack(mpi_buffer, myres_size, &position, tarray, slen, MPI_DOUBLE, MPI_COMM_WORLD);
   
   	       for (i=0;i<slen;i++) {
   	
                  if ((xarray[i]==0) || (yarray[i]==0))
   		   continue;
   	          tmat[xarray[i]][yarray[i]] = tarray[i];
   	          tmat[yarray[i]][xarray[i]] = tarray[i];
   	       }
   
   	       free(mpi_buffer);
   	       free(xarray);
   	       free(yarray);
   	       free(tarray);

	       pdest = -1;
	       MPI_Send(&pdest, 1, MPI_INT, status.MPI_SOURCE, WHICH_INTERVAL, MPI_COMM_WORLD);
	    work--;
	}

#else
        /* we will be using "STATIC_SCHEDULING_PAIRALIGN". */

	pdest = 1;

	while (pdest < np) {


            MPI_Recv(&myres_size, 1, MPI_INT, MPI_ANY_SOURCE, MY_BSIZE_TAG, MPI_COMM_WORLD, &status);
            mpi_buffer=(char *)malloc(myres_size*sizeof(char));
	    assert(mpi_buffer);

         //   MPI_Recv(mpi_buffer, myres_size, MPI_PACKED, status.MPI_SOURCE, MY_RESULT_TAG, MPI_COMM_WORLD, &status); 
MPI_Recv(mpi_buffer, myres_size, MPI_PACKED, MPI_ANY_SOURCE, MY_RESULT_TAG, MPI_COMM_WORLD, &status);  //YHB_version
	    position=0;
            MPI_Unpack(mpi_buffer, myres_size, &position, &slen, 1, MPI_INT, MPI_COMM_WORLD);

            xarray = (short *)calloc(slen,sizeof(short));
            assert(xarray);
            yarray = (short *)calloc(slen,sizeof(short));
            assert(yarray);
            tarray = (double *)calloc(slen,sizeof(double));
            assert(tarray);
            MPI_Unpack(mpi_buffer, myres_size, &position, xarray, slen, MPI_SHORT, MPI_COMM_WORLD);
            MPI_Unpack(mpi_buffer, myres_size, &position, yarray, slen, MPI_SHORT, MPI_COMM_WORLD);
            MPI_Unpack(mpi_buffer, myres_size, &position, tarray, slen, MPI_DOUBLE, MPI_COMM_WORLD);

	    for (i=0;i<slen;i++) {
	
               if ((xarray[i]==0) || (yarray[i]==0))
		   continue;
	       tmat[xarray[i]][yarray[i]] = tarray[i];
	       tmat[yarray[i]][xarray[i]] = tarray[i];
	    }


	    free(mpi_buffer);
	    free(xarray);
	    free(yarray);
	    free(tarray);
	    pdest++;
	}
#endif



    }

	/** Measuring wall time by MPI_Wtime() **/
	wtime2 = MPI_Wtime();

	fprintf(stderr,"\nDEBUG: pairalign time = %5.3f sec\n", wtime2 - wtime1);
	fflush(stderr);




    displ = ckfree((void *) displ);
    HH = ckfree((void *) HH);
    DD = ckfree((void *) DD);
    RR = ckfree((void *) RR);
    SS = ckfree((void *) SS);


    return ((sint) 1);
}


static void add(sint v)
{

    if (last_print < 0) {
	displ[print_ptr - 1] = v;
	displ[print_ptr++] = last_print;
    } else
	last_print = displ[print_ptr++] = v;
}

static sint calc_score(sint iat, sint jat, sint v1, sint v2)
{
    sint ipos, jpos;
    sint ret;

    ipos = v1 + iat;
    jpos = v2 + jat;

    ret = matrix[(int) seq_array[seq1][ipos]][(int) seq_array[seq2][jpos]];

    return (ret);
}


static float tracepath(sint tsb1, sint tsb2)
{
    char c1, c2;
    sint i1, i2, r;
    sint i, k, pos, to_do;
    sint count;
    float score;
    char s1[600], s2[600];

    to_do = print_ptr - 1;
    i1 = tsb1;
    i2 = tsb2;

    pos = 0;
    count = 0;
    for (i = 1; i <= to_do; ++i) {

	if (debug > 1)
	    fprintf(stdout, "%d ", (pint) displ[i]);
	if (displ[i] == 0) {
	    c1 = seq_array[seq1][i1];
	    c2 = seq_array[seq2][i2];

	    if (debug > 0) {
		if (c1 > max_aa)
		    s1[pos] = '-';
		else
		    s1[pos] = amino_acid_codes[c1];
		if (c2 > max_aa)
		    s2[pos] = '-';
		else
		    s2[pos] = amino_acid_codes[c2];
	    }

	    if ((c1 != gap_pos1) && (c1 != gap_pos2) && (c1 == c2))
		count++;
	    ++i1;
	    ++i2;
	    ++pos;
	} else {
	    if ((k = displ[i]) > 0) {

		if (debug > 0)
		    for (r = 0; r < k; r++) {
			s1[pos + r] = '-';
			if (seq_array[seq2][i2 + r] > max_aa)
			    s2[pos + r] = '-';
			else
			    s2[pos + r] =
				amino_acid_codes[seq_array[seq2][i2 + r]];
		    }

		i2 += k;
		pos += k;
	    } else {

		if (debug > 0)
		    for (r = 0; r < (-k); r++) {
			s2[pos + r] = '-';
			if (seq_array[seq1][i1 + r] > max_aa)
			    s1[pos + r] = '-';
			else
			    s1[pos + r] =
				amino_acid_codes[seq_array[seq1][i1 + r]];
		    }

		i1 -= k;
		pos -= k;
	    }
	}
    }
    if (debug > 0)
	fprintf(stdout, "\n");
    if (debug > 0) {
	for (i = 0; i < pos; i++)
	    fprintf(stdout, "%c", s1[i]);
	fprintf(stdout, "\n");
	for (i = 0; i < pos; i++)
	    fprintf(stdout, "%c", s2[i]);
	fprintf(stdout, "\n");
    }
/*
        if (count <= 0) count = 1;
*/
    score = 100.0 * (float) count;
    return (score);
}


static void forward_pass(char *ia, char *ib, sint n, sint m)
{

    sint i, j;
    pwint f, hh, p, t;

    maxscore = 0;
    se1 = se2 = 0;
    for (i = 0; i <= m; i++) {
	HH[i] = 0;
	DD[i] = -g;
    }

    for (i = 1; i <= n; i++) {
	hh = p = 0;
	f = -g;

	for (j = 1; j <= m; j++) {

	    f -= gh;
	    t = hh - g - gh;
	    if (f < t)
		f = t;

	    DD[j] -= gh;
	    t = HH[j] - g - gh;
	    if (DD[j] < t)
		DD[j] = t;

	    hh = p + matrix[(int) ia[i]][(int) ib[j]];
	    if (hh < f)
		hh = f;
	    if (hh < DD[j])
		hh = DD[j];
	    if (hh < 0)
		hh = 0;

	    p = HH[j];
	    HH[j] = hh;

	    if (hh > maxscore) {
		maxscore = hh;
		se1 = i;
		se2 = j;
	    }
	}
    }

}


static void reverse_pass(char *ia, char *ib)
{

    sint i, j;
    pwint f, hh, p, t;
    pwint cost;

    cost = 0;
    sb1 = sb2 = 1;
    for (i = se2; i > 0; i--) {
	HH[i] = -1;
	DD[i] = -1;
    }

    for (i = se1; i > 0; i--) {
	hh = f = -1;
	if (i == se1)
	    p = 0;
	else
	    p = -1;

	for (j = se2; j > 0; j--) {

	    f -= gh;
	    t = hh - g - gh;
	    if (f < t)
		f = t;

	    DD[j] -= gh;
	    t = HH[j] - g - gh;
	    if (DD[j] < t)
		DD[j] = t;

	    hh = p + matrix[(int) ia[i]][(int) ib[j]];
	    if (hh < f)
		hh = f;
	    if (hh < DD[j])
		hh = DD[j];

	    p = HH[j];
	    HH[j] = hh;

	    if (hh > cost) {
		cost = hh;
		sb1 = i;
		sb2 = j;
		if (cost >= maxscore)
		    break;
	    }
	}
	if (cost >= maxscore)
	    break;
    }

}

static int diff(sint A, sint B, sint M, sint N, sint tb, sint te)
{
    sint type;
    sint midi, midj, i, j;
    int midh;
    static pwint f, hh, e, s, t;

    if (N <= 0) {
	if (M > 0) {
	    del(M);
	}

	return (-(int) tbgap(M));
    }

    if (M <= 1) {
	if (M <= 0) {
	    add(N);
	    return (-(int) tbgap(N));
	}

	midh = -(tb + gh) - tegap(N);
	hh = -(te + gh) - tbgap(N);
	if (hh > midh)
	    midh = hh;
	midj = 0;
	for (j = 1; j <= N; j++) {
	    hh = calc_score(1, j, A, B)
		- tegap(N - j) - tbgap(j - 1);
	    if (hh > midh) {
		midh = hh;
		midj = j;
	    }
	}

	if (midj == 0) {
	    del(1);
	    add(N);
	} else {
	    if (midj > 1)
		add(midj - 1);
	    displ[print_ptr++] = last_print = 0;
	    if (midj < N)
		add(N - midj);
	}
	return midh;
    }

/* Divide: Find optimum midpoint (midi,midj) of cost midh */

    midi = M / 2;
    HH[0] = 0.0;
    t = -tb;
    for (j = 1; j <= N; j++) {
	HH[j] = t = t - gh;
	DD[j] = t - g;
    }

    t = -tb;
    for (i = 1; i <= midi; i++) {
	s = HH[0];
	HH[0] = hh = t = t - gh;
	f = t - g;
	for (j = 1; j <= N; j++) {
	    if ((hh = hh - g - gh) > (f = f - gh))
		f = hh;
	    if ((hh = HH[j] - g - gh) > (e = DD[j] - gh))
		e = hh;
	    hh = s + calc_score(i, j, A, B);
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

    RR[N] = 0;
    t = -te;
    for (j = N - 1; j >= 0; j--) {
	RR[j] = t = t - gh;
	SS[j] = t - g;
    }

    t = -te;
    for (i = M - 1; i >= midi; i--) {
	s = RR[N];
	RR[N] = hh = t = t - gh;
	f = t - g;

	for (j = N - 1; j >= 0; j--) {

	    if ((hh = hh - g - gh) > (f = f - gh))
		f = hh;
	    if ((hh = RR[j] - g - gh) > (e = SS[j] - gh))
		e = hh;
	    hh = s + calc_score(i + 1, j + 1, A, B);
	    if (f > hh)
		hh = f;
	    if (e > hh)
		hh = e;

	    s = RR[j];
	    RR[j] = hh;
	    SS[j] = e;

	}
    }

    SS[N] = RR[N];

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
	hh = DD[j] + SS[j] + g;
	if (hh > midh) {
	    midh = hh;
	    midj = j;
	    type = 2;
	}
    }

    /* Conquer recursively around midpoint  */


    if (type == 1) {		/* Type 1 gaps  */
	diff(A, B, midi, midj, tb, g);
	diff(A + midi, B + midj, M - midi, N - midj, g, te);
    } else {
	diff(A, B, midi - 1, midj, tb, 0.0);
	del(2);
	diff(A + midi + 1, B + midj, M - midi - 1, N - midj, 0.0, te);
    }

    return midh;		/* Return the score of the best alignment */
}

static void del(sint k)
{
    if (last_print < 0)
	last_print = displ[print_ptr - 1] -= k;
    else
	last_print = displ[print_ptr++] = -(k);
}
