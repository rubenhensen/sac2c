#
# $Log$
# Revision 1.13  1995/05/26 14:23:42  asi
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

INCS=-I../global -I../../inc -I../typecheck -I../print

all: optimize.o ConstantFolding.o DeadCodeRemoval.o WorkReduction.o \
     LoopInvariantRemoval.o DupTree.o Inline.o Unroll.o

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

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak

# DO NOT DELETE THIS LINE -- make depend depends on it.

ConstantFolding.o: /usr/include/stdio.h /usr/include/stdlib.h
ConstantFolding.o: /usr/include/sys/stdtypes.h ../global/tree.h
ConstantFolding.o: ../global/type_info.mac ../global/node_info.mac
ConstantFolding.o: ../global/prf_node_info.mac ../global/free.h
ConstantFolding.o: ../print/print.h ../global/Error.h ../../inc/dbug.h
ConstantFolding.o: ../global/my_debug.h ../global/traverse.h
ConstantFolding.o: ../typecheck/typecheck.h optimize.h DupTree.h
ConstantFolding.o: ConstantFolding.h
DeadCodeRemoval.o: /usr/include/stdio.h /usr/include/stdlib.h
DeadCodeRemoval.o: /usr/include/sys/stdtypes.h ../global/tree.h
DeadCodeRemoval.o: ../global/type_info.mac ../global/node_info.mac
DeadCodeRemoval.o: ../global/prf_node_info.mac ../global/free.h
DeadCodeRemoval.o: ../global/Error.h ../../inc/dbug.h ../global/my_debug.h
DeadCodeRemoval.o: ../global/traverse.h optimize.h DeadCodeRemoval.h
DupTree.o: /usr/include/string.h /usr/include/sys/stdtypes.h ../global/tree.h
DupTree.o: ../global/type_info.mac ../global/node_info.mac
DupTree.o: ../global/prf_node_info.mac ../../inc/dbug.h /usr/include/stdio.h
DupTree.o: ../global/my_debug.h ../global/traverse.h ../typecheck/typecheck.h
DupTree.o: DupTree.h optimize.h
LoopInvariantRemoval.o: /usr/include/stdio.h /usr/include/stdlib.h
LoopInvariantRemoval.o: /usr/include/sys/stdtypes.h ../global/tree.h
LoopInvariantRemoval.o: ../global/type_info.mac ../global/node_info.mac
LoopInvariantRemoval.o: ../global/prf_node_info.mac ../global/free.h
LoopInvariantRemoval.o: ../global/Error.h ../../inc/dbug.h
LoopInvariantRemoval.o: ../global/my_debug.h ../global/traverse.h
LoopInvariantRemoval.o: ../typecheck/typecheck.h optimize.h DupTree.h
LoopInvariantRemoval.o: LoopInvariantRemoval.h
WorkReduction.o: /usr/include/stdio.h /usr/include/stdlib.h
WorkReduction.o: /usr/include/sys/stdtypes.h ../global/tree.h
WorkReduction.o: ../global/type_info.mac ../global/node_info.mac
WorkReduction.o: ../global/prf_node_info.mac ../global/Error.h
WorkReduction.o: ../../inc/dbug.h ../global/my_debug.h ../global/traverse.h
WorkReduction.o: optimize.h WorkReduction.h
optimize.o: /usr/include/stdio.h /usr/include/stdlib.h
optimize.o: /usr/include/sys/stdtypes.h ../global/tree.h
optimize.o: ../global/type_info.mac ../global/node_info.mac
optimize.o: ../global/prf_node_info.mac ../global/free.h ../global/Error.h
optimize.o: ../../inc/dbug.h ../global/my_debug.h ../global/traverse.h
optimize.o: optimize.h ConstantFolding.h DeadCodeRemoval.h WorkReduction.h
optimize.o: LoopInvariantRemoval.h
