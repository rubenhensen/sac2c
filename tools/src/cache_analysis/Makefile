#
#
# $Log$
# Revision 1.9  2003/04/01 10:53:32  sbs
# Makefile.Config and Makefile.Deps included properly now.
#
# Revision 1.8  2001/12/06 14:39:46  dkr
# target 'clean' corrected
#
# Revision 1.7  2001/11/20 10:42:09  sacbase
# mkdir integrated into make rule rather than separate rule
#
# Revision 1.6  2001/11/20 10:18:15  sacbase
# missing dep for $(LIBS) added.
#
# Revision 1.5  2001/11/20 10:17:20  sacbase
# rule for $(LIBS) added.
#
# Revision 1.4  2001/11/20 10:13:38  sacbase
# new destination directory $(TOOLS)/bin added.
#
# Revision 1.3  2001/11/19 18:20:33  dkr
# rcs header added
# Makefile.Config included
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

all: $(TOOLS)/bin/ctest

%.o: %.c
	$(CC) $(CCFLAGS) $(INCS) -c $<

$(TOOLS)/bin/ctest: accesstime.o ctest.o verbose.o l1.o l2.o bits.o $(LIBS)
	mkdir -p $(TOOLS)/bin
	$(CC) $(CCFLAGS) -o $@ $(LIBS) accesstime.o ctest.o verbose.o l1.o l2.o bits.o

$(LIBS):
	$(MAKE) -C $(LIBDIR)/src

clean:
	$(RM) *.o *~
	$(RM) $(TOOLS)/bin/ctest


#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include $(PROJECT_ROOT)/Makefile.Deps

