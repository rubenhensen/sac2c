#
# $Log$
# Revision 1.3  1998/03/24 14:41:54  dkr
# make deps done
#
# Revision 1.2  1998/03/24 13:51:04  cg
# additional include directory specified
#
# Revision 1.1  1998/03/19 16:32:50  cg
# Initial revision
#
#
# 


CC        := gcc
CCFLAGS   := -ansi -Wall -D__EXTENSIONS__

INCS := -I../global -I../../inc
OBJS := $(patsubst %.c,%.o,$(wildcard libsac_*.c))

.PHONY: all


all:  libsac.a

libsac.a: $(OBJS)
	ar rc libsac.a $(OBJS)
	ranlib libsac.a

%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<


deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o libsac.a *.bak *.old *~
	$(RM) -r .sb

# DO NOT DELETE THIS LINE -- make depend depends on it.

libsac_malloc.o: /usr/include/malloc.h /usr/include/stdio.h
libsac_malloc.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
libsac_malloc.o: sac_message.h
libsac_message.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
libsac_message.o: /usr/include/sys/va_list.h /usr/include/stdarg.h
libsac_message.o: /usr/include/varargs.h /usr/include/stdlib.h
libsac_profile.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
libsac_profile.o: /usr/include/sys/va_list.h /usr/include/sys/time.h
libsac_profile.o: /usr/include/sys/types.h /usr/include/sys/isa_defs.h
libsac_profile.o: /usr/include/sys/machtypes.h /usr/include/sys/resource.h
libsac_trace.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
libsac_trace.o: /usr/include/sys/va_list.h /usr/include/stdarg.h
libsac_trace.o: /usr/include/varargs.h /usr/include/string.h
