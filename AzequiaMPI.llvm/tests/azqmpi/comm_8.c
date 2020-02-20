#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define MPI_Aint int

int test_communicators ( int iter );
int copy_fn ( MPI_Comm, int, void *, void *, void *, int * );
int delete_fn ( MPI_Comm, int, void *, void * );


int main (int argc, char **argv) {

    int     i;
    int     myid;
    double  t_start, t_end;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    t_start = MPI_Wtime();
    for (i = 0; i < 1000; i++)
      test_communicators(i);
    t_end = MPI_Wtime();
    if (myid == 0)
       fprintf(stdout, "Time: %lf\n", t_end - t_start);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    return MPI_SUCCESS;
}

int copy_fn( MPI_Comm oldcomm, int keyval, void *extra_state,
	     void *attribute_val_in, void *attribute_val_out,
	     int *flag)
{
/* Note that if (sizeof(int) < sizeof(void *), just setting the int
   part of attribute_val_out may leave some dirty bits
 */
  *(MPI_Aint *)attribute_val_out = (MPI_Aint)attribute_val_in;
  *flag = 1;
  return MPI_SUCCESS;
}

int delete_fn( MPI_Comm comm, int keyval, void *attribute_val,
	       void *extra_state)
{
  int world_rank;
  MPI_Comm_rank( MPI_COMM_WORLD, &world_rank );
  if ((MPI_Aint)attribute_val != (MPI_Aint)world_rank) {
    printf( "incorrect attribute value %d\n", *(int*)attribute_val );
    MPI_Abort(MPI_COMM_WORLD, 1005 );
  }
  return MPI_SUCCESS;
}

int test_communicators( int iter ) {

  MPI_Comm dup_comm_world, lo_comm, rev_comm, dup_comm, split_comm, world_comm;
  MPI_Group world_group, lo_group, rev_group;
  void *vvalue;
  int ranges[1][3];
  int flag, world_rank, world_size, rank, size, n, key_1, key_3;
  int color, key, result;
  MPI_Aint value;

  
  MPI_Comm_rank( MPI_COMM_WORLD, &world_rank );
  MPI_Comm_size( MPI_COMM_WORLD, &world_size );
  if (world_rank == 0) {
    printf( "*** Communicators  (%d) ***\n", iter );  fflush(stdout);

  }

  MPI_Comm_dup( MPI_COMM_WORLD, &dup_comm_world );

  if (world_rank == 0)
    printf( "    Comm_create\n" );  fflush(stdout);



  MPI_Comm_group( dup_comm_world, &world_group );
  MPI_Comm_create( dup_comm_world, world_group, &world_comm );
  MPI_Comm_rank( world_comm, &rank );
  if (rank != world_rank) {
    printf( "incorrect rank in world comm: %d\n", rank );  fflush(stdout);

    MPI_Abort(MPI_COMM_WORLD, 3001 );
  }

  n = world_size / 2;

  ranges[0][0] = 0;
  ranges[0][1] = (world_size - n) - 1;
  ranges[0][2] = 1;

  MPI_Group_range_incl(world_group, 1, ranges, &lo_group );
  MPI_Comm_create(world_comm, lo_group, &lo_comm );
  MPI_Group_free( &lo_group );

  if (world_rank < (world_size - n)) {
    MPI_Comm_rank(lo_comm, &rank );
    if (rank == MPI_UNDEFINED) {
	  printf( "incorrect lo group rank: %d\n", rank );  fflush(stdout);

	    MPI_Abort(MPI_COMM_WORLD, 3002 );
	  }
    else {
	    MPI_Barrier(lo_comm );
    }
  }
  else {
    if (lo_comm != MPI_COMM_NULL) {
	  printf( "incorrect lo comm:\n" );  fflush(stdout);

	    MPI_Abort(MPI_COMM_WORLD, 3003 );
	  }
  }

  //MPI_Barrier(world_comm);

  if (lo_comm != MPI_COMM_NULL) {
	
    int myid, my_size, id, k;
	
    MPI_Comm_dup(lo_comm, &dup_comm );
	
	MPI_Comm_rank(dup_comm, &myid);
	MPI_Comm_size(dup_comm, &my_size);
	
	for (k = 0; k < 1000; k++) {
	if (myid % 2) {
	  MPI_Send(&myid, 1, MPI_INT, myid - 1, 99, dup_comm);
	} else {
	  MPI_Recv(&id, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, dup_comm, MPI_STATUS_IGNORE);
	  if (myid != (id - 1)) {
		printf("ERROR: Soy el %d y recibo de %d\n", myid, id);
	  }
	}
	}
  }
  
  if (world_rank == 0)
    printf( "    Comm_dup\n" );  fflush(stdout);

/*
 
  if (lo_comm != MPI_COMM_NULL) {
    value = 9;
    MPI_Keyval_create(copy_fn,     delete_fn,   &key_1, &value );
    value = 8;
    value = 7;
    MPI_Keyval_create(MPI_NULL_COPY_FN, MPI_NULL_DELETE_FN,
                               &key_3, &value );

    MPI_Attr_put(lo_comm, key_1, (void *)world_rank );

    MPI_Attr_put(lo_comm, key_3, (void *)0 );

    MPI_Comm_dup(lo_comm, &dup_comm );

    /* Note that if sizeof(int) < sizeof(void *), we can't use
       (void **)&value to get the value we passed into Attr_put.  To avoid
       problems (e.g., alignment errors), we recover the value into
       a (void *) and cast to int. Note that this may generate warning
       messages from the compiler.  *
    MPI_Attr_get(dup_comm, key_1, (void **)&vvalue, &flag );
    value = (MPI_Aint)vvalue;

    if (! flag) {
	    printf( "dup_comm key_1 not found on %d\n", world_rank );
	    MPI_Abort(MPI_COMM_WORLD, 3004 );
	  }

    if (value != world_rank) {
	    printf( "dup_comm key_1 value incorrect: %ld\n", (long)value );
	    MPI_Abort(MPI_COMM_WORLD, 3005 );
	  }

    MPI_Attr_get(dup_comm, key_3, (void **)&vvalue, &flag );
    value = (int)vvalue;
    if (flag) {
      printf( "dup_comm key_3 found!\n" );
	    MPI_Abort(MPI_COMM_WORLD, 3008 );
	  }
    MPI_Keyval_free(&key_1 );
  
    MPI_Keyval_free(&key_3 );
  }

  /*
  if (world_rank == 0)
	  printf( "    Comm_split\n" );

    color = world_rank % 2;
    key   = world_size - world_rank;

    MPI_Comm_split(dup_comm_world, color, key, &split_comm );
    MPI_Comm_size(split_comm, &size );
    MPI_Comm_rank(split_comm, &rank );
    if (rank != ((size - world_rank/2) - 1)) {
	  printf( "incorrect split rank: %d\n", rank );
	  MPI_Abort(MPI_COMM_WORLD, 3009 );
	}

  MPI_Barrier(split_comm );

  if (world_rank == 0)
    printf( "    Comm_compare\n" );

    MPI_Comm_compare(world_comm, world_comm, &result );
    if (result != MPI_IDENT) {
      printf( "incorrect ident result: %d\n", result );
      MPI_Abort(MPI_COMM_WORLD, 3010 );
	  }

    if (lo_comm != MPI_COMM_NULL) {
      MPI_Comm_compare(lo_comm, dup_comm, &result );
      if (result != MPI_CONGRUENT) {
        printf( "incorrect congruent result: %d\n", result );
        MPI_Abort(MPI_COMM_WORLD, 3011 );
	    }
	  }

    ranges[0][0] = world_size - 1;
    ranges[0][1] = 0;
    ranges[0][2] = -1;

    MPI_Group_range_incl(world_group, 1, ranges, &rev_group );
    MPI_Comm_create(world_comm, rev_group, &rev_comm );
    MPI_Comm_compare(world_comm, rev_comm, &result );
    if (result != MPI_SIMILAR) {
      printf( "incorrect similar result: %d\n", result );
      MPI_Abort(MPI_COMM_WORLD, 3012 );
	  }

    if (lo_comm != MPI_COMM_NULL) {
	    MPI_Comm_compare(world_comm, lo_comm, &result );
	    if (result != MPI_UNEQUAL) {
	      printf( "incorrect unequal result: %d\n", result );
	      MPI_Abort(MPI_COMM_WORLD, 3013 );
	    }
	  }
*/
   if (world_rank == 0)
	 printf( "    Comm_free\n" );  fflush(stdout);


  
    MPI_Comm_free( &world_comm );
    MPI_Comm_free( &dup_comm_world );

//    MPI_Comm_free( &rev_comm );
//    MPI_Comm_free( &split_comm );

    MPI_Group_free( &world_group );
//    MPI_Group_free( &rev_group );

    if (lo_comm != MPI_COMM_NULL) {
      MPI_Comm_free( &lo_comm );
      MPI_Comm_free( &dup_comm );
    }
  
   return 0;
}

