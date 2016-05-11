#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>

void main(int argc, char *argv[]) {
	int recvArray[5], size, myid;     /* recvArray will store different elements that come from other threads */
    int root=0, *sbuf;         /* sbuf can be used to store the elements that are going to spread to other processors */
    int i, sendCount, m;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    sendCount = sizeof(recvArray) / sizeof(recvArray[0]);       /* number of elements in recvArray */
    m = sendCount*size;       /* number of elements sbuf */
    /* 
       Dynamically allocate the memory to the sbuf
       and then generate random integers from 1 to 9. 
       Put the random numbers in the sbuf.
    */
    srand((unsigned)time(NULL));
    if(myid == root)
        {   
            sbuf = (int *)malloc(size*sendCount*sizeof(int)); /* Dynamic memory allocation */
            for(i=0;i<m;i++)
                sbuf[i]=rand()%9+1;
        }   
    /* 
       Scatter now: root processor (processor 0) will scatter the data that is 
       in the sbuf to other processors (include root processor itself).
       Other processor will receive the data using recvArray. 
    */
    MPI_Scatter(sbuf,sendCount,MPI_INT,recvArray,sendCount,MPI_INT,root,MPI_COMM_WORLD);

    if (myid==root) /* Root processor prints the data that it's scattered */
    {   
        fprintf(stderr,"\nScattered data, Process %d :",myid);
        for(i=0;i<m;i++)
            fprintf(stderr,"%d,",sbuf[i]);
        free(sbuf);     /* Release the memory. */
    }  
    /* 
      Syncronization: Every processor wait root processor to print.
	  Otherwise, the output in the screen will looks weird.
     */
    MPI_Barrier(MPI_COMM_WORLD);    
    /* Processors print what they've received. */
    fprintf(stderr,"\nProcess %d received :",myid);
    for(i=0;i<sendCount;i++)
        fprintf(stderr,"%d,",recvArray[i]);
    /* This MPI_Barrier is used for format the output of printf() */
    MPI_Barrier(MPI_COMM_WORLD);
    printf("\n");
    MPI_Finalize();     
}