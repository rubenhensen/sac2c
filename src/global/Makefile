#
#
# $Log$
# Revision 3.23  2004/11/27 02:39:02  sbs
# *** empty log message ***
#
# Revision 3.22  2004/11/23 12:51:04  cg
# Removed legacy code.
#
# Revision 3.21  2004/07/14 15:29:54  ktr
# Nothing really changed.
#
# Revision 3.20  2003/10/19 21:41:07  dkrHH
# dependency prf_node_info.mac -> globals.c added
#
# Revision 3.19  2003/06/11 21:40:58  ktr
# included ../constants
#
# Revision 3.18  2003/03/25 16:25:09  sah
# added portable way to get the
# current hostname to build.c
# mechanism
#
# Revision 3.17  2002/06/07 14:42:16  dkr
# header compressed
#
# Revision 3.16  2002/06/07 14:41:32  dkr
# dependencies for argtag_info.mac modified
#
# Revision 3.15  2002/06/06 19:56:03  dkr
# target clean modified
#
# Revision 3.14  2002/05/31 17:22:47  dkr
# NameTuplesUtils.o added
#
# Revision 3.13  2002/03/01 02:43:00  dkr
# file argtag_info.mac added
#
# Revision 3.12  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 3.11  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.5  2001/03/09 11:15:11  sbs
# -I ../profile/ added
#
# Revision 3.4  2001/03/02 10:36:02  dkr
# /home/dkr/sac/BASE/sac2c/src/global/RCS/Makefile,v used in touch-rules
#
# Revision 3.1  2000/11/20 17:59:26  sacbase
# new release made
#
# [...]
#
# Revision 1.2  1994/11/10  15:44:34  sbs
# RCS-header inserted
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


all: main.o Error.o usage.o internal_lib.o globals.o resource.o \
     build.o interrupt.o options.o NameTuplesUtils.o filemgr.o

build.c: 
	echo "char build_date[] = \"`date`\";"  >  build.c
	echo "char build_user[] = \"$(USER)\";" >> build.c
	echo "char build_host[] = \"`hostname`\";" >> build.c
	echo "char build_os[]   = \"$(OS)\";"   >> build.c
	cat build.c
	$(CLOCK_SKEW_ELIMINATION) $@

build.o: build.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o build.o -c build.c $(INCS)
	$(RM) build.c
	$(CLOCK_SKEW_ELIMINATION) $@

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)
	$(CLOCK_SKEW_ELIMINATION) $@

clean:
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) -r .sb SunWS_cache


################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps
