#
# $Log$
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

VERSION    =  0_8


#
#  Settings for file and package sets
#

SYSTEMS    =  SOLARIS_SPARC LINUX_X86
PACKAGES   =  runtime.tar stdlib.tar demo.tar
FILES      =  install_sac LICENSE README
SPECIFICS  =  sac2c libsac.a sac2crc


#
#  Settings for source directories
#

SAC2CDIR   =  ../..
RUNTIMEDIR =  $(SAC2CDIR)/src/runtime
STDLIBDIR  =  $(SACBASE)/stdlib
DEMODIR    =  $(SACBASE)/demo


#
#  Settings for target directories
#

BINTARGETDIR  =  ../binpacks
SRCTARGETDIR  =  ../srcpacks



#
#  Settings for system SOLARIS_SPARC
#

SOLARIS_DIR   =  /var/tmp/$(USER)_sac_distrib


#
#  Settings for system LINUX_X86
#

LX86_HOST  =  fafnir
LX86_USER  =  sac
LX86_DIR   =  sac_distrib
LX86_MAKE  =  make


#
#  Internal settings
#

DATE          :=  $(shell date +%d_%b_%y)
BINDISTRIBS   :=  $(addprefix $(BINTARGETDIR)/SAC-$(VERSION)-,$(addsuffix -$(DATE).tar.gz,$(SYSTEMS))) 
SRCDISTRIBS   :=  $(SRCTARGETDIR)/SAC-$(VERSION)-$(DATE).tar.gz
CHECKFILES    :=  $(PACKAGES) $(foreach system,$(SYSTEMS), $(addsuffix .for.$(system),$(SPECIFICS)))



.PHONY: all linux solaris source clean update new allnew check_existing_files 


all: check_existing_files $(BINDISTRIBS) $(SRCDISTRIBS)

linux: check_existing_files $(BINTARGETDIR)/SAC-$(VERSION)-LINUX_X86-$(DATE).tar.gz

solaris: check_existing_files $(BINTARGETDIR)/SAC-$(VERSION)-SOLARIS_SPARC-$(DATE).tar.gz

source: check_existing_files $(SRCTARGETDIR)/SAC-$(VERSION)-$(DATE).tar.gz

$(BINTARGETDIR)/SAC-$(VERSION)-%-$(DATE).tar.gz: \
   $(PACKAGES) $(FILES) $(addsuffix .for.%,$(SPECIFICS))
	@echo "**************************************************************"
	@echo "*  Creating SAC binary distribution for $*"
	@echo "**************************************************************"
	rm -f $(SPECIFICS)
	for file in $(SPECIFICS); do ln -s $${file}.for.$* $${file}; done
	tar cvfh $(BINTARGETDIR)/SAC-$(VERSION)-$*-$(DATE).tar $(SPECIFICS) $(PACKAGES) $(FILES)
	gzip -f $(BINTARGETDIR)/SAC-$(VERSION)-$*-$(DATE).tar
	rm -f $(SPECIFICS)

sac2c.for.SOLARIS_SPARC libsac.a.for.SOLARIS_SPARC: 
	@echo "**************************************************************"
	@echo "*  Creating sac2c for SOLARIS_SPARC"
	@echo "**************************************************************"
	(cd $(SAC2CDIR); $(MAKE) tar) 
	rm -rf $(SOLARIS_DIR)
	mkdir -p $(SOLARIS_DIR)
	cp $(SAC2CDIR)/src.tar.gz $(SOLARIS_DIR)
	(cd $(SOLARIS_DIR); gunzip -f src.tar.gz)
	(cd $(SOLARIS_DIR); tar xvf src.tar)
	(cd $(SOLARIS_DIR); $(MAKE) distrib_product OS=SOLARIS_SPARC)
	cp $(SOLARIS_DIR)/sac2c sac2c.for.SOLARIS_SPARC
	cp $(SOLARIS_DIR)/src/runtime/libsac.a libsac.a.for.SOLARIS_SPARC
	@ if [ $${?} -eq 0 ]; then  \
            rm -rf $(SOLARIS_DIR); \
          else \
            echo "Compilation of sac2c for SOLARIS_SPARC failed!"; \
            echo "Unable to create SOLARIS_SPARC distribution !"; \
            exit 1; \
          fi

sac2c.for.LINUX_X86 libsac.a.for.LINUX_X86: 
	@echo "**************************************************************"
	@echo "*  Creating sac2c for LINUX_X86"
	@echo "**************************************************************"
	@ ping $(LX86_HOST) >/dev/null; \
          if [ $${?} -ne 0 ]; then \
            echo "Host $(LX86_HOST) is down !"; \
            echo "Unable to create LINUX_X86 distribution !"; \
            exit 1; \
          fi
	(cd $(SAC2CDIR); $(MAKE) tar) 
	rsh -l $(LX86_USER) $(LX86_HOST) 'rm -rf $(LX86_DIR); mkdir -p $(LX86_DIR)' 
	rcp $(SAC2CDIR)/src.tar.gz $(LX86_USER)@$(LX86_HOST):$(LX86_DIR)
	rsh -l $(LX86_USER) $(LX86_HOST) \
	    'cd $(LX86_DIR);'  \
	    'gunzip -f src.tar.gz;'  \
	    'tar xvf src.tar;'  \
	    '$(LX86_MAKE) deps;' \
	    '$(LX86_MAKE) distrib_product OS=LINUX_X86'
	rcp $(LX86_USER)@$(LX86_HOST):$(LX86_DIR)/sac2c ./sac2c.for.LINUX_X86
	rcp $(LX86_USER)@$(LX86_HOST):$(LX86_DIR)/src/runtime/libsac.a ./libsac.a.for.LINUX_X86
	@ if [ $${?} -eq 0 ]; then  \
            rsh -l $(LX86_USER) $(LX86_HOST) 'rm -rf $(LX86_DIR)'; \
          else \
            echo "Compilation of sac2c for LINUX_X86 failed!"; \
            echo "Unable to create LINUX_X86 distribution !"; \
            exit 1; \
          fi

$(SRCTARGETDIR)/SAC-$(VERSION)-$(DATE).tar.gz: $(PACKAGES) $(FILES) src.tar.gz
	@echo "**************************************************************"
	@echo "*  Creating SAC source distribution"
	@echo "**************************************************************"
	tar cvfh $(SRCTARGETDIR)/SAC-$(VERSION)-$(DATE).tar $^
	gzip -f $(SRCTARGETDIR)/SAC-$(VERSION)-$(DATE).tar
	rm -f src.tar.gz


sac2crc.for.%: $(RUNTIMEDIR)/sac2crc.%
	cp $< $@


src.tar.gz:
	(cd $(SAC2CDIR); $(MAKE) tar)
	ln -s $(SAC2CDIR)/src.tar.gz


runtime.tar: 
	@echo "**************************************************************"
	@echo "*  Creating package runtime"
	@echo "**************************************************************"
	(cd $(RUNTIMEDIR); $(MAKE) runtime.tar)
	cp $(RUNTIMEDIR)/runtime.tar .


stdlib.tar:
	@echo "**************************************************************"
	@echo "*  Creating package stdlib"
	@echo "**************************************************************"
	( cd $(STDLIBDIR); $(MAKE) tar)
	cp $(STDLIBDIR)/stdlib.tar .

demo.tar:
	@echo "**************************************************************"
	@echo "*  Creating package demo"
	@echo "**************************************************************"
	( cd $(DEMODIR); $(MAKE) tar TARFILENAME=demo.tar)
	cp $(DEMODIR)/demo.tar . 


check_existing_files:
	@ for file in $(CHECKFILES); do  \
            if [ -f $${file} ]; then echo 'WARNING: '$${file}' is being reused!'; fi ;\
          done


clean:
	rm -f $(BINDISTRIBS) $(SRCDISTRIBS) $(addsuffix .for.*,$(SPECIFICS)) $(PACKAGES)
	rm -f *~ *.bak *.old

update: clean
	update $(SAC2CDIR)
	update $(DEMODIR)
	update $(STDLIBDIR)

new:
	rm -f $(BINDISTRIBS) $(SRCDISTRIBS) $(addsuffix .for.*,$(SPECIFICS))
	$(MAKE)

allnew:
	$(MAKE) clean
	$(MAKE)
