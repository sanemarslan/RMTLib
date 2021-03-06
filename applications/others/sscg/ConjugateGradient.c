/**
  * Copyright 2019 José Manuel Abuín Mosquera <josemanuel.abuin@usc.es>
  * 
  * This file is part of Matrix Market Suite.
  *
  * Matrix Market Suite is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * Matrix Market Suite is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with Matrix Market Suite. If not, see <http://www.gnu.org/licenses/>.
  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cblas.h>

#include "ConjugateGradient.h"
#include "ConjugateGradientSolver.h"
#include "ConjugateGradientSolverBasic.h"

/* Include redundant execution header. */
#include "../../../include/ourRMTlib.h"

void usageConjugateGradient(){

	fprintf(stderr, "\n");
	fprintf(stderr, "Usage: MM-Suite ConjugateGradient [options] <input-matrix> <input-vector>\n");
	fprintf(stderr, "Algorithm options:\n\n");
	fprintf(stderr, "       -i INT        Iteration number. Default: number of matrix rows * 2\n");
	fprintf(stderr, "       -b            Uses basic operations instead of optimized BLAS libraries. Default: False\n");
	fprintf(stderr, "\nInput/output options:\n\n");
	fprintf(stderr, "       -o STR        Output file name. Default: stdout\n");
	fprintf(stderr, "       -r            Input format is row per line. Default: False\n");
	fprintf(stderr, "\nPerformance options:\n\n");
	fprintf(stderr, "       -t INT        Number of threads to use in OpenBLAS. Default: 1\n");
	fprintf(stderr, "\n");

}

//int ConjugateGradient(int argc, char *argv[]) {
int main(int argc, char *argv[]) {

	int 			ret_code;
	int 			option;
	
	unsigned long 		*II;
	unsigned long 		*J;
	double 			*A;
	
	unsigned long 		M;
	unsigned long 		N;
	unsigned long long 	nz;
	
	
	double 			*b;
	unsigned long 		M_Vector;
	unsigned long 		N_Vector;
	unsigned long long 	nz_vector;
	
	char			*outputFileName = NULL;
	int			iterationNumber = 0;
	
	char			*inputMatrixFile = NULL;
	char			*inputVectorFile = NULL;
	
	int			numThreads = 1;
	
	int			inputFormatRow = 0;
	int			basicOps = 0;
	int			F=0;
	
	while ((option = getopt(argc, argv,"bro:i:t:f:")) >= 0) {
		switch (option) {
			case 'o' : 
				//free(outputFileName);
				
				outputFileName = (char *) malloc(sizeof(char)*strlen(optarg)+1);
				strcpy(outputFileName,optarg);
				
				break;
			
			case 'i' :
				iterationNumber = atoi(optarg);
				break;
				
			case 'r':
				inputFormatRow = 1;
				break;
			case 'b':
				basicOps = 1;
				break;
			
			case 't':
				numThreads = atoi(optarg);
				break;
				
			case 'f':
				F = atoi(optarg);
				break;
			
			default: break;
		}
	
	}
	
	if ((optind + 2 > argc) || (optind + 3 <= argc)) {
		usageConjugateGradient();
		return -1;
	}
	
	//openblas_set_num_threads(numThreads);
	
	if(outputFileName == NULL) {
		outputFileName = (char *) malloc(sizeof(char)*7);
		sprintf(outputFileName,"stdout");
	}
	
	start_timer();
	
	inputMatrixFile = (char *)malloc(sizeof(char)*strlen(argv[optind])+1);
	inputVectorFile = (char *)malloc(sizeof(char)*strlen(argv[optind+1])+1);
	
	strcpy(inputMatrixFile,argv[optind]);
	strcpy(inputVectorFile,argv[optind+1]);
	
	//Read matrix
	
	//Read matrix
	if(inputFormatRow){
	
		if(!readDenseCoordinateMatrixRowLine(inputMatrixFile,&II,&J,&A,&M,&N,&nz)){
			fprintf(stderr, "[%s] Can not read Matrix\n",__func__);
			return -1;
		}
		
		//writeDenseVector("stdout",values,M,N,nz);
		
	
	}
	else {
		if(!readDenseCoordinateMatrix(inputMatrixFile,&II,&J,&A,&M,&N,&nz)){
			fprintf(stderr, "[%s] Can not read Matrix\n",__func__);
			return -1;
		}
	}
	
	
	
	//Read vector
	if(!readDenseVector(inputVectorFile, &b,&M_Vector,&N_Vector,&nz_vector)){
		fprintf(stderr, "[%s] Can not read Vector\n",__func__);
		return -1;
	}
	
	
        
    //double *y=(double *) malloc(nz_vector * sizeof(double));
    fprintf(stderr,"[%s] Solving system using conjugate gradient method\n",__func__);
    double t_real = realtime();

	// Vector to store result
	double *x=(double *) calloc(nz_vector,sizeof(double));

	if(basicOps){
		ret_code = ConjugateGradientSolverBasic(II,J,A,M,N,nz,b,M_Vector,N_Vector,nz_vector, x, iterationNumber);
	}
	else{
		# ifdef ENABLE_RMT 
			  activateRMT("f-L-lp-lp-2d-l-l-l-1dC-l-l-l-1dC-i-i-ipC", &ConjugateGradientSolver, 14, II, J, A, M, N, M, N, nz , b, N, M_Vector, N_Vector, 
																	  nz_vector, x, nz_vector, iterationNumber, F, &ret_code);
															
		# else 			
			ret_code = ConjugateGradientSolver(II,J,A,M,N,nz,b,M_Vector,N_Vector,nz_vector, x, iterationNumber, F, &ret_code);
		# endif
		
	}
	
	fprintf(stderr, "\n[%s] Time spent in Conjugate Gradient: %.6f sec\n", __func__, realtime() - t_real);
	
	if(ret_code){
	
		writeDenseVector(outputFileName, x,M_Vector,N_Vector,nz_vector);
	
	}
	else{
		fprintf(stderr,"[%s] Error executing ConjugateGradientSolver\n",__func__);
		return -1;
	
	}
	
	end_timer();
	
	return 0;
}

