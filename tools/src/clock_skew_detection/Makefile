#
#
# $Log$
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

all: ../csd ../cse

../csd: csd.c
	$(CC) $(CCFLAGS) -o ../csd csd.c

../cse: cse.c $(LIBS)
	$(CC) $(CCFLAGS) -o ../cse cse.c $(INCS) $(LIBS) 

$(LIBS):
	$(MAKE) -C $(LIBDIR)/src

clean:
	$(RM) *~ *.o
	$(RM) ../csd ../cse
