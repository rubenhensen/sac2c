#
#
# $Log$
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

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

CheckAvis.o: ../../inc/dbug.h /usr/include/stdio.h
CheckAvis.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
CheckAvis.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
CheckAvis.o: /usr/include/stdio_impl.h ../global/types.h tree_basic.h
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
DataFlowMaskUtils.o: ../global/types.h tree_basic.h tree_compound.h
DataFlowMaskUtils.o: ../global/Error.h /usr/include/stdio.h
DataFlowMaskUtils.o: /usr/include/sys/feature_tests.h
DataFlowMaskUtils.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
DataFlowMaskUtils.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
DataFlowMaskUtils.o: /usr/include/string.h /usr/include/stdlib.h
DataFlowMaskUtils.o: ../global/globals.h ../global/internal_lib.h free.h
DataFlowMaskUtils.o: ../../inc/dbug.h ../global/my_debug.h DupTree.h
DataFlowMaskUtils.o: LookUpTable.h DataFlowMask.h ../typecheck/typecheck.h
DupTree.o: /usr/include/string.h /usr/include/sys/feature_tests.h
DupTree.o: /usr/include/sys/isa_defs.h ../global/types.h tree_basic.h
DupTree.o: tree_compound.h ../global/Error.h /usr/include/stdio.h
DupTree.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
DupTree.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
DupTree.o: ../global/globals.h ../global/internal_lib.h traverse.h free.h
DupTree.o: ../../inc/dbug.h ../global/my_debug.h DupTree.h LookUpTable.h
DupTree.o: DataFlowMask.h DataFlowMaskUtils.h scheduling.h
DupTree.o: ../optimize/generatemasks.h ../constants/constants.h
DupTree.o: ../constants/shape.h
InferDFMs.o: ../global/types.h tree_basic.h tree_compound.h ../global/Error.h
InferDFMs.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
InferDFMs.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
InferDFMs.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
InferDFMs.o: /usr/include/string.h /usr/include/stdlib.h ../global/globals.h
InferDFMs.o: ../global/internal_lib.h traverse.h free.h ../../inc/dbug.h
InferDFMs.o: ../global/my_debug.h DupTree.h LookUpTable.h DataFlowMask.h
InferDFMs.o: DataFlowMaskUtils.h InferDFMs.h
LookUpTable.o: /usr/include/string.h /usr/include/sys/feature_tests.h
LookUpTable.o: /usr/include/sys/isa_defs.h ../global/internal_lib.h
LookUpTable.o: ../global/types.h free.h /usr/include/stdlib.h
LookUpTable.o: ../../inc/dbug.h /usr/include/stdio.h
LookUpTable.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
LookUpTable.o: /usr/include/stdio_impl.h ../global/my_debug.h
LookUpTable.o: ../global/globals.h
SSATransform.o: ../global/types.h tree_basic.h tree_compound.h
SSATransform.o: ../global/Error.h /usr/include/stdio.h
SSATransform.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
SSATransform.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
SSATransform.o: /usr/include/stdio_impl.h /usr/include/string.h
SSATransform.o: /usr/include/stdlib.h ../global/globals.h
SSATransform.o: ../global/internal_lib.h ../../inc/dbug.h traverse.h free.h
SSATransform.o: ../global/my_debug.h DupTree.h LookUpTable.h SSATransform.h
UndoSSATransform.o: ../global/types.h tree_basic.h tree_compound.h
UndoSSATransform.o: ../global/Error.h /usr/include/stdio.h
UndoSSATransform.o: /usr/include/sys/feature_tests.h
UndoSSATransform.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
UndoSSATransform.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
UndoSSATransform.o: /usr/include/string.h /usr/include/stdlib.h
UndoSSATransform.o: ../global/globals.h ../global/internal_lib.h
UndoSSATransform.o: ../../inc/dbug.h traverse.h free.h ../global/my_debug.h
UndoSSATransform.o: UndoSSATransform.h DupTree.h LookUpTable.h
adjust_ids.o: ../../inc/dbug.h /usr/include/stdio.h
adjust_ids.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
adjust_ids.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
adjust_ids.o: /usr/include/stdio_impl.h ../global/types.h tree_basic.h
adjust_ids.o: tree_compound.h ../global/Error.h /usr/include/string.h
adjust_ids.o: /usr/include/stdlib.h ../global/globals.h
adjust_ids.o: ../global/internal_lib.h traverse.h free.h ../global/my_debug.h
adjust_ids.o: DupTree.h LookUpTable.h ../refcount/refcount.h ../print/print.h
change_signature.o: ../global/types.h tree_basic.h tree_compound.h
change_signature.o: ../global/Error.h /usr/include/stdio.h
change_signature.o: /usr/include/sys/feature_tests.h
change_signature.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
change_signature.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
change_signature.o: /usr/include/string.h /usr/include/stdlib.h
change_signature.o: ../global/globals.h ../global/internal_lib.h
change_signature.o: ../../inc/dbug.h free.h ../global/my_debug.h DupTree.h
change_signature.o: LookUpTable.h ../optimize/optimize.h
cleanup_decls.o: ../global/types.h tree_basic.h tree_compound.h
cleanup_decls.o: ../global/Error.h /usr/include/stdio.h
cleanup_decls.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
cleanup_decls.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
cleanup_decls.o: /usr/include/stdio_impl.h /usr/include/string.h
cleanup_decls.o: /usr/include/stdlib.h ../global/globals.h
cleanup_decls.o: ../global/internal_lib.h traverse.h free.h ../../inc/dbug.h
cleanup_decls.o: ../global/my_debug.h DataFlowMask.h DataFlowMaskUtils.h
cleanup_decls.o: LookUpTable.h
compare_tree.o: ../global/types.h tree_basic.h tree_compound.h
compare_tree.o: ../global/Error.h /usr/include/stdio.h
compare_tree.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
compare_tree.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
compare_tree.o: /usr/include/stdio_impl.h /usr/include/string.h
compare_tree.o: /usr/include/stdlib.h ../global/globals.h
compare_tree.o: ../global/internal_lib.h ../../inc/dbug.h traverse.h free.h
compare_tree.o: ../global/my_debug.h compare_tree.h ../typecheck/typecheck.h
free.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
free.o: /usr/include/sys/isa_defs.h /usr/include/stdio.h
free.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
free.o: /usr/include/stdio_impl.h ../../inc/dbug.h ../global/my_debug.h
free.o: ../global/types.h tree_basic.h tree_compound.h ../global/Error.h
free.o: /usr/include/string.h ../global/globals.h ../global/internal_lib.h
free.o: traverse.h DataFlowMask.h scheduling.h ../constants/constants.h
free.o: ../constants/shape.h free.h
scheduling.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
scheduling.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
scheduling.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
scheduling.o: /usr/include/string.h /usr/include/varargs.h ../global/types.h
scheduling.o: tree_basic.h tree_compound.h ../global/Error.h
scheduling.o: /usr/include/stdlib.h ../global/globals.h
scheduling.o: ../global/internal_lib.h free.h ../../inc/dbug.h
scheduling.o: ../global/my_debug.h traverse.h ../compile/precompile.h
scheduling.o: wl_bounds.h
traverse.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
traverse.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
traverse.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
traverse.o: /usr/include/stdlib.h ../global/types.h tree_basic.h
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
traverse.o: ../objects/uniquecheck.h ../refcount/refcount.h
traverse.o: ../compile/wltransform.h ../compile/precompile.h
traverse.o: ../compile/compile.h ../compile/ReuseWithArrays.h
traverse.o: ../modules/cccall.h ../compile/PatchWith.h
traverse.o: ../psi-opt/WithloopFolding.h ../psi-opt/WLT.h ../psi-opt/WLI.h
traverse.o: ../psi-opt/WLF.h ../compile/gen_startup_code.h scheduling.h
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
traverse.o: UndoSSATransform.h ../optimize/SSADeadCodeRemoval.h
traverse.o: ../optimize/SSACSE.h compare_tree.h
traverse.o: ../profile/annotate_fun_calls.h ../optimize/SSAConstantFolding.h
traverse.o: ../optimize/SSALIR.h ../flatten/while2do.h ../optimize/SSALUR.h
traverse.o: traverse.h
tree.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
tree.o: /usr/include/sys/isa_defs.h tree.h ../global/types.h tree_basic.h
tree.o: tree_compound.h ../global/Error.h /usr/include/stdio.h
tree.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
tree.o: /usr/include/stdio_impl.h /usr/include/string.h ../global/globals.h
tree.o: ../global/internal_lib.h ../../inc/dbug.h ../global/my_debug.h
tree_basic.o: ../global/types.h tree_basic.h ../global/Error.h
tree_basic.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
tree_basic.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
tree_basic.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
tree_basic.o: /usr/include/string.h /usr/include/stdlib.h ../global/globals.h
tree_basic.o: ../../inc/dbug.h ../global/my_debug.h free.h
tree_basic.o: ../global/internal_lib.h ../refcount/refcount.h
tree_basic.o: prf_node_info.mac
tree_compound.o: ../global/types.h tree_basic.h tree_compound.h
tree_compound.o: ../global/Error.h /usr/include/stdio.h
tree_compound.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
tree_compound.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
tree_compound.o: /usr/include/stdio_impl.h /usr/include/string.h
tree_compound.o: /usr/include/stdlib.h ../global/globals.h DupTree.h
tree_compound.o: LookUpTable.h ../../inc/dbug.h ../global/my_debug.h free.h
tree_compound.o: ../global/internal_lib.h ../typecheck/typecheck.h
tree_compound.o: ../compile/wltransform.h
wl_bounds.o: wl_bounds.h ../global/types.h tree_basic.h tree_compound.h
wl_bounds.o: ../global/Error.h /usr/include/stdio.h
wl_bounds.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
wl_bounds.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
wl_bounds.o: /usr/include/stdio_impl.h /usr/include/string.h
wl_bounds.o: /usr/include/stdlib.h ../global/globals.h
wl_bounds.o: ../global/internal_lib.h ../../inc/dbug.h free.h
wl_bounds.o: ../global/my_debug.h DupTree.h LookUpTable.h
