RMT_OPTION := orig

CC=gcc
OUTPUT=dgemm-$(RMT_OPTION)
INCLUDE=../../../include

cflags.common= -w 
cflags.rmt= -DENABLE_RMT -DENABLE_TMR 
cflags.orig=  
cflags.end= -I $(OUTPUT)
CFLAGS := ${cflags.common} ${cflags.${RMT_OPTION}} ${cflags.end}

LINKER_FLAGS=-O3 -fopenmp -lpthread -lm -lffcall -o $(OUTPUT)

build:dgemm.c   $(INCLUDE)/ourRMTlib.c 
	$(CC)  $(CFLAGS) dgemm.c  $(INCLUDE)/ourRMTlib.c $(LINKER_FLAGS)

clean:
	rm -rf *.o dgemm-*



