#
#
# $Log$
# Revision 3.9  2001/11/13 20:32:49  dkr
# make deps done
#
# Revision 3.8  2001/11/13 20:00:58  dkr
# parameters for cc: no space between -D and the definition!!
#
# Revision 3.7  2001/07/18 13:08:20  cg
# make deps done
#
# Revision 3.6  2001/03/02 10:36:09  dkr
# /home/dkr/sac/BASE/sac2c/src/libsac/RCS/Makefile,v used in touch-rules
#
# Revision 3.5  2000/12/12 15:09:19  dkr
# make deps done
#
# Revision 3.4  2000/12/08 10:41:48  cg
# Option "-w" removed again.
#
# Revision 3.3  2000/12/06 20:09:09  dkr
# make deps done
#
# Revision 3.2  2000/12/06 17:39:01  cg
# Added new gcc option "-w" in order to disable nasty compiler warnings
# with respect to missing braces in the static initialization of mutex
# locks.
#
# Revision 3.1  2000/11/20 18:02:39  sacbase
# new release made
#
# Revision 1.23  2000/11/02 14:51:45  dkr
# INC path -I../tree added
# explicit .mac dependencies added
#
# Revision 1.19  2000/08/01 15:05:17  nmw
# c-interface in libsab_mt.a, too
#
# Revision 1.16  2000/07/20 11:39:19  nmw
# free_cinterface_handler.c added
#
# Revision 1.13  2000/07/05 12:52:17  nmw
# sac_Arg.c and cinterface.c added
#
# Revision 1.10  2000/01/17 19:14:24  cg
# Small bug fixed.
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
CCFLAGS   := -ansi -Wall -D__EXTENSIONS__ -g -DSAC_FOR_$(OS)

MTFLAGS   := -DMT -D_REENTRANT -D_POSIX_C_SOURCE=199506L

INCS := -I../tree -I../runtime -I../../inc
LIBS := -L.

OBJS_LIBSAC := boundcheck.o message.o misc.o profile.o trace.o nophm.o \
               cachesim_access_advanced.o cachesim_access_simple.o   \
               cachesim_basic.o ../../lib/main_args.o sac_arg.o cinterface.o

OBJS_LIBSAC_MT := boundcheck_mt.o message_mt.o misc_mt.o trace_mt.o mt.o  \
                  mt_trace.o nophm_mt.o sac_arg.o cinterface.o


.PHONY: all tar clean prod


all: libsac.a libsac_mt.a 


#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#
cinterface.c: ../tree/type_info.mac
	touch $@


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
	$(CC) -DTYPE=D -DDETAILED $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<


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
cachesim_access.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
cachesim_access.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
cachesim_access.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
cachesim_access.o: /usr/include/stdio_impl.h ../runtime/sac_cachesim.h
cachesim_access.o: cachesim.h
cachesim_basic.o: /usr/include/memory.h /usr/include/sys/types.h
cachesim_basic.o: /usr/include/sys/isa_defs.h
cachesim_basic.o: /usr/include/sys/feature_tests.h
cachesim_basic.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
cachesim_basic.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
cachesim_basic.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
cachesim_basic.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
cachesim_basic.o: /usr/include/stdio_impl.h /usr/include/string.h
cachesim_basic.o: /usr/include/iso/string_iso.h /usr/include/unistd.h
cachesim_basic.o: /usr/include/sys/unistd.h ../runtime/sac_cachesim.h
cachesim_basic.o: ../runtime/sac_message.h cachesim.h ../../inc/main_args.h
cinterface.o: /usr/include/stdarg.h /usr/include/varargs.h
cinterface.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
cinterface.o: /usr/include/iso/stdlib_iso.h /usr/include/sys/feature_tests.h
cinterface.o: /usr/include/sys/isa_defs.h /usr/include/stdio.h
cinterface.o: /usr/include/iso/stdio_iso.h /usr/include/stdio_tag.h
cinterface.o: /usr/include/stdio_impl.h /usr/include/string.h
cinterface.o: /usr/include/iso/string_iso.h ../runtime/sac.h
cinterface.o: ../runtime/sac_icm.h ../runtime/sac_boundcheck.h
cinterface.o: ../runtime/sac_misc.h ../runtime/sac_message.h
cinterface.o: ../runtime/sac_bool.h ../runtime/sac_std.h ../runtime/sac_idx.h
cinterface.o: ../runtime/sac_prf.h ../runtime/sac_mt.h
cinterface.o: ../runtime/sac_heapmgr.h ../runtime/sac_wl.h
cinterface.o: ../runtime/sac_trace.h ../runtime/sac_profile.h
cinterface.o: ../runtime/sac_cachesim.h ../runtime/sac_arg.h
cinterface.o: ../runtime/sac_cinterface.h
message.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
message.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
message.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
message.o: /usr/include/stdio_impl.h /usr/include/stdarg.h
message.o: /usr/include/varargs.h /usr/include/stdlib.h
message.o: /usr/include/iso/stdlib_iso.h ../runtime/sac_heapmgr.h
message.o: ../runtime/sac_mt.h
mt.o: /usr/include/pthread.h /usr/include/sys/types.h
mt.o: /usr/include/sys/isa_defs.h /usr/include/sys/feature_tests.h
mt.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
mt.o: /usr/include/time.h /usr/include/iso/time_iso.h /usr/include/sched.h
mt.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
mt.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
mt.o: /usr/include/stdio_impl.h ../runtime/sac_message.h
mt.o: ../runtime/sac_trace.h ../runtime/sac_mt.h
nophm.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
nophm.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
nophm.o: ../runtime/sac_message.h ../runtime/sac_mt.h
nophm.o: ../runtime/sac_heapmgr.h
profile.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
profile.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
profile.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
profile.o: /usr/include/stdio_impl.h /usr/include/sys/time.h
profile.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
profile.o: /usr/include/sys/int_types.h /usr/include/sys/resource.h
sac_arg.o: /usr/include/stdarg.h /usr/include/varargs.h
sac_arg.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
sac_arg.o: /usr/include/iso/stdlib_iso.h /usr/include/sys/feature_tests.h
sac_arg.o: /usr/include/sys/isa_defs.h /usr/include/stdio.h
sac_arg.o: /usr/include/iso/stdio_iso.h /usr/include/stdio_tag.h
sac_arg.o: /usr/include/stdio_impl.h /usr/include/string.h
sac_arg.o: /usr/include/iso/string_iso.h ../runtime/sac.h
sac_arg.o: ../runtime/sac_icm.h ../runtime/sac_boundcheck.h
sac_arg.o: ../runtime/sac_misc.h ../runtime/sac_message.h
sac_arg.o: ../runtime/sac_bool.h ../runtime/sac_std.h ../runtime/sac_idx.h
sac_arg.o: ../runtime/sac_prf.h ../runtime/sac_mt.h ../runtime/sac_heapmgr.h
sac_arg.o: ../runtime/sac_wl.h ../runtime/sac_trace.h
sac_arg.o: ../runtime/sac_profile.h ../runtime/sac_cachesim.h
sac_arg.o: ../runtime/sac_arg.h ../runtime/sac_cinterface.h
sac_arg.o: ../tree/type_info.mac
trace.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
trace.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
trace.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
trace.o: /usr/include/stdio_impl.h /usr/include/stdarg.h
trace.o: /usr/include/varargs.h /usr/include/string.h
trace.o: /usr/include/iso/string_iso.h ../runtime/sac_mt.h
