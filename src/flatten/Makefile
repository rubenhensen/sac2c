#
# $Log$
# Revision 1.12  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.11  1997/11/11 16:49:21  sbs
# make deps done
#
# Revision 1.10  1997/08/05 11:32:22  sbs
# make deps done
#
# Revision 1.9  1996/01/04 18:11:05  cg
# make deps done
#
# Revision 1.8  1995/12/15  10:41:06  cg
# make deps done
#
# Revision 1.7  1995/10/08  11:29:44  sbs
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

CC        := gcc
CCFLAGS   := -ansi -Wall -g
CFLAGS    := -D__EXTENSIONS__


INCS=-I../global -I../../inc

all: flatten.o

flatten.o: flatten.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o flatten.o -c flatten.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) flatten.o *.bak

# DO NOT DELETE THIS LINE -- make depend depends on it.

flatten.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
flatten.o: /usr/include/stdlib.h ../global/tree.h ../global/types.h
flatten.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
flatten.o: /usr/include/string.h ../global/globals.h ../global/internal_lib.h
flatten.o: ../../inc/dbug.h ../global/my_debug.h ../global/traverse.h
flatten.o: ../global/access_macros.h ../global/free.h /usr/include/malloc.h
