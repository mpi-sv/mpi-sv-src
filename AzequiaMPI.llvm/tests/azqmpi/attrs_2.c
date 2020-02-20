#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

struct Test_t {
  int One;
  int Two;
  int Three;
} val, val_dup, *val_out;


int copy_fn(MPI_Comm oldcomm, int keyval, void *extra_state,
	     void *attribute_val_in, void *attribute_val_out, int *flag) {

  printf("COPY_FN\n");

  *(int *)attribute_val_out = (int)attribute_val_in;
  *flag = 1;
  return MPI_SUCCESS;
}

int delete_fn( MPI_Comm comm, int keyval, void *attribute_val, void *extra_state) {

  printf("DELETE_FN\n");

  return MPI_SUCCESS;
}


int main (int argc, char **argv) {

  int       myid, numprocs;
  MPI_Comm  dup_comm_world;
  int       kv, kv_copy;
  int       flag;
  void     *value;

  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);


  if (myid == 0) {
    MPI_Keyval_create(copy_fn, delete_fn, &kv, NULL);

    kv_copy = kv;

    val.One   = 1;
    val.Two   = 2;
    val.Three = 3;
    printf("Putting in MPI_COMM_WORLD (keyval  %d)\n", kv);
    MPI_Attr_put(MPI_COMM_WORLD, kv, &val);
  }

  MPI_Comm_dup(MPI_COMM_WORLD, &dup_comm_world);

  if (myid == 0) {
    printf("MPI_COMM_WORLD: 0x%x\n", MPI_COMM_WORLD);
    printf("MPI_COMM_SELF:  0x%x\n", MPI_COMM_SELF);
    printf("dup_comm_world: 0x%x\n", dup_comm_world);
  }

  if (myid == 0) {
    val_dup.One   = 4;
    val_dup.Two   = 5;
    val_dup.Three = 6;
    MPI_Attr_put(dup_comm_world, kv, &val_dup);

    printf("VAL_OUT values for DUPLICATE communicator:\n");
    MPI_Attr_get(dup_comm_world, kv, &value, &flag);
    val_out = value;
    if (!flag)  { printf("ERROR: Attribute not found\n"); exit(1); }
    printf("\tOne:\t%d\n", val_out->One);
    printf("\tTwo:\t%d\n", val_out->Two);
    printf("\tThree:\t%d\n", val_out->Three);

    printf("VAL_OUT values for MPI_COMM_WORLD:\n");
    MPI_Attr_get(MPI_COMM_WORLD, kv, &value, &flag);
    val_out = value;
    if (!flag)  { printf("ERROR: Attribute not found\n"); exit(1); }
    printf("Getting from MPI_COMM_WORLD (keyval  %d)\n", kv);
    printf("\tOne:\t%d\n", val_out->One);
    printf("\tTwo:\t%d\n", val_out->Two);
    printf("\tThree:\t%d\n", val_out->Three);

  }

  MPI_Barrier(dup_comm_world);

  if (myid == 0) {
    printf("Freeing key ... \n");
    MPI_Keyval_free(&kv);
  }

  if (myid == 0) {
    printf("Freeing communicator ... \n");
  }

  MPI_Comm_free(&dup_comm_world);

  if (myid == 0) {
    printf("Finishing ... \n");

    printf("******* VAL_OUT values for MPI_COMM_WORLD:\n");
    MPI_Attr_get(MPI_COMM_WORLD, kv_copy, &value, &flag);
    val_out = value;
    if (!flag)  { printf("ERROR: Attribute not found\n"); exit(1); }
    printf("Getting from MPI_COMM_WORLD (keyval  %d)\n", kv_copy);
    printf("\tOne:\t%d\n", val_out->One);
    printf("\tTwo:\t%d\n", val_out->Two);
    printf("\tThree:\t%d\n", val_out->Three);

  }

  MPI_Finalize();
  return MPI_SUCCESS;
}
