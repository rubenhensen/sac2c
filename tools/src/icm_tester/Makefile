#
#
# $Log$
# Revision 1.1  2003/09/22 15:19:53  dkr
# Initial revision
#
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
LIBS   = 

TOOLS = ../..

all: $(TOOLS)/bin/icm_tester

$(TOOLS)/bin/icm_tester: icm_tester.c
	mkdir -p $(TOOLS)/bin
	$(CC) $(CCFLAGS) -o $@ $< $(INCS) $(LIBS) 

$(LIBS):
	$(MAKE) -C $(LIBDIR)/src

clean:
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) $(TOOLS)/bin/icm_tester

#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include $(PROJECT_ROOT)/Makefile.Deps

