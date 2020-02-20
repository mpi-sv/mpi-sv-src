#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "clustalw.h"

#include <mpi.h>

static void make_p_ptrs(sint * tptr, sint * pl, sint naseq, sint l);
static void make_n_ptrs(sint * tptr, sint * pl, sint naseq, sint len);
static void put_frag(sint fs, sint v1, sint v2, sint flen);
static sint frag_rel_pos(sint a1, sint b1, sint a2, sint b2);
static void des_quick_sort(sint * array1, sint * array2, sint array_size);
static void pair_align(sint seq_no, sint l1, sint l2);

typedef struct {
	int i;
	int j;
}IANDJ;


/*
*	Prototypes
*/

/*
*	 Global variables
*/
extern sint *seqlen_array;
extern char **seq_array;
extern sint dna_ktup, dna_window, dna_wind_gap, dna_signif;	/* params for DNA */
extern sint prot_ktup, prot_window, prot_wind_gap, prot_signif;	/* params for prots */
extern sint nseqs;
extern Boolean dnaflag;
extern double **tmat;
extern sint max_aa;
extern sint max_aln_length;

static sint next;
static sint curr_frag, maxsf, vatend;
static sint **accum;
static sint *diag_index;
static char *slopes;

sint ktup, window, wind_gap, signif;	/* Pairwise aln. params */
sint *displ;
sint *zza, *zzb, *zzc, *zzd;

extern Boolean percent;


static void make_p_ptrs(sint * tptr, sint * pl, sint naseq, sint l)
{
    static sint a[10];
    sint i, j, limit, code, flag;
    char residue;

    for (i = 1; i <= ktup; i++)
	a[i] = (sint) pow((double) (max_aa + 1), (double) (i - 1));

    limit = (sint) pow((double) (max_aa + 1), (double) ktup);
    for (i = 1; i <= limit; ++i)
	pl[i] = 0;
    for (i = 1; i <= l; ++i)
	tptr[i] = 0;

    for (i = 1; i <= (l - ktup + 1); ++i) {
	code = 0;
	flag = FALSE;
	for (j = 1; j <= ktup; ++j) {
	    residue = seq_array[naseq][i + j - 1];
	    if ((residue < 0) || (residue > max_aa)) {
		flag = TRUE;
		break;
	    }
	    code += ((residue) * a[j]);
	}
	if (flag)
	    continue;
	++code;
	if (pl[code] != 0)
	    tptr[i] = pl[code];
	pl[code] = i;
    }
}


static void make_n_ptrs(sint * tptr, sint * pl, sint naseq, sint len)
{
    static sint pot[] = { 0, 1, 4, 16, 64, 256, 1024, 4096 };
    sint i, j, limit, code, flag;
    char residue;

    limit = (sint) pow((double) 4, (double) ktup);

    for (i = 1; i <= limit; ++i)
	pl[i] = 0;
    for (i = 1; i <= len; ++i)
	tptr[i] = 0;

    for (i = 1; i <= len - ktup + 1; ++i) {
	code = 0;
	flag = FALSE;
	for (j = 1; j <= ktup; ++j) {
	    residue = seq_array[naseq][i + j - 1];
	    if ((residue < 0) || (residue > 4)) {
		flag = TRUE;
		break;
	    }
	    code += ((residue) * pot[j]);	/* DES */
	}
	if (flag)
	    continue;
	++code;
	if (pl[code] != 0)
	    tptr[i] = pl[code];
	pl[code] = i;
    }
}


static void put_frag(sint fs, sint v1, sint v2, sint flen)
{
    sint end;
    accum[0][curr_frag] = fs;
    accum[1][curr_frag] = v1;
    accum[2][curr_frag] = v2;
    accum[3][curr_frag] = flen;

    if (!maxsf) {
	maxsf = 1;
	accum[4][curr_frag] = 0;
	return;
    }

    if (fs >= accum[0][maxsf]) {
	accum[4][curr_frag] = maxsf;
	maxsf = curr_frag;
	return;
    } else {
	next = maxsf;
	while (TRUE) {
	    end = next;
	    next = accum[4][next];
	    if (fs >= accum[0][next])
		break;
	}
	accum[4][curr_frag] = next;
	accum[4][end] = curr_frag;
    }
}


static sint frag_rel_pos(sint a1, sint b1, sint a2, sint b2)
{
    sint ret;

    ret = FALSE;
    if (a1 - b1 == a2 - b2) {
	if (a2 < a1)
	    ret = TRUE;
    } else {
	if (a2 + ktup - 1 < a1 && b2 + ktup - 1 < b1)
	    ret = TRUE;
    }
    return ret;
}


static void des_quick_sort(sint * array1, sint * array2, sint array_size)
/*  */
/* Quicksort routine, adapted from chapter 4, page 115 of software tools */
/* by Kernighan and Plauger, (1986) */
/* Sort the elements of array1 and sort the */
/* elements of array2 accordingly */
/*  */
{
    sint temp1, temp2;
    sint p, pivlin;
    sint i, j;
    sint lst[50], ust[50];	/* the maximum no. of elements must be */
    /* < log(base2) of 50 */

    lst[1] = 1;
    ust[1] = array_size - 1;
    p = 1;

    while (p > 0) {
	if (lst[p] >= ust[p])
	    p--;
	else {
	    i = lst[p] - 1;
	    j = ust[p];
	    pivlin = array1[j];
	    while (i < j) {
		for (i = i + 1; array1[i] < pivlin; i++);
		for (j = j - 1; j > i; j--)
		    if (array1[j] <= pivlin)
			break;
		if (i < j) {
		    temp1 = array1[i];
		    array1[i] = array1[j];
		    array1[j] = temp1;

		    temp2 = array2[i];
		    array2[i] = array2[j];
		    array2[j] = temp2;
		}
	    }

	    j = ust[p];

	    temp1 = array1[i];
	    array1[i] = array1[j];
	    array1[j] = temp1;

	    temp2 = array2[i];
	    array2[i] = array2[j];
	    array2[j] = temp2;

	    if (i - lst[p] < ust[p] - i) {
		lst[p + 1] = lst[p];
		ust[p + 1] = i - 1;
		lst[p] = i + 1;
	    } else {
		lst[p + 1] = i + 1;
		ust[p + 1] = ust[p];
		ust[p] = i - 1;
	    }
	    p = p + 1;
	}
    }
    return;

}





static void pair_align(sint seq_no, sint l1, sint l2)
{
    sint pot[8], i, j, l, m, flag, limit, pos, tl1, vn1, vn2, flen, osptr,
	fs;
    sint tv1, tv2, encrypt, subt1, subt2, rmndr;
    char residue;

    if (dnaflag) {
	for (i = 1; i <= ktup; ++i)
	    pot[i] = (sint) pow((double) 4, (double) (i - 1));
	limit = (sint) pow((double) 4, (double) ktup);
    } else {
	for (i = 1; i <= ktup; i++)
	    pot[i] = (sint) pow((double) (max_aa + 1), (double) (i - 1));
	limit = (sint) pow((double) (max_aa + 1), (double) ktup);
    }

    tl1 = (l1 + l2) - 1;

    for (i = 1; i <= tl1; ++i) {
	slopes[i] = displ[i] = 0;
	diag_index[i] = i;
    }


/* increment diagonal score for each k_tuple match */

    for (i = 1; i <= limit; ++i) {
	vn1 = zzc[i];
	while (TRUE) {
	    if (!vn1)
		break;
	    vn2 = zzd[i];
	    while (vn2 != 0) {
		osptr = vn1 - vn2 + l2;
		++displ[osptr];
		vn2 = zzb[vn2];
	    }
	    vn1 = zza[vn1];
	}
    }

/* choose the top SIGNIF diagonals */

    des_quick_sort(displ, diag_index, tl1);

    j = tl1 - signif + 1;
    if (j < 1)
	j = 1;

/* flag all diagonals within WINDOW of a top diagonal */

    for (i = tl1; i >= j; i--)
	if (displ[i] > 0) {
	    pos = diag_index[i];
	    l = (1 > pos - window) ? 1 : pos - window;
	    m = (tl1 < pos + window) ? tl1 : pos + window;
	    for (; l <= m; l++)
		slopes[l] = 1;
	}

    for (i = 1; i <= tl1; i++)
	displ[i] = 0;


    curr_frag = maxsf = 0;

    for (i = 1; i <= (l1 - ktup + 1); ++i) {
	encrypt = flag = 0;
	for (j = 1; j <= ktup; ++j) {
	    residue = seq_array[seq_no][i + j - 1];
	    if ((residue < 0) || (residue > max_aa)) {
		flag = TRUE;
		break;
	    }
	    encrypt += ((residue) * pot[j]);
	}
	if (flag)
	    continue;
	++encrypt;

	vn2 = zzd[encrypt];

	flag = FALSE;
	while (TRUE) {
	    if (!vn2) {
		flag = TRUE;
		break;
	    }
	    osptr = i - vn2 + l2;
	    if (slopes[osptr] != 1) {
		vn2 = zzb[vn2];
		continue;
	    }
	    flen = 0;
	    fs = ktup;
	    next = maxsf;

	    /*
	       * A-loop
	     */

	    while (TRUE) {
		if (!next) {
		    ++curr_frag;
		    if (curr_frag >= 2 * max_aln_length) {
			info("(Partial alignment)");
			vatend = 1;
			return;
		    }
		    displ[osptr] = curr_frag;
		    put_frag(fs, i, vn2, flen);
		} else {
		    tv1 = accum[1][next];
		    tv2 = accum[2][next];
		    if (frag_rel_pos(i, vn2, tv1, tv2)) {
			if (i - vn2 == accum[1][next] - accum[2][next]) {
			    if (i > accum[1][next] + (ktup - 1))
				fs = accum[0][next] + ktup;
			    else {
				rmndr = i - accum[1][next];
				fs = accum[0][next] + rmndr;
			    }
			    flen = next;
			    next = 0;
			    continue;
			} else {
			    if (displ[osptr] == 0)
				subt1 = ktup;
			    else {
				if (i >
				    accum[1][displ[osptr]] + (ktup - 1))
				    subt1 = accum[0][displ[osptr]] + ktup;
				else {
				    rmndr = i - accum[1][displ[osptr]];
				    subt1 = accum[0][displ[osptr]] + rmndr;
				}
			    }
			    subt2 = accum[0][next] - wind_gap + ktup;
			    if (subt2 > subt1) {
				flen = next;
				fs = subt2;
			    } else {
				flen = displ[osptr];
				fs = subt1;
			    }
			    next = 0;
			    continue;
			}
		    } else {
			next = accum[4][next];
			continue;
		    }
		}
		break;
	    }
	    /*
	       * End of Aloop
	     */

	    vn2 = zzb[vn2];
	}
    }
    vatend = 0;
}

void show_pair(sint istart, sint iend, sint jstart, sint jend)
{
    sint i, j, dsr;
    double calc_score;

    /* MPI variables */
    int np, sbsize, sbsize2, position,tmp_score;
    char *sbuffer;
    int ii,jj;
    int work, from, myrank,seq1,seq2;
    IANDJ *pij; 
    int idx, loopidx;
    double wtime1,wtime2;
    MPI_Status status;


    /* MPI bookeeping */
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    /* determine the exact (i:j) based on istart and jstart */
    pij = (IANDJ *)malloc(((iend-istart)*(jend-jstart))*sizeof(IANDJ));
    assert(pij);

    idx=0;
    for (i=istart+1;i<=(jstart+1);i++) {
	for (j=(jstart+2);j<=jend;j++) {
	    pij[idx].i = i;
	    pij[idx].j = j;
	    idx++;
	}
    }

    for (i=jstart+2;i<=iend;i++) {
	for (j=(i+1);j<=jend;j++) {
	    pij[idx].i = j;
	    pij[idx].j = i;
	    idx++;
	}
    }

    wtime1 = MPI_Wtime();

    /* retain the old codes for the case where (np==1) ... */
    if (np > 1)
	goto doing_show_pair;

    accum = (sint **) ckalloc(5 * sizeof(sint *));
    for (i = 0; i < 5; i++)
	accum[i] =
	    (sint *) ckalloc((2 * max_aln_length + 1) * sizeof(sint));

    displ = (sint *) ckalloc((2 * max_aln_length + 1) * sizeof(sint));
    slopes = (char *) ckalloc((2 * max_aln_length + 1) * sizeof(char));
    diag_index = (sint *) ckalloc((2 * max_aln_length + 1) * sizeof(sint));

    zza = (sint *) ckalloc((max_aln_length + 1) * sizeof(sint));
    zzb = (sint *) ckalloc((max_aln_length + 1) * sizeof(sint));

    zzc = (sint *) ckalloc((max_aln_length + 1) * sizeof(sint));
    zzd = (sint *) ckalloc((max_aln_length + 1) * sizeof(sint));

    if (dnaflag) {
	ktup = dna_ktup;
	window = dna_window;
	signif = dna_signif;
	wind_gap = dna_wind_gap;
    } else {
	ktup = prot_ktup;
	window = prot_window;
	signif = prot_signif;
	wind_gap = prot_wind_gap;
    }

    fprintf(stdout, "\n\n");

    for (i = istart + 1; i <= iend; ++i) {
	if (dnaflag)
	    make_n_ptrs(zza, zzc, i, seqlen_array[i]);
	else
	    make_p_ptrs(zza, zzc, i, seqlen_array[i]);
	for (j = jstart + 2; j <= jend; ++j) {
	    if (dnaflag)
		make_n_ptrs(zzb, zzd, j, seqlen_array[j]);
	    else
		make_p_ptrs(zzb, zzd, j, seqlen_array[j]);
	    pair_align(i, seqlen_array[i], seqlen_array[j]);
	    if (!maxsf)
		calc_score = 0.0;
	    else {
		calc_score = (double) accum[0][maxsf];
		if (percent) {
		    dsr = (seqlen_array[i] < seqlen_array[j]) ?
			seqlen_array[i] : seqlen_array[j];
		    calc_score = (calc_score / (double) dsr) * 100.0;
		}
	    }
/*
			tmat[i][j]=calc_score;
			tmat[j][i]=calc_score;
*/

	    tmat[i][j] = (100.0 - calc_score) / 100.0;
	    tmat[j][i] = (100.0 - calc_score) / 100.0;
	    if (calc_score > 0.1)
		info("Sequences (%d:%d) Aligned. Score: %lg",
		     (pint) i, (pint) j, calc_score);
	    else
		info("Sequences (%d:%d) Not Aligned", (pint) i, (pint) j);
	}
    }

    for (i = 0; i < 5; i++)
	accum[i] = ckfree((void *) accum[i]);
    accum = ckfree((void *) accum);

    displ = ckfree((void *) displ);
    slopes = ckfree((void *) slopes);
    diag_index = ckfree((void *) diag_index);

    zza = ckfree((void *) zza);
    zzb = ckfree((void *) zzb);
    zzc = ckfree((void *) zzc);
    zzd = ckfree((void *) zzd);

  doing_show_pair:

    accum = (sint **) ckalloc(5 * sizeof(sint *));
    for (i = 0; i < 5; i++)
	accum[i] =
	    (sint *) ckalloc((2 * max_aln_length + 1) * sizeof(sint));


    work = 1;


    for (loopidx=0;loopidx<idx;loopidx++) {

	ii = pij[loopidx].i;
	jj = pij[loopidx].j;

	    if (work < np) {
		/* we have idle processes remaining. */

		/* determine the size of the sending buffer */
		sbsize = 0;
		sbsize += 15 * sizeof(int) + 1 * sizeof(char) +
		    (seqlen_array[ii] + 1 + seqlen_array[jj] +
		     1) * sizeof(char);
		sbsize += (nseqs)*sizeof(int);

		MPI_Send(&sbsize, 1, MPI_INT, work, DOING_SHOW_PAIR,
			 MPI_COMM_WORLD);

		sbuffer = (char *) malloc(sbsize * sizeof(char));
		assert(sbuffer);
		position = 0;

		/* pack data */
		MPI_Pack(&dnaflag, 1, MPI_CHAR, sbuffer, sbsize, &position,
			 MPI_COMM_WORLD);
		MPI_Pack(&ii, 1, MPI_INT, sbuffer, sbsize, &position,
			 MPI_COMM_WORLD);
		MPI_Pack(&jj, 1, MPI_INT, sbuffer, sbsize, &position,
			 MPI_COMM_WORLD);
		MPI_Pack(&nseqs, 1, MPI_INT, sbuffer, sbsize, &position,
			 MPI_COMM_WORLD);

		MPI_Pack(seqlen_array, (nseqs+1), MPI_INT, sbuffer, sbsize,
			 &position, MPI_COMM_WORLD);

		MPI_Pack(seq_array[ii], seqlen_array[ii] + 1, MPI_CHAR,
			 sbuffer, sbsize, &position, MPI_COMM_WORLD);
		MPI_Pack(seq_array[jj], seqlen_array[jj] + 1, MPI_CHAR,
			 sbuffer, sbsize, &position, MPI_COMM_WORLD);

		MPI_Pack(&max_aa, 1, MPI_INT, sbuffer, sbsize, &position,
			 MPI_COMM_WORLD);
		MPI_Pack(&max_aln_length, 1, MPI_INT, sbuffer, sbsize,
			 &position, MPI_COMM_WORLD);
		MPI_Pack(&dna_ktup, 1, MPI_INT, sbuffer, sbsize, &position,
			 MPI_COMM_WORLD);
		MPI_Pack(&dna_window, 1, MPI_INT, sbuffer, sbsize,
			 &position, MPI_COMM_WORLD);
		MPI_Pack(&dna_signif, 1, MPI_INT, sbuffer, sbsize,
			 &position, MPI_COMM_WORLD);
		MPI_Pack(&dna_wind_gap, 1, MPI_INT, sbuffer, sbsize,
			 &position, MPI_COMM_WORLD);
		MPI_Pack(&prot_ktup, 1, MPI_INT, sbuffer, sbsize,
			 &position, MPI_COMM_WORLD);
		MPI_Pack(&prot_window, 1, MPI_INT, sbuffer, sbsize,
			 &position, MPI_COMM_WORLD);
		MPI_Pack(&prot_signif, 1, MPI_INT, sbuffer, sbsize,
			 &position, MPI_COMM_WORLD);
		MPI_Pack(&prot_wind_gap, 1, MPI_INT, sbuffer, sbsize,
			 &position, MPI_COMM_WORLD);

		/* send data */
		MPI_Send(sbuffer, sbsize, MPI_PACKED, work, SHOW_PAIR_DATA,
			 MPI_COMM_WORLD);
		free(sbuffer);

		work++;

	    } else {

		/* we have to wait for somebody sending his result back */


		/* receive data */
		MPI_Recv(&sbsize2, 1, MPI_INT, MPI_ANY_SOURCE,
			 SHOW_PAIR_RESULT_SIZE, MPI_COMM_WORLD, &status);
		from = status.MPI_SOURCE;

		sbuffer = (char *) malloc(sbsize2 * sizeof(char));
		assert(sbuffer);

	        MPI_Recv(sbuffer, sbsize2, MPI_PACKED, from, SHOW_PAIR_RESULT,
		           MPI_COMM_WORLD, &status);

		position = 0;
		MPI_Unpack(sbuffer, sbsize2, &position, &tmp_score, 1, MPI_INT,
			   MPI_COMM_WORLD);
		MPI_Unpack(sbuffer, sbsize2, &position, &seq1, 1, MPI_INT,
			   MPI_COMM_WORLD);
		MPI_Unpack(sbuffer, sbsize2, &position, &seq2, 1, MPI_INT,
			   MPI_COMM_WORLD);

		free(sbuffer);

		/* compute calc_score */
		calc_score = (double)tmp_score;

	        if (percent) {
			dsr = (seqlen_array[seq1] < seqlen_array[seq2]) ?
			    seqlen_array[seq1] : seqlen_array[seq2];
			calc_score = (calc_score / (double) dsr) * 100.0;
	        }

		tmat[seq1][seq2] = (100.0 - calc_score) / 100.0;
		tmat[seq2][seq1] = (100.0 - calc_score) / 100.0;

		if (calc_score > 0.1)
		    info("Sequences (%d:%d) Aligned. Score: %lg\tdone by rank %d", 
			    seq1, seq2, calc_score, from);
		else
		    info("Sequences (%d:%d) Not Aligned", seq1, seq2);


		/* determine the size of the sending buffer */
		sbsize = 0;
		sbsize += 15 * sizeof(int) + 1 * sizeof(char) +
		    (seqlen_array[ii] + 1 + seqlen_array[jj] +
		     1) * sizeof(char);
		sbsize += (nseqs)*sizeof(int);

		MPI_Send(&sbsize, 1, MPI_INT, from, DOING_SHOW_PAIR,
			 MPI_COMM_WORLD);

		sbuffer = (char *) malloc(sbsize * sizeof(char));
		assert(sbuffer);
		position = 0;

		/* pack data */
		MPI_Pack(&dnaflag, 1, MPI_CHAR, sbuffer, sbsize, &position,
			 MPI_COMM_WORLD);
		MPI_Pack(&ii, 1, MPI_INT, sbuffer, sbsize, &position,
			 MPI_COMM_WORLD);
		MPI_Pack(&jj, 1, MPI_INT, sbuffer, sbsize, &position,
			 MPI_COMM_WORLD);
		MPI_Pack(&nseqs, 1, MPI_INT, sbuffer, sbsize, &position,
			 MPI_COMM_WORLD);

		MPI_Pack(seqlen_array, (nseqs+1), MPI_INT, sbuffer, sbsize,
			 &position, MPI_COMM_WORLD);

		MPI_Pack(seq_array[ii], seqlen_array[ii] + 1, MPI_CHAR,
			 sbuffer, sbsize, &position, MPI_COMM_WORLD);
		MPI_Pack(seq_array[jj], seqlen_array[jj] + 1, MPI_CHAR,
			 sbuffer, sbsize, &position, MPI_COMM_WORLD);
		MPI_Pack(&max_aa, 1, MPI_INT, sbuffer, sbsize, &position,
			 MPI_COMM_WORLD);
		MPI_Pack(&max_aln_length, 1, MPI_INT, sbuffer, sbsize,
			 &position, MPI_COMM_WORLD);
		MPI_Pack(&dna_ktup, 1, MPI_INT, sbuffer, sbsize, &position,
			 MPI_COMM_WORLD);
		MPI_Pack(&dna_window, 1, MPI_INT, sbuffer, sbsize,
			 &position, MPI_COMM_WORLD);
		MPI_Pack(&dna_signif, 1, MPI_INT, sbuffer, sbsize,
			 &position, MPI_COMM_WORLD);
		MPI_Pack(&dna_wind_gap, 1, MPI_INT, sbuffer, sbsize,
			 &position, MPI_COMM_WORLD);
		MPI_Pack(&prot_ktup, 1, MPI_INT, sbuffer, sbsize,
			 &position, MPI_COMM_WORLD);
		MPI_Pack(&prot_window, 1, MPI_INT, sbuffer, sbsize,
			 &position, MPI_COMM_WORLD);
		MPI_Pack(&prot_signif, 1, MPI_INT, sbuffer, sbsize,
			 &position, MPI_COMM_WORLD);
		MPI_Pack(&prot_wind_gap, 1, MPI_INT, sbuffer, sbsize,
			 &position, MPI_COMM_WORLD);

		/* send data */
		MPI_Send(sbuffer, sbsize, MPI_PACKED, from, SHOW_PAIR_DATA,
			 MPI_COMM_WORLD);
		free(sbuffer);

	    }
    }

    /* handle the remaining ones */
    for (i = 0; i < (work - 1); i++) {

	/* receive data */
	MPI_Recv(&sbsize2, 1, MPI_INT, MPI_ANY_SOURCE,
		 SHOW_PAIR_RESULT_SIZE, MPI_COMM_WORLD, &status);
	from = status.MPI_SOURCE;

	sbuffer = (char *) malloc(sbsize2 * sizeof(char));
	assert(sbuffer);

        MPI_Recv(sbuffer, sbsize2, MPI_PACKED, from, SHOW_PAIR_RESULT,
	           MPI_COMM_WORLD, &status);

	position = 0;
	MPI_Unpack(sbuffer, sbsize2, &position, &tmp_score, 1, MPI_INT,
		   MPI_COMM_WORLD);
	MPI_Unpack(sbuffer, sbsize2, &position, &seq1, 1, MPI_INT,
			   MPI_COMM_WORLD);
	MPI_Unpack(sbuffer, sbsize2, &position, &seq2, 1, MPI_INT,
			   MPI_COMM_WORLD);

	free(sbuffer);

	/* compute calc_score */

	calc_score = (double)tmp_score;
        if (percent) {
		dsr = (seqlen_array[seq1] < seqlen_array[seq2]) ?
		    seqlen_array[seq1] : seqlen_array[seq2];
		calc_score = (calc_score / (double) dsr) * 100.0;
        }

	tmat[seq1][seq2] = (100.0 - calc_score) / 100.0;
	tmat[seq2][seq1] = (100.0 - calc_score) / 100.0;

	if (calc_score > 0.1)
	    info("Sequences (%d:%d) Aligned. Score: %lg\tdone by rank %d", 
	    seq1, seq2, calc_score, from);
	else
	    info("Sequences (%d:%d) Not Aligned", seq1, seq2);
    }

/** Measuring wall time by MPI_Wtime() **/
     wtime2 = MPI_Wtime();

    fprintf(stderr,"\nDEBUG: quick align (show_pair) time = %5.3f sec\n", wtime2 - wtime1);
    fflush(stderr);


    for (i = 0; i < 5; i++)
	accum[i] = ckfree((void *) accum[i]);
    accum = ckfree((void *) accum);

    free(pij);

    return;
}
