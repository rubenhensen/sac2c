#
# $Log$
# Revision 1.6  1998/04/20 01:09:36  dkr
# make deps done
#
# Revision 1.5  1998/04/17 17:24:45  dkr
# concregions.[ch] renamed to spmdregions.[ch]
#
# Revision 1.4  1998/04/09 12:04:04  dkr
# make deps done
#
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

all: spmdregions.o

spmdregions.o: spmdregions.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o spmdregions.o -c spmdregions.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
	$(RM) -r .sb

# DO NOT DELETE THIS LINE -- make depend depends on it.

spmdregions.o: ../global/types.h ../global/tree_basic.h
spmdregions.o: ../global/tree_compound.h ../global/Error.h
spmdregions.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
spmdregions.o: /usr/include/sys/va_list.h /usr/include/string.h
spmdregions.o: /usr/include/stdlib.h ../global/globals.h ../global/free.h
spmdregions.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
spmdregions.o: ../global/internal_lib.h ../global/traverse.h
spmdregions.o: ../optimize/optimize.h spmdregions.h
