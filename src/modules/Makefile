#
#
# $Log$
# Revision 3.22  2004/10/22 14:47:40  sah
# added usesymbols traversal
#
# Revision 3.21  2004/10/22 09:01:55  sah
# added annotatenamespace traversal.
#
# Revision 3.20  2004/10/21 17:21:22  sah
# added resolveall.o
#
# Revision 3.19  2004/10/17 17:49:05  sah
# added libbuilder
#
# Revision 3.18  2004/10/17 17:02:58  sah
# added new ccmanager
#
# Revision 3.17  2004/10/17 14:52:06  sah
# added export traversal
#
# Revision 3.16  2004/10/04 17:18:29  sah
# a different subset of files is compiled, depending
# on the compiler version to create
#
# Revision 3.15  2004/09/30 20:01:26  sah
# added a switch to compile a subset of
# the compiler only
#
# Revision 3.14  2004/09/27 13:19:41  sah
# added symboltable
#
# Revision 3.13  2004/09/27 12:55:25  sah
# added local directory to includes
#
# Revision 3.12  2003/06/11 21:48:16  ktr
# included ../constants
#
# Revision 3.11  2002/06/06 19:56:03  dkr
# target clean modified
#
# Revision 3.10  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 3.9  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.8  2001/11/13 20:32:44  dkr
# make deps done
#
# Revision 3.7  2001/07/18 13:08:17  cg
# make deps done
#
# Revision 3.6  2001/03/22 20:20:00  dkr
# make deps done
#
# Revision 3.5  2001/03/21 11:19:56  dkr
# make deps done
#
# Revision 3.4  2001/02/02 10:39:02  dkr
# make deps done
#
# Revision 3.3  2001/01/17 10:13:04  dkr
# make deps done
#
# Revision 3.2  2000/12/06 20:09:03  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:00:46  sacbase
# new release made
#
# ... [eliminated] ...
#
# Revision 1.1  1994/12/11  17:33:27  sbs
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

INCS = -I../global -I../tree -I../../inc -I../scanparse -I../print  \
       -I../typecheck -I../compile -I../constants -I../optimize -I.
OBJS := filemgr.o symboltable.o implicittypes.o

ifeq ($(NEWAST),no)
  OBJS := import.o readsib.o writesib.o analysis.o \
          checkdec.o cccall.o $(OBJS)
else
  OBJS := libmanager.o modulemanager.o libstat.o export.o \
          ccmanager.o libbuilder.o resolveall.o annotatenamespace.o \
          usesymbols.o $(OBJS)
endif

all: $(OBJS)

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
