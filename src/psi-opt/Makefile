#
#
# $Log$
# Revision 3.17  2002/06/06 19:56:03  dkr
# target clean modified
#
# Revision 3.16  2002/03/13 16:00:18  ktr
# WithloopScalarization.c added for compilation
#
# Revision 3.15  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 3.14  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.13  2001/11/13 20:32:45  dkr
# make deps done
#
# Revision 3.12  2001/07/18 13:08:19  cg
# make deps done
#
# Revision 3.11  2001/05/15 16:38:38  nmw
# SSAWLI.o and SSAWLF.o added
#
# Revision 3.10  2001/05/14 15:58:50  nmw
# SSAWLT.o and SSAWithloopFolding.o added
#
# Revision 3.9  2001/04/26 21:09:48  dkr
# make deps done
#
# Revision 3.8  2001/03/27 11:12:49  nmw
# include of directory ../constants added
#
# ... [eliminated] ...
#
# Revision 1.1  1995/06/02  10:09:27  sbs
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


INCS = -I../global -I../../inc -I../tree -I../compile -I../typecheck -I../modules \
       -I../print -I../optimize -I../constants


all: index.o ArrayElimination.o wl_access_analyze.o tile_size_inference.o \
     WithloopFolding.o WLT.o WLI.o WLF.o \
     SSAWithloopFolding.o SSAWLT.o SSAWLI.o SSAWLF.o \
     pad.o pad_info.o pad_collect.o pad_infer.o pad_transform.o \
     WithloopScalarization.o


#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#
pad_collect.c: ../tree/prf_node_info.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@

pad_infer.c: ../tree/type_info.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@

pad_transform.c: ../tree/prf_node_info.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@


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
