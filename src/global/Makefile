#
# $Id$
#

################################################################################
#
# include general setup:
#

PROJECT_ROOT := ../..
include $(PROJECT_ROOT)/Makefile.Config


################################################################################
#
# local settings:
#

REV = $(shell svn info | grep Revision: | sed -e 's/Revision: //g')

all: main.o usage.o internal_lib.o globals.o resource.o \
     build.o options.o NameTuplesUtils.o filemgr.o \
     ctinfo.o phase.o setup.o private_heap.o

build.c: 
	echo "char build_date[] = \"`date`\";"  >  build.c
	echo "char build_user[] = \"$(USER)\";" >> build.c
	echo "char build_host[] = \"`hostname`\";" >> build.c
	echo "char build_os[]   = \"$(OS)\";"   >> build.c
	echo "char build_rev[]  = \"$(REV)\";"  >> build.c
	$(CLOCK_SKEW_ELIMINATION) $@

build.o: build.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o build.o -c build.c $(INCS)
	$(RM) build.c
	$(CLOCK_SKEW_ELIMINATION) $@

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)
	$(CLOCK_SKEW_ELIMINATION) $@

clean:
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) -r .sb SunWS_cache


################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps
