#
# $Log$
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
     WithloopFolding.o WLT.o WLI.o WLF.o DataFlowMask.o WLUnroll.o \
     DeadFunctionRemoval.o freemasks.o

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

WithloopFolding.o: WithloopFolding.c WithloopFolding.h
	$(CC) $(CCFLAGS) $(CFLAGS) -o WithloopFolding.o -c WithloopFolding.c $(INCS)

WLT.o: WLT.c WLT.h
	$(CC) $(CCFLAGS) $(CFLAGS) -o WLT.o -c WLT.c $(INCS)

WLI.o: WLI.c WLI.h
	$(CC) $(CCFLAGS) $(CFLAGS) -o WLI.o -c WLI.c $(INCS)

WLF.o: WLF.c WLF.h
	$(CC) $(CCFLAGS) $(CFLAGS) -o WLF.o -c WLF.c $(INCS)

DataFlowMask.o: DataFlowMask.c 
	$(CC) $(CCFLAGS) $(CFLAGS) -o DataFlowMask.o -c DataFlowMask.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
	$(RM) -r .sb

# DO NOT DELETE THIS LINE -- make depend depends on it.

CSE.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
CSE.o: /usr/include/sys/va_list.h /usr/include/stdlib.h ../global/tree.h
CSE.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
CSE.o: ../global/Error.h /usr/include/string.h ../global/globals.h
CSE.o: ../global/internal_lib.h ../global/free.h ../../inc/dbug.h
CSE.o: ../global/my_debug.h /usr/include/malloc.h ../print/print.h
CSE.o: ../global/traverse.h ../typecheck/typecheck.h optimize.h
CSE.o: generatemasks.h DupTree.h CSE.h
ConstantFolding.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
ConstantFolding.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
ConstantFolding.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
ConstantFolding.o: ../global/tree_compound.h ../global/Error.h
ConstantFolding.o: /usr/include/string.h ../global/globals.h
ConstantFolding.o: ../global/internal_lib.h ../global/free.h ../../inc/dbug.h
ConstantFolding.o: ../global/my_debug.h /usr/include/malloc.h
ConstantFolding.o: ../print/print.h ../global/traverse.h
ConstantFolding.o: ../typecheck/typecheck.h ../global/access_macros.h
ConstantFolding.o: optimize.h generatemasks.h DupTree.h
ConstantFolding.o: LoopInvariantRemoval.h ConstantFolding.h
DataFlowMask.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
DataFlowMask.o: /usr/include/sys/va_list.h /usr/include/string.h
DataFlowMask.o: ../../inc/dbug.h ../global/types.h ../global/tree_basic.h
DataFlowMask.o: ../global/tree_compound.h ../global/Error.h
DataFlowMask.o: /usr/include/stdlib.h ../global/globals.h
DataFlowMask.o: ../global/internal_lib.h ../global/free.h
DataFlowMask.o: ../global/my_debug.h /usr/include/malloc.h
DeadCodeRemoval.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
DeadCodeRemoval.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
DeadCodeRemoval.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
DeadCodeRemoval.o: ../global/tree_compound.h ../global/Error.h
DeadCodeRemoval.o: /usr/include/string.h ../global/globals.h
DeadCodeRemoval.o: ../global/internal_lib.h ../global/free.h ../../inc/dbug.h
DeadCodeRemoval.o: ../global/my_debug.h /usr/include/malloc.h
DeadCodeRemoval.o: ../global/traverse.h optimize.h generatemasks.h
DeadCodeRemoval.o: DeadCodeRemoval.h
DeadFunctionRemoval.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
DeadFunctionRemoval.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
DeadFunctionRemoval.o: ../global/tree.h ../global/types.h
DeadFunctionRemoval.o: ../global/tree_basic.h ../global/tree_compound.h
DeadFunctionRemoval.o: ../global/Error.h /usr/include/string.h
DeadFunctionRemoval.o: ../global/globals.h ../global/internal_lib.h
DeadFunctionRemoval.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
DeadFunctionRemoval.o: /usr/include/malloc.h ../global/traverse.h optimize.h
DupTree.o: /usr/include/string.h /usr/include/sys/feature_tests.h
DupTree.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
DupTree.o: ../global/tree_compound.h ../global/Error.h /usr/include/stdio.h
DupTree.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
DupTree.o: ../global/globals.h ../global/internal_lib.h ../global/traverse.h
DupTree.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
DupTree.o: /usr/include/malloc.h ../typecheck/typecheck.h DupTree.h
DupTree.o: DataFlowMask.h optimize.h Inline.h ../concurrent/scheduling.h
Inline.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
Inline.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
Inline.o: ../global/globals.h ../global/types.h ../global/tree.h
Inline.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
Inline.o: /usr/include/string.h ../global/internal_lib.h ../../inc/dbug.h
Inline.o: ../global/my_debug.h ../global/traverse.h ../global/free.h
Inline.o: /usr/include/malloc.h ../typecheck/typecheck.h optimize.h
Inline.o: LoopInvariantRemoval.h DupTree.h Inline.h
LoopInvariantRemoval.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
LoopInvariantRemoval.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
LoopInvariantRemoval.o: ../global/tree.h ../global/types.h
LoopInvariantRemoval.o: ../global/tree_basic.h ../global/tree_compound.h
LoopInvariantRemoval.o: ../global/Error.h /usr/include/string.h
LoopInvariantRemoval.o: ../global/globals.h ../global/internal_lib.h
LoopInvariantRemoval.o: ../global/free.h ../../inc/dbug.h
LoopInvariantRemoval.o: ../global/my_debug.h /usr/include/malloc.h
LoopInvariantRemoval.o: ../global/traverse.h ../typecheck/typecheck.h
LoopInvariantRemoval.o: ../global/access_macros.h optimize.h generatemasks.h
LoopInvariantRemoval.o: DupTree.h LoopInvariantRemoval.h
Unroll.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
Unroll.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
Unroll.o: ../global/globals.h ../global/types.h ../global/tree.h
Unroll.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
Unroll.o: /usr/include/string.h ../global/internal_lib.h ../global/free.h
Unroll.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
Unroll.o: ../global/traverse.h ../global/access_macros.h optimize.h
Unroll.o: generatemasks.h ConstantFolding.h DupTree.h Unroll.h WLUnroll.h
Unswitch.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
Unswitch.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
Unswitch.o: ../global/globals.h ../global/types.h ../global/tree.h
Unswitch.o: ../global/tree_basic.h ../global/tree_compound.h
Unswitch.o: ../global/Error.h /usr/include/string.h ../global/internal_lib.h
Unswitch.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
Unswitch.o: /usr/include/malloc.h ../global/traverse.h
Unswitch.o: ../global/access_macros.h optimize.h generatemasks.h
Unswitch.o: ConstantFolding.h DupTree.h Unroll.h Unswitch.h
WLF.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WLF.o: /usr/include/sys/va_list.h /usr/include/stdlib.h /usr/include/limits.h
WLF.o: /usr/include/sys/isa_defs.h ../global/tree.h ../global/types.h
WLF.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
WLF.o: /usr/include/string.h ../global/globals.h ../global/internal_lib.h
WLF.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
WLF.o: /usr/include/malloc.h ../print/print.h DupTree.h ../global/traverse.h
WLF.o: optimize.h generatemasks.h ConstantFolding.h WithloopFolding.h WLF.h
WLI.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WLI.o: /usr/include/sys/va_list.h /usr/include/stdlib.h ../global/tree.h
WLI.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
WLI.o: ../global/Error.h /usr/include/string.h ../global/globals.h
WLI.o: ../global/internal_lib.h ../global/free.h ../../inc/dbug.h
WLI.o: ../global/my_debug.h /usr/include/malloc.h ../print/print.h
WLI.o: ../global/traverse.h optimize.h generatemasks.h ConstantFolding.h
WLI.o: WithloopFolding.h WLI.h
WLT.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WLT.o: /usr/include/sys/va_list.h /usr/include/stdlib.h ../global/tree.h
WLT.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
WLT.o: ../global/Error.h /usr/include/string.h ../global/globals.h
WLT.o: ../global/internal_lib.h ../global/free.h ../../inc/dbug.h
WLT.o: ../global/my_debug.h /usr/include/malloc.h ../print/print.h DupTree.h
WLT.o: ../global/traverse.h optimize.h generatemasks.h WithloopFolding.h
WLT.o: WLT.h
WLUnroll.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WLUnroll.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
WLUnroll.o: ../global/globals.h ../global/types.h ../global/tree.h
WLUnroll.o: ../global/tree_basic.h ../global/tree_compound.h
WLUnroll.o: ../global/Error.h /usr/include/string.h ../global/internal_lib.h
WLUnroll.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
WLUnroll.o: /usr/include/malloc.h ../global/traverse.h
WLUnroll.o: ../typecheck/typecheck.h ../compile/compile.h optimize.h Inline.h
WLUnroll.o: DupTree.h WithloopFolding.h
WithloopFolding.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WithloopFolding.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
WithloopFolding.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
WithloopFolding.o: ../global/tree_compound.h ../global/Error.h
WithloopFolding.o: /usr/include/string.h ../global/globals.h
WithloopFolding.o: ../global/internal_lib.h ../global/free.h ../../inc/dbug.h
WithloopFolding.o: ../global/my_debug.h /usr/include/malloc.h
WithloopFolding.o: ../print/print.h DupTree.h ../global/traverse.h optimize.h
WithloopFolding.o: generatemasks.h Inline.h ../typecheck/typecheck.h
WithloopFolding.o: ConstantFolding.h WithloopFolding.h WLT.h WLI.h WLF.h
WorkReduction.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WorkReduction.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
WorkReduction.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
WorkReduction.o: ../global/tree_compound.h ../global/Error.h
WorkReduction.o: /usr/include/string.h ../global/globals.h
WorkReduction.o: ../global/internal_lib.h ../../inc/dbug.h
WorkReduction.o: ../global/my_debug.h ../global/traverse.h optimize.h
WorkReduction.o: ../global/free.h /usr/include/malloc.h WorkReduction.h
freemasks.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
freemasks.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
freemasks.o: /usr/include/string.h ../global/tree.h ../global/types.h
freemasks.o: ../global/tree_basic.h ../global/tree_compound.h
freemasks.o: ../global/Error.h ../global/globals.h ../global/internal_lib.h
freemasks.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
freemasks.o: /usr/include/malloc.h ../global/traverse.h
generatemasks.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
generatemasks.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
generatemasks.o: /usr/include/string.h ../global/tree.h ../global/types.h
generatemasks.o: ../global/tree_basic.h ../global/tree_compound.h
generatemasks.o: ../global/Error.h ../global/globals.h
generatemasks.o: ../global/internal_lib.h ../global/free.h ../../inc/dbug.h
generatemasks.o: ../global/my_debug.h /usr/include/malloc.h
generatemasks.o: ../global/traverse.h ../print/print.h optimize.h
generatemasks.o: generatemasks.h ConstantFolding.h DeadCodeRemoval.h
generatemasks.o: LoopInvariantRemoval.h Inline.h Unroll.h Unswitch.h
generatemasks.o: ../psi-opt/ArrayElimination.h CSE.h WithloopFolding.h
optimize.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
optimize.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
optimize.o: /usr/include/string.h ../global/tree.h ../global/types.h
optimize.o: ../global/tree_basic.h ../global/tree_compound.h
optimize.o: ../global/Error.h ../global/globals.h ../global/internal_lib.h
optimize.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
optimize.o: /usr/include/malloc.h ../global/traverse.h ../print/print.h
optimize.o: ../print/convert.h optimize.h generatemasks.h freemasks.h
optimize.o: ConstantFolding.h DeadCodeRemoval.h DeadFunctionRemoval.h
optimize.o: LoopInvariantRemoval.h Inline.h Unroll.h Unswitch.h
optimize.o: ../psi-opt/ArrayElimination.h CSE.h WithloopFolding.h

