#
# $Log$
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

scheduling.o: /usr/include/string.h /usr/include/sys/feature_tests.h
scheduling.o: /usr/include/varargs.h /usr/include/sys/va_list.h
scheduling.o: ../global/types.h ../global/tree_basic.h
scheduling.o: ../global/internal_lib.h ../global/free.h ../../inc/dbug.h
scheduling.o: /usr/include/stdio.h ../global/my_debug.h /usr/include/malloc.h
scheduling.o: ../global/traverse.h ../global/Error.h /usr/include/stdlib.h
scheduling.o: ../global/globals.h ../compile/precompile.h
spmdregions.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
spmdregions.o: ../global/tree_compound.h ../global/Error.h
spmdregions.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
spmdregions.o: /usr/include/sys/va_list.h /usr/include/string.h
spmdregions.o: /usr/include/stdlib.h ../global/globals.h
spmdregions.o: ../global/internal_lib.h ../global/free.h ../../inc/dbug.h
spmdregions.o: ../global/my_debug.h /usr/include/malloc.h
spmdregions.o: ../global/traverse.h ../print/convert.h ../optimize/DupTree.h
spmdregions.o: ../optimize/DataFlowMask.h ../optimize/optimize.h
spmdregions.o: ../refcount/refcount.h ../typecheck/typecheck.h spmdregions.h
