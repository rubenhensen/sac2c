#
# $Log$
# Revision 1.12  1995/10/22 17:40:38  cg
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

INCS=-I../global -I../../inc -I../scanparse -I../print

all: filemgr.o import.o sib.o implicittypes.o analysis.o checkdec.o

filemgr.o: filemgr.c
	$(CC) -o filemgr.o -c filemgr.c $(INCS)

import.o: import.c
	$(CC) -o import.o -c import.c $(INCS)

sib.o: sib.c
	$(CC) -o sib.o -c sib.c $(INCS)

implicittypes.o: implicittypes.c
	$(CC) -o implicittypes.o -c implicittypes.c $(INCS)

analysis.o: analysis.c
	$(CC) -o analysis.o -c analysis.c $(INCS)

checkdec.o: checkdec.c
	$(CC) -o checkdec.o -c checkdec.c $(INCS)


deps:
	makedepend $(INCS) *.c

clean:
	$(RM) filemgr.o import.o sib.o implicittypes.o analysis.o  \
              checkdec.o *.bak



# DO NOT DELETE THIS LINE -- make depend depends on it.

analysis.o: ../../inc/dbug.h /usr/include/stdio.h ../global/types.h
analysis.o: ../global/tree_basic.h ../global/tree_compound.h
analysis.o: ../global/Error.h ../global/traverse.h
checkdec.o: /usr/include/string.h /usr/include/sys/stdtypes.h
checkdec.o: ../global/types.h ../global/tree_basic.h
checkdec.o: ../global/tree_compound.h ../global/Error.h /usr/include/stdio.h
checkdec.o: ../../inc/dbug.h ../global/my_debug.h ../global/traverse.h
checkdec.o: import.h ../print/convert.h filemgr.h
filemgr.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
filemgr.o: /usr/include/string.h ../../inc/dbug.h /usr/include/stdio.h
filemgr.o: filemgr.h
implicittypes.o: /usr/include/malloc.h ../global/types.h
implicittypes.o: ../global/tree_basic.h ../global/tree_compound.h
implicittypes.o: ../global/Error.h /usr/include/stdio.h ../print/convert.h
implicittypes.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
implicittypes.o: ../global/traverse.h
import.o: /usr/include/string.h /usr/include/sys/stdtypes.h
import.o: /usr/include/limits.h /usr/include/malloc.h ../global/tree.h
import.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
import.o: ../global/Error.h /usr/include/stdio.h ../global/free.h
import.o: ../../inc/dbug.h ../global/my_debug.h ../global/internal_lib.h
import.o: ../scanparse/scnprs.h ../scanparse/y.tab.h ../global/traverse.h
import.o: ../global/access_macros.h filemgr.h import.h
sib.o: /usr/include/malloc.h /usr/include/string.h
sib.o: /usr/include/sys/stdtypes.h ../global/types.h ../global/tree_basic.h
sib.o: ../global/tree_compound.h ../global/Error.h /usr/include/stdio.h
sib.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
sib.o: ../scanparse/scnprs.h ../scanparse/y.tab.h ../global/traverse.h
sib.o: filemgr.h import.h ../print/print.h ../global/tree.h
sib.o: ../print/convert.h sib.h
