#
#
# $Log$
# Revision 1.7  2001/11/20 10:34:13  sacbase
# $(TOOLS)/bin added.
#
# Revision 1.6  2001/11/19 18:16:35  dkr
# clean target added
#
# Revision 1.5  2001/11/15 16:03:10  sacbase
# rule for main_args.o added.
#
#

include ../../../Makefile.Config

INCS   = -I../../../inc
LIBDIR = ../../../lib
LIBS   = $(LIBDIR)/main_args.o

TOOLS = ../../

all: $(TOOLS)/bin/csd $(TOOLS)/bin/cse

$(TOOLS)/bin/csd: $(TOOLS)/bin csd.c
	$(CC) $(CCFLAGS) -o $@ csd.c

$(TOOLS)/bin/cse: $(TOOLS)/bin cse.c $(LIBS) 
	$(CC) $(CCFLAGS) -o $@ cse.c $(INCS) $(LIBS) 

$(TOOLS)/bin:
	mkdir -p $(TOOLS)/bin

$(LIBS):
	$(MAKE) -C $(LIBDIR)/src

clean:
	$(RM) *~ *.o
	$(RM) ../csd ../cse
