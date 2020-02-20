#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include "mpi.h"
#include "clustalw.h"


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

static lint pdiff(sint A, sint B, sint i, sint j, sint go1, sint go2);
static lint prfscore(sint n, sint m);
static sint gap_penalty1(sint i, sint j, sint k);
static sint open_penalty1(sint i, sint j);
static sint ext_penalty1(sint i, sint j);
static sint gap_penalty2(sint i, sint j, sint k);
static sint open_penalty2(sint i, sint j);
static sint ext_penalty2(sint i, sint j);
static void padd(sint k);
static void pdel(sint k);
static void palign(void);
static void my_padd(int *pa, int arg, int *pidx);
static void my_pdel(int *pa, int arg, int *pidx);
static void my_palign(int *pa, int *pidx);


static void mypairwise(int si, int sj, Boolean dnaflag, 
	float pw_go_penalty, float pw_ge_penalty, int int_scale,
	float gscale,float ghscale, int mat_avscore, 
	short *xarrary, short *yarray, double *tarray);


static void mpi_njtree_slave(int, int);

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

#define gap(k)  ((k) <= 0 ? 0 : g + gh * (k))
#define tbgap(k)  ((k) <= 0 ? 0 : tb + gh * (k))
#define tegap(k)  ((k) <= 0 ? 0 : te + gh * (k))

/*
 *   Global variables
 */
#define pwint   int
static sint int_scale;

extern sint nseqs;
extern sint max_aa;
extern sint max_aln_length;
extern sint gap_pos1, gap_pos2;
extern sint debug;
extern sint *seqlen_array;
extern char **seq_array;
extern char *amino_acid_codes;

/* for show_pair() */
extern sint  dna_ktup, dna_window, dna_wind_gap, dna_signif; 
extern sint prot_ktup,prot_window,prot_wind_gap,prot_signif;
extern Boolean 	dnaflag;
static sint 	next;
static sint 	curr_frag,maxsf,vatend;
static sint 	**accum;
static sint 	*diag_index;
static char 	*slopes;
sint ktup,window,wind_gap,signif; 
sint *zza, *zzb, *zzc, *zzd;
extern Boolean percent;
static void make_p_ptrs(sint *tptr, sint *pl, sint naseq, sint l);
static void make_n_ptrs(sint *tptr, sint *pl, sint naseq, sint len);
static void put_frag(sint fs, sint v1, sint v2, sint flen);
static sint frag_rel_pos(sint a1, sint b1, sint a2, sint b2);
static void des_quick_sort(sint *array1, sint *array2, sint array_size);
static void pair_align(sint seq_no, sint l1, sint l2);
static void mpi_show_pair_slave(int length, int from);



static float mm_score;
static sint print_ptr, last_print;
static sint *displ;
static pwint *HH, *DD, *RR, *SS;
static sint g, gh;
static int seq1, seq2;
static sint matrix[NUMRES][NUMRES];
static pwint maxscore;
static sint sb1, sb2, se1, se2;

static char **alignment;
static sint **profile1, **profile2;
static lint *gS;
static sint prf_length1, prf_length2;
static Boolean endgappenalties;

static int pidx = 0;
static int *pa = NULL;		/* array holding the calling sequence of palign(),
				   padd(x) and pdel(y) */

static int *avail_p;
static int len_of_xytarray=0;

static MPI_Request send_req;
static MPI_Request send_req2;

void pforward_pass(int midi, int t, int tl, int A, int B, int N,
			  pwint * HH, pwint * DD);
void preverse_pass(int midi, int A, int B, int M, int N, int go2,
			  pwint * RR, pwint * SS, lint * gS);

void stupid(int a, int b, int c, int d, sint go1, sint go2, 
	      lint *pscore, sint *pprint_ptr, sint *plast_print, lint **pdispl,
	      int pmax_aln_length, sint pmax_aa,
	      sint pgap_pos1, sint pgap_pos2,
	      sint **pprofile1, sint **pprofile2,
	      Boolean pendgappenalties,
	      sint pprf_length1, sint pprf_length2, int preverse_rank);


void parallel_compare()
{
    char *mpi_buffer;
    int i, position, mystrlen, idx1, idx2, len1, len2;
    int istart,iend,jstart,jend;
    int si,sj,ave_load,npair;
    short *xarray,*yarray;
    double *tarray;
    int slen;
    int ntimes;
    int num_siarray;

    int mybsize;
    char *mystr;
    MPI_Status status;
    static lint score;
    int arg1, arg2, arg3, arg4, arg5, arg6, origin;
    int A, B, N, M, go2, midi;
    int my_rank, np;
    int navailp;		/* number of avaialble processes */
    int go1;
    int *group;
    int which_set;
    int preverse_rank;
    float pw_go_penalty,pw_ge_penalty,gscale,ghscale;
    int int_scale,mat_avscore;
    double wtime1, wtime2;
    long long totallength, totalsquare, total_2_term, bload;
    long long length1, length2;
    long long tmp1;
    int len_siarray, which_interval;
    int *siarray,*sjarray;


  mainloop:

    MPI_Recv(&mybsize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
	     MPI_COMM_WORLD, &status);

    switch (status.MPI_TAG) {
    case PAIRWISE_TAG:
             printf("hengbiao1\n");
	goto pairwise_task;
	break;
    case MALIGN_TAG:
    printf("hengbiao2\n");
	goto malign_task;
	break;

    case CALLING_STUPID:
    printf("hengbiao3\n");
	goto calling_stupid;
	break;

    case DOING_NJTREE:
    printf("hengbiao4\n");
	goto doing_njtree;
	break;

    case DOING_SHOW_PAIR:
    printf("hengbiao5\n");
	goto doing_show_pair;
	break;

    case PREVERSE_TAG:
    printf("hengbiao6\n");
	goto preverse_task;
	break;

    case MY_ENDING_TAG:
    printf("hengbiao7\n");
	MPI_Finalize();
	return;
	break;
    default:
	fprintf(stderr, "Wrong task submitting.\n");
	exit(1);
	break;
    }



  pairwise_task:


    /* receive from the process whose rank is equal to '0' */

    mpi_buffer = (char *) malloc(mybsize * sizeof(char));
    assert(mpi_buffer);

    MPI_Recv(mpi_buffer, mybsize, MPI_PACKED, 0, MY_DATA_TAG,
	     MPI_COMM_WORLD, &status);

    position = 0;

    MPI_Unpack(mpi_buffer, mybsize, &position, &nseqs, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &istart, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &iend, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &jstart, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &jend, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &dnaflag, 1, MPI_CHAR, MPI_COMM_WORLD);

    MPI_Unpack(mpi_buffer, mybsize, &position, &pw_go_penalty, 1, MPI_FLOAT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &pw_ge_penalty, 1, MPI_FLOAT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &int_scale, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &gscale, 1, MPI_FLOAT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &ghscale, 1, MPI_FLOAT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &mat_avscore, 1, MPI_INT, MPI_COMM_WORLD);

    seqlen_array = (sint *) malloc((nseqs + 1) * sizeof(sint));
    assert(seqlen_array);
    MPI_Unpack(mpi_buffer, mybsize, &position, seqlen_array,
	       (nseqs + 1), MPI_INT, MPI_COMM_WORLD);

    seq_array = (char **) malloc((nseqs + 1) * sizeof(char *));
    assert(seq_array);
    for (i = 1; i <= nseqs; i++) {
	*(seq_array + i) = malloc((seqlen_array[i] + 1) * sizeof(char));
	assert(*(seq_array + i));
        MPI_Unpack(mpi_buffer, mybsize, &position, seq_array[i],
	       (seqlen_array[i] + 1), MPI_CHAR, MPI_COMM_WORLD);
    }


    amino_acid_codes = (char *) malloc(26 * sizeof(char));
    assert(amino_acid_codes);
    MPI_Unpack(mpi_buffer, mybsize, &position, amino_acid_codes, 26,
	       MPI_CHAR, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &max_aa, 1, MPI_INT,
	       MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &gap_pos1, 1, MPI_INT,
	       MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &gap_pos2, 1, MPI_INT,
	       MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &max_aln_length, 1,
	       MPI_INT, MPI_COMM_WORLD);

    for (i = 0; i < NUMRES; i++)
	MPI_Unpack(mpi_buffer, mybsize, &position, *(matrix + i),
		   NUMRES, MPI_INT, MPI_COMM_WORLD);

    free(mpi_buffer);


	/************** do computation here *******************/

    displ = (sint *) ckalloc((2 * max_aln_length + 1) * sizeof(sint));
    HH = (pwint *) ckalloc((max_aln_length) * sizeof(pwint));
    DD = (pwint *) ckalloc((max_aln_length) * sizeof(pwint));
    RR = (pwint *) ckalloc((max_aln_length) * sizeof(pwint));
    SS = (pwint *) ckalloc((max_aln_length) * sizeof(pwint));



#ifdef STATIC_SCHEDULING_PAIRALIGN

    /* the default is to use DYNAMIC_SCHEDULING_PARIALIGN */

    len_siarray = nseqs*(nseqs-1)/2+1;

    siarray = (int *)calloc(len_siarray, sizeof(int));
    assert(siarray);
    sjarray = (int *)calloc(len_siarray, sizeof(int));
    assert(sjarray);


    num_siarray=0;
    for (si = MAX(0, istart); si < nseqs && si < iend; si++) {
       for (sj = MAX(si + 1, jstart + 1); sj < nseqs && sj < jend; sj++) {
	   siarray[num_siarray]=si;
	   sjarray[num_siarray]=sj;
	   num_siarray++;
       }
    }

    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);


    ave_load = (num_siarray/(np-1))/PAIRALIGN_NCHUNK;
    if (ave_load==0)
	    ave_load = 1;


    xarray = (short *)calloc(ave_load,sizeof(short));
    assert(xarray);
    yarray = (short *)calloc(ave_load,sizeof(short));
    assert(yarray);
    tarray = (double *)calloc(ave_load,sizeof(double));
    assert(tarray);

    ntimes = 0;
    while (1) {
   	MPI_Recv(&which_interval, 1, MPI_INT, 0, WHICH_INTERVAL,
	     MPI_COMM_WORLD, &status);
              printf("hengbiaoAA%d\n",which_interval );
	if (which_interval<0) { 

	    /* we have finished our task */
	    break;

	} else {

	    /* do real work here */
	    for (i=which_interval*ave_load ; i<(which_interval+1)*ave_load && (i<num_siarray);i++){

		 ntimes++;
		 mypairwise(siarray[i],sjarray[i],dnaflag,pw_go_penalty,pw_ge_penalty,int_scale,
			 gscale,ghscale,mat_avscore,xarray,yarray,tarray);
	    }


            mybsize = (ave_load)*(sizeof(short)*2+sizeof(double)) + sizeof(int);
            MPI_Send(&mybsize, 1, MPI_INT, 0, MY_BSIZE_TAG, MPI_COMM_WORLD);
            mpi_buffer = (char *) malloc(mybsize * sizeof(char));
            assert(mpi_buffer);
        
            position = 0;
        
            slen = ave_load;
            MPI_Pack(&slen, 1, MPI_INT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
            MPI_Pack(xarray, ave_load, MPI_SHORT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
            MPI_Pack(yarray, ave_load, MPI_SHORT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
            MPI_Pack(tarray, ave_load, MPI_DOUBLE, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
            MPI_Send(mpi_buffer, mybsize, MPI_PACKED, 0, MY_RESULT_TAG, MPI_COMM_WORLD);
            free(mpi_buffer);

	    memset(xarray, 0, ave_load*sizeof(short));
	    memset(yarray, 0, ave_load*sizeof(short));
	    memset(tarray, 0, ave_load*sizeof(double));
	    len_of_xytarray=0;

	}
    }

    free(siarray);
    free(sjarray);

#else

    /* we will be using STATIC_SCHEDULING_PAIRALIGN */


    npair=0;
    for (si = MAX(0, istart); si < nseqs && si < iend; si++) {
       for (sj = MAX(si + 1, jstart + 1); sj < nseqs && sj < jend; sj++) {
	   npair++;
       }
    }

    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    ave_load = npair;

    xarray = (short *)calloc(ave_load,sizeof(short));
    assert(xarray);
    yarray = (short *)calloc(ave_load,sizeof(short));
    assert(yarray);
    tarray = (double *)calloc(ave_load,sizeof(double));
    assert(tarray);

    totallength = 0LL;
    totalsquare = 0LL;
    for (i=1;i<=nseqs;i++) {
	totallength += (long long)seqlen_array[i];
	totalsquare+= (long long)seqlen_array[i]*(long long)seqlen_array[i];
    }
    total_2_term = (totallength*totallength - totalsquare)/2LL;
    bload = total_2_term/(long long)(np - 1);

    length1 = 0LL;
    for (si = MAX(0, istart); si < nseqs && si < iend; si++) {
       for (sj = MAX(si + 1, jstart + 1); sj < nseqs && sj < jend; sj++) {

	   length1 += ((long long)(seqlen_array[si+1])*((long long)seqlen_array[sj+1]));

	   if (np - my_rank != 1) {

	     if ((length1>=((long long)my_rank-1LL)*bload) && 
		     (length1<((long long)my_rank*bload))) {
	       /* do pairwise alignment (si:sj) */

		 mypairwise(si,sj,dnaflag,pw_go_penalty,pw_ge_penalty,int_scale,
			 gscale,ghscale,mat_avscore,xarray,yarray,tarray);

	     }
	   } else {

	       /* this is the last MPI process */
	     if (length1>=((long long)my_rank-1LL)*bload) {

	       /* do pairwise alignment (si:sj) */
		 mypairwise(si,sj,dnaflag,pw_go_penalty,pw_ge_penalty,int_scale,
			 gscale,ghscale,mat_avscore,xarray,yarray,tarray);

	     }
	   }
       }
    }


    mybsize = (ave_load)*(sizeof(short)*2+sizeof(double)) + sizeof(int);
    //MPI_Send(&mybsize, 1, MPI_INT, 0, MY_BSIZE_TAG, MPI_COMM_WORLD);
    MPI_Ssend(&mybsize, 1, MPI_INT, 0, MY_BSIZE_TAG, MPI_COMM_WORLD);//YHB_version
    mpi_buffer = (char *) malloc(mybsize * sizeof(char));
    assert(mpi_buffer);

    position = 0;

    slen = ave_load;

    MPI_Pack(&slen, 1, MPI_INT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
    MPI_Pack(xarray, ave_load, MPI_SHORT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
    MPI_Pack(yarray, ave_load, MPI_SHORT, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
    MPI_Pack(tarray, ave_load, MPI_DOUBLE, mpi_buffer, mybsize, &position, MPI_COMM_WORLD);
    //MPI_Send(mpi_buffer, mybsize, MPI_PACKED, 0, MY_RESULT_TAG, MPI_COMM_WORLD);
    MPI_Ssend(mpi_buffer, mybsize, MPI_PACKED, 0, MY_RESULT_TAG, MPI_COMM_WORLD);
    free(mpi_buffer);

#endif

    /**************** house cleaning **************/
    free(seqlen_array);
    for (i = 1; i <= nseqs; i++)
	free(*(seq_array + i));
    free(seq_array);
    free(amino_acid_codes);

    free(xarray);
    free(yarray);
    free(tarray);

    displ = ckfree((void *) displ);
    HH = ckfree((void *) HH);
    DD = ckfree((void *) DD);
    RR = ckfree((void *) RR);
    SS = ckfree((void *) SS);


    goto mainloop;

    /*************************************************************
     * Start of parallel job No.2 .....
     ************************************************************/

calling_stupid:


    /* 
     * Doing MPI_Recv stuff here, pass the received data
     * to stupid() as regular function arguments.
     */


    /* unpack data */
    mpi_buffer = (char *) malloc(mybsize * sizeof(char));
    assert(mpi_buffer);


    MPI_Recv(mpi_buffer, mybsize, MPI_PACKED, 0, MY_DATA_TAG,
	     MPI_COMM_WORLD, &status);

    position = 0;

    MPI_Unpack(mpi_buffer, mybsize, &position, &nseqs, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &which_set, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &sb1, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &sb2, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &se1, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &se2, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &go1, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &go2, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &last_print, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &print_ptr, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &max_aln_length, 
	    1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &max_aa, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &gap_pos1, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &gap_pos2, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &endgappenalties, 
	    1, MPI_CHAR, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &prf_length1, 
	    1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &prf_length2, 
	    1, MPI_INT, MPI_COMM_WORLD);

    MPI_Unpack(mpi_buffer, mybsize, &position, &preverse_rank, 1, MPI_INT, MPI_COMM_WORLD);

    profile1 = (sint **) ckalloc((prf_length1 + 2) * sizeof(sint *));
    for (i = 0; i < prf_length1 + 2; i++)
	profile1[i] = (sint *) ckalloc((LENCOL + 2) * sizeof(sint));
    profile2 = (sint **) ckalloc((prf_length2 + 2) * sizeof(sint *));
    for (i = 0; i < prf_length2 + 2; i++)
	profile2[i] = (sint *) ckalloc((LENCOL + 2) * sizeof(sint));

    for (i = 0; i < prf_length1 + 2; i++)
         MPI_Unpack(mpi_buffer, mybsize, &position, profile1[i], 
	    (LENCOL+2), MPI_INT, MPI_COMM_WORLD);
    for (i = 0; i < prf_length2 + 2; i++)
         MPI_Unpack(mpi_buffer, mybsize, &position, profile2[i], 
	    (LENCOL+2), MPI_INT, MPI_COMM_WORLD);


     group = (int *) ckalloc((nseqs+1)*sizeof(int));

     MPI_Unpack(mpi_buffer, mybsize, &position, group, 
	    (nseqs+1), MPI_INT, MPI_COMM_WORLD);

    free(mpi_buffer);


    /*
     * Main work is here ..............
     */
    stupid(sb1, sb2, se1 - sb1, se2 - sb2, 
	    go1, go2, 
	    &score, &print_ptr, &last_print, &displ, 
	    max_aln_length,max_aa, gap_pos1,gap_pos2, 
	    profile1, profile2,endgappenalties, 
	    prf_length1,prf_length2, preverse_rank); 


    for (i = 0; i < prf_length1 + 2; i++)
	   profile1[i] = ckfree((void *) profile1[i]);
    profile1 = ckfree((void *) profile1);

    for (i = 0; i < prf_length2 + 2; i++)
           profile2[i] = ckfree((void *) profile2[i]);
    profile2 = ckfree((void *) profile2);


    /* 
     * Doing MPI_Send stuff here. 
     * Data comes from stupid() as regular function arguments.
     */

    mybsize = 0;
    mybsize += 8*sizeof(int) + (max_aln_length + 1)*sizeof(int);
    mybsize += (nseqs+1)*sizeof(int);


    MPI_Send(&mybsize, 1, MPI_INT, 0, MY_BSIZE_TAG, MPI_COMM_WORLD);
    mpi_buffer = (char *) malloc(mybsize * sizeof(char));
    assert(mpi_buffer);


    position = 0;

    MPI_Pack(&score, 1, MPI_INT, mpi_buffer,
	     mybsize, &position, MPI_COMM_WORLD);
    MPI_Pack(&print_ptr, 1, MPI_INT, mpi_buffer,
	     mybsize, &position, MPI_COMM_WORLD);
    MPI_Pack(&last_print, 1, MPI_INT, mpi_buffer,
	     mybsize, &position, MPI_COMM_WORLD);
    MPI_Pack(&prf_length1, 1, MPI_INT, mpi_buffer,
	     mybsize, &position, MPI_COMM_WORLD);
    MPI_Pack(&prf_length2, 1, MPI_INT, mpi_buffer,
	     mybsize, &position, MPI_COMM_WORLD);
    MPI_Pack(&max_aln_length, 1, MPI_INT, mpi_buffer,
	     mybsize, &position, MPI_COMM_WORLD);
    MPI_Pack(&which_set, 1, MPI_INT, mpi_buffer,
	     mybsize, &position, MPI_COMM_WORLD);
    MPI_Pack(&preverse_rank, 1, MPI_INT, mpi_buffer,
	     mybsize, &position, MPI_COMM_WORLD);



    MPI_Pack(displ, (max_aln_length+1), MPI_INT, mpi_buffer,
	     mybsize, &position, MPI_COMM_WORLD);

    MPI_Pack(group, (nseqs+1), MPI_INT, mpi_buffer,
	     mybsize, &position, MPI_COMM_WORLD);



    MPI_Send(mpi_buffer, mybsize, MPI_PACKED, 0, MY_RESULT_TAG,
	     MPI_COMM_WORLD);


    free(mpi_buffer);

    free(group);
    free(displ);


    goto mainloop;

  doing_njtree:

    mpi_njtree_slave(mybsize, status.MPI_SOURCE);
    goto mainloop;

doing_show_pair:


    mpi_show_pair_slave(mybsize, status.MPI_SOURCE);
    goto mainloop;

  malign_task:

    /* receive from the upstream process */

    MPI_Comm_size(MPI_COMM_WORLD, &np);


    /*
       MPI_Recv(&mybsize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, 
       MPI_COMM_WORLD, &status);
     */

    /* check the tag */
    /*
       if (status.MPI_TAG == MY_ENDING_TAG) {
       MPI_Finalize();
       return;
       }
     */


    /* unpack data */

    mpi_buffer = (char *) malloc(mybsize * sizeof(char));
    assert(mpi_buffer);

    origin = status.MPI_SOURCE;

    MPI_Recv(mpi_buffer, mybsize, MPI_PACKED, origin, MY_DATA_TAG,
	     MPI_COMM_WORLD, &status);



    position = 0;

    MPI_Unpack(mpi_buffer, mybsize, &position,
	       &prf_length1, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position,
	       &prf_length2, 1, MPI_INT, MPI_COMM_WORLD);

    MPI_Unpack(mpi_buffer, mybsize, &position, &gap_pos1, 1, MPI_INT,
	       MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &gap_pos2, 1, MPI_INT,
	       MPI_COMM_WORLD);

    MPI_Unpack(mpi_buffer, mybsize, &position, &arg1, 1, MPI_INT,
	       MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &arg2, 1, MPI_INT,
	       MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &arg3, 1, MPI_INT,
	       MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &arg4, 1, MPI_INT,
	       MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &arg5, 1, MPI_INT,
	       MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position, &arg6, 1, MPI_INT,
	       MPI_COMM_WORLD);

    MPI_Unpack(mpi_buffer, mybsize, &position,
	       &endgappenalties, 1, MPI_CHAR, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position,
	       &max_aa, 1, MPI_INT, MPI_COMM_WORLD);

    max_aln_length = prf_length1 + prf_length2 + 2;


    HH = (lint *) ckalloc((max_aln_length + 1) * sizeof(lint));
    DD = (lint *) ckalloc((max_aln_length + 1) * sizeof(lint));
    RR = (lint *) ckalloc((max_aln_length + 1) * sizeof(lint));
    SS = (lint *) ckalloc((max_aln_length + 1) * sizeof(lint));
    gS = (lint *) ckalloc((max_aln_length + 1) * sizeof(lint));

    profile1 = (sint **) ckalloc((prf_length1 + 2) * sizeof(sint *));
    for (i = 0; i < prf_length1 + 2; i++)
	profile1[i] = (sint *) ckalloc((LENCOL + 2) * sizeof(sint));

    profile2 = (sint **) ckalloc((prf_length2 + 2) * sizeof(sint *));
    for (i = 0; i < prf_length2 + 2; i++)
	profile2[i] = (sint *) ckalloc((LENCOL + 2) * sizeof(sint));


    for (i = arg1; i < arg1 + arg3 + 2; i++)
	MPI_Unpack(mpi_buffer, mybsize, &position,
		   profile1[i], (LENCOL + 2), MPI_INT, MPI_COMM_WORLD);

    for (i = arg2; i < arg2 + arg4 + 2; i++)
	MPI_Unpack(mpi_buffer, mybsize, &position,
		   profile2[i], (LENCOL + 2), MPI_INT, MPI_COMM_WORLD);

    MPI_Unpack(mpi_buffer, mybsize, &position, &navailp, 1, MPI_INT,
	       MPI_COMM_WORLD);

    avail_p = (int *) malloc((navailp + 1) * sizeof(int));
    assert(avail_p);

    MPI_Unpack(mpi_buffer, mybsize, &position, avail_p, (navailp + 1),
	       MPI_INT, MPI_COMM_WORLD);

    free(mpi_buffer);

    /**************************************************************
     * Allocate pa[] only if the subsequent pdiff() calls will be
     * executed on the SAME MPI process.
     *
     * We assume the argument to padd() and pdel() are always
     * positive integer!!!
     * pa[i] == 1: it means a call to palign() has been made
     * pa[i] == 2: it means a call to padd(x) has been made,
     *             where x == pa[i+1]
     * pa[i] == 3: it means a call to pdel(x) has been made,
     *             where x == pa[i+1]
     * pa[i] == 0: the end of this pa[] array.
     ***************************************************************/

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);


    /* DEBUG: temporarily set the length to three times of max_aln_length */
    pa = (int *) calloc(3 * max_aln_length, sizeof(int));
    assert(pa);


    /*
     * Initialize pa[] by setting pidx to 0. This has to be
     * done for every instance of parallel_compare().
     */

    pidx = 0;

    score = pdiff(arg1, arg2, arg3, arg4, arg5, arg6);


    /* sending result back to "origin" */
    mybsize = 0;


    mybsize += 1 * sizeof(int);

    mybsize += 3 * max_aln_length * sizeof(int);	/* for pa[] */

    MPI_Send(&mybsize, 1, MPI_INT, origin, MY_RESULT_TAG, MPI_COMM_WORLD);

    mpi_buffer = (char *) malloc(mybsize * sizeof(char));
    assert(mpi_buffer);


    position = 0;


    MPI_Pack(pa, 3 * max_aln_length, MPI_INT, mpi_buffer,
	     mybsize, &position, MPI_COMM_WORLD);


    MPI_Pack(&score, 1, MPI_INT, mpi_buffer, mybsize,
	     &position, MPI_COMM_WORLD);


    MPI_Send(mpi_buffer, mybsize, MPI_PACKED, origin, MY_RESULT_TAG,
	     MPI_COMM_WORLD);

    free(mpi_buffer);


    HH = ckfree((void *) HH);
    DD = ckfree((void *) DD);
    RR = ckfree((void *) RR);
    SS = ckfree((void *) SS);
    gS = ckfree((void *) gS);

    for (i = 0; i < prf_length1 + 2; i++)
	profile1[i] = ckfree((void *) profile1[i]);

    for (i = 0; i < prf_length2 + 2; i++)
	profile2[i] = ckfree((void *) profile2[i]);

    profile1 = ckfree((void *) profile1);
    profile2 = ckfree((void *) profile2);

    free(avail_p);
    free(pa);

    goto mainloop;

  preverse_task:

    /* unpack data */

    mpi_buffer = (char *) malloc(mybsize * sizeof(char));
    assert(mpi_buffer);

    origin = status.MPI_SOURCE;

    MPI_Recv(mpi_buffer, mybsize, MPI_PACKED, origin, MY_DATA_TAG,
	     MPI_COMM_WORLD, &status);

    position = 0;

    MPI_Unpack(mpi_buffer, mybsize, &position,
	       &prf_length1, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position,
	       &prf_length2, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position,
	       &endgappenalties, 1, MPI_CHAR, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position,
	       &max_aa, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position,
	       &midi, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position,
	       &go2, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position,
	       &A, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position,
	       &B, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position,
	       &M, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(mpi_buffer, mybsize, &position,
	       &N, 1, MPI_INT, MPI_COMM_WORLD);

    max_aln_length = prf_length1 + prf_length2 + 2;

    RR = (lint *) ckalloc((max_aln_length + 1) * sizeof(lint));
    SS = (lint *) ckalloc((max_aln_length + 1) * sizeof(lint));
    gS = (lint *) ckalloc((max_aln_length + 1) * sizeof(lint));

    profile1 = (sint **) ckalloc((prf_length1 + 2) * sizeof(sint *));
    for (i = 0; i < prf_length1 + 2; i++)
	profile1[i] = (sint *) ckalloc((LENCOL + 2) * sizeof(sint));

    profile2 = (sint **) ckalloc((prf_length2 + 2) * sizeof(sint *));
    for (i = 0; i < prf_length2 + 2; i++)
	profile2[i] = (sint *) ckalloc((LENCOL + 2) * sizeof(sint));


    for (i = A; i < A + M + 2; i++)
	MPI_Unpack(mpi_buffer, mybsize, &position,
		   profile1[i], (LENCOL + 2), MPI_INT, MPI_COMM_WORLD);

    for (i = B; i < B + N + 2; i++)
	MPI_Unpack(mpi_buffer, mybsize, &position,
		   profile2[i], (LENCOL + 2), MPI_INT, MPI_COMM_WORLD);

    free(mpi_buffer);

    preverse_pass(midi, A, B, M, N, go2, RR, SS, gS);

    /* sending back RR[], SS[] and gS[] */

    mybsize = 0;
    mybsize += 3 * (max_aln_length + 1) * sizeof(int);

    MPI_Send(&mybsize, 1, MPI_INT, origin, MY_RESULT_TAG, MPI_COMM_WORLD);

    mpi_buffer = (char *) malloc(mybsize * sizeof(char));
    assert(mpi_buffer);

    position = 0;

    MPI_Pack(RR, (max_aln_length + 1), MPI_INT, mpi_buffer,
	     mybsize, &position, MPI_COMM_WORLD);
    MPI_Pack(SS, (max_aln_length + 1), MPI_INT, mpi_buffer,
	     mybsize, &position, MPI_COMM_WORLD);
    MPI_Pack(gS, (max_aln_length + 1), MPI_INT, mpi_buffer,
	     mybsize, &position, MPI_COMM_WORLD);

    MPI_Send(mpi_buffer, mybsize, MPI_PACKED, origin, MY_RESULT_TAG,
	     MPI_COMM_WORLD);

    free(mpi_buffer);


    RR = ckfree((void *) RR);
    SS = ckfree((void *) SS);
    gS = ckfree((void *) gS);

    for (i = 0; i < prf_length1 + 2; i++)
	profile1[i] = ckfree((void *) profile1[i]);

    for (i = 0; i < prf_length2 + 2; i++)
	profile2[i] = ckfree((void *) profile2[i]);

    profile1 = ckfree((void *) profile1);
    profile2 = ckfree((void *) profile2);

    goto mainloop;

    return;
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

    return;

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
    return;

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
    char s1[100], s2[100];

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



static void del(sint k)
{
    if (last_print < 0)
	last_print = displ[print_ptr - 1] -= k;
    else
	last_print = displ[print_ptr++] = -(k);
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

/*
 * pidx: index to pa[]
 */
static void my_palign(int *pa, int *pidx)
{
    pa[(*pidx)++] = 1;

    return;
}

/*
 * pidx: index to pa[]
 */
static void my_padd(int *pa, int arg, int *pidx)
{
    pa[(*pidx)++] = 2;
    pa[(*pidx)++] = arg;

    return;
}

/*
 * pidx: index to pa[]
 */
static void my_pdel(int *pa, int arg, int *pidx)
{
    pa[(*pidx)++] = 3;
    pa[(*pidx)++] = arg;

    return;
}


lint pdiff(sint A, sint B, sint M, sint N, sint go1, sint go2)
{
    sint midi, midj, type;
    lint midh;
    static lint t, tl, g, h;
    int my_rank, np;
    int dest1, dest2, lastp;
    int mybsize, position;
    MPI_Status status;
    char *mpi_buffer;


    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    {
	static sint i, j;
	static lint hh, f, e, s;

	/* Boundary cases: M <= 1 or N == 0 */
	if (debug > 2)
	    fprintf(stdout, "A %d B %d M %d N %d midi %d go1 %d go2 %d\n",
		    (pint) A, (pint) B, (pint) M, (pint) N, (pint) M / 2,
		    (pint) go1, (pint) go2);

	/* if sequence B is empty */

	if (N <= 0) {

	    /* if sequence A is not empty */

	    if (M > 0) {

		/* delete residues A[1] to A[M] */

		/* pdel(M); */
		my_pdel(pa, M, &pidx);

	    }
	    return (-gap_penalty1(A, B, M));
	}

	/* if sequence A is empty */

	if (M <= 1) {
	    if (M <= 0) {

		/* insert residues B[1] to B[N] */

		/* padd(N); */
		my_padd(pa, N, &pidx);

		return (-gap_penalty2(A, B, N));
	    }

	    /* if sequence A has just one residue */

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
		/* padd(N); */
		my_padd(pa, N, &pidx);

		/* pdel(1); */
		my_pdel(pa, 1, &pidx);

	    } else {

		if (midj > 1) {
		    /* padd(midj - 1); */
		    my_padd(pa, midj - 1, &pidx);
		}

		/* palign() */
		my_palign(pa, &pidx);

		if (midj < N) {
		    /*  padd(N - midj); */
		    my_padd(pa, N - midj, &pidx);

		}

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


	    if (avail_p[0] > 1) {

		/*
		 * Let "dest2" to compute preverse_pass()
		 **/
		if ((avail_p[0] % 2) == 0) {
		    dest1 = avail_p[1];
		    dest2 = avail_p[1] + avail_p[0] / 2;
		} else {
		    dest1 = avail_p[1];
		    dest2 = avail_p[1] + (avail_p[0] + 1) / 2;
		}

		/* determine buffer size */

		mybsize = 0;
		mybsize += 9 * sizeof(int) + sizeof(Boolean);
		for (i = A; i < A + M + 2; i++)
		    mybsize += (LENCOL + 2) * sizeof(sint);
		for (i = B; i < B + N + 2; i++)
		    mybsize += (LENCOL + 2) * sizeof(sint);

		MPI_Send(&mybsize, 1, MPI_INT, dest2, PREVERSE_TAG,
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

		MPI_Send(mpi_buffer, mybsize, MPI_PACKED, dest2,
			 MY_DATA_TAG, MPI_COMM_WORLD);
		free(mpi_buffer);


		/* in the meantime, we will be computing pforward_pass() */
		pforward_pass(midi, t, tl, A, B, N, HH, DD);
		DD[0] = HH[0];
		/* preverse_pass(midi, A, B, M, N, go2, RR, SS, gS); */


		/* receiving RR[], SS[], gS[] */

		MPI_Recv(&mybsize, 1, MPI_INT, dest2, MY_RESULT_TAG,
			 MPI_COMM_WORLD, &status);

		mpi_buffer = (char *) malloc(mybsize * sizeof(char));
		assert(mpi_buffer);

		position = 0;

		MPI_Recv(mpi_buffer, mybsize, MPI_PACKED, dest2,
			 MY_RESULT_TAG, MPI_COMM_WORLD, &status);


		MPI_Unpack(mpi_buffer, mybsize, &position,
			   RR, (max_aln_length + 1), MPI_INT,
			   MPI_COMM_WORLD);
		MPI_Unpack(mpi_buffer, mybsize, &position, SS,
			   (max_aln_length + 1), MPI_INT, MPI_COMM_WORLD);
		MPI_Unpack(mpi_buffer, mybsize, &position, gS,
			   (max_aln_length + 1), MPI_INT, MPI_COMM_WORLD);

		free(mpi_buffer);

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
    }

/* Conquer recursively around midpoint  */

    /*
     * avail_p[0] = navailp: the number of available computing processes,
     *              excluding the process whose rank is 0.
     * avail_p[1] = 1: the MPI rank of the first available process is 1.
     * avail_p[1] = 2: the MPI rank of the second available process is 2.
     */

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /*
     * If only one available process ...
     */
    if (avail_p[0] == 1) {

	if (type == 1) {
	    pdiff(A, B, midi, midj, go1, 1);
	    pdiff(A + midi, B + midj, M - midi, N - midj, 1, go2);
	} else {
	    pdiff(A, B, midi - 1, midj, go1, 0);

	    my_pdel(pa, 2, &pidx);
	    /* pdel(2); */

	    pdiff(A + midi + 1, B + midj, M - midi - 1, N - midj, 0, go2);
	}

	return midh;
    }

    /*
     * We have more than two, including two, available processes.
     * 
     * dest1: the MPI rank of the first pdiff() task 
     * dest2: the MPI rank of the second pdiff() task 
     */

    if ((avail_p[0] % 2) == 0) {

	dest1 = avail_p[1];
	dest2 = avail_p[1] + avail_p[0] / 2;

    } else {

	dest1 = avail_p[1];
	dest2 = avail_p[1] + (avail_p[0] + 1) / 2;

    }
    lastp = avail_p[0] + avail_p[1] - 1;


    if (type == 1) {		/* Type 1 gaps  */


	int mybsize;
	char *mpi_buffer, *mpi_buffer2;
	int i, j, arg1, arg2, arg3, arg4, arg5, arg6;
	int *HH2, *RR2, *SS2, *DD2, *displ2, *gS2;
	int print_ptr2, last_print2;
	int score, position;
	int navailp;		/* number of available processes */
	int *original_pa, *patmp;
	MPI_Status status;
	MPI_Request dest2_req;

    /*****************************************************************
     	MPI_Send():
		prf_length1, prf_length2,
		profile1[][], profile2[][],
		endgappenalties, max_aa,
		arg1, arg2, arg3, arg4, arg5, arg6,
    *****************************************************************/

    /**********************************************
     * DEBUG: send to dest2
     **********************************************/

	arg1 = A + midi;
	arg2 = B + midj;
	arg3 = M - midi;
	arg4 = N - midj;
	arg5 = 1;
	arg6 = go2;

	mybsize = 0;
	mybsize += 9 * sizeof(sint) + sizeof(Boolean);
	for (i = arg1; i < arg1 + arg3 + 2; i++)
	    mybsize += (LENCOL + 2) * sizeof(sint);
	for (i = arg2; i < arg2 + arg4 + 2; i++)
	    mybsize += (LENCOL + 2) * sizeof(sint);

	mybsize += sizeof(int);

	navailp = lastp - dest2 + 1;
	mybsize += (navailp + 1) * sizeof(int);

        /* for gap_pos1 and gap_pos2 */
        mybsize += 2*sizeof(sint);

	MPI_Send(&mybsize, 1, MPI_INT, dest2, MALIGN_TAG, MPI_COMM_WORLD);

	mpi_buffer2 = (char *) malloc(mybsize * sizeof(char));
	assert(mpi_buffer2);

	position = 0;

	MPI_Pack(&prf_length1, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);
	MPI_Pack(&prf_length2, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);

        MPI_Pack(&gap_pos1, 1, MPI_INT, mpi_buffer2, mybsize, &position, MPI_COMM_WORLD);
        MPI_Pack(&gap_pos2, 1, MPI_INT, mpi_buffer2, mybsize, &position, MPI_COMM_WORLD);

	MPI_Pack(&arg1, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);
	MPI_Pack(&arg2, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);
	MPI_Pack(&arg3, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);
	MPI_Pack(&arg4, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);
	MPI_Pack(&arg5, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);
	MPI_Pack(&arg6, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);
	MPI_Pack(&endgappenalties, 1, MPI_CHAR, mpi_buffer2, mybsize,
		 &position, MPI_COMM_WORLD);
	MPI_Pack(&max_aa, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);


	for (i = arg1; i < arg1 + arg3 + 2; i++)
	    MPI_Pack(profile1[i], (LENCOL + 2), MPI_INT, mpi_buffer2,
		     mybsize, &position, MPI_COMM_WORLD);
	for (i = arg2; i < arg2 + arg4 + 2; i++)
	    MPI_Pack(profile2[i], (LENCOL + 2), MPI_INT, mpi_buffer2,
		     mybsize, &position, MPI_COMM_WORLD);

	/*
	 * We need to adjust the list of available processes.
	 */
	avail_p[0] = navailp;
	for (i = 1; i <= navailp; i++)
	    avail_p[i] = dest2 + i - 1;

	MPI_Pack(&navailp, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);
	MPI_Pack(avail_p, (navailp + 1), MPI_INT, mpi_buffer2, mybsize,
		 &position, MPI_COMM_WORLD);

#ifdef NON_BLOCKING
	MPI_Isend(mpi_buffer2, mybsize, MPI_PACKED, dest2, MY_DATA_TAG,
		  MPI_COMM_WORLD, &dest2_req);

	/* remember to put MPI_Wait() to a proper place */
	MPI_Wait(&dest2_req, &status);
	free(mpi_buffer2);

#else
	MPI_Send(mpi_buffer2, mybsize, MPI_PACKED, dest2, MY_DATA_TAG,
		 MPI_COMM_WORLD);
	free(mpi_buffer2);
#endif

    /*********************************************
     * DEBUG: recursively call pdiff() on dest1 
     *********************************************/

	arg1 = A;
	arg2 = B;
	arg3 = midi;
	arg4 = midj;
	arg5 = go1;
	arg6 = 1;

	original_pa = pa;

	/* DEBUG: temporarily set the length to three times of max_aln_length */
	pa = (int *) calloc(3 * max_aln_length, sizeof(int));
	assert(pa);

	navailp = dest2 - dest1;

	/*
	 * We need to adjust the list of available processes.
	 */
	avail_p[0] = navailp;
	for (i = 1; i <= navailp; i++)
	    avail_p[i] = dest1 + i - 1;

	score = pdiff(arg1, arg2, arg3, arg4, arg5, arg6);

    /*****************************************************************
       MPI_Recv():
	       score,
	       pa
    *****************************************************************/

    /************************************************************
     * DEBUG: merge the results of the previous pdiff() call
     ************************************************************/


	/*
	 * Integrate pa[] into the global array pa[] ...
	 */
	patmp = pa;
	pa = original_pa;

	i = 0;
	/* find the end of pa[] */
	while (pa[i])
	    i++;

	j = 0;
	while (patmp[j])
	    pa[i++] = patmp[j++];

	free(patmp);

    /*********************************************
     * DEBUG: receive from dest2
     *********************************************/

	MPI_Recv(&mybsize, 1, MPI_INT, dest2, MY_RESULT_TAG,
		 MPI_COMM_WORLD, &status);

	mpi_buffer = (char *) malloc(mybsize * sizeof(char));
	assert(mpi_buffer);

	position = 0;

	MPI_Recv(mpi_buffer, mybsize, MPI_PACKED, dest2, MY_RESULT_TAG,
		 MPI_COMM_WORLD, &status);

	patmp = (int *) calloc(3 * max_aln_length, sizeof(int));
	assert(patmp);


	MPI_Unpack(mpi_buffer, mybsize, &position,
		   patmp, 3 * max_aln_length, MPI_INT, MPI_COMM_WORLD);

	MPI_Unpack(mpi_buffer, mybsize, &position,
		   &score, 1, MPI_INT, MPI_COMM_WORLD);

	free(mpi_buffer);


	/*
	 * Integrate patmp[] into the global array pa[] ...
	 */

	i = 0;
	/* find the end of pa[] */
	while (pa[i])
	    i++;

	j = 0;
	while (patmp[j])
	    pa[i++] = patmp[j++];

	free(patmp);


    } else {			/* Type 2 gaps */


	int mybsize;
	char *mpi_buffer, *mpi_buffer2;
	int i, j, arg1, arg2, arg3, arg4, arg5, arg6;
	int *HH2, *RR2, *SS2, *DD2, *displ2, *gS2;
	int print_ptr2, last_print2;
	int score, position;
	int navailp;		/* number of available processes */
	int *original_pa, *patmp;
	MPI_Status status;
	MPI_Request dest2_req;


    /*****************************************************************
     	MPI_Send():
		prf_length1, prf_length2,
		profile1[][], profile2[][],
		endgappenalties, max_aa,
		arg1, arg2, arg3, arg4, arg5, arg6,
    *****************************************************************/

    /**********************************************
     * DEBUG: send to dest2
     **********************************************/

	arg1 = A + midi + 1;
	arg2 = B + midj;
	arg3 = M - midi - 1;
	arg4 = N - midj;
	arg5 = 0;
	arg6 = go2;


	mybsize = 0;
	mybsize += 9 * sizeof(sint) + sizeof(Boolean);
	for (i = arg1; i < arg1 + arg3 + 2; i++)
	    mybsize += (LENCOL + 2) * sizeof(sint);
	for (i = arg2; i < arg2 + arg4 + 2; i++)
	    mybsize += (LENCOL + 2) * sizeof(sint);

	mybsize += sizeof(int);

	navailp = lastp - dest2 + 1;
	mybsize += (navailp + 1) * sizeof(int);

        /* for gap_pos1 and gap_pos2 */
        mybsize += 2*sizeof(sint);

	MPI_Send(&mybsize, 1, MPI_INT, dest2, MALIGN_TAG, MPI_COMM_WORLD);

	mpi_buffer2 = (char *) malloc(mybsize * sizeof(char));
	assert(mpi_buffer2);

	position = 0;

	MPI_Pack(&prf_length1, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);
	MPI_Pack(&prf_length2, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);

        MPI_Pack(&gap_pos1, 1, MPI_INT, mpi_buffer2, mybsize, &position, MPI_COMM_WORLD);
        MPI_Pack(&gap_pos2, 1, MPI_INT, mpi_buffer2, mybsize, &position, MPI_COMM_WORLD);

	MPI_Pack(&arg1, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);
	MPI_Pack(&arg2, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);
	MPI_Pack(&arg3, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);
	MPI_Pack(&arg4, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);
	MPI_Pack(&arg5, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);
	MPI_Pack(&arg6, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);
	MPI_Pack(&endgappenalties, 1, MPI_CHAR, mpi_buffer2, mybsize,
		 &position, MPI_COMM_WORLD);
	MPI_Pack(&max_aa, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);


	for (i = arg1; i < arg1 + arg3 + 2; i++)
	    MPI_Pack(profile1[i], (LENCOL + 2), MPI_INT, mpi_buffer2,
		     mybsize, &position, MPI_COMM_WORLD);
	for (i = arg2; i < arg2 + arg4 + 2; i++)
	    MPI_Pack(profile2[i], (LENCOL + 2), MPI_INT, mpi_buffer2,
		     mybsize, &position, MPI_COMM_WORLD);

	/*
	 * We need to adjust the list of available processes.
	 */
	avail_p[0] = navailp;
	for (i = 1; i <= navailp; i++)
	    avail_p[i] = dest2 + i - 1;

	MPI_Pack(&navailp, 1, MPI_INT, mpi_buffer2, mybsize, &position,
		 MPI_COMM_WORLD);
	MPI_Pack(avail_p, (navailp + 1), MPI_INT, mpi_buffer2, mybsize,
		 &position, MPI_COMM_WORLD);

	MPI_Send(mpi_buffer2, mybsize, MPI_PACKED, dest2, MY_DATA_TAG,
		 MPI_COMM_WORLD);

	free(mpi_buffer2);

    /*********************************************
     * DEBUG: recursively call pdiff() on dest1 
     *********************************************/

	arg1 = A;
	arg2 = B;
	arg3 = midi - 1;
	arg4 = midj;
	arg5 = go1;
	arg6 = 0;

	original_pa = pa;

	/* DEBUG: temporarily set the length to three times of max_aln_length */
	pa = (int *) calloc(3 * max_aln_length, sizeof(int));
	assert(pa);


	navailp = dest2 - dest1;

	/*
	 * We need to adjust the list of available processes.
	 */
	avail_p[0] = navailp;
	for (i = 1; i <= navailp; i++)
	    avail_p[i] = dest1 + i - 1;

	score = pdiff(arg1, arg2, arg3, arg4, arg5, arg6);

    /*****************************************************************
       MPI_Recv():
	       score,
	       pa
    *****************************************************************/

    /************************************************************
     * DEBUG: merge the results of the previous pdiff() call
     ************************************************************/

	/*
	 * Integrate pa[] into the global array pa[] ...
	 */
	patmp = pa;
	pa = original_pa;

	/*
	 * Integrate patmp[] into the global array pa[] ...
	 */

	i = 0;
	/* find the end of pa[] */
	while (pa[i])
	    i++;

	j = 0;
	while (patmp[j])
	    pa[i++] = patmp[j++];

	free(patmp);


	/* We need to correct the pidx value for the subsequence my_pdel() */
	pidx = i;


    /*********************************************
     * perform the original pdel(2) in between
     * the two pdiff()'s.
     *********************************************/
	/* pdel(2); */
	my_pdel(pa, 2, &pidx);

    /*********************************************
     * DEBUG: receive from dest2
     *********************************************/

	/* MPE_Log_event(5,0,"recv"); */
	MPI_Recv(&mybsize, 1, MPI_INT, dest2, MY_RESULT_TAG,
		 MPI_COMM_WORLD, &status);
	/* MPE_Log_event(6,0,"recved"); */

	mpi_buffer = (char *) malloc(mybsize * sizeof(char));
	assert(mpi_buffer);

	position = 0;
	/* MPE_Log_event(5,0,"recv"); */
	MPI_Recv(mpi_buffer, mybsize, MPI_PACKED, dest2, MY_RESULT_TAG,
		 MPI_COMM_WORLD, &status);
	/* MPE_Log_event(6,0,"recved"); */


	patmp = (int *) calloc(3 * max_aln_length, sizeof(int));
	assert(patmp);

	/* MPE_Log_event(13,0,"unpacking"); */
	MPI_Unpack(mpi_buffer, mybsize, &position,
		   patmp, 3 * max_aln_length, MPI_INT, MPI_COMM_WORLD);

	MPI_Unpack(mpi_buffer, mybsize, &position,
		   &score, 1, MPI_INT, MPI_COMM_WORLD);
	/* MPE_Log_event(14,0,"unpacked"); */

	free(mpi_buffer);

	/*
	 * Integrate patmp[] into the global array pa[] ...
	 */

	i = 0;
	/* find the end of pa[] */
	while (pa[i])
	    i++;

	j = 0;
	while (patmp[j])
	    pa[i++] = patmp[j++];

	free(patmp);

    }

    return midh;		/* Return the score of the best alignment */
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


void pforward_pass(int midi, int t, int tl, int A, int B, int N,
			  pwint * HH, pwint * DD)
{
    int i, j;
    int s, f, g, h, hh, e;

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

    return;
}

void preverse_pass(int midi, int A, int B, int M, int N, int go2,
			  pwint * RR, pwint * SS, lint * gS)
{
    int i, j, h, t, tl, g, s, f, e, hh;

    tl = 0;
    RR[N] = 0;

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

    return;
}

/*
 * Compute a few arrays used in "nj_tree()/trees.c".
 *
 * length: the length of the incoming data (in bytes)
 * from: the MPI rank of the sending process
 */
static void mpi_njtree_slave(int length, int from)
{
    MPI_Status status;
    int myrank;
    int position;
    double stmp;
    int npidx;			/* idx for nonempty[] */
    char *pbuffer, *pblock;
    int *mpirow;
    int *nonempty;
    double **tmat;
    int tmat_len;
    double sumd2;
    int i, j, which_row;
    double *rdiq;

    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	/* We need a buffer to store the packed data */

	pbuffer = (char *) malloc(length * sizeof(char));
	assert(pbuffer);

	MPI_Recv(pbuffer, length, MPI_PACKED, from, NJTREE_DATA,
		 MPI_COMM_WORLD, &status);

	position = 0;
	MPI_Unpack(pbuffer, length, &position, &tmat_len, 1, MPI_INT,
		   MPI_COMM_WORLD);

	mpirow = (int *) malloc((tmat_len+1) * sizeof(int));
	assert(mpirow);
	MPI_Unpack(pbuffer, length, &position, mpirow, (tmat_len+1),
		   MPI_INT, MPI_COMM_WORLD);


	pblock =
	    (char *) malloc((tmat_len + 1) * (tmat_len + 1) *
			    sizeof(double));
	assert(pblock);

	tmat = (double **) malloc((tmat_len + 1) * sizeof(double *));
	assert(tmat);
	for (i = 0; i < (tmat_len + 1); i++)
	    tmat[i] = (double *) pblock + (tmat_len * i);


	/*
	 * Since not all tmat[][] rows were sending to this MPI slave,
	 * we'd better remember those rows that are not empty.
	 */

	nonempty = (int *) calloc((tmat_len+1), sizeof(int));
	assert(nonempty);


	npidx = 0;
	for (i = 1; i <=tmat_len; i++) {
	    if (mpirow[i] == myrank) {

		nonempty[npidx++] = i;
		MPI_Unpack(pbuffer, length, &position, &tmat[i][1],
			   tmat_len, MPI_DOUBLE, MPI_COMM_WORLD);
	    }
	}

	/*
	 * Allocate space for rdiq[1..tmat_len]
	 */
	rdiq = (double *) calloc((tmat_len+1), sizeof(double));
	assert(rdiq);

	    /**** Doing computation here ***********/

	sumd2 = 0.0;
	for (i = 0; i < npidx; i++) {

	    which_row = nonempty[i];
	    for (j = 1; j <= tmat_len; j++) 
	         rdiq[which_row] += tmat[which_row][j];
	    for (j = which_row+1; j <= tmat_len; j++) 
		sumd2 += tmat[which_row][j];
	}



	    /**** Sending information back ***********/

	/* Note that I intentionally use rdiq[0] to store "sumd2" */

	rdiq[0] = sumd2;

	MPI_Send(rdiq, (tmat_len+1), MPI_DOUBLE, from, TMAT_ROW_SUM, MPI_COMM_WORLD);


	free(rdiq);
	free(mpirow);
	free(pbuffer);
	free(pblock);
	free(tmat);
	free(nonempty);


    return;
}














static void mpi_show_pair_slave(int length, int from)
{
    MPI_Status status;
    int myrank;
    int position,mybsize;
    double stmp;
    char *pbuffer, *pblock;
    int i, j, which_row,seq1,seq2;
    int zero;

    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	/* We need a buffer to store the packed data */

	pbuffer = (char *) malloc(length * sizeof(char));
	assert(pbuffer);

	MPI_Recv(pbuffer, length, MPI_PACKED, from, SHOW_PAIR_DATA,
		 MPI_COMM_WORLD, &status);

	position = 0;
	MPI_Unpack(pbuffer, length, &position, &dnaflag, 1, MPI_CHAR, MPI_COMM_WORLD);
	MPI_Unpack(pbuffer, length, &position, &seq1, 1, MPI_INT, MPI_COMM_WORLD);
	MPI_Unpack(pbuffer, length, &position, &seq2, 1, MPI_INT, MPI_COMM_WORLD);
	MPI_Unpack(pbuffer, length, &position, &nseqs, 1, MPI_INT, MPI_COMM_WORLD);

        seqlen_array = (int *) malloc((nseqs + 1) * sizeof(int));
        assert(seqlen_array);

	MPI_Unpack(pbuffer, length, &position, seqlen_array, (nseqs+1), MPI_INT, MPI_COMM_WORLD);

        seq_array = (char **) malloc((nseqs + 1) * sizeof(char *));
        assert(seq_array);

	seq_array[seq1] = (char *)malloc((seqlen_array[seq1]+1) * sizeof(char));
	assert(seq_array[seq1]);
	seq_array[seq2] = (char *)malloc((seqlen_array[seq2]+1) * sizeof(char));
	assert(seq_array[seq2]);

	MPI_Unpack(pbuffer, length, &position, seq_array[seq1], (seqlen_array[seq1]+1), 
		MPI_CHAR, MPI_COMM_WORLD);
	MPI_Unpack(pbuffer, length, &position, seq_array[seq2], (seqlen_array[seq2]+1), 
		MPI_CHAR, MPI_COMM_WORLD);

	MPI_Unpack(pbuffer, length, &position, &max_aa, 1, MPI_INT, MPI_COMM_WORLD);
	MPI_Unpack(pbuffer, length, &position, &max_aln_length, 1, MPI_INT, MPI_COMM_WORLD);
	MPI_Unpack(pbuffer, length, &position, &dna_ktup, 1, MPI_INT, MPI_COMM_WORLD);
	MPI_Unpack(pbuffer, length, &position, &dna_window, 1, MPI_INT, MPI_COMM_WORLD);
	MPI_Unpack(pbuffer, length, &position, &dna_signif, 1, MPI_INT, MPI_COMM_WORLD);
	MPI_Unpack(pbuffer, length, &position, &dna_wind_gap, 1, MPI_INT, MPI_COMM_WORLD);
	MPI_Unpack(pbuffer, length, &position, &prot_ktup, 1, MPI_INT, MPI_COMM_WORLD);
	MPI_Unpack(pbuffer, length, &position, &prot_window, 1, MPI_INT, MPI_COMM_WORLD);
	MPI_Unpack(pbuffer, length, &position, &prot_signif, 1, MPI_INT, MPI_COMM_WORLD);
	MPI_Unpack(pbuffer, length, &position, &prot_wind_gap, 1, MPI_INT, MPI_COMM_WORLD);

	free(pbuffer);

	    /**** Allocating spaces ***********/
	accum = (sint **)ckalloc( 5*sizeof (sint *) );
	for (i=0;i<5;i++)
		accum[i] = (sint *) ckalloc((2*max_aln_length+1) * sizeof (sint) );

	displ      = (sint *) ckalloc( (2*max_aln_length +1) * sizeof (sint) );
	slopes     = (char *)ckalloc( (2*max_aln_length +1) * sizeof (char));
	diag_index = (sint *) ckalloc( (2*max_aln_length +1) * sizeof (sint) );
	zza = (sint *)ckalloc( (max_aln_length+1) * sizeof (sint) );
	zzb = (sint *)ckalloc( (max_aln_length+1) * sizeof (sint) );
	zzc = (sint *)ckalloc( (max_aln_length+1) * sizeof (sint) );
	zzd = (sint *)ckalloc( (max_aln_length+1) * sizeof (sint) );

	    /**** Doing computation here ***********/

    if (dnaflag) {

	ktup = dna_ktup;
	window = dna_window;
	signif = dna_signif;
	wind_gap = dna_wind_gap;
        make_n_ptrs(zza, zzc, seq1, seqlen_array[seq1]);
        make_n_ptrs(zzb, zzd, seq2, seqlen_array[seq2]);
        pair_align(seq1, seqlen_array[seq1], seqlen_array[seq2]);

    } else {

	ktup = prot_ktup;
	window = prot_window;
	signif = prot_signif;
	wind_gap = prot_wind_gap;

        make_p_ptrs(zza, zzc, seq1, seqlen_array[seq1]);
        make_p_ptrs(zzb, zzd, seq2, seqlen_array[seq2]);
        pair_align(seq1, seqlen_array[seq1], seqlen_array[seq2]);

    }

        /* Send information back:
	 * 1. accum[0][maxsf]
	 * 2. seq1, seq2
	 */
	mybsize = 3*sizeof(int);

	MPI_Send(&mybsize, 1, MPI_INT, from, SHOW_PAIR_RESULT_SIZE, 
		MPI_COMM_WORLD);

	pbuffer = (char *)malloc(mybsize*sizeof(char));
	assert(pbuffer);
	position=0;
	zero=0;

	if (maxsf)
        	MPI_Pack(&accum[0][maxsf], 1, MPI_INT, 
			pbuffer, mybsize, &position, MPI_COMM_WORLD);
	else
        	MPI_Pack(&zero, 1, MPI_INT, 
			pbuffer, mybsize, &position, MPI_COMM_WORLD);

        MPI_Pack(&seq1, 1, MPI_INT, pbuffer, mybsize, &position, MPI_COMM_WORLD);
        MPI_Pack(&seq2, 1, MPI_INT, pbuffer, mybsize, &position, MPI_COMM_WORLD);
	MPI_Send(pbuffer, mybsize, MPI_PACKED, from, SHOW_PAIR_RESULT, MPI_COMM_WORLD);
	free(pbuffer);

	/* house cleaning */
	for (i=0;i<5;i++)
	   accum[i]=ckfree((void *)accum[i]);
	accum=ckfree((void *)accum);

	displ=ckfree((void *)displ);
	slopes=ckfree((void *)slopes);

	diag_index=ckfree((void *)diag_index);
	zza=ckfree((void *)zza);
	zzb=ckfree((void *)zzb);
	zzc=ckfree((void *)zzc);
	zzd=ckfree((void *)zzd);

	if (seq1!=seq2) {
	   free(seq_array[seq1]);
	   free(seq_array[seq2]);
	} else {
	   free(seq_array[seq1]);
	}

	free(seq_array);
	free(seqlen_array);

    return;
}


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





static void make_n_ptrs(sint *tptr,sint *pl,sint naseq,sint len)
{
	static sint pot[]={ 0, 1, 4, 16, 64, 256, 1024, 4096 };
	sint i,j,limit,code,flag;
	char residue;
	
	limit = (sint) pow((double)4,(double)ktup);
	
	for(i=1;i<=limit;++i)
		pl[i]=0;
	for(i=1;i<=len;++i)
		tptr[i]=0;
	
	for(i=1;i<=len-ktup+1;++i) {
		code=0;
		flag=FALSE;
		for(j=1;j<=ktup;++j) {
			residue = seq_array[naseq][i+j-1];
			if((residue<0) || (residue>4)){
				flag=TRUE;
				break;
			}
			code += ((residue) * pot[j]);  /* DES */
		}
		if(flag)
			continue;
		++code;
		if(pl[code]!=0)
			tptr[i]=pl[code];
		pl[code]=i;
	}
}


static void put_frag(sint fs,sint v1,sint v2,sint flen)
{
	sint end;
	accum[0][curr_frag]=fs;
	accum[1][curr_frag]=v1;
	accum[2][curr_frag]=v2;
	accum[3][curr_frag]=flen;
	
	if(!maxsf) {
		maxsf=1;
		accum[4][curr_frag]=0;
		return;
	}
	
        if(fs >= accum[0][maxsf]) {
		accum[4][curr_frag]=maxsf;
		maxsf=curr_frag;
		return;
	}
	else {
		next=maxsf;
		while(TRUE) {
			end=next;
			next=accum[4][next];
			if(fs>=accum[0][next])
				break;
		}
		accum[4][curr_frag]=next;
		accum[4][end]=curr_frag;
	}
}


static sint frag_rel_pos(sint a1,sint b1,sint a2,sint b2)
{
	sint ret;
	
	ret=FALSE;
	if(a1-b1==a2-b2) {
		if(a2<a1)
			ret=TRUE;
	}
	else {
		if(a2+ktup-1<a1 && b2+ktup-1<b1)
			ret=TRUE;
	}
	return ret;
}


static void des_quick_sort(sint *array1, sint *array2, sint array_size)
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
	sint lst[50], ust[50];       /* the maximum no. of elements must be*/
								/* < log(base2) of 50 */

	lst[1] = 1;
	ust[1] = array_size-1;
	p = 1;

	while(p > 0) {
		if(lst[p] >= ust[p])
			p--;
		else {
			i = lst[p] - 1;
			j = ust[p];
			pivlin = array1[j];
			while(i < j) {
				for(i=i+1; array1[i] < pivlin; i++)
					;
				for(j=j-1; j > i; j--)
					if(array1[j] <= pivlin) break;
				if(i < j) {
					temp1     = array1[i];
					array1[i] = array1[j];
					array1[j] = temp1;
					
					temp2     = array2[i];
					array2[i] = array2[j];
					array2[j] = temp2;
				}
			}
			
			j = ust[p];

			temp1     = array1[i];
			array1[i] = array1[j];
			array1[j] = temp1;

			temp2     = array2[i];
			array2[i] = array2[j];
			array2[j] = temp2;

			if(i-lst[p] < ust[p] - i) {
				lst[p+1] = lst[p];
				ust[p+1] = i - 1;
				lst[p]   = i + 1;
			}
			else {
				lst[p+1] = i + 1;
				ust[p+1] = ust[p];
				ust[p]   = i - 1;
			}
			p = p + 1;
		}
	}
	return;

}





static void pair_align(sint seq_no,sint l1,sint l2)
{
	sint pot[8],i,j,l,m,flag,limit,pos,tl1,vn1,vn2,flen,osptr,fs;
	sint tv1,tv2,encrypt,subt1,subt2,rmndr;
	char residue;
	
	if(dnaflag) {
		for(i=1;i<=ktup;++i)
			pot[i] = (sint) pow((double)4,(double)(i-1));
		limit = (sint) pow((double)4,(double)ktup);
	}
	else {
		for (i=1;i<=ktup;i++)
           		pot[i] = (sint) pow((double)(max_aa+1),(double)(i-1));
		limit = (sint) pow((double)(max_aa+1),(double)ktup);
	}
	
	tl1 = (l1+l2)-1;
	
	for(i=1;i<=tl1;++i) {
		slopes[i]=displ[i]=0;
		diag_index[i] = i;
	}
	

/* increment diagonal score for each k_tuple match */

	for(i=1;i<=limit;++i) {
		vn1=zzc[i];
		while(TRUE) {
			if(!vn1) break;
			vn2=zzd[i];
			while(vn2 != 0) {
				osptr=vn1-vn2+l2;
				++displ[osptr];
				vn2=zzb[vn2];
			}
			vn1=zza[vn1];
		}
	}

/* choose the top SIGNIF diagonals */

	des_quick_sort(displ, diag_index, tl1);

	j = tl1 - signif + 1;
	if(j < 1) j = 1;
 
/* flag all diagonals within WINDOW of a top diagonal */

	for(i=tl1; i>=j; i--) 
		if(displ[i] > 0) {
			pos = diag_index[i];
			l = (1  >pos-window) ? 1   : pos-window;
			m = (tl1<pos+window) ? tl1 : pos+window;
			for(; l <= m; l++) 
				slopes[l] = 1;
		}

	for(i=1; i<=tl1; i++)  displ[i] = 0;

	
	curr_frag=maxsf=0;
	
	for(i=1;i<=(l1-ktup+1);++i) {
		encrypt=flag=0;
		for(j=1;j<=ktup;++j) {
			residue = seq_array[seq_no][i+j-1];
			if((residue<0) || (residue>max_aa)) {
				flag=TRUE;
				break;
			}
			encrypt += ((residue)*pot[j]);
		}
		if(flag) continue;
		++encrypt;
	
		vn2=zzd[encrypt];
	
		flag=FALSE;
		while(TRUE) {
			if(!vn2) {
				flag=TRUE;
				break;
			}
			osptr=i-vn2+l2;
			if(slopes[osptr]!=1) {
				vn2=zzb[vn2];
				continue;
			}
			flen=0;
			fs=ktup;
			next=maxsf;		
		
		/*
		* A-loop
		*/
		
			while(TRUE) {
				if(!next) {
					++curr_frag;
					if(curr_frag>=2*max_aln_length) {
						info("(Partial alignment)");
						vatend=1;
						return;
					}
					displ[osptr]=curr_frag;
					put_frag(fs,i,vn2,flen);
				}
				else {
					tv1=accum[1][next];
					tv2=accum[2][next];
					if(frag_rel_pos(i,vn2,tv1,tv2)) {
						if(i-vn2==accum[1][next]-accum[2][next]) {
							if(i>accum[1][next]+(ktup-1))
								fs=accum[0][next]+ktup;
							else {
								rmndr=i-accum[1][next];
								fs=accum[0][next]+rmndr;
							}
							flen=next;
							next=0;
							continue;
						}
						else {
							if(displ[osptr]==0)
								subt1=ktup;
							else {
								if(i>accum[1][displ[osptr]]+(ktup-1))
									subt1=accum[0][displ[osptr]]+ktup;
								else {
									rmndr=i-accum[1][displ[osptr]];
									subt1=accum[0][displ[osptr]]+rmndr;
								}
							}
							subt2=accum[0][next]-wind_gap+ktup;
							if(subt2>subt1) {
								flen=next;
								fs=subt2;
							}
							else {
								flen=displ[osptr];
								fs=subt1;
							}
							next=0;
							continue;
						}
					}
					else {
						next=accum[4][next];
						continue;
					}
				}
				break;
			}
		/*
		* End of Aloop
		*/
		
			vn2=zzb[vn2];
		}
	}
	vatend=0;
}


static void mypairwise(int si, int sj, Boolean dnaflag, 
	float pw_go_penalty, float pw_ge_penalty, int int_scale,
	float gscale,float ghscale, int mat_avscore, 
	short *xarray, short *yarray, double *tarray)
{
    int n,m,len1,len2,i,j;
    char c;
    double wtime1,wtime2;

	n = seqlen_array[si + 1];
	len1 = 0;
	for (i = 1; i <= n; i++) {
	    c = seq_array[si + 1][i];
	    if ((c != gap_pos1) && (c != gap_pos2))
		len1++;
	}

	    m = seqlen_array[sj + 1];
	    if (n == 0 || m == 0) {

		xarray[len_of_xytarray] = si+1;
		yarray[len_of_xytarray] = sj+1;
		tarray[len_of_xytarray] = 1.0;
		len_of_xytarray++;
		return;
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

	    xarray[len_of_xytarray] = si+1;
	    yarray[len_of_xytarray] = sj+1;
	    tarray[len_of_xytarray] = ((float) 100.0 - mm_score) / (float) 100.0;
	    len_of_xytarray++;


	    {
	      int rank;
	      MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	      fprintf(stdout,"Sequences (%d:%d) Aligned. Score:  %5.2f (by rank %d)\n",
		     (pint) si + 1, (pint) sj + 1, mm_score, rank);
	    }


    return;
}
