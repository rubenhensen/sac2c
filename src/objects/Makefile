#
# $Log$
# Revision 2.5  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 2.4  1999/12/13 11:27:22  dkr
# *** empty log message ***
#
# Revision 2.3  1999/03/11 18:18:26  dkr
# make deps done
#
# Revision 2.2  1999/02/26 14:17:43  dkr
# make deps done
#
# Revision 2.1  1999/02/23 12:43:19  sacbase
# new release made
#
# Revision 1.14  1998/03/04 00:11:13  dkr
# 'clean' now deletes the subdir .sb, too
#
# Revision 1.13  1997/11/20 15:02:11  dkr
# make deps done
#
# Revision 1.12  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.11  1997/11/11 16:49:33  sbs
# make deps done
#
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

RM=rm -f

INCS=-I../global -I../../inc -I../typecheck -I../modules

all: objinit.o objects.o uniquecheck.o rmvoidfun.o


objinit.o: objinit.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o objinit.o -c objinit.c $(INCS)

objects.o: objects.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o objects.o -c objects.c $(INCS)

uniquecheck.o: uniquecheck.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o uniquecheck.o -c uniquecheck.c $(INCS)

rmvoidfun.o: rmvoidfun.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o rmvoidfun.o -c rmvoidfun.c $(INCS)


deps:
	makedepend $(INCS) *.c

clean:
	$(RM) objinit.o objects.o uniquecheck.o rmvoidfun.o  *.bak
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

objects.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
objects.o: ../global/Error.h /usr/include/stdio.h
objects.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
objects.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
objects.o: /usr/include/stdio_impl.h /usr/include/string.h
objects.o: /usr/include/stdlib.h ../global/globals.h ../../inc/dbug.h
objects.o: ../global/traverse.h ../global/internal_lib.h
objinit.o: /usr/include/string.h /usr/include/sys/feature_tests.h
objinit.o: /usr/include/sys/isa_defs.h ../global/types.h
objinit.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
objinit.o: /usr/include/stdio.h /usr/include/sys/va_list.h
objinit.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
objinit.o: /usr/include/stdlib.h ../global/globals.h ../global/traverse.h
objinit.o: ../../inc/dbug.h ../global/internal_lib.h ../global/free.h
objinit.o: ../global/my_debug.h /usr/include/malloc.h
objinit.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
objinit.o: /usr/include/sys/int_types.h ../typecheck/typecheck.h
objinit.o: ../modules/filemgr.h
rmvoidfun.o: ../global/types.h ../global/tree_basic.h
rmvoidfun.o: ../global/tree_compound.h ../global/Error.h /usr/include/stdio.h
rmvoidfun.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
rmvoidfun.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
rmvoidfun.o: /usr/include/stdio_impl.h /usr/include/string.h
rmvoidfun.o: /usr/include/stdlib.h ../global/globals.h ../../inc/dbug.h
rmvoidfun.o: ../global/traverse.h ../global/internal_lib.h ../global/free.h
rmvoidfun.o: ../global/my_debug.h /usr/include/malloc.h
rmvoidfun.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
rmvoidfun.o: /usr/include/sys/int_types.h
uniquecheck.o: ../global/types.h ../global/tree_basic.h
uniquecheck.o: ../global/tree_compound.h ../global/Error.h
uniquecheck.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
uniquecheck.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
uniquecheck.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
uniquecheck.o: /usr/include/string.h /usr/include/stdlib.h
uniquecheck.o: ../global/globals.h ../../inc/dbug.h ../global/traverse.h
uniquecheck.o: ../global/internal_lib.h ../global/free.h ../global/my_debug.h
uniquecheck.o: /usr/include/malloc.h /usr/include/sys/types.h
uniquecheck.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
