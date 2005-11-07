#
# $Id$
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

OBJS := tree_basic.o tree_compound.o free.o DupTree.o traverse.o \
        DataFlowMask.o DataFlowMaskUtils.o InferDFMs.o \
        LookUpTable.o adjust_ids.o cleanup_decls.o scheduling.o \
        change_signature.o compare_tree.o wl_bounds.o \
        node_basic.o free_node.o free_attribs.o \
        traverse_tables.o traverse_helper.o check_lib.o check.o checktst.o \
        check_mem.o prepare_inlining.o map_lac_funs.o map_fun_trav.o


all: $(OBJS)

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
