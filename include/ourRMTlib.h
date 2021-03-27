#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

# ifdef ENABLE_TMR
	#define NUM_OF_THREADS 3
# else  
	#define NUM_OF_THREADS 2
# endif

#define TRUE 1
#define FALSE 0


#define BILLION 1000000000L

//timing structures
struct timeval  tv1, tv2;
clock_t start_t, end_t, total_t;
struct timespec start, end;
long long unsigned int diff;

  
void *function_address; //RMT activated function name

void driver();

void activateRMT(const char* fmt, ...);


