RMT_OPTION := orig

CC=clang
OUTPUT=jacobi-2d-$(RMT_OPTION)
INCLUDE=../../../../include

cflags.common= -w -fno-use-cxa-atexit -DPOLYBENCH_DUMP_ARRAYS -DEXTRALARGE_DATASET 
cflags.rmt= -DENABLE_RMT -DENABLE_TMR 
cflags.orig=  
cflags.end= -I $(OUTPUT)
CFLAGS := ${cflags.common} ${cflags.${RMT_OPTION}} ${cflags.end}

LINKER_FLAGS=-lpthread -lm -lffcall -o $(OUTPUT)

build:jacobi-2d.c  $(INCLUDE)/polybench.c  $(INCLUDE)/ourRMTlib.c 
	$(CC)  $(CFLAGS) jacobi-2d.c  $(INCLUDE)/polybench.c   $(INCLUDE)/ourRMTlib.c $(LINKER_FLAGS)

clean:
	rm -rf *.o jacobi-2d-*

