#
#
# $Log$
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
# Revision 3.14  2001/03/06 13:17:29  nmw
# compare_tree added
#
# Revision 3.13  2001/03/02 15:50:58  nmw
# change_signature added
#
# Revision 3.12  2001/03/02 14:44:52  sbs
# constants added to INCS
#
# Revision 3.11  2001/03/02 10:36:37  dkr
# /home/dkr/sac/BASE/sac2c/src/tree/RCS/Makefile,v used in touch-rules
#
# Revision 3.10  2001/02/22 12:41:12  nmw
# UndoSSATransfrom added
#
# Revision 3.9  2001/02/13 15:15:45  nmw
# SSATransform added
#
# Revision 3.8  2001/02/12 17:02:29  nmw
# CheckAvis added
#
# Revision 3.7  2001/02/12 10:52:40  nmw
# make deps made
#
# Revision 3.6  2001/02/02 10:39:11  dkr
# make deps done
#
# Revision 3.5  2001/01/19 14:19:06  dkr
# make deps done
#
# Revision 3.4  2001/01/17 10:13:12  dkr
# make deps done
#
# Revision 3.3  2000/12/15 18:16:31  dkr
# cleanup_decls.* adjust_ids.* infer_dfms.* moved from flatten to tree
#
# Revision 3.2  2000/12/06 20:09:12  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:03:22  sacbase
# new release made
#
# [ eliminated ] .....
#
#

#######################################################################################
#
# include general setup:
#

include ../../Makefile.Config


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

tree_basic.c: prf_node_info.mac
	touch $@


%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<

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



