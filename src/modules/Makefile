#
# $Log$
# Revision 1.30  1998/04/09 21:27:06  dkr
# make deps done
#
# Revision 1.29  1998/03/13 12:55:30  dkr
# make deps done
#
# Revision 1.28  1998/03/04 00:10:55  dkr
# 'clean' now deletes the subdir .sb, too
#
# Revision 1.27  1998/03/03 22:08:59  dkr
# make deps done
#
# Revision 1.26  1998/02/05 17:08:41  dkr
# make deps done
#
# Revision 1.25  1997/11/20 15:02:08  dkr
# make deps done
#
# Revision 1.24  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.23  1997/11/11 16:49:28  sbs
# make deps done
#
# Revision 1.22  1997/10/29 14:40:48  srs
# make deps done
#
# Revision 1.21  1997/09/23 12:59:42  dkr
# make deps done
#
# Revision 1.20  1997/08/05 11:32:28  sbs
# make deps done
#
# Revision 1.19  1997/04/25 12:04:09  sbs
# make deps done
#
# Revision 1.18  1996/01/18  13:05:42  asi
# make deps done
#
# Revision 1.17  1996/01/04  18:11:09  cg
# make deps done
#
# Revision 1.16  1996/01/02  16:01:02  cg
# added compilation of cccall.c
#
# Revision 1.15  1995/12/29  10:38:56  cg
# added compilation of readsib.c
#
# Revision 1.14  1995/12/15  10:41:11  cg
# make deps done
#
# Revision 1.13  1995/10/26  14:48:19  cg
# make deps done
#
# Revision 1.12  1995/10/22  17:40:38  cg
# make deps done and a bloody bug fixed.
#
# Revision 1.11  1995/10/20  09:28:12  cg
# added analysis.c
#
# Revision 1.10  1995/10/08  11:29:54  sbs
# make deps done
#
# Revision 1.9  1995/10/05  16:14:11  cg
# now implicittypes.o will be removed by make clean
#
# Revision 1.8  1995/10/05  16:04:54  cg
# implicittypes.c added
#
# Revision 1.7  1995/08/31  08:40:13  cg
# sib.c and sib.h added
#
# Revision 1.6  1995/08/30  14:02:17  cg
# now print.o depends on compare_macros.h
#
# Revision 1.5  1995/08/08  09:52:15  cg
# include of typecheck.h made possible.
#
# Revision 1.4  1994/12/31  14:31:32  sbs
# preliminary (working) version
#
# Revision 1.3  1994/12/21  12:22:07  sbs
# includes extended
#
# Revision 1.2  1994/12/16  14:33:39  sbs
# import.o inserted
#
# Revision 1.1  1994/12/11  17:33:27  sbs
# Initial revision
#
#
#

CC        := gcc
CCFLAGS   := -ansi -Wall -g
CFLAGS    := -D__EXTENSIONS__

INCS=-I../global -I../../inc -I../scanparse -I../print -I../typecheck

all: filemgr.o import.o readsib.o writesib.o implicittypes.o analysis.o \
     checkdec.o cccall.o

filemgr.o: filemgr.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o filemgr.o -c filemgr.c $(INCS)

import.o: import.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o import.o -c import.c $(INCS)

writesib.o: writesib.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o writesib.o -c writesib.c $(INCS)

readsib.o: readsib.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o readsib.o -c readsib.c $(INCS)

implicittypes.o: implicittypes.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o implicittypes.o -c implicittypes.c $(INCS)

analysis.o: analysis.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o analysis.o -c analysis.c $(INCS)

checkdec.o: checkdec.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o checkdec.o -c checkdec.c $(INCS)

cccall.o: cccall.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o cccall.o -c cccall.c $(INCS)


deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak *.sac *.sib *.dec
	$(RM) -r .sb

# DO NOT DELETE THIS LINE -- make depend depends on it.

analysis.o: ../../inc/dbug.h /usr/include/stdio.h
analysis.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
analysis.o: ../global/types.h ../global/tree_basic.h
analysis.o: ../global/tree_compound.h ../global/Error.h /usr/include/string.h
analysis.o: /usr/include/stdlib.h ../global/globals.h ../global/traverse.h
cccall.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
cccall.o: /usr/include/sys/va_list.h /usr/include/time.h
cccall.o: /usr/include/stdlib.h ../global/types.h ../global/tree_basic.h
cccall.o: ../global/tree_compound.h ../global/Error.h /usr/include/string.h
cccall.o: ../global/globals.h ../../inc/dbug.h ../global/internal_lib.h
cccall.o: ../global/free.h ../global/my_debug.h /usr/include/malloc.h
cccall.o: ../scanparse/scnprs.h ../global/resource.h filemgr.h
cccall.o: ../global/traverse.h
checkdec.o: /usr/include/string.h /usr/include/sys/feature_tests.h
checkdec.o: ../global/types.h ../global/tree_basic.h
checkdec.o: ../global/tree_compound.h ../global/Error.h /usr/include/stdio.h
checkdec.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
checkdec.o: ../global/globals.h ../../inc/dbug.h ../global/my_debug.h
checkdec.o: ../global/traverse.h ../global/free.h /usr/include/malloc.h
checkdec.o: ../global/internal_lib.h import.h ../print/convert.h filemgr.h
checkdec.o: ../scanparse/scnprs.h ../global/resource.h implicittypes.h
checkdec.o: ../print/print.h ../global/tree.h
filemgr.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
filemgr.o: /usr/include/string.h /usr/include/stdarg.h /usr/include/varargs.h
filemgr.o: /usr/include/sys/va_list.h /usr/include/unistd.h
filemgr.o: /usr/include/sys/types.h /usr/include/sys/isa_defs.h
filemgr.o: /usr/include/sys/machtypes.h /usr/include/sys/unistd.h
filemgr.o: ../../inc/dbug.h /usr/include/stdio.h ../global/internal_lib.h
filemgr.o: ../global/Error.h ../global/types.h ../global/globals.h filemgr.h
filemgr.o: ../global/resource.h
implicittypes.o: ../global/types.h ../global/tree_basic.h
implicittypes.o: ../global/tree_compound.h ../global/Error.h
implicittypes.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
implicittypes.o: /usr/include/sys/va_list.h /usr/include/string.h
implicittypes.o: /usr/include/stdlib.h ../global/globals.h ../print/convert.h
implicittypes.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
implicittypes.o: /usr/include/malloc.h ../global/internal_lib.h
implicittypes.o: ../global/traverse.h
import.o: /usr/include/string.h /usr/include/sys/feature_tests.h
import.o: /usr/include/limits.h /usr/include/sys/isa_defs.h ../global/tree.h
import.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
import.o: ../global/Error.h /usr/include/stdio.h /usr/include/sys/va_list.h
import.o: /usr/include/stdlib.h ../global/globals.h ../global/internal_lib.h
import.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
import.o: /usr/include/malloc.h ../scanparse/scnprs.h ../global/resource.h
import.o: ../global/traverse.h ../global/access_macros.h filemgr.h import.h
readsib.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
readsib.o: ../global/Error.h /usr/include/stdio.h
readsib.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
readsib.o: /usr/include/string.h /usr/include/stdlib.h ../global/globals.h
readsib.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
readsib.o: /usr/include/malloc.h ../global/internal_lib.h
readsib.o: ../global/traverse.h import.h ../scanparse/scnprs.h
readsib.o: ../global/resource.h filemgr.h cccall.h
writesib.o: /usr/include/string.h /usr/include/sys/feature_tests.h
writesib.o: ../global/types.h ../global/tree_basic.h
writesib.o: ../global/tree_compound.h ../global/Error.h /usr/include/stdio.h
writesib.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
writesib.o: ../global/globals.h ../global/free.h ../../inc/dbug.h
writesib.o: ../global/my_debug.h /usr/include/malloc.h
writesib.o: ../global/internal_lib.h ../global/traverse.h ../print/convert.h
writesib.o: filemgr.h ../print/print.h ../global/tree.h
writesib.o: ../typecheck/typecheck.h
