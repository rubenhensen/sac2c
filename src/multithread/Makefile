#
# 
# $Log$
# Revision 1.1  2000/01/21 13:11:16  jhs
# Initial revision
#
#
#

CC        := gcc
CCFLAGS   := -ansi -Wall -g -O3
CFLAGS    := -D__EXTENSIONS__

INCS=-I../global -I../../inc -I../optimize -I../typecheck -I../refcount -I../print  \
     -I../compile

all: multithread.o

%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak *~
	$(RM) -r .sb

# DO NOT DELETE THIS LINE -- make depend depends on it.

multithread.o: ../../inc/dbug.h /usr/include/stdio.h
multithread.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
multithread.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
multithread.o: /usr/include/stdio_impl.h ../global/types.h
multithread.o: ../global/tree_basic.h ../global/traverse.h
multithread.o: ../global/globals.h ../global/free.h ../global/my_debug.h
multithread.o: /usr/include/malloc.h /usr/include/sys/types.h
multithread.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
multithread.o: /usr/include/sys/select.h /usr/include/sys/time.h
multithread.o: /usr/include/sys/time.h ../global/internal_lib.h
multithread.o: ../global/Error.h /usr/include/string.h /usr/include/stdlib.h
