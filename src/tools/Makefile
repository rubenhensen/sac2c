#
#
# $Log$
# Revision 3.10  2002/06/06 19:56:03  dkr
# target clean modified
#
# Revision 3.9  2002/06/06 19:35:17  dkr
# target clean: *.old removed
#
# Revision 3.8  2001/11/21 18:38:20  sbs
# strange requirement on MACOS to re-run ranlib on libsac.a 8-(
#
# Revision 3.7  2001/11/20 13:37:42  sbs
# definition of LIBM moved to Makefile.Config as it is platform specific.
#
# Revision 3.6  2001/11/20 13:35:23  sbs
# -lm commented out on MAC_OS
#
# Revision 3.5  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 3.4  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.3  2001/11/13 20:32:49  dkr
# make deps done
#
# Revision 3.2  2000/12/06 20:09:10  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:02:49  sacbase
# new release made
#
# Revision 1.8  2000/11/02 14:43:50  dkr
# header compressed
#
# Revision 1.7  2000/07/28 09:16:27  mab
# make deps done
#
# Revision 1.5  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 1.2  1999/07/08 15:39:55  cg
# Dependency from libsac now uses libsac/libsac.a instead of
# runtime/libsac.a
#
# Revision 1.1  1999/07/08 12:02:34  cg
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


LIBDIR    := ../libsac
  # Here, we cannot use the links in ../runtime because after a gmake clean
  # they are not yet existing.

INCS      := -I../runtime -I../libsac -I../../inc
LIBS      := -L$(LIBDIR)


.PHONY: all tar


all:  CacheSimAnalyser CacheSimTest


CacheSimAnalyser: CacheSimAnalyser.o $(LIBDIR)/libsac.a
ifeq ($(OS),OSX_MAC)
	ranlib ../libsac/libsac.a
endif
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) $(LIBS) -o $@ $< -lsac $(LIBM)
	$(CLOCK_SKEW_ELIMINATION) $@


CacheSimTest: CacheSimTest.o $(LIBDIR)/libsac.a
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) $(LIBS) -o $@ $< -lsac $(LIBM)
	$(CLOCK_SKEW_ELIMINATION) $@


%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<
	$(CLOCK_SKEW_ELIMINATION) $@


clean:
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) -r .sb SunWS_cache
	$(RM) CacheSimTest CacheSimAnalyser


#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps

