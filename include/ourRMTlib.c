#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <stdarg.h> 
#include <stdarg.h>
#include <avcall.h>

#include <sys/time.h>
#include <time.h>

#include "ourRMTlib.h"


#define DATA_INT 0
#define DATA_LONG_INT 1
#define DATA_FLOAT 2
#define DATA_DOUBLE 3
#define DATA_CHAR 4
#define DATA_POINTER_INT 5
#define DATA_POINTER_DOUBLE 6
#define DATA_POINTER_FLOAT 7
#define DATA_POINTER_LONG 8
#define DATA_POINTER_VOID 9
#define DATA_POINTER_CHAR 10


#define STRING_SIZE 20

struct arguments{ 
   	void **arg;
	int n_total;
   	int *lengths;
   	int *dTypes;
   	int *outCheck;
   	int *corrupted;
};

// struct pointer to keep function arguments for redundant threads!
struct arguments* func_args[NUM_OF_THREADS];

// keep a copy of function arguments inc ase of DMR-with-re-execution
struct arguments* orig_args;

//store the function name of the redundantly executed function
void *th_function_name;


// a function for obtaining string length
int string_length(char *s) {
   int c = 0;
   
   while(s[c] != '\0')
      c++;
     
   return c;
}


//This function is invoked from the application side to activate redundant execution.
void activateRMT(const char* fmt, ...){ 
	
	//create variable length struct
	func_args[0] = (struct argument *)malloc(sizeof(struct arguments));
	
	int index=0;
	
	va_list args;
    va_start(args, fmt);
 
    while (*fmt != '\0') {
		if (*fmt != '-') {
			if (*fmt == 'f') { //code for get length
				function_address = va_arg(args, void *);
			}
			if (*fmt == 'L') { //code for get length
				int i = va_arg(args, int);
				func_args[0]->n_total = i; 
				func_args[0]->arg = malloc(func_args[0]->n_total * sizeof(void *));
				func_args[0]->lengths = malloc(func_args[0]->n_total * sizeof(int));
				func_args[0]->dTypes = malloc(func_args[0]->n_total * sizeof(int));
				func_args[0]->outCheck = malloc(func_args[0]->n_total * sizeof(int));
				func_args[0]->corrupted = malloc(func_args[0]->n_total * sizeof(int));
			}
			else if (*fmt == 'i') { //code for get an integer
++fmt;
				if(*fmt == 'p') {
					int *c = va_arg(args, int *);
					func_args[0]->arg[index] = (void *)c;   //get variable
					func_args[0]->lengths[index]=1;			   //get variable length
					func_args[0]->dTypes[index]=DATA_POINTER_INT;	
					func_args[0]->corrupted[index]=FALSE;
				}	
				else{
					int i = va_arg(args, int);
					int *iptr = (int *)calloc(1, sizeof(int));
					memcpy(iptr, &i, sizeof(int));
					func_args[0]->arg[index] = (void *)iptr;   //get variable
					func_args[0]->lengths[index]=1;			   //get variable length
					func_args[0]->dTypes[index]=DATA_INT;	
					func_args[0]->corrupted[index]=FALSE;	
					--fmt;
				}			   		   
				++fmt;
				if(*fmt == 'C') {
					func_args[0]->outCheck[index]=TRUE;	
				}
				else{
					func_args[0]->outCheck[index]=FALSE;
					--fmt;
				}  
				index++;
			} else if (*fmt == 'd') {  //code for get a double						
				++fmt;
				if(*fmt == 'p') { //code for get a double pointer	
					double *c = va_arg(args, double *);
					func_args[0]->arg[index] = (void *)c;   //get variable
					func_args[0]->lengths[index]=1;			   //get variable length
					func_args[0]->dTypes[index]=DATA_POINTER_DOUBLE;	
					func_args[0]->corrupted[index]=FALSE;
				}	
				else{
					double c = va_arg(args, double);
					double *dptr = (double *)malloc(sizeof(double));
					memcpy(dptr, &c, sizeof(double));
					func_args[0]->arg[index] = (void *)dptr;   //get variable
					func_args[0]->lengths[index]=1;			   //get variable length
					func_args[0]->dTypes[index]=DATA_DOUBLE;	
					func_args[0]->corrupted[index]=FALSE;
					--fmt;	
				}  		   	
				++fmt;
				if(*fmt == 'C') {  // Check whether this variable compared at the end of redundant executions
					func_args[0]->outCheck[index]=TRUE;	
				}	
				else{
					func_args[0]->outCheck[index]=FALSE;
					--fmt;
				}  		   
				index++;
			} else if (*fmt == 'l') { //code for get a long
					++fmt;				
				if(*fmt == 'p') {
					long *c = va_arg(args, long *);
					func_args[0]->arg[index] = (void *)c;   //get variable
					func_args[0]->lengths[index]=1;			   //get variable length
					func_args[0]->dTypes[index]=DATA_POINTER_LONG;	
					func_args[0]->corrupted[index]=FALSE;
				}	
				else{
					long c = va_arg(args, long); //code for get a long
					long *lptr = (long *)calloc(1, sizeof(long));
					memcpy(lptr, &c, sizeof(long));
					func_args[0]->arg[index] = (void *)lptr;   //get variable
					func_args[0]->lengths[index]=1;			   //get variable length
					func_args[0]->dTypes[index]=DATA_LONG_INT;	
					func_args[0]->corrupted[index]=FALSE;
					--fmt;
				}		
				++fmt;
				if(*fmt == 'C') {
					func_args[0]->outCheck[index]=TRUE;	
				}		
				else {
					func_args[0]->outCheck[index]=FALSE;
					--fmt;
				}   
				index++;
			} else if (*fmt == 's') {  //code for get a string
				char* c = va_arg(args, char*);
				char cptr[STRING_SIZE];
				strcpy(cptr, c);
				func_args[0]->arg[index] = (void *)cptr;   
				func_args[0]->lengths[index] = string_length(c);	
				func_args[0]->corrupted[index]=FALSE;	
				func_args[0]->dTypes[index]=DATA_POINTER_CHAR;
				++fmt;
				if(*fmt == 'C') { // Check whether this variable compared at the end of redundant executions
					func_args[0]->outCheck[index]=TRUE;	
				}		
				else {
					func_args[0]->outCheck[index]=FALSE;
					--fmt;
				}   
				index++;	
			}else if (*fmt == '1') { //code for get a 1d array
				++fmt;
				if (*fmt == 'i') {  // code for get a 1d int-array
					int *d = va_arg(args, int *);
					int size = 0;
					size = va_arg(args, int);
					func_args[0]->arg[index] = (void *)d;   //get variable
					func_args[0]->lengths[index]=size;		 //get variable length
					func_args[0]->dTypes[index]=DATA_POINTER_INT;	
					func_args[0]->corrupted[index]=FALSE;		
					++fmt;
					if(*fmt == 'C') {
						func_args[0]->outCheck[index]=TRUE;	
					}	
					else{
						func_args[0]->outCheck[index]=FALSE;
						--fmt;
					}	   
					index++;							
				}
				else if (*fmt == 'd') {  // code for get a 1d double-array
					double *d = va_arg(args, double *);
					int size = 0;
					size = va_arg(args, int);
					func_args[0]->arg[index] = (void *)d;   //get variable
					func_args[0]->lengths[index]=size;		 //get variable length
					func_args[0]->dTypes[index]=DATA_POINTER_DOUBLE;	
					func_args[0]->corrupted[index]=FALSE;		
					++fmt;
					if(*fmt == 'C') {
						func_args[0]->outCheck[index]=TRUE;	
					}				
					else {
						func_args[0]->outCheck[index]=FALSE;
						--fmt;
					}	   
					index++;							
				}
				else if (*fmt == 'c') {
					char *d = va_arg(args, char *);
					int size = 0;
					size = va_arg(args, int);
			
					func_args[0]->arg[index] = (void *)d;   //get variable
					func_args[0]->lengths[index]=size;		 //get variable length
					func_args[0]->dTypes[index]=DATA_POINTER_CHAR;	
					func_args[0]->corrupted[index]=FALSE;		
					++fmt;
					if(*fmt == 'C') {
						func_args[0]->outCheck[index]=TRUE;	
					}	
					else{
						func_args[0]->outCheck[index]=FALSE;
						--fmt;
					}	   
					index++;							
				}						
			}
			else if (*fmt == '2') { //code for get a 2d array
				++fmt;
				if (*fmt == 'i') { // code for get a 2d int-array
					int *d = va_arg(args, int *);
					int row_size = 0; int column_size = 0;
					row_size = va_arg(args, int);
					column_size = va_arg(args, int);
					func_args[0]->arg[index] = (void *)d;   //get variable
					func_args[0]->lengths[index]=row_size*column_size;		 //get variable length
					func_args[0]->dTypes[index]=DATA_POINTER_INT;		
					func_args[0]->corrupted[index]=FALSE;			
					++fmt;
					if(*fmt == 'C') {
						func_args[0]->outCheck[index]=TRUE;	
					}			
					else {
						func_args[0]->outCheck[index]=FALSE;
						--fmt;
					}	   
					index++;		
				}
				else if (*fmt == 'd') {  // code for get a 2d double array
					double *d = va_arg(args, double *);
					int row_size = 0; int column_size = 0;
					row_size = va_arg(args, int);
					column_size = va_arg(args, int);
					func_args[0]->arg[index] = (void *)d;   //get variable
					func_args[0]->lengths[index]=row_size*column_size;		 //get variable length
					func_args[0]->dTypes[index]=DATA_POINTER_DOUBLE;	
					func_args[0]->corrupted[index]=FALSE;		
					++fmt;
					if(*fmt == 'C') {
						func_args[0]->outCheck[index]=TRUE;	
					}
					else{
						func_args[0]->outCheck[index]=FALSE;
						--fmt;
					}		   
					index++;
				}	
				else if (*fmt == 'f') {  // code for get a 2d float array
					float *f = va_arg(args, float *);
					int row_size = 0; int column_size = 0;
					row_size = va_arg(args, int);
					column_size = va_arg(args, int);						
					func_args[0]->arg[index] = (void *)f;   //get variable
					func_args[0]->lengths[index]=row_size*column_size;		 //get variable length
					func_args[0]->dTypes[index]=DATA_POINTER_FLOAT;		
					func_args[0]->corrupted[index]=FALSE;		
					++fmt;
					if(*fmt == 'C') {
						func_args[0]->outCheck[index]=TRUE;	
					}			
					else{
						func_args[0]->outCheck[index]=FALSE;
						--fmt;
					}		   
					index++;
				}			
			}
			else if (*fmt == '3') { //code for get a 3d array
				++fmt;
				if (*fmt == 'i') {	//code for get a 3d int-array
					int *d = va_arg(args, int *);
					int first_size = 0; int second_size = 0; int third_size = 0; 
					first_size = va_arg(args, int);
					second_size = va_arg(args, int);
					third_size = va_arg(args, int);	
					func_args[0]->arg[index] = (void *)d;   //get variable
					func_args[0]->lengths[index]=first_size*second_size*third_size;		 //get variable length
					func_args[0]->dTypes[index]=DATA_POINTER_INT;		
					func_args[0]->corrupted[index]=FALSE;			
					++fmt;
					if(*fmt == 'C') {
						func_args[0]->outCheck[index]=TRUE;	
					}			
					else {
						func_args[0]->outCheck[index]=FALSE;
						--fmt;
					}	   
					index++;		
				}
				else if (*fmt == 'd') {//code for get a 3d double array
					double *d = va_arg(args, double *);
					int first_size = 0; int second_size = 0; int third_size = 0; 
					first_size = va_arg(args, int);
					second_size = va_arg(args, int);
					third_size = va_arg(args, int);func_args[0]->arg[index] = (void *)d;   //get variable
					func_args[0]->lengths[index]=first_size*second_size*third_size;		 //get variable length
					func_args[0]->dTypes[index]=DATA_POINTER_DOUBLE;
					func_args[0]->corrupted[index]=FALSE;		
					++fmt;
					if(*fmt == 'C') {
						func_args[0]->outCheck[index]=TRUE;	
					}
					else{
						func_args[0]->outCheck[index]=FALSE;
						--fmt;
					}		   
					index++;
				}		
			}
			else if (*fmt == 'p') { //code for void pointer type
				void *d = va_arg(args, void *);
				++fmt;
				int size = 0;
				size = va_arg(args, int);
				
				func_args[0]->arg[index] = d;   //get variable
				func_args[0]->lengths[index]=size;		 //get variable length
				func_args[0]->dTypes[index]=DATA_POINTER_VOID;	
				func_args[0]->corrupted[index]=FALSE;		
				++fmt;
				if(*fmt == 'C') {
					func_args[0]->outCheck[index]=TRUE;	
				}				
				else {
					func_args[0]->outCheck[index]=FALSE;
					--fmt;
				}	   
				index++;							
			}
		}
        ++fmt;
    } 
    va_end(args);
    //printf("Total length found in handle_inputs is %d\n", index);
    
    //invoke driver!
    driver();
} 

/* This function inserts the arguments of redundant threads to invoke candidate function.
 * Since pthread_create takes a single argument. Thread arguments are arranged according to that.
 * */
void insert_arguments(av_alist* alist, struct arguments* myarg){
	int i;
	
	for(int i=0; i<myarg->n_total; i++){
		if(myarg->dTypes[i] == DATA_INT){
			int i1 = *((int *)(myarg->arg[i]));
			av_int(*alist, i1);
		}
		else if(myarg->dTypes[i] == DATA_DOUBLE){
			double d1 = *((double *)(myarg->arg[i]));
			av_double(*alist, d1);
		}
		else if(myarg->dTypes[i] == DATA_LONG_INT){
			long l1 = *((long *)(myarg->arg[i]));
			av_long(*alist, l1);
		}
		else if(myarg->dTypes[i] == DATA_FLOAT){
			float f1 = *((float *)(myarg->arg[i]));
			av_float(*alist, f1);
		}
		else if(myarg->dTypes[i] == DATA_CHAR){
			char c1 = *((char *)(myarg->arg[i]));
			av_char(*alist, c1);
		}
		else if(myarg->dTypes[i] == DATA_POINTER_INT){
			int *iptr = (int *)(myarg->arg[i]);
			av_ptr(*alist, int*, iptr);
		}
		else if(myarg->dTypes[i] == DATA_POINTER_DOUBLE){
			double *dptr = (double *)(myarg->arg[i]);
			av_ptr(*alist, double*, dptr);
		}
		else if(myarg->dTypes[i] == DATA_POINTER_FLOAT){
			float *fptr = (float *)(myarg->arg[i]);
			av_ptr(*alist, float*, fptr);
		}		
		else if(myarg->dTypes[i] == DATA_POINTER_LONG){
			long *lptr = (long *)(myarg->arg[i]);
			av_ptr(*alist, long*, lptr);
		}	
		else if(myarg->dTypes[i] == DATA_POINTER_CHAR){
			char *cptr = (char *)(myarg->arg[i]);
			av_ptr(*alist, char*, cptr);
		}
		else if(myarg->dTypes[i] == DATA_POINTER_VOID){
			av_ptr(*alist, void*, (myarg->arg[i]));
		}
	}
}

//This function creates the arguments of redundant threads
void create_arguments(struct arguments* dest_arg, struct arguments* source_arg, int tid){
	int i;
		
	dest_arg->n_total = source_arg->n_total ; 
	dest_arg->arg = malloc(dest_arg->n_total * sizeof(void *));
	dest_arg->lengths = malloc(dest_arg->n_total * sizeof(int));
	dest_arg->dTypes = malloc(dest_arg->n_total * sizeof(int));
	dest_arg->outCheck = malloc(dest_arg->n_total * sizeof(int));
	dest_arg->corrupted = malloc(dest_arg->n_total * sizeof(int));
	
	for(int i=0; i<source_arg->n_total; i++){
		if(source_arg->dTypes[i] == DATA_INT){
			int *iptr = (int *)malloc(sizeof(int));
            memcpy(iptr, source_arg->arg[i], sizeof(int));
            dest_arg->arg[i] = (void *)iptr;   //get variable
            dest_arg->lengths[i]=1;			   //get variable length			
            dest_arg->dTypes[i]=DATA_INT;				
            dest_arg->outCheck[i]=source_arg->outCheck[i];	
            dest_arg->corrupted[i]=FALSE;				
		}
		else if(source_arg->dTypes[i] == DATA_DOUBLE){
			double *dptr = (double *)malloc(sizeof(double));
            memcpy(dptr, source_arg->arg[i], sizeof(double));
            dest_arg->arg[i] = (void *)dptr;   //get variable
            dest_arg->lengths[i]=1;			   //get variable length
            dest_arg->dTypes[i]=DATA_DOUBLE;	
            dest_arg->outCheck[i]=source_arg->outCheck[i];	
            dest_arg->corrupted[i]=FALSE;				
		}
		else if(source_arg->dTypes[i] == DATA_LONG_INT){
			long *lptr = (long *)malloc(sizeof(long));
            memcpy(lptr, source_arg->arg[i], sizeof(long));
            dest_arg->arg[i] = (void *)lptr;   //get variable
            dest_arg->lengths[i]=1;			   //get variable length
            dest_arg->dTypes[i]=DATA_LONG_INT;	
            dest_arg->outCheck[i]=source_arg->outCheck[i];
            dest_arg->corrupted[i]=FALSE;					
		}
		else if(source_arg->dTypes[i] == DATA_FLOAT){
			float *fptr = (float *)malloc(sizeof(float));
            memcpy(fptr, source_arg->arg[i], sizeof(float));
            dest_arg->arg[i] = (void *)fptr;   //get variable
            dest_arg->lengths[i]=1;			   //get variable length
            dest_arg->dTypes[i]=DATA_FLOAT;	
            dest_arg->outCheck[i]=source_arg->outCheck[i];	
            dest_arg->corrupted[i]=FALSE;				
		}
		else if(source_arg->dTypes[i] == DATA_CHAR){
			char *cptr = (char *)malloc(sizeof(char));
            memcpy(cptr, source_arg->arg[i], sizeof(char));
            dest_arg->arg[i] = (void *)cptr;   //get variable
            dest_arg->lengths[i]=1;			   //get variable length
            dest_arg->dTypes[i]=DATA_CHAR;	
            dest_arg->outCheck[i]=source_arg->outCheck[i];	
            dest_arg->corrupted[i]=FALSE;				
		}
		else if(source_arg->dTypes[i] == DATA_POINTER_INT){
			int size = source_arg->lengths[i];
			int *d1 = (int(*)[size])malloc(size * sizeof(int));
			memcpy(d1, source_arg->arg[i], (size * sizeof(int)));
            dest_arg->arg[i] = (void *)d1;   //get variable
            dest_arg->lengths[i]=size;			   //get variable length
            dest_arg->dTypes[i]=DATA_POINTER_INT;	
            dest_arg->outCheck[i]=source_arg->outCheck[i];	
            dest_arg->corrupted[i]=FALSE;				
		}
		else if(source_arg->dTypes[i] == DATA_POINTER_DOUBLE){ 
			int size = source_arg->lengths[i];
			double *d1 = (double(*)[size])malloc(size * sizeof(double));
			memcpy(d1, source_arg->arg[i], (size * sizeof(double)));
			dest_arg->arg[i] = (void *)d1;   //get variable
            dest_arg->lengths[i]=size;			   //get variable length
            dest_arg->dTypes[i]=DATA_POINTER_DOUBLE;	
            dest_arg->outCheck[i]=source_arg->outCheck[i];	
            dest_arg->corrupted[i]=FALSE;				
		}
		else if(source_arg->dTypes[i] == DATA_POINTER_FLOAT){
			int size = source_arg->lengths[i];
			float *d1 = (float(*)[size])malloc(size * sizeof(float));
			memcpy(d1, source_arg->arg[i], (size * sizeof(float)));
            dest_arg->arg[i] = (void *)d1;   //get variable
            dest_arg->lengths[i]=size;			   //get variable length
            dest_arg->dTypes[i]=DATA_POINTER_FLOAT;	
            dest_arg->outCheck[i]=source_arg->outCheck[i];	
            dest_arg->corrupted[i]=FALSE;				
		}
		else if(source_arg->dTypes[i] == DATA_POINTER_LONG){
			int size = source_arg->lengths[i];
			long *d1 = (long(*)[size])malloc(size * sizeof(long));
			memcpy(d1, source_arg->arg[i], (size * sizeof(long)));
            dest_arg->arg[i] = (void *)d1;   //get variable
            dest_arg->lengths[i]=size;			   //get variable length
            dest_arg->dTypes[i]=DATA_POINTER_LONG;	
            dest_arg->outCheck[i]=source_arg->outCheck[i];	
            dest_arg->corrupted[i]=FALSE;				
		}
		else if(source_arg->dTypes[i] == DATA_POINTER_VOID){
			int size = source_arg->lengths[i];
			void *d1 = malloc(size * sizeof(void));
			memcpy(d1, source_arg->arg[i], (size * sizeof(void)));
            dest_arg->arg[i] = (void *)d1;   //get variable
            dest_arg->lengths[i]=size;			   //get variable length
            dest_arg->dTypes[i]=DATA_POINTER_VOID;
            dest_arg->outCheck[i]=source_arg->outCheck[i];	
            dest_arg->corrupted[i]=FALSE;					
		}
		else if(source_arg->dTypes[i] == DATA_POINTER_CHAR){
			int size = source_arg->lengths[i];
			char *d1 = (char(*)[size])malloc(size * sizeof(char));
			memcpy(d1, source_arg->arg[i], (size * sizeof(char)));
            dest_arg->arg[i] = (void *)d1;   //get variable
            dest_arg->lengths[i]=size;			   //get variable length
            dest_arg->dTypes[i]=DATA_POINTER_CHAR;
            dest_arg->outCheck[i]=source_arg->outCheck[i];	
            dest_arg->corrupted[i]=FALSE;					
		}
	}
	
}

/* if there is a mismatch on thread outputs, this function copies the correct one
 * from source to destination.
*/
void copy_arguments(struct arguments* dest_arg, struct arguments* source_arg){
	int i;
	
	
	for(int i=0; i<source_arg->n_total; i++){
		if(dest_arg->corrupted[i] == TRUE){
			if(source_arg->dTypes[i] == DATA_INT){  
				memcpy(dest_arg->arg[i], source_arg->arg[i], sizeof(int));
				//fprintf(stderr, "int copied!\n");
			}
			else if(source_arg->dTypes[i] == DATA_DOUBLE){
				memcpy(dest_arg->arg[i], source_arg->arg[i], sizeof(double));
				//fprintf(stderr, "double copied!\n");
			}
			else if(source_arg->dTypes[i] == DATA_LONG_INT){
				memcpy(dest_arg->arg[i], source_arg->arg[i], sizeof(long));
				//fprintf(stderr, "long copied!\n");
			}
			else if(source_arg->dTypes[i] == DATA_FLOAT){
				memcpy(dest_arg->arg[i], source_arg->arg[i], sizeof(float));
				//fprintf(stderr, "float copied!\n");
			}
			else if(source_arg->dTypes[i] == DATA_CHAR){
				memcpy(dest_arg->arg[i], source_arg->arg[i], sizeof(char));
				//fprintf(stderr, "char copied!\n");
			}
			else if(source_arg->dTypes[i] == DATA_POINTER_INT){
				int size = source_arg->lengths[i];
				memcpy(dest_arg->arg[i], source_arg->arg[i], (size * sizeof(int)));
				//fprintf(stderr, "DATA_POINTER_INT copied!\n");
			}
			else if(source_arg->dTypes[i] == DATA_POINTER_DOUBLE){
				int size = source_arg->lengths[i];
				memcpy(dest_arg->arg[i], source_arg->arg[i], (size * sizeof(double)));
				//fprintf(stderr, "3.DATA_POINTER_DOUBLE copied! i:%d\n", i);
			}
			else if(source_arg->dTypes[i] == DATA_POINTER_FLOAT){
				int size = source_arg->lengths[i];
				memcpy(dest_arg->arg[i], source_arg->arg[i], (size * sizeof(float)));
				//fprintf(stderr, "DATA_POINTER_FLOAT copied!\n");
			}
			else if(source_arg->dTypes[i] == DATA_POINTER_LONG){
				int size = source_arg->lengths[i];
				memcpy(dest_arg->arg[i], source_arg->arg[i], (size * sizeof(long)));
				//fprintf(stderr, "DATA_POINTER_LONG copied!\n");
			}
			else if(source_arg->dTypes[i] == DATA_POINTER_VOID){
				int size = source_arg->lengths[i];
				memcpy(dest_arg->arg[i], source_arg->arg[i], (size * sizeof(void)));	
				//fprintf(stderr, "DATA_POINTER_VOID copied!\n");
			}
			else if(source_arg->dTypes[i] == DATA_POINTER_CHAR){
				int size = source_arg->lengths[i];
				memcpy(dest_arg->arg[i], source_arg->arg[i], (size * sizeof(char)));	
				//fprintf(stderr, "DATA_POINTER_CHAR copied!\n");
			}
		}			
	}	
	//fprintf(stderr, "copy_arguments is over!\n");	
}

//this function arranges the thread arguments and invoke the candidate
void *start_thread(void *thread_arg){
	struct arguments *data;            
    data = (struct arguments *) thread_arg;  /* type cast to a pointer to thdata */
    	
    av_alist alist;	
	av_start_void(alist, function_address);		
	insert_arguments(&alist, data);	
	av_call (alist);
}

//this function compates the arguments of the two threads after redundant execution
int compare_outputs_2t(struct arguments* newarg, struct arguments* myarg){ 
	int flag = FALSE;
	
	for(int i=0; i<myarg->n_total; i++){
		if(myarg->outCheck[i] == TRUE){
			if(myarg->dTypes[i] == DATA_INT){			
				int n = memcmp (newarg->arg[i], myarg->arg[i], sizeof(int)); 
				if(n!=0){ 
					fprintf(stderr, "ints are different\n");
					newarg->corrupted[i] = TRUE;
					flag = TRUE;
				}					
			}
			else if(myarg->dTypes[i] == DATA_DOUBLE){
				int n = memcmp (newarg->arg[i], myarg->arg[i], sizeof(double)); 
				if(n!=0){ 
					fprintf(stderr, "doubles are different\n");
					newarg->corrupted[i] = TRUE;
					flag = TRUE;
				}			
			}
			else if(myarg->dTypes[i] == DATA_LONG_INT){
				int n = memcmp (newarg->arg[i], myarg->arg[i], sizeof(long)); 
				if(n!=0){ 
					fprintf(stderr, "longs are different\n");	
					newarg->corrupted[i] = TRUE;
					flag = TRUE;
				}			
			}
			else if(myarg->dTypes[i] == DATA_FLOAT){
				int n = memcmp (newarg->arg[i], myarg->arg[i], sizeof(float)); 
				if(n!=0){ 
					fprintf(stderr, "floats are different\n");
					newarg->corrupted[i] = TRUE;
					flag = TRUE;
				}			
			}
			else if(myarg->dTypes[i] == DATA_CHAR){
				int n = memcmp (newarg->arg[i], myarg->arg[i], sizeof(char)); 
				if(n!=0) {
					fprintf(stderr, "chars are different\n");
					newarg->corrupted[i] = TRUE;
					flag = TRUE;
				}			
			}
			else if(myarg->dTypes[i] == DATA_POINTER_INT){
				int size = myarg->lengths[i];			
				int n = memcmp (newarg->arg[i], myarg->arg[i], (size * sizeof(int))); 
				if(n!=0) {
					fprintf(stderr, "DATA_POINTER_INTs are different\n");
					newarg->corrupted[i] = TRUE;
					flag = TRUE;
				}			
			}
			else if(myarg->dTypes[i] == DATA_POINTER_DOUBLE){ 
				int size = myarg->lengths[i];
				int n = memcmp (newarg->arg[i], myarg->arg[i], (size * sizeof(double))); 
				if(n!=0) {
					fprintf(stderr, "DATA_POINTER_DOUBLEs are different i:%d\n", i);
					newarg->corrupted[i] = TRUE;
					flag = TRUE;
				}						
			}
			else if(myarg->dTypes[i] == DATA_POINTER_FLOAT){
				int size = myarg->lengths[i];
				int n = memcmp (newarg->arg[i], myarg->arg[i], (size * sizeof(float))); 
				if(n!=0) {
					fprintf(stderr, "DATA_POINTER_FLOATs are different\n");
					newarg->corrupted[i] = TRUE;
					flag = TRUE;
				}			
			}
			else if(myarg->dTypes[i] == DATA_POINTER_LONG){
				int size = myarg->lengths[i];
				int n = memcmp (newarg->arg[i], myarg->arg[i], (size * sizeof(long))); 
				if(n!=0) {
					fprintf(stderr, "DATA_POINTER_LONGs are different\n");
					newarg->corrupted[i] = TRUE;
					flag = TRUE;
				}			
			}
			else if(myarg->dTypes[i] == DATA_POINTER_VOID){
				int size = myarg->lengths[i];
				int n = memcmp (newarg->arg[i], myarg->arg[i], (size * sizeof(void))); 
				if(n!=0) {
					fprintf(stderr, "DATA_POINTER_VOIDs are different\n");
					newarg->corrupted[i] = TRUE;	
					flag = TRUE;
				}			
			}
			else if(myarg->dTypes[i] == DATA_POINTER_CHAR){
				int size = myarg->lengths[i];
				int n = memcmp (newarg->arg[i], myarg->arg[i], (size * sizeof(char))); 
				if(n!=0) {
					fprintf(stderr, "DATA_POINTER_CHARs are different\n");
					newarg->corrupted[i] = TRUE;	
					flag = TRUE;
				}			
			}
		}	
	}
	if(flag)
		return 0;
	return 1;
} 

// This function checks the output of 3 threads in case of TMR
void compare_outputs_TMR(){
	
   int result[3]={0, 0, 0}; 
  
   result[0] = compare_outputs_2t(func_args[0], func_args[1]); // compare the outputs of Threads t0 and t1
   result[1] = compare_outputs_2t(func_args[0], func_args[2]); // compare the outputs of Threads t0 and t2
   result[2] = compare_outputs_2t(func_args[1], func_args[2]); // compare the outputs of Threads t1 and t2
	
	if(result[0] || result[1] || result[2]){
    	if (!result[0] && !result[1]){ // true if threads t1 and t2 are equal, but t0 is different.
			// Thread t0's argument is corrupted! Copy the correct output from T1
			// Since Thread t0 is the main thread, copy_arguments will be only invoked in case of t0's output is different!
			copy_arguments(func_args[0], func_args[1]); 
		}
   }
   else{
  	 fprintf(stderr,"Mismatch for all cases compare_outputs_2t! Program exiting..\n");
	 exit(-1);
   }
}

/* This function checks the output of 2 threads in case of DMR, 
 * and re-executes another thread in case of output mismatch of DMR
 * */
void compare_outputs_DMR(){
	
   int result[3]; 
   pthread_t TID;
   result[0] = compare_outputs_2t(func_args[0], func_args[1]);
	
	if(!result[0]){ // if there is a mismatch execute a third thread now!
		if(pthread_create(&TID, NULL, start_thread, orig_args)) {
				fprintf(stderr, "error creating thread\n");
				exit(1);
	   }
	   if(pthread_join(TID, NULL)) {
			fprintf(stderr, "error joining thread\n");
			exit(2);

	  }
	   result[1] = compare_outputs_2t(func_args[0], orig_args); //compare the output of newly created thread with previous threads
	   result[2] = compare_outputs_2t(func_args[1], orig_args);
	   if(result[0] || result[1] || result[2]){
			if (!result[0] && !result[1]){
				// Thread t0's argument is corrupted! Copy the correct output from T1
				copy_arguments(func_args[0], func_args[1]); 
			}
	   }
	   else{
		 fprintf(stderr,"Mismatch for all cases compare_outputs_2t! Program exiting..\n");
		 exit(-1);
	   }
	  
   }
}

//These functions related with timing measurements.
void start_timer(){
  gettimeofday(&tv1, NULL);
  start_t = clock();
  clock_gettime(CLOCK_MONOTONIC, &start);	/* mark start time */
}

void end_timer(){
  clock_gettime(CLOCK_MONOTONIC, &end);	/* mark the end time */
  end_t = clock();
  gettimeofday(&tv2, NULL);
  
  fprintf(stderr, "Total time = %f seconds\n",
         (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec));
         
  total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;
  fprintf(stderr, "Total time taken by CPU: %f\n", total_t );
   
  diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
  fprintf(stderr,"elapsed time with clock_gettime= %llu nanoseconds\n", (long long unsigned int) diff);

}

void driver(){
		
	pthread_t TIDs[NUM_OF_THREADS];
	int i;
  
# ifndef ENABLE_TMR
    orig_args = (struct argument *)malloc(sizeof(struct arguments));
    create_arguments(orig_args, func_args[0], 0);
# endif
    
	for(i=1; i<NUM_OF_THREADS; i++){
		func_args[i] = (struct argument *)malloc(sizeof(struct arguments));
		create_arguments(func_args[i], func_args[0], i);
    }
	
	for(i=0; i<NUM_OF_THREADS; i++){	  
	  th_function_name = &start_thread ;	
		
	  // Create the threads here!
	  if(pthread_create(&TIDs[i], NULL, th_function_name, func_args[i])) {
				fprintf(stderr, "error creating thread\n");
				exit(1);

	  }
	}
	
	for(i=0; i<NUM_OF_THREADS; i++){
	  if(pthread_join(TIDs[i], NULL)) {
			fprintf(stderr, "error joining thread\n");
			exit(2);

	  }
	}
  
# ifdef ENABLE_TMR
    compare_outputs_TMR();
# else  
	compare_outputs_DMR();
# endif
    
}
