#
# $Log$
# Revision 1.19  2000/07/11 15:56:40  dkr
# make deps done
#
# Revision 1.18  2000/07/05 14:24:01  dkr
# make deps done
#
# Revision 1.17  2000/07/05 11:42:23  nmw
# moved cinterface files from print to c-interface
#
# Revision 1.16  2000/06/13 13:38:34  dkr
# path -I../Old2NewWith removed
#
# Revision 1.15  2000/06/08 15:47:18  nmw
# make deps
#
# Revision 1.14  2000/05/29 14:36:48  dkr
# make deps done
#
# Revision 1.13  2000/03/31 09:33:50  jhs
# make deps done
#
# Revision 1.12  2000/03/31 09:22:42  cg
# make deps done
#
# Revision 1.11  2000/03/19 11:09:06  dkr
# make deps done
#
# Revision 1.10  2000/03/16 14:32:53  dkr
# make deps done
#
# Revision 1.9  2000/03/01 14:52:44  dkr
# make deps done
#
# Revision 1.8  2000/01/28 13:52:41  jhs
# Added LookUpTable.o, gmake deps done.
#
# Revision 1.7  2000/01/28 12:46:27  dkr
# LookUpTable.o added
#
# Revision 1.6  2000/01/25 13:52:40  dkr
# make deps done
#
# Revision 1.5  2000/01/24 17:58:43  dkr
# make deps done
#
# Revision 1.4  2000/01/24 11:49:49  jhs
# Added scheduling.o (moved here from concurrent).
#
# Revision 1.3  2000/01/21 17:02:17  dkr
# make deps done
#
# Revision 1.2  2000/01/21 16:45:49  dkr
# tree related stuff moved from directory 'global' into 'tree'
#
# Revision 1.1  2000/01/21 12:46:00  dkr
# Initial revision
#
#


#
# OS setup:
#
# legal values for OS currently are:
#    SOLARIS_SPARC
#    LINUX_X86
#    OSF_ALPHA
#
OS        := SOLARIS_SPARC


CC        := gcc
CCFLAGS   := -ansi -Wall -g -DSAC_FOR_$(OS)
CFLAGS    := -D__EXTENSIONS__


INCS=-I../../inc -I../tree -I../print -I../flatten -I../scanparse -I../typecheck \
     -I../optimize -I../modules -I../global -I../refcount -I../compile \
     -I../psi-opt -I../objects -I../concurrent -I../multithread \
     -I../c-interface

all: tree.o tree_basic.o tree_compound.o free.o traverse.o DupTree.o LookUpTable.o \
     DataFlowMask.o DataFlowMaskUtils.o scheduling.o LookUpTable.o

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

DataFlowMask.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
DataFlowMask.o: /usr/include/sys/va_list.h /usr/include/string.h
DataFlowMask.o: ../../inc/dbug.h ../global/types.h tree_basic.h
DataFlowMask.o: tree_compound.h ../global/Error.h /usr/include/stdlib.h
DataFlowMask.o: ../global/globals.h ../global/internal_lib.h free.h
DataFlowMask.o: ../global/my_debug.h /usr/include/malloc.h
DataFlowMaskUtils.o: tree.h ../global/types.h tree_basic.h tree_compound.h
DataFlowMaskUtils.o: ../global/Error.h /usr/include/stdio.h
DataFlowMaskUtils.o: /usr/include/sys/feature_tests.h
DataFlowMaskUtils.o: /usr/include/sys/va_list.h /usr/include/string.h
DataFlowMaskUtils.o: /usr/include/stdlib.h ../global/globals.h
DataFlowMaskUtils.o: ../global/internal_lib.h free.h ../../inc/dbug.h
DataFlowMaskUtils.o: ../global/my_debug.h /usr/include/malloc.h DupTree.h
DataFlowMaskUtils.o: LookUpTable.h DataFlowMask.h ../typecheck/typecheck.h
DupTree.o: /usr/include/string.h /usr/include/sys/feature_tests.h tree.h
DupTree.o: ../global/types.h tree_basic.h tree_compound.h ../global/Error.h
DupTree.o: /usr/include/stdio.h /usr/include/sys/va_list.h
DupTree.o: /usr/include/stdlib.h ../global/globals.h ../global/internal_lib.h
DupTree.o: traverse.h free.h ../../inc/dbug.h ../global/my_debug.h
DupTree.o: /usr/include/malloc.h DupTree.h LookUpTable.h DataFlowMask.h
DupTree.o: DataFlowMaskUtils.h ../optimize/Inline.h ../typecheck/typecheck.h
DupTree.o: scheduling.h ../optimize/generatemasks.h
LookUpTable.o: /usr/include/string.h /usr/include/sys/feature_tests.h tree.h
LookUpTable.o: ../global/types.h tree_basic.h tree_compound.h
LookUpTable.o: ../global/Error.h /usr/include/stdio.h
LookUpTable.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
LookUpTable.o: ../global/globals.h ../global/internal_lib.h free.h
LookUpTable.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
free.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
free.o: /usr/include/stdio.h /usr/include/sys/va_list.h ../../inc/dbug.h
free.o: ../global/my_debug.h ../global/types.h tree.h tree_basic.h
free.o: tree_compound.h ../global/Error.h /usr/include/string.h
free.o: ../global/globals.h ../global/internal_lib.h traverse.h
free.o: DataFlowMask.h scheduling.h free.h /usr/include/malloc.h
scheduling.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
scheduling.o: /usr/include/sys/va_list.h /usr/include/string.h
scheduling.o: /usr/include/varargs.h ../global/types.h tree_basic.h
scheduling.o: ../global/internal_lib.h free.h ../../inc/dbug.h
scheduling.o: ../global/my_debug.h /usr/include/malloc.h ../global/globals.h
scheduling.o: traverse.h ../global/Error.h /usr/include/stdlib.h
scheduling.o: ../compile/precompile.h
traverse.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
traverse.o: /usr/include/sys/va_list.h /usr/include/stdlib.h tree.h
traverse.o: ../global/types.h tree_basic.h tree_compound.h ../global/Error.h
traverse.o: /usr/include/string.h ../global/globals.h
traverse.o: ../global/internal_lib.h ../../inc/dbug.h ../global/my_debug.h
traverse.o: ../flatten/flatten.h ../print/print.h ../typecheck/typecheck.h
traverse.o: ../typecheck/typecheck_WL.h ../optimize/optimize.h free.h
traverse.o: /usr/include/malloc.h ../optimize/generatemasks.h
traverse.o: ../optimize/freemasks.h ../optimize/ConstantFolding.h
traverse.o: ../optimize/DeadCodeRemoval.h ../optimize/DeadFunctionRemoval.h
traverse.o: ../optimize/LoopInvariantRemoval.h ../optimize/CSE.h
traverse.o: ../modules/import.h DupTree.h LookUpTable.h ../optimize/Inline.h
traverse.o: ../optimize/Unroll.h ../optimize/Unswitch.h
traverse.o: ../psi-opt/ArrayElimination.h ../psi-opt/index.h
traverse.o: ../modules/writesib.h ../modules/readsib.h
traverse.o: ../modules/implicittypes.h ../objects/objinit.h
traverse.o: ../modules/analysis.h ../modules/checkdec.h ../objects/objects.h
traverse.o: ../objects/uniquecheck.h ../objects/rmvoidfun.h
traverse.o: ../refcount/refcount.h ../compile/wltransform.h
traverse.o: ../compile/precompile.h ../compile/compile.h
traverse.o: ../compile/ReuseWithArrays.h ../modules/cccall.h
traverse.o: ../compile/PatchWith.h ../psi-opt/WithloopFolding.h
traverse.o: ../psi-opt/WLT.h ../psi-opt/WLI.h ../psi-opt/WLF.h
traverse.o: ../compile/gen_startup_code.h scheduling.h
traverse.o: ../concurrent/concurrent.h ../concurrent/spmd_init.h
traverse.o: ../concurrent/spmd_opt.h ../concurrent/spmd_lift.h
traverse.o: ../concurrent/sync_init.h ../concurrent/sync_opt.h
traverse.o: ../concurrent/spmd_trav.h DataFlowMask.h
traverse.o: ../concurrent/spmd_cons.h ../concurrent/schedule.h
traverse.o: ../psi-opt/wl_access_analyze.h ../psi-opt/tile_size_inference.h
traverse.o: ../typecheck/new_typecheck.h ../multithread/multithread.h
traverse.o: ../flatten/lac2fun.h ../flatten/cleanup_decls.h
traverse.o: ../flatten/fun2lac.h ../flatten/adjust_ids.h
traverse.o: ../multithread/schedule_init.h ../multithread/blocks_init.h
traverse.o: ../multithread/blocks_expand.h ../multithread/repfuns_init.h
traverse.o: ../multithread/mtfuns_init.h ../multithread/blocks_cons.h
traverse.o: ../multithread/dataflow_analysis.h
traverse.o: ../multithread/blocks_propagate.h ../multithread/barriers_init.h
traverse.o: ../multithread/blocks_lift.h ../multithread/adjust_calls.h
traverse.o: ../psi-opt/pad_collect.h ../psi-opt/pad_transform.h
traverse.o: ../c-interface/print_interface.h ../c-interface/map_cwrapper.h
traverse.o: traverse.h
tree.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h tree.h
tree.o: ../global/types.h tree_basic.h tree_compound.h ../global/Error.h
tree.o: /usr/include/stdio.h /usr/include/sys/va_list.h /usr/include/string.h
tree.o: ../global/globals.h ../global/internal_lib.h ../../inc/dbug.h
tree.o: ../global/my_debug.h ../scanparse/scnprs.h ../global/resource.h
tree.o: ../scanparse/y.tab.h ../optimize/optimize.h free.h
tree.o: /usr/include/malloc.h
tree_basic.o: ../global/types.h tree_basic.h ../global/Error.h
tree_basic.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
tree_basic.o: /usr/include/sys/va_list.h /usr/include/string.h
tree_basic.o: /usr/include/stdlib.h ../global/globals.h ../../inc/dbug.h
tree_basic.o: ../global/my_debug.h ../scanparse/scnprs.h ../global/resource.h
tree_basic.o: ../scanparse/y.tab.h free.h /usr/include/malloc.h
tree_basic.o: ../global/internal_lib.h prf_node_info.mac
tree_compound.o: ../global/types.h tree_basic.h tree_compound.h
tree_compound.o: ../global/Error.h /usr/include/stdio.h
tree_compound.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
tree_compound.o: /usr/include/string.h /usr/include/stdlib.h
tree_compound.o: ../global/globals.h ../../inc/dbug.h ../global/my_debug.h
tree_compound.o: free.h /usr/include/malloc.h ../global/internal_lib.h
