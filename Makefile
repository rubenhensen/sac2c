
#
# $Id$
#
#


###############################################################################
#
# Calling conventions:
#
#  Start rules: 
#    devel   compile developer code (default)
#    prod    compile product code
#    efence  compile efence debugging code
#    clean   cleanup derived files
#
#  Parameters:
#    CHECK_DEPS="no"  de-activate dependency checking meachanism
#    HIDE=""          show important commands issued by make (debugging)
#
###############################################################################


#######################################################################################
#
# general setup:
#

PROJECT_ROOT := .

HIDE := @
CHECK_DEPS := yes

include $(PROJECT_ROOT)/Makefile.Config
include $(PROJECT_ROOT)/Makefile.Targets

ALL_SOURCE_DIRS  := $(addprefix src/,$(src))
SOURCE_MAKEFILES := $(addsuffix /Makefile,$(ALL_SOURCE_DIRS))


  
###############################################################################
#
# Dummy rules
#

.PHONY: clean %.clean cleanprod %.cleanprod all devel prod %.track
.PHONY: efence check_os maketools makefiles libsac libsac2c heapmgr 
.PHONY: distrib ctags runtime tools lib  makesubdir xml


.PRECIOUS: %.c %.h %.o %.prod.o .%.d %.c %.mac %.lex.c %.tab.c %.tab.h



###############################################################################
#
# Start rules
#

all: devel

devel: check_os lib maketools makefiles xml make_sac2c libsac heapmgr runtime tools

prod: check_os lib maketools makefiles xml make_sac2c.prod libsac heapmgr runtime tools 

efence: check_os lib maketools makefiles xml make_sac2c.efence libsac heapmgr runtime tools 



###############################################################################
#
# Auxiliary rules
#

check_os:
	@ if [ "$(OS)" = "" -o "$(ARCH)" = "" ]; \
	  then $(ECHO) "*** Unknown OS or unknown ARCH! Please specify!"; \
	       exit 1; \
	  fi
	@ $(ECHO)
	@ $(ECHO) "Building for $(OS) on $(ARCH).";

maketools:
	$(MAKE) -C tools

makefiles: $(SOURCE_MAKEFILES)

xml:
	$(MAKE) -C src/xml

src/%/Makefile: Makefile.Source
	@$(ECHO) "Creating makefile: $@"
	@cp -f $< $@

make_%: 
	@$(ECHO) ""
	@$(ECHO) "************************************"
	@$(ECHO) "* Making $*"
	@$(ECHO) "************************************"
	@touch make_track
	@$(MAKE) -f Makefile.Sac2c CHECK_DEPS="$(CHECK_DEPS)" \
         HIDE="$(HIDE)" TARGET="$(src)" $*



doxygen:
	doxygen sac2cdoxy




###############################################################################
#
# Rules for cleaning directories
#

clean: makefiles $(addsuffix .clean,$(ALL_SOURCE_DIRS))
	$(RM) sac2c
	$(RM) sac2c.prod
	$(RM) sac2c.efence
	$(RM) -r .sb SunWS_cache
	$(RM) src.tar.gz src/global/build.c
	$(MAKE) -C tools clean
	$(MAKE) -C src/libsac clean
	$(MAKE) -C src/heapmgr clean
	$(MAKE) -C src/runtime clean
	$(MAKE) -C src/tools clean

%.clean:
	@$(ECHO) "Cleaning directory $*"
	@$(MAKE) -C $* clean


cleanprod: $(addsuffix .cleanprod,$(ALL_SOURCE_DIRS))

%.cleanprod:
	@$(ECHO) "Cleaning directory $* (product sources)"
	@$(MAKE) -C $* cleanprod


###############################################################################
#
# Rules for old style recursive make invocations for non-sac2c stuff
#

lib: 
	$(MAKE) CHECK_DEPS="no" -C lib/src

libsac: 
	$(MAKE) CHECK_DEPS="no" -C src/libsac

heapmgr:
ifneq ($(DISABLE_PHM),yes)
	$(MAKE) CHECK_DEPS="no" -C src/heapmgr
endif

runtime: 
	$(MAKE) CHECK_DEPS="no" -C src/runtime

tools: 
	$(MAKE) CHECK_DEPS="no" -C src/tools

libsac2c: 
	$(MAKE) -C src/libsac libsac2c.so

distrib:
	(cd distrib/src; $(MAKE))

ctags: 
	ctags src/*/*.[ch] >/dev/null

$(PROJECT_ROOT)/Makefile.Config: $(PROJECT_ROOT)/Makefile.Config.in
	(cd $(PROJECT_ROOT); ./configure)

configure: configure.ac
	svn lock configure src/global/config.h.in
	autoconf
	autoheader
	svn commit configure src/global/config.h.in



