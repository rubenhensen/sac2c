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

all: $(TOOLS)/bin/ctest

%.o: %.c
	$(CC) $(CCFLAGS) $(INCS) -c $<

$(TOOLS)/bin/ctest: accesstime.o ctest.o verbose.o l1.o l2.o bits.o $(LIBS)
	mkdir -p $(TOOLS)/bin
	$(CC) $(CCFLAGS) -o $@ $(LIBS) accesstime.o ctest.o verbose.o l1.o l2.o bits.o

$(LIBS):
	$(MAKE) -C $(LIBDIR)/src

clean:
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) $(TOOLS)/bin/ctest


#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include $(PROJECT_ROOT)/Makefile.Deps

