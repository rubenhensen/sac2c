#
# $Log$
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

INCS=-I../global -I../../inc -I../optimize

all: concregions.o

concregions.o: concregions.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o concregions.o -c concregions.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
	$(RM) -r .sb

# DO NOT DELETE THIS LINE -- make depend depends on it.
