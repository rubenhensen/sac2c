#
#
# $Log$
# Revision 1.3  2001/11/20 10:42:09  sacbase
# mkdir integrated into make rule rather than separate rule
#
# Revision 1.2  2001/11/20 10:26:14  sacbase
# $(TOOLS)/bin added.
#
# Revision 1.1  2001/11/19 18:23:06  dkr
# Initial revision
#
#

include ../../../Makefile.Config

TOOLS = ../../

all: $(TOOLS)/bin/echo

$(TOOLS)/bin/%: %.c
	mkdir -p $(TOOLS)/bin
	$(CC) $(CCFLAGS) -o $@ $<


clean:
	$(RM) *~ *.o
	$(RM) ../echo
