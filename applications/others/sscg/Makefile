RMT_OPTION := orig

CC=gcc
OUTPUT=SSCG_$(RMT_OPTION)
INCLUDE=../../../include

cflags.common=  -w -g -Wall
cflags.rmt= -DENABLE_RMT -DENABLE_TMR
cflags.orig=  
CFLAGS := ${cflags.common} ${cflags.${RMT_OPTION}}

LINKER_FLAGS=-O3 -o $(OUTPUT)  -lm  -lffcall -lpthread -lopenblas



build:ConjugateGradient.c ConjugateGradientSolver.c ConjugateGradientSolverBasic.c  mmio.c utils.c basic.c  $(INCLUDE)/ourRMTlib.c 
	$(CC)  $(CFLAGS) ConjugateGradient.c ConjugateGradientSolver.c ConjugateGradientSolverBasic.c  mmio.c utils.c basic.c  $(INCLUDE)/ourRMTlib.c   $(LINKER_FLAGS)

clean:
	rm -rf *.o SSCG_* *.txt



