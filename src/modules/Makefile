#
# $Log$
# Revision 2.4  1999/05/12 15:13:26  cg
# make deps done
#
# Revision 2.3  1999/03/11 18:18:23  dkr
# make deps done
#
# Revision 2.2  1999/02/26 14:17:40  dkr
# make deps done
#
# Revision 2.1  1999/02/23 12:41:56  sacbase
# new release made
#
# Revision 1.36  1999/02/09 20:04:22  dkr
# make deps done
#
# Revision 1.35  1998/11/04 18:22:35  dkr
# make deps done
#
# Revision 1.34  1998/07/03 14:34:10  cg
# added ../compile as include directory
#
# Revision 1.33  1998/06/23 11:42:11  cg
# make deps done
#
# Revision 1.32  1998/04/26 14:42:14  dkr
# make deps done
#
# Revision 1.31  1998/04/09 23:47:32  dkr
# make deps done
#
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

INCS=-I../global -I../../inc -I../scanparse -I../print -I../typecheck \
     -I../compile

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
analysis.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
analysis.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
analysis.o: /usr/include/stdio_impl.h ../global/types.h
analysis.o: ../global/tree_basic.h ../global/tree_compound.h
analysis.o: ../global/Error.h /usr/include/string.h /usr/include/stdlib.h
analysis.o: ../global/globals.h ../global/traverse.h
cccall.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
cccall.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
cccall.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
cccall.o: /usr/include/time.h /usr/include/sys/types.h
cccall.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
cccall.o: /usr/include/stdlib.h ../global/types.h ../global/tree_basic.h
cccall.o: ../global/tree_compound.h ../global/Error.h /usr/include/string.h
cccall.o: ../global/globals.h ../../inc/dbug.h ../global/internal_lib.h
cccall.o: ../global/free.h ../global/my_debug.h /usr/include/malloc.h
cccall.o: ../scanparse/scnprs.h ../global/resource.h ../scanparse/y.tab.h
cccall.o: filemgr.h ../global/traverse.h ../compile/gen_startup_code.h
checkdec.o: /usr/include/string.h /usr/include/sys/feature_tests.h
checkdec.o: /usr/include/sys/isa_defs.h ../global/types.h
checkdec.o: ../global/tree_basic.h ../global/tree_compound.h
checkdec.o: ../global/Error.h /usr/include/stdio.h /usr/include/sys/va_list.h
checkdec.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
checkdec.o: /usr/include/stdlib.h ../global/globals.h ../../inc/dbug.h
checkdec.o: ../global/my_debug.h ../global/traverse.h ../global/free.h
checkdec.o: /usr/include/malloc.h /usr/include/sys/types.h
checkdec.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
checkdec.o: ../global/internal_lib.h import.h ../print/convert.h filemgr.h
checkdec.o: ../scanparse/scnprs.h ../global/resource.h ../scanparse/y.tab.h
checkdec.o: implicittypes.h ../print/print.h ../global/tree.h
filemgr.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
filemgr.o: /usr/include/sys/isa_defs.h /usr/include/string.h
filemgr.o: /usr/include/stdarg.h /usr/include/varargs.h
filemgr.o: /usr/include/sys/va_list.h /usr/include/unistd.h
filemgr.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
filemgr.o: /usr/include/sys/int_types.h /usr/include/sys/unistd.h
filemgr.o: ../../inc/dbug.h /usr/include/stdio.h /usr/include/stdio_tag.h
filemgr.o: /usr/include/stdio_impl.h ../global/internal_lib.h
filemgr.o: ../global/types.h ../global/Error.h ../global/globals.h filemgr.h
filemgr.o: ../global/resource.h
implicittypes.o: ../global/types.h ../global/tree_basic.h
implicittypes.o: ../global/tree_compound.h ../global/Error.h
implicittypes.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
implicittypes.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
implicittypes.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
implicittypes.o: /usr/include/string.h /usr/include/stdlib.h
implicittypes.o: ../global/globals.h ../print/convert.h ../global/free.h
implicittypes.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
implicittypes.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
implicittypes.o: /usr/include/sys/int_types.h ../global/internal_lib.h
implicittypes.o: ../global/traverse.h
import.o: /usr/include/string.h /usr/include/sys/feature_tests.h
import.o: /usr/include/sys/isa_defs.h /usr/include/limits.h ../global/tree.h
import.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
import.o: ../global/Error.h /usr/include/stdio.h /usr/include/sys/va_list.h
import.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
import.o: /usr/include/stdlib.h ../global/globals.h ../global/internal_lib.h
import.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
import.o: /usr/include/malloc.h /usr/include/sys/types.h
import.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
import.o: ../scanparse/scnprs.h ../global/resource.h ../scanparse/y.tab.h
import.o: ../global/traverse.h ../global/access_macros.h filemgr.h import.h
readsib.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
readsib.o: ../global/Error.h /usr/include/stdio.h
readsib.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
readsib.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
readsib.o: /usr/include/stdio_impl.h /usr/include/string.h
readsib.o: /usr/include/stdlib.h ../global/globals.h ../global/free.h
readsib.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
readsib.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
readsib.o: /usr/include/sys/int_types.h ../global/internal_lib.h
readsib.o: ../global/traverse.h import.h ../scanparse/scnprs.h
readsib.o: ../global/resource.h ../scanparse/y.tab.h filemgr.h cccall.h
readsib.o: ../typecheck/gen_pseudo_fun.h
writesib.o: /usr/include/string.h /usr/include/sys/feature_tests.h
writesib.o: /usr/include/sys/isa_defs.h ../global/types.h
writesib.o: ../global/tree_basic.h ../global/tree_compound.h
writesib.o: ../global/Error.h /usr/include/stdio.h /usr/include/sys/va_list.h
writesib.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
writesib.o: /usr/include/stdlib.h ../global/globals.h ../global/free.h
writesib.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
writesib.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
writesib.o: /usr/include/sys/int_types.h ../global/internal_lib.h
writesib.o: ../global/traverse.h ../print/convert.h filemgr.h
writesib.o: ../print/print.h ../global/tree.h ../typecheck/typecheck.h
