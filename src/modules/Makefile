#
# $Log$
# Revision 1.8  1995/10/05 16:04:54  cg
# implicittypes.c added
#
# Revision 1.7  1995/08/31  08:40:13  cg
# sib.c and sib.h added
#
# Revision 1.6  1995/08/30  14:02:17  cg
# now print.o depends on compare_macros.h
#
# Revision 1.5  1995/08/08  09:52:15  cg
# include of typecheck.h made possible.
#
# Revision 1.4  1994/12/31  14:31:32  sbs
# preliminary (working) version
#
# Revision 1.3  1994/12/21  12:22:07  sbs
# includes extended
#
# Revision 1.2  1994/12/16  14:33:39  sbs
# import.o inserted
#
# Revision 1.1  1994/12/11  17:33:27  sbs
# Initial revision
#
#
#

MAKE=make
CC=gcc -ansi -g

INCS=-I../global -I../../inc -I../scanparse -I../print

all: filemgr.o import.o sib.o implicittypes.o

filemgr.o: filemgr.c
	$(CC) -o filemgr.o -c filemgr.c $(INCS)

import.o: import.c
	$(CC) -o import.o -c import.c $(INCS)

sib.o: sib.c
	$(CC) -o sib.o -c sib.c $(INCS)

implicittypes.o: implicittypes.c
	$(CC) -o implicittypes.o -c implicittypes.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) filemgr.o import.o sib.o *.bak

# DO NOT DELETE THIS LINE -- make depend depends on it.

filemgr.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
filemgr.o: /usr/include/string.h ../../inc/dbug.h /usr/include/stdio.h
filemgr.o: filemgr.h
implicittypes.o: ../global/types.h ../global/tree_basic.h
implicittypes.o: ../global/tree_compound.h ../global/free.h ../global/Error.h
implicittypes.o: /usr/include/stdio.h ../../inc/dbug.h ../global/my_debug.h
import.o: /usr/include/malloc.h /usr/include/string.h
import.o: /usr/include/sys/stdtypes.h /usr/include/limits.h ../global/tree.h
import.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
import.o: ../global/free.h ../global/Error.h /usr/include/stdio.h
import.o: ../../inc/dbug.h ../global/my_debug.h ../scanparse/scnprs.h
import.o: ../scanparse/y.tab.h ../global/traverse.h ../global/access_macros.h
import.o: filemgr.h import.h
sib.o: /usr/include/malloc.h /usr/include/string.h
sib.o: /usr/include/sys/stdtypes.h ../global/types.h ../global/tree_basic.h
sib.o: ../global/tree_compound.h ../global/free.h ../global/Error.h
sib.o: /usr/include/stdio.h ../../inc/dbug.h ../global/my_debug.h
sib.o: ../scanparse/scnprs.h ../scanparse/y.tab.h ../global/traverse.h
sib.o: filemgr.h import.h ../print/print.h ../global/tree.h
sib.o: ../print/convert.h sib.h
