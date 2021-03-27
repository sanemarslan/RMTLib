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

#include <cblas.h>
#include "ConjugateGradientSolver.h"
#include <time.h>

/* Include redundant execution header. */
#include "../../../include/ourRMTlib.h"



int ipow(int base, int exp)
{
    int result = 1;
    for (;;)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }

    return result;
}


int ConjugateGradientSolver(unsigned long *II, unsigned long *J, double *A, unsigned long M, unsigned long N, 
	unsigned long long nz, double *b, unsigned long M_Vector, unsigned long N_Vector, unsigned long long nz_vector, 
	double *x, int iterationNumber, int F, int *ret_code) {
	
	//A*x=b

        double *Ap=(double *) malloc(nz_vector * sizeof(double));
        double *r=(double *) malloc(nz_vector * sizeof(double));
        double *p=(double *) malloc(nz_vector * sizeof(double));
        
        
	//double *x=(double *) calloc(nz_vector,sizeof(double));

	//r = b-A*x
	//If we take x=0 the init multiplication is avoided and r=b
	
	memcpy(r, b, N*sizeof(double));
	
	
	//p=r

	memcpy(p, r, N*sizeof(double));
	
	
	
	//rsold = r'*r
	double rsold = cblas_ddot(N,r,1,r,1);
	double rs_0 = cblas_ddot(N,r,1,r,1);
	
	int stop = 0;
		
	double alphaCG = 0.0;
		
	double rsnew = 0.0, rsnew2 = 0.0;
	unsigned long k = 0, selfCG_k=0;
	
	unsigned long maxIterations = M*2;
	
	if(iterationNumber != 0 ){
		maxIterations = iterationNumber;
	}

	int flag=0;
	
	while(!stop){
		
		//if( ((k+1) % F == 0) ){ //execute CG reliably
		//if(k == pow((double)F, (double)(selfCG_k+1))){
		if(k == ipow(F, (selfCG_k+1))){
				//fprintf(stderr, "F:%d k:%lu selfCG_k:%lu \n",F,k, selfCG_k);
								
				//Ap=A*p
				cblas_dgemv(CblasRowMajor, CblasNoTrans, M,N , 1.0, A, N, p, 1, 0.0, Ap, 1);
				
				//r=A*x
				cblas_dgemv(CblasRowMajor, CblasNoTrans, M,N , 1.0, A, N, x, 1, 0.0, r, 1);
				
				//r=b-r	
				cblas_dscal(N, -1, r, 1); //r = -r
				cblas_daxpy(N, 1, b, 1, r, 1); //r = r + b
				
				
				//alphaCG=r'*p /(p'*Ap)
				alphaCG = cblas_ddot(N,r,1,p,1)/cblas_ddot(N,p,1,Ap,1);
				
				//x=x+alphaCG*p
				cblas_daxpy(N,alphaCG,p,1,x,1);
				
				//r=r-alphaCG*Ap
				cblas_daxpy(N,-alphaCG,Ap,1,r,1);
				
				//rsnew = r'*r
				rsnew = cblas_ddot(N,r,1,r,1);
				
				//fprintf(stderr, "k:%ld F_iteration:%ld alphaCG:%.10f error:%.10f\n",k,selfCG_k,alphaCG, sqrt(rsnew));
				
				// p=r+((-r'*Ap/(p'*Ap))*p) 
				cblas_dscal(N, -(cblas_ddot(N,r,1,Ap,1)/cblas_ddot(N,p,1,Ap,1)), p, 1);
				cblas_daxpy(N,1.0,r,1,p,1);
								
				selfCG_k++;
				flag=0;
				
		}else{
	
				//Ap=A*p
		
				cblas_dgemv(CblasRowMajor, CblasNoTrans, M,N , 1.0, A, N, p, 1, 0.0, Ap, 1);
			

				//alphaCG=rsold/(p'*Ap)
				alphaCG = rsold/cblas_ddot(N,p,1,Ap,1);
				
				//x=x+alphaCG*p
				cblas_daxpy(N,alphaCG,p,1,x,1);

				//r=r-alphaCG*Ap
				cblas_daxpy(N,-alphaCG,Ap,1,r,1);
			
				//rsnew = r'*r
				rsnew = cblas_ddot(N,r,1,r,1);
				//fprintf(stderr, "k:%ld  alphaCG:%.10f error:%.10f\n",k,alphaCG, sqrt(rsnew));
				
				//p=r+rsnew/rsold*p
				cblas_dscal(N, rsnew/rsold, p, 1);
				cblas_daxpy(N,1.0,r,1,p,1);
				
				rsold = rsnew;
				
				if(isnan(alphaCG)){ //checking whether alphaCG is nan!
					fprintf(stderr, "!!! alphaCG:%.10f\n", alphaCG);
					exit(-1);
				}
		}
		k++;
		
		//if((sqrt(rsnew)<=EPSILON)||(k == maxIterations)){
		if(sqrt(rsnew)/sqrt(rs_0)<=EPSILON){
			stop = 1;
			fprintf(stderr, "STOPPED by CG\n");
		}
				
	}
	
	//memcpy(b, x, N*sizeof(double));

        free(Ap);
        free(r);
        free(p);
	//free(x);

	fprintf(stderr, "[%s] Number of total iterations: %lu Number of iterations in SS step:%lu \n",__func__,k, selfCG_k);

    *ret_code=1;
	return 1;
}



