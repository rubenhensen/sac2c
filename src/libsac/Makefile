#
# $Log$
# Revision 1.9  2000/01/17 18:41:17  cg
# make deps done
#
# Revision 1.8  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 1.7  2000/01/17 16:25:58  cg
# Reorganized generation of two runtime libraries, one for
# sequential execution and one dedicated to multi-thraeded execution.
#
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



#
# OS setup:
#
# legal values for OS currently are:
#    SOLARIS_SPARC
#    LINUX_X86
#    OSF_ALPHA
#
OS        := SOLARIS_SPARC


CC        := gcc
CCFLAGS   := -ansi -Wall -D__EXTENSIONS__ -g 

MTFLAGS   := -DMT -D_REENTRANT -D_POSIX_C_SOURCE=199506L

INCS := -I../runtime -I../../inc
LIBS := -L.

OBJS_LIBSAC := boundcheck.o message.o misc.o profile.o trace.o nophm.o \
               cachesim_access_advanced.o cachesim_access_simple.o   \
               cachesim_basic.o ../../lib/main_args.o	

OBJS_LIBSAC_MT := boundcheck_mt.o message_mt.o misc_mt.o trace_mt.o mt.o  \
                  mt_trace.o nophm_mt.o


.PHONY: all tar clean prod


all: libsac.a libsac_mt.a 


libsac.a: $(OBJS_LIBSAC)
	ar rc $@ $^
	ranlib $@

libsac_mt.a: $(OBJS_LIBSAC_MT)
	ar rc $@ $^
	ranlib $@


mt_trace.o: mt.c 
	$(CC) -DTRACE $(CCFLAGS) $(CFLAGS) $(INCS) $(MTFLAGS) -o $@ -c $<

cachesim_access_simple.o: cachesim_access.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<

cachesim_access_advanced.o: cachesim_access.c 
	$(CC) -D TYPE=D -D DETAILED $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<


%_mt.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) $(MTFLAGS) -o $@ -c $<

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
cachesim_basic.o: /usr/include/unistd.h /usr/include/sys/unistd.h
cachesim_basic.o: ../runtime/sac_cachesim.h ../runtime/sac_message.h
cachesim_basic.o: cachesim.h ../../inc/main_args.h
message.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
message.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
message.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
message.o: /usr/include/stdarg.h /usr/include/varargs.h /usr/include/stdlib.h
message.o: ../runtime/sac_mt.h
mt.o: /usr/include/pthread.h /usr/include/sys/types.h
mt.o: /usr/include/sys/isa_defs.h /usr/include/sys/feature_tests.h
mt.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
mt.o: /usr/include/time.h /usr/include/sched.h /usr/include/stdio.h
mt.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
mt.o: /usr/include/stdio_impl.h ../runtime/sac_message.h
mt.o: ../runtime/sac_trace.h ../runtime/sac_mt.h
nophm.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
nophm.o: /usr/include/sys/isa_defs.h ../runtime/sac_message.h
nophm.o: ../runtime/sac_mt.h ../runtime/sac_heapmgr.h
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
trace.o: ../runtime/sac_mt.h
