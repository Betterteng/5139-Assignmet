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
    setlocale(LC_NUMERIC, "");
    double timeForInitialization;

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes, which is also the fleet size
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Get the rank of the process
    int myId;
    MPI_Comm_rank(MPI_COMM_WORLD, &myId);

    // Random number that need to be generated
    int num_rands;
    if (myId != 0) {
        num_rands = numOfVessels / (size - 1);
        if (numOfVessels % (size - 1) > myId - 1) {
            num_rands++;
        }
    }
    // Seed
    srand(time(NULL) + myId);

    // Prepare variables
    unsigned int i, location;
    unsigned int num_round = 0;
    unsigned long total_launches = 0;
    // single round time spent
    double round_time;
    double best_round_time;
    // Track time
    double time_start;
    double time_elapse;
    double time_elapse_prev;

    // Ensure all processes have done initialization
    MPI_Barrier(MPI_COMM_WORLD); 
    if (myId == 0) {
        end = clock();
        // initialization time
        timeForInitialization = (double) (end - start) / CLOCKS_PER_SEC; 
        // Track elapsed time
        time_start = 0.0 - MPI_Wtime() + timeForInitialization; // use MPI_Wtime for high accuracy
        time_elapse_prev = 0; // initial value for time_elapse_prev
    }
    
    // Enter loop, will execute for one minute
    while (1) {
        // Location array
        unsigned short location_count[TOTALLOCATIONS] = {0};
        // Buffer to send, also the global count
        unsigned short global_count[TOTALLOCATIONS] = {0};
        // Slave processes
        if (myId != 0) {
            // Generate random number and count numbers in location array
            for (i = 0; i < num_rands; i++) {
                location = rand() % TOTALLOCATIONS;
                location_count[location]++;
            }

        }

        // Gather results using MPI_Reduce
        MPI_Reduce(location_count, global_count, TOTALLOCATIONS, MPI_UNSIGNED_SHORT, MPI_SUM, 0, MPI_COMM_WORLD);

        // Master process
        if (myId == 0) {
            // increment rounds count
            num_round++;
            int launch_number = 0;
            // Loop through locations to calculate launches
            for (i = 0; i < TOTALLOCATIONS; i++) {
                if (global_count[i] > 4) {
                    launch_number++;
                }
            }
            // sum up total launches over 1 minute
            total_launches += launch_number;
            // Track time elapse
            time_elapse = time_start + MPI_Wtime();
            // Make sure root process will end loop
            if (time_elapse > 5) {
                break;
            }
            round_time = time_elapse - time_elapse_prev;
            time_elapse_prev = time_elapse;
            // Track best round time
            if (round_time < best_round_time) {
                best_round_time = round_time;
            }
            printf("Round %u: Launches = %i, Sampling Interval = %lf, Time elapsed = %lf\n",
                   num_round, launch_number, round_time, time_elapse);
        }

    }

     //MPI_Barrier(MPI_COMM_WORLD);
    // Master
    if (myId == 0) {
        printf("***************************************************************************\n");
        printf("*************************************************************\n");
        printf("*********************************************\n");
        printf("Number of strikes = %'lu\n", total_launches);
        printf("*********************************************\n");
        printf("*************************************************************\n");
        printf("***************************************************************************\n");
        // End all processes after printing all wanted results
        MPI_Abort(MPI_COMM_WORLD, 0);
    }

    MPI_Finalize();
    //return (0);
}