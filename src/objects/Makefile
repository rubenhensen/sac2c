#
#
# $Log$
# Revision 3.10  2004/01/29 13:06:05  skt
# some adaptions because of moving ssa-files
# from src/tree to src/flatten
#
# Revision 3.9  2003/06/11 21:52:51  ktr
# included ../constants
#
# Revision 3.8  2002/06/06 19:56:03  dkr
# target clean modified
#
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

INCS = -I../global -I../tree -I../../inc -I../typecheck -I../modules -I../print \
       -I../constants -I../flatten


all: objinit.o objects.o uniquecheck.o


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
