#
# $Log$
# Revision 1.3  1998/04/08 22:11:16  dkr
# expanded INCS
#
# Revision 1.2  1998/04/03 11:52:52  srs
# made deps
#
# Revision 1.1  1998/04/03 11:47:26  dkr
# Initial revision
#
# Revision 1.42  1998/04/02 16:07:31  dkr
#
#
# 


CC        := gcc
CCFLAGS   := -ansi -Wall -g
CFLAGS    := -D__EXTENSIONS__

INCS=-I../global -I../../inc -I../optimize -I../typecheck

all: concregions.o

concregions.o: concregions.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o concregions.o -c concregions.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
	$(RM) -r .sb

# DO NOT DELETE THIS LINE -- make depend depends on it.

concregions.o: concregions.h ../global/types.h ../global/tree_basic.h
concregions.o: ../global/tree_compound.h ../global/Error.h
concregions.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
concregions.o: /usr/include/sys/va_list.h /usr/include/string.h
concregions.o: /usr/include/stdlib.h ../global/globals.h ../global/free.h
concregions.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
concregions.o: ../global/internal_lib.h ../global/traverse.h
concregions.o: ../optimize/DupTree.h
