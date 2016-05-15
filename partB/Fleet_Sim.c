#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <locale.h>
#include "mpi.h"

#define TOTALLOCATIONS 1500

int main(int argc, char **argv) {

    int numOfVessels;  /* System accepts the first argument value and treat it as total number of vessels. */
    sscanf(argv[1], "%d", &numOfVessels);

    clock_t start, end;  /* Using start and end (clock_t type) to calculate the duration of initialization. */
    start = clock();  /* System now start to record the time. This is the beginning stage. */
    double timeForInitialization;  /* This declaration will record the initialization time. */
    int location;  /* This variable can be used as an index to help system retrieve data. */
    long numberOfRounds = 0;  /* Root processor can calculate which round is going on via this variable. */
    long totalNumberOfStrikes = 0;  /* Root processor can calculate how many strikes occurs via this variable. */
    double timeToMoveVessels;  /* The first timestamp after the initialization. */
    double previousTimestamp;  /* Help calculate timestamps that for printing on the terminal. */
    double timestamp;  /* Use this variable to print timestamp of every round. */
    
    int myId, size; 
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myId);  /* Every processor gets its own rank ID. */
    MPI_Comm_size(MPI_COMM_WORLD, &size);  /* Every processor knows the size of processors. */

    srand(time(NULL) + myId); /* Prepare a seed to generate random number for slave processors. */
    
    /* Allocate vessels to different slave processors. */
    int numOfVesselsForEachProc;
    if (myId != 0) {
        numOfVesselsForEachProc = numOfVessels / (size - 1);
        if (numOfVessels % (size - 1) > myId - 1) {
            numOfVesselsForEachProc += 1;
        }
    }

    /* Synchronization: Wait until every processor complete the initialization. */
    MPI_Barrier(MPI_COMM_WORLD); 

    /* Go into the root processor. */
    if (myId == 0) {
        end = clock();  /* Initialization period ends here. */
        timeForInitialization = (double) (end - start) / CLOCKS_PER_SEC;  /* Calculate the initialization time. */
        /* Calculate the first timestamp after the initialization and use MPI_Wtime() to increase the accuracy. */
        timeToMoveVessels = 0.0 - MPI_Wtime() + timeForInitialization; 
        previousTimestamp = 0; /* Set previousTimestamp zero. */
    }
    
    while (1) {
        /* Execute for 60 seconds, then exit from the loop. */
        if (timestamp > 60) {
            break;
        }
        /* Use as a send buffer to send data (how many vessels are there in a spot) to root processor. */
        short locationCount[TOTALLOCATIONS] = {0}; 
        /* Use as a receive buffer to store data (how many strikes occur) for root processor. */ 
        short globalCount[TOTALLOCATIONS] = {0};  
        
        if (myId != 0) {
            int i;
            for (i = 0; i < numOfVesselsForEachProc; i++) {
                /* Generate a rondom number (treated as a location) for every vessel. */
                location = rand() % TOTALLOCATIONS;  
                locationCount[location] += 1;  /* Store how many vessels are there in the same location. */
            }
        }

        /* 
          Root processor uses MPI_Reduce to sum up all the information from slave processors.
          --> locationCount comes from slave processors.
          --> globalCount is used to store data.
          --> MPI_SUM tells system to do the sum up operation.
        */
        MPI_Reduce(locationCount, globalCount, TOTALLOCATIONS, MPI_UNSIGNED_SHORT, MPI_SUM, 0, MPI_COMM_WORLD);

        if (myId == 0) {
            numberOfRounds += 1;  /* Number of rounds increases by one. */
            int numberOfStrikes = 0;  /* Use this variable to store how many strikes occur in each round. */
            
            /* Go through all the locations. */
            int i;
            for (i = 0; i < TOTALLOCATIONS; i++) {
                /* If there are more 4 vessels occur in the same location at the same time, number of strikes increases by one. */
                if (globalCount[i] >= 5) {  
                    numberOfStrikes += 1;
                }
            }
            
            totalNumberOfStrikes += numberOfStrikes; /* Accumulate total number of strikes. */
            timestamp = timeToMoveVessels + MPI_Wtime();  /* To print each round's timestamp. */
            printf("NO. Round %u -> Number of Strikes: %i, Timestamp: %lf\n", numberOfRounds, numberOfStrikes, timestamp);
        }
    }

    /* Root processor print the final result. */
    if (myId == 0) {
        printf("***************************************************************************\n");
        printf("*************************************************************\n");
        printf("*********************************************\n");
        printf("Number of strikes = %'lu\n", totalNumberOfStrikes);
        printf("*********************************************\n");
        printf("*************************************************************\n");
        printf("***************************************************************************\n");
    }
    MPI_Finalize();  /* End. */
}