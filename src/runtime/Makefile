#
# $Log$
# Revision 2.5  1999/04/09 09:40:53  her
# added libsac_message.o to the CacheSimTest target
#
# Revision 2.4  1999/03/19 11:09:04  her
# new OBJS (libsac_cachesim_access_detailed.o) added
# new targets (CacheSimTest.o, CacheSimTest) added
# make deps done
#
# Revision 2.3  1999/03/11 18:18:27  dkr
# make deps done
#
# Revision 2.2  1999/02/26 14:17:45  dkr
# make deps done
#
# Revision 2.1  1999/02/23 12:43:33  sacbase
# new release made
#
# Revision 1.15  1999/02/09 20:04:26  dkr
# make deps done
#
# Revision 1.14  1999/01/14 13:16:19  sacbase
# clean: rm sac2crc    added
#
# Revision 1.13  1998/12/21 15:46:21  cg
# File sac2crc no longer added to runtime.tar because system dependent
# variants do exist now. File sac2c.README added instead.
#
# Revision 1.12  1998/12/17 15:49:43  cg
# added rm runtime.tar for phony target clean
#
# Revision 1.11  1998/12/17 09:50:28  cg
# File libsac.a will no longer be added to runtime.tar because
# it depends on the target architecture.
#
# Revision 1.10  1998/11/04 18:22:41  dkr
# make deps done
#
# Revision 1.9  1998/10/28 20:35:18  sbs
# Now, sac2crc will be part of runtime.tar as well 8-))
#
# Revision 1.8  1998/10/28 19:32:10  sbs
# runtime.tar rule inserted
#
# Revision 1.7  1998/06/29 09:16:42  cg
# The SAC runtime library is now compiled with optimizations and
# debug options.
#
# Revision 1.6  1998/05/12 14:22:27  dkr
# make deps done
#
# Revision 1.5  1998/03/24 15:39:00  dkr
# make deps done
#
# Revision 1.4  1998/03/24 15:19:05  cg
# make deps done
#
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
CCFLAGS   := -ansi -Wall -D__EXTENSIONS__ -g -O3

INCS := -I../global -I../../inc
OBJS := $(patsubst %.c,%.o,$(wildcard libsac_*.c)) libsac_trace_mt.o\
        libsac_cachesim_access_detailed.o


.PHONY: all tar


all:  libsac.a

tar: runtime.tar

libsac.a: $(OBJS)
	ar rc libsac.a $(OBJS)
	ranlib libsac.a

libsac_trace_mt.o: libsac_mt.c 
	$(CC) -DTRACE $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<

libsac_cachesim_access_detailed.o: libsac_cachesim_access.c 
	$(CC) -D TYPE=D -D DETAILED $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<

%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<

runtime.tar: *.h sac2crc.README
	tar cvf $@ $^

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o libsac.a *.bak *.old *~
	$(RM) -r .sb
	$(RM) runtime.tar
	$(RM) sac2crc CacheSimTest

CacheSimTest.o: CacheSimTest.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<

CacheSimTest: CacheSimTest.o libsac_cachesim_basic.o libsac_cachesim_access.o\
              libsac_cachesim_access_detailed.o libsac_message.o
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ $^


# DO NOT DELETE THIS LINE -- make depend depends on it.

CacheSimTest.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
CacheSimTest.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
CacheSimTest.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
CacheSimTest.o: sac_cachesim.h
libsac_cachesim_access.o: /usr/include/memory.h /usr/include/sys/types.h
libsac_cachesim_access.o: /usr/include/sys/isa_defs.h
libsac_cachesim_access.o: /usr/include/sys/feature_tests.h
libsac_cachesim_access.o: /usr/include/sys/machtypes.h
libsac_cachesim_access.o: /usr/include/sys/int_types.h
libsac_cachesim_access.o: /usr/include/sys/select.h /usr/include/sys/time.h
libsac_cachesim_access.o: /usr/include/sys/time.h /usr/include/stdlib.h
libsac_cachesim_access.o: /usr/include/stdio.h /usr/include/sys/va_list.h
libsac_cachesim_access.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
libsac_cachesim_access.o: sac_cachesim.h
libsac_cachesim_basic.o: /usr/include/memory.h /usr/include/sys/types.h
libsac_cachesim_basic.o: /usr/include/sys/isa_defs.h
libsac_cachesim_basic.o: /usr/include/sys/feature_tests.h
libsac_cachesim_basic.o: /usr/include/sys/machtypes.h
libsac_cachesim_basic.o: /usr/include/sys/int_types.h
libsac_cachesim_basic.o: /usr/include/sys/select.h /usr/include/sys/time.h
libsac_cachesim_basic.o: /usr/include/sys/time.h /usr/include/stdlib.h
libsac_cachesim_basic.o: /usr/include/stdio.h /usr/include/sys/va_list.h
libsac_cachesim_basic.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
libsac_cachesim_basic.o: sac_cachesim.h
libsac_malloc.o: /usr/include/malloc.h /usr/include/sys/types.h
libsac_malloc.o: /usr/include/sys/isa_defs.h /usr/include/sys/feature_tests.h
libsac_malloc.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
libsac_malloc.o: /usr/include/sys/select.h /usr/include/sys/time.h
libsac_malloc.o: /usr/include/sys/time.h /usr/include/stdio.h
libsac_malloc.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
libsac_malloc.o: /usr/include/stdio_impl.h sac_message.h
libsac_message.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
libsac_message.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
libsac_message.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
libsac_message.o: /usr/include/stdarg.h /usr/include/stdlib.h
libsac_mt.o: /usr/include/pthread.h /usr/include/sys/types.h
libsac_mt.o: /usr/include/sys/isa_defs.h /usr/include/sys/feature_tests.h
libsac_mt.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
libsac_mt.o: /usr/include/sys/select.h /usr/include/sys/time.h
libsac_mt.o: /usr/include/sys/time.h /usr/include/sched.h
libsac_mt.o: /usr/include/stdio.h /usr/include/sys/va_list.h
libsac_mt.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h sac_message.h
libsac_profile.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
libsac_profile.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
libsac_profile.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
libsac_profile.o: /usr/include/sys/time.h /usr/include/sys/types.h
libsac_profile.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
libsac_profile.o: /usr/include/sys/select.h /usr/include/sys/time.h
libsac_profile.o: /usr/include/sys/resource.h
libsac_trace.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
libsac_trace.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
libsac_trace.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
libsac_trace.o: /usr/include/stdarg.h /usr/include/string.h
