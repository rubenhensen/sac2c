#
# $Log$
# Revision 1.1  1994/12/11 17:33:27  sbs
# Initial revision
#
#
#

MAKE=make
CC=gcc -ansi -g

INCS=-I../global -I../../inc

all: filemgr.o

filemgr.o: filemgr.c
	$(CC) -o filemgr.o -c filemgr.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) filemgr.o *.bak

# DO NOT DELETE THIS LINE -- make depend depends on it.

filemgr.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
filemgr.o: /usr/include/string.h ../../inc/dbug.h /usr/include/stdio.h
filemgr.o: filemgr.h
prg.o: ../../inc/main.h ../../inc/dbug.h /usr/include/stdio.h filemgr.h
