#
#
# $Log$
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
# Revision 1.23  2000/11/02 14:42:26  dkr
# explicit .mac dependencies added
#
# Revision 1.17  2000/07/05 11:42:23  nmw
# moved cinterface files from print to c-interface
#
# Revision 1.16  2000/06/13 13:38:34  dkr
# path -I../Old2NewWith removed
#
# Revision 1.8  2000/01/28 13:52:41  jhs
# Added LookUpTable.o, gmake deps done.
#
# Revision 1.7  2000/01/28 12:46:27  dkr
# LookUpTable.o added
#
# Revision 1.4  2000/01/24 11:49:49  jhs
# Added scheduling.o (moved here from concurrent).
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

all: tree.o tree_basic.o tree_compound.o free.o DupTree.o traverse.o \
     DataFlowMask.o DataFlowMaskUtils.o InferDFMs.o CheckAvis.o \
     LookUpTable.o adjust_ids.o cleanup_decls.o scheduling.o \
     SSATransform.o UndoSSATransform.o


#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#
traverse.c: node_info.mac
	touch traverse.c

tree_basic.c: prf_node_info.mac
	touch tree_basic.c


%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

CheckAvis.o: ../../inc/dbug.h /usr/include/stdio.h
CheckAvis.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
CheckAvis.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
CheckAvis.o: /usr/include/stdio_impl.h tree.h ../global/types.h tree_basic.h
CheckAvis.o: tree_compound.h ../global/Error.h /usr/include/string.h
CheckAvis.o: /usr/include/stdlib.h ../global/globals.h
CheckAvis.o: ../global/internal_lib.h traverse.h free.h ../global/my_debug.h
CheckAvis.o: CheckAvis.h
DataFlowMask.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
DataFlowMask.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
DataFlowMask.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
DataFlowMask.o: /usr/include/string.h ../../inc/dbug.h ../global/types.h
DataFlowMask.o: tree_basic.h tree_compound.h ../global/Error.h
DataFlowMask.o: /usr/include/stdlib.h ../global/globals.h
DataFlowMask.o: ../global/internal_lib.h free.h ../global/my_debug.h
DataFlowMaskUtils.o: tree.h ../global/types.h tree_basic.h tree_compound.h
DataFlowMaskUtils.o: ../global/Error.h /usr/include/stdio.h
DataFlowMaskUtils.o: /usr/include/sys/feature_tests.h
DataFlowMaskUtils.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
DataFlowMaskUtils.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
DataFlowMaskUtils.o: /usr/include/string.h /usr/include/stdlib.h
DataFlowMaskUtils.o: ../global/globals.h ../global/internal_lib.h free.h
DataFlowMaskUtils.o: ../../inc/dbug.h ../global/my_debug.h DupTree.h
DataFlowMaskUtils.o: LookUpTable.h DataFlowMask.h ../typecheck/typecheck.h
DupTree.o: /usr/include/string.h /usr/include/sys/feature_tests.h
DupTree.o: /usr/include/sys/isa_defs.h tree.h ../global/types.h tree_basic.h
DupTree.o: tree_compound.h ../global/Error.h /usr/include/stdio.h
DupTree.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
DupTree.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
DupTree.o: ../global/globals.h ../global/internal_lib.h traverse.h free.h
DupTree.o: ../../inc/dbug.h ../global/my_debug.h DupTree.h LookUpTable.h
DupTree.o: DataFlowMask.h DataFlowMaskUtils.h ../optimize/Inline.h
DupTree.o: ../typecheck/typecheck.h scheduling.h ../optimize/generatemasks.h
DupTree.o: ../typecheck/constants.h ../typecheck/shape.h
InferDFMs.o: tree.h ../global/types.h tree_basic.h tree_compound.h
InferDFMs.o: ../global/Error.h /usr/include/stdio.h
InferDFMs.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
InferDFMs.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
InferDFMs.o: /usr/include/stdio_impl.h /usr/include/string.h
InferDFMs.o: /usr/include/stdlib.h ../global/globals.h
InferDFMs.o: ../global/internal_lib.h traverse.h free.h ../../inc/dbug.h
InferDFMs.o: ../global/my_debug.h DupTree.h LookUpTable.h DataFlowMask.h
InferDFMs.o: DataFlowMaskUtils.h InferDFMs.h
LookUpTable.o: /usr/include/string.h /usr/include/sys/feature_tests.h
LookUpTable.o: /usr/include/sys/isa_defs.h tree.h ../global/types.h
LookUpTable.o: tree_basic.h tree_compound.h ../global/Error.h
LookUpTable.o: /usr/include/stdio.h /usr/include/sys/va_list.h
LookUpTable.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
LookUpTable.o: /usr/include/stdlib.h ../global/globals.h
LookUpTable.o: ../global/internal_lib.h free.h ../../inc/dbug.h
LookUpTable.o: ../global/my_debug.h
SSATransform.o: ../../inc/dbug.h /usr/include/stdio.h
SSATransform.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
SSATransform.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
SSATransform.o: /usr/include/stdio_impl.h ../global/globals.h
SSATransform.o: ../global/types.h tree.h tree_basic.h tree_compound.h
SSATransform.o: ../global/Error.h /usr/include/string.h /usr/include/stdlib.h
SSATransform.o: ../global/internal_lib.h traverse.h free.h
SSATransform.o: ../global/my_debug.h DupTree.h LookUpTable.h SSATransform.h
UndoSSATransform.o: ../../inc/dbug.h /usr/include/stdio.h
UndoSSATransform.o: /usr/include/sys/feature_tests.h
UndoSSATransform.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
UndoSSATransform.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h tree.h
UndoSSATransform.o: ../global/types.h tree_basic.h tree_compound.h
UndoSSATransform.o: ../global/Error.h /usr/include/string.h
UndoSSATransform.o: /usr/include/stdlib.h ../global/globals.h
UndoSSATransform.o: ../global/internal_lib.h traverse.h free.h
UndoSSATransform.o: ../global/my_debug.h UndoSSATransform.h
adjust_ids.o: ../../inc/dbug.h /usr/include/stdio.h
adjust_ids.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
adjust_ids.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
adjust_ids.o: /usr/include/stdio_impl.h tree.h ../global/types.h tree_basic.h
adjust_ids.o: tree_compound.h ../global/Error.h /usr/include/string.h
adjust_ids.o: /usr/include/stdlib.h ../global/globals.h
adjust_ids.o: ../global/internal_lib.h traverse.h free.h ../global/my_debug.h
adjust_ids.o: DupTree.h LookUpTable.h ../print/print.h
cleanup_decls.o: tree.h ../global/types.h tree_basic.h tree_compound.h
cleanup_decls.o: ../global/Error.h /usr/include/stdio.h
cleanup_decls.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
cleanup_decls.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
cleanup_decls.o: /usr/include/stdio_impl.h /usr/include/string.h
cleanup_decls.o: /usr/include/stdlib.h ../global/globals.h
cleanup_decls.o: ../global/internal_lib.h traverse.h free.h ../../inc/dbug.h
cleanup_decls.o: ../global/my_debug.h DataFlowMask.h DataFlowMaskUtils.h
cleanup_decls.o: LookUpTable.h
free.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
free.o: /usr/include/sys/isa_defs.h /usr/include/stdio.h
free.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
free.o: /usr/include/stdio_impl.h ../../inc/dbug.h ../global/my_debug.h
free.o: ../global/types.h tree.h tree_basic.h tree_compound.h
free.o: ../global/Error.h /usr/include/string.h ../global/globals.h
free.o: ../global/internal_lib.h traverse.h DataFlowMask.h scheduling.h
free.o: ../typecheck/constants.h ../typecheck/shape.h free.h
scheduling.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
scheduling.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
scheduling.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
scheduling.o: /usr/include/string.h /usr/include/varargs.h
scheduling.o: /usr/include/limits.h ../global/types.h tree_basic.h
scheduling.o: tree_compound.h ../global/Error.h /usr/include/stdlib.h
scheduling.o: ../global/globals.h ../global/internal_lib.h free.h
scheduling.o: ../../inc/dbug.h ../global/my_debug.h traverse.h
scheduling.o: ../compile/precompile.h
traverse.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
traverse.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
traverse.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
traverse.o: /usr/include/stdlib.h tree.h ../global/types.h tree_basic.h
traverse.o: tree_compound.h ../global/Error.h /usr/include/string.h
traverse.o: ../global/globals.h ../global/internal_lib.h ../../inc/dbug.h
traverse.o: ../global/my_debug.h ../flatten/flatten.h ../print/print.h
traverse.o: ../typecheck/typecheck.h ../typecheck/typecheck_WL.h
traverse.o: ../optimize/optimize.h free.h ../optimize/generatemasks.h
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
traverse.o: InferDFMs.h ../flatten/lac2fun.h cleanup_decls.h
traverse.o: ../flatten/fun2lac.h adjust_ids.h ../multithread/schedule_init.h
traverse.o: ../multithread/blocks_init.h ../multithread/blocks_expand.h
traverse.o: ../multithread/repfuns_init.h ../multithread/mtfuns_init.h
traverse.o: ../multithread/blocks_cons.h ../multithread/dataflow_analysis.h
traverse.o: ../multithread/blocks_propagate.h ../multithread/barriers_init.h
traverse.o: ../multithread/blocks_lift.h ../multithread/adjust_calls.h
traverse.o: ../psi-opt/pad_collect.h ../psi-opt/pad_transform.h
traverse.o: ../c-interface/print_interface_header.h
traverse.o: ../c-interface/print_interface_wrapper.h
traverse.o: ../c-interface/map_cwrapper.h
traverse.o: ../c-interface/import_specialization.h CheckAvis.h SSATransform.h
traverse.o: UndoSSATransform.h traverse.h
tree.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
tree.o: /usr/include/sys/isa_defs.h tree.h ../global/types.h tree_basic.h
tree.o: tree_compound.h ../global/Error.h /usr/include/stdio.h
tree.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
tree.o: /usr/include/stdio_impl.h /usr/include/string.h ../global/globals.h
tree.o: ../global/internal_lib.h ../../inc/dbug.h ../global/my_debug.h
tree.o: ../scanparse/scnprs.h ../global/resource.h ../scanparse/y.tab.h
tree.o: ../optimize/optimize.h free.h
tree_basic.o: ../global/types.h tree_basic.h ../global/Error.h
tree_basic.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
tree_basic.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
tree_basic.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
tree_basic.o: /usr/include/string.h /usr/include/stdlib.h ../global/globals.h
tree_basic.o: ../../inc/dbug.h ../global/my_debug.h ../scanparse/scnprs.h
tree_basic.o: ../global/resource.h ../scanparse/y.tab.h free.h
tree_basic.o: ../global/internal_lib.h prf_node_info.mac
tree_compound.o: ../global/types.h tree_basic.h tree_compound.h
tree_compound.o: ../global/Error.h /usr/include/stdio.h
tree_compound.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
tree_compound.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
tree_compound.o: /usr/include/stdio_impl.h /usr/include/string.h
tree_compound.o: /usr/include/stdlib.h ../global/globals.h DupTree.h
tree_compound.o: LookUpTable.h ../../inc/dbug.h ../global/my_debug.h free.h
tree_compound.o: ../global/internal_lib.h ../typecheck/typecheck.h
tree_compound.o: ../compile/wltransform.h
