/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* floyd-warshall.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>


/* Include polybench common header. */
#include "../../../../include/polybench.h"

/* Include benchmark-specific header. */
#include "floyd-warshall.h"

/* Include redundant execution header. */
#include "../../../../include/ourRMTlib.h"

/* Array initialization. */
static
void init_array (int n,
		 DATA_TYPE POLYBENCH_2D(path,N,N,n,n))
{
  int i, j;

  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++) {
      path[i][j] = i*j%7+1;
      if ((i+j)%13 == 0 || (i+j)%7==0 || (i+j)%11 == 0)
         path[i][j] = 999;
    }
}


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int n,
		 DATA_TYPE POLYBENCH_2D(path,N,N,n,n), char* filename)

{
  int i, j;
  
  FILE* fptr = fopen(filename, "w+");
  if(fptr==NULL)
	fprintf(stderr,"fptr is NULL\n");
	

  //POLYBENCH_DUMP_START;
  fprintf(fptr, "==BEGIN DUMP_ARRAYS==\n");
  //POLYBENCH_DUMP_BEGIN("path");
  fprintf(fptr, "begin dump: %s", "path");
  
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++) {
      if ((i * n + j) % 20 == 0) fprintf (fptr, "\n");
      //fprintf (POLYBENCH_DUMP_TARGET, DATA_PRINTF_MODIFIER, path[i][j]);
      fprintf (fptr, DATA_PRINTF_MODIFIER, path[i][j]);
    }
  //POLYBENCH_DUMP_END("path");
  fprintf(fptr, "\nend   dump: %s\n", "path");
  //POLYBENCH_DUMP_FINISH;
  fprintf(fptr, "==END   DUMP_ARRAYS==\n");
  fclose(fptr);
}

/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static
void kernel_floyd_warshall(int n,
			   DATA_TYPE POLYBENCH_2D(path,N,N,n,n))
{
  int i, j, k;

#pragma scop
  for (k = 0; k < _PB_N; k++)
    {
      for(i = 0; i < _PB_N; i++)
	for (j = 0; j < _PB_N; j++)
	  path[i][j] = path[i][j] < path[i][k] + path[k][j] ?
	    path[i][j] : path[i][k] + path[k][j];
    }
#pragma endscop

}

int main(int argc, char** argv)
{
  if(argc!=2){
		fprintf(stderr, "out file name required!\n");
		exit(1);
  }
  start_timer();
  
  /* Retrieve problem size. */
  int n = N;

  /* Variable declaration/allocation. */
  POLYBENCH_2D_ARRAY_DECL(path, DATA_TYPE, N, N, n, n);


  /* Initialize array(s). */
  init_array (n, POLYBENCH_ARRAY(path));

  /* Start timer. */
  polybench_start_instruments;


# ifdef ENABLE_RMT 
  /* Run the kernel in redundant mode */
  activateRMT("f-L-i-2iC", &kernel_floyd_warshall, 2, n, path, n, n);
# else
  /* Run kernel. */
  kernel_floyd_warshall (n, POLYBENCH_ARRAY(path));
# endif

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */

  polybench_prevent_dce(print_array(n, POLYBENCH_ARRAY(path), argv[1]));
  
  
  /* Be clean. */
  POLYBENCH_FREE_ARRAY(path);

  end_timer();
  
  return 0;
}
