#
# $Log$
# Revision 1.7  1997/08/05 11:32:30  sbs
# make deps done
#
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

refcount.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
refcount.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
refcount.o: ../global/tree_compound.h ../global/Error.h /usr/include/stdio.h
refcount.o: /usr/include/sys/va_list.h /usr/include/string.h
refcount.o: ../global/globals.h ../global/internal_lib.h ../global/my_debug.h
refcount.o: ../../inc/dbug.h ../typecheck/typecheck.h ../global/traverse.h
refcount.o: ../global/free.h /usr/include/malloc.h refcount.h
