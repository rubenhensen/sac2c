#
#
# $Id$
#
# 



################################################################################
#
# include general setup:
#

PROJECT_ROOT := ../..
include $(PROJECT_ROOT)/Makefile.Config


################################################################################
#
# local settings:
#

MACS = icm.data icm_comment.c icm_trace.c

BETEST_OBJS = icm2c_std.BEtest.o icm2c_prf.BEtest.o icm2c_wl.BEtest.o \
              icm2c_mt.BEtest.o icm2c_sched.BEtest.o icm2c_error.BEtest.o \
              icm2c_basic.o icm2c_utils.o \
              ../global/NameTuples.o ../global/globals.o ../../lib/dbug.o

SAC2C_OBJS := wlpragma_funs.o precompile.o compile.o wltransform.o \
              gen_startup_code.o icm2c.o \
              icm2c_std.o icm2c_prf.o icm2c_wl.o icm2c_mt.o icm2c_sched.o \
              icm2c_error.o icm2c_basic.o icm2c_utils.o markmemvals.o \
              typeconv_precompile.o functionprecompile.o renameidentifiers.o \
              setlinksign.o remove_external_code.o wlidxs.o simd_infer.o \
              mark_noop_grids.o


all: $(SAC2C_OBJS)

# BEtest


#
# the wltransform.c object is to fool makedepend which does not
# recognize the dependency between wltransform.c and the .mac files!
#
wltransform.c: wlpragma_funs.mac
	touch $@


%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<
	$(CLOCK_SKEW_ELIMINATION) $@


BEtest: BEtest.c icm_vars.c icm_betest.c icm.data $(BETEST_OBJS)
	$(CC) $(CCFLAGS) $(CFLAGS) -o BEtest $(INCS) BEtest.c $(BETEST_OBJS)
	$(CLOCK_SKEW_ELIMINATION) $@

%.BEtest.o: %.c $(MACS)
	$(CC) $(CCFLAGS) $(CFLAGS) -DBEtest -o $@ -c $< $(INCS)
	$(CLOCK_SKEW_ELIMINATION) $@


clean:
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) BEtest
	$(RM) -r .sb SunWS_cache


################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps
