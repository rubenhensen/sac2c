#
#
# $Log$
# Revision 3.12  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 3.11  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.10  2001/11/13 20:32:47  dkr
# make deps done
#
# Revision 3.9  2001/07/18 13:08:20  cg
# make deps done
#
# Revision 3.8  2001/05/08 13:29:40  dkr
# make deps done
#
# Revision 3.7  2001/04/26 21:09:52  dkr
# make deps done
#
# Revision 3.6  2001/01/17 10:13:08  dkr
# make deps done
#
# Revision 3.5  2000/12/15 18:18:04  dkr
# make deps done
#
# Revision 3.4  2000/12/12 15:09:15  dkr
# make deps done
#
# Revision 3.3  2000/12/08 15:21:10  dkr
# include path extended
#
# Revision 3.2  2000/12/06 20:09:08  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:02:23  sacbase
# new release made
#
# ... [eliminated] ...
#
# Revision 1.1  1998/04/03 11:47:26  dkr
# Initial revision
#
#


#######################################################################################
#
# include general setup:
#

PROJECT_ROOT := ../..
include $(PROJECT_ROOT)/Makefile.Config


#######################################################################################
#
# local settings:
#


INCS= -I../global -I../tree -I../../inc -I../optimize -I../typecheck -I../refcount \
      -I../flatten -I../print -I../compile

all: concurrent.o spmd_init.o spmd_opt.o spmd_lift.o sync_init.o \
     sync_opt.o schedule.o spmd_trav.o spmd_cons.o concurrent_lib.o

%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<
	$(CLOCK_SKEW_ELIMINATION) $@

clean:
	$(RM) *.o *.bak *~ .*.d
	$(RM) -r .sb SunWS_cache



#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps



