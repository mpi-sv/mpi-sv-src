#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
 
//#define MAX_BUF_SIZE_LG 1
//#define NUM_MSGS_PER_BUF_SIZE 1

#define MAX_BUF_SIZE_LG        20
#define NUM_MSGS_PER_BUF_SIZE  10000
#define CACHE_LINE_SIZE        64

//char buf[1 << MAX_BUF_SIZE_LG];
 
/*
* This program verifies that MPI_Probe() is operating properly in the face of
* unexpected messages arriving after MPI_Probe() has
* been called.
*/
int main(int argc, char **argv)
{
    int   p_size;
    int   p_rank;
    int   msg_size_lg;
    int   errs = 0;
    int   mpi_errno;
    char *buf;
 
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &p_rank);

    posix_memalign((void *)&buf, CACHE_LINE_SIZE, (1 << MAX_BUF_SIZE_LG) * sizeof(char));

    printf("MPI_Probe. ^^^ %d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n", p_rank); fflush(stdout);
    for (msg_size_lg = 0; msg_size_lg <= MAX_BUF_SIZE_LG; msg_size_lg++)
    {
        const int msg_size = 1 << msg_size_lg;
        int msg_cnt;
        //if (p_rank == 0) printf( "\n\nTesting messages of size %d ------------------------------------\n", msg_size );fflush(stdout);
        for (msg_cnt = 0; msg_cnt < NUM_MSGS_PER_BUF_SIZE; msg_cnt++)
        {
            MPI_Status status;
            const int tag = (msg_size_lg * NUM_MSGS_PER_BUF_SIZE + msg_cnt) % 128;

            if (p_rank == 0)
            {
                int p;

              //printf( "Iter %d\n", msg_cnt );fflush(stdout);
              //if(msg_cnt % 10 == 0) {printf( "%d ", msg_cnt );fflush(stdout);}
                for (p = 1; p < p_size; p ++)
                {
                    /* Wait for synchronization message */
                    MPI_Recv(NULL, 0, MPI_CHAR, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
                    if (status.MPI_TAG != tag)  {
                        printf("ERROR: unexpected message tag    from MPI_Recv(): lp=0, rp=%6d, expected=%6d, actual=%6d, count=%6d\n",
                          status.MPI_SOURCE, status.MPI_TAG, tag, msg_cnt); 
                        fflush(stdout);
                        exit(1);
                    }
 
                    /* Send unexpected message which hopefully MPI_Probe() is already waiting for at the remote process */
                    MPI_Send (buf, msg_size, MPI_CHAR, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD);
                }
            }
            else
            {
                int incoming_msg_size;
 
                /* Send synchronization message */
                MPI_Send(NULL, 0, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
                /* Perform probe, hopefully before the master process can send its reply */
                MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, MPI_CHAR, &incoming_msg_size);
                if (status.MPI_SOURCE != 0)
                {
                    printf("ERROR: unexpected message source from MPI_Probe(): p=%6d, expected=     0, actual=%6d, count=%6d\n", 
                      p_rank, status.MPI_SOURCE, msg_cnt);fflush(stdout);
                    exit(1);
                }
                if (status.MPI_TAG != tag)
                {
                    printf("ERROR: unexpected message tag    from MPI_Probe(): p=%6d, expected=%6d, actual=%6d, count=%6d\n", 
                      p_rank, tag, status.MPI_TAG, msg_cnt);fflush(stdout);
                    exit(1);
                }
                if (incoming_msg_size != msg_size)
                {
                    printf("ERROR: unexpected message size   from MPI_Probe(): p=%6d, expected=%6d, actual=%6d, count=%6d\n", 
                      p_rank, msg_size, incoming_msg_size, msg_cnt);fflush(stdout);
                    exit(1);
                }
 
                /* Receive the probed message from the master process */
                MPI_Recv(buf, msg_size, MPI_BYTE, 0, tag, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, MPI_BYTE, &incoming_msg_size);
                if (status.MPI_SOURCE != 0)
                {
                    printf("ERROR: unexpected message source from MPI_Recv(): p=%6d, expected=     0, actual=%6d, count=%6d\n", 
                      p_rank, status.MPI_SOURCE, msg_cnt);fflush(stdout);
                    exit(1);
                }
                if (status.MPI_TAG != tag)
                {
                    printf("ERROR: unexpected message tag from MPI_Recv(): p=%d, expected=%6d, actual=%6d, count=%6d\n", 
                      p_rank, tag, status.MPI_TAG, msg_cnt);fflush(stdout);
                    exit(1);
                }
                if (incoming_msg_size != msg_size)
                {
                    printf("ERROR: unexpected message size from MPI_Recv(): p=%6d, expected=%6d, actual=%6d, count=%6d\n", 
                      p_rank, msg_size, incoming_msg_size, msg_cnt);fflush(stdout);
                    exit(1);
                }
            }
        }
    }
    printf("MPI_Probe. vvv %d vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n", p_rank); fflush(stdout); 
    MPI_Finalize();
    return 0;
}
