#
#
# $Log$
# Revision 3.9  2001/03/22 20:20:02  dkr
# make deps done
#
# Revision 3.8  2001/02/02 10:39:04  dkr
# make deps done
#
# Revision 3.7  2001/01/17 10:13:05  dkr
# make deps done
#
# Revision 3.6  2000/12/15 18:18:01  dkr
# make deps done
#
# Revision 3.5  2000/12/12 15:09:10  dkr
# make deps done
#
# Revision 3.4  2000/12/12 11:38:39  dkr
# include path extended
#
# Revision 3.3  2000/12/07 13:45:36  dkr
# include path extended
#
# Revision 3.2  2000/12/06 20:09:05  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:01:34  sacbase
# new release made
#
# Revision 2.16  2000/11/02 14:23:00  dkr
# header compressed
#
# Revision 2.13  2000/07/24 14:46:34  dkr
# suffix rule for .c.o added
#
# Revision 2.6  2000/01/21 12:03:36  dkr
# directory tree added
#
# Revision 2.5  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 1.13  1998/03/04 00:10:18  dkr
# 'clean' now deletes the subdir .sb, too
#
# Revision 1.11  1998/02/09 17:40:31  dkr
# added INCS: -I../optimize (function GenerateMasks)
#
# Revision 1.9  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.1  1995/03/09  16:17:01  hw
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


INCS = -I../global -I../tree -I../../inc -I ../flatten -I../typecheck \
       -I../optimize -I../compile

all: refcount.o

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) refcount.o *.bak
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

refcount.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
refcount.o: /usr/include/sys/isa_defs.h ../global/types.h
refcount.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
refcount.o: /usr/include/stdio.h /usr/include/sys/va_list.h
refcount.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
refcount.o: /usr/include/string.h ../global/globals.h
refcount.o: ../global/internal_lib.h ../global/my_debug.h ../../inc/dbug.h
refcount.o: ../tree/traverse.h ../tree/free.h ../tree/DupTree.h
refcount.o: ../tree/LookUpTable.h ../tree/DataFlowMask.h ../tree/InferDFMs.h
refcount.o: ../optimize/generatemasks.h ../optimize/optimize.h refcount.h
