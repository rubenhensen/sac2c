#
# $Id$
#

###############################################################################
#
# include general setup:
#

PROJECT_ROOT := ../../../
include $(PROJECT_ROOT)/Makefile.Config


INCS   = -I../../../inc
LIBDIR = ../../../lib
LIBS   = $(LIBDIR)/md5.o

TOOLS = ../../

all: $(TOOLS)/bin/fingerprinter

$(TOOLS)/bin/fingerprinter: fingerprinter.c
	mkdir -p $(TOOLS)/bin
	$(CC) $(CCFLAGS) -o $@ fingerprinter.c $(INCS) $(LIBS)

$(LIBS):
	$(MAKE) -C $(LIBDIR)/src

clean:
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) $(TOOLS)/bin/fingerprinter

###############################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include $(PROJECT_ROOT)/Makefile.Deps

