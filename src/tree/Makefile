#
#
# $Log$
# Revision 3.27  2002/06/06 19:56:03  dkr
# target clean modified
#
# Revision 3.26  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 3.25  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.24  2001/11/13 20:32:51  dkr
# make deps done
#
# Revision 3.23  2001/07/18 13:08:21  cg
# make deps done
#
# Revision 3.22  2001/07/18 12:57:45  cg
# Compilation of file tree.c eliminated.
#
# Revision 3.21  2001/05/08 13:29:45  dkr
# make deps done
#
# Revision 3.20  2001/04/26 21:09:57  dkr
# make deps done
#
# Revision 3.19  2001/04/02 11:45:39  dkr
# wl_bounds.o added
#
# Revision 3.18  2001/03/22 20:20:20  dkr
# make deps done
#
# Revision 3.17  2001/03/21 11:20:07  dkr
# make deps done
#
# Revision 3.16  2001/03/09 11:53:45  sbs
# make deps done
#
# Revision 3.15  2001/03/09 11:48:23  sbs
# -I../profile added.
#
# [ eliminated ] .....
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

INCS=-I../../inc -I../tree -I../print -I../flatten -I../scanparse -I../typecheck \
     -I../optimize -I../modules -I../global -I../refcount -I../compile \
     -I../psi-opt -I../objects -I../concurrent -I../multithread \
     -I../c-interface -I../constants -I../profile

all: tree_basic.o tree_compound.o free.o DupTree.o traverse.o \
     DataFlowMask.o DataFlowMaskUtils.o InferDFMs.o CheckAvis.o \
     LookUpTable.o adjust_ids.o cleanup_decls.o scheduling.o \
     SSATransform.o UndoSSATransform.o change_signature.o \
     compare_tree.o wl_bounds.o


#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#
traverse.c: node_info.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@

tree_basic.c: prf_node_info.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@


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
