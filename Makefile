#
# $Log$
# Revision 1.102  1999/01/15 16:59:46  sbs
# freemasks.o added
#
# Revision 1.101  1999/01/15 15:28:15  cg
# added linking of file psi-opt/tile_size_inference.o
#
# Revision 1.100  1999/01/14 13:13:21  sacbase
# in clean: rm src.tar.gz added .
#
# Revision 1.99  1999/01/07 13:59:23  sbs
# NEWTREE turned off + generatemasks.o, DeadFunctionRemoval added
#
# Revision 1.98  1998/12/17 09:48:39  cg
# added new targets tar, distrib, and fafnir.
# improved targets src.tar.gz and floppy
#
# Revision 1.97  1998/12/10 12:36:37  cg
# added -DPRODUCTION for production compilations of sac2c in order
# to allow for different code sections in production and developer
# versions of sac2c.
#
# Revision 1.96  1998/12/07 17:28:34  cg
# Added mechanism to link with different libraries on different
# platforms; added correct settings to please target LINUX_X86.
#
# Revision 1.95  1998/12/07 09:41:28  sbs
# check_os added. check_os makes sure that OS is set properly!
#
# Revision 1.94  1998/12/04 15:02:34  sbs
# some LOGs killed and OS diversification standardized 8-)
# i.e., during compilation a flag -DSOLARIS_SPARC or
# -DLINUX_X86 will be set.
# If the latter is to be triggered, simply call:
# gmake OS=LINUX_X86
#
# Revision 1.93  1998/10/26 12:32:59  cg
# new mechanism implemented that stores sac2c build information
# which may later be retrieved by the -h or -i option, repsectively.
#
# Revision 1.92  1998/10/26 12:28:34  sbs
# tar / floppy added
#
# Revision 1.91  1998/09/02 09:26:24  sbs
# inserted floppy, %.gz, and src.tar - rules!
#
# Revision 1.90  1998/08/26 12:57:01  sbs
# -pedantic for product-line eliminated!
# reason: illegal cast in lvalue!
#
# Revision 1.89  1998/08/14 22:21:15  dkr
# path to efence changed
#
# Revision 1.88  1998/07/16 17:44:06  dkr
# eleminated MAKEPROD reference in dummy-rule
#
# Revision 1.87  1998/06/29 09:16:04  cg
# The SAC runtime library is now compiled with optimizations.
#
# Revision 1.86  1998/06/18 13:24:00  cg
# added linking of spmd_init.o spmd_opt.o spmd_lift.o sync_init.o
# sync_opt.o sched.o and scheduling.o
# removed linking of spmdregions.o
#
# Revision 1.85  1998/06/12 14:18:33  cg
# added linking of concurrent/scheduling.o
#
# Revision 1.84  1998/06/07 18:38:33  dkr
# added src/compile/ReuseWithArrays.o
#
# Revision 1.83  1998/05/28 14:57:10  sbs
# gen_pseudo_fun.o in typecheck added!
#
# Revision 1.82  1998/05/13 13:37:38  srs
# added WLUnroll
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
cc_FLAGS  := -xsb -erroff=E_CAST_DOESNT_YIELD_LVALUE
cc_PROD_FLAGS  := 

################################################################################
#
# OS setup:
#
# legal values for OS currently are:
#    SOLARIS_SPARC
#    LINUX_X86
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
SOLARIS_SPARC_LIBS  := -ll -ly

#
# LINUX_X86 specific flags and libraries:
#
LINUX_X86_FLAGS     := -D_POSIX_SOURCE
LINUX_X86_LIBS      := -lfl


################################################################################
#
# general setup:
#

CCFLAGS :=$($(CC)_FLAGS) -g $($(OS)_FLAGS)
CCPROD_FLAGS := $($(CC)_PROD_FLAGS) $($(OS)_FLAGS) 

CFLAGS := -DSHOW_MALLOC -D$(OS) 
CPROD_FLAGS  :=-DDBUG_OFF -DPRODUCTION -D$(OS)

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

LIB          :=lib/dbug.o
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
        src/global/globals.o src/global/resource.o src/global/build.o
SCANP= src/scanparse/y.tab.o src/scanparse/lex.yy.o \
       src/scanparse/scnprs.o
PRINT= src/print/print.o src/print/convert.o
FLATTEN= src/flatten/flatten.o
TYPECHECK= src/typecheck/typecheck.o src/typecheck/prim_fun.o \
           src/typecheck/typecheck_WL.o src/typecheck/gen_pseudo_fun.o
OPTIMIZE= src/optimize/optimize.o src/optimize/ConstantFolding.o \
          src/optimize/generatemasks.o src/optimize/DeadCodeRemoval.o \
          src/optimize/DeadFunctionRemoval.o src/optimize/freemasks.o \
	  src/optimize/LoopInvariantRemoval.o src/optimize/DupTree.o \
	  src/optimize/Inline.o src/optimize/Unroll.o src/optimize/WLUnroll.o \
          src/optimize/Unswitch.o src/optimize/CSE.o \
	  src/optimize/WithloopFolding.o src/optimize/WLT.o \
	  src/optimize/WLI.o src/optimize/WLF.o src/optimize/DataFlowMask.o
PSIOPT= src/psi-opt/index.o src/psi-opt/psi-opt.o src/psi-opt/ArrayElimination.o \
        src/psi-opt/tile_size_inference.o
MODULES= src/modules/filemgr.o src/modules/import.o src/modules/writesib.o  \
         src/modules/implicittypes.o src/modules/analysis.o \
         src/modules/checkdec.o src/modules/readsib.o \
         src/modules/cccall.o
OBJECTS= src/objects/objinit.o src/objects/objects.o \
         src/objects/uniquecheck.o src/objects/rmvoidfun.o
REFCOUNT= src/refcount/refcount.o
CONCURRENT= src/concurrent/concurrent.o src/concurrent/scheduling.o  \
            src/concurrent/spmd_init.o src/concurrent/spmd_opt.o   \
            src/concurrent/spmd_lift.o src/concurrent/sync_init.o   \
            src/concurrent/sync_opt.o src/concurrent/schedule.o 
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

product: check_os clean prod sac2c

distrib_product: prod sac2c



check_os:
	@ if [ "$(OS)" != "SOLARIS_SPARC" -a "$(OS)" != "LINUX_X86" ]; \
	  then $(ECHO) "*** Unknown OS! Please specify:"; \
               $(ECHO) "SOLARIS_SPARC (default)"; \
               $(ECHO) "LINUX_X86"; \
	       exit 1; \
	  fi

dummy:
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
	(cd src/runtime; $(MAKE_NORM) )
	(cd lib/src; $(MAKE_NORM) )

prod:
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
	(cd src/runtime; $(MAKE_PROD) )
	(cd lib/src; $(MAKE_PROD) )

sac2c: $(OBJ) $(LIB)
	$(CC) $(CCFLAGS) $(CFLAGS) -o sac2c $(OBJ) $(LIB) $(LIBS)

sac2c.efence: $(OBJ) $(LIB)
	$(CC) $(CCFLAGS) $(CFLAGS) -o sac2c.efence $(OBJ) $(LIB) $(LIBS) $(EFLIBS)

deps:
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
	(cd src/runtime; $(MAKE) deps)
	(cd lib/src; $(MAKE) deps)

clean:
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
	(cd src/runtime; $(MAKE) clean)
	(cd lib/src; $(MAKE) clean)
	$(RM) sac2c
	$(RM) sac2c.efence
	$(RM) -r .sb
	$(RM) src.tar.gz

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




