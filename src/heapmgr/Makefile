#
#
# $Log$
# Revision 3.13  2003/03/20 20:36:36  sah
# added global to includes
#
# Revision 3.12  2003/03/20 14:39:13  sbs
# now builds the mt version iff DISABLE_MT is not set!
#
# Revision 3.11  2002/06/06 19:56:03  dkr
# target clean modified
#
# Revision 3.10  2002/06/06 19:35:17  dkr
# target clean: *.old removed
#
# Revision 3.9  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
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

PROJECT_ROOT := ../..
include $(PROJECT_ROOT)/Makefile.Config


#######################################################################################
#
# local settings:
#

INCS := -I../runtime -I../../inc -I../global
LIBS := -L.

FILES_HEAPMGR := calloc.c diagnostics.c internal.c large_chunks.c malloc.c memalign.c  \
                 realloc.c setup.c small_chunks.c wrapper.c placement.c

OBJS_HEAPMGR := $(patsubst %.c, %.o, $(FILES_HEAPMGR))

OBJS_HEAPMGR_MT := $(patsubst %.c, %_mt.o, $(FILES_HEAPMGR))

OBJS_HEAPMGR_DIAG := $(patsubst %.c, %_diag.o, $(FILES_HEAPMGR))

OBJS_HEAPMGR_MT_DIAG := $(patsubst %.c, %_mt_diag.o, $(FILES_HEAPMGR))



.PHONY: all tar clean prod


all: libsac_heapmgr.a libsac_heapmgr_diag.a

ifneq ($(DISABLE_MT),yes)
all: libsac_heapmgr_mt.a libsac_heapmgr_mt_diag.a 
endif


libsac_heapmgr.a: $(OBJS_HEAPMGR)
	ar rc $@ $^
	ranlib $@
	$(CLOCK_SKEW_ELIMINATION) $@

libsac_heapmgr_diag.a: $(OBJS_HEAPMGR_DIAG)
	ar rc $@ $^
	ranlib $@
	$(CLOCK_SKEW_ELIMINATION) $@

libsac_heapmgr_mt.a: $(OBJS_HEAPMGR_MT)
	ar rc $@ $^
	ranlib $@
	$(CLOCK_SKEW_ELIMINATION) $@

libsac_heapmgr_mt_diag.a: $(OBJS_HEAPMGR_MT_DIAG)
	ar rc $@ $^
	ranlib $@
	$(CLOCK_SKEW_ELIMINATION) $@


%_mt_diag.o: %.c heapmgr.h
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -DMT -DDIAG -o $@ -c $<
	$(CLOCK_SKEW_ELIMINATION) $@

%_mt.o: %.c heapmgr.h
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -DMT -o $@ -c $<
	$(CLOCK_SKEW_ELIMINATION) $@

%_diag.o: %.c heapmgr.h
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -DDIAG -o $@ -c $<
	$(CLOCK_SKEW_ELIMINATION) $@

%.o: %.c heapmgr.h
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<
	$(CLOCK_SKEW_ELIMINATION) $@


prod:
	$(MAKE) CCFLAGS="$(CCPROD_FLAGS)"


clean:
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) -r .sb SunWS_cache


#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps


