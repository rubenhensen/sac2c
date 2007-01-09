#
# $Id$

#######################################################################################
#
# include general setup:
#

PROJECT_ROOT := ../../../
include $(PROJECT_ROOT)/Makefile.Config


INCS   = -I../../../inc
LIBDIR = ../../../lib
LIBS   = $(LIBDIR)/getoptions.o

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
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) $(TOOLS)/bin/csd $(TOOLS)/bin/cse

#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include $(PROJECT_ROOT)/Makefile.Deps

