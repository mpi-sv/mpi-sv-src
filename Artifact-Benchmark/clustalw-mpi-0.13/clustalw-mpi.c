#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <signal.h>
#include <setjmp.h>
#include "clustalw.h"

#include "mpi.h"

extern void *ckalloc(size_t);
extern void init_matrix(void);
extern void init_interface(void);
extern void fill_chartab(void);

/*
*	Global variables
*/
double **tmat;

char revision_level[] = "W (1.82)";	/* JULIE  feb 2001 */
Boolean interactive = FALSE;

char *help_file_name = "clustalw_help";

sint max_names;			/* maximum length of names in current alignment file */

float gap_open, gap_extend;
float pw_go_penalty, pw_ge_penalty;

FILE *tree;
FILE *clustal_outfile, *gcg_outfile, *nbrf_outfile, *phylip_outfile,
    *gde_outfile, *nexus_outfile;
sint *seqlen_array;
sint max_aln_length;
short usermat[NUMRES][NUMRES], pw_usermat[NUMRES][NUMRES];
short def_aa_xref[NUMRES + 1], aa_xref[NUMRES + 1], pw_aa_xref[NUMRES + 1];
short userdnamat[NUMRES][NUMRES], pw_userdnamat[NUMRES][NUMRES];
short def_dna_xref[NUMRES + 1], dna_xref[NUMRES + 1],
    pw_dna_xref[NUMRES + 1];
sint nseqs;
sint nsets;
sint *output_index;
sint **sets;
sint *seq_weight;
sint max_aa;
sint gap_pos1;
sint gap_pos2;
sint mat_avscore;
sint profile_no;

Boolean usemenu;
Boolean dnaflag;
Boolean distance_tree;

char **seq_array;
char **names, **titles;
char **args;
char seqname[FILENAMELEN + 1];

char *gap_penalty_mask1 = NULL, *gap_penalty_mask2 = NULL;
char *sec_struct_mask1 = NULL, *sec_struct_mask2 = NULL;
sint struct_penalties;
char *ss_name1 = NULL, *ss_name2 = NULL;

Boolean user_series = FALSE;
UserMatSeries matseries;
short usermatseries[MAXMAT][NUMRES][NUMRES];
short aa_xrefseries[MAXMAT][NUMRES + 1];




int main(int argc, char **argv)
{
    int i;
    int my_rank, np;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    if (my_rank == 0) {


	/* init_amenu(); */
	init_interface();
	init_matrix();
	fill_chartab();

	args = (char **) ckalloc(argc * sizeof(char *));

	for (i = 1; i < argc; ++i) {
	    args[i - 1] =
		(char *) ckalloc((strlen(argv[i]) + 1) * sizeof(char));
	    strcpy(args[i - 1], argv[i]);
	}
	usemenu = FALSE;
	parse_params(FALSE);

    } else {

	/* the parallel pairwise alignment */
	parallel_compare();

    }

    return 0;

}

/*
*	fatal()
*
*	Prints error msg to stdout and exits.
*	Variadic parameter list can be passed.
*
*	Return values:
*		none
*/

void fatal(char *msg, ...)
{
    va_list ap;

    va_start(ap, msg);
    fprintf(stdout, "\n\nFATAL ERROR: ");
    vfprintf(stdout, msg, ap);
    fprintf(stdout, "\n\n");
    va_end(ap);
    exit(1);
}

/*
*	error()
*
*	Prints error msg to stdout.
*	Variadic parameter list can be passed.
*
*	Return values:
*		none
*/

void error(char *msg, ...)
{
    va_list ap;

    va_start(ap, msg);
    fprintf(stdout, "\n\nERROR: ");
    vfprintf(stdout, msg, ap);
    fprintf(stdout, "\n\n");
    va_end(ap);
}

/*
*	warning()
*
*	Prints warning msg to stdout.
*	Variadic parameter list can be passed.
*
*	Return values:
*		none
*/

void warning(char *msg, ...)
{
    va_list ap;

    va_start(ap, msg);
    fprintf(stdout, "\n\nWARNING: ");
    vfprintf(stdout, msg, ap);
    fprintf(stdout, "\n\n");
    va_end(ap);
}

/*
*	info()
*
*	Prints info msg to stdout.
*	Variadic parameter list can be passed.
*
*	Return values:
*		none
*/

void info(char *msg, ...)
{
    va_list ap;

    va_start(ap, msg);
    fprintf(stdout, "\n");
    vfprintf(stdout, msg, ap);
    va_end(ap);
}

char prompt_for_yes_no(char *title, char *prompt)
{
    char line[80];
    char lin2[80];

    fprintf(stdout, "\n%s\n", title);
    strcpy(line, prompt);
    strcat(line, "(y/n) ? [y]");
    getstr(line, lin2);
    if ((*lin2 != 'n') && (*lin2 != 'N'))
	return ('y');
    else
	return ('n');

}
