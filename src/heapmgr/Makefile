#
# $Log$
# Revision 1.4  2000/01/17 18:41:20  cg
# make deps done
#
# Revision 1.3  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 1.2  2000/01/17 16:25:58  cg
# Reorganzied compilation of heap manager library with respect
# to the integration of a multi-threaded heap manager
#
# Revision 1.1  2000/01/03 17:33:17  cg
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


CC          := gcc
CCFLAGS     := -ansi -Wall -D__EXTENSIONS__ -g  -DSAC_FOR_$(OS)
CCFLAGSPROD := -ansi -Wall -D__EXTENSIONS__ -O3 -DSAC_FOR_$(OS)

INCS := -I../runtime -I../../inc
LIBS := -L.

FILES_HEAPMGR := calloc.c diagnostics.c internal.c large_chunks.c malloc.c memalign.c  \
                 realloc.c setup.c small_chunks.c wrapper.c

OBJS_HEAPMGR := $(patsubst %.c, %.o, $(FILES_HEAPMGR))

OBJS_HEAPMGR_MT := $(patsubst %.c, %_mt.o, $(FILES_HEAPMGR))

OBJS_HEAPMGR_DIAG := $(patsubst %.c, %_diag.o, $(FILES_HEAPMGR))

OBJS_HEAPMGR_MT_DIAG := $(patsubst %.c, %_mt_diag.o, $(FILES_HEAPMGR))



.PHONY: all tar clean prod


all: libsac_heapmgr.a libsac_heapmgr_diag.a   \
     libsac_heapmgr_mt.a libsac_heapmgr_mt_diag.a 


libsac_heapmgr.a: $(OBJS_HEAPMGR)
	ar rc $@ $^
	ranlib $@

libsac_heapmgr_diag.a: $(OBJS_HEAPMGR_DIAG)
	ar rc $@ $^
	ranlib $@

libsac_heapmgr_mt.a: $(OBJS_HEAPMGR_MT)
	ar rc $@ $^
	ranlib $@

libsac_heapmgr_mt_diag.a: $(OBJS_HEAPMGR_MT_DIAG)
	ar rc $@ $^
	ranlib $@


%_mt_diag.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -DMT -DDIAG -o $@ -c $<

%_mt.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -DMT -o $@ -c $<

%_diag.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -DDIAG -o $@ -c $<

%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<


deps:
	makedepend $(INCS) *.c

prod:
	$(MAKE) CCFLAGS="$(CCFLAGSPROD)"


clean:
	$(RM) *.o *.a *.bak *.old *~
	$(RM) -r .sb SunWS_cache


# DO NOT DELETE THIS LINE -- make depend depends on it.

calloc.o: /usr/include/string.h /usr/include/sys/feature_tests.h
calloc.o: /usr/include/sys/isa_defs.h heapmgr.h ../runtime/sac_mt.h
calloc.o: ../runtime/sac_heapmgr.h
diagnostics.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
diagnostics.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
diagnostics.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
diagnostics.o: /usr/include/stdlib.h heapmgr.h ../runtime/sac_mt.h
diagnostics.o: ../runtime/sac_heapmgr.h ../runtime/sac_message.h
internal.o: /usr/include/unistd.h /usr/include/sys/feature_tests.h
internal.o: /usr/include/sys/isa_defs.h /usr/include/sys/types.h
internal.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
internal.o: /usr/include/sys/unistd.h heapmgr.h ../runtime/sac_mt.h
internal.o: ../runtime/sac_heapmgr.h ../runtime/sac_message.h
large_chunks.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
large_chunks.o: /usr/include/sys/isa_defs.h heapmgr.h ../runtime/sac_mt.h
large_chunks.o: ../runtime/sac_heapmgr.h ../runtime/sac_message.h
malloc.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
malloc.o: /usr/include/sys/isa_defs.h heapmgr.h ../runtime/sac_mt.h
malloc.o: ../runtime/sac_heapmgr.h
memalign.o: /usr/include/unistd.h /usr/include/sys/feature_tests.h
memalign.o: /usr/include/sys/isa_defs.h /usr/include/sys/types.h
memalign.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
memalign.o: /usr/include/sys/unistd.h heapmgr.h ../runtime/sac_mt.h
memalign.o: ../runtime/sac_heapmgr.h
realloc.o: /usr/include/string.h /usr/include/sys/feature_tests.h
realloc.o: /usr/include/sys/isa_defs.h heapmgr.h ../runtime/sac_mt.h
realloc.o: ../runtime/sac_heapmgr.h
setup.o: /usr/include/unistd.h /usr/include/sys/feature_tests.h
setup.o: /usr/include/sys/isa_defs.h /usr/include/sys/types.h
setup.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
setup.o: /usr/include/sys/unistd.h heapmgr.h ../runtime/sac_mt.h
setup.o: ../runtime/sac_heapmgr.h ../runtime/sac_message.h
small_chunks.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
small_chunks.o: /usr/include/sys/isa_defs.h heapmgr.h ../runtime/sac_mt.h
small_chunks.o: ../runtime/sac_heapmgr.h
wrapper.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
wrapper.o: /usr/include/sys/isa_defs.h heapmgr.h ../runtime/sac_mt.h
wrapper.o: ../runtime/sac_heapmgr.h
