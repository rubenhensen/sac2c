#
# $Log$
# Revision 1.6  1995/08/30 14:02:17  cg
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

INCS=-I../global -I../../inc -I../scanparse -I../typecheck

all: filemgr.o import.o

filemgr.o: filemgr.c
	$(CC) -o filemgr.o -c filemgr.c $(INCS)

import.o: import.c
	$(CC) -o import.o -c import.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) filemgr.o import.o *.bak

# DO NOT DELETE THIS LINE -- make depend depends on it.

filemgr.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
filemgr.o: /usr/include/string.h ../../inc/dbug.h /usr/include/stdio.h
filemgr.o: filemgr.h
import.o: /usr/include/malloc.h /usr/include/string.h
import.o: /usr/include/sys/stdtypes.h /usr/include/limits.h ../../inc/dbug.h
import.o: /usr/include/stdio.h ../global/tree.h ../global/type_info.mac
import.o: ../global/node_info.mac ../global/prf_node_info.mac
import.o: ../global/free.h ../global/Error.h ../global/my_debug.h
import.o: ../scanparse/scnprs.h ../scanparse/y.tab.h ../global/traverse.h
import.o: ../global/access_macros.h filemgr.h import.h compare_macros.h
sib.o: /usr/include/malloc.h /usr/include/string.h
sib.o: /usr/include/sys/stdtypes.h /usr/include/limits.h ../../inc/dbug.h
sib.o: /usr/include/stdio.h ../global/tree.h ../global/type_info.mac
sib.o: ../global/node_info.mac ../global/prf_node_info.mac ../global/free.h
sib.o: ../global/Error.h ../global/my_debug.h ../scanparse/scnprs.h
sib.o: ../scanparse/y.tab.h ../global/traverse.h ../global/access_macros.h
sib.o: filemgr.h import.h
