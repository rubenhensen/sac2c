#
# $Log$
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

CC        := gcc
CCFLAGS   := -ansi -Wall -g
CFLAGS    := -D__EXTENSIONS__


INCS=-I../global -I../../inc -I../typecheck -I../print -I../psi-opt -I../modules \
     -I../compile -I../concurrent

all: optimize.o generatemasks.o ConstantFolding.o DeadCodeRemoval.o \
     LoopInvariantRemoval.o DupTree.o Inline.o Unroll.o Unswitch.o CSE.o \
     DataFlowMask.o WLUnroll.o DeadFunctionRemoval.o freemasks.o

freemasks.o: freemasks.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o freemasks.o -c freemasks.c $(INCS)

generatemasks.o: generatemasks.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o generatemasks.o -c generatemasks.c $(INCS)

optimize.o: optimize.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o optimize.o -c optimize.c $(INCS)

DupTree.o: DupTree.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o DupTree.o -c DupTree.c $(INCS)

ConstantFolding.o: ConstantFolding.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o ConstantFolding.o -c ConstantFolding.c $(INCS)

DeadCodeRemoval.o: DeadCodeRemoval.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o DeadCodeRemoval.o -c DeadCodeRemoval.c $(INCS)

DeadFunctionRemoval.o: DeadFunctionRemoval.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o DeadFunctionRemoval.o -c DeadFunctionRemoval.c $(INCS)

WorkReduction.o: WorkReduction.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o WorkReduction.o -c WorkReduction.c $(INCS)

LoopInvariantRemoval.o: LoopInvariantRemoval.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o LoopInvariantRemoval.o -c LoopInvariantRemoval.c $(INCS)

Inline.o: Inline.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o Inline.o -c Inline.c $(INCS)

Unroll.o: Unroll.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o Unroll.o -c Unroll.c $(INCS)

WLUnroll.o: WLUnroll.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o WLUnroll.o -c WLUnroll.c $(INCS)

Unswitch.o: Unswitch.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o Unswitch.o -c Unswitch.c $(INCS)

CSE.o: CSE.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o CSE.o -c CSE.c $(INCS)

DataFlowMask.o: DataFlowMask.c 
	$(CC) $(CCFLAGS) $(CFLAGS) -o DataFlowMask.o -c DataFlowMask.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
	$(RM) -r .sb

# DO NOT DELETE THIS LINE -- make depend depends on it.

CSE.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
CSE.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
CSE.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
CSE.o: /usr/include/stdlib.h ../global/tree.h ../global/types.h
CSE.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
CSE.o: /usr/include/string.h ../global/globals.h ../global/internal_lib.h
CSE.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
CSE.o: /usr/include/malloc.h /usr/include/sys/types.h
CSE.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
CSE.o: ../print/print.h ../global/traverse.h ../typecheck/typecheck.h
CSE.o: optimize.h generatemasks.h DupTree.h CSE.h
ConstantFolding.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
ConstantFolding.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
ConstantFolding.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
ConstantFolding.o: /usr/include/stdlib.h ../global/tree.h ../global/types.h
ConstantFolding.o: ../global/tree_basic.h ../global/tree_compound.h
ConstantFolding.o: ../global/Error.h /usr/include/string.h
ConstantFolding.o: ../global/globals.h ../global/internal_lib.h
ConstantFolding.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
ConstantFolding.o: /usr/include/malloc.h /usr/include/sys/types.h
ConstantFolding.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
ConstantFolding.o: ../print/print.h ../global/traverse.h
ConstantFolding.o: ../typecheck/typecheck.h ../global/access_macros.h
ConstantFolding.o: optimize.h generatemasks.h DupTree.h
ConstantFolding.o: LoopInvariantRemoval.h ConstantFolding.h
DataFlowMask.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
DataFlowMask.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
DataFlowMask.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
DataFlowMask.o: /usr/include/string.h ../../inc/dbug.h ../global/types.h
DataFlowMask.o: ../global/tree_basic.h ../global/tree_compound.h
DataFlowMask.o: ../global/Error.h /usr/include/stdlib.h ../global/globals.h
DataFlowMask.o: ../global/internal_lib.h ../global/free.h
DataFlowMask.o: ../global/my_debug.h /usr/include/malloc.h
DataFlowMask.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
DataFlowMask.o: /usr/include/sys/int_types.h
DeadCodeRemoval.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
DeadCodeRemoval.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
DeadCodeRemoval.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
DeadCodeRemoval.o: /usr/include/stdlib.h ../global/tree.h ../global/types.h
DeadCodeRemoval.o: ../global/tree_basic.h ../global/tree_compound.h
DeadCodeRemoval.o: ../global/Error.h /usr/include/string.h
DeadCodeRemoval.o: ../global/globals.h ../global/internal_lib.h
DeadCodeRemoval.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
DeadCodeRemoval.o: /usr/include/malloc.h /usr/include/sys/types.h
DeadCodeRemoval.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
DeadCodeRemoval.o: ../global/traverse.h optimize.h generatemasks.h
DeadCodeRemoval.o: DeadCodeRemoval.h
DeadFunctionRemoval.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
DeadFunctionRemoval.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
DeadFunctionRemoval.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
DeadFunctionRemoval.o: /usr/include/stdlib.h ../global/tree.h
DeadFunctionRemoval.o: ../global/types.h ../global/tree_basic.h
DeadFunctionRemoval.o: ../global/tree_compound.h ../global/Error.h
DeadFunctionRemoval.o: /usr/include/string.h ../global/globals.h
DeadFunctionRemoval.o: ../global/internal_lib.h ../global/free.h
DeadFunctionRemoval.o: ../../inc/dbug.h ../global/my_debug.h
DeadFunctionRemoval.o: /usr/include/malloc.h /usr/include/sys/types.h
DeadFunctionRemoval.o: /usr/include/sys/machtypes.h
DeadFunctionRemoval.o: /usr/include/sys/int_types.h ../global/traverse.h
DeadFunctionRemoval.o: optimize.h
DupTree.o: /usr/include/string.h /usr/include/sys/feature_tests.h
DupTree.o: /usr/include/sys/isa_defs.h ../global/tree.h ../global/types.h
DupTree.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
DupTree.o: /usr/include/stdio.h /usr/include/sys/va_list.h
DupTree.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
DupTree.o: /usr/include/stdlib.h ../global/globals.h ../global/internal_lib.h
DupTree.o: ../global/traverse.h ../global/free.h ../../inc/dbug.h
DupTree.o: ../global/my_debug.h /usr/include/malloc.h
DupTree.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
DupTree.o: /usr/include/sys/int_types.h ../typecheck/typecheck.h DupTree.h
DupTree.o: DataFlowMask.h optimize.h Inline.h ../concurrent/scheduling.h
Inline.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
Inline.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
Inline.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
Inline.o: /usr/include/stdlib.h ../global/globals.h ../global/types.h
Inline.o: ../global/tree.h ../global/tree_basic.h ../global/tree_compound.h
Inline.o: ../global/Error.h /usr/include/string.h ../global/internal_lib.h
Inline.o: ../../inc/dbug.h ../global/my_debug.h ../global/traverse.h
Inline.o: ../global/free.h /usr/include/malloc.h /usr/include/sys/types.h
Inline.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
Inline.o: ../typecheck/typecheck.h optimize.h LoopInvariantRemoval.h
Inline.o: DupTree.h Inline.h
LoopInvariantRemoval.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
LoopInvariantRemoval.o: /usr/include/sys/isa_defs.h
LoopInvariantRemoval.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
LoopInvariantRemoval.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
LoopInvariantRemoval.o: ../global/tree.h ../global/types.h
LoopInvariantRemoval.o: ../global/tree_basic.h ../global/tree_compound.h
LoopInvariantRemoval.o: ../global/Error.h /usr/include/string.h
LoopInvariantRemoval.o: ../global/globals.h ../global/internal_lib.h
LoopInvariantRemoval.o: ../global/free.h ../../inc/dbug.h
LoopInvariantRemoval.o: ../global/my_debug.h /usr/include/malloc.h
LoopInvariantRemoval.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
LoopInvariantRemoval.o: /usr/include/sys/int_types.h ../global/traverse.h
LoopInvariantRemoval.o: ../typecheck/typecheck.h ../global/access_macros.h
LoopInvariantRemoval.o: optimize.h generatemasks.h DupTree.h
LoopInvariantRemoval.o: LoopInvariantRemoval.h
Unroll.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
Unroll.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
Unroll.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
Unroll.o: /usr/include/stdlib.h ../global/globals.h ../global/types.h
Unroll.o: ../global/tree.h ../global/tree_basic.h ../global/tree_compound.h
Unroll.o: ../global/Error.h /usr/include/string.h ../global/internal_lib.h
Unroll.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
Unroll.o: /usr/include/malloc.h /usr/include/sys/types.h
Unroll.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
Unroll.o: ../global/traverse.h ../global/access_macros.h optimize.h
Unroll.o: generatemasks.h ConstantFolding.h DupTree.h Unroll.h WLUnroll.h
Unswitch.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
Unswitch.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
Unswitch.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
Unswitch.o: /usr/include/stdlib.h ../global/globals.h ../global/types.h
Unswitch.o: ../global/tree.h ../global/tree_basic.h ../global/tree_compound.h
Unswitch.o: ../global/Error.h /usr/include/string.h ../global/internal_lib.h
Unswitch.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
Unswitch.o: /usr/include/malloc.h /usr/include/sys/types.h
Unswitch.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
Unswitch.o: ../global/traverse.h ../global/access_macros.h optimize.h
Unswitch.o: generatemasks.h ConstantFolding.h DupTree.h Unroll.h Unswitch.h
WLUnroll.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WLUnroll.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
WLUnroll.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
WLUnroll.o: /usr/include/stdlib.h ../global/globals.h ../global/types.h
WLUnroll.o: ../global/tree.h ../global/tree_basic.h ../global/tree_compound.h
WLUnroll.o: ../global/Error.h /usr/include/string.h ../global/internal_lib.h
WLUnroll.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
WLUnroll.o: /usr/include/malloc.h /usr/include/sys/types.h
WLUnroll.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
WLUnroll.o: ../global/traverse.h ../typecheck/typecheck.h
WLUnroll.o: ../compile/compile.h optimize.h Inline.h DupTree.h
WLUnroll.o: ../psi-opt/WithloopFolding.h
WorkReduction.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WorkReduction.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
WorkReduction.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
WorkReduction.o: /usr/include/stdlib.h ../global/tree.h ../global/types.h
WorkReduction.o: ../global/tree_basic.h ../global/tree_compound.h
WorkReduction.o: ../global/Error.h /usr/include/string.h ../global/globals.h
WorkReduction.o: ../global/internal_lib.h ../../inc/dbug.h
WorkReduction.o: ../global/my_debug.h ../global/traverse.h optimize.h
WorkReduction.o: ../global/free.h /usr/include/malloc.h
WorkReduction.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
WorkReduction.o: /usr/include/sys/int_types.h WorkReduction.h
freemasks.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
freemasks.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
freemasks.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
freemasks.o: /usr/include/stdlib.h /usr/include/string.h ../global/tree.h
freemasks.o: ../global/types.h ../global/tree_basic.h
freemasks.o: ../global/tree_compound.h ../global/Error.h ../global/globals.h
freemasks.o: ../global/internal_lib.h ../global/free.h ../../inc/dbug.h
freemasks.o: ../global/my_debug.h /usr/include/malloc.h
freemasks.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
freemasks.o: /usr/include/sys/int_types.h ../global/traverse.h
generatemasks.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
generatemasks.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
generatemasks.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
generatemasks.o: /usr/include/stdlib.h /usr/include/string.h ../global/tree.h
generatemasks.o: ../global/types.h ../global/tree_basic.h
generatemasks.o: ../global/tree_compound.h ../global/Error.h
generatemasks.o: ../global/globals.h ../global/internal_lib.h
generatemasks.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
generatemasks.o: /usr/include/malloc.h /usr/include/sys/types.h
generatemasks.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
generatemasks.o: ../global/traverse.h ../print/print.h optimize.h
generatemasks.o: generatemasks.h ConstantFolding.h DeadCodeRemoval.h
generatemasks.o: LoopInvariantRemoval.h Inline.h Unroll.h Unswitch.h
generatemasks.o: ../psi-opt/ArrayElimination.h CSE.h
generatemasks.o: ../psi-opt/WithloopFolding.h
optimize.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
optimize.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
optimize.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
optimize.o: /usr/include/stdlib.h /usr/include/string.h ../global/tree.h
optimize.o: ../global/types.h ../global/tree_basic.h
optimize.o: ../global/tree_compound.h ../global/Error.h ../global/globals.h
optimize.o: ../global/internal_lib.h ../global/free.h ../../inc/dbug.h
optimize.o: ../global/my_debug.h /usr/include/malloc.h
optimize.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
optimize.o: /usr/include/sys/int_types.h ../global/traverse.h
optimize.o: ../print/print.h ../print/convert.h optimize.h generatemasks.h
optimize.o: freemasks.h ConstantFolding.h DeadCodeRemoval.h
optimize.o: DeadFunctionRemoval.h LoopInvariantRemoval.h Inline.h Unroll.h
optimize.o: Unswitch.h ../psi-opt/ArrayElimination.h CSE.h
optimize.o: ../psi-opt/WithloopFolding.h ../psi-opt/wl_access_analyze.h
