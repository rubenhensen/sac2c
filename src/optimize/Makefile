#
# $Log$
# Revision 1.37  1998/02/16 21:43:59  dkr
# make deps done
#
# Revision 1.36  1998/02/06 18:09:51  dkr
# make deps done
#
# Revision 1.35  1998/02/06 13:22:55  srs
# added WithloopFolding.o
#
# Revision 1.34  1998/02/06 11:13:48  srs
# make deps done
#
# Revision 1.33  1998/02/05 17:08:40  dkr
# make deps done
#
# Revision 1.32  1998/02/05 17:03:44  srs
# removed file WorkReduction (from all:) and rebuild dependencies
#
# Revision 1.31  1997/11/20 15:02:07  dkr
# make deps done
#
# Revision 1.30  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.29  1997/11/11 16:49:26  sbs
# make deps done
#
# Revision 1.28  1997/08/05 11:32:27  sbs
# make deps done
#
# Revision 1.27  1997/04/25 12:04:08  sbs
# make deps done
#
# Revision 1.26  1996/01/18  13:05:41  asi
# make deps done
#
# Revision 1.25  1996/01/17  16:49:21  asi
# added common subexpression elimination
#
# Revision 1.23  1996/01/17  14:42:22  asi
# added ../modules to INCS
#
# Revision 1.22  1995/12/15  10:41:10  cg
# make deps done
#
# Revision 1.21  1995/10/08  11:29:52  sbs
# make deps done
#
# Revision 1.20  1995/07/28  13:01:11  asi
# make deps done
#
# Revision 1.19  1995/07/24  11:56:05  asi
# make deps done
#
# Revision 1.18  1995/07/24  11:53:12  asi
# added -I../psi-opt to INCS
#
# Revision 1.17  1995/07/07  15:47:50  asi
# make deps done
#
# Revision 1.16  1995/07/07  14:58:38  asi
# added loop unswitching
#
# Revision 1.15  1995/06/02  11:58:26  asi
# make deps done
#
# Revision 1.14  1995/05/26  14:28:10  asi
# make deps done
#
# Revision 1.13  1995/05/26  14:23:42  asi
# function inlineing and loop unrolling added
#
# Revision 1.12  1995/05/15  13:38:50  asi
# make deps done
#
# Revision 1.11  1995/05/04  11:54:26  asi
# added -I../print
#
# Revision 1.10  1995/05/01  15:38:20  asi
# DupTree added
#
# Revision 1.9  1995/04/20  10:21:20  asi
# make deps done
#
# Revision 1.8  1995/04/05  15:54:58  asi
# make deps done
#
# Revision 1.7  1995/04/05  15:52:38  asi
# loop invariant removal added
#
# Revision 1.6  1995/04/04  11:03:51  sbs
# make deps done
#
# Revision 1.5  1995/03/17  17:42:24  asi
# added work reduction
#
# Revision 1.4  1995/03/06  11:09:01  asi
# added -I../typecheck and make deps
#
# Revision 1.3  1995/02/13  17:17:59  asi
# ConstantFolding.o and DeadCodeRemoval.o added
#
# Revision 1.2  1995/01/18  17:40:09  asi
# make deps
#
# Revision 1.1  1994/12/09  10:48:23  sbs
# Initial revision
#
#

CC        := gcc
CCFLAGS   := -ansi -Wall -g
CFLAGS    := -D__EXTENSIONS__


INCS=-I../global -I../../inc -I../typecheck -I../print -I../psi-opt -I../modules

all: optimize.o ConstantFolding.o DeadCodeRemoval.o \
     LoopInvariantRemoval.o DupTree.o Inline.o Unroll.o Unswitch.o CSE.o \
     WithloopFolding.o

optimize.o: optimize.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o optimize.o -c optimize.c $(INCS)

DupTree.o: DupTree.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o DupTree.o -c DupTree.c $(INCS)

ConstantFolding.o: ConstantFolding.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o ConstantFolding.o -c ConstantFolding.c $(INCS)

DeadCodeRemoval.o: DeadCodeRemoval.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o DeadCodeRemoval.o -c DeadCodeRemoval.c $(INCS)

WorkReduction.o: WorkReduction.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o WorkReduction.o -c WorkReduction.c $(INCS)

LoopInvariantRemoval.o: LoopInvariantRemoval.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o LoopInvariantRemoval.o -c LoopInvariantRemoval.c $(INCS)

Inline.o: Inline.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o Inline.o -c Inline.c $(INCS)

Unroll.o: Unroll.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o Unroll.o -c Unroll.c $(INCS)

Unswitch.o: Unswitch.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o Unswitch.o -c Unswitch.c $(INCS)

CSE.o: CSE.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o CSE.o -c CSE.c $(INCS)

WithloopFolding.o: WithloopFolding.c WithloopFolding.h
	$(CC) $(CCFLAGS) $(CFLAGS) -o WithloopFolding.o -c WithloopFolding.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak

# DO NOT DELETE THIS LINE -- make depend depends on it.

CSE.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
CSE.o: /usr/include/sys/va_list.h /usr/include/stdlib.h ../global/tree.h
CSE.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
CSE.o: ../global/Error.h /usr/include/string.h ../global/globals.h
CSE.o: ../global/internal_lib.h ../global/free.h ../../inc/dbug.h
CSE.o: ../global/my_debug.h /usr/include/malloc.h ../print/print.h
CSE.o: ../global/traverse.h ../typecheck/typecheck.h optimize.h DupTree.h
CSE.o: CSE.h
ConstantFolding.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
ConstantFolding.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
ConstantFolding.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
ConstantFolding.o: ../global/tree_compound.h ../global/Error.h
ConstantFolding.o: /usr/include/string.h ../global/globals.h
ConstantFolding.o: ../global/internal_lib.h ../global/free.h ../../inc/dbug.h
ConstantFolding.o: ../global/my_debug.h /usr/include/malloc.h
ConstantFolding.o: ../print/print.h ../global/traverse.h
ConstantFolding.o: ../typecheck/typecheck.h ../global/access_macros.h
ConstantFolding.o: optimize.h DupTree.h LoopInvariantRemoval.h
ConstantFolding.o: ConstantFolding.h
DeadCodeRemoval.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
DeadCodeRemoval.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
DeadCodeRemoval.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
DeadCodeRemoval.o: ../global/tree_compound.h ../global/Error.h
DeadCodeRemoval.o: /usr/include/string.h ../global/globals.h
DeadCodeRemoval.o: ../global/internal_lib.h ../global/free.h ../../inc/dbug.h
DeadCodeRemoval.o: ../global/my_debug.h /usr/include/malloc.h
DeadCodeRemoval.o: ../global/traverse.h optimize.h DeadCodeRemoval.h
DupTree.o: /usr/include/string.h /usr/include/sys/feature_tests.h
DupTree.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
DupTree.o: ../global/tree_compound.h ../global/Error.h /usr/include/stdio.h
DupTree.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
DupTree.o: ../global/globals.h ../global/internal_lib.h ../global/traverse.h
DupTree.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
DupTree.o: /usr/include/malloc.h ../typecheck/typecheck.h DupTree.h
DupTree.o: optimize.h Inline.h
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
LoopInvariantRemoval.o: ../global/access_macros.h optimize.h DupTree.h
LoopInvariantRemoval.o: LoopInvariantRemoval.h
Unroll.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
Unroll.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
Unroll.o: ../global/globals.h ../global/types.h ../global/tree.h
Unroll.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
Unroll.o: /usr/include/string.h ../global/internal_lib.h ../global/free.h
Unroll.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
Unroll.o: ../global/traverse.h ../global/access_macros.h optimize.h
Unroll.o: ConstantFolding.h DupTree.h Unroll.h
Unswitch.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
Unswitch.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
Unswitch.o: ../global/globals.h ../global/types.h ../global/tree.h
Unswitch.o: ../global/tree_basic.h ../global/tree_compound.h
Unswitch.o: ../global/Error.h /usr/include/string.h ../global/internal_lib.h
Unswitch.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
Unswitch.o: /usr/include/malloc.h ../global/traverse.h
Unswitch.o: ../global/access_macros.h optimize.h ConstantFolding.h DupTree.h
Unswitch.o: Unroll.h Unswitch.h
WithloopFolding.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WithloopFolding.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
WithloopFolding.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
WithloopFolding.o: ../global/tree_compound.h ../global/Error.h
WithloopFolding.o: /usr/include/string.h ../global/globals.h
WithloopFolding.o: ../global/internal_lib.h ../global/free.h ../../inc/dbug.h
WithloopFolding.o: ../global/my_debug.h /usr/include/malloc.h
WithloopFolding.o: ../global/traverse.h WithloopFolding.h
WorkReduction.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WorkReduction.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
WorkReduction.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
WorkReduction.o: ../global/tree_compound.h ../global/Error.h
WorkReduction.o: /usr/include/string.h ../global/globals.h
WorkReduction.o: ../global/internal_lib.h ../../inc/dbug.h
WorkReduction.o: ../global/my_debug.h ../global/traverse.h optimize.h
WorkReduction.o: ../global/free.h /usr/include/malloc.h WorkReduction.h
optimize.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
optimize.o: /usr/include/sys/va_list.h /usr/include/stdlib.h ../global/tree.h
optimize.o: ../global/types.h ../global/tree_basic.h
optimize.o: ../global/tree_compound.h ../global/Error.h /usr/include/string.h
optimize.o: ../global/globals.h ../global/internal_lib.h ../global/free.h
optimize.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
optimize.o: ../global/traverse.h ../print/print.h optimize.h
optimize.o: ConstantFolding.h DeadCodeRemoval.h LoopInvariantRemoval.h
optimize.o: Inline.h Unroll.h Unswitch.h ../psi-opt/ArrayElimination.h CSE.h
optimize.o: WithloopFolding.h
