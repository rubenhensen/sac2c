#
#
# $Log$
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

$(TOOLS)/bin/%: %.c $(TOOLS)/bin
	$(CC) $(CCFLAGS) -o $@ $<

$(TOOLS)/bin:
	mkdir -p $(TOOLS)/bin

clean:
	$(RM) *~ *.o
	$(RM) ../echo
