#
#
# $Log$
# Revision 3.23  2004/10/11 14:51:32  ktr
# Added rcphase.o
#
# Revision 3.22  2004/10/10 09:55:45  ktr
# Added rcopt.o
#
# Revision 3.21  2004/09/28 14:08:58  ktr
# removed old refcount and generatemasks
#
# Revision 3.20  2004/08/10 13:31:30  ktr
# reuse.o added.
#
# Revision 3.19  2004/08/09 14:57:13  ktr
# added allocation.o
#
# Revision 3.18  2004/07/18 08:50:42  ktr
# added refcounting.o.
#
# Revision 3.17  2004/07/14 15:24:49  ktr
# alloc.o added.
#
# Revision 3.16  2004/04/21 16:41:45  ktr
# Added SSARefCount.[ch]
#
# Revision 3.15  2003/06/11 21:50:17  ktr
# included ../constants
#
# Revision 3.14  2002/09/25 14:23:14  sbs
# dangerous space eliminated 8-)
#
# Revision 3.13  2002/06/06 19:56:03  dkr
# target clean modified
#
# Revision 3.12  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 3.11  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.10  2001/11/13 20:32:45  dkr
# make deps done
#
# Revision 3.9  2001/03/22 20:20:02  dkr
# make deps done
#
# Revision 3.8  2001/02/02 10:39:04  dkr
# make deps done
#
# Revision 3.7  2001/01/17 10:13:05  dkr
# make deps done
#
# Revision 3.6  2000/12/15 18:18:01  dkr
# make deps done
#
# Revision 3.5  2000/12/12 15:09:10  dkr
# make deps done
#
# Revision 3.4  2000/12/12 11:38:39  dkr
# include path extended
#
# Revision 3.3  2000/12/07 13:45:36  dkr
# include path extended
#
# Revision 3.2  2000/12/06 20:09:05  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:01:34  sacbase
# new release made
#
# ... [eliminated] ...
#
# Revision 1.1  1995/03/09  16:17:01  hw
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

INCS = -I../global -I../tree -I../../inc -I../flatten -I../typecheck \
       -I../compile -I../constants -I../print -I../optimize


all: allocation.o alloc.o refcounting.o reuse.o rcopt.o rcphase.o


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
