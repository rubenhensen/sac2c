#
# $Log$
# Revision 1.1  2000/01/03 17:33:17  cg
# Initial revision
#
#


CC        := gcc
CCFLAGS   := -ansi -Wall -D__EXTENSIONS__ -g 

MTFLAGS   := -D_REENTRANT -D_POSIX_C_SOURCE=199506L

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
     libsac_heapmgr_mt.a libsac_heapmgr_mt_diag.a   \
     libsac_heapmgr_nophm.a


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

libsac_heapmgr_nophm.a: nophm.o
	ar rc $@ $^
	ranlib $@


%_mt_diag.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) $(MTFLAGS) -DMT -DDIAG -o $@ -c $<

%_mt.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) $(MTFLAGS) -DMT -o $@ -c $<

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


