#
# $Log$
# Revision 1.7  1995/10/08 11:29:44  sbs
# make deps done
#
# Revision 1.6  1995/05/15  13:38:40  asi
# make deps done
#
# Revision 1.5  1995/04/04  11:03:44  sbs
# make deps done
#
# Revision 1.4  1994/12/09  10:43:16  sbs
# *** empty log message ***
#
# Revision 1.3  1994/11/10  15:39:42  sbs
# RCS-header inserted
#
#

MAKE=make
CC=gcc -ansi -g

INCS=-I../global -I../../inc

all: flatten.o

flatten.o: flatten.c
	$(CC) -o flatten.o -c flatten.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) flatten.o *.bak

# DO NOT DELETE THIS LINE -- make depend depends on it.

flatten.o: /usr/include/stdio.h /usr/include/stdlib.h
flatten.o: /usr/include/sys/stdtypes.h ../global/tree.h ../global/types.h
flatten.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
flatten.o: ../../inc/dbug.h ../global/my_debug.h ../global/traverse.h
flatten.o: ../global/internal_lib.h ../global/access_macros.h
