include ../../../Makefile.Config

INCS   = -I../../../inc
LIBDIR = ../../../lib
LIBS   = $(LIBDIR)/main_args.o

all: ../csd ../cse

../csd: csd.c
	$(CC) $(CCFLAGS) -o ../csd csd.c

../cse: cse.c
	$(CC) $(CCFLAGS) -o ../cse cse.c $(INCS) $(LIBS) 
