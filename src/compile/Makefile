#
#
# $Log$
# Revision 3.21  2002/05/31 17:19:55  dkr
# icm2c_utils.o and NameTuples.o added for target BEtest
#
# Revision 3.20  2001/12/10 15:33:03  dkr
# compile.tagged.o added
#
# Revision 3.19  2001/11/21 11:04:43  dkr
# target 'all': BEtest added
#
# Revision 3.18  2001/11/21 10:32:04  dkr
# rules simplified
#
# Revision 3.17  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 3.16  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# (.xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.13  2001/05/17 13:17:19  dkr
# minor changes done
#
# Revision 3.12  2001/05/11 14:36:56  cg
# Added compilation of new source file icm2c_sched.c
#
# Revision 3.7  2001/03/02 10:36:30  dkr
# /home/dkr/sac/BASE/sac2c/src/compile/RCS/Makefile,v used in touch-rules
#
# Revision 3.1  2000/11/20 18:01:00  sacbase
# new release made
#
# [...]
#
# Revision 1.1  1995/03/29  12:12:52  hw
# Initial revision
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


INCS=-I../global -I../tree -I../../inc -I../print -I../typecheck -I../refcount \
     -I../optimize -I../psi-opt -I../scanparse -I../concurrent -I../flatten \
     -I../c-interface

MACS=icm.data icm_comment.c icm_trace.c

BETEST_OBJS = icm2c_std.BEtest.o icm2c_wl.BEtest.o \
              icm2c_mt.BEtest.o icm2c_sched.BEtest.o \
              icm2c_utils.o ../global/NameTuples.o \
              ../global/globals.o ../../lib/dbug.o

SAC2C_OBJS = wltransform.o wlpragma_funs.o precompile.o compile.o compile.tagged.o \
             ReuseWithArrays.o gen_startup_code.o PatchWith.o icm2c.o \
             icm2c_std.o icm2c_wl.o icm2c_mt.o icm2c_sched.o icm2c_utils.o


all: $(SAC2C_OBJS) BEtest


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
	$(RM) *.o *.bak *~ .*.d
	$(RM) BEtest
	$(RM) -r .sb SunWS_cache


################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps
