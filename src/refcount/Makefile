#
# $Log$
# Revision 1.6  1996/02/16 09:46:55  sbs
# make deps done
#
# Revision 1.5  1996/01/04  18:11:11  cg
# make deps done
#
# Revision 1.4  1995/12/15  10:41:12  cg
# make deps done
#
# Revision 1.3  1995/10/08  11:29:58  sbs
# make deps done
#
# Revision 1.2  1995/03/28  12:09:23  hw
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
refcount.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
refcount.o: ../global/tree_compound.h ../global/Error.h /usr/include/stdio.h
refcount.o: /usr/include/string.h ../global/globals.h
refcount.o: ../global/internal_lib.h ../global/my_debug.h ../../inc/dbug.h
refcount.o: ../typecheck/typecheck.h ../global/traverse.h ../global/free.h
refcount.o: /usr/include/malloc.h refcount.h
