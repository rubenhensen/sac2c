#
# $Log$
# Revision 1.1  1997/11/25 13:44:09  dkr
# Initial revision
#
#
#

CC        := gcc
CCFLAGS   := -ansi -Wall -g
CFLAGS    := -D__EXTENSIONS__


INCS=

all: ../dbug.o

../dbug.o: dbug.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o ../dbug.o -c dbug.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) ../dbug.o *.bak

