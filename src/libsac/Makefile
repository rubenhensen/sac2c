#
# $Log$
# Revision 1.6  1999/12/13 11:29:06  dkr
# *** empty log message ***
#
# Revision 1.5  1999/09/20 13:16:23  cg
# Added compilation of new source file heapmgr_diag.c
#
# Revision 1.4  1999/07/23 12:42:47  cg
# Added new target prod to allow local compilations with
# optimizations enabled.
#
# Revision 1.3  1999/07/16 09:41:16  cg
# Added facilities for heap management diagnostics.
#
# Revision 1.2  1999/07/09 11:39:06  cg
# make deps done
#
# Revision 1.1  1999/07/08 12:30:54  cg
# Initial revision
#
# 


CC        := gcc
CCFLAGS   := -ansi -Wall -D__EXTENSIONS__ -g 

MTFLAGS   := -D_REENTRANT -D_POSIX_C_SOURCE=199506L

INCS := -I../runtime -I../../inc
LIBS := -L.

OBJS_LIBSAC := boundcheck.o message.o misc.o profile.o trace.o  \
               cachesim_access_advanced.o cachesim_access_simple.o   \
               cachesim_basic.o ../../lib/main_args.o	

OBJS_LIBSAC_MT := boundcheck_mt.o message_mt.o misc_mt.o trace_mt.o mt.o 

OBJS_LIBSAC_HEAPMGR := heapmgr_basic.o heapmgr_malloc.o heapmgr_memalign.o \
                       heapmgr_realloc.o

OBJS_LIBSAC_HEAPMGR_DIAG := heapmgr_basic_diag.o heapmgr_malloc_diag.o heapmgr_memalign_diag.o \
                            heapmgr_realloc_diag.o heapmgr_diag.o

OBJS_LIBSAC_NOHEAPMGR := heapmgr_disabled.o

OBJS_LIBSAC_NOHEAPMGR_MT := heapmgr_disabled_mt.o



.PHONY: all tar clean prod


all: libsac.a libsac_mt.a libsac_heapmgr.a libsac_heapmgr_diag.a   \
     libsac_noheapmgr.a libsac_noheapmgr_mt.a


libsac.a: $(OBJS_LIBSAC)
	ar rc $@ $^
	ranlib $@

libsac_mt.a: $(OBJS_LIBSAC_MT)
	ar rc $@ $^
	ranlib $@

libsac_heapmgr.a: $(OBJS_LIBSAC_HEAPMGR)
	ar rc $@ $^
	ranlib $@

libsac_heapmgr_diag.a: $(OBJS_LIBSAC_HEAPMGR_DIAG)
	ar rc $@ $^
	ranlib $@

libsac_noheapmgr.a: $(OBJS_LIBSAC_NOHEAPMGR)
	ar rc $@ $^
	ranlib $@

libsac_noheapmgr_mt.a: $(OBJS_LIBSAC_NOHEAPMGR_MT)
	ar rc $@ $^
	ranlib $@


trace_mt.o: mt.c 
	$(CC) -DTRACE $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<

cachesim_access_simple.o: cachesim_access.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<

cachesim_access_advanced.o: cachesim_access.c 
	$(CC) -D TYPE=D -D DETAILED $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<


%_mt.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) $(MTFLAGS) -o $@ -c $<

%_diag.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -DDIAG -o $@ -c $<

%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<


deps:
	makedepend $(INCS) *.c

prod:
	$(MAKE) CCFLAGS="$(CCFLAGS) -O3"


clean:
	$(RM) *.o *.a *.bak *.old *~
	$(RM) -r .sb SunWS_cache


# DO NOT DELETE THIS LINE -- make depend depends on it.

cachesim_access.o: /usr/include/memory.h /usr/include/sys/types.h
cachesim_access.o: /usr/include/sys/isa_defs.h
cachesim_access.o: /usr/include/sys/feature_tests.h
cachesim_access.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
cachesim_access.o: /usr/include/stdlib.h /usr/include/stdio.h
cachesim_access.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
cachesim_access.o: /usr/include/stdio_impl.h ../runtime/sac_cachesim.h
cachesim_access.o: cachesim.h
cachesim_basic.o: /usr/include/memory.h /usr/include/sys/types.h
cachesim_basic.o: /usr/include/sys/isa_defs.h
cachesim_basic.o: /usr/include/sys/feature_tests.h
cachesim_basic.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
cachesim_basic.o: /usr/include/stdlib.h /usr/include/stdio.h
cachesim_basic.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
cachesim_basic.o: /usr/include/stdio_impl.h /usr/include/string.h
cachesim_basic.o: /usr/include/math.h ../runtime/sac_cachesim.h
cachesim_basic.o: ../runtime/sac_message.h cachesim.h ../../inc/main_args.h
heapmgr_basic.o: /usr/include/unistd.h /usr/include/sys/feature_tests.h
heapmgr_basic.o: /usr/include/sys/isa_defs.h /usr/include/sys/types.h
heapmgr_basic.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
heapmgr_basic.o: /usr/include/sys/unistd.h /usr/include/string.h heapmgr.h
heapmgr_basic.o: ../runtime/sac_heapmgr.h ../runtime/sac_message.h
heapmgr_disabled.o: ../runtime/sac_message.h heapmgr.h
heapmgr_disabled.o: ../runtime/sac_heapmgr.h
heapmgr_malloc.o: heapmgr.h ../runtime/sac_heapmgr.h
heapmgr_memalign.o: heapmgr.h ../runtime/sac_heapmgr.h
heapmgr_realloc.o: heapmgr.h ../runtime/sac_heapmgr.h
message.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
message.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
message.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
message.o: /usr/include/stdarg.h /usr/include/varargs.h /usr/include/stdlib.h
mt.o: /usr/include/pthread.h /usr/include/sys/types.h
mt.o: /usr/include/sys/isa_defs.h /usr/include/sys/feature_tests.h
mt.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
mt.o: /usr/include/time.h /usr/include/sched.h /usr/include/stdio.h
mt.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
mt.o: /usr/include/stdio_impl.h ../runtime/sac_message.h
profile.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
profile.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
profile.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
profile.o: /usr/include/sys/time.h /usr/include/sys/types.h
profile.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
profile.o: /usr/include/sys/resource.h
trace.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
trace.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
trace.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
trace.o: /usr/include/stdarg.h /usr/include/varargs.h /usr/include/string.h
