#
# $Log$
# Revision 1.10  1997/08/05 11:32:32  sbs
# make deps done
#
# Revision 1.9  1996/02/16 09:46:58  sbs
# make deps done
#
# Revision 1.8  1996/01/25  18:45:52  cg
# added new include directories
#
# Revision 1.7  1996/01/04  18:11:12  cg
# make deps done
#
# Revision 1.6  1995/12/15  10:41:14  cg
# make deps done
#
# Revision 1.5  1995/11/16  19:45:25  cg
# added compilation of rmvoidfun.c
#
# Revision 1.4  1995/11/06  09:23:30  cg
# added uniquecheck.c
#
# Revision 1.3  1995/11/01  08:03:53  cg
# added compilation of objects.c
#
# Revision 1.2  1995/10/25  15:18:16  cg
# bug fixed, now make clean with rm -f
#
# Revision 1.1  1995/10/16  12:07:18  cg
# Initial revision
#
#
#


MAKE=make
CC=gcc -ansi -g
RM=rm -f

INCS=-I../global -I../../inc -I../typecheck -I../modules

all: objinit.o objects.o uniquecheck.o rmvoidfun.o


objinit.o: objinit.c
	$(CC) -o objinit.o -c objinit.c $(INCS)

objects.o: objects.c
	$(CC) -o objects.o -c objects.c $(INCS)

uniquecheck.o: uniquecheck.c
	$(CC) -o uniquecheck.o -c uniquecheck.c $(INCS)

rmvoidfun.o: rmvoidfun.c
	$(CC) -o rmvoidfun.o -c rmvoidfun.c $(INCS)


deps:
	makedepend $(INCS) *.c

clean:
	$(RM) objinit.o objects.o uniquecheck.o rmvoidfun.o  *.bak




# DO NOT DELETE THIS LINE -- make depend depends on it.

objects.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
objects.o: ../global/Error.h /usr/include/stdio.h
objects.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
objects.o: /usr/include/string.h /usr/include/stdlib.h ../global/globals.h
objects.o: ../../inc/dbug.h ../global/traverse.h ../global/internal_lib.h
objinit.o: /usr/include/string.h /usr/include/sys/feature_tests.h
objinit.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
objinit.o: ../global/Error.h /usr/include/stdio.h /usr/include/sys/va_list.h
objinit.o: /usr/include/stdlib.h ../global/globals.h ../global/traverse.h
objinit.o: ../../inc/dbug.h ../global/internal_lib.h ../global/free.h
objinit.o: ../global/my_debug.h /usr/include/malloc.h
objinit.o: ../typecheck/typecheck.h ../modules/filemgr.h
rmvoidfun.o: ../global/types.h ../global/tree_basic.h
rmvoidfun.o: ../global/tree_compound.h ../global/Error.h /usr/include/stdio.h
rmvoidfun.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
rmvoidfun.o: /usr/include/string.h /usr/include/stdlib.h ../global/globals.h
rmvoidfun.o: ../../inc/dbug.h ../global/traverse.h ../global/internal_lib.h
rmvoidfun.o: ../global/free.h ../global/my_debug.h /usr/include/malloc.h
uniquecheck.o: ../global/types.h ../global/tree_basic.h
uniquecheck.o: ../global/tree_compound.h ../global/Error.h
uniquecheck.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
uniquecheck.o: /usr/include/sys/va_list.h /usr/include/string.h
uniquecheck.o: /usr/include/stdlib.h ../global/globals.h ../../inc/dbug.h
uniquecheck.o: ../global/traverse.h ../global/internal_lib.h ../global/free.h
uniquecheck.o: ../global/my_debug.h /usr/include/malloc.h
