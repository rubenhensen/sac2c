#
# $Log$
# Revision 1.2  1995/01/18 17:40:09  asi
# make deps
#
# Revision 1.1  1994/12/09  10:48:23  sbs
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

optimize.o: /usr/include/stdio.h /usr/include/stdlib.h
optimize.o: /usr/include/sys/stdtypes.h ../global/tree.h
optimize.o: ../global/type_info.mac ../global/node_info.mac
optimize.o: ../global/prf_node_info.mac ../global/free.h ../global/Error.h
optimize.o: ../../inc/dbug.h ../global/my_debug.h ../global/traverse.h
optimize.o: optimize.h
