
LIBDIR  = ../../../lib
INCDIR  = ../../../inc
LIBS    = $(LIBDIR)/main_args.o

CC      = gcc

gccFLAGS = -Wall -ansi
ccFLAGS = 
CCFLAGS = $($(CC)FLAGS) -g -I$(INCDIR)

all: ctest

verbose.o: verbose.c verbose.h
	$(CC) $(CCFLAGS) -c verbose.c

accesstime.o: ctest.h accesstime.h verbose.h accesstime.c 
	$(CC) $(CCFLAGS) -c accesstime.c

ctest.o: verbose.h ctest.h accesstime.h l1.h l2.h ctest.c
	$(CC) $(CCFLAGS) -c ctest.c

l1.o: ctest.c l1.h ctest.h l1.c
	$(CC) $(CCFLAGS) -c l1.c

l2.o: ctest.c l2.h ctest.h l2.c
	$(CC) $(CCFLAGS) -c l2.c

bits.o: bits.c bits.h
	$(CC) $(CCFLAGS) -c bits.c

ctest: accesstime.o ctest.o verbose.o l1.o l2.o bits.o
	$(CC) $(CCFLAGS) -o ctest $(LIBS) ctest.o accesstime.o verbose.o l1.o l2.o bits.o

clean:
	$(RM) verbose.o accesstime.o ctest.o l1.o l2.o bits.o ctest
