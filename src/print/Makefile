#
#
# $Log$
# Revision 3.11  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 3.10  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.9  2001/11/13 20:32:41  dkr
# make deps done
#
# Revision 3.8  2001/05/08 13:29:30  dkr
# make deps done
#
# Revision 3.7  2001/05/08 12:24:18  dkr
# INCS modified
#
# Revision 3.6  2001/04/26 21:09:42  dkr
# make deps done
#
# Revision 3.5  2001/03/22 20:19:57  dkr
# make deps done
#
# Revision 3.4  2001/03/02 10:36:15  dkr
# /home/dkr/sac/BASE/sac2c/src/print/RCS/Makefile,v used in touch-rules
#
# Revision 3.3  2001/01/17 10:13:01  dkr
# make deps done
#
# Revision 3.2  2000/12/06 20:09:00  dkr
# make deps done
#
# Revision 3.1  2000/11/20 17:59:42  sacbase
# new release made
#
# Revision 2.24  2000/11/02 14:49:01  dkr
# explicit .mac dependencies added
#
# Revision 2.20  2000/07/24 14:46:21  dkr
# suffix rule for .c.o added
#
# Revision 2.17  2000/07/05 11:41:49  nmw
# moved cinterface files from print to c-interface
#
# Revision 2.16  2000/06/23 16:38:42  nmw
# map_cwrapper inserted
#
# Revision 2.15  2000/06/21 12:12:24  nmw
# make deps, includes modified
#
# Revision 2.13  2000/06/08 12:47:38  nmw
# print_interface added
#
# Revision 2.6  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# ... [eliminated] ...
#
# Revision 1.4  1994/11/10  15:35:50  sbs
# RCS-header inserted
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


INCS = -I../global -I../tree -I../compile -I../modules -I../refcount \
       -I../concurrent -I../psi-opt -I../optimize -I../typecheck \
       -I../c-interface -I../../inc

all: print.o convert.o


#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#
convert.c: ../tree/type_info.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@

print.c: ../tree/prf_node_info.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@


%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)
	$(CLOCK_SKEW_ELIMINATION) $@

clean:
	$(RM) print.o convert.o *.bak .*.d
	$(RM) -r .sb SunWS_cache

#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps



