#
# $Log$
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
     -I../psi-opt -I../objects -I../concurrent -I../Old2NewWith \
     -I../multithread

all: tree.o tree_basic.o tree_compound.o free.o traverse.o DupTree.o \
     DataFlowMask.o DataFlowMaskUtils.o scheduling.o

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

DataFlowMask.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
DataFlowMask.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
DataFlowMask.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
DataFlowMask.o: /usr/include/string.h ../../inc/dbug.h ../global/types.h
DataFlowMask.o: tree_basic.h tree_compound.h ../global/Error.h
DataFlowMask.o: /usr/include/stdlib.h ../global/globals.h
DataFlowMask.o: ../global/internal_lib.h free.h ../global/my_debug.h
DataFlowMask.o: /usr/include/malloc.h /usr/include/sys/types.h
DataFlowMask.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
DataFlowMaskUtils.o: tree.h ../global/types.h tree_basic.h tree_compound.h
DataFlowMaskUtils.o: ../global/Error.h /usr/include/stdio.h
DataFlowMaskUtils.o: /usr/include/sys/feature_tests.h
DataFlowMaskUtils.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
DataFlowMaskUtils.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
DataFlowMaskUtils.o: /usr/include/string.h /usr/include/stdlib.h
DataFlowMaskUtils.o: ../global/globals.h ../global/internal_lib.h
DataFlowMaskUtils.o: ../../inc/dbug.h DupTree.h DataFlowMask.h
DataFlowMaskUtils.o: ../typecheck/typecheck.h
DupTree.o: /usr/include/string.h /usr/include/sys/feature_tests.h
DupTree.o: /usr/include/sys/isa_defs.h tree.h ../global/types.h tree_basic.h
DupTree.o: tree_compound.h ../global/Error.h /usr/include/stdio.h
DupTree.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
DupTree.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
DupTree.o: ../global/globals.h ../global/internal_lib.h traverse.h free.h
DupTree.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
DupTree.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
DupTree.o: /usr/include/sys/int_types.h ../typecheck/typecheck.h DupTree.h
DupTree.o: DataFlowMask.h ../optimize/optimize.h ../optimize/Inline.h
DupTree.o: scheduling.h ../optimize/generatemasks.h
free.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
free.o: /usr/include/sys/isa_defs.h /usr/include/stdio.h
free.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
free.o: /usr/include/stdio_impl.h ../../inc/dbug.h ../global/my_debug.h
free.o: ../global/types.h tree.h tree_basic.h tree_compound.h
free.o: ../global/Error.h /usr/include/string.h ../global/globals.h
free.o: ../global/internal_lib.h traverse.h DataFlowMask.h scheduling.h
free.o: free.h /usr/include/malloc.h /usr/include/sys/types.h
free.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
scheduling.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
scheduling.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
scheduling.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
scheduling.o: /usr/include/string.h /usr/include/varargs.h ../global/types.h
scheduling.o: tree_basic.h ../global/internal_lib.h free.h ../../inc/dbug.h
scheduling.o: ../global/my_debug.h /usr/include/malloc.h
scheduling.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
scheduling.o: /usr/include/sys/int_types.h ../global/globals.h traverse.h
scheduling.o: ../global/Error.h /usr/include/stdlib.h ../compile/precompile.h
traverse.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
traverse.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
traverse.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
traverse.o: /usr/include/stdlib.h tree.h ../global/types.h tree_basic.h
traverse.o: tree_compound.h ../global/Error.h /usr/include/string.h
traverse.o: ../global/globals.h ../global/internal_lib.h ../../inc/dbug.h
traverse.o: ../global/my_debug.h ../flatten/flatten.h ../print/print.h
traverse.o: ../typecheck/typecheck.h ../typecheck/typecheck_WL.h
traverse.o: ../optimize/optimize.h free.h /usr/include/malloc.h
traverse.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
traverse.o: /usr/include/sys/int_types.h ../optimize/generatemasks.h
traverse.o: ../optimize/freemasks.h ../optimize/ConstantFolding.h
traverse.o: ../optimize/DeadCodeRemoval.h ../optimize/DeadFunctionRemoval.h
traverse.o: ../optimize/LoopInvariantRemoval.h ../optimize/CSE.h
traverse.o: ../modules/import.h DupTree.h ../optimize/Inline.h
traverse.o: ../optimize/Unroll.h ../optimize/Unswitch.h
traverse.o: ../psi-opt/ArrayElimination.h ../psi-opt/index.h
traverse.o: ../modules/writesib.h ../modules/readsib.h
traverse.o: ../modules/implicittypes.h ../objects/objinit.h
traverse.o: ../modules/analysis.h ../modules/checkdec.h ../objects/objects.h
traverse.o: ../objects/uniquecheck.h ../objects/rmvoidfun.h
traverse.o: ../refcount/refcount.h ../compile/wltransform.h
traverse.o: ../compile/precompile.h ../compile/compile.h
traverse.o: ../compile/ReuseWithArrays.h ../modules/cccall.h
traverse.o: ../compile/Old2NewWith.h ../psi-opt/WithloopFolding.h
traverse.o: ../psi-opt/WLT.h ../psi-opt/WLI.h ../psi-opt/WLF.h
traverse.o: ../compile/gen_startup_code.h scheduling.h
traverse.o: ../concurrent/concurrent.h ../concurrent/spmd_init.h
traverse.o: ../concurrent/spmd_opt.h ../concurrent/spmd_lift.h
traverse.o: ../concurrent/sync_init.h ../concurrent/sync_opt.h
traverse.o: ../concurrent/spmd_trav.h DataFlowMask.h
traverse.o: ../concurrent/spmd_cons.h ../concurrent/schedule.h
traverse.o: ../psi-opt/wl_access_analyze.h ../psi-opt/tile_size_inference.h
traverse.o: ../typecheck/new_typecheck.h ../multithread/multithread.h
traverse.o: ../flatten/lac2fun.h traverse.h node_info.mac
tree.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
tree.o: /usr/include/sys/isa_defs.h tree.h ../global/types.h tree_basic.h
tree.o: tree_compound.h ../global/Error.h /usr/include/stdio.h
tree.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
tree.o: /usr/include/stdio_impl.h /usr/include/string.h ../global/globals.h
tree.o: ../global/internal_lib.h ../../inc/dbug.h ../global/my_debug.h
tree.o: ../scanparse/scnprs.h ../global/resource.h ../scanparse/y.tab.h
tree.o: ../optimize/optimize.h free.h /usr/include/malloc.h
tree.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
tree.o: /usr/include/sys/int_types.h
tree_basic.o: ../global/types.h tree_basic.h ../global/Error.h
tree_basic.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
tree_basic.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
tree_basic.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
tree_basic.o: /usr/include/string.h /usr/include/stdlib.h ../global/globals.h
tree_basic.o: ../../inc/dbug.h ../global/my_debug.h ../scanparse/scnprs.h
tree_basic.o: ../global/resource.h ../scanparse/y.tab.h free.h
tree_basic.o: /usr/include/malloc.h /usr/include/sys/types.h
tree_basic.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
tree_basic.o: ../global/internal_lib.h prf_node_info.mac
tree_compound.o: ../global/types.h tree_basic.h tree_compound.h
tree_compound.o: ../global/Error.h /usr/include/stdio.h
tree_compound.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
tree_compound.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
tree_compound.o: /usr/include/stdio_impl.h /usr/include/string.h
tree_compound.o: /usr/include/stdlib.h ../global/globals.h ../../inc/dbug.h
tree_compound.o: ../global/my_debug.h free.h /usr/include/malloc.h
tree_compound.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
tree_compound.o: /usr/include/sys/int_types.h ../global/internal_lib.h
