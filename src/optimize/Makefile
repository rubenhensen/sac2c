#
# $Log$
# Revision 2.24  2000/10/12 15:48:19  dkr
# make deps done
#
# Revision 2.23  2000/08/17 10:17:31  dkr
# make deps done
#
# Revision 2.22  2000/07/28 09:16:22  mab
# make deps done
#
# Revision 2.21  2000/07/11 15:56:23  dkr
# make deps done
#
# Revision 2.20  2000/07/11 15:52:12  dkr
# explicit *.o rules removed
#
# Revision 2.19  2000/05/29 14:36:38  dkr
# make deps done
#
# Revision 2.18  2000/05/29 14:35:13  dkr
# make deps done
#
# Revision 2.17  2000/03/31 09:33:43  jhs
# make deps done
#
# Revision 2.16  2000/03/31 09:22:18  cg
# make deps done
#
# Revision 2.15  2000/03/01 14:52:35  dkr
# make deps done
#
# Revision 2.14  2000/01/23 15:01:13  dkr
# make deps done
#
# Revision 2.13  2000/01/21 17:02:06  dkr
# make deps done
#
# Revision 2.12  2000/01/21 12:04:29  dkr
# directory tree added
#
# Revision 2.11  2000/01/17 18:40:59  cg
# make deps done
#
# Revision 2.10  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 2.8  1999/07/09 11:38:47  cg
# make deps done
#
# Revision 2.7  1999/06/01 15:17:04  dkr
# make deps done
#
# Revision 2.6  1999/05/12 15:13:25  cg
# make deps done
#
# Revision 2.5  1999/05/10 10:48:10  bs
# make deps done
#
# Revision 2.4  1999/03/15 21:15:30  dkr
# make deps done
#
# Revision 2.3  1999/03/11 18:18:22  dkr
# make deps done
#
# Revision 2.2  1999/02/26 14:17:40  dkr
# make deps done
#
# Revision 2.1  1999/02/23 12:41:25  sacbase
# new release made
#
# Revision 1.57  1999/02/09 20:04:21  dkr
# make deps done
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
# Revision 1.53  1998/11/04 18:22:34  dkr
# make deps done
#
# Revision 1.52  1998/06/23 11:42:10  cg
# make deps done
#
# Revision 1.51  1998/06/17 14:47:39  sbs
# further includes added 8-)
#
# Revision 1.50  1998/05/21 09:51:57  dkr
# make deps done
#
# Revision 1.49  1998/05/13 14:07:29  srs
# added WLUnroll
#
# Revision 1.48  1998/05/12 14:22:21  dkr
# make deps done
#
# Revision 1.47  1998/05/05 15:51:42  cg
# added compilation of DataFlowMask.c
# flag NEWTREE is now set locally as well.
#
# Revision 1.46  1998/04/20 12:08:47  dkr
# make deps done
#
# Revision 1.45  1998/03/25 17:03:47  srs
# made deps
#
# Revision 1.44  1998/03/22 18:03:15  srs
# added rules for WLT, WLI, WLF
#
# Revision 1.43  1998/03/19 17:08:35  dkr
# make deps done
#
# Revision 1.42  1998/03/13 12:55:29  dkr
# make deps done
#
# Revision 1.41  1998/03/10 17:30:16  srs
# made deps
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
     -I../compile -I../concurrent -I../tree

all: optimize.o generatemasks.o ConstantFolding.o DeadCodeRemoval.o \
     LoopInvariantRemoval.o Inline.o Unroll.o Unswitch.o CSE.o \
     WLUnroll.o DeadFunctionRemoval.o freemasks.o

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
CSE.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
CSE.o: /usr/include/malloc.h /usr/include/sys/types.h
CSE.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
CSE.o: ../print/print.h ../tree/traverse.h ../typecheck/typecheck.h
CSE.o: optimize.h generatemasks.h ../tree/DupTree.h ../tree/LookUpTable.h
CSE.o: CSE.h
ConstantFolding.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
ConstantFolding.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
ConstantFolding.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
ConstantFolding.o: /usr/include/stdlib.h ../tree/tree.h ../global/types.h
ConstantFolding.o: ../tree/tree_basic.h ../tree/tree_compound.h
ConstantFolding.o: ../global/Error.h /usr/include/string.h
ConstantFolding.o: ../global/globals.h ../global/internal_lib.h
ConstantFolding.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
ConstantFolding.o: /usr/include/malloc.h /usr/include/sys/types.h
ConstantFolding.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
ConstantFolding.o: ../tree/prf.h ../print/print.h ../tree/traverse.h
ConstantFolding.o: ../tree/access_macros.h ../typecheck/typecheck.h
ConstantFolding.o: optimize.h generatemasks.h ../tree/DupTree.h
ConstantFolding.o: ../tree/LookUpTable.h LoopInvariantRemoval.h
ConstantFolding.o: ConstantFolding.h
DeadCodeRemoval.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
DeadCodeRemoval.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
DeadCodeRemoval.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
DeadCodeRemoval.o: /usr/include/stdlib.h ../tree/tree.h ../global/types.h
DeadCodeRemoval.o: ../tree/tree_basic.h ../tree/tree_compound.h
DeadCodeRemoval.o: ../global/Error.h /usr/include/string.h
DeadCodeRemoval.o: ../global/globals.h ../global/internal_lib.h
DeadCodeRemoval.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
DeadCodeRemoval.o: /usr/include/malloc.h /usr/include/sys/types.h
DeadCodeRemoval.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
DeadCodeRemoval.o: ../tree/traverse.h optimize.h generatemasks.h
DeadCodeRemoval.o: DeadCodeRemoval.h
DeadFunctionRemoval.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
DeadFunctionRemoval.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
DeadFunctionRemoval.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
DeadFunctionRemoval.o: /usr/include/stdlib.h ../tree/tree.h ../global/types.h
DeadFunctionRemoval.o: ../tree/tree_basic.h ../tree/tree_compound.h
DeadFunctionRemoval.o: ../global/Error.h /usr/include/string.h
DeadFunctionRemoval.o: ../global/globals.h ../global/internal_lib.h
DeadFunctionRemoval.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
DeadFunctionRemoval.o: /usr/include/malloc.h /usr/include/sys/types.h
DeadFunctionRemoval.o: /usr/include/sys/machtypes.h
DeadFunctionRemoval.o: /usr/include/sys/int_types.h ../tree/traverse.h
DeadFunctionRemoval.o: optimize.h
Inline.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
Inline.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
Inline.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
Inline.o: /usr/include/stdlib.h ../global/globals.h ../global/types.h
Inline.o: ../tree/tree.h ../tree/tree_basic.h ../tree/tree_compound.h
Inline.o: ../global/Error.h /usr/include/string.h ../global/internal_lib.h
Inline.o: ../../inc/dbug.h ../global/my_debug.h ../tree/traverse.h
Inline.o: ../tree/free.h /usr/include/malloc.h /usr/include/sys/types.h
Inline.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
Inline.o: ../typecheck/typecheck.h optimize.h LoopInvariantRemoval.h
Inline.o: ../tree/DupTree.h ../tree/LookUpTable.h Inline.h
LoopInvariantRemoval.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
LoopInvariantRemoval.o: /usr/include/sys/isa_defs.h
LoopInvariantRemoval.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
LoopInvariantRemoval.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
LoopInvariantRemoval.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
LoopInvariantRemoval.o: ../tree/tree_compound.h ../global/Error.h
LoopInvariantRemoval.o: /usr/include/string.h ../global/globals.h
LoopInvariantRemoval.o: ../global/internal_lib.h ../tree/free.h
LoopInvariantRemoval.o: ../../inc/dbug.h ../global/my_debug.h
LoopInvariantRemoval.o: /usr/include/malloc.h /usr/include/sys/types.h
LoopInvariantRemoval.o: /usr/include/sys/machtypes.h
LoopInvariantRemoval.o: /usr/include/sys/int_types.h ../tree/traverse.h
LoopInvariantRemoval.o: ../typecheck/typecheck.h ../tree/access_macros.h
LoopInvariantRemoval.o: optimize.h generatemasks.h ../tree/DupTree.h
LoopInvariantRemoval.o: ../tree/LookUpTable.h LoopInvariantRemoval.h
Unroll.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
Unroll.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
Unroll.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
Unroll.o: /usr/include/stdlib.h ../global/globals.h ../global/types.h
Unroll.o: ../tree/tree.h ../tree/tree_basic.h ../tree/tree_compound.h
Unroll.o: ../global/Error.h /usr/include/string.h ../global/internal_lib.h
Unroll.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
Unroll.o: /usr/include/malloc.h /usr/include/sys/types.h
Unroll.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
Unroll.o: ../tree/traverse.h ../tree/access_macros.h optimize.h
Unroll.o: generatemasks.h ConstantFolding.h ../tree/DupTree.h
Unroll.o: ../tree/LookUpTable.h Unroll.h WLUnroll.h
Unswitch.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
Unswitch.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
Unswitch.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
Unswitch.o: /usr/include/stdlib.h ../global/globals.h ../global/types.h
Unswitch.o: ../tree/tree.h ../tree/tree_basic.h ../tree/tree_compound.h
Unswitch.o: ../global/Error.h /usr/include/string.h ../global/internal_lib.h
Unswitch.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
Unswitch.o: /usr/include/malloc.h /usr/include/sys/types.h
Unswitch.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
Unswitch.o: ../tree/traverse.h ../tree/access_macros.h optimize.h
Unswitch.o: generatemasks.h ConstantFolding.h ../tree/DupTree.h
Unswitch.o: ../tree/LookUpTable.h Unroll.h Unswitch.h
WLUnroll.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WLUnroll.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
WLUnroll.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
WLUnroll.o: /usr/include/stdlib.h ../global/globals.h ../global/types.h
WLUnroll.o: ../tree/tree.h ../tree/tree_basic.h ../tree/tree_compound.h
WLUnroll.o: ../global/Error.h /usr/include/string.h ../global/internal_lib.h
WLUnroll.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
WLUnroll.o: /usr/include/malloc.h /usr/include/sys/types.h
WLUnroll.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
WLUnroll.o: ../tree/traverse.h ../typecheck/typecheck.h ../compile/compile.h
WLUnroll.o: optimize.h Inline.h ../tree/DupTree.h ../tree/LookUpTable.h
WLUnroll.o: ../psi-opt/WithloopFolding.h
WorkReduction.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WorkReduction.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
WorkReduction.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
WorkReduction.o: /usr/include/stdlib.h ../tree/tree.h ../global/types.h
WorkReduction.o: ../tree/tree_basic.h ../tree/tree_compound.h
WorkReduction.o: ../global/Error.h /usr/include/string.h ../global/globals.h
WorkReduction.o: ../global/internal_lib.h ../../inc/dbug.h
WorkReduction.o: ../global/my_debug.h ../tree/traverse.h optimize.h
WorkReduction.o: ../tree/free.h /usr/include/malloc.h
WorkReduction.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
WorkReduction.o: /usr/include/sys/int_types.h WorkReduction.h
freemasks.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
freemasks.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
freemasks.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
freemasks.o: /usr/include/stdlib.h /usr/include/string.h ../tree/tree.h
freemasks.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
freemasks.o: ../global/Error.h ../global/globals.h ../global/internal_lib.h
freemasks.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
freemasks.o: /usr/include/malloc.h /usr/include/sys/types.h
freemasks.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
freemasks.o: ../tree/traverse.h
generatemasks.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
generatemasks.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
generatemasks.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
generatemasks.o: /usr/include/stdlib.h /usr/include/string.h ../tree/tree.h
generatemasks.o: ../global/types.h ../tree/tree_basic.h
generatemasks.o: ../tree/tree_compound.h ../global/Error.h
generatemasks.o: ../global/globals.h ../global/internal_lib.h ../tree/free.h
generatemasks.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
generatemasks.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
generatemasks.o: /usr/include/sys/int_types.h ../tree/traverse.h
generatemasks.o: ../print/print.h optimize.h generatemasks.h
generatemasks.o: ConstantFolding.h DeadCodeRemoval.h LoopInvariantRemoval.h
generatemasks.o: Inline.h Unroll.h Unswitch.h ../psi-opt/ArrayElimination.h
generatemasks.o: CSE.h ../psi-opt/WithloopFolding.h
optimize.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
optimize.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
optimize.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
optimize.o: /usr/include/stdlib.h /usr/include/string.h ../tree/tree.h
optimize.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
optimize.o: ../global/Error.h ../global/globals.h ../global/internal_lib.h
optimize.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
optimize.o: /usr/include/malloc.h /usr/include/sys/types.h
optimize.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
optimize.o: ../tree/traverse.h ../print/print.h ../print/convert.h optimize.h
optimize.o: generatemasks.h freemasks.h ConstantFolding.h DeadCodeRemoval.h
optimize.o: DeadFunctionRemoval.h LoopInvariantRemoval.h Inline.h Unroll.h
optimize.o: Unswitch.h ../psi-opt/ArrayElimination.h CSE.h
optimize.o: ../psi-opt/WithloopFolding.h ../psi-opt/wl_access_analyze.h
optimize.o: ../psi-opt/tile_size_inference.h ../psi-opt/index.h
optimize.o: ../psi-opt/pad.h
