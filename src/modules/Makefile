#
# $Log$
# Revision 1.18  1996/01/18 13:05:42  asi
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

MAKE=make
CC=gcc -ansi -g

INCS=-I../global -I../../inc -I../scanparse -I../print -I../typecheck

all: filemgr.o import.o readsib.o writesib.o implicittypes.o analysis.o \
     checkdec.o cccall.o

filemgr.o: filemgr.c
	$(CC) -o filemgr.o -c filemgr.c $(INCS)

import.o: import.c
	$(CC) -o import.o -c import.c $(INCS)

writesib.o: writesib.c
	$(CC) -o writesib.o -c writesib.c $(INCS)

readsib.o: readsib.c
	$(CC) -o readsib.o -c readsib.c $(INCS)

implicittypes.o: implicittypes.c
	$(CC) -o implicittypes.o -c implicittypes.c $(INCS)

analysis.o: analysis.c
	$(CC) -o analysis.o -c analysis.c $(INCS)

checkdec.o: checkdec.c
	$(CC) -o checkdec.o -c checkdec.c $(INCS)

cccall.o: cccall.c
	$(CC) -o cccall.o -c cccall.c $(INCS)


deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak *.sac *.sib *.dec



# DO NOT DELETE THIS LINE -- make depend depends on it.

analysis.o: ../../inc/dbug.h /usr/include/stdio.h ../global/types.h
analysis.o: ../global/tree_basic.h ../global/tree_compound.h
analysis.o: ../global/Error.h /usr/include/string.h
analysis.o: /usr/include/sys/stdtypes.h /usr/include/stdlib.h
analysis.o: ../global/globals.h ../global/traverse.h
cccall.o: /usr/include/stdio.h ../global/types.h ../global/tree_basic.h
cccall.o: ../global/tree_compound.h ../global/Error.h /usr/include/string.h
cccall.o: /usr/include/sys/stdtypes.h /usr/include/stdlib.h
cccall.o: ../global/globals.h ../../inc/dbug.h ../global/internal_lib.h
cccall.o: ../global/free.h ../global/my_debug.h /usr/include/malloc.h
cccall.o: ../scanparse/scnprs.h ../scanparse/y.tab.h filemgr.h
cccall.o: ../global/traverse.h
checkdec.o: /usr/include/string.h /usr/include/sys/stdtypes.h
checkdec.o: ../global/types.h ../global/tree_basic.h
checkdec.o: ../global/tree_compound.h ../global/Error.h /usr/include/stdio.h
checkdec.o: /usr/include/stdlib.h ../global/globals.h ../../inc/dbug.h
checkdec.o: ../global/my_debug.h ../global/traverse.h ../global/free.h
checkdec.o: /usr/include/malloc.h import.h ../print/convert.h filemgr.h
checkdec.o: ../scanparse/scnprs.h ../scanparse/y.tab.h implicittypes.h
checkdec.o: ../print/print.h ../global/tree.h ../global/internal_lib.h
filemgr.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
filemgr.o: /usr/include/string.h ../../inc/dbug.h /usr/include/stdio.h
filemgr.o: ../global/internal_lib.h ../global/Error.h ../global/types.h
filemgr.o: ../global/globals.h filemgr.h
implicittypes.o: /usr/include/malloc.h ../global/types.h
implicittypes.o: ../global/tree_basic.h ../global/tree_compound.h
implicittypes.o: ../global/Error.h /usr/include/stdio.h /usr/include/string.h
implicittypes.o: /usr/include/sys/stdtypes.h /usr/include/stdlib.h
implicittypes.o: ../global/globals.h ../print/convert.h ../global/free.h
implicittypes.o: ../../inc/dbug.h ../global/my_debug.h ../global/traverse.h
import.o: /usr/include/string.h /usr/include/sys/stdtypes.h
import.o: /usr/include/limits.h /usr/include/malloc.h ../global/tree.h
import.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
import.o: ../global/Error.h /usr/include/stdio.h /usr/include/stdlib.h
import.o: ../global/globals.h ../global/internal_lib.h ../global/free.h
import.o: ../../inc/dbug.h ../global/my_debug.h ../scanparse/scnprs.h
import.o: ../scanparse/y.tab.h ../global/traverse.h ../global/access_macros.h
import.o: filemgr.h import.h
readsib.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
readsib.o: ../global/Error.h /usr/include/stdio.h /usr/include/string.h
readsib.o: /usr/include/sys/stdtypes.h /usr/include/stdlib.h
readsib.o: ../global/globals.h ../global/free.h ../../inc/dbug.h
readsib.o: ../global/my_debug.h /usr/include/malloc.h
readsib.o: ../global/internal_lib.h ../global/traverse.h import.h
readsib.o: ../scanparse/scnprs.h ../scanparse/y.tab.h filemgr.h
sib.o: /usr/include/string.h /usr/include/sys/stdtypes.h
sib.o: /usr/include/malloc.h ../global/types.h ../global/tree_basic.h
sib.o: ../global/tree_compound.h ../global/Error.h /usr/include/stdio.h
sib.o: /usr/include/stdlib.h ../global/globals.h ../global/free.h
sib.o: ../../inc/dbug.h ../global/my_debug.h ../global/traverse.h
sib.o: ../print/convert.h filemgr.h sib.h
writesib.o: /usr/include/string.h /usr/include/sys/stdtypes.h
writesib.o: /usr/include/malloc.h ../global/types.h ../global/tree_basic.h
writesib.o: ../global/tree_compound.h ../global/Error.h /usr/include/stdio.h
writesib.o: /usr/include/stdlib.h ../global/globals.h ../global/free.h
writesib.o: ../../inc/dbug.h ../global/my_debug.h ../global/traverse.h
writesib.o: ../print/convert.h filemgr.h ../print/print.h ../global/tree.h
writesib.o: ../global/internal_lib.h ../typecheck/typecheck.h
