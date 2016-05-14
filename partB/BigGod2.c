#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <locale.h>
#include "mpi.h"

#define TOTALLOCATIONS 1500

int main(int argc, char **argv) {

    // Get fleet size from the first argument value
    int numOfVessels;
    sscanf(argv[1], "%d", &numOfVessels);

    // Initialization time
    clock_t start, end;
    start = clock();
    double timeForInitialization;

    // Prepare variables
    int location;
    long numberOfRounds = 0;
    long numberOfStrikes = 0;
    // single round time spent
    double eachRoundTime;
    // Track time
    double timeToMoveVessels;
    double previousTimestamp;
    double timestamp;
    
    int myId, size;
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myId);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Seed
    srand(time(NULL) + myId);
    
    // Random number that need to be generated
    int numOfVesselsForEachProc;
    if (myId != 0) {
        numOfVesselsForEachProc = numOfVessels / (size - 1);
        if (numOfVessels % (size - 1) > myId - 1) {
            numOfVesselsForEachProc += 1;
        }
    }

    // Ensure all processes have done initialization
    MPI_Barrier(MPI_COMM_WORLD); 

    if (myId == 0) {
        end = clock();
        // initialization time
        timeForInitialization = (double) (end - start) / CLOCKS_PER_SEC; 
        // Track elapsed time
        timeToMoveVessels = 0.0 - MPI_Wtime() + timeForInitialization; // use MPI_Wtime for high accuracy
        previousTimestamp = 0; // initial value for previousTimestamp
    }
    
    // Enter loop, will execute for one minute
    while (1) {
        // Location array
        short locationCount[TOTALLOCATIONS] = {0};
        // Buffer to send, also the global count
        short globalCount[TOTALLOCATIONS] = {0};
        // Slave processes
        if (myId != 0) {
            // Generate random number and count numbers in location array
            int i;
            for (i = 0; i < numOfVesselsForEachProc; i++) {
                location = rand() % TOTALLOCATIONS;
                locationCount[location] += 1;
            }

        }

        // Gather results using MPI_Reduce
        MPI_Reduce(locationCount, globalCount, TOTALLOCATIONS, MPI_UNSIGNED_SHORT, MPI_SUM, 0, MPI_COMM_WORLD);

        // Master process
        if (myId == 0) {
            // increment rounds count
            numberOfRounds += 1;
            int launch_number = 0;
            // Loop through locations to calculate launches
            int i;
            for (i = 0; i < TOTALLOCATIONS; i++) {
                if (globalCount[i] > 4) {
                    launch_number++;
                }
            }
            // sum up total launches over 1 minute
            numberOfStrikes += launch_number;
            // Track time elapse
            timestamp = timeToMoveVessels + MPI_Wtime();
            // Make sure root process will end loop
            if (timestamp > 5) {
                break;
            }
            eachRoundTime = timestamp - previousTimestamp;
            previousTimestamp = timestamp;
           
            printf("NO. Round %u -> Number of Strikes: %i, Timestamp: %lf\n",
                   numberOfRounds, launch_number, timestamp);
        }

    }

    // Master
    if (myId == 0) {
        printf("***************************************************************************\n");
        printf("*************************************************************\n");
        printf("*********************************************\n");
        printf("Number of strikes = %'lu\n", numberOfStrikes);
        printf("*********************************************\n");
        printf("*************************************************************\n");
        printf("***************************************************************************\n");
        // End all processes after printing all wanted results
        //MPI_Abort(MPI_COMM_WORLD, 0);
    }

    MPI_Finalize();
}