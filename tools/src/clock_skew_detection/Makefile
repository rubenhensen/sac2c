#
#
# $Log$
# Revision 1.10  2003/04/01 10:53:32  sbs
# Makefile.Config and Makefile.Deps included properly now.
#
# Revision 1.9  2001/12/06 14:39:38  dkr
# target 'clean' corrected
#
# Revision 1.8  2001/11/20 10:42:09  sacbase
# mkdir integrated into make rule rather than separate rule
#
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

#######################################################################################
#
# include general setup:
#

PROJECT_ROOT := ../../../
include $(PROJECT_ROOT)/Makefile.Config


INCS   = -I../../../inc
LIBDIR = ../../../lib
LIBS   = $(LIBDIR)/main_args.o

TOOLS = ../../

all: $(TOOLS)/bin/csd $(TOOLS)/bin/cse

$(TOOLS)/bin/csd: csd.c
	mkdir -p $(TOOLS)/bin
	$(CC) $(CCFLAGS) -o $@ csd.c

$(TOOLS)/bin/cse: cse.c $(LIBS) 
	mkdir -p $(TOOLS)/bin
	$(CC) $(CCFLAGS) -o $@ cse.c $(INCS) $(LIBS) 

$(LIBS):
	$(MAKE) -C $(LIBDIR)/src

clean:
	$(RM) *~ *.o
	$(RM) $(TOOLS)/bin/csd $(TOOLS)/bin/cse

#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include $(PROJECT_ROOT)/Makefile.Deps

