#
# $Log$
# Revision 1.1  1999/07/08 12:30:54  cg
# Initial revision
#
# 


CC        := gcc
CCFLAGS   := -ansi -Wall -D__EXTENSIONS__ -g -O3

MTFLAGS   := -D_REENTRANT -D_POSIX_C_SOURCE=199506L

INCS := -I../runtime -I../../inc
LIBS := -L.

OBJS_LIBSAC := boundcheck.o message.o misc.o profile.o trace.o  \
               cachesim_access_advanced.o cachesim_access_simple.o   \
               cachesim_basic.o ../../lib/main_args.o	

OBJS_LIBSAC_MT := boundcheck_mt.o message_mt.o misc_mt.o trace_mt.o mt.o 

OBJS_LIBSAC_HEAPMGR := heapmgr_basic.o heapmgr_malloc.o heapmgr_memalign.o \
                       heapmgr_realloc.o

OBJS_LIBSAC_NOHEAPMGR := heapmgr_disabled.o

OBJS_LIBSAC_NOHEAPMGR_MT := heapmgr_disabled_mt.o



.PHONY: all tar


all: libsac.a libsac_mt.a libsac_heapmgr.a libsac_noheapmgr.a libsac_noheapmgr_mt.a


libsac.a: $(OBJS_LIBSAC)
	ar rc $@ $^
	ranlib $@

libsac_mt.a: $(OBJS_LIBSAC_MT)
	ar rc $@ $^
	ranlib $@

libsac_heapmgr.a: $(OBJS_LIBSAC_HEAPMGR)
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

%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<


deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.a *.bak *.old *~
	$(RM) -r .sb


