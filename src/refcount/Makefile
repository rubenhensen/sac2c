#
# $Log$
# Revision 2.13  2000/07/24 14:46:34  dkr
# suffix rule for .c.o added
#
# Revision 2.12  2000/07/11 15:56:27  dkr
# make deps done
#
# Revision 2.11  2000/05/29 14:36:41  dkr
# make deps done
#
# Revision 2.10  2000/05/29 14:35:17  dkr
# make deps done
#
# Revision 2.9  2000/03/31 09:33:45  jhs
# make deps done
#
# Revision 2.8  2000/03/01 14:52:37  dkr
# make deps done
#
# Revision 2.7  2000/01/21 17:02:08  dkr
# make deps done
#
# Revision 2.6  2000/01/21 12:03:36  dkr
# directory tree added
#
# Revision 2.5  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 2.3  1999/03/11 18:18:24  dkr
# make deps done
#
# Revision 2.2  1999/02/26 14:17:42  dkr
# make deps done
#
# Revision 2.1  1999/02/23 12:43:03  sacbase
# new release made
#
# Revision 1.15  1999/02/09 20:04:24  dkr
# make deps done
#
# Revision 1.14  1998/05/12 14:22:24  dkr
# make deps done
#
# Revision 1.13  1998/03/04 00:10:18  dkr
# 'clean' now deletes the subdir .sb, too
#
# Revision 1.12  1998/02/16 21:44:02  dkr
# make deps done
#
# Revision 1.11  1998/02/09 17:40:31  dkr
# added INCS: -I../optimize (function GenerateMasks)
#
# Revision 1.10  1997/11/20 15:02:10  dkr
# make deps done
#
# Revision 1.9  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.8  1997/11/11 16:49:31  sbs
# make deps done
#
# Revision 1.7  1997/08/05 11:32:30  sbs
# make deps done
#
# Revision 1.6  1996/02/16 09:46:55  sbs
# make deps done
#
# Revision 1.5  1996/01/04  18:11:11  cg
# make deps done
#
# Revision 1.4  1995/12/15  10:41:12  cg
# make deps done
#
# Revision 1.3  1995/10/08  11:29:58  sbs
# make deps done
#
# Revision 1.2  1995/03/28  12:09:23  hw
# make deps done
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


INCS=-I../global -I../tree -I../../inc -I../typecheck -I../optimize

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
refcount.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
refcount.o: ../tree/tree_compound.h ../global/Error.h /usr/include/stdio.h
refcount.o: /usr/include/sys/va_list.h /usr/include/string.h
refcount.o: ../global/globals.h ../global/internal_lib.h ../global/my_debug.h
refcount.o: ../../inc/dbug.h ../tree/DupTree.h ../tree/LookUpTable.h
refcount.o: ../tree/DataFlowMask.h ../tree/traverse.h ../optimize/optimize.h
refcount.o: ../tree/free.h /usr/include/malloc.h ../optimize/generatemasks.h
refcount.o: refcount.h
