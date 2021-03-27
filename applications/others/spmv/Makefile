RMT_OPTION := orig

CC=gcc
OUTPUT=csr_spmv_$(RMT_OPTION)
INCLUDE=../../../include

CFLAGS += -w -g -Wall
ifndef NO_OPENMP
CFLAGS += -fopenmp
endif


cflags.rmt= -DENABLE_RMT -DENABLE_TMR
cflags.orig=  
CFLAGS += ${cflags.${RMT_OPTION}}

LDFLAGS += -lffcall


build:csr_spmv.c mmio.c mmio.h  $(INCLUDE)/ourRMTlib.c  $(INCLUDE)/ourRMTlib.h
	$(CC)  $(CFLAGS) csr_spmv.c mmio.c  $(INCLUDE)/ourRMTlib.c $(LDFLAGS) -o $(OUTPUT)

clean:
	rm -rf *.o csr_spmv_*

