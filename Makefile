
#
# $Id$
#
#

#######################################################################################
#
# include general setup:
#

PROJECT_ROOT := .

include $(PROJECT_ROOT)/Makefile.Config
include $(PROJECT_ROOT)/Makefile.Targets

LINK := $(foreach dir,$(SOURCE_DIRS),$(addprefix src/$(dir)/,$($(dir))))

LINK_PROD := $(patsubst .o,.prod.o,$(LINK))

SOURCE_DIRS_DEVEL    := $(addsuffix .devel,$(SOURCE_DIRS))
SOURCE_DIRS_PROD     := $(addsuffix .prod,$(SOURCE_DIRS))
SOURCE_DIRS_CLEAN    := $(addsuffix .clean,$(SOURCE_DIRS))

SOURCE_MAKEFILES := $(addprefix src/,$(addsuffix /Makefile,$(SOURCE_DIRS)))

LIB          := lib/dbug.o lib/main_args.o


#
#  Rules section
#

.PHONY: all efence product check_os maketools makefiles prod clean libsac libsac2c heapmgr \
        distrib ctags runtime tools lib

all: devel

devel: check_os lib maketools makefiles sac2c libsac heapmgr runtime tools

prod: check_os lib maketools makefiles sac2c.prod libsac heapmgr runtime tools 

efence: check_os maketools sac2c.efence


check_os:
	@ if [ "$(OS)" = "" -o "$(ARCH)" = "" ]; \
	  then $(ECHO) "*** Unknown OS or unknown ARCH! Please specify!"; \
	       exit 1; \
	  fi
	@ $(ECHO)
	@ $(ECHO) "Building for $(OS) on $(ARCH).";

maketools:
	$(MAKE) -C tools
	$(CLOCK_SKEW_ELIMINATION) Makefile.Config
	$(CLOCK_SKEW_ELIMINATION) src/global/config.h

makefiles: $(SOURCE_MAKEFILES)

src/%/Makefile: Makefile.Source
	@echo "Creating makefile: $@"
	@cp $< $@

sac2c: $(SOURCE_DIRS_DEVEL)
	@echo ""
	@echo "Linking sac2c (developer version)"
	@$(LIBTOOL) $(CC) $(CCLINKFLAGS) -o $@ $(LINK) $(LIB) $(LIBS) $(LDDYNFLAG)


%.devel:
	@echo ""
	@echo "Making subdirectory $* (developer version)"
	@$(MAKE) TARGET="$*" CHECK_DEPS="yes" -C src/$* make_devel 


sac2c.prod: $(SOURCE_DIRS_PROD)
	@echo ""
	@echo "Linking sac2c (product version)"
	@$(LIBTOOL) $(CC) $(CCLINKFLAGS) -o $@ $(LINK) $(LIB) $(LIBS) $(LDDYNFLAG)

%.prod:
	@echo ""
	@echo "Making subdirectory $* (product version)"
	@$(MAKE) TARGET="$*" CHECK_DEPS="yes" -C src/$* make_prod


sac2c.efence: $(SOURCE_DIRS_DEVEL)
	@echo ""
	@echo "Linking sac2c (efence version)"
	@$(LIBTOOL) $(CC) $(CCLINKFLAGS) -o sac2c.efence $(LINK) $(LIBS) $(EFLIBS) $(LDDYNFLAG)

doxygen:
	doxygen sac2cdoxy

clean: $(SOURCE_DIRS_CLEAN)
	$(RM) sac2c
	$(RM) sac2c.efence
	$(RM) -r .sb SunWS_cache
	$(RM) src.tar.gz
	$(MAKE) -C tools clean
	$(MAKE) -C src/libsac clean
	$(MAKE) -C src/heapmgr clean
	$(MAKE) -C src/runtime clean
	$(MAKE) -C src/tools clean

%.clean:
	@$(ECHO) "Cleaning directory $*"
	@$(MAKE) CHECK_DEPS="no" -C src/$* clean


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


