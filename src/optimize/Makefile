#
# $Log$
# Revision 1.1  1994/12/09 10:48:23  sbs
# Initial revision
#
#

MAKE=make
CC=gcc -ansi -g

INCS=-I../global -I../../inc

all: optimize.o

optimize.o: optimize.c
	$(CC) -o optimize.o -c optimize.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) optimize.o *.bak

# DO NOT DELETE THIS LINE -- make depend depends on it.

optimize.o: ../global/tree.h ../global/node_info.mac
optimize.o: ../global/prf_node_info.mac ../global/Error.h
optimize.o: /usr/include/stdio.h ../../inc/dbug.h ../global/my_debug.h
optimize.o: ../global/traverse.h optimize.h
