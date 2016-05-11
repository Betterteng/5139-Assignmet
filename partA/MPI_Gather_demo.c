/*
        组通信MPI程序：test_13_3_1.c
        收集MPI_Gather。
        实现从进程组中的每个进程收集10个整型数送给根进程(进程0)。
*/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
void main(int argc, char *argv[])
{
        int sendArray[10],size,myid;
        int root=0,*rbuf;       /* rbuf用于在root进程中存储从各进程收集的数据 */
        int i,n,m;
        MPI_Init(&argc,&argv);
        MPI_Comm_rank(MPI_COMM_WORLD,&myid);
        MPI_Comm_size(MPI_COMM_WORLD,&size);
        n=sizeof(sendArray)/sizeof(sendArray[0]);       /* sendArray数组元素个数 */
        m=n*size;       /* rbuf中数据元素个数 */
        /* 各进程中的数组sendArray赋初值：1~9之间的随机整数 */
        srand((unsigned)(time(NULL)+myid));
        for(i=0;i<n;i++)
           sendArray[i]=rand()%9+1;
        if(myid==root)
                rbuf=(int *)malloc(size*n*sizeof(int)); /* 动态分配内存 */
        /* 收集：各进程（包括root进程本身）发送数组到root进程，
           root进程用接收缓冲区rbuf接收这些数据 */
        //这里的接收个数为n,而不是m
        MPI_Gather(sendArray,n,MPI_INT,rbuf,n,MPI_INT,root,MPI_COMM_WORLD);
        /* 打印各进程数组数据 */
        fprintf(stderr,"\nProcess %d :",myid);
        for(i=0;i<n;i++)
                fprintf(stderr,"%d,",sendArray[i]);
        MPI_Barrier(MPI_COMM_WORLD);    /* 同步 */
        if (myid==root) /* root进程打印收集到的数据 */
        {
                fprintf(stderr,"\nAfter gathering data,Process %d :",myid);
                for(i=0;i<m;i++)
                        fprintf(stderr,"%d,",rbuf[i]);
                free(rbuf);     /* 释放动态分配的内存 */
        }
    MPI_Finalize();
} 