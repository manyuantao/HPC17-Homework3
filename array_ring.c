// MPI ring communicaiton, sending an array

#include <stdio.h>
#include <mpi.h>
#include "util.h"

int main (int argc, char *argv[])
{
    int rank, size, array_length, tag, origin, destination, loop, N, i;
    MPI_Status status;
    
    char hostname[1024];
    gethostname(hostname, 1024);
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (argc != 2) {
        fprintf(stderr, "Function needs number of loops as input arguments!\n");
        MPI_Abort(MPI_COMM_WORLD,1);
    }
    
    sscanf(argv[1], "%d", &N);  // frequency of communications
    array_length = 2e6/sizeof(int);   // 2e6 bytes = 2MB
    tag = 99;
    
    int *message_in = malloc(sizeof(int) * array_length);
    int *message_out = malloc(sizeof(int) * array_length);
    
    for (i = 0; i < array_length; i++)
        message_out[i] = 0;
    
    /* timing */
    timestamp_type time1, time2;
    get_timestamp(&time1);
    
    for (loop = 0; loop < N; loop++) {
        
        origin = (rank - 1 + size) % size;
        destination = (rank + 1) % size;
        
        if (loop == 0 && rank == 0)
            MPI_Send(message_out, array_length, MPI_INT, destination, tag, MPI_COMM_WORLD);
        else {
            
            MPI_Recv(message_in,  array_length, MPI_INT, origin,      tag, MPI_COMM_WORLD, &status);
            
            for (i = 0; i < array_length; i++)
                message_out[i] = message_in[i] + rank;
            MPI_Send(message_out, array_length, MPI_INT, destination, tag, MPI_COMM_WORLD);
        }
    }
    
    if (rank == 0) {
        
        MPI_Recv(message_in,  array_length, MPI_INT, origin,      tag, MPI_COMM_WORLD, &status);
        printf("LoopEND: rank %d received message %d from rank %d, hosted on %s\n", rank, message_in[0], origin, hostname);
    }
    
    /* timing */
    get_timestamp(&time2);
    
    MPI_Finalize();
    
    double elapsed = timestamp_diff_in_seconds(time1, time2);
    double bandwidth = array_length * sizeof(int) * 2 * size * N / elapsed / 1e6;
    
    if (rank == 0) {
        printf("Number in the final array is %d.\n", message_in[0]);
        printf("The bandwidth is %f MB per second.\n", bandwidth);
        printf("Total elapsed time is %f seconds.\n", elapsed);
    }
    
    free(message_in);
    free(message_out);
    return 0;
}
