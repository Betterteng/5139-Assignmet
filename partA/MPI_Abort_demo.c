#include "mpi.h"
#include <stdio.h>

int main(int argc,char *argv[])
{
        int rank,size;

        MPI_Init(&argc,&argv);
        /* Get processor tag and total number of processors */
        MPI_Comm_rank(MPI_COMM_WORLD,&rank);
        MPI_Comm_size(MPI_COMM_WORLD,&size);

        /* If number of processors is less than 4 */
        if(size < 4)
        {
                fprintf(stderr,"system requires at least 4 processes.\n");
                /* 
                   Terminates all MPI processes associated with the communicator
                   MPI_COMM_WORLD (error code: 99)
                */
                MPI_Abort(MPI_COMM_WORLD,99);
        }

        /* If number of processors is more than 3 */
        if (size >= 4) {
                /* Print to remind user totol munber of processors is more than 3 */
                fprintf(stderr, "Processor: %d ->This is fine.\n", rank);
        }
        MPI_Finalize();
}