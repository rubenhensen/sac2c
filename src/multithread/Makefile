#
# 
# $Log$
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
     blocks_expand.o

%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak *~
	$(RM) -r .sb

# DO NOT DELETE THIS LINE -- make depend depends on it.

blocks_expand.o: ../../inc/dbug.h /usr/include/stdio.h
blocks_expand.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
blocks_expand.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
blocks_expand.o: /usr/include/stdio_impl.h ../global/types.h
blocks_expand.o: ../tree/tree_basic.h ../tree/tree_compound.h
blocks_expand.o: ../global/Error.h /usr/include/string.h
blocks_expand.o: /usr/include/stdlib.h ../global/globals.h ../tree/traverse.h
blocks_expand.o: ../tree/scheduling.h ../tree/DupTree.h ../tree/LookUpTable.h
blocks_expand.o: ../optimize/generatemasks.h ../global/my_debug.h
blocks_expand.o: ../global/internal_lib.h
blocks_init.o: ../../inc/dbug.h /usr/include/stdio.h
blocks_init.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
blocks_init.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
blocks_init.o: /usr/include/stdio_impl.h ../global/types.h
blocks_init.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
blocks_init.o: /usr/include/string.h /usr/include/stdlib.h
blocks_init.o: ../global/globals.h ../tree/traverse.h ../tree/scheduling.h
blocks_init.o: ../tree/DupTree.h ../tree/LookUpTable.h
blocks_init.o: ../optimize/generatemasks.h ../global/my_debug.h
blocks_init.o: ../global/internal_lib.h
multithread.o: ../../inc/dbug.h /usr/include/stdio.h
multithread.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
multithread.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
multithread.o: /usr/include/stdio_impl.h ../global/types.h
multithread.o: ../tree/tree_basic.h ../tree/traverse.h ../global/globals.h
multithread.o: ../tree/free.h ../global/my_debug.h /usr/include/malloc.h
multithread.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
multithread.o: /usr/include/sys/int_types.h /usr/include/sys/select.h
multithread.o: /usr/include/sys/time.h /usr/include/sys/time.h
multithread.o: ../global/internal_lib.h ../global/Error.h
multithread.o: /usr/include/string.h /usr/include/stdlib.h schedule_init.h
multithread.o: repfuns_init.h blocks_init.h blocks_expand.h
repfuns_init.o: ../../inc/dbug.h /usr/include/stdio.h
repfuns_init.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
repfuns_init.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
repfuns_init.o: /usr/include/stdio_impl.h ../global/types.h
repfuns_init.o: ../tree/tree_basic.h ../tree/tree_compound.h
repfuns_init.o: ../global/Error.h /usr/include/string.h /usr/include/stdlib.h
repfuns_init.o: ../global/globals.h ../tree/traverse.h ../tree/scheduling.h
repfuns_init.o: ../tree/DupTree.h ../tree/LookUpTable.h
repfuns_init.o: ../optimize/generatemasks.h ../tree/free.h
repfuns_init.o: ../global/my_debug.h /usr/include/malloc.h
repfuns_init.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
repfuns_init.o: /usr/include/sys/int_types.h /usr/include/sys/select.h
repfuns_init.o: /usr/include/sys/time.h /usr/include/sys/time.h
repfuns_init.o: ../global/internal_lib.h
schedule_init.o: ../../inc/dbug.h /usr/include/stdio.h
schedule_init.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
schedule_init.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
schedule_init.o: /usr/include/stdio_impl.h ../global/types.h
schedule_init.o: ../tree/tree_basic.h ../tree/tree_compound.h
schedule_init.o: ../global/Error.h /usr/include/string.h
schedule_init.o: /usr/include/stdlib.h ../global/globals.h ../tree/traverse.h
schedule_init.o: ../tree/scheduling.h ../global/internal_lib.h
