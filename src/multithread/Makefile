#
# 
# $Log$
# Revision 3.10  2001/11/13 20:32:50  dkr
# make deps done
#
# Revision 3.9  2001/07/18 13:08:21  cg
# make deps done
#
# Revision 3.8  2001/05/08 13:29:44  dkr
# make deps done
#
# Revision 3.7  2001/01/17 10:13:11  dkr
# make deps done
#
# Revision 3.6  2000/12/15 18:18:08  dkr
# make deps done
#
# Revision 3.5  2000/12/12 15:09:27  dkr
# make deps done
#
# Revision 3.4  2000/12/08 10:05:27  dkr
# include path extended
#
# Revision 3.3  2000/12/07 14:19:55  dkr
# include path modified
#
# Revision 3.2  2000/12/06 20:09:11  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:02:58  sacbase
# new release made
#
# Revision 1.21  2000/11/02 14:39:12  dkr
# header compressed
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
# Revision 1.9  2000/03/09 18:32:39  jhs
# Added dfa and blkpp.
#
# Revision 1.8  2000/03/02 12:55:48  jhs
# Added mtfuns_init.o and blkco_cons.o.
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
# Revision 1.2  2000/01/21 14:46:09  dkr
# directory tree added
#
# Revision 1.1  2000/01/21 13:11:16  jhs
# Initial revision
#
#

CC        := gcc
CCFLAGS   := -ansi -Wall -g -O3
CFLAGS    := -D__EXTENSIONS__

INCS=-I../global -I../tree -I../../inc -I../optimize -I../typecheck -I../flatten \
     -I../print -I../compile -I../refcount

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
adjust_calls.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
adjust_calls.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
adjust_calls.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
adjust_calls.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
adjust_calls.o: ../global/types.h ../tree/tree_basic.h
adjust_calls.o: ../tree/tree_compound.h ../global/Error.h
adjust_calls.o: /usr/include/string.h /usr/include/iso/string_iso.h
adjust_calls.o: ../global/globals.h ../tree/traverse.h ../tree/scheduling.h
adjust_calls.o: ../tree/DupTree.h ../tree/LookUpTable.h
adjust_calls.o: ../optimize/generatemasks.h ../tree/free.h
adjust_calls.o: ../global/my_debug.h ../global/internal_lib.h
adjust_calls.o: multithread_lib.h ../refcount/refcount.h
barriers_init.o: ../../inc/dbug.h /usr/include/stdio.h
barriers_init.o: /usr/include/iso/stdio_iso.h
barriers_init.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
barriers_init.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
barriers_init.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
barriers_init.o: /usr/include/iso/stdlib_iso.h ../global/types.h
barriers_init.o: ../tree/tree_basic.h ../tree/tree_compound.h
barriers_init.o: ../global/Error.h /usr/include/string.h
barriers_init.o: /usr/include/iso/string_iso.h ../global/globals.h
barriers_init.o: ../tree/traverse.h ../tree/scheduling.h ../tree/DupTree.h
barriers_init.o: ../tree/LookUpTable.h ../optimize/generatemasks.h
barriers_init.o: ../tree/free.h ../global/my_debug.h ../global/internal_lib.h
barriers_init.o: ../tree/DataFlowMask.h multithread_lib.h
barriers_init.o: ../refcount/refcount.h
blocks_cons.o: ../../inc/dbug.h /usr/include/stdio.h
blocks_cons.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
blocks_cons.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
blocks_cons.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
blocks_cons.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
blocks_cons.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
blocks_cons.o: ../global/Error.h /usr/include/string.h
blocks_cons.o: /usr/include/iso/string_iso.h ../global/globals.h
blocks_cons.o: ../tree/traverse.h ../tree/scheduling.h ../tree/DupTree.h
blocks_cons.o: ../tree/LookUpTable.h ../optimize/generatemasks.h
blocks_cons.o: ../tree/free.h ../global/my_debug.h ../global/internal_lib.h
blocks_cons.o: multithread_lib.h ../refcount/refcount.h
blocks_expand.o: ../../inc/dbug.h /usr/include/stdio.h
blocks_expand.o: /usr/include/iso/stdio_iso.h
blocks_expand.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
blocks_expand.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
blocks_expand.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
blocks_expand.o: /usr/include/iso/stdlib_iso.h ../global/types.h
blocks_expand.o: ../tree/tree_basic.h ../tree/tree_compound.h
blocks_expand.o: ../global/Error.h /usr/include/string.h
blocks_expand.o: /usr/include/iso/string_iso.h ../global/globals.h
blocks_expand.o: ../tree/traverse.h ../tree/scheduling.h ../tree/DupTree.h
blocks_expand.o: ../tree/LookUpTable.h ../optimize/generatemasks.h
blocks_expand.o: ../global/my_debug.h multithread_lib.h
blocks_expand.o: ../refcount/refcount.h ../tree/free.h
blocks_expand.o: ../global/internal_lib.h
blocks_init.o: ../../inc/dbug.h /usr/include/stdio.h
blocks_init.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
blocks_init.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
blocks_init.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
blocks_init.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
blocks_init.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
blocks_init.o: ../global/Error.h /usr/include/string.h
blocks_init.o: /usr/include/iso/string_iso.h ../global/globals.h
blocks_init.o: ../tree/traverse.h ../tree/scheduling.h ../tree/DupTree.h
blocks_init.o: ../tree/LookUpTable.h ../optimize/generatemasks.h
blocks_init.o: ../global/my_debug.h multithread_lib.h ../refcount/refcount.h
blocks_init.o: ../global/internal_lib.h
blocks_lift.o: ../../inc/dbug.h /usr/include/stdio.h
blocks_lift.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
blocks_lift.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
blocks_lift.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
blocks_lift.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
blocks_lift.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
blocks_lift.o: ../global/Error.h /usr/include/string.h
blocks_lift.o: /usr/include/iso/string_iso.h ../global/globals.h
blocks_lift.o: ../tree/traverse.h ../tree/scheduling.h ../tree/DupTree.h
blocks_lift.o: ../tree/LookUpTable.h ../optimize/generatemasks.h
blocks_lift.o: ../global/my_debug.h ../tree/DataFlowMask.h
blocks_lift.o: ../tree/DataFlowMaskUtils.h multithread_lib.h
blocks_lift.o: ../refcount/refcount.h ../global/internal_lib.h
blocks_propagate.o: ../../inc/dbug.h /usr/include/stdio.h
blocks_propagate.o: /usr/include/iso/stdio_iso.h
blocks_propagate.o: /usr/include/sys/feature_tests.h
blocks_propagate.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
blocks_propagate.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
blocks_propagate.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
blocks_propagate.o: ../global/types.h ../tree/tree_basic.h
blocks_propagate.o: ../tree/tree_compound.h ../global/Error.h
blocks_propagate.o: /usr/include/string.h /usr/include/iso/string_iso.h
blocks_propagate.o: ../global/globals.h ../tree/traverse.h
blocks_propagate.o: ../tree/scheduling.h ../tree/DupTree.h
blocks_propagate.o: ../tree/LookUpTable.h ../optimize/generatemasks.h
blocks_propagate.o: ../tree/free.h ../global/my_debug.h
blocks_propagate.o: ../global/internal_lib.h blocks_init.h multithread_lib.h
blocks_propagate.o: ../refcount/refcount.h
dataflow_analysis.o: ../../inc/dbug.h /usr/include/stdio.h
dataflow_analysis.o: /usr/include/iso/stdio_iso.h
dataflow_analysis.o: /usr/include/sys/feature_tests.h
dataflow_analysis.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
dataflow_analysis.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
dataflow_analysis.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
dataflow_analysis.o: ../global/types.h ../tree/tree_basic.h
dataflow_analysis.o: ../tree/tree_compound.h ../global/Error.h
dataflow_analysis.o: /usr/include/string.h /usr/include/iso/string_iso.h
dataflow_analysis.o: ../global/globals.h ../tree/traverse.h
dataflow_analysis.o: ../tree/scheduling.h ../tree/DupTree.h
dataflow_analysis.o: ../tree/LookUpTable.h ../optimize/generatemasks.h
dataflow_analysis.o: ../tree/free.h ../global/my_debug.h
dataflow_analysis.o: ../global/internal_lib.h ../tree/DataFlowMask.h
dataflow_analysis.o: ../tree/InferDFMs.h multithread_lib.h
dataflow_analysis.o: ../refcount/refcount.h mtfuns_init.h
mtfuns_init.o: ../../inc/dbug.h /usr/include/stdio.h
mtfuns_init.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
mtfuns_init.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
mtfuns_init.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
mtfuns_init.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
mtfuns_init.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
mtfuns_init.o: ../global/Error.h /usr/include/string.h
mtfuns_init.o: /usr/include/iso/string_iso.h ../global/globals.h
mtfuns_init.o: ../tree/traverse.h ../tree/scheduling.h ../tree/DupTree.h
mtfuns_init.o: ../tree/LookUpTable.h ../optimize/generatemasks.h
mtfuns_init.o: ../tree/free.h ../global/my_debug.h ../global/internal_lib.h
mtfuns_init.o: multithread_lib.h ../refcount/refcount.h mtfuns_init.h
multithread.o: ../../inc/dbug.h /usr/include/stdio.h
multithread.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
multithread.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
multithread.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
multithread.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
multithread.o: ../global/types.h ../tree/tree_basic.h ../tree/traverse.h
multithread.o: ../global/globals.h ../tree/free.h ../global/my_debug.h
multithread.o: ../global/internal_lib.h ../global/Error.h
multithread.o: /usr/include/string.h /usr/include/iso/string_iso.h
multithread.o: schedule_init.h repfuns_init.h blocks_init.h
multithread.o: blocks_propagate.h blocks_expand.h mtfuns_init.h blocks_cons.h
multithread.o: dataflow_analysis.h barriers_init.h blocks_lift.h
multithread.o: adjust_calls.h
multithread_lib.o: ../../inc/dbug.h /usr/include/stdio.h
multithread_lib.o: /usr/include/iso/stdio_iso.h
multithread_lib.o: /usr/include/sys/feature_tests.h
multithread_lib.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
multithread_lib.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
multithread_lib.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
multithread_lib.o: /usr/include/string.h /usr/include/iso/string_iso.h
multithread_lib.o: ../tree/tree_basic.h ../global/types.h
multithread_lib.o: ../tree/tree_compound.h ../global/Error.h
multithread_lib.o: ../global/globals.h ../tree/free.h ../global/my_debug.h
multithread_lib.o: ../global/internal_lib.h ../tree/DupTree.h
multithread_lib.o: ../tree/LookUpTable.h ../tree/DataFlowMask.h
multithread_lib.o: ../optimize/generatemasks.h
repfuns_init.o: ../../inc/dbug.h /usr/include/stdio.h
repfuns_init.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
repfuns_init.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
repfuns_init.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
repfuns_init.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
repfuns_init.o: ../global/types.h ../tree/tree_basic.h
repfuns_init.o: ../tree/tree_compound.h ../global/Error.h
repfuns_init.o: /usr/include/string.h /usr/include/iso/string_iso.h
repfuns_init.o: ../global/globals.h ../tree/traverse.h ../tree/scheduling.h
repfuns_init.o: ../tree/DupTree.h ../tree/LookUpTable.h
repfuns_init.o: ../optimize/generatemasks.h ../tree/free.h
repfuns_init.o: ../global/my_debug.h ../global/internal_lib.h
repfuns_init.o: multithread_lib.h ../refcount/refcount.h
schedule_init.o: ../../inc/dbug.h /usr/include/stdio.h
schedule_init.o: /usr/include/iso/stdio_iso.h
schedule_init.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
schedule_init.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
schedule_init.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
schedule_init.o: /usr/include/iso/stdlib_iso.h ../global/types.h
schedule_init.o: ../tree/tree_basic.h ../tree/tree_compound.h
schedule_init.o: ../global/Error.h /usr/include/string.h
schedule_init.o: /usr/include/iso/string_iso.h ../global/globals.h
schedule_init.o: ../tree/traverse.h ../tree/scheduling.h
schedule_init.o: ../global/internal_lib.h
