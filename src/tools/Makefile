#
#
# $Log$
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

include ../../Makefile.Config


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
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) $(LIBS) -o $@ $< -lsac -lm

CacheSimTest: CacheSimTest.o $(LIBDIR)/libsac.a
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) $(LIBS) -o $@ $< -lsac -lm

%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<


clean:
	$(RM) *.o *.a *.bak *.old *~ .*.d
	$(RM) -r .sb SunWS_cache
	$(RM) CacheSimTest CacheSimAnalyser


#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps



