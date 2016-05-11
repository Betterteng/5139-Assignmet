/*
    Processor 0 will gather data from other processor.
*/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
void main(int argc, char *argv[])
{
        int sendArray[1], size, myid;
        /* rbuf is used to help root processor (processor 0) to gather data from other processors */
        int root = 0, *rbuf;       
        int i, n, m;
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &myid);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        n = sizeof(sendArray) / sizeof(sendArray[0]);       /* number of elements in the sendArray[]*/
        m = n * size;       /* number of elements in the receive buffer */
        /* 
           Generate random integer form 1 to 9,
           then put it in sendArray of each processor.
        */
        srand((unsigned)(time(NULL)+myid));
        for(i = 0; i < n; i++) {
           sendArray[i] = rand() % 9 + 1;
        }
        if(myid == root) {
            /* Allocate memory to receive buffer dynamically. */
            rbuf = (int *)malloc(size*n*sizeof(int)); 
        }
        /* 
           Gather: Each processor (include root processor 0) pass data to root processor (processor 0).
           Root processor use receive buffer (rbut) to gather these data.
        */
        // NOTICE: here the number of receiving should be n but not m.
        MPI_Gather(sendArray,n,MPI_INT,rbuf,n,MPI_INT,root,MPI_COMM_WORLD);
        /* Print data that each processor has sent. */
        fprintf(stderr, "\nProcess %d :", myid);
        for(i = 0; i < n; i++) {
                fprintf(stderr, "%d ", sendArray[i]);
        }
        /* Syncronization: Wait until all the processors have sent data to root processor 0. */
        MPI_Barrier(MPI_COMM_WORLD);  
        /* Root processor 0 print what it has received. */  
        if (myid == root) 
        {
            fprintf(stderr,"\nAfter gathering data,Process %d : ",myid);
            for(i = 0; i < m; i++) {
                fprintf(stderr, "%d  ", rbuf[i]);
            }
            free(rbuf);     /* Release the memory. */
        }
    MPI_Finalize();
} 