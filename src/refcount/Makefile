#
# $Log$
# Revision 2.3  1999/03/11 18:18:24  dkr
# make deps done
#
# Revision 2.2  1999/02/26 14:17:42  dkr
# make deps done
#
# Revision 2.1  1999/02/23 12:43:03  sacbase
# new release made
#
# Revision 1.15  1999/02/09 20:04:24  dkr
# make deps done
#
# Revision 1.14  1998/05/12 14:22:24  dkr
# make deps done
#
# Revision 1.13  1998/03/04 00:10:18  dkr
# 'clean' now deletes the subdir .sb, too
#
# Revision 1.12  1998/02/16 21:44:02  dkr
# make deps done
#
# Revision 1.11  1998/02/09 17:40:31  dkr
# added INCS: -I../optimize (function GenerateMasks)
#
# Revision 1.10  1997/11/20 15:02:10  dkr
# make deps done
#
# Revision 1.9  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.8  1997/11/11 16:49:31  sbs
# make deps done
#
# Revision 1.7  1997/08/05 11:32:30  sbs
# make deps done
#
# Revision 1.6  1996/02/16 09:46:55  sbs
# make deps done
#
# Revision 1.5  1996/01/04  18:11:11  cg
# make deps done
#
# Revision 1.4  1995/12/15  10:41:12  cg
# make deps done
#
# Revision 1.3  1995/10/08  11:29:58  sbs
# make deps done
#
# Revision 1.2  1995/03/28  12:09:23  hw
# make deps done
#
# Revision 1.1  1995/03/09  16:17:01  hw
# Initial revision
#
# 


CC        := gcc
CCFLAGS   := -ansi -Wall -g
CFLAGS    := -D__EXTENSIONS__


INCS=-I../global -I../../inc -I../typecheck -I../optimize

all: refcount.o

refcount.o: refcount.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o refcount.o -c refcount.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) refcount.o *.bak
	$(RM) -r .sb

# DO NOT DELETE THIS LINE -- make depend depends on it.

refcount.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
refcount.o: /usr/include/sys/isa_defs.h ../global/tree.h ../global/types.h
refcount.o: ../global/tree_basic.h ../global/tree_compound.h
refcount.o: ../global/Error.h /usr/include/stdio.h /usr/include/sys/va_list.h
refcount.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
refcount.o: /usr/include/string.h ../global/globals.h
refcount.o: ../global/internal_lib.h ../global/my_debug.h ../../inc/dbug.h
refcount.o: ../optimize/DupTree.h ../typecheck/typecheck.h
refcount.o: ../optimize/DataFlowMask.h ../global/traverse.h
refcount.o: ../optimize/optimize.h ../global/free.h /usr/include/malloc.h
refcount.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
refcount.o: /usr/include/sys/int_types.h ../optimize/generatemasks.h
refcount.o: refcount.h
