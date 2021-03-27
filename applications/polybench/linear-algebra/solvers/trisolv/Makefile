RMT_OPTION := orig

CC=clang
OUTPUT=trisolv-$(RMT_OPTION)
INCLUDE=../../../../../include

cflags.common= -w -fno-use-cxa-atexit -DPOLYBENCH_DUMP_ARRAYS -DEXTRALARGE_DATASET 
cflags.rmt= -DENABLE_RMT -DENABLE_TMR 
cflags.orig=  
cflags.end= -I $(OUTPUT)
CFLAGS := ${cflags.common} ${cflags.${RMT_OPTION}} ${cflags.end}

LINKER_FLAGS=-lpthread -lm -lffcall -o $(OUTPUT)

build:trisolv.c  $(INCLUDE)/polybench.c  $(INCLUDE)/ourRMTlib.c 
	$(CC)  $(CFLAGS) trisolv.c  $(INCLUDE)/polybench.c   $(INCLUDE)/ourRMTlib.c $(LINKER_FLAGS)

clean:
	rm -rf *.o trisolv-*

