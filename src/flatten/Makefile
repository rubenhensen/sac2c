#
# $Log$
# Revision 1.4  1994/12/09 10:43:16  sbs
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
flatten.o: /usr/include/sys/stdtypes.h ../global/tree.h
flatten.o: ../global/node_info.mac ../global/prf_node_info.mac
flatten.o: ../global/Error.h ../../inc/dbug.h ../global/my_debug.h
flatten.o: ../global/traverse.h
