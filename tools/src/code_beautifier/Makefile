#
# $Id$
#

#######################################################################################
#
# include general setup:
#

PROJECT_ROOT := ../../../
include $(PROJECT_ROOT)/Makefile.Config


INCS   = -I../../../inc
LIBDIR = ../../../lib
LIBS   = $(LIBDIR)/getoptions.o

TOOLS = ../..

all: $(TOOLS)/bin/cb

$(TOOLS)/bin/cb: cb.c $(LIBS)
	mkdir -p $(TOOLS)/bin
	$(CC) $(CCFLAGS) -o $@ $< $(INCS) $(LIBS) 

$(LIBS):
	$(MAKE) -C $(LIBDIR)/src

clean:
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) $(TOOLS)/bin/cb

#######################################################################################
#
#  we do NOT include the automatic dependency mechanism here !!
#
#  the reason being that all *.h files generated from *.xml files have to
#  be generated PRIOR to generating the dependencies. However, their generation
#  requires the existance of cb!!!
#

# include $(PROJECT_ROOT)/Makefile.Deps

