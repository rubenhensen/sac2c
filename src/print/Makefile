CC=gcc -ansi -g

INCS=-I../global -I../../inc

all: print.o

print.o: print.c
	$(CC) -o print.o -c print.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) print.o *.bak


# DO NOT DELETE THIS LINE -- make depend depends on it.

print.o: /usr/include/stdio.h ../global/tree.h ../global/node_info.mac
print.o: ../global/prf_node_info.mac ../global/my_debug.h ../../inc/dbug.h
print.o: ../global/traverse.h print.h
