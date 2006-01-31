#
# $Id$
#


###############################################################################
#
# include general setup:
#

PROJECT_ROOT := ../..

include $(PROJECT_ROOT)/Makefile.Source



###############################################################################
#
# subdirectory specific aspects
#


MACS = icm.data icm_comment.c icm_trace.c

BETEST_OBJS = icm2c_std.BEtest.o icm2c_prf.BEtest.o icm2c_wl.BEtest.o \
              icm2c_mt.BEtest.o icm2c_sched.BEtest.o icm2c_error.BEtest.o \
              icm2c_basic.o icm2c_utils.o \
              ../global/NameTuples.o ../global/globals.o ../../lib/dbug.o

BEtest: BEtest.c icm_vars.c icm_betest.c icm.data $(BETEST_OBJS)
	$(CC) $(CCFLAGS) $(CFLAGS) -o BEtest $(INCS) BEtest.c $(BETEST_OBJS)
	$(CLOCK_SKEW_ELIMINATION) $@

%.BEtest.o: %.c $(MACS)
	$(CC) $(CCFLAGS) $(CFLAGS) -DBEtest -o $@ -c $< $(INCS)
	$(CLOCK_SKEW_ELIMINATION) $@


