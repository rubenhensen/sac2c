#
# $Log$
# Revision 1.3  1994/12/21 12:22:07  sbs
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

INCS=-I../global -I../../inc -I../scanparse

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
import.o: ../global/tree.h ../global/node_info.mac
import.o: ../global/prf_node_info.mac ../global/Error.h /usr/include/stdio.h
import.o: ../../inc/dbug.h ../global/my_debug.h ../global/traverse.h import.h
