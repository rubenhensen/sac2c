#
#
# $Log$
# Revision 1.1  2003/09/19 14:40:02  dkr
# Initial revision
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

TOOLS = ../..

all: $(TOOLS)/bin/cb

$(TOOLS)/bin/cb: cb.c
	mkdir -p $(TOOLS)/bin
	$(CC) $(CCFLAGS) -o $@ $< $(INCS) $(LIBS) 

$(LIBS):
	$(MAKE) -C $(LIBDIR)/src

clean:
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) $(TOOLS)/bin/cb

#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include $(PROJECT_ROOT)/Makefile.Deps

