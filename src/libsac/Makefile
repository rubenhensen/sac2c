#
#
# $Log$
# Revision 3.18  2003/03/20 21:52:51  sah
# added -I../global to includes
#
# Revision 3.17  2003/03/20 14:41:09  sbs
# now builds mt version iff DISABLE_MT is not set
#
# Revision 3.16  2003/03/20 14:23:56  sbs
# libsac_mt.a hidden behind $(LIBSAC_MT).
#
# Revision 3.15  2002/07/03 13:49:05  dkr
# type corrected #$%-(
#
# Revision 3.14  2002/07/03 13:47:00  dkr
# boundcheck.o removed
#
# Revision 3.13  2002/06/06 19:56:03  dkr
# target clean modified
#
# Revision 3.12  2002/06/06 19:35:17  dkr
# target clean: *.old removed
#
# Revision 3.11  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 3.10  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
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
# ... [eliminated] ...
#
# Revision 1.1  1999/07/08 12:30:54  cg
# Initial revision
#
# 


##############################################################################
#
# include general setup:
#

PROJECT_ROOT := ../..
include $(PROJECT_ROOT)/Makefile.Config


##############################################################################
#
# local settings:
#

MTFLAGS   := -DMT -D_REENTRANT -D_POSIX_C_SOURCE=199506L

INCS := -I../tree -I../runtime -I../../inc -I../global
LIBS := -L.

OBJS_LIBSAC := message.o misc.o profile.o trace.o nophm.o sac_arg.o \
               cachesim_access_advanced.o cachesim_access_simple.o \
               cachesim_basic.o ../../lib/main_args.o cinterface.o

OBJS_LIBSAC_MT := message_mt.o misc_mt.o trace_mt.o mt.o mt_trace.o \
                  nophm_mt.o sac_arg.o cinterface.o


.PHONY: all tar clean prod


all: libsac.a
ifneq ($(DISABLE_MT),yes)
all: libsac_mt.a
endif


#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#
cinterface.c: ../tree/type_info.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@


libsac.a: $(OBJS_LIBSAC)
	ar rc $@ $^
	ranlib $@
	$(CLOCK_SKEW_ELIMINATION) $@

libsac_mt.a: $(OBJS_LIBSAC_MT)
	ar rc $@ $^
	ranlib $@
	$(CLOCK_SKEW_ELIMINATION) $@


mt_trace.o: mt.c 
	$(CC) -DTRACE $(CCFLAGS) $(CFLAGS) $(INCS) $(MTFLAGS) -o $@ -c $<
	$(CLOCK_SKEW_ELIMINATION) $@

cachesim_access_simple.o: cachesim_access.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<
	$(CLOCK_SKEW_ELIMINATION) $@

cachesim_access_advanced.o: cachesim_access.c 
	$(CC) -DTYPE=D -DDETAILED $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<
	$(CLOCK_SKEW_ELIMINATION) $@


%_mt.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) $(MTFLAGS) -o $@ -c $<
	$(CLOCK_SKEW_ELIMINATION) $@

%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<
	$(CLOCK_SKEW_ELIMINATION) $@


prod:
	$(MAKE) CCFLAGS="$(CCFLAGS) -O3"


clean:
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) -r .sb SunWS_cache


##############################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps
