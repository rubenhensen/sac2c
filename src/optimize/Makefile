#
# $Log$
# Revision 1.21  1995/10/08 11:29:52  sbs
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

MAKE=make
CC=gcc -ansi -g

INCS=-I../global -I../../inc -I../typecheck -I../print -I../psi-opt

all: optimize.o ConstantFolding.o DeadCodeRemoval.o WorkReduction.o \
     LoopInvariantRemoval.o DupTree.o Inline.o Unroll.o Unswitch.o

optimize.o: optimize.c
	$(CC) -o optimize.o -c optimize.c $(INCS)

DupTree.o: DupTree.c
	$(CC) -o DupTree.o -c DupTree.c $(INCS)

ConstantFolding.o: ConstantFolding.c
	$(CC) -o ConstantFolding.o -c ConstantFolding.c $(INCS)

DeadCodeRemoval.o: DeadCodeRemoval.c
	$(CC) -o DeadCodeRemoval.o -c DeadCodeRemoval.c $(INCS)

WorkReduction.o: WorkReduction.c
	$(CC) -o WorkReduction.o -c WorkReduction.c $(INCS)

LoopInvariantRemoval.o: LoopInvariantRemoval.c
	$(CC) -o LoopInvariantRemoval.o -c LoopInvariantRemoval.c $(INCS)

Inline.o: Inline.c
	$(CC) -o Inline.o -c Inline.c $(INCS)

Unroll.o: Unroll.c
	$(CC) -o Unroll.o -c Unroll.c $(INCS)

Unswitch.o: Unswitch.c
	$(CC) -o Unswitch.o -c Unswitch.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak

# DO NOT DELETE THIS LINE -- make depend depends on it.

ConstantFolding.o: /usr/include/stdio.h /usr/include/stdlib.h
ConstantFolding.o: /usr/include/sys/stdtypes.h ../global/tree.h
ConstantFolding.o: ../global/types.h ../global/tree_basic.h
ConstantFolding.o: ../global/tree_compound.h ../global/free.h
ConstantFolding.o: ../print/print.h ../global/Error.h ../../inc/dbug.h
ConstantFolding.o: ../global/my_debug.h ../global/traverse.h
ConstantFolding.o: ../typecheck/typecheck.h ../global/internal_lib.h
ConstantFolding.o: ../global/access_macros.h optimize.h DupTree.h
ConstantFolding.o: LoopInvariantRemoval.h ConstantFolding.h
DeadCodeRemoval.o: /usr/include/stdio.h /usr/include/stdlib.h
DeadCodeRemoval.o: /usr/include/sys/stdtypes.h ../global/tree.h
DeadCodeRemoval.o: ../global/types.h ../global/tree_basic.h
DeadCodeRemoval.o: ../global/tree_compound.h ../global/free.h
DeadCodeRemoval.o: ../global/Error.h ../../inc/dbug.h ../global/my_debug.h
DeadCodeRemoval.o: ../global/traverse.h optimize.h DeadCodeRemoval.h
DupTree.o: /usr/include/string.h /usr/include/sys/stdtypes.h ../global/tree.h
DupTree.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
DupTree.o: ../../inc/dbug.h /usr/include/stdio.h ../global/my_debug.h
DupTree.o: ../global/traverse.h ../typecheck/typecheck.h
DupTree.o: ../global/internal_lib.h ../global/free.h DupTree.h optimize.h
DupTree.o: Inline.h
Inline.o: /usr/include/stdio.h /usr/include/stdlib.h
Inline.o: /usr/include/sys/stdtypes.h ../global/tree.h ../global/types.h
Inline.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
Inline.o: ../../inc/dbug.h ../global/my_debug.h ../global/traverse.h
Inline.o: ../global/free.h /usr/include/string.h ../typecheck/typecheck.h
Inline.o: ../global/internal_lib.h optimize.h LoopInvariantRemoval.h
Inline.o: DupTree.h Inline.h
LoopInvariantRemoval.o: /usr/include/stdio.h /usr/include/stdlib.h
LoopInvariantRemoval.o: /usr/include/sys/stdtypes.h ../global/tree.h
LoopInvariantRemoval.o: ../global/types.h ../global/tree_basic.h
LoopInvariantRemoval.o: ../global/tree_compound.h ../global/free.h
LoopInvariantRemoval.o: ../global/Error.h ../../inc/dbug.h
LoopInvariantRemoval.o: ../global/my_debug.h ../global/traverse.h
LoopInvariantRemoval.o: ../typecheck/typecheck.h ../global/access_macros.h
LoopInvariantRemoval.o: optimize.h DupTree.h LoopInvariantRemoval.h
Unroll.o: /usr/include/stdio.h /usr/include/stdlib.h
Unroll.o: /usr/include/sys/stdtypes.h ../global/tree.h ../global/types.h
Unroll.o: ../global/tree_basic.h ../global/tree_compound.h ../global/free.h
Unroll.o: ../global/Error.h ../../inc/dbug.h ../global/my_debug.h
Unroll.o: ../global/traverse.h ../global/access_macros.h
Unroll.o: ../global/internal_lib.h optimize.h ConstantFolding.h DupTree.h
Unroll.o: Unroll.h
Unswitch.o: /usr/include/stdio.h /usr/include/stdlib.h
Unswitch.o: /usr/include/sys/stdtypes.h ../global/tree.h ../global/types.h
Unswitch.o: ../global/tree_basic.h ../global/tree_compound.h ../global/free.h
Unswitch.o: ../global/Error.h ../../inc/dbug.h ../global/my_debug.h
Unswitch.o: ../global/traverse.h ../global/access_macros.h
Unswitch.o: ../global/internal_lib.h optimize.h ConstantFolding.h DupTree.h
Unswitch.o: Unroll.h Unswitch.h
WorkReduction.o: /usr/include/stdio.h /usr/include/stdlib.h
WorkReduction.o: /usr/include/sys/stdtypes.h ../global/tree.h
WorkReduction.o: ../global/types.h ../global/tree_basic.h
WorkReduction.o: ../global/tree_compound.h ../global/Error.h ../../inc/dbug.h
WorkReduction.o: ../global/my_debug.h ../global/traverse.h optimize.h
WorkReduction.o: WorkReduction.h
optimize.o: /usr/include/stdio.h /usr/include/stdlib.h
optimize.o: /usr/include/sys/stdtypes.h ../global/tree.h ../global/types.h
optimize.o: ../global/tree_basic.h ../global/tree_compound.h ../global/free.h
optimize.o: ../global/Error.h ../../inc/dbug.h ../global/my_debug.h
optimize.o: ../global/traverse.h ../print/print.h optimize.h
optimize.o: ConstantFolding.h DeadCodeRemoval.h WorkReduction.h
optimize.o: LoopInvariantRemoval.h Inline.h Unroll.h Unswitch.h
optimize.o: ../psi-opt/ArrayElimination.h
