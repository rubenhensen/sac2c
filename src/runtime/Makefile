#
# $Log$
# Revision 2.11  1999/06/02 16:55:39  dkr
# path to main_args.o is now correct
#
# Revision 2.10  1999/06/01 15:17:09  dkr
# make deps done
#
# Revision 2.8  1999/05/20 14:13:44  cg
# Compilation of CacheSimAnalyser streamlined:
# it is now linked with the SAC runtime library.
#
# Revision 2.6  1999/04/26 11:42:05  her
# modifications for the piped-cachesimulation
#
# Revision 2.5  1999/04/09 09:40:53  her
# added libsac_message.o to the CacheSimTest target
#
# Revision 2.4  1999/03/19 11:09:04  her
# new OBJS (libsac_cachesim_access_detailed.o) added
# new targets (CacheSimTest.o, CacheSimTest) added
# make deps done
#
# Revision 2.1  1999/02/23 12:43:33  sacbase
# new release made
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
LIBS := -L.

OBJS := $(patsubst %.c,%.o,$(wildcard libsac_*.c)) libsac_trace_mt.o\
        libsac_cachesim_access_advanced.o ../../lib/main_args.o


.PHONY: all tar


all:  libsac.a CacheSimAnalyser

tar: runtime.tar

libsac.a: $(OBJS)
	ar rc libsac.a $(OBJS)
	ranlib libsac.a

libsac_trace_mt.o: libsac_mt.c 
	$(CC) -DTRACE $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<

libsac_cachesim_access_advanced.o: libsac_cachesim_access.c 
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
	$(RM) sac2crc CacheSimTest CacheSimAnalyser

CacheSimAnalyser: CacheSimAnalyser.o libsac.a
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) $(LIBS) -o $@ $< -lsac -lm

CacheSimTest: CacheSimTest.o libsac.a
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) $(LIBS) -o $@ $< -lsac -lm


# DO NOT DELETE THIS LINE -- make depend depends on it.

CacheSimAnalyser.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
CacheSimAnalyser.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
CacheSimAnalyser.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
CacheSimAnalyser.o: sac_cachesim.h libsac_cachesim.h sac_message.h
CacheSimTest.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
CacheSimTest.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
CacheSimTest.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
CacheSimTest.o: sac_cachesim.h
libsac_cachesim_access.o: /usr/include/memory.h /usr/include/sys/types.h
libsac_cachesim_access.o: /usr/include/sys/isa_defs.h
libsac_cachesim_access.o: /usr/include/sys/feature_tests.h
libsac_cachesim_access.o: /usr/include/sys/machtypes.h
libsac_cachesim_access.o: /usr/include/sys/int_types.h /usr/include/stdlib.h
libsac_cachesim_access.o: /usr/include/stdio.h /usr/include/sys/va_list.h
libsac_cachesim_access.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
libsac_cachesim_access.o: sac_cachesim.h libsac_cachesim.h
libsac_cachesim_basic.o: /usr/include/memory.h /usr/include/sys/types.h
libsac_cachesim_basic.o: /usr/include/sys/isa_defs.h
libsac_cachesim_basic.o: /usr/include/sys/feature_tests.h
libsac_cachesim_basic.o: /usr/include/sys/machtypes.h
libsac_cachesim_basic.o: /usr/include/sys/int_types.h /usr/include/stdlib.h
libsac_cachesim_basic.o: /usr/include/stdio.h /usr/include/sys/va_list.h
libsac_cachesim_basic.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
libsac_cachesim_basic.o: /usr/include/string.h /usr/include/math.h
libsac_cachesim_basic.o: sac_cachesim.h libsac_cachesim.h sac_message.h
libsac_cachesim_basic.o: ../../inc/main_args.h
libsac_malloc.o: /usr/include/malloc.h /usr/include/sys/types.h
libsac_malloc.o: /usr/include/sys/isa_defs.h /usr/include/sys/feature_tests.h
libsac_malloc.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
libsac_malloc.o: /usr/include/stdio.h /usr/include/sys/va_list.h
libsac_malloc.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
libsac_malloc.o: sac_message.h
libsac_message.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
libsac_message.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
libsac_message.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
libsac_message.o: /usr/include/stdarg.h /usr/include/varargs.h
libsac_message.o: /usr/include/stdlib.h
libsac_mt.o: /usr/include/pthread.h /usr/include/sys/types.h
libsac_mt.o: /usr/include/sys/isa_defs.h /usr/include/sys/feature_tests.h
libsac_mt.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
libsac_mt.o: /usr/include/time.h /usr/include/sched.h /usr/include/stdio.h
libsac_mt.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
libsac_mt.o: /usr/include/stdio_impl.h sac_message.h
libsac_profile.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
libsac_profile.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
libsac_profile.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
libsac_profile.o: /usr/include/sys/time.h /usr/include/sys/types.h
libsac_profile.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
libsac_profile.o: /usr/include/sys/resource.h
libsac_trace.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
libsac_trace.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
libsac_trace.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
libsac_trace.o: /usr/include/stdarg.h /usr/include/varargs.h
libsac_trace.o: /usr/include/string.h
