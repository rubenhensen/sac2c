#
# $Log$
# Revision 1.3  1997/12/02 18:47:53  dkr
# now uses his own CFLAGS!!
#
# Revision 1.2  1997/11/28 13:11:07  dkr
# changed CCFLAGS
#
# Revision 1.1  1997/11/25 13:44:09  dkr
# Initial revision
#
#
#

CC        := gcc
MYFLAGS   := -g


INCS=

all: ../dbug.o

../dbug.o: dbug.c
	$(CC) $(MYFLAGS) -o ../dbug.o -c dbug.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) ../dbug.o *.bak

