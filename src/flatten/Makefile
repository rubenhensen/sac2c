#
#
# $Log$
# Revision 3.1  2000/11/20 17:59:15  sacbase
# new release made
#
# Revision 2.23  2000/11/02 14:23:58  dkr
# header compressed
#
# Revision 2.14  2000/03/17 15:56:56  dkr
# cleanup_decls.o added
#
# Revision 2.12  2000/02/18 16:32:31  cg
# Added -I../print
#
# Revision 2.11  2000/02/17 16:16:55  cg
# Added compilation of adjust_ids.c and fun2lac.c.
#
# Revision 2.8  2000/01/21 12:53:35  dkr
# target clean changed
#
# Revision 2.7  2000/01/21 12:02:30  dkr
# lac2fun.o added
#
# Revision 2.6  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 1.16  1998/05/20 20:16:07  sbs
# since flatten.c uses DupNode ../optimize had to be added
# to the include-paths!
#
# Revision 1.15  1998/03/04 00:09:44  dkr
# 'clean' now deletes the subdir .sb, too
#
# Revision 1.14  1998/02/05 17:08:35  dkr
# make deps done
#
# Revision 1.13  1997/11/20 15:02:02  dkr
# make deps done
#
# Revision 1.12  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.3  1994/11/10  15:39:42  sbs
# RCS-header inserted
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


INCS=-I../global -I../../inc -I../tree -I../print

all: flatten.o lac2fun.o cleanup_decls.o fun2lac.o adjust_ids.o

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak *~
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

adjust_ids.o: ../../inc/dbug.h /usr/include/stdio.h
adjust_ids.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
adjust_ids.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
adjust_ids.o: /usr/include/stdio_impl.h ../tree/tree.h ../global/types.h
adjust_ids.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
adjust_ids.o: /usr/include/string.h /usr/include/stdlib.h ../global/globals.h
adjust_ids.o: ../global/internal_lib.h ../tree/traverse.h ../tree/free.h
adjust_ids.o: ../global/my_debug.h /usr/include/malloc.h
adjust_ids.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
adjust_ids.o: /usr/include/sys/int_types.h ../tree/DupTree.h
adjust_ids.o: ../tree/LookUpTable.h ../print/print.h
cleanup_decls.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
cleanup_decls.o: ../tree/tree_compound.h ../global/Error.h
cleanup_decls.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
cleanup_decls.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
cleanup_decls.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
cleanup_decls.o: /usr/include/string.h /usr/include/stdlib.h
cleanup_decls.o: ../global/globals.h ../global/internal_lib.h
cleanup_decls.o: ../tree/traverse.h ../tree/free.h ../../inc/dbug.h
cleanup_decls.o: ../global/my_debug.h /usr/include/malloc.h
cleanup_decls.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
cleanup_decls.o: /usr/include/sys/int_types.h ../tree/DataFlowMask.h
cleanup_decls.o: ../tree/DataFlowMaskUtils.h ../tree/LookUpTable.h
flatten.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
flatten.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
flatten.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
flatten.o: ../global/globals.h ../global/types.h ../global/Error.h
flatten.o: /usr/include/string.h /usr/include/stdlib.h ../../inc/dbug.h
flatten.o: ../global/my_debug.h ../global/internal_lib.h ../tree/tree_basic.h
flatten.o: ../tree/tree_compound.h ../tree/traverse.h ../tree/free.h
flatten.o: /usr/include/malloc.h /usr/include/sys/types.h
flatten.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
flatten.o: ../tree/DupTree.h ../tree/LookUpTable.h flatten.h
fun2lac.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
fun2lac.o: ../tree/tree_compound.h ../global/Error.h /usr/include/stdio.h
fun2lac.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
fun2lac.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
fun2lac.o: /usr/include/stdio_impl.h /usr/include/string.h
fun2lac.o: /usr/include/stdlib.h ../global/globals.h ../global/internal_lib.h
fun2lac.o: ../tree/traverse.h ../tree/free.h ../../inc/dbug.h
fun2lac.o: ../global/my_debug.h /usr/include/malloc.h
fun2lac.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
fun2lac.o: /usr/include/sys/int_types.h adjust_ids.h
lac2fun.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
lac2fun.o: ../tree/tree_compound.h ../global/Error.h /usr/include/stdio.h
lac2fun.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
lac2fun.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
lac2fun.o: /usr/include/stdio_impl.h /usr/include/string.h
lac2fun.o: /usr/include/stdlib.h ../global/globals.h ../global/internal_lib.h
lac2fun.o: ../tree/traverse.h ../tree/free.h ../../inc/dbug.h
lac2fun.o: ../global/my_debug.h /usr/include/malloc.h
lac2fun.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
lac2fun.o: /usr/include/sys/int_types.h ../tree/DupTree.h
lac2fun.o: ../tree/LookUpTable.h ../tree/DataFlowMask.h
lac2fun.o: ../tree/DataFlowMaskUtils.h cleanup_decls.h

