#
#
# $Log$
# Revision 3.3  2000/11/21 18:37:26  dkr
# .SECONDARY reactivated and targets libsac.tar.for.% and sac2c.for....
# splitted:
# Now, neither the temporary files are removed automatically nor sac2c is
# build twice for LINUX and ALPHA.
# I really don't know why the rule splitting is nessesary BUT IT WORKS
# :-)
#
# Revision 3.2  2000/11/21 16:48:13  dkr
# new rules Makefile.Config.for.%-2_5, Makefile.Config.for.%-2_7 added
# new rule allclean added
# rule allnew uses allclean instead of clean now
#
# Revision 3.1  2000/11/20 18:02:37  sacbase
# new release made
#
# Revision 2.13  2000/11/20 17:10:00  sbs
# sac2crc.for.%-2_5 / sac2crc.for.%-2_7 rules extended by mv!
#
# Revision 2.12  2000/11/20 16:38:17  dkr
# new targets solaris25, solaris27 added
#
# Revision 2.11  2000/11/20 14:32:58  sacbase
# locker and update targets extended by DEMOBASE and rmobsfiles
#
# Revision 2.10  2000/11/16 12:58:24  dkr
# libsac.tar is merged into runtime.tar
# Makefile.Config is merged into stdlib.tar
#
# Revision 2.9  2000/11/15 19:39:24  dkr
# no line contains more than 80 chars now
#
# Revision 2.8  2000/11/15 17:32:07  dkr
# dependency for Makefile.Config.for.% added
#
# Revision 2.7  2000/11/15 17:12:25  dkr
# system specific Makefile.Config added
# .SECONDARY added to avoid removal of temporary files
#
# Revision 2.4  2000/11/14 16:29:35  dkr
# OSF_ALPHA added
#
# Revision 2.3  2000/04/27 15:44:51  cg
# Makefile for binary SAC distributions now handles multipart
# SAC runtime libraries for Solaris as well as Linux.
#
# Revision 2.2  1999/05/07 09:44:00  sbs
# version set to 0.9
#
# Revision 2.1  1999/02/23 12:44:24  sacbase
# new release made
#
# Revision 1.3  1999/01/18 16:08:22  sbs
# version changed from 0.7 to 0.8!
#
# Revision 1.2  1998/12/21 15:47:42  cg
# major improvements over initial revision
#
# Revision 1.1  1998/12/17 09:46:56  cg
# Initial revision
#
#


#
# Makefile for generating SAC distributions
#


#
#  Settings for version control
#

VERSION    =  0_9


#
#  Settings for file and package sets
#

SYSTEMS  =  SOLARIS_SPARC-2_5 SOLARIS_SPARC-2_7 LINUX_X86 OSF_ALPHA
INST_PACKAGES   =  runtime.tar stdlib.tar demo.tar
INST_FILES      =  install_sac LICENSE README
INST_SPECIFICS  =  sac2c sac2crc
SPECIFICS  =  $(INST_SPECIFICS) libsac.tar Makefile.Config


#
#  Settings for temp directories
#

TEMPDIR    =  /tmp


#
#  Settings for source directories
#

SAC2CDIR    =  $(shell pwd)/../..
RUNTIMEDIR  =  $(SAC2CDIR)/src/runtime
STDLIBDIR   =  $(SACBASE)/stdlib
DEMOBASE    =  $(HOME)/sac
DEMODIR     =  $(SACBASE)/demo


#
#  Settings for target directories
#

BINTARGETDIR  =  ../binpacks
SRCTARGETDIR  =  ../srcpacks


#
#  Settings for system SOLARIS_SPARC
#

SOLARIS_HOST-2_5  =  twice
SOLARIS_HOST-2_7  =  idefix
SOLARIS_DIR       =  /var/tmp/$(USER)_sac_distrib


#
#  Settings for system LINUX_X86
#

LX86_HOST  =  bunasera
LX86_USER  =  sac
LX86_DIR   =  sac_distrib
LX86_MAKE  =  make


#
#  Settings for system OSF_ALPHA
#

ALPHA_HOST  =  scalar03.rz
ALPHA_USER  =  sunip112
ALPHA_DIR   =  sac_distrib
ALPHA_MAKE  =  gmake


#
#  Internal settings
#

DATE          :=  $(shell date +%d_%b_%y)
BINDISTRIBS   :=  $(addprefix $(BINTARGETDIR)/SAC-$(VERSION)-, \
                  $(addsuffix -$(DATE).tar.gz,$(SYSTEMS))) 
SRCDISTRIBS   :=  $(SRCTARGETDIR)/SAC-$(VERSION)-$(DATE).tar.gz
CHECKFILES    :=  $(INST_PACKAGES) $(foreach system,$(SYSTEMS), \
                  $(addsuffix .for.$(system),$(SPECIFICS)))


.PHONY: all source linux solaris25 solaris27 alpha check_existing_files \
  clean allclean locker update new allnew

.SECONDARY: $(CHECKFILES)


all:     check_existing_files $(BINDISTRIBS) $(SRCDISTRIBS)

source:  check_existing_files $(SRCDISTRIBS)

linux:   check_existing_files \
  $(BINTARGETDIR)/SAC-$(VERSION)-LINUX_X86-$(DATE).tar.gz

solaris25: check_existing_files \
  $(BINTARGETDIR)/SAC-$(VERSION)-SOLARIS_SPARC-2_5-$(DATE).tar.gz

solaris27: check_existing_files \
  $(BINTARGETDIR)/SAC-$(VERSION)-SOLARIS_SPARC-2_7-$(DATE).tar.gz

alpha:   check_existing_files \
  $(BINTARGETDIR)/SAC-$(VERSION)-OSF_ALPHA-$(DATE).tar.gz


$(SRCTARGETDIR)/SAC-$(VERSION)-$(DATE).tar.gz: \
  $(INST_PACKAGES) $(INST_FILES) src.tar.gz
	@echo "**************************************************************"
	@echo "*  Creating SAC source distribution"
	@echo "**************************************************************"
	tar cvfh $(SRCTARGETDIR)/SAC-$(VERSION)-$(DATE).tar $^
	gzip -f $(SRCTARGETDIR)/SAC-$(VERSION)-$(DATE).tar
	rm -f src.tar.gz


$(BINTARGETDIR)/SAC-$(VERSION)-%-$(DATE).tar.gz: \
  $(INST_PACKAGES) $(INST_FILES) $(addsuffix .for.%,$(SPECIFICS))
	@echo "**************************************************************"
	@echo "*  Creating SAC binary distribution for $*"
	@echo "**************************************************************"
	mkdir -p $(TEMPDIR)/$*
	mkdir -p $(TEMPDIR)/$*/_tar_
	cp -f $(INST_PACKAGES) $(TEMPDIR)/$*
	cp -f $(INST_FILES) $(TEMPDIR)/$*
	for file in $(SPECIFICS); \
	    do cp -f $${file}.for.$* $(TEMPDIR)/$*/$${file}; done
	@echo "*** merging libsac.tar into runtime.tar ***"
	( cd $(TEMPDIR)/$*/_tar_;                        \
	  rm -rf *;                                      \
	  tar xvf ../libsac.tar;                         \
	  tar uvf ../runtime.tar `tar tf ../libsac.tar`; )
	@echo "*** merging Makefile.Config into stdlib.tar ***"
	( cd $(TEMPDIR)/$*/_tar_;             \
	  rm -rf *;                           \
	  tar xvf ../stdlib.tar;              \
	  mv -f ../Makefile.Config Makefiles; \
	  tar cvf ../stdlib.tar `tar tf ../stdlib.tar`; )
	@echo "*** creating SAC-$(VERSION)-$*-$(DATE).tar ***"
	( cd $(TEMPDIR)/$*;                                     \
	  tar cvf SAC-$(VERSION)-$*-$(DATE).tar                 \
	      $(INST_PACKAGES) $(INST_FILES) $(INST_SPECIFICS); \
	  gzip -f SAC-$(VERSION)-$*-$(DATE).tar )
	mv -f $(TEMPDIR)/$*/SAC-$(VERSION)-$*-$(DATE).tar.gz $(BINTARGETDIR)
	rm -rf $(TEMPDIR)/$*


src.tar.gz:
	( cd $(SAC2CDIR); $(MAKE) tar )
	ln -s $(SAC2CDIR)/$@


sac2c.for.SOLARIS_SPARC-%:
	@echo "**************************************************************"
	@echo "*  Creating sac2c for SOLARIS_SPARC-$* on host $(SOLARIS_HOST-$*)"
	@echo "**************************************************************"
	rsh $(SOLARIS_HOST-$*)            \
	    'setenv PATH $(PATH);'        \
            'cd $(SAC2CDIR)/distrib/src;' \
	    '$(MAKE) sac2c.for.SOLARIS_SPARC'
	mv -f sac2c.for.SOLARIS_SPARC sac2c.for.SOLARIS_SPARC-$*
	mv -f libsac.tar.for.SOLARIS_SPARC libsac.tar.for.SOLARIS_SPARC-$*


sac2c.for.SOLARIS_SPARC:
	( cd $(SAC2CDIR); $(MAKE) tar )
	rm -rf $(SOLARIS_DIR)
	mkdir -p $(SOLARIS_DIR)
	cp $(SAC2CDIR)/src.tar.gz $(SOLARIS_DIR)
	( cd $(SOLARIS_DIR); gunzip -f src.tar.gz )
	( cd $(SOLARIS_DIR); tar xvf src.tar )
	( cd $(SOLARIS_DIR); $(MAKE) deps OS=SOLARIS_SPARC )
	( cd $(SOLARIS_DIR); $(MAKE) distrib_product OS=SOLARIS_SPARC )
	cp $(SOLARIS_DIR)/sac2c sac2c.for.SOLARIS_SPARC
	( cd $(SOLARIS_DIR)/src/runtime; tar cvfh libsac.tar libsac*.a ) 
	cp $(SOLARIS_DIR)/src/runtime/libsac.tar libsac.tar.for.SOLARIS_SPARC
	@ if [ $${?} -eq 0 ]; then                                 \
            rm -rf $(SOLARIS_DIR);                                 \
          else                                                     \
            echo "Compilation of sac2c for SOLARIS_SPARC failed!"; \
            echo "Unable to create SOLARIS_SPARC distribution !";  \
            exit 1;                                                \
          fi

sac2c.for.LINUX_X86:
	@echo "**************************************************************"
	@echo "*  Creating sac2c for LINUX_X86"
	@echo "**************************************************************"
	@ ping $(LX86_HOST) >/dev/null;                       \
          if [ $${?} -ne 0 ]; then                            \
            echo "Host $(LX86_HOST) is down !";               \
            echo "Unable to create LINUX_X86 distribution !"; \
            exit 1;                                           \
          fi
	( cd $(SAC2CDIR); $(MAKE) tar )
	rsh -l $(LX86_USER) $(LX86_HOST) \
	    'rm -rf $(LX86_DIR); mkdir -p $(LX86_DIR)'
	rcp $(SAC2CDIR)/src.tar.gz $(LX86_USER)@$(LX86_HOST):$(LX86_DIR)
	rsh -l $(LX86_USER) $(LX86_HOST)      \
	    'cd $(LX86_DIR);'                 \
	    'gunzip -f src.tar.gz;'           \
	    'tar xvf src.tar;'                \
	    'chmod -R u+w *;'                 \
	    'touch src/*/*;'                  \
	    '$(LX86_MAKE) deps OS=LINUX_X86;' \
	    '$(LX86_MAKE) distrib_product OS=LINUX_X86'
	rcp $(LX86_USER)@$(LX86_HOST):$(LX86_DIR)/sac2c sac2c.for.LINUX_X86
	rsh -l $(LX86_USER) $(LX86_HOST)  \
	    'cd $(LX86_DIR)/src/runtime;' \
	    'tar cvfh libsac.tar libsac*.a'
	rcp $(LX86_USER)@$(LX86_HOST):$(LX86_DIR)/src/runtime/libsac.tar \
	    libsac.tar.for.LINUX_X86
	@ if [ $${?} -eq 0 ]; then                                 \
	    rsh -l $(LX86_USER) $(LX86_HOST) 'rm -rf $(LX86_DIR)'; \
	  else                                                     \
	    echo "Compilation of sac2c for LINUX_X86 failed!";     \
	    echo "Unable to create LINUX_X86 distribution !";      \
	    exit 1;                                                \
	  fi

sac2c.for.OSF_ALPHA:
	@echo "**************************************************************"
	@echo "*  Creating sac2c for OSF_ALPHA"
	@echo "**************************************************************"
	@ ping $(ALPHA_HOST) >/dev/null;                      \
          if [ $${?} -ne 0 ]; then                            \
            echo "Host $(ALPHA_HOST) is down !";              \
            echo "Unable to create OSF_ALPHA distribution !"; \
            exit 1;                                           \
          fi
	( cd $(SAC2CDIR); $(MAKE) tar )
	rsh -l $(ALPHA_USER) $(ALPHA_HOST) \
	    'rm -rf $(ALPHA_DIR); mkdir -p $(ALPHA_DIR)' 
	rcp $(SAC2CDIR)/src.tar.gz $(ALPHA_USER)@$(ALPHA_HOST):$(ALPHA_DIR)
	rsh -l $(ALPHA_USER) $(ALPHA_HOST)                               \
	    'export PATH=$$HOME/bin:$$PATH:/usr/local/bin:/usr/bin/X11;' \
	    'cd $(ALPHA_DIR);'                                           \
	    'gunzip -f src.tar.gz;'                                      \
	    'tar xvf src.tar;'                                           \
	    'chmod -R u+w *;'                                            \
	    'touch src/*/*;'                                             \
	    '$(ALPHA_MAKE) deps OS=OSF_ALPHA;'                           \
	    '$(ALPHA_MAKE) distrib_product OS=OSF_ALPHA'
	rcp $(ALPHA_USER)@$(ALPHA_HOST):$(ALPHA_DIR)/sac2c sac2c.for.OSF_ALPHA
	rsh -l $(ALPHA_USER) $(ALPHA_HOST) \
	    'cd $(ALPHA_DIR)/src/runtime;' \
	    'tar cvfh libsac.tar libsac*.a'
	rcp $(ALPHA_USER)@$(ALPHA_HOST):$(ALPHA_DIR)/src/runtime/libsac.tar \
	    libsac.tar.for.OSF_ALPHA
	@ if [ $${?} -eq 0 ]; then                                    \
	    rsh -l $(ALPHA_USER) $(ALPHA_HOST) 'rm -rf $(ALPHA_DIR)'; \
	  else                                                        \
	    echo "Compilation of sac2c for OSF_ALPHA failed!";        \
	    echo "Unable to create OSF_ALPHA distribution !";         \
	    exit 1;                                                   \
	  fi


libsac.tar.for.%:
	$(MAKE) sac2c.for.$*


sac2crc.for.%-2_5:
	$(MAKE) sac2crc.for.$*
	mv sac2crc.for.$* $@

sac2crc.for.%-2_7:
	$(MAKE) sac2crc.for.$*
	mv sac2crc.for.$* $@

sac2crc.for.%: $(RUNTIMEDIR)/sac2crc.%
	@echo "**************************************************************"
	@echo "*  Creating sac2crc for $*"
	@echo "**************************************************************"
	cp $< $@


runtime.tar:
	@echo "**************************************************************"
	@echo "*  Creating package $@"
	@echo "**************************************************************"
	( cd $(RUNTIMEDIR); $(MAKE) $@ )
	cp $(RUNTIMEDIR)/$@ .


stdlib.tar:
	@echo "**************************************************************"
	@echo "*  Creating package $@"
	@echo "**************************************************************"
	( cd $(STDLIBDIR); $(MAKE) tar )
	cp $(STDLIBDIR)/$@ .


Makefile.Config.for.%-2_5:
	$(MAKE) Makefile.Config.for.$*
	mv Makefile.Config.for.$* $@

Makefile.Config.for.%-2_7:
	$(MAKE) Makefile.Config.for.$*
	mv Makefile.Config.for.$* $@

Makefile.Config.for.%: $(STDLIBDIR)/Makefiles/Makefile.Config
	@echo "**************************************************************"
	@echo "*  Creating Makefile.Config for $*"
	@echo "**************************************************************"
	sed -e 's/^[ ]*OS[ ]*:=[ ]*[_A-Za-z0-9]*/OS := $*/g' $< > $@


demo.tar:
	@echo "**************************************************************"
	@echo "*  Creating package $@"
	@echo "**************************************************************"
	( cd $(DEMODIR); $(MAKE) tar TARFILENAME=$@ )
	cp $(DEMODIR)/$@ . 


check_existing_files:
	@ for file in $(CHECKFILES); do                        \
            if [ -f $${file} ]; then                           \
	      echo 'WARNING: '$${file}' is being reused!'; fi; \
          done


clean:
	rm -f $(BINDISTRIBS) $(SRCDISTRIBS) $(addsuffix .for.*,$(SPECIFICS)) \
	    $(INST_PACKAGES)
	rm -f *~ *.bak *.old

allclean: clean
	rm -f $(SAC2CDIR)/src.tar.gz
	rm -f $(RUNTIMEDIR)/runtime.tar
	rm -f $(STDLIBDIR)/stdlib.tar
	rm -f $(DEMODIR)/demo.tar


locker:
	locker $(SAC2CDIR)
	locker $(DEMOBASE)
	locker $(STDLIBDIR)

update: clean
	update $(SAC2CDIR)
	rmobsfiles $(SAC2CDIR)
	update $(DEMOBASE)
	rmobsfiles $(DEMOBASE)
	update $(STDLIBDIR)
	rmobsfiles $(STDLIBDIR)


new:
	rm -f $(BINDISTRIBS) $(SRCDISTRIBS) $(addsuffix .for.*,$(SPECIFICS))
	$(MAKE)

allnew: allclean
	$(MAKE)
