#
# 
# $Log$
# Revision 1.18  2000/07/11 15:56:39  dkr
# make deps done
#
# Revision 1.17  2000/06/13 13:46:34  dkr
# deps done
#
# Revision 1.16  2000/05/29 14:36:48  dkr
# make deps done
#
# Revision 1.15  2000/03/31 09:33:49  jhs
# make deps done
#
# Revision 1.14  2000/03/31 09:22:41  cg
# make deps done
#
# Revision 1.13  2000/03/30 15:09:32  jhs
# added adjust_calls.o
#
# Revision 1.12  2000/03/29 16:08:30  jhs
# Added blocks_lift.o
#
# Revision 1.11  2000/03/22 17:30:16  jhs
# Added barriers_init.o
#
# Revision 1.10  2000/03/16 14:32:52  dkr
# make deps done
#
# Revision 1.9  2000/03/09 18:32:39  jhs
# Added dfa and blkpp.
#
# Revision 1.8  2000/03/02 12:55:48  jhs
# Added mtfuns_init.o and blkco_cons.o.
#
# Revision 1.7  2000/03/01 14:52:44  dkr
# make deps done
#
# Revision 1.6  2000/02/21 17:51:16  jhs
# Added multithread_lib.o, blokcs_expand.o
#
# Revision 1.5  2000/02/04 14:43:14  jhs
# Added repfuns_init.[ch]
#
# Revision 1.4  2000/01/28 13:49:43  jhs
# blocks_init added
#
# Revision 1.3  2000/01/25 13:52:39  dkr
# make deps done
#
# Revision 1.2  2000/01/21 14:46:09  dkr
# directory tree added
#
# Revision 1.1  2000/01/21 13:11:16  jhs
# Initial revision
#
#
#

CC        := gcc
CCFLAGS   := -ansi -Wall -g -O3
CFLAGS    := -D__EXTENSIONS__

INCS=-I../global -I../tree -I../../inc -I../optimize -I../typecheck -I../refcount \
     -I../print -I../compile

all: multithread.o \
     multithread_lib.o \
     schedule_init.o \
     repfuns_init.o \
     blocks_init.o \
     blocks_propagate.o \
     blocks_expand.o \
     mtfuns_init.o \
     blocks_cons.o \
     dataflow_analysis.o \
     barriers_init.o \
     blocks_lift.o \
     adjust_calls.o


%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak *~
	$(RM) -r .sb

# DO NOT DELETE THIS LINE -- make depend depends on it.

adjust_calls.o: ../../inc/dbug.h /usr/include/stdio.h
adjust_calls.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
adjust_calls.o: ../global/types.h ../tree/tree_basic.h
adjust_calls.o: ../tree/tree_compound.h ../global/Error.h
adjust_calls.o: /usr/include/string.h /usr/include/stdlib.h
adjust_calls.o: ../global/globals.h ../tree/traverse.h ../tree/scheduling.h
adjust_calls.o: ../tree/DupTree.h ../tree/LookUpTable.h
adjust_calls.o: ../optimize/generatemasks.h ../tree/free.h
adjust_calls.o: ../global/my_debug.h /usr/include/malloc.h
adjust_calls.o: ../global/internal_lib.h multithread_lib.h
barriers_init.o: ../../inc/dbug.h /usr/include/stdio.h
barriers_init.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
barriers_init.o: ../global/types.h ../tree/tree_basic.h
barriers_init.o: ../tree/tree_compound.h ../global/Error.h
barriers_init.o: /usr/include/string.h /usr/include/stdlib.h
barriers_init.o: ../global/globals.h ../tree/traverse.h ../tree/scheduling.h
barriers_init.o: ../tree/DupTree.h ../tree/LookUpTable.h
barriers_init.o: ../optimize/generatemasks.h ../tree/free.h
barriers_init.o: ../global/my_debug.h /usr/include/malloc.h
barriers_init.o: ../global/internal_lib.h ../tree/DataFlowMask.h
barriers_init.o: multithread_lib.h
blocks_cons.o: ../../inc/dbug.h /usr/include/stdio.h
blocks_cons.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
blocks_cons.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
blocks_cons.o: ../global/Error.h /usr/include/string.h /usr/include/stdlib.h
blocks_cons.o: ../global/globals.h ../tree/traverse.h ../tree/scheduling.h
blocks_cons.o: ../tree/DupTree.h ../tree/LookUpTable.h
blocks_cons.o: ../optimize/generatemasks.h ../tree/free.h
blocks_cons.o: ../global/my_debug.h /usr/include/malloc.h
blocks_cons.o: ../global/internal_lib.h multithread_lib.h
blocks_expand.o: ../../inc/dbug.h /usr/include/stdio.h
blocks_expand.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
blocks_expand.o: ../global/types.h ../tree/tree_basic.h
blocks_expand.o: ../tree/tree_compound.h ../global/Error.h
blocks_expand.o: /usr/include/string.h /usr/include/stdlib.h
blocks_expand.o: ../global/globals.h ../tree/traverse.h ../tree/scheduling.h
blocks_expand.o: ../tree/DupTree.h ../tree/LookUpTable.h
blocks_expand.o: ../optimize/generatemasks.h ../global/my_debug.h
blocks_expand.o: multithread_lib.h ../tree/free.h /usr/include/malloc.h
blocks_expand.o: ../global/internal_lib.h
blocks_init.o: ../../inc/dbug.h /usr/include/stdio.h
blocks_init.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
blocks_init.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
blocks_init.o: ../global/Error.h /usr/include/string.h /usr/include/stdlib.h
blocks_init.o: ../global/globals.h ../tree/traverse.h ../tree/scheduling.h
blocks_init.o: ../tree/DupTree.h ../tree/LookUpTable.h
blocks_init.o: ../optimize/generatemasks.h ../global/my_debug.h
blocks_init.o: ../global/internal_lib.h
blocks_lift.o: ../../inc/dbug.h /usr/include/stdio.h
blocks_lift.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
blocks_lift.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
blocks_lift.o: ../global/Error.h /usr/include/string.h /usr/include/stdlib.h
blocks_lift.o: ../global/globals.h ../tree/traverse.h ../tree/scheduling.h
blocks_lift.o: ../tree/DupTree.h ../tree/LookUpTable.h
blocks_lift.o: ../optimize/generatemasks.h ../global/my_debug.h
blocks_lift.o: ../tree/DataFlowMask.h multithread_lib.h
blocks_lift.o: ../global/internal_lib.h
blocks_propagate.o: ../../inc/dbug.h /usr/include/stdio.h
blocks_propagate.o: /usr/include/sys/feature_tests.h
blocks_propagate.o: /usr/include/sys/va_list.h ../global/types.h
blocks_propagate.o: ../tree/tree_basic.h ../tree/tree_compound.h
blocks_propagate.o: ../global/Error.h /usr/include/string.h
blocks_propagate.o: /usr/include/stdlib.h ../global/globals.h
blocks_propagate.o: ../tree/traverse.h ../tree/scheduling.h ../tree/DupTree.h
blocks_propagate.o: ../tree/LookUpTable.h ../optimize/generatemasks.h
blocks_propagate.o: ../tree/free.h ../global/my_debug.h /usr/include/malloc.h
blocks_propagate.o: ../global/internal_lib.h blocks_init.h multithread_lib.h
dataflow_analysis.o: ../../inc/dbug.h /usr/include/stdio.h
dataflow_analysis.o: /usr/include/sys/feature_tests.h
dataflow_analysis.o: /usr/include/sys/va_list.h ../global/types.h
dataflow_analysis.o: ../tree/tree_basic.h ../tree/tree_compound.h
dataflow_analysis.o: ../global/Error.h /usr/include/string.h
dataflow_analysis.o: /usr/include/stdlib.h ../global/globals.h
dataflow_analysis.o: ../tree/traverse.h ../tree/scheduling.h
dataflow_analysis.o: ../tree/DupTree.h ../tree/LookUpTable.h
dataflow_analysis.o: ../optimize/generatemasks.h ../tree/free.h
dataflow_analysis.o: ../global/my_debug.h /usr/include/malloc.h
dataflow_analysis.o: ../global/internal_lib.h ../tree/DataFlowMask.h
dataflow_analysis.o: ../refcount/refcount.h multithread_lib.h mtfuns_init.h
mtfuns_init.o: ../../inc/dbug.h /usr/include/stdio.h
mtfuns_init.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
mtfuns_init.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
mtfuns_init.o: ../global/Error.h /usr/include/string.h /usr/include/stdlib.h
mtfuns_init.o: ../global/globals.h ../tree/traverse.h ../tree/scheduling.h
mtfuns_init.o: ../tree/DupTree.h ../tree/LookUpTable.h
mtfuns_init.o: ../optimize/generatemasks.h ../tree/free.h
mtfuns_init.o: ../global/my_debug.h /usr/include/malloc.h
mtfuns_init.o: ../global/internal_lib.h multithread_lib.h mtfuns_init.h
multithread.o: ../../inc/dbug.h /usr/include/stdio.h
multithread.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
multithread.o: ../global/types.h ../tree/tree_basic.h ../tree/traverse.h
multithread.o: ../global/globals.h ../tree/free.h ../global/my_debug.h
multithread.o: /usr/include/malloc.h ../global/internal_lib.h
multithread.o: ../global/Error.h /usr/include/string.h /usr/include/stdlib.h
multithread.o: schedule_init.h repfuns_init.h blocks_init.h
multithread.o: blocks_propagate.h blocks_expand.h mtfuns_init.h blocks_cons.h
multithread.o: dataflow_analysis.h barriers_init.h blocks_lift.h
multithread.o: adjust_calls.h
multithread_lib.o: ../../inc/dbug.h /usr/include/stdio.h
multithread_lib.o: /usr/include/sys/feature_tests.h
multithread_lib.o: /usr/include/sys/va_list.h /usr/include/string.h
multithread_lib.o: ../tree/tree_basic.h ../global/types.h ../tree/free.h
multithread_lib.o: ../global/my_debug.h /usr/include/malloc.h
multithread_lib.o: ../global/globals.h ../global/internal_lib.h
multithread_lib.o: ../tree/DupTree.h ../tree/LookUpTable.h
multithread_lib.o: ../tree/DataFlowMask.h
repfuns_init.o: ../../inc/dbug.h /usr/include/stdio.h
repfuns_init.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
repfuns_init.o: ../global/types.h ../tree/tree_basic.h
repfuns_init.o: ../tree/tree_compound.h ../global/Error.h
repfuns_init.o: /usr/include/string.h /usr/include/stdlib.h
repfuns_init.o: ../global/globals.h ../tree/traverse.h ../tree/scheduling.h
repfuns_init.o: ../tree/DupTree.h ../tree/LookUpTable.h
repfuns_init.o: ../optimize/generatemasks.h ../tree/free.h
repfuns_init.o: ../global/my_debug.h /usr/include/malloc.h
repfuns_init.o: ../global/internal_lib.h multithread_lib.h
schedule_init.o: ../../inc/dbug.h /usr/include/stdio.h
schedule_init.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
schedule_init.o: ../global/types.h ../tree/tree_basic.h
schedule_init.o: ../tree/tree_compound.h ../global/Error.h
schedule_init.o: /usr/include/string.h /usr/include/stdlib.h
schedule_init.o: ../global/globals.h ../tree/traverse.h ../tree/scheduling.h
schedule_init.o: ../global/internal_lib.h
