#
#
# $Log$
# Revision 3.7  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 3.6  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.5  2001/11/13 20:32:46  dkr
# make deps done
#
# Revision 3.4  2001/04/26 17:07:49  dkr
# rmvoidfun.o removed
#
# Revision 3.3  2001/01/17 10:13:06  dkr
# make deps done
#
# Revision 3.2  2000/12/06 20:09:06  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:01:58  sacbase
# new release made
#
# ... [eliminated] ...
#
# Revision 1.1  1995/10/16  12:07:18  cg
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

INCS= -I../global -I../tree -I../../inc -I../typecheck -I../modules -I../print

all: objinit.o objects.o uniquecheck.o

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)
	$(CLOCK_SKEW_ELIMINATION) $@

clean:
	$(RM) *.o *.bak .*.d
	$(RM) -r .sb SunWS_cache



#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps



