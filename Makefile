#
# $Log$
# Revision 2.19  2000/01/17 18:35:55  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 2.18  2000/01/04 11:48:52  cg
# Added recursive make call for new subdirectory src/heapmgr.
#
# Revision 2.17  1999/12/13 11:26:07  dkr
# *** empty log message ***
#
# Revision 2.16  1999/10/19 17:11:34  sbs
# new files in typecheck included and UNIX_ALPHA changed to OSF_ALPHA
#
# Revision 2.15  1999/09/15 16:26:35  sbs
# OSF_ALPHA as new OS added.
#
# Revision 2.14  1999/07/30 13:49:50  jhs
# Added concurrent_lib.o
#
# Revision 2.13  1999/07/27 08:33:19  jhs
# Added spmd_cons.o.
#
# Revision 2.12  1999/07/08 15:41:12  cg
# Sequence of src directories modifified in order to enable
# compilation after gmake clean.
#
# Revision 2.11  1999/07/08 12:45:46  cg
# Added making of new source directories src/tools and src/libsac.
#
# Revision 2.10  1999/06/25 14:49:54  jhs
# Added src/concurrent/spmd_trav.o to includes.
#
# ... [eliminated] 
#
# Revision 1.81  1998/05/13 07:12:25  cg
# added linking of file icm2c_mt.o
#




# CC=gcc -ansi -Wall -pedantic -g 
#
# -pedantic switched off to avoid those disgusting warnings about
# casts in lvalues (-erroff=E_CAST_DOESNT_YIELD_LVALUE for cc)
#

################################################################################
#
# C compiler setup:
#
# legal values for CC and CCPROD currently are:
#    gcc
#    cc
#
CC           := gcc
CCPROD       := gcc

#
# gcc specific flags:
#
gcc_FLAGS := -ansi -Wall 
gcc_PROD_FLAGS := -Wall -O3

#
# cc specific flags:
#
cc_FLAGS  := -erroff=E_CAST_DOESNT_YIELD_LVALUE
cc_PROD_FLAGS  := -erroff=E_CAST_DOESNT_YIELD_LVALUE -xO4

################################################################################
#
# OS setup:
#
# legal values for OS currently are:
#    SOLARIS_SPARC
#    LINUX_X86
#    OSF_ALPHA
#
OS        := SOLARIS_SPARC

#
# SOLARIS_SPARC specific flags and libraries:
#
#
# The switch __EXTENSIONS__ must be set under Solaris in order to avoid warnings
# when using non-ANSI-compliant functions.
# Currently, these are popen(), pclose(), tempnam(), and strdup().
#
SOLARIS_SPARC_FLAGS := -D__EXTENSIONS__
SOLARIS_SPARC_LIBS  := -ll

#
# LINUX_X86 specific flags and libraries:
#
LINUX_X86_FLAGS     := -D_POSIX_SOURCE
LINUX_X86_LIBS      := -lfl

#
# UNIX_ALPHA specific flags and libraries:
#
OSF_ALPHA_FLAGS    := 
OSF_ALPHA_LIBS     := -ll

################################################################################
#
# general setup:
#

CCFLAGS      := $($(CC)_FLAGS) -g $($(OS)_FLAGS)
CCPROD_FLAGS := $($(CCPROD)_PROD_FLAGS) $($(OS)_FLAGS) 

CFLAGS       := -DSHOW_MALLOC -DSAC_FOR_$(OS) 
CPROD_FLAGS  := -DDBUG_OFF -DPRODUCTION -DSAC_FOR_$(OS)

MAKE_NORM    :=$(MAKE) CC="$(CC)" CCFLAGS="$(CCFLAGS)" CFLAGS="$(CFLAGS)"
MAKE_PROD    :=$(MAKE) CC="$(CCPROD)" CCFLAGS="$(CCPROD_FLAGS)" CFLAGS="$(CPROD_FLAGS)"
MAKEFLAGS    += --no-print-directory

TAR          :=tar
LEX          :=lex
YACC         :=yacc -dv
LIBS         :=-lm $($(OS)_LIBS)
EFLIBS       :=-L/home/sacbase/efence -lefence
RM           :=rm -f
GZIP         :=gzip -f
ECHO         :=echo

LIB          :=lib/dbug.o lib/main_args.o
# /usr/lib/debug/malloc.o


#
# Collection of source files
#

SOURCE_DIRS  := . $(shell cat RCS-directories)
SOURCE_FILES := $(foreach dir,$(SOURCE_DIRS),$(addprefix $(dir)/,RCS-files $(shell (cd $(dir); cat RCS-files))))


#
# Collection of object files
#

GLOBAL= src/global/main.o src/global/Error.o src/global/usage.o \
        src/global/my_debug.o src/global/traverse.o  src/global/tree.o \
	src/global/tree_basic.o src/global/tree_compound.o \
        src/global/free.o src/global/internal_lib.o \
        src/global/globals.o src/global/resource.o src/global/build.o \
	src/global/interrupt.o src/global/options.o
SCANP= src/scanparse/y.tab.o src/scanparse/lex.yy.o \
       src/scanparse/scnprs.o
PRINT= src/print/print.o src/print/convert.o
FLATTEN= src/flatten/flatten.o
TYPECHECK= src/typecheck/typecheck.o src/typecheck/prim_fun.o \
           src/typecheck/typecheck_WL.o src/typecheck/gen_pseudo_fun.o \
           src/typecheck/new_typecheck.o src/typecheck/new_types.o \
           src/typecheck/shape.o src/typecheck/user_types.o \
           src/typecheck/constants.o src/typecheck/cv2cv.o \
           src/typecheck/cv2scalar.o
OPTIMIZE= src/optimize/optimize.o src/optimize/ConstantFolding.o \
          src/optimize/generatemasks.o src/optimize/DeadCodeRemoval.o \
          src/optimize/DeadFunctionRemoval.o src/optimize/freemasks.o \
	  src/optimize/LoopInvariantRemoval.o src/optimize/DupTree.o \
	  src/optimize/Inline.o src/optimize/Unroll.o src/optimize/WLUnroll.o \
          src/optimize/Unswitch.o src/optimize/CSE.o src/optimize/DataFlowMask.o
PSIOPT= src/psi-opt/index.o src/psi-opt/psi-opt.o src/psi-opt/ArrayElimination.o \
	src/psi-opt/wl_access_analyze.o src/psi-opt/tile_size_inference.o \
	src/psi-opt/WithloopFolding.o src/psi-opt/WLT.o src/psi-opt/WLI.o \
	src/psi-opt/WLF.o
MODULES= src/modules/filemgr.o src/modules/import.o src/modules/writesib.o  \
         src/modules/implicittypes.o src/modules/analysis.o \
         src/modules/checkdec.o src/modules/readsib.o \
         src/modules/cccall.o
OBJECTS= src/objects/objinit.o src/objects/objects.o \
         src/objects/uniquecheck.o src/objects/rmvoidfun.o
REFCOUNT= src/refcount/refcount.o
CONCURRENT= src/concurrent/concurrent.o src/concurrent/scheduling.o  \
            src/concurrent/spmd_init.o src/concurrent/spmd_opt.o     \
            src/concurrent/spmd_lift.o src/concurrent/sync_init.o    \
            src/concurrent/sync_opt.o src/concurrent/schedule.o      \
            src/concurrent/spmd_trav.o src/concurrent/spmd_cons.o    \
            src/concurrent/concurrent_lib.o
COMPILE=  src/compile/wltransform.o src/compile/wlpragma_funs.o \
          src/compile/precompile.o \
          src/compile/compile.o src/compile/gen_startup_code.o \
          src/compile/icm2c.o src/compile/icm2c_std.o src/compile/icm2c_mt.o \
          src/compile/icm2c_wl.o src/compile/ReuseWithArrays.o \
          src/compile/Old2NewWith.o

OBJ=$(GLOBAL) $(SCANP) $(PRINT) $(FLATTEN) $(TYPECHECK) $(OPTIMIZE) \
    $(MODULES) $(OBJECTS) $(REFCOUNT) $(COMPILE) $(PSIOPT) $(CONCURRENT)




#
#  Rules section
#

.PHONY: all efence product check_os dummy prod clean tar floppy distrib distrib_product fafnir

all: check_os dummy sac2c

efence: check_os dummy sac2c.efence

product: check_os clean prod sac2c.prod

distrib_product: prod sac2c.prod



check_os:
	@ if [ "$(OS)" != "SOLARIS_SPARC" -a "$(OS)" != "LINUX_X86" \
               -a "$(OS)" != "OSF_ALPHA" ]; \
	  then $(ECHO) "*** Unknown OS! Please specify:"; \
               $(ECHO) "SOLARIS_SPARC (default)"; \
               $(ECHO) "LINUX_X86"; \
               $(ECHO) "OSF_ALPHA"; \
	       exit 1; \
	  fi

dummy:
	(cd lib/src; $(MAKE_NORM) )
	(cd src/scanparse; $(MAKE_NORM) )
	(cd src/global; $(MAKE_NORM) )
	(cd src/print; $(MAKE_NORM) )
	(cd src/flatten; $(MAKE_NORM) )
	(cd src/typecheck; $(MAKE_NORM) )
	(cd src/optimize; $(MAKE_NORM) )
	(cd src/modules; $(MAKE_NORM) )
	(cd src/objects; $(MAKE_NORM) )
	(cd src/refcount; $(MAKE_NORM) )       
	(cd src/concurrent; $(MAKE_NORM) )
	(cd src/compile; $(MAKE_NORM) )
	(cd src/psi-opt; $(MAKE_NORM) )
	(cd src/libsac; $(MAKE_PROD) )
	(cd src/heapmgr; $(MAKE_PROD) )
	(cd src/runtime; $(MAKE_NORM) )
	(cd src/tools; $(MAKE_PROD) )
#
# $(MAKE_PROD) is used in the above lines by purpose in order to compile
# the SAC runtime library, the privat heap manager, and the additional 
# tools, e.g. the cache simulator, with full optimizations
# enabled even though sac2c itself is only compiled in the developper
# version.
#


prod:
	(cd lib/src; $(MAKE_PROD) )
	(cd src/scanparse; $(MAKE_PROD) )
	(cd src/global; $(MAKE_PROD) )
	(cd src/print; $(MAKE_PROD) )
	(cd src/flatten; $(MAKE_PROD) )
	(cd src/typecheck; $(MAKE_PROD) )
	(cd src/optimize; $(MAKE_PROD) )
	(cd src/modules; $(MAKE_PROD) )
	(cd src/objects; $(MAKE_PROD) )
	(cd src/refcount; $(MAKE_PROD) )       
	(cd src/concurrent; $(MAKE_PROD) )
	(cd src/compile; $(MAKE_PROD) )
	(cd src/psi-opt; $(MAKE_PROD) )
	(cd src/libsac; $(MAKE_PROD) )
	(cd src/heapmgr; $(MAKE_PROD) )
	(cd src/runtime; $(MAKE_PROD) )
	(cd src/tools; $(MAKE_PROD) )

sac2c: $(OBJ) $(LIB)
	$(CC) $(CCFLAGS) $(CFLAGS) -o sac2c $(OBJ) $(LIB) $(LIBS)

sac2c.efence: $(OBJ) $(LIB)
	$(CC) $(CCFLAGS) $(CFLAGS) -o sac2c.efence $(OBJ) $(LIB) $(LIBS) $(EFLIBS)

sac2c.prod:  $(OBJ) $(LIB)
	$(CCPROD) $(CCPROD_FLAGS) $(CPROD_FLAGS) -o sac2c $(OBJ) $(LIB) $(LIBS)

deps:
	(cd lib/src; $(MAKE) deps)
	(cd src/scanparse; $(MAKE) deps)
	(cd src/global; $(MAKE) deps)
	(cd src/print; $(MAKE) deps)
	(cd src/flatten; $(MAKE) deps)
	(cd src/typecheck; $(MAKE) deps)
	(cd src/optimize; $(MAKE) deps)
	(cd src/modules; $(MAKE) deps)
	(cd src/objects; $(MAKE) deps)
	(cd src/refcount; $(MAKE) deps)
	(cd src/concurrent; $(MAKE) deps)
	(cd src/compile; $(MAKE) deps)
	(cd src/psi-opt; $(MAKE) deps)
	(cd src/libsac; $(MAKE) deps)
	(cd src/heapmgr; $(MAKE) deps)
	(cd src/tools; $(MAKE) deps)
	(cd src/runtime; $(MAKE) deps)

clean:
	(cd lib/src; $(MAKE) clean)
	(cd src/scanparse; $(MAKE) clean)
	(cd src/global; $(MAKE) clean)
	(cd src/print; $(MAKE) clean)
	(cd src/flatten; $(MAKE) clean)
	(cd src/typecheck; $(MAKE) clean)
	(cd src/optimize; $(MAKE) clean)
	(cd src/modules; $(MAKE) clean)
	(cd src/objects; $(MAKE) clean)
	(cd src/refcount; $(MAKE) clean)
	(cd src/concurrent; $(MAKE) clean)
	(cd src/compile; $(MAKE) clean )
	(cd src/psi-opt; $(MAKE) clean)
	(cd src/libsac; $(MAKE) clean)
	(cd src/heapmgr; $(MAKE) clean)
	(cd src/tools; $(MAKE) clean)
	(cd src/runtime; $(MAKE) clean)
	$(RM) sac2c
	$(RM) sac2c.efence
	$(RM) -r .sb SunWS_cache
	$(RM) src.tar.gz

clean_sb:
	$(RM) -r .sb SunWS_cache
	$(RM) -r src/*/.sb src/*/SunWS_cache

floppy: src.tar.gz
	$(TAR) -cvf /dev/rfd0c src.tar.gz

tar: src.tar.gz

src.tar.gz: $(SOURCE_FILES)
	$(TAR) -cvf src.tar $(SOURCE_FILES)
	$(GZIP) src.tar

distrib:
	(cd distrib/src; $(MAKE))

tags: 
	ctags src/*/*.[ch] >/dev/null


FAFNIR_USER  = sac
FAFNIR_DIR   = sac2c

fafnir: src.tar.gz
	@ ping $@ >/dev/null; \
          if [ $${?} -ne 0 ]; then \
            echo "Host $@ is down !"; \
            exit 1; \
          fi
	rsh -l $(FAFNIR_USER) $@ mkdir -p $(FAFNIR_DIR)
	rcp src.tar.gz sac@$@:$(FAFNIR_DIR)
	rsh -l $(FAFNIR_USER) $@  \
            'cd $(FAFNIR_DIR);' \
            'rm -f $(SOURCE_FILES);' \
            'gunzip -f src.tar.gz;' \
            'tar xvf src.tar;' \
            'chmod 644 $(SOURCE_FILES);' \
            'make deps OS=LINUX_X86;' \
            'make OS=LINUX_X86'

