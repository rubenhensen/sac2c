#
#
# $Log$
# Revision 3.10  2001/11/13 20:32:47  dkr
# make deps done
#
# Revision 3.9  2001/07/18 13:08:20  cg
# make deps done
#
# Revision 3.8  2001/05/08 13:29:40  dkr
# make deps done
#
# Revision 3.7  2001/04/26 21:09:52  dkr
# make deps done
#
# Revision 3.6  2001/01/17 10:13:08  dkr
# make deps done
#
# Revision 3.5  2000/12/15 18:18:04  dkr
# make deps done
#
# Revision 3.4  2000/12/12 15:09:15  dkr
# make deps done
#
# Revision 3.3  2000/12/08 15:21:10  dkr
# include path extended
#
# Revision 3.2  2000/12/06 20:09:08  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:02:23  sacbase
# new release made
#
# Revision 2.24  2000/11/02 14:39:00  dkr
# header compressed
#
# Revision 2.14  2000/01/24 11:54:55  jhs
# Moved scheduling.[ch] to tree.
#
# Revision 2.12  2000/01/21 12:03:29  dkr
# directory tree added
#
# Revision 2.10  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 2.8  1999/07/30 13:45:12  jhs
# Added concurrent_lib.[ch], make deps done.
#
# Revision 2.7  1999/07/28 08:32:46  jhs
# Added spmd_cons.o.
#
# Revision 2.5  1999/06/25 15:00:17  jhs
# Added spmd_trav.c.
#
# Revision 1.14  1998/06/18 13:26:26  cg
# added compilation of spmd_init.c spmd_opt.c spmd_lift.c sync_init.c
# sync_opt.c sched.c scheduling.c
# removed compilation of spmdregions.c
#
# Revision 1.13  1998/06/12 14:18:07  cg
# added compilation of scheduling.c
#
# Revision 1.10  1998/04/30 13:56:13  dkr
# added path to INCS
#
# Revision 1.8  1998/04/23 22:08:51  dkr
# changed INCS
#
# Revision 1.5  1998/04/17 17:24:45  dkr
# concregions.[ch] renamed to spmdregions.[ch]
#
# Revision 1.3  1998/04/08 22:11:16  dkr
# expanded INCS
#
# Revision 1.1  1998/04/03 11:47:26  dkr
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

INCS= -I../global -I../tree -I../../inc -I../optimize -I../typecheck -I../refcount \
      -I../flatten -I../print -I../compile

all: concurrent.o spmd_init.o spmd_opt.o spmd_lift.o sync_init.o \
     sync_opt.o schedule.o spmd_trav.o spmd_cons.o concurrent_lib.o

%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak *~
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

concurrent.o: ../../inc/dbug.h /usr/include/stdio.h
concurrent.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
concurrent.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
concurrent.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
concurrent.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
concurrent.o: ../global/types.h ../tree/tree_basic.h ../tree/traverse.h
concurrent.o: ../global/globals.h ../tree/free.h ../global/my_debug.h
concurrent.o: ../global/internal_lib.h ../global/Error.h
concurrent.o: /usr/include/string.h /usr/include/iso/string_iso.h
concurrent.o: ../tree/InferDFMs.h
concurrent_lib.o: ../../inc/dbug.h /usr/include/stdio.h
concurrent_lib.o: /usr/include/iso/stdio_iso.h
concurrent_lib.o: /usr/include/sys/feature_tests.h
concurrent_lib.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
concurrent_lib.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
concurrent_lib.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
concurrent_lib.o: ../tree/tree_basic.h ../global/types.h ../tree/free.h
concurrent_lib.o: ../global/my_debug.h ../global/globals.h
concurrent_lib.o: ../global/internal_lib.h ../tree/DupTree.h
concurrent_lib.o: ../tree/LookUpTable.h ../tree/DataFlowMask.h
schedule.o: ../../inc/dbug.h /usr/include/stdio.h
schedule.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
schedule.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
schedule.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
schedule.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
schedule.o: ../global/types.h ../tree/tree_basic.h ../tree/traverse.h
schedule.o: ../tree/scheduling.h
spmd_cons.o: ../../inc/dbug.h /usr/include/stdio.h
spmd_cons.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
spmd_cons.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
spmd_cons.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
spmd_cons.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
spmd_cons.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
spmd_cons.o: ../global/Error.h /usr/include/string.h
spmd_cons.o: /usr/include/iso/string_iso.h ../global/globals.h
spmd_cons.o: ../tree/traverse.h ../tree/DupTree.h ../tree/LookUpTable.h
spmd_cons.o: ../tree/DataFlowMask.h ../global/internal_lib.h ../tree/free.h
spmd_cons.o: ../global/my_debug.h ../typecheck/typecheck.h concurrent_lib.h
spmd_cons.o: ../refcount/refcount.h spmd_cons.h
spmd_init.o: ../../inc/dbug.h /usr/include/stdio.h
spmd_init.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
spmd_init.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
spmd_init.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
spmd_init.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
spmd_init.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
spmd_init.o: ../global/Error.h /usr/include/string.h
spmd_init.o: /usr/include/iso/string_iso.h ../global/globals.h
spmd_init.o: ../tree/traverse.h ../tree/DupTree.h ../tree/LookUpTable.h
spmd_init.o: ../tree/DataFlowMask.h ../global/internal_lib.h
spmd_init.o: ../global/my_debug.h spmd_trav.h ../optimize/generatemasks.h
spmd_init.o: spmd_opt.h
spmd_lift.o: ../../inc/dbug.h /usr/include/stdio.h
spmd_lift.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
spmd_lift.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
spmd_lift.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
spmd_lift.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
spmd_lift.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
spmd_lift.o: ../global/Error.h /usr/include/string.h
spmd_lift.o: /usr/include/iso/string_iso.h ../global/globals.h
spmd_lift.o: ../tree/traverse.h ../tree/DupTree.h ../tree/LookUpTable.h
spmd_lift.o: ../tree/DataFlowMask.h ../global/internal_lib.h concurrent_lib.h
spmd_lift.o: ../refcount/refcount.h ../tree/InferDFMs.h
spmd_opt.o: ../../inc/dbug.h /usr/include/stdio.h
spmd_opt.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
spmd_opt.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
spmd_opt.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
spmd_opt.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
spmd_opt.o: ../global/types.h ../tree/tree_basic.h ../tree/traverse.h
spmd_opt.o: ../tree/free.h ../global/my_debug.h ../global/globals.h
spmd_opt.o: ../global/internal_lib.h ../tree/DupTree.h ../tree/LookUpTable.h
spmd_opt.o: ../tree/DataFlowMask.h spmd_trav.h concurrent_lib.h
spmd_opt.o: ../refcount/refcount.h spmd_opt.h
spmd_trav.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
spmd_trav.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
spmd_trav.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
spmd_trav.o: /usr/include/stdio_impl.h ../../inc/dbug.h /usr/include/stdlib.h
spmd_trav.o: /usr/include/iso/stdlib_iso.h ../tree/DataFlowMask.h
spmd_trav.o: ../global/types.h ../tree/traverse.h ../global/internal_lib.h
spmd_trav.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
spmd_trav.o: /usr/include/string.h /usr/include/iso/string_iso.h
spmd_trav.o: ../global/globals.h ../tree/free.h ../global/my_debug.h
spmd_trav.o: ../refcount/refcount.h spmd_trav.h
sync_init.o: ../../inc/dbug.h /usr/include/stdio.h
sync_init.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
sync_init.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
sync_init.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
sync_init.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
sync_init.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
sync_init.o: ../global/Error.h /usr/include/string.h
sync_init.o: /usr/include/iso/string_iso.h ../global/globals.h
sync_init.o: ../tree/traverse.h ../tree/DupTree.h ../tree/LookUpTable.h
sync_init.o: ../tree/DataFlowMask.h ../global/internal_lib.h
sync_opt.o: ../../inc/dbug.h /usr/include/stdio.h
sync_opt.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
sync_opt.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
sync_opt.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
sync_opt.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
sync_opt.o: ../global/types.h ../tree/tree_basic.h ../tree/traverse.h
sync_opt.o: ../tree/free.h ../global/my_debug.h ../global/globals.h
sync_opt.o: ../global/internal_lib.h ../tree/DupTree.h ../tree/LookUpTable.h
sync_opt.o: ../tree/DataFlowMask.h spmd_trav.h ../global/Error.h
sync_opt.o: /usr/include/string.h /usr/include/iso/string_iso.h
sync_opt.o: concurrent_lib.h ../refcount/refcount.h sync_opt.h
