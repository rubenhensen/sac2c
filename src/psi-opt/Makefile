#
# $Log$
# Revision 1.18  1998/02/05 17:08:43  dkr
# make deps done
#
# Revision 1.17  1997/11/20 15:02:11  dkr
# make deps done
#
# Revision 1.16  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
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

CC        := gcc
CCFLAGS   := -ansi -Wall -g
CFLAGS    := -D__EXTENSIONS__


INCS=-I../global -I../../inc -I../optimize -I../compile -I../typecheck -I../modules

all: index.o psi-opt.o ArrayElimination.o

index.o: index.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o index.o -c index.c $(INCS)

psi-opt.o: psi-opt.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o psi-opt.o -c psi-opt.c $(INCS)

ArrayElimination.o: ArrayElimination.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o ArrayElimination.o -c ArrayElimination.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) index.o psi-opt.o ArrayElimination.o *.bak

# DO NOT DELETE THIS LINE -- make depend depends on it.

ArrayElimination.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
ArrayElimination.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
ArrayElimination.o: /usr/include/string.h ../global/globals.h
ArrayElimination.o: ../global/types.h ../global/Error.h ../../inc/dbug.h
ArrayElimination.o: ../global/my_debug.h ../global/traverse.h
ArrayElimination.o: ../typecheck/typecheck.h ../global/free.h
ArrayElimination.o: /usr/include/malloc.h ../global/internal_lib.h
ArrayElimination.o: ../optimize/DupTree.h ../global/tree.h
ArrayElimination.o: ../global/tree_basic.h ../global/tree_compound.h
ArrayElimination.o: ../optimize/optimize.h ../optimize/Inline.h
ArrayElimination.o: ../optimize/LoopInvariantRemoval.h ArrayElimination.h
index.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
index.o: /usr/include/sys/va_list.h /usr/include/string.h ../../inc/dbug.h
index.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
index.o: ../global/tree_compound.h ../global/Error.h /usr/include/stdlib.h
index.o: ../global/globals.h ../global/internal_lib.h ../global/traverse.h
index.o: ../optimize/DupTree.h index.h psi-opt.h ../global/free.h
index.o: ../global/my_debug.h /usr/include/malloc.h ../global/access_macros.h
index.o: ../compile/compile.h
psi-opt.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
psi-opt.o: /usr/include/sys/va_list.h /usr/include/stdlib.h ../global/tree.h
psi-opt.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
psi-opt.o: ../global/Error.h /usr/include/string.h ../global/globals.h
psi-opt.o: ../global/internal_lib.h ../../inc/dbug.h ../global/traverse.h
psi-opt.o: psi-opt.h
