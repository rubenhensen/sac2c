#
# $Log$
# Revision 1.15  1997/11/11 16:49:32  sbs
# make deps done
#
# Revision 1.14  1997/11/05 11:35:21  dkr
# *** empty log message ***
#
# Revision 1.13  1997/08/05 11:32:31  sbs
# make deps done
#
# Revision 1.12  1997/04/25 12:04:11  sbs
# make deps done
#
# Revision 1.11  1996/07/18  12:23:42  sbs
# ArrayElimination.o included in clean-line
#
# Revision 1.10  1996/02/16  09:46:57  sbs
# make deps done
#
# Revision 1.9  1996/01/18  13:05:44  asi
# make deps done
#
# Revision 1.8  1996/01/17  14:19:52  asi
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
	$(RM) index.o psi-opt.o ArrayElimination.o *.bak

# DO NOT DELETE THIS LINE -- make depend depends on it.

ArrayElimination.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
ArrayElimination.o: /usr/include/stdlib.h /usr/include/string.h
ArrayElimination.o: ../global/globals.h ../global/types.h ../global/Error.h
ArrayElimination.o: ../../inc/dbug.h ../global/my_debug.h
ArrayElimination.o: ../global/traverse.h ../typecheck/typecheck.h
ArrayElimination.o: ../global/free.h /usr/include/malloc.h
ArrayElimination.o: ../optimize/DupTree.h ../global/tree.h
ArrayElimination.o: ../global/tree_basic.h ../global/tree_compound.h
ArrayElimination.o: ../global/internal_lib.h ../optimize/optimize.h
ArrayElimination.o: ../optimize/Inline.h ../optimize/LoopInvariantRemoval.h
ArrayElimination.o: ArrayElimination.h
index.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
index.o: /usr/include/string.h ../../inc/dbug.h ../global/tree.h
index.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
index.o: ../global/Error.h /usr/include/stdlib.h ../global/globals.h
index.o: ../global/internal_lib.h ../global/traverse.h ../optimize/DupTree.h
index.o: index.h psi-opt.h ../global/free.h ../global/my_debug.h
index.o: /usr/include/malloc.h ../global/access_macros.h ../compile/compile.h
psi-opt.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
psi-opt.o: /usr/include/stdlib.h ../global/tree.h ../global/types.h
psi-opt.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
psi-opt.o: /usr/include/string.h ../global/globals.h ../global/internal_lib.h
psi-opt.o: ../../inc/dbug.h ../global/traverse.h psi-opt.h
