#
# $Log$
# Revision 1.1  1995/06/02 10:09:27  sbs
# Initial revision
#
#
#

MAKE=make
CC=gcc -ansi -g

INCS=-I../global -I../../inc -I../optimize

all: index.o psi-opt.o

index.o: index.c
	$(CC) -o index.o -c index.c $(INCS)

psi-opt.o: psi-opt.c
	$(CC) -o psi-opt.o -c psi-opt.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) index.o psi-opt.o *.bak

# DO NOT DELETE THIS LINE -- make depend depends on it.

index.o: /usr/include/stdio.h ../../inc/dbug.h ../global/tree.h
index.o: ../global/type_info.mac ../global/node_info.mac
index.o: ../global/prf_node_info.mac ../global/traverse.h index.h
psi-opt.o: /usr/include/stdio.h /usr/include/stdlib.h
psi-opt.o: /usr/include/sys/stdtypes.h ../global/tree.h
psi-opt.o: ../global/type_info.mac ../global/node_info.mac
psi-opt.o: ../global/prf_node_info.mac ../global/Error.h ../../inc/dbug.h
psi-opt.o: ../global/traverse.h psi-opt.h ../optimize/optimize.h
