#
# $Log$
# Revision 2.14  1999/07/08 12:34:12  cg
# Complete implementation of libsac is now moved to new directory
# src/libsac; implementation of CacheSimAnalyser is moved to new
# directory /src/tools.
#
# Revision 2.13  1999/07/05 14:40:26  sbs
# sac2crc rule corrected.
# Now, a make after make clean will "automatically" create
# a link from sac2crc to sac2crc.$OS .
#
# Revision 2.12  1999/06/23 10:28:07  rob
# Insert make target of sac2crc.
#
# Revision 2.11  1999/06/02 16:55:39  dkr
# path to main_args.o is now correct
#
# Revision 2.10  1999/06/01 15:17:09  dkr
# make deps done
#
# Revision 2.8  1999/05/20 14:13:44  cg
# Compilation of CacheSimAnalyser streamlined:
# it is now linked with the SAC runtime library.
#
# Revision 2.6  1999/04/26 11:42:05  her
# modifications for the piped-cachesimulation
#
# Revision 2.5  1999/04/09 09:40:53  her
# added libsac_message.o to the CacheSimTest target
#
# Revision 2.4  1999/03/19 11:09:04  her
# new OBJS (libsac_cachesim_access_detailed.o) added
# new targets (CacheSimTest.o, CacheSimTest) added
# make deps done
#
# Revision 2.1  1999/02/23 12:43:33  sacbase
# new release made
#
# Revision 1.14  1999/01/14 13:16:19  sacbase
# clean: rm sac2crc    added
#
# Revision 1.13  1998/12/21 15:46:21  cg
# File sac2crc no longer added to runtime.tar because system dependent
# variants do exist now. File sac2c.README added instead.
#
# Revision 1.12  1998/12/17 15:49:43  cg
# added rm runtime.tar for phony target clean
#
# Revision 1.11  1998/12/17 09:50:28  cg
# File libsac.a will no longer be added to runtime.tar because
# it depends on the target architecture.
#
# Revision 1.9  1998/10/28 20:35:18  sbs
# Now, sac2crc will be part of runtime.tar as well 8-))
#
# Revision 1.8  1998/10/28 19:32:10  sbs
# runtime.tar rule inserted
#
# Revision 1.7  1998/06/29 09:16:42  cg
# The SAC runtime library is now compiled with optimizations and
# debug options.
#
# Revision 1.2  1998/03/24 13:51:04  cg
# additional include directory specified
#
# Revision 1.1  1998/03/19 16:32:50  cg
# Initial revision
#
#
# 


CC        := gcc
CCFLAGS   := -ansi -Wall -D__EXTENSIONS__ -g 

INCS := 
LIBS := 


#
# just in case we want to make sac2crc locally!!
#
OS        := SOLARIS_SPARC

.PHONY: all tar deps clean


all:  libsac.a libsac_mt.a libsac_heapmgr.a libsac_noheapmgr.a  libsac_noheapmgr_mt.a  \
      CacheSimAnalyser sac2crc

tar: runtime.tar

%.a : 
	@ if [ ! -f $@ ]; \
	    then ln -s ../libsac/$@; \
	  fi

sac2crc: 
	@ if [ ! -f $@ ]; \
	    then ln -s $@.$(OS) $@; \
	  fi

CacheSimAnalyser: 
	@ if [ ! -f $@ ]; \
	    then ln -s ../tools/$@; \
	  fi


runtime.tar: *.h sac2crc.README
	tar cvf $@ $^

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.a *.bak *.old *~
	$(RM) -r .sb
	$(RM) runtime.tar
	$(RM) sac2crc CacheSimAnalyser


