#
#
# $Log$
# Revision 3.38  2004/10/26 11:11:30  sah
# moved some .o files to NEWAST only
#
# Revision 3.37  2004/10/25 11:59:14  sah
# added SEL traversal
#
# Revision 3.36  2004/10/17 17:47:54  sah
# added serialize_helper.
#
# Revision 3.35  2004/10/11 16:49:02  sah
# added serialize_buildstack.o
#
# Revision 3.34  2004/10/04 17:18:29  sah
# a different subset of files is compiled, depending
# on the compiler version to create
#
# Revision 3.33  2004/09/27 13:19:01  sah
# added serialization files
#
# Revision 3.32  2004/08/09 10:04:35  sah
# xml generates files are removed on clean
# now
#
# Revision 3.31  2004/08/01 19:34:29  sah
# removed target node_compat.o
#
# Revision 3.30  2004/07/03 15:09:35  sah
# added the new node representation into the source code.
# the new ast can be enabled by make newast int the
# project root
#
# Revision 3.29  2004/01/29 12:00:41  skt
# some adaptions (removed CheckAvis.o, SSATransform.o
# and UndoSSATransform.o) because of moving of
# CheckAvis.[ch], SSATransform.[ch] and
# UndoSSATransform.[ch] from src/tree
# to src/flatten
#
# Revision 3.28  2002/09/09 19:17:52  dkr
# tree_basic.c does no longer include prf_node_info.mac
#
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
ifeq ($(shell test -f $(PROJECT_ROOT)/_NEW_AST && echo yes),yes)
  NEWASTTARGETS := node_basic.o free_node.o free_attribs.o \
                   serialize_node.o serialize_attribs.o \
                   deserialize.o serialize_buildstack.o \
                   serialize_helper.o serialize_link.o \
                   serialize.o serialize_stack.o
else
  NEWASTTARGETS :=
endif

INCS=-I../../inc -I../print -I../flatten -I../scanparse -I../typecheck \
     -I../optimize -I../modules -I../global -I../refcount -I../compile \
     -I../psi-opt -I../objects -I../concurrent -I../multithread \
     -I../c-interface -I../constants -I../profile -I.

all: tree_basic.o tree_compound.o free.o DupTree.o traverse.o \
     DataFlowMask.o DataFlowMaskUtils.o InferDFMs.o \
     LookUpTable.o adjust_ids.o cleanup_decls.o scheduling.o \
     change_signature.o compare_tree.o wl_bounds.o \
     $(NEWASTTARGETS)


#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#
traverse.c: node_info.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<
	$(CLOCK_SKEW_ELIMINATION) $@

%.c: xml/ast2%_c.xsl xml/ast.xml
	$(MAKE) -C xml ../$@

clean:
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) -r .sb SunWS_cache
	(cd xml; $(MAKE) clean)


#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps
