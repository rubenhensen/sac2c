#
# $Id$
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

MTFLAGS   := -DMT -D_POSIX_C_SOURCE=200112L

INCS := -I../tree -I../runtime -I../../inc -I../global
LIBS := -L.

OBJS_LIBSAC := message.o misc.o profile.o trace.o nophm.o sac_arg.o \
               cachesim_access_advanced.o cachesim_access_simple.o \
               cachesim_basic.o ../../lib/getoptions.o cinterface.o \
               commandline.o

OBJS_LIBSAC_MT := message_mt.o misc_mt.o trace_mt.o mt.o mt_trace.o \
                  nophm_mt.o sac_arg.o cinterface.o commandline.o


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

$(PROJECT_ROOT)/src/runtime/sac_mt_gen.h:
	cd $(PROJECT_ROOT)/src/runtime; make sac_mt_gen.h

$(PROJECT_ROOT)/src/runtime/sac_std_gen.h:
	cd $(PROJECT_ROOT)/src/runtime; make sac_std_gen.h

libsac.a: $(PROJECT_ROOT)/src/runtime/sac_std_gen.h $(OBJS_LIBSAC)
	ar rc $@ $^
	ranlib $@
	$(CLOCK_SKEW_ELIMINATION) $@

libsac_mt.a: $(PROJECT_ROOT)/src/runtime/sac_mt_gen.h $(OBJS_LIBSAC_MT)
	ar rc $@ $^
	ranlib $@
	$(CLOCK_SKEW_ELIMINATION) $@


libsac2c.so: sac2c.sac $(PROJECT_ROOT)/sac2c
	sac2c -v0 -O3 sac2c.sac

$(PROJECT_ROOT)/sac2c:
	cd $(PROJECT_ROOT); $(MAKE) sac2c

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
