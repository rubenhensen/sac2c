#
#
# $Log$
# Revision 3.16  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.15  2001/11/13 20:32:44  dkr
# make deps done
#
# Revision 3.14  2001/07/18 13:08:18  cg
# make deps done
#
# Revision 3.13  2001/05/17 13:17:19  dkr
# minor changes done
#
# Revision 3.12  2001/05/11 14:36:56  cg
# Added compilation of new source file icm2c_sched.c
#
# Revision 3.11  2001/05/08 13:29:35  dkr
# make deps done
#
# Revision 3.10  2001/04/26 21:09:47  dkr
# make deps done
#
# Revision 3.9  2001/03/22 20:20:01  dkr
# make deps done
#
# Revision 3.8  2001/03/21 11:19:57  dkr
# make deps done
#
# Revision 3.7  2001/03/02 10:36:30  dkr
# /home/dkr/sac/BASE/sac2c/src/compile/RCS/Makefile,v used in touch-rules
#
# Revision 3.6  2001/02/02 10:39:03  dkr
# make deps done
#
# Revision 3.5  2001/01/17 10:13:04  dkr
# make deps done
#
# Revision 3.4  2000/12/15 18:18:00  dkr
# make deps done
#
# Revision 3.3  2000/12/12 15:09:09  dkr
# make deps done
#
# Revision 3.2  2000/12/06 20:09:04  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:01:00  sacbase
# new release made
#
#
# [...]
#
# Revision 1.1  1995/03/29  12:12:52  hw
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


INCS=-I../global -I../tree -I../../inc -I../print -I../typecheck -I../refcount \
     -I../optimize -I../psi-opt -I../scanparse -I../concurrent -I../flatten \
     -I../c-interface

MACS=icm.data icm_comment.c icm_trace.c

BETEST_OBJS = icm2c_std.BEtest.o icm2c_wl.BEtest.o \
              icm2c_mt.BEtest.o icm2c_sched.BEtest.o \
              ../global/globals.o ../../lib/dbug.o


all: wltransform.o wlpragma_funs.o precompile.o compile.o ReuseWithArrays.o \
     icm2c.o icm2c_std.o icm2c_wl.o icm2c_mt.o icm2c_sched.o icm2c_utils.o \
     gen_startup_code.o PatchWith.o


#
# the wltransform.c object is to fool makedepend which does not
# recognize the dependency between wltransform.c and the .mac files!
#
wltransform.c: wlpragma_funs.mac
	touch $@

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<


BEtest: BEtest.c icm_vars.c icm_betest.c icm.data $(BETEST_OBJS)
	$(CC) $(CCFLAGS) $(CFLAGS) -o BEtest $(INCS) BEtest.c $(BETEST_OBJS)

icm2c_std.BEtest.o: icm2c_std.c $(MACS)
	$(CC) $(CCFLAGS) $(CFLAGS) -DBEtest -o icm2c_std.BEtest.o -c icm2c_std.c $(INCS)

icm2c_wl.BEtest.o: icm2c_wl.c $(MACS)
	$(CC) $(CCFLAGS) $(CFLAGS) -DBEtest -o icm2c_wl.BEtest.o -c icm2c_wl.c $(INCS)

icm2c_mt.BEtest.o: icm2c_mt.c $(MACS)
	$(CC) $(CCFLAGS) $(CFLAGS) -DBEtest -o icm2c_mt.BEtest.o -c icm2c_mt.c $(INCS)

icm2c_sched.BEtest.o: icm2c_sched.c $(MACS)
	$(CC) $(CCFLAGS) $(CFLAGS) -DBEtest -o icm2c_sched.BEtest.o -c icm2c_sched.c $(INCS)

icm2c_utils.BEtest.o: icm2c_utils.c $(MACS)
	$(CC) $(CCFLAGS) $(CFLAGS) -DBEtest -o icm2c_utils.BEtest.o -c icm2c_utils.c $(INCS)


clean:
	$(RM) *.o *.bak *~ BEtest *.sib *.sac .*.d
	$(RM) -r .sb SunWS_cache


#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps


