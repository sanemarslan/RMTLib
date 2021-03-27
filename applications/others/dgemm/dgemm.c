
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>

/* Include redundant execution header. */
#include "../../../include/ourRMTlib.h"

#define DGEMM_RESTRICT __restrict__

//global variables

int N = 256;
int repeats = 30;

double alpha = 1.0;
double beta  = 1.0;

// ------------------------------------------------------- //
// Function: get_seconds
//
// Vendor may modify this call to provide higher resolution
// timing if required
// ------------------------------------------------------- //
double get_seconds() {
	struct timeval now;
	gettimeofday(&now, NULL);

	const double seconds = (double) now.tv_sec;
	const double usec    = (double) now.tv_usec;

	return seconds + (usec * 1.0e-6);
}


//function out matrix multiplication!

double matrix_multiply(double* matrixA, double* matrixB, double* matrixC,
 double* sum){
	int r, i, j, k, check_threads=0, nthreads;
	
		#pragma omp parallel for private(j,k)
		for(i = 0; i < N; i++) {
			int ID = omp_get_thread_num();
			if(ID==0 && check_threads==0){
				nthreads=omp_get_num_threads();
				printf("matrix_multiply: Number of threads = %d\n",nthreads);
				check_threads=1;
			}
			for(j = 0; j < N; j++) {
				*sum = 0;

				for(k = 0; k < N; k++) {
					*sum += matrixA[i*N + k] * matrixB[k*N + j];
				}

				matrixC[i*N + j] = (alpha * (*sum)) + (beta * matrixC[i*N + j]);
			}
		}
}


// ------------------------------------------------------- //
// Function: main
//
// Modify only in permitted regions (see comments in the
// function)
// ------------------------------------------------------- //
int main(int argc, char* argv[]) {

	// ------------------------------------------------------- //
	// DO NOT CHANGE CODE BELOW
	// ------------------------------------------------------- //


	if(argc > 1) {
		N = atoi(argv[1]);
		printf("Matrix size input by command line: %d\n", N);

		if(argc > 2) {
			repeats = atoi(argv[2]);

			/*if(repeats < 30) {
				fprintf(stderr, "Error: repeats must be at least 30, setting is: %d\n", repeats);
				exit(-1);
			}*/

			printf("Repeat multiply %d times.\n", repeats);

            if(argc > 3) {
                alpha = (double) atof(argv[3]);

                if(argc > 4) {
                    beta = (double) atof(argv[4]);
                }
            }
		} else {
			printf("Repeat multiply defaulted to %d\n", repeats);
		}
	} else {
		printf("Matrix size defaulted to %d\n", N);
	}

    printf("Alpha =    %f\n", alpha);
    printf("Beta  =    %f\n", beta);

	if(N < 128) {
		printf("Error: N (%d) is less than 128, the matrix is too small.\n", N);
		exit(-1);
	}



# ifndef ENABLE_TIMING_ANALYSIS	
    start_timer();
# else
    start_timer(0);
# endif

	printf("Allocating Matrices...\n");

	double* DGEMM_RESTRICT matrixA = (double*) malloc(sizeof(double) * N * N);
	double* DGEMM_RESTRICT matrixB = (double*) malloc(sizeof(double) * N * N);
	double* DGEMM_RESTRICT matrixC = (double*) malloc(sizeof(double) * N * N);

	printf("Allocation complete, populating with values...\n");

	int i, j, k, r, tid, nthreads, check_threads=0;

	
	#pragma omp parallel for private(i,j,tid)
		for(i = 0; i < N; i++) {
			for(j = 0; j < N; j++) {
				matrixA[i*N + j] = 2.0;
				matrixB[i*N + j] = 0.5;
				matrixC[i*N + j] = 1.0;
			}
		}
	

	printf("Performing multiplication...\n");

	const double start = get_seconds();

	// ------------------------------------------------------- //
	// VENDOR NOTIFICATION: START MODIFIABLE REGION
	//
	// Vendor is able to change the lines below to call optimized
	// DGEMM or other matrix multiplication routines. Do *NOT*
	// change any lines above this statement.
	// ------------------------------------------------------- //

	
	double sum = 0;
	
	// Repeat multiple times
	for(r = 0; r < repeats; r++) {
		# ifdef ENABLE_RMT 
		  /* Run matrix_multiply in redundant mode */
		  activateRMT("f-L-2d-2d-2dC-dpC", &matrix_multiply, 4, matrixA, N, N, matrixB, N, N, matrixC, N, N, &sum);
		# else 
		  matrix_multiply(matrixA, matrixB, matrixC, &sum);
		# endif	
	}
	
	
 

	// ------------------------------------------------------- //
	// VENDOR NOTIFICATION: END MODIFIABLE REGION
	// ------------------------------------------------------- //

	// ------------------------------------------------------- //
	// DO NOT CHANGE CODE BELOW
	// ------------------------------------------------------- //

	const double end = get_seconds();

	printf("Calculating matrix check...\n");

	double final_sum = 0;
	long long int count     = 0;

	#pragma omp parallel for reduction(+:final_sum, count) private(i,j)
	for(i = 0; i < N; i++) {
		for(j = 0; j < N; j++) {
			final_sum += matrixC[i*N + j];
			count++;
		}
	}

	double N_dbl = (double) N;
	double matrix_memory = (3 * N_dbl * N_dbl) * ((double) sizeof(double));

	printf("\n");
	printf("===============================================================\n");

	const double count_dbl = (double) count;
	const double scaled_result = (final_sum / (count_dbl * repeats));

	printf("Final Sum is:         %f\n", scaled_result);

	const double check_sum = N_dbl + (1.0 / (double) (repeats));
	const double allowed_margin = 1.0e-8;

	if( (check_sum >= (scaled_result - allowed_margin)) &&
		(check_sum <= (scaled_result + allowed_margin)) ) {
		printf(" -> Solution check PASSED successfully.\n");
	} else {
		printf(" -> Solution check FAILED.\n");
	}

	printf("Memory for Matrices:  %f MB\n",
		(matrix_memory / (1024 * 1024)));

	const double time_taken = (end - start);

	printf("Multiply time:        %f seconds\n", time_taken);

	const double flops_computed = (N_dbl * N_dbl * N_dbl * 2.0 * (double)(repeats)) +
        	(N_dbl * N_dbl * 2 * (double)(repeats));

	printf("FLOPs computed:       %f\n", flops_computed);
	printf("GFLOP/s rate:         %f GF/s\n", (flops_computed / time_taken) / 1000000000.0);

	printf("===============================================================\n");
	printf("\n");
	
	
# ifndef ENABLE_TIMING_ANALYSIS	
    end_timer();
# else
    end_timer(0);
# endif
  
	free(matrixA);
	free(matrixB);
	free(matrixC);

	return 0;
}
