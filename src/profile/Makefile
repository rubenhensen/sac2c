#
#
# $Log$
# Revision 1.6  2002/06/06 19:56:03  dkr
# target clean modified
#
# Revision 1.5  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 1.4  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 1.3  2001/11/13 20:32:53  dkr
# make deps done
#
# Revision 1.2  2001/03/22 20:20:23  dkr
# make deps done
#
# Revision 1.1  2001/03/09 11:08:05  sbs
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

INCS = -I../../inc -I../tree -I../global -I../print


all: annotate_fun_calls.o


%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)
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
