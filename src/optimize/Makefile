#
#
# $Log$
# Revision 3.20  2001/05/30 14:00:40  nmw
# SSAInferLI.o added
#
# Revision 3.19  2001/05/22 14:52:18  nmw
# rmcasts.o added
#
# Revision 3.18  2001/05/08 13:29:33  dkr
# make deps done
#
# Revision 3.17  2001/04/26 21:09:44  dkr
# make deps done
#
# Revision 3.16  2001/04/20 11:17:07  nmw
# SSALUR.o added
#
# Revision 3.15  2001/03/26 15:35:35  nmw
# SSALIR added
#
# Revision 3.14  2001/03/22 20:20:00  dkr
# make deps done
#
# Revision 3.13  2001/03/22 20:19:00  dkr
# INCS corrected
#
# Revision 3.12  2001/03/21 15:51:38  nmw
# include ../constants added
#
# Revision 3.11  2001/03/21 11:19:55  dkr
# make deps done
#
# Revision 3.10  2001/03/20 16:17:00  nmw
# SSAConstantFolding.o added
#
# Revision 3.9  2001/03/09 11:53:40  sbs
# make deps done
#
# Revision 3.8  2001/03/05 16:03:23  nmw
# SSACSE added
#
# Revision 3.7  2001/03/02 16:13:52  dkr
# make deps done
#
# Revision 3.6  2001/02/23 13:38:39  nmw
# SSADeadCodeRemoval added
#
# Revision 3.5  2001/02/13 15:54:04  nmw
# include for lac2fun.h added
#
# Revision 3.4  2001/02/02 10:39:01  dkr
# make deps done
#
# Revision 3.3  2001/01/17 10:13:03  dkr
# make deps done
#
# Revision 3.2  2000/12/06 20:09:02  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:00:34  sacbase
# new release made
#
# Revision 2.25  2000/11/02 14:43:26  dkr
# header compressed
#
# Revision 2.20  2000/07/11 15:52:12  dkr
# explicit *.o rules removed
#
# Revision 2.12  2000/01/21 12:04:29  dkr
# directory tree added
#
# Revision 2.10  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 1.56  1999/01/26 15:37:22  sbs
# -DNEWTREE eliminated from CFLAGS
#
# Revision 1.55  1999/01/15 16:59:24  sbs
# rule for freemasks.o added
#
# Revision 1.54  1999/01/07 13:55:20  sbs
# generatemasks and DeadFunctionRemoval inserted.
#
# Revision 1.51  1998/06/17 14:47:39  sbs
# further includes added 8-)
#
# Revision 1.49  1998/05/13 14:07:29  srs
# added WLUnroll
#
# Revision 1.47  1998/05/05 15:51:42  cg
# added compilation of DataFlowMask.c
# flag NEWTREE is now set locally as well.
#
# Revision 1.44  1998/03/22 18:03:15  srs
# added rules for WLT, WLI, WLF
#
# ... [eliminated]
#
# Revision 1.1  1994/12/09  10:48:23  sbs
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


INCS=-I../global -I../../inc -I../typecheck -I../print -I../psi-opt -I../modules \
     -I../compile -I../concurrent -I../tree -I../flatten -I../constants

all: optimize.o generatemasks.o ConstantFolding.o DeadCodeRemoval.o \
     LoopInvariantRemoval.o Inline.o Unroll.o Unswitch.o CSE.o \
     WLUnroll.o DeadFunctionRemoval.o freemasks.o \
     SSADeadCodeRemoval.o SSACSE.o SSAConstantFolding.o SSALIR.o SSALUR.o \
     rmcasts.o SSAInferLI.o

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

CSE.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
CSE.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
CSE.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
CSE.o: /usr/include/stdlib.h ../tree/tree.h ../global/types.h
CSE.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
CSE.o: /usr/include/string.h ../global/globals.h ../global/internal_lib.h
CSE.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h ../print/print.h
CSE.o: ../tree/traverse.h ../typecheck/typecheck.h optimize.h generatemasks.h
CSE.o: ../tree/DupTree.h ../tree/LookUpTable.h CSE.h
ConstantFolding.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
ConstantFolding.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
ConstantFolding.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
ConstantFolding.o: /usr/include/stdlib.h ../tree/tree.h ../global/types.h
ConstantFolding.o: ../tree/tree_basic.h ../tree/tree_compound.h
ConstantFolding.o: ../global/Error.h /usr/include/string.h
ConstantFolding.o: ../global/globals.h ../global/internal_lib.h
ConstantFolding.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
ConstantFolding.o: ../tree/prf.h ../print/print.h ../tree/traverse.h
ConstantFolding.o: ../typecheck/typecheck.h optimize.h generatemasks.h
ConstantFolding.o: ../tree/DupTree.h ../tree/LookUpTable.h
ConstantFolding.o: LoopInvariantRemoval.h ConstantFolding.h
DeadCodeRemoval.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
DeadCodeRemoval.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
DeadCodeRemoval.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
DeadCodeRemoval.o: /usr/include/stdlib.h ../global/types.h
DeadCodeRemoval.o: ../tree/tree_basic.h ../tree/tree_compound.h
DeadCodeRemoval.o: ../global/Error.h /usr/include/string.h
DeadCodeRemoval.o: ../global/globals.h ../global/internal_lib.h
DeadCodeRemoval.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
DeadCodeRemoval.o: ../tree/traverse.h optimize.h generatemasks.h
DeadCodeRemoval.o: DeadCodeRemoval.h
DeadFunctionRemoval.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
DeadFunctionRemoval.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
DeadFunctionRemoval.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
DeadFunctionRemoval.o: /usr/include/stdlib.h ../global/types.h
DeadFunctionRemoval.o: ../tree/tree_basic.h ../tree/tree_compound.h
DeadFunctionRemoval.o: ../global/Error.h /usr/include/string.h
DeadFunctionRemoval.o: ../global/globals.h ../global/internal_lib.h
DeadFunctionRemoval.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
DeadFunctionRemoval.o: ../tree/traverse.h optimize.h
Inline.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
Inline.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
Inline.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
Inline.o: /usr/include/stdlib.h ../global/globals.h ../global/types.h
Inline.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
Inline.o: /usr/include/string.h ../../inc/dbug.h ../global/my_debug.h
Inline.o: ../tree/traverse.h ../tree/free.h ../global/internal_lib.h
Inline.o: ../typecheck/typecheck.h ../tree/InferDFMs.h optimize.h
Inline.o: LoopInvariantRemoval.h ../tree/DupTree.h ../tree/LookUpTable.h
Inline.o: Inline.h
LoopInvariantRemoval.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
LoopInvariantRemoval.o: /usr/include/sys/isa_defs.h
LoopInvariantRemoval.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
LoopInvariantRemoval.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
LoopInvariantRemoval.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
LoopInvariantRemoval.o: ../tree/tree_compound.h ../global/Error.h
LoopInvariantRemoval.o: /usr/include/string.h ../global/globals.h
LoopInvariantRemoval.o: ../global/internal_lib.h ../tree/free.h
LoopInvariantRemoval.o: ../../inc/dbug.h ../global/my_debug.h
LoopInvariantRemoval.o: ../tree/traverse.h ../typecheck/typecheck.h
LoopInvariantRemoval.o: optimize.h generatemasks.h ../tree/DupTree.h
LoopInvariantRemoval.o: ../tree/LookUpTable.h LoopInvariantRemoval.h
SSACSE.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
SSACSE.o: ../global/Error.h /usr/include/stdio.h
SSACSE.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
SSACSE.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
SSACSE.o: /usr/include/stdio_impl.h /usr/include/string.h
SSACSE.o: /usr/include/stdlib.h ../global/globals.h ../global/internal_lib.h
SSACSE.o: ../../inc/dbug.h ../tree/traverse.h ../tree/free.h
SSACSE.o: ../global/my_debug.h ../tree/DupTree.h ../tree/LookUpTable.h
SSACSE.o: SSACSE.h ../tree/compare_tree.h optimize.h ../typecheck/typecheck.h
SSAConstantFolding.o: ../../inc/dbug.h /usr/include/stdio.h
SSAConstantFolding.o: /usr/include/sys/feature_tests.h
SSAConstantFolding.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
SSAConstantFolding.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
SSAConstantFolding.o: ../global/types.h ../tree/tree_basic.h
SSAConstantFolding.o: ../tree/tree_compound.h ../global/Error.h
SSAConstantFolding.o: /usr/include/string.h /usr/include/stdlib.h
SSAConstantFolding.o: ../global/globals.h ../global/internal_lib.h
SSAConstantFolding.o: ../tree/traverse.h ../tree/free.h ../global/my_debug.h
SSAConstantFolding.o: ../tree/DupTree.h ../tree/LookUpTable.h
SSAConstantFolding.o: SSAConstantFolding.h ../constants/constants.h
SSAConstantFolding.o: ../constants/shape.h optimize.h ../tree/SSATransform.h
SSAConstantFolding.o: Inline.h ../tree/compare_tree.h
SSAConstantFolding.o: ../tree/prf_node_info.mac
SSADeadCodeRemoval.o: ../global/types.h ../tree/tree_basic.h
SSADeadCodeRemoval.o: ../tree/tree_compound.h ../global/Error.h
SSADeadCodeRemoval.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
SSADeadCodeRemoval.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
SSADeadCodeRemoval.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
SSADeadCodeRemoval.o: /usr/include/string.h /usr/include/stdlib.h
SSADeadCodeRemoval.o: ../global/globals.h ../global/internal_lib.h
SSADeadCodeRemoval.o: ../../inc/dbug.h ../tree/traverse.h ../tree/free.h
SSADeadCodeRemoval.o: ../global/my_debug.h ../tree/DupTree.h
SSADeadCodeRemoval.o: ../tree/LookUpTable.h SSADeadCodeRemoval.h optimize.h
SSADeadCodeRemoval.o: ../tree/change_signature.h
SSAInferLI.o: ../../inc/dbug.h /usr/include/stdio.h
SSAInferLI.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
SSAInferLI.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
SSAInferLI.o: /usr/include/stdio_impl.h ../tree/tree_basic.h
SSAInferLI.o: ../global/types.h ../tree/tree_compound.h ../global/Error.h
SSAInferLI.o: /usr/include/string.h /usr/include/stdlib.h ../global/globals.h
SSAInferLI.o: ../tree/traverse.h SSAInferLI.h
SSALIR.o: ../../inc/dbug.h /usr/include/stdio.h
SSALIR.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
SSALIR.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
SSALIR.o: /usr/include/stdio_impl.h ../tree/tree_basic.h ../global/types.h
SSALIR.o: ../global/internal_lib.h ../tree/traverse.h ../tree/free.h
SSALIR.o: /usr/include/stdlib.h ../global/my_debug.h ../global/globals.h
SSALIR.o: ../tree/DupTree.h ../tree/LookUpTable.h optimize.h SSALIR.h
SSALIR.o: ../tree/tree_compound.h ../global/Error.h /usr/include/string.h
SSALIR.o: ../tree/change_signature.h ../tree/SSATransform.h
SSALUR.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
SSALUR.o: ../global/Error.h /usr/include/stdio.h
SSALUR.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
SSALUR.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
SSALUR.o: /usr/include/stdio_impl.h /usr/include/string.h
SSALUR.o: /usr/include/stdlib.h ../global/globals.h ../global/internal_lib.h
SSALUR.o: ../../inc/dbug.h ../tree/traverse.h ../tree/free.h
SSALUR.o: ../global/my_debug.h ../tree/DupTree.h ../tree/LookUpTable.h
SSALUR.o: SSALUR.h optimize.h ../constants/constants.h ../constants/shape.h
SSALUR.o: /usr/include/math.h ../tree/SSATransform.h ../tree/CheckAvis.h
SSALUR.o: WLUnroll.h ../tree/prf_node_info.mac
Unroll.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
Unroll.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
Unroll.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
Unroll.o: /usr/include/stdlib.h ../tree/tree.h ../global/types.h
Unroll.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
Unroll.o: /usr/include/string.h ../global/globals.h ../global/internal_lib.h
Unroll.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
Unroll.o: ../tree/traverse.h optimize.h generatemasks.h ConstantFolding.h
Unroll.o: ../tree/DupTree.h ../tree/LookUpTable.h Unroll.h WLUnroll.h
Unswitch.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
Unswitch.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
Unswitch.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
Unswitch.o: /usr/include/stdlib.h ../tree/tree.h ../global/types.h
Unswitch.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
Unswitch.o: /usr/include/string.h ../global/globals.h
Unswitch.o: ../global/internal_lib.h ../tree/free.h ../../inc/dbug.h
Unswitch.o: ../global/my_debug.h ../tree/traverse.h optimize.h
Unswitch.o: generatemasks.h ConstantFolding.h ../tree/DupTree.h
Unswitch.o: ../tree/LookUpTable.h Unroll.h Unswitch.h
WLUnroll.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WLUnroll.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
WLUnroll.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
WLUnroll.o: /usr/include/stdlib.h ../global/types.h ../tree/tree_basic.h
WLUnroll.o: ../tree/tree_compound.h ../global/Error.h /usr/include/string.h
WLUnroll.o: ../global/globals.h ../global/internal_lib.h ../tree/free.h
WLUnroll.o: ../../inc/dbug.h ../global/my_debug.h ../tree/traverse.h
WLUnroll.o: ../typecheck/typecheck.h optimize.h Inline.h ../tree/DupTree.h
WLUnroll.o: ../tree/LookUpTable.h ../psi-opt/WithloopFolding.h
WorkReduction.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WorkReduction.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
WorkReduction.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
WorkReduction.o: /usr/include/stdlib.h ../global/types.h ../tree/tree_basic.h
WorkReduction.o: ../tree/tree_compound.h ../global/Error.h
WorkReduction.o: /usr/include/string.h ../global/globals.h
WorkReduction.o: ../global/internal_lib.h ../../inc/dbug.h
WorkReduction.o: ../global/my_debug.h ../tree/traverse.h optimize.h
WorkReduction.o: ../tree/free.h WorkReduction.h
freemasks.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
freemasks.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
freemasks.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
freemasks.o: /usr/include/stdlib.h /usr/include/string.h ../global/types.h
freemasks.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
freemasks.o: ../global/globals.h ../global/internal_lib.h ../tree/free.h
freemasks.o: ../../inc/dbug.h ../global/my_debug.h ../tree/traverse.h
generatemasks.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
generatemasks.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
generatemasks.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
generatemasks.o: /usr/include/stdlib.h /usr/include/string.h ../tree/tree.h
generatemasks.o: ../global/types.h ../tree/tree_basic.h
generatemasks.o: ../tree/tree_compound.h ../global/Error.h
generatemasks.o: ../global/globals.h ../global/internal_lib.h ../tree/free.h
generatemasks.o: ../../inc/dbug.h ../global/my_debug.h ../tree/traverse.h
generatemasks.o: ../print/print.h optimize.h generatemasks.h
generatemasks.o: ConstantFolding.h DeadCodeRemoval.h LoopInvariantRemoval.h
generatemasks.o: Inline.h Unroll.h Unswitch.h ../psi-opt/ArrayElimination.h
generatemasks.o: CSE.h ../psi-opt/WithloopFolding.h
optimize.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
optimize.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
optimize.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
optimize.o: /usr/include/stdlib.h /usr/include/string.h ../global/types.h
optimize.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
optimize.o: ../global/globals.h ../global/internal_lib.h ../tree/free.h
optimize.o: ../../inc/dbug.h ../global/my_debug.h ../tree/traverse.h
optimize.o: ../print/print.h ../print/convert.h optimize.h generatemasks.h
optimize.o: freemasks.h ConstantFolding.h DeadCodeRemoval.h
optimize.o: DeadFunctionRemoval.h LoopInvariantRemoval.h Inline.h Unroll.h
optimize.o: Unswitch.h ../psi-opt/ArrayElimination.h CSE.h
optimize.o: ../psi-opt/WithloopFolding.h ../psi-opt/wl_access_analyze.h
optimize.o: ../psi-opt/tile_size_inference.h ../psi-opt/index.h
optimize.o: ../psi-opt/pad.h ../tree/CheckAvis.h ../tree/SSATransform.h
optimize.o: ../tree/UndoSSATransform.h ../flatten/lac2fun.h
optimize.o: ../flatten/fun2lac.h SSADeadCodeRemoval.h SSACSE.h
optimize.o: SSAConstantFolding.h SSALIR.h ../flatten/while2do.h SSALUR.h
optimize.o: ../psi-opt/SSAWithloopFolding.h rmcasts.h SSAInferLI.h
rmcasts.o: ../../inc/dbug.h /usr/include/stdio.h
rmcasts.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
rmcasts.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
rmcasts.o: /usr/include/stdio_impl.h ../global/types.h ../tree/tree_basic.h
rmcasts.o: ../tree/traverse.h ../tree/free.h /usr/include/stdlib.h
rmcasts.o: ../global/my_debug.h ../global/globals.h ../global/internal_lib.h
rmcasts.o: rmcasts.h
