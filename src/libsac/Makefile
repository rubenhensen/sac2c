#
#
# $Log$
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


#######################################################################################
#
# include general setup:
#

include ../../Makefile.Config


#######################################################################################
#
# local settings:
#

MTFLAGS   := -DMT -D_REENTRANT -D_POSIX_C_SOURCE=199506L

INCS := -I../tree -I../runtime -I../../inc
LIBS := -L.

OBJS_LIBSAC := boundcheck.o message.o misc.o profile.o trace.o nophm.o \
               cachesim_access_advanced.o cachesim_access_simple.o   \
               cachesim_basic.o ../../lib/main_args.o sac_arg.o cinterface.o

OBJS_LIBSAC_MT := boundcheck_mt.o message_mt.o misc_mt.o trace_mt.o mt.o  \
                  mt_trace.o nophm_mt.o sac_arg.o cinterface.o


.PHONY: all tar clean prod


all: libsac.a libsac_mt.a 


#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#
cinterface.c: ../tree/type_info.mac
	touch $@


libsac.a: $(OBJS_LIBSAC)
	ar rc $@ $^
	ranlib $@

libsac_mt.a: $(OBJS_LIBSAC_MT)
	ar rc $@ $^
	ranlib $@


mt_trace.o: mt.c 
	$(CC) -DTRACE $(CCFLAGS) $(CFLAGS) $(INCS) $(MTFLAGS) -o $@ -c $<

cachesim_access_simple.o: cachesim_access.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<

cachesim_access_advanced.o: cachesim_access.c 
	$(CC) -DTYPE=D -DDETAILED $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<


%_mt.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) $(MTFLAGS) -o $@ -c $<

%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<


prod:
	$(MAKE) CCFLAGS="$(CCFLAGS) -O3"


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


