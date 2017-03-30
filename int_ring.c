// MPI ring communicaiton, sending an interger

#include <stdio.h>
#include <mpi.h>
#include "util.h"

int main (int argc, char *argv[])
{
    int rank, size, tag, origin, destination, loop, N;
    int message_in, message_out;
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
    
    sscanf(argv[1], "%d", &N); // frequency of communications
    message_out = 0;
    tag = 99;
    
    /* timing */
    timestamp_type time1, time2;
    get_timestamp(&time1);
    
    for (loop = 0; loop < N; loop++) {
        
        origin = (rank - 1 + size) % size;
        destination = (rank + 1) % size;
        
        if (loop == 0 && rank == 0) {
            
            MPI_Send(&message_out, 1, MPI_INT, destination, tag, MPI_COMM_WORLD);
            printf("Loop %d: rank %d sent \t\b message %-3d to \t rank %d, hosted on %s\n", loop, rank, message_out, destination, hostname);
        }
        else {
            
            MPI_Recv(&message_in,  1, MPI_INT, origin,      tag, MPI_COMM_WORLD, &status);
            
            message_out = message_in + rank;
            MPI_Send(&message_out, 1, MPI_INT, destination, tag, MPI_COMM_WORLD);
            
            printf("Loop %d: rank %d received message %-3d from rank %d, hosted on %s\n", loop, rank, message_in, origin, hostname);
            printf("Loop %d: rank %d sent \t\b message %-3d to \t rank %d, hosted on %s\n", loop, rank, message_out, destination, hostname);
        }
    }
    
    if (rank == 0) {
        
        MPI_Recv(&message_in,  1, MPI_INT, origin,      tag, MPI_COMM_WORLD, &status);
        printf("LoopEND:rank %d received message %-3d from rank %d, hosted on %s\n", rank, message_in, origin, hostname);
    }
    
    /* timing */
    get_timestamp(&time2);
    
    MPI_Finalize();
    
    double elapsed = timestamp_diff_in_seconds(time1, time2);
    double latency = elapsed / N/size;
    
    if (rank == 0) {
        printf("The final message is %d.\n", message_in);
        printf("The latency is %.12f seconds per communication.\n", latency);
        printf("Total elapsed time is %f seconds.\n", elapsed);
    }
    
    return 0;
}
