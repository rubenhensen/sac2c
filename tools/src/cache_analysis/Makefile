
LIBDIR  = ../../../lib
INCDIR  = ../../../inc
LIBS    = $(LIBDIR)/main_args.o

CC      = gcc

gccFLAGS = -Wall -ansi
ccFLAGS = 
CCFLAGS = $($(CC)FLAGS) -g -I$(INCDIR)

all: ../ctest

verbose.o: verbose.c verbose.h
accesstime.o: ctest.h accesstime.h verbose.h accesstime.c 
ctest.o: verbose.h ctest.h accesstime.h l1.h l2.h ctest.c
l1.o: ctest.c l1.h ctest.h l1.c
l2.o: ctest.c l2.h ctest.h l2.c
bits.o: bits.c bits.h

%.o: %.c
	$(CC) $(CCFLAGS) -c $<

../ctest: accesstime.o ctest.o verbose.o l1.o l2.o bits.o
	$(CC) $(CCFLAGS) -o $@ $(LIBS) $^

clean:
	$(RM) *.o *~
	$(RM) ../ctest
