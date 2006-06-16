
#
# $Id$
#
#

#######################################################################################
#
# general setup:
#

PROJECT_ROOT := .

include $(PROJECT_ROOT)/Makefile.Config
include $(PROJECT_ROOT)/Makefile.Targets

ALL_SOURCE_DIRS = $(addprefix src/,$(src))
SOURCE_DIRS     = $(addprefix src/,$(TARGET))

TARGETS_DEVEL = $(addprefix src/,$(foreach target,$(TARGET),$(addprefix $(target)/,$($(target)))))
TARGETS_PROD  = $(patsubst .o,.prod.o,$(TARGETS_DEVEL))

ifeq ($(MODE),prod)
  TARGETS = $(TARGETS_PROD)
else
  TARGETS = $(TARGETS_DEVEL)
endif

SOURCE_MAKEFILES = $(addsuffix /Makefile,$(ALL_SOURCE_DIRS))

LIB          = lib/dbug.o lib/main_args.o
INCS         = -Iinc $(patsubst %,-I%,$(ALL_SOURCE_DIRS))

REVISION = $(shell svn info | grep Revision: | sed -e 's/Revision: //g')

XML_DIR     = $(PROJECT_ROOT)/src/xml
XML_COMMONS = $(wildcard $(XML_DIR)/common-*.xsl)

GENERATED_INCLUDE_FILES = $(patsubst %.xsl,%,$(foreach dir,$(SOURCE_DIRS),$(wildcard $(dir)/*.h.xsl))) \
                          $(patsubst %.xsl,%,$(foreach dir,$(SOURCE_DIRS),$(wildcard $(dir)/*.mac.xsl))) \
                          $(patsubst %.y,%.tab.h,$(foreach dir,$(SOURCE_DIRS),$(wildcard $(dir)/*.y)))

GENERATED_SOURCE_FILES = $(patsubst %.xsl,%,$(foreach dir,$(SOURCE_DIRS),$(wildcard $(dir)/*.c.xsl))) \
                         $(patsubst %.y,%.tab.c,$(foreach dir,$(SOURCE_DIRS),$(wildcard $(dir)/*.y))) \
                         $(patsubst %.l,%.lex.c,$(foreach dir,$(SOURCE_DIRS),$(wildcard $(dir)/*.l))) \

GENERATED_FILES = $(GENERATED_INCLUDE_FILES) $(GENERATED_SOURCE_FILES)

DEPENDENCY_FILES = $(patsubst %.o,%.d,$(TARGETS_DEVEL))

DEPS = $(foreach file,$(DEPENDENCY_FILES),$(dir $(file)).$(notdir $(file)))


  
###############################################################################
#
# Dummy rules
#

.PHONY: clean %.clean all devel prod %.track
.PHONY: efence check_os maketools makefiles libsac libsac2c heapmgr 
.PHONY: distrib ctags runtime tools lib %.go makesubdir xml

.PRECIOUS: %.c %.h %.o %.prod.o .%.d %.c %.mac %.lex.c %.tab.c %.tab.h



###############################################################################
#
# Start rules
#

all: devel

devel: check_os lib maketools makefiles xml sac2c.go libsac heapmgr runtime tools

prod: check_os lib maketools makefiles xml sac2c.prod.go libsac heapmgr runtime tools 

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

makefiles: $(SOURCE_MAKEFILES)

xml:
	$(MAKE) -C src/xml

src/%/Makefile: Makefile.Source
	@$(ECHO) "Creating makefile: $@"
	@cp -f $< $@

%.go: 
	@$(ECHO) ""
	@$(ECHO) "************************************"
	@$(ECHO) "* Making $*"
	@$(ECHO) "************************************"
	@touch make_track
	@$(MAKE) CHECK_DEPS="yes" MODE="$(suffix $*)" TARGET="$(src)" $*

sac2c.prod sac2c: src/global/build.o
	@$(ECHO) ""
	@$(ECHO) "Linking sac2c (developer version)"
	@$(LIBTOOL) $(CC) $(CCLINKFLAGS) -o $@ $(TARGETS) $< $(LIB) $(LIBS) $(LDDYNFLAG)
	@$(RM) make_track

src/global/build.c: $(TARGETS)
	@$(ECHO) ""
	@$(ECHO) "Creating revision information file:  $@"
	@$(ECHO) "char build_date[] = \"`date`\";"  >  $@
	@$(ECHO) "char build_user[] = \"$(USER)\";" >> $@
	@$(ECHO) "char build_host[] = \"`hostname`\";" >> $@
	@$(ECHO) "char build_os[]   = \"$(OS)\";"   >> $@
	@$(ECHO) "char build_rev[]  = \"$(REVISION)\";"  >> $@
	@$(ECHO) "char build_ast[]  = \"`$(XSLTENGINE) src/xml/ast2fingerprint.xsl src/xml/ast.xml | $(FINGERPRINTER)`\";" >> $@
	@$(CLOCK_SKEW_ELIMINATION) $@ 
	@$(ECHO) "$(dir $@)" > make_track


sac2c.efence: $(TARGETS_DEVEL)
	@$(ECHO) ""
	@$(ECHO) "Linking sac2c (efence version)"
	@$(LIBTOOL) $(CC) $(CCLINKFLAGS) -o sac2c.efence $(TARGETS_DEVEL) $(LIBS) $(EFLIBS) $(LDDYNFLAG)

doxygen:
	doxygen sac2cdoxy



###############################################################################
#
# Rules for making subdirectories
#

makesubdir: $(TARGETS)
	@$(ECHO) ""


###############################################################################
#
# Rules for cleaning directories
#

clean: makefiles $(addsuffix .clean,$(ALL_SOURCE_DIRS))
	$(RM) sac2c
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
	@$(MAKE) CHECK_DEPS="no" -C $* clean


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



###############################################################################
#
# Pattern rules for compilation
#

%.prod.o: %.c %.maketrack.comp
	@$(ECHO) "  Compiling product code:  $(notdir $<)"
	@$(CCPROD) $(CCPROD_FLAGS) $(CPROD_FLAGS) $(YYFLAGS) $(INCS) -o $@ -c $<
	@$(CLOCK_SKEW_ELIMINATION) $@


%.o: %.c 
	@if [ ! -f make_track -o "$(dir $*)" != "`cat make_track`" ] ; \
         then $(ECHO) "$(dir $*)" > make_track; \
              $(ECHO) ""; \
              $(ECHO) "Compiling files in directory $(dir $@)" ; \
         fi
	@$(ECHO) "  Compiling developer code:  $(notdir $<)"
	@$(CC) $(CCFLAGS) $(CFLAGS) $(YYFLAGS) $(INCS) -o $@ -c $<
	@$(CLOCK_SKEW_ELIMINATION) $@



###############################################################################
#
# Pattern rules for source code generation
#

%.h: %.h.xsl $(XML_DIR)/ast.xml $(XML_COMMONS) %.track
	@$(ECHO) "  Generating header file from XML specification:  $(notdir $@)"
	@$(XSLTENGINE) $< $(XML_DIR)/ast.xml | $(CODE_BEAUTIFIER) >$@

%.mac: %.mac.xsl $(XML_DIR)/ast.xml $(XML_COMMONS) %.track
	@$(ECHO) "  Generating macro file from XML specification:  $(notdir $@)"
	@$(XSLTENGINE) $< $(XML_DIR)/ast.xml | $(CODE_BEAUTIFIER) >$@

%.c: %.c.xsl $(XML_DIR)/ast.xml $(XML_COMMONS) %.track
	@$(ECHO) "  Generating source code from XML specification:  $(notdir $@)"
	@$(XSLTENGINE) $< $(XML_DIR)/ast.xml | $(CODE_BEAUTIFIER) >$@


%.lex.c: %.l %.track
	@$(ECHO) "  Generating source code from LEX specification:  $(notdir $<)"
	@$(LEX) $<
	@mv lex.yy.c $@
	@$(CLOCK_SKEW_ELIMINATION) $@

%.tab.c: %.y %.track
	@$(ECHO) "  Generating source code from YACC specification:  $(notdir $<)"
	@$(YACC) $<
	@mv y.tab.c $@
	@$(RM) y.tab.h
	@mv y.output $(dir $@)
	@$(CLOCK_SKEW_ELIMINATION) $@

%.tab.h: %.y %.track
	@$(ECHO) "  Generating header file from YACC specification:  $(notdir $<)"
	@$(YACC) $<
	@mv y.tab.h $@
	@$(RM) y.tab.c 
	@mv y.output $(dir $@)
	@$(CLOCK_SKEW_ELIMINATION) $@

%.track: 
	@if [ ! -f make_track -o "$(dir $*)" != "`cat make_track`" ] ; \
         then $(ECHO) "$(dir $*)" > make_track; \
              $(ECHO) ""; \
              $(ECHO) "Generating files in directory $(dir $@)" ; \
         fi



#######################################################################################
#
# Pattern rules for dependency tracking mechanism:
#

.%.d: %.c $(GENERATED_FILES) 
	@if [ ! -f make_track -o "$(dir $*)" != "`cat make_track`" ] ; \
        then $(ECHO) "$(dir $*)" > make_track; \
             $(ECHO) ""; \
             $(ECHO) "Checking dependencies in directory $(dir $@)" ; \
        fi
	@$(ECHO) "  Checking dependencies of source file: $(notdir $<)"
	@if $(CC) $(CCDEPS_FLAGS) $(CFLAGS) $(INCS) $<  > $@d ; \
	 then sed 's/\($(notdir $*)\)\.o[ :]*/$(subst /,\/,$*)\.o $(subst /,\/,$@)\: $$\(PROJECT_ROOT\)\/Makefile.Config /'  <$@d >$@; \
	      $(RM) $@d ; \
	 else $(RM) $@d ; \
	      exit 1 ;  \
	 fi
	@$(CLOCK_SKEW_ELIMINATION) $@





###############################################################################
#
# Includes for dependency tracking mechanism
#

ifeq ($(CHECK_DEPS),yes)
  ifneq ($(DEPS),)
    -include $(sort $(DEPS))
  endif
endif
