#
# $Log$
# Revision 1.2  1995/03/28 12:09:23  hw
# make deps done
#
# Revision 1.1  1995/03/09  16:17:01  hw
# Initial revision
#
# 


MAKE=make
CC=gcc -ansi -g

INCS=-I../global -I../../inc -I../typecheck 

all: refcount.o

refcount.o: refcount.c
	$(CC) -o refcount.o -c refcount.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) refcount.o *.bak

# DO NOT DELETE THIS LINE -- make depend depends on it.

refcount.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
refcount.o: ../global/tree.h ../global/type_info.mac ../global/node_info.mac
refcount.o: ../global/prf_node_info.mac ../global/my_debug.h ../../inc/dbug.h
refcount.o: /usr/include/stdio.h ../typecheck/typecheck.h
refcount.o: ../global/traverse.h ../global/internal_lib.h refcount.h
