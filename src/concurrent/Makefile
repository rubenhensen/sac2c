#
# $Id$
#


###############################################################################
#
# include general setup:
#

PROJECT_ROOT := ../..
include $(PROJECT_ROOT)/Makefile.Config


###############################################################################
#
# local settings:
#


#INCS = -I../global -I../tree -I../../inc -I../optimize -I../typecheck -I../refcount \
 #      -I../flatten -I../print -I../compile -I../constants

OBJS := concurrent.o spmd_init.o spmd_lift.o sync_init.o \
        sync_opt.o schedule.o spmd_trav.o 

all: $(OBJS)

%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<
	$(CLOCK_SKEW_ELIMINATION) $@


clean:
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) -r .sb SunWS_cache


#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps
