#
#
# $Log$
# Revision 3.8  2001/11/13 20:32:44  dkr
# make deps done
#
# Revision 3.7  2001/07/18 13:08:17  cg
# make deps done
#
# Revision 3.6  2001/03/22 20:20:00  dkr
# make deps done
#
# Revision 3.5  2001/03/21 11:19:56  dkr
# make deps done
#
# Revision 3.4  2001/02/02 10:39:02  dkr
# make deps done
#
# Revision 3.3  2001/01/17 10:13:04  dkr
# make deps done
#
# Revision 3.2  2000/12/06 20:09:03  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:00:46  sacbase
# new release made
#
# Revision 2.19  2000/11/02 14:23:46  dkr
# header compressed
#
# Revision 2.15  2000/07/24 14:44:33  dkr
# suffix rule for .c.o added
#
# Revision 2.9  2000/01/21 14:45:28  dkr
# directory tree added
#
# Revision 2.8  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 1.28  1998/03/04 00:10:55  dkr
# 'clean' now deletes the subdir .sb, too
#
# Revision 1.24  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.16  1996/01/02  16:01:02  cg
# added compilation of cccall.c
#
# Revision 1.15  1995/12/29  10:38:56  cg
# added compilation of readsib.c
#
# Revision 1.12  1995/10/22  17:40:38  cg
# a bloody bug fixed.
#
# Revision 1.11  1995/10/20  09:28:12  cg
# added analysis.c
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
# OS setup:
#
# legal values for OS currently are:
#    SOLARIS_SPARC
#    LINUX_X86
#    OSF_ALPHA
#
OS        := SOLARIS_SPARC


CC        := gcc
CCFLAGS   := -ansi -Wall -g -DSAC_FOR_$(OS)
CFLAGS    := -D__EXTENSIONS__

INCS=-I../global -I../tree -I../../inc -I../scanparse -I../print -I../typecheck \
     -I../compile

all: filemgr.o import.o readsib.o writesib.o implicittypes.o analysis.o \
     checkdec.o cccall.o

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak *.sac *.sib *.dec
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

analysis.o: ../../inc/dbug.h /usr/include/stdio.h
analysis.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
analysis.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
analysis.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
analysis.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
analysis.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
analysis.o: ../global/Error.h /usr/include/string.h
analysis.o: /usr/include/iso/string_iso.h ../global/globals.h
analysis.o: ../tree/traverse.h
cccall.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
cccall.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
cccall.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
cccall.o: /usr/include/stdio_impl.h /usr/include/time.h
cccall.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
cccall.o: /usr/include/sys/int_types.h /usr/include/iso/time_iso.h
cccall.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
cccall.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
cccall.o: ../global/Error.h /usr/include/string.h
cccall.o: /usr/include/iso/string_iso.h ../global/globals.h ../../inc/dbug.h
cccall.o: ../global/internal_lib.h ../tree/free.h ../global/my_debug.h
cccall.o: filemgr.h ../tree/traverse.h ../global/resource.h
cccall.o: ../compile/gen_startup_code.h
checkdec.o: /usr/include/string.h /usr/include/iso/string_iso.h
checkdec.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
checkdec.o: ../global/types.h ../tree/prf.h ../tree/tree_basic.h
checkdec.o: ../tree/tree_compound.h ../global/Error.h /usr/include/stdio.h
checkdec.o: /usr/include/iso/stdio_iso.h /usr/include/sys/va_list.h
checkdec.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
checkdec.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
checkdec.o: ../global/globals.h ../../inc/dbug.h ../global/my_debug.h
checkdec.o: ../tree/traverse.h ../tree/free.h ../global/internal_lib.h
checkdec.o: import.h ../print/convert.h filemgr.h implicittypes.h
checkdec.o: ../print/print.h checkdec.h
filemgr.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
filemgr.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
filemgr.o: /usr/include/string.h /usr/include/iso/string_iso.h
filemgr.o: /usr/include/stdarg.h /usr/include/varargs.h
filemgr.o: /usr/include/sys/va_list.h /usr/include/unistd.h
filemgr.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
filemgr.o: /usr/include/sys/int_types.h /usr/include/sys/unistd.h
filemgr.o: ../../inc/dbug.h /usr/include/stdio.h /usr/include/iso/stdio_iso.h
filemgr.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
filemgr.o: ../global/internal_lib.h ../global/types.h ../global/Error.h
filemgr.o: ../global/globals.h ../tree/free.h ../global/my_debug.h filemgr.h
filemgr.o: ../global/resource.h
implicittypes.o: ../global/types.h ../tree/tree_basic.h
implicittypes.o: ../tree/tree_compound.h ../global/Error.h
implicittypes.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
implicittypes.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
implicittypes.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
implicittypes.o: /usr/include/stdio_impl.h /usr/include/string.h
implicittypes.o: /usr/include/iso/string_iso.h /usr/include/stdlib.h
implicittypes.o: /usr/include/iso/stdlib_iso.h ../global/globals.h
implicittypes.o: ../tree/DupTree.h ../tree/LookUpTable.h ../print/convert.h
implicittypes.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
implicittypes.o: ../global/internal_lib.h ../tree/traverse.h
import.o: /usr/include/string.h /usr/include/iso/string_iso.h
import.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
import.o: /usr/include/limits.h /usr/include/iso/limits_iso.h
import.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
import.o: ../global/Error.h /usr/include/stdio.h /usr/include/iso/stdio_iso.h
import.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
import.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
import.o: /usr/include/iso/stdlib_iso.h ../global/globals.h
import.o: ../global/internal_lib.h ../tree/free.h ../../inc/dbug.h
import.o: ../global/my_debug.h ../scanparse/scnprs.h ../global/resource.h
import.o: ../scanparse/y.tab.h ../tree/traverse.h filemgr.h import.h
readsib.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
readsib.o: ../global/Error.h /usr/include/stdio.h
readsib.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
readsib.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
readsib.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
readsib.o: /usr/include/string.h /usr/include/iso/string_iso.h
readsib.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
readsib.o: ../global/globals.h ../tree/free.h ../../inc/dbug.h
readsib.o: ../global/my_debug.h ../global/internal_lib.h ../tree/traverse.h
readsib.o: import.h ../scanparse/scnprs.h ../global/resource.h
readsib.o: ../scanparse/y.tab.h filemgr.h cccall.h
readsib.o: ../typecheck/gen_pseudo_fun.h
writesib.o: /usr/include/string.h /usr/include/iso/string_iso.h
writesib.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
writesib.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
writesib.o: ../global/Error.h /usr/include/stdio.h
writesib.o: /usr/include/iso/stdio_iso.h /usr/include/sys/va_list.h
writesib.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
writesib.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
writesib.o: ../global/globals.h ../tree/free.h ../../inc/dbug.h
writesib.o: ../global/my_debug.h ../global/internal_lib.h ../tree/traverse.h
writesib.o: ../print/convert.h filemgr.h ../print/print.h
