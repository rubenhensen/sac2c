#
#
# $Log$
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
#

include ../../../Makefile.Config

INCS   = -I../../../inc
LIBDIR = ../../../lib
LIBS   = $(LIBDIR)/main_args.o

TOOLS = ../../

all: $(TOOLS)/bin/ctest

verbose.o: verbose.c verbose.h
accesstime.o: ctest.h accesstime.h verbose.h accesstime.c 
ctest.o: verbose.h ctest.h accesstime.h l1.h l2.h ctest.c
l1.o: ctest.c l1.h ctest.h l1.c
l2.o: ctest.c l2.h ctest.h l2.c
bits.o: bits.c bits.h

%.o: %.c
	$(CC) $(CCFLAGS) $(INCS) -c $<

$(TOOLS)/bin/ctest: $(TOOLS)/bin accesstime.o ctest.o verbose.o l1.o l2.o bits.o $(LIBS)
	$(CC) $(CCFLAGS) -o $@ $(LIBS) accesstime.o ctest.o verbose.o l1.o l2.o bits.o

$(TOOLS)/bin:
	mkdir -p $(TOOLS)/bin

$(LIBS):
	$(MAKE) -C $(LIBDIR)/src

clean:
	$(RM) *.o *~
	$(RM) $(TOOLS)/bin/ctest
