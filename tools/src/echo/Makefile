#
#
# $Log$
# Revision 1.1  2001/11/19 18:23:06  dkr
# Initial revision
#
#

include ../../../Makefile.Config

all: ../echo

../%: %.c
	$(CC) $(CCFLAGS) -o $@ $<

clean:
	$(RM) *~ *.o
	$(RM) ../echo
