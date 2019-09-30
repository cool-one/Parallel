/*  
  RC
  monteCarlo2.c
  10-20-16
  Summary: MPI program that calculates value of pi, using a dartboard simulation of a 1 unit radius circle over
  a 2 unit sided square.  
  This version utilizes all requested cores in pooled calculations
  To compile:  mpicc -g -Wall -o mpi_monte2 monteCarlo2.c
  To execute:  mpiexec -n <# processes> ./mpi_monte2
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#include <time.h>
long long int playDarts(long long int howManyTosses);  //prototype

int main(int argc, char* argv[]) 
{
  // !Can manually set globalTosses here to test
  long long int globalTosses;  //g # of total tosses requested
  long long int number_of_tosses;  //g # tosses given per process
  long long int number_in_circle = 0;  //g #  number of throws into circle
  int comm_sz; //Number of processes
  int my_rank; //Rank of local process

  // seed the random num generator, otherwise same #s every run
  srand(time(NULL));
    
  // MPI Start
  MPI_Init(NULL, NULL);
  // This initiatest the size of MPI_COMM, taken from command line
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
  // This initiates the rank of the particular iterated process
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // for all other processes
    if (my_rank != 0) 
    {
      //Get number_of_tosses through Bcast from process 0 
      MPI_Bcast(&number_of_tosses, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

      number_in_circle = playDarts(number_of_tosses);

      int i = 0;
      //Sends data for Reduce, i is dummy variable
      MPI_Reduce(&number_in_circle, &i, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    }
  // specific to process 0
  else
  {
    // logging info to record trial times
    FILE* outFile;
    outFile = fopen("yourFile", "a");

    //timer
    clock_t start, end;
    double cpu_time_used;
    start = clock();

    // request number of total tosses from user
    printf("Please enter the number of tosses: \n");
    scanf("%lli", &globalTosses);

    double pi_estimate;
    long long int global_circle;  //Total of throws into circle

    // Calculate "number_of_tosses" to give each process, "comm_sz" = # of processes
    number_of_tosses = globalTosses / comm_sz;

    // Broadcast number_of_tosses to all processes, INCLUDING PROCESS 0           
    MPI_Bcast(&number_of_tosses, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    number_in_circle = playDarts(number_of_tosses);

    //Get global sum by combining all local sums
    MPI_Reduce(&number_in_circle, &global_circle, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    // calculate estimate of pi from data received
    pi_estimate = 4*global_circle / ((double)globalTosses);
    printf("pi: %f\n", pi_estimate);

    // timer
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    fprintf(outFile, "%d, %lld, %f, %f\n", comm_sz, globalTosses, cpu_time_used, pi_estimate);
    fclose(outFile);
    printf("Total time taken by CPU: %f\n", cpu_time_used);
  }
  // MPI game over
  MPI_Finalize();
  return 0;
}

// Find number of throws in circle
long long int playDarts(long long int howManyTosses)
{
  double x;  // x-position of throw
  double y;  // y-position of throw
  long long int toss; //counter
  double distance_squared;  //used in calculation 
  long long int N_I_C = 0;  //number in circe (its final value is returned)
  for(toss=0; toss < howManyTosses; toss++) 
  {
    // generate random number -1 to 1
    // RAND_MAX is a constant, rand() = from 0 to RAND_MAX
    x = ((rand()/(double)RAND_MAX)) * 2 - 1;  
    y = ((rand()/(double)RAND_MAX)) * 2 - 1; 
    //~printf("x: %f, y: %f\n", x, y);
    distance_squared = x*x + y*y;
    if(distance_squared <= 1)
    {
      N_I_C++;
    }
  }
  return N_I_C;
}
