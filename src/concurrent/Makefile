#
# $Log$
# Revision 2.1  1999/02/23 12:44:03  sacbase
# new release made
#
# Revision 1.16  1998/11/04 18:22:43  dkr
# make deps done
#
# Revision 1.15  1998/06/23 11:42:17  cg
# make deps done
#
# Revision 1.14  1998/06/18 13:26:26  cg
# added compilation of spmd_init.c spmd_opt.c spmd_lift.c sync_init.c
# sync_opt.c sched.c scheduling.c
# removed compilation of spmdregions.c
#
# Revision 1.13  1998/06/12 14:18:07  cg
# added compilation of scheduling.c
#
# Revision 1.12  1998/05/12 14:22:28  dkr
# make deps done
#
# Revision 1.11  1998/05/03 14:04:43  dkr
# make deps done
#
# Revision 1.10  1998/04/30 13:56:13  dkr
# added path to INCS
#
# Revision 1.9  1998/04/25 13:17:53  dkr
# make deps done
#
# Revision 1.8  1998/04/23 22:08:51  dkr
# changed INCS
#
# Revision 1.7  1998/04/20 02:43:57  dkr
# make deps done
#
# Revision 1.6  1998/04/20 01:09:36  dkr
# make deps done
#
# Revision 1.5  1998/04/17 17:24:45  dkr
# concregions.[ch] renamed to spmdregions.[ch]
#
# Revision 1.4  1998/04/09 12:04:04  dkr
# make deps done
#
# Revision 1.3  1998/04/08 22:11:16  dkr
# expanded INCS
#
# Revision 1.2  1998/04/03 11:52:52  srs
# made deps
#
# Revision 1.1  1998/04/03 11:47:26  dkr
# Initial revision
#



CC        := gcc
CCFLAGS   := -ansi -Wall -g
CFLAGS    := -D__EXTENSIONS__

INCS=-I../global -I../../inc -I../optimize -I../typecheck -I../refcount -I../print  \
     -I../compile

all: concurrent.o scheduling.o spmd_init.o spmd_opt.o spmd_lift.o sync_init.o \
     sync_opt.o schedule.o

%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak *~
	$(RM) -r .sb

# DO NOT DELETE THIS LINE -- make depend depends on it.

concurrent.o: ../../inc/dbug.h /usr/include/stdio.h
concurrent.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
concurrent.o: ../global/types.h ../global/tree_basic.h ../global/traverse.h
concurrent.o: ../global/globals.h ../global/free.h ../global/my_debug.h
concurrent.o: /usr/include/malloc.h ../global/internal_lib.h
schedule.o: ../../inc/dbug.h /usr/include/stdio.h
schedule.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
schedule.o: ../global/types.h ../global/tree_basic.h ../global/traverse.h
schedule.o: scheduling.h
scheduling.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
scheduling.o: /usr/include/sys/va_list.h /usr/include/string.h
scheduling.o: /usr/include/varargs.h ../global/types.h ../global/tree_basic.h
scheduling.o: ../global/internal_lib.h ../global/free.h ../../inc/dbug.h
scheduling.o: ../global/my_debug.h /usr/include/malloc.h ../global/traverse.h
scheduling.o: ../global/Error.h /usr/include/stdlib.h ../global/globals.h
scheduling.o: ../compile/precompile.h
spmd_init.o: ../../inc/dbug.h /usr/include/stdio.h
spmd_init.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
spmd_init.o: ../global/types.h ../global/tree_basic.h
spmd_init.o: ../global/tree_compound.h ../global/Error.h
spmd_init.o: /usr/include/string.h /usr/include/stdlib.h ../global/globals.h
spmd_init.o: ../global/traverse.h ../optimize/DupTree.h
spmd_init.o: ../optimize/DataFlowMask.h
spmd_lift.o: ../../inc/dbug.h /usr/include/stdio.h
spmd_lift.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
spmd_lift.o: ../global/types.h ../global/tree_basic.h
spmd_lift.o: ../global/tree_compound.h ../global/Error.h
spmd_lift.o: /usr/include/string.h /usr/include/stdlib.h ../global/globals.h
spmd_lift.o: ../global/traverse.h ../optimize/DupTree.h
spmd_lift.o: ../optimize/DataFlowMask.h ../typecheck/typecheck.h
spmd_lift.o: ../refcount/refcount.h ../global/internal_lib.h
spmd_opt.o: ../../inc/dbug.h /usr/include/stdio.h
spmd_opt.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
spmd_opt.o: ../global/types.h ../global/tree_basic.h ../global/traverse.h
sync_init.o: ../../inc/dbug.h /usr/include/stdio.h
sync_init.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
sync_init.o: ../global/types.h ../global/tree_basic.h ../global/traverse.h
sync_init.o: ../optimize/DupTree.h ../optimize/DataFlowMask.h
sync_opt.o: ../../inc/dbug.h /usr/include/stdio.h
sync_opt.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
sync_opt.o: ../global/types.h ../global/tree_basic.h ../global/traverse.h

