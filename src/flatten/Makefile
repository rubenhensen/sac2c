#
# $Log$
# Revision 2.11  2000/02/17 16:16:55  cg
# Added compilation of adjust_ids.c and fun2lac.c.
#
# Revision 2.10  2000/01/23 15:01:10  dkr
# make deps done
#
# Revision 2.9  2000/01/21 17:02:02  dkr
# make deps done
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
# Revision 2.4  1999/05/12 15:13:21  cg
# make deps done
#
# Revision 2.3  1999/03/11 18:18:18  dkr
# make deps done
#
# Revision 2.2  1999/02/26 14:17:36  dkr
# make deps done
#
# Revision 2.1  1999/02/23 12:39:06  sacbase
# new release made
#
# Revision 1.17  1998/05/21 09:51:47  dkr
# make deps done
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
# Revision 1.11  1997/11/11 16:49:21  sbs
# make deps done
#
# Revision 1.10  1997/08/05 11:32:22  sbs
# make deps done
#
# Revision 1.9  1996/01/04 18:11:05  cg
# make deps done
#
# Revision 1.8  1995/12/15  10:41:06  cg
# make deps done
#
# Revision 1.7  1995/10/08  11:29:44  sbs
# make deps done
#
# Revision 1.6  1995/05/15  13:38:40  asi
# make deps done
#
# Revision 1.5  1995/04/04  11:03:44  sbs
# make deps done
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


INCS=-I../global -I../../inc -I../tree

all: flatten.o lac2fun.o fun2lac.o adjust_ids.o

flatten.o: flatten.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o flatten.o -c flatten.c $(INCS)

lac2fun.o: lac2fun.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o lac2fun.o -c lac2fun.c $(INCS)

fun2lac.o: fun2lac.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o fun2lac.o -c fun2lac.c $(INCS)

adjust_ids.o: adjust_ids.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o adjust_ids.o -c adjust_ids.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak *~
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

flatten.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
flatten.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
flatten.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
flatten.o: ../global/globals.h ../global/types.h ../global/Error.h
flatten.o: /usr/include/string.h /usr/include/stdlib.h ../../inc/dbug.h
flatten.o: ../global/my_debug.h ../global/internal_lib.h ../tree/tree_basic.h
flatten.o: ../tree/tree_compound.h ../tree/traverse.h ../tree/free.h
flatten.o: /usr/include/malloc.h /usr/include/sys/types.h
flatten.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
flatten.o: ../tree/DupTree.h flatten.h
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
lac2fun.o: ../tree/DataFlowMask.h ../tree/DataFlowMaskUtils.h
