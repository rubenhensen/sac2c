#
# $Log$
# Revision 1.8  1996/01/17 14:19:52  asi
# added ../modules to INCS
#
# Revision 1.7  1995/12/15  10:41:13  cg
# make deps done
#
# Revision 1.6  1995/10/08  11:30:00  sbs
# make deps done
#
# Revision 1.5  1995/07/24  11:56:09  asi
# make deps done
#
# Revision 1.4  1995/07/24  11:54:09  asi
# added array elimination
#
# Revision 1.3  1995/07/14  08:33:01  sbs
# make deps done
#
# Revision 1.2  1995/07/14  08:31:48  sbs
# *** empty log message ***
#
# Revision 1.1  1995/06/02  10:09:27  sbs
# Initial revision
#
#
#

MAKE=make
CC=gcc -ansi -g

INCS=-I../global -I../../inc -I../optimize -I../compile -I../typecheck -I../modules

all: index.o psi-opt.o ArrayElimination.o

index.o: index.c
	$(CC) -o index.o -c index.c $(INCS)

psi-opt.o: psi-opt.c
	$(CC) -o psi-opt.o -c psi-opt.c $(INCS)

ArrayElimination.o: ArrayElimination.c
	$(CC) -o ArrayElimination.o -c ArrayElimination.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) index.o psi-opt.o *.bak

# DO NOT DELETE THIS LINE -- make depend depends on it.

ArrayElimination.o: /usr/include/stdio.h /usr/include/stdlib.h
ArrayElimination.o: /usr/include/sys/stdtypes.h /usr/include/string.h
ArrayElimination.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
ArrayElimination.o: ../global/tree_compound.h ../global/Error.h
ArrayElimination.o: ../global/internal_lib.h ../../inc/dbug.h
ArrayElimination.o: ../global/my_debug.h ../global/traverse.h
ArrayElimination.o: ../typecheck/typecheck.h ../global/free.h
ArrayElimination.o: ../optimize/DupTree.h ../global/access_macros.h
ArrayElimination.o: ../optimize/optimize.h ../optimize/Inline.h
ArrayElimination.o: ../optimize/LoopInvariantRemoval.h ArrayElimination.h
index.o: /usr/include/stdio.h /usr/include/string.h
index.o: /usr/include/sys/stdtypes.h ../../inc/dbug.h ../global/tree.h
index.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
index.o: ../global/Error.h /usr/include/stdlib.h ../global/internal_lib.h
index.o: ../global/traverse.h ../optimize/DupTree.h index.h
index.o: ../global/access_macros.h ../compile/compile.h
psi-opt.o: /usr/include/stdio.h /usr/include/stdlib.h
psi-opt.o: /usr/include/sys/stdtypes.h ../global/tree.h ../global/types.h
psi-opt.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
psi-opt.o: /usr/include/string.h ../global/internal_lib.h ../../inc/dbug.h
psi-opt.o: ../global/traverse.h psi-opt.h ../optimize/optimize.h
psi-opt.o: ../global/free.h ../global/my_debug.h
