#
# $Log$
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

freemasks.o: freemasks.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o freemasks.o -c freemasks.c $(INCS)

generatemasks.o: generatemasks.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o generatemasks.o -c generatemasks.c $(INCS)

optimize.o: optimize.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o optimize.o -c optimize.c $(INCS)

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

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

CSE.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
CSE.o: /usr/include/sys/va_list.h /usr/include/stdlib.h ../tree/tree.h
CSE.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
CSE.o: ../global/Error.h /usr/include/string.h ../global/globals.h
CSE.o: ../global/internal_lib.h ../tree/free.h ../../inc/dbug.h
CSE.o: ../global/my_debug.h /usr/include/malloc.h ../print/print.h
CSE.o: ../tree/traverse.h ../typecheck/typecheck.h optimize.h generatemasks.h
CSE.o: ../tree/DupTree.h ../tree/LookUpTable.h CSE.h
ConstantFolding.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
ConstantFolding.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
ConstantFolding.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
ConstantFolding.o: ../tree/tree_compound.h ../global/Error.h
ConstantFolding.o: /usr/include/string.h ../global/globals.h
ConstantFolding.o: ../global/internal_lib.h ../tree/free.h ../../inc/dbug.h
ConstantFolding.o: ../global/my_debug.h /usr/include/malloc.h
ConstantFolding.o: ../print/print.h ../tree/traverse.h
ConstantFolding.o: ../tree/access_macros.h ../typecheck/typecheck.h
ConstantFolding.o: optimize.h generatemasks.h ../tree/DupTree.h
ConstantFolding.o: ../tree/LookUpTable.h LoopInvariantRemoval.h
ConstantFolding.o: ConstantFolding.h
DeadCodeRemoval.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
DeadCodeRemoval.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
DeadCodeRemoval.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
DeadCodeRemoval.o: ../tree/tree_compound.h ../global/Error.h
DeadCodeRemoval.o: /usr/include/string.h ../global/globals.h
DeadCodeRemoval.o: ../global/internal_lib.h ../tree/free.h ../../inc/dbug.h
DeadCodeRemoval.o: ../global/my_debug.h /usr/include/malloc.h
DeadCodeRemoval.o: ../tree/traverse.h optimize.h generatemasks.h
DeadCodeRemoval.o: DeadCodeRemoval.h
DeadFunctionRemoval.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
DeadFunctionRemoval.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
DeadFunctionRemoval.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
DeadFunctionRemoval.o: ../tree/tree_compound.h ../global/Error.h
DeadFunctionRemoval.o: /usr/include/string.h ../global/globals.h
DeadFunctionRemoval.o: ../global/internal_lib.h ../tree/free.h
DeadFunctionRemoval.o: ../../inc/dbug.h ../global/my_debug.h
DeadFunctionRemoval.o: /usr/include/malloc.h ../tree/traverse.h optimize.h
Inline.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
Inline.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
Inline.o: ../global/globals.h ../global/types.h ../tree/tree.h
Inline.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
Inline.o: /usr/include/string.h ../global/internal_lib.h ../../inc/dbug.h
Inline.o: ../global/my_debug.h ../tree/traverse.h ../tree/free.h
Inline.o: /usr/include/malloc.h ../typecheck/typecheck.h optimize.h
Inline.o: LoopInvariantRemoval.h ../tree/DupTree.h ../tree/LookUpTable.h
Inline.o: Inline.h
LoopInvariantRemoval.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
LoopInvariantRemoval.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
LoopInvariantRemoval.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
LoopInvariantRemoval.o: ../tree/tree_compound.h ../global/Error.h
LoopInvariantRemoval.o: /usr/include/string.h ../global/globals.h
LoopInvariantRemoval.o: ../global/internal_lib.h ../tree/free.h
LoopInvariantRemoval.o: ../../inc/dbug.h ../global/my_debug.h
LoopInvariantRemoval.o: /usr/include/malloc.h ../tree/traverse.h
LoopInvariantRemoval.o: ../typecheck/typecheck.h ../tree/access_macros.h
LoopInvariantRemoval.o: optimize.h generatemasks.h ../tree/DupTree.h
LoopInvariantRemoval.o: ../tree/LookUpTable.h LoopInvariantRemoval.h
Unroll.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
Unroll.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
Unroll.o: ../global/globals.h ../global/types.h ../tree/tree.h
Unroll.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
Unroll.o: /usr/include/string.h ../global/internal_lib.h ../tree/free.h
Unroll.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
Unroll.o: ../tree/traverse.h ../tree/access_macros.h optimize.h
Unroll.o: generatemasks.h ConstantFolding.h ../tree/DupTree.h
Unroll.o: ../tree/LookUpTable.h Unroll.h WLUnroll.h
Unswitch.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
Unswitch.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
Unswitch.o: ../global/globals.h ../global/types.h ../tree/tree.h
Unswitch.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
Unswitch.o: /usr/include/string.h ../global/internal_lib.h ../tree/free.h
Unswitch.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
Unswitch.o: ../tree/traverse.h ../tree/access_macros.h optimize.h
Unswitch.o: generatemasks.h ConstantFolding.h ../tree/DupTree.h
Unswitch.o: ../tree/LookUpTable.h Unroll.h Unswitch.h
WLUnroll.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WLUnroll.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
WLUnroll.o: ../global/globals.h ../global/types.h ../tree/tree.h
WLUnroll.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
WLUnroll.o: /usr/include/string.h ../global/internal_lib.h ../tree/free.h
WLUnroll.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
WLUnroll.o: ../tree/traverse.h ../typecheck/typecheck.h ../compile/compile.h
WLUnroll.o: optimize.h Inline.h ../tree/DupTree.h ../tree/LookUpTable.h
WLUnroll.o: ../psi-opt/WithloopFolding.h
WorkReduction.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WorkReduction.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
WorkReduction.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
WorkReduction.o: ../tree/tree_compound.h ../global/Error.h
WorkReduction.o: /usr/include/string.h ../global/globals.h
WorkReduction.o: ../global/internal_lib.h ../../inc/dbug.h
WorkReduction.o: ../global/my_debug.h ../tree/traverse.h optimize.h
WorkReduction.o: ../tree/free.h /usr/include/malloc.h WorkReduction.h
freemasks.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
freemasks.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
freemasks.o: /usr/include/string.h ../tree/tree.h ../global/types.h
freemasks.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
freemasks.o: ../global/globals.h ../global/internal_lib.h ../tree/free.h
freemasks.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
freemasks.o: ../tree/traverse.h
generatemasks.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
generatemasks.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
generatemasks.o: /usr/include/string.h ../tree/tree.h ../global/types.h
generatemasks.o: ../tree/tree_basic.h ../tree/tree_compound.h
generatemasks.o: ../global/Error.h ../global/globals.h
generatemasks.o: ../global/internal_lib.h ../tree/free.h ../../inc/dbug.h
generatemasks.o: ../global/my_debug.h /usr/include/malloc.h
generatemasks.o: ../tree/traverse.h ../print/print.h optimize.h
generatemasks.o: generatemasks.h ConstantFolding.h DeadCodeRemoval.h
generatemasks.o: LoopInvariantRemoval.h Inline.h Unroll.h Unswitch.h
generatemasks.o: ../psi-opt/ArrayElimination.h CSE.h
generatemasks.o: ../psi-opt/WithloopFolding.h
optimize.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
optimize.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
optimize.o: /usr/include/string.h ../tree/tree.h ../global/types.h
optimize.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
optimize.o: ../global/globals.h ../global/internal_lib.h ../tree/free.h
optimize.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
optimize.o: ../tree/traverse.h ../print/print.h ../print/convert.h optimize.h
optimize.o: generatemasks.h freemasks.h ConstantFolding.h DeadCodeRemoval.h
optimize.o: DeadFunctionRemoval.h LoopInvariantRemoval.h Inline.h Unroll.h
optimize.o: Unswitch.h ../psi-opt/ArrayElimination.h CSE.h
optimize.o: ../psi-opt/WithloopFolding.h ../psi-opt/wl_access_analyze.h
optimize.o: ../psi-opt/tile_size_inference.h ../psi-opt/pad.h
