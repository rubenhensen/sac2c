#
#
# $Log$
# Revision 3.8  2001/11/14 18:49:32  dkr
# explicit depedency 'heapmgr.h' added
#
# Revision 3.7  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.3  2000/12/06 18:11:01  cg
# Added initial version of array placement.
#
# Revision 3.2  2000/12/06 17:42:15  cg
# Added new gcc option "-w" in order to disable nasty compiler warnings
# with respect to missing braces in the static initialization of mutex
# locks.
#
# Revision 3.1  2000/11/20 18:02:50  sacbase
# new release made
#
# ... [eliminated] ...
#
# Revision 1.1  2000/01/03 17:33:17  cg
# Initial revision
#
#



#######################################################################################
#
# include general setup:
#

include ../../Makefile.Config


#######################################################################################
#
# local settings:
#


INCS := -I../runtime -I../../inc
LIBS := -L.

FILES_HEAPMGR := calloc.c diagnostics.c internal.c large_chunks.c malloc.c memalign.c  \
                 realloc.c setup.c small_chunks.c wrapper.c placement.c

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


%_mt_diag.o: %.c heapmgr.h
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -DMT -DDIAG -o $@ -c $<

%_mt.o: %.c heapmgr.h
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -DMT -o $@ -c $<

%_diag.o: %.c heapmgr.h
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -DDIAG -o $@ -c $<

%.o: %.c heapmgr.h
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<


prod:
	$(MAKE) CCFLAGS="$(CCPROD_FLAGS)"


clean:
	$(RM) *.o *.a *.bak *.old *~ .*.d
	$(RM) -r .sb SunWS_cache




#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps



