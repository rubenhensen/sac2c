#
# $Log$
# Revision 1.1  1995/10/16 12:07:18  cg
# Initial revision
#
#
#


MAKE=make
CC=gcc -ansi -g
RM=rm

INCS=-I../global -I../../inc 

all: objinit.o


objinit.o: objinit.c
	$(CC) -o objinit.o -c objinit.c $(INCS)



deps:
	makedepend $(INCS) *.c

clean:
	$(RM) objinit.o  *.bak




# DO NOT DELETE THIS LINE -- make depend depends on it.

objinit.o: /usr/include/string.h /usr/include/sys/stdtypes.h
objinit.o: /usr/include/malloc.h ../global/types.h ../global/tree_basic.h
objinit.o: ../global/tree_compound.h ../global/traverse.h ../../inc/dbug.h
objinit.o: /usr/include/stdio.h
