#
#
# $Log$
# Revision 3.5  2000/12/13 11:11:57  sbs
# -D_SVID_SOURCE added when compiling on linux; this makes several
# system functions available such as popen, tempnam, or sbrk.
#
# Revision 3.4  2000/12/08 10:43:01  cg
# Option "-w" removed again.
#
# Revision 3.3  2000/12/06 19:51:22  dkr
# src/flatten/infer_dfms.o added
#
# Revision 3.2  2000/12/06 17:39:57  cg
# Added new gcc option "-w" in order to disable nasty compiler warnings
# with respect to missing braces in the static initialization of mutex
# locks.
#
# Revision 3.1  2000/11/20 17:59:10  sacbase
# new release made
#
#
# ... [eliminated] 
#
# Revision 1.81  1998/05/13 07:12:25  cg
# added linking of file icm2c_mt.o
#
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
gcc_FLAGS      := -ansi -Wall -g
gcc_PROD_FLAGS := -ansi -Wall -O3

#
# cc specific flags:
#
cc_FLAGS      := -erroff=E_CAST_DOESNT_YIELD_LVALUE -g
cc_PROD_FLAGS := -erroff=E_CAST_DOESNT_YIELD_LVALUE -g -xO4

################################################################################
#
# OS setup:
#
# legal values for OS currently are:
#    SOLARIS_SPARC
#    LINUX_X86
#    OSF_ALPHA
#
# to allow easy changes: 
# do not delete os lines, but "comment" all not used lines
#
OS        := SOLARIS_SPARC
#OS        := LINUX_X86
#OS        := OSF_ALPHA

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
LINUX_X86_FLAGS     := -D_POSIX_SOURCE -D_SVID_SOURCE
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

CCFLAGS      := $($(CC)_FLAGS) $($(OS)_FLAGS)
CCPROD_FLAGS := $($(CCPROD)_PROD_FLAGS) $($(OS)_FLAGS) 

CFLAGS       := -DSHOW_MALLOC -DSAC_FOR_$(OS)
CPROD_FLAGS  := -DDBUG_OFF -DPRODUCTION -DSAC_FOR_$(OS)

MAKE_NORM    := $(MAKE) CC="$(CC)" CCFLAGS="$(CCFLAGS)" CFLAGS="$(CFLAGS)" OS="$(OS)"
MAKE_PROD    := $(MAKE) CC="$(CCPROD)" CCFLAGS="$(CCPROD_FLAGS)" CFLAGS="$(CPROD_FLAGS)" OS="$(OS)"
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
SOURCE_FILES := $(foreach dir,$(SOURCE_DIRS),$(addprefix $(dir)/,$(filter-out RCS-directories,$(shell (cd $(dir); cat RCS-files)))))


#
# Collection of object files
#

GLOBAL= src/global/main.o src/global/Error.o src/global/usage.o \
        src/global/my_debug.o src/global/internal_lib.o src/global/globals.o \
        src/global/resource.o src/global/build.o src/global/interrupt.o \
        src/global/options.o src/global/NameTuples.o
TREE= src/tree/traverse.o src/tree/tree.o src/tree/tree_basic.o src/tree/free.o \
      src/tree/tree_compound.o src/tree/DupTree.o src/tree/LookUpTable.o \
      src/tree/DataFlowMask.o src/tree/DataFlowMaskUtils.o \
      src/tree/scheduling.o
SCANP= src/scanparse/y.tab.o src/scanparse/lex.yy.o \
       src/scanparse/scnprs.o
PRINT= src/print/print.o src/print/convert.o

FLATTEN= src/flatten/flatten.o \
         src/flatten/lac2fun.o \
         src/flatten/infer_dfms.o src/flatten/cleanup_decls.o \
         src/flatten/fun2lac.o src/flatten/adjust_ids.o
TYPECHECK= src/typecheck/typecheck.o src/typecheck/prim_fun.o \
           src/typecheck/typecheck_WL.o src/typecheck/gen_pseudo_fun.o \
           src/typecheck/new_typecheck.o src/typecheck/new_types.o \
           src/typecheck/shape.o src/typecheck/user_types.o \
           src/typecheck/constants.o src/typecheck/cv2cv.o \
           src/typecheck/cv2scalar.o
OPTIMIZE= src/optimize/optimize.o src/optimize/ConstantFolding.o \
          src/optimize/generatemasks.o src/optimize/DeadCodeRemoval.o \
          src/optimize/DeadFunctionRemoval.o src/optimize/freemasks.o \
	  src/optimize/LoopInvariantRemoval.o src/optimize/Inline.o \
          src/optimize/Unroll.o src/optimize/WLUnroll.o src/optimize/Unswitch.o \
          src/optimize/CSE.o
PSIOPT= src/psi-opt/index.o src/psi-opt/ArrayElimination.o \
	src/psi-opt/wl_access_analyze.o src/psi-opt/tile_size_inference.o \
	src/psi-opt/WithloopFolding.o src/psi-opt/WLT.o src/psi-opt/WLI.o \
	src/psi-opt/WLF.o \
	src/psi-opt/pad.o src/psi-opt/pad_collect.o src/psi-opt/pad_infer.o \
	src/psi-opt/pad_transform.o src/psi-opt/pad_info.o
MODULES= src/modules/filemgr.o src/modules/import.o src/modules/writesib.o  \
         src/modules/implicittypes.o src/modules/analysis.o \
         src/modules/checkdec.o src/modules/readsib.o \
         src/modules/cccall.o
OBJECTS= src/objects/objinit.o src/objects/objects.o \
         src/objects/uniquecheck.o src/objects/rmvoidfun.o
REFCOUNT= src/refcount/refcount.o
CONCURRENT= src/concurrent/concurrent.o \
            src/concurrent/spmd_init.o src/concurrent/spmd_opt.o     \
            src/concurrent/spmd_lift.o src/concurrent/sync_init.o    \
            src/concurrent/sync_opt.o src/concurrent/schedule.o      \
            src/concurrent/spmd_trav.o src/concurrent/spmd_cons.o    \
            src/concurrent/concurrent_lib.o
MULTITHREAD= src/multithread/multithread.o src/multithread/schedule_init.o \
             src/multithread/repfuns_init.o src/multithread/blocks_init.o \
             src/multithread/blocks_expand.o src/multithread/multithread_lib.o \
             src/multithread/mtfuns_init.o src/multithread/blocks_cons.o \
             src/multithread/blocks_propagate.o \
             src/multithread/dataflow_analysis.o \
             src/multithread/barriers_init.o src/multithread/blocks_lift.o \
             src/multithread/adjust_calls.o
COMPILE=  src/compile/wltransform.o src/compile/wlpragma_funs.o \
          src/compile/precompile.o \
          src/compile/compile.o src/compile/gen_startup_code.o \
          src/compile/icm2c.o src/compile/icm2c_std.o src/compile/icm2c_mt.o \
          src/compile/icm2c_wl.o src/compile/ReuseWithArrays.o \
          src/compile/PatchWith.o

CINTERFACE= src/c-interface/map_cwrapper.o src/c-interface/print_interface.o \
            src/c-interface/import_specialization.o \
            src/c-interface/print_interface_header.o \
            src/c-interface/print_interface_wrapper.o

OBJ=$(GLOBAL) $(TREE) $(SCANP) $(PRINT) $(FLATTEN) $(TYPECHECK) $(OPTIMIZE) \
    $(MODULES) $(OBJECTS) $(REFCOUNT) $(COMPILE) $(PSIOPT) $(CONCURRENT) \
    $(MULTITHREAD) $(CINTERFACE)


#
#  Rules section
#

.PHONY: all efence product check_os dummy prod clean tar floppy distrib distrib_product linux

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
	@ $(ECHO)
	@ $(ECHO) "Building for $(OS).";
	@ $(ECHO)

dummy:
	(cd lib/src; $(MAKE_NORM) )
	(cd src/scanparse; $(MAKE_NORM) )
	(cd src/global; $(MAKE_NORM) )
	(cd src/tree; $(MAKE_NORM) )
	(cd src/print; $(MAKE_NORM) )
	(cd src/flatten; $(MAKE_NORM) )
	(cd src/typecheck; $(MAKE_NORM) )
	(cd src/optimize; $(MAKE_NORM) )
	(cd src/modules; $(MAKE_NORM) )
	(cd src/objects; $(MAKE_NORM) )
	(cd src/refcount; $(MAKE_NORM) )       
	(cd src/concurrent; $(MAKE_NORM) )
	(cd src/multithread; $(MAKE_NORM) )
	(cd src/compile; $(MAKE_NORM) )
	(cd src/psi-opt; $(MAKE_NORM) )
	(cd src/libsac; $(MAKE_PROD) )
	(cd src/heapmgr; $(MAKE_PROD) )
	(cd src/runtime; $(MAKE_NORM) )
	(cd src/tools; $(MAKE_PROD) )
	(cd src/c-interface; $(MAKE_NORM) )
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
	(cd src/tree; $(MAKE_PROD) )
	(cd src/print; $(MAKE_PROD) )
	(cd src/flatten; $(MAKE_PROD) )
	(cd src/typecheck; $(MAKE_PROD) )
	(cd src/optimize; $(MAKE_PROD) )
	(cd src/modules; $(MAKE_PROD) )
	(cd src/objects; $(MAKE_PROD) )
	(cd src/refcount; $(MAKE_PROD) )       
	(cd src/concurrent; $(MAKE_PROD) )
	(cd src/multithread; $(MAKE_PROD) )
	(cd src/compile; $(MAKE_PROD) )
	(cd src/psi-opt; $(MAKE_PROD) )
	(cd src/libsac; $(MAKE_PROD) )
	(cd src/heapmgr; $(MAKE_PROD) )
	(cd src/runtime; $(MAKE_PROD) )
	(cd src/tools; $(MAKE_PROD) )
	(cd src/c-interface; $(MAKE_PROD) )

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
	(cd src/tree; $(MAKE) deps)
	(cd src/print; $(MAKE) deps)
	(cd src/flatten; $(MAKE) deps)
	(cd src/typecheck; $(MAKE) deps)
	(cd src/optimize; $(MAKE) deps)
	(cd src/modules; $(MAKE) deps)
	(cd src/objects; $(MAKE) deps)
	(cd src/refcount; $(MAKE) deps)
	(cd src/concurrent; $(MAKE) deps)
	(cd src/multithread; $(MAKE) deps)
	(cd src/compile; $(MAKE) deps)
	(cd src/psi-opt; $(MAKE) deps)
	(cd src/libsac; $(MAKE) deps)
	(cd src/heapmgr; $(MAKE) deps)
	(cd src/tools; $(MAKE) deps)
	(cd src/runtime; $(MAKE) deps)
	(cd src/c-interface; $(MAKE) deps)

clean:
	(cd lib/src; $(MAKE) clean)
	(cd src/scanparse; $(MAKE) clean)
	(cd src/global; $(MAKE) clean)
	(cd src/tree; $(MAKE) clean)
	(cd src/print; $(MAKE) clean)
	(cd src/flatten; $(MAKE) clean)
	(cd src/typecheck; $(MAKE) clean)
	(cd src/optimize; $(MAKE) clean)
	(cd src/modules; $(MAKE) clean)
	(cd src/objects; $(MAKE) clean)
	(cd src/refcount; $(MAKE) clean)
	(cd src/concurrent; $(MAKE) clean)
	(cd src/multithread; $(MAKE) clean)
	(cd src/compile; $(MAKE) clean )
	(cd src/psi-opt; $(MAKE) clean)
	(cd src/libsac; $(MAKE) clean)
	(cd src/heapmgr; $(MAKE) clean)
	(cd src/tools; $(MAKE) clean)
	(cd src/runtime; $(MAKE) clean)
	(cd src/c-interface; $(MAKE) clean)
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


LINUX_HOST = bunasera
LINUX_USER = sac
LINUX_DIR  = sac2c

linux: src.tar.gz
	@ ping $@ >/dev/null; \
          if [ $${?} -ne 0 ]; then \
            echo "Host $@ is down !"; \
            exit 1; \
          fi
	rsh -l $(LINUX_USER) $(LINUX_HOST) 'mkdir -p $(LINUX_DIR)'
	rcp src.tar.gz $(LINUX_USER)@$(LINUX_HOST):$(LINUX_DIR)
	rsh -l $(LINUX_USER) $(LINUX_HOST) \
            'cd $(LINUX_DIR);'             \
            'rm -rf src;'                  \
            'gunzip -f src.tar.gz;'        \
            'tar xvf src.tar;'             \
            'chmod 644 $(SOURCE_FILES);'   \
            'make deps OS=LINUX_X86;'      \
            'make OS=LINUX_X86'

