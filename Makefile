#
# $Log$
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
# Revision 1.80  1998/05/05 15:56:26  cg
# added linking of file src/optimize/DataFlowMask.c
#
# Revision 1.79  1998/05/05 12:17:52  dkr
# added cc-flag -erroff=E_CAST_DOESNT_YIELD_LVALUE
#
# Revision 1.78  1998/05/03 14:02:57  dkr
# added icm2c_wl.o (compile)
#
# Revision 1.77  1998/04/29 17:12:43  dkr
# added compile/wltransform.o
#
# Revision 1.76  1998/04/28 15:43:07  srs
# added typecheck_WL.o to TYPECHECK
#
# Revision 1.75  1998/04/25 16:27:55  sbs
# icm2c_std.o added in link-list
#
# Revision 1.74  1998/04/17 17:24:14  dkr
# concregions.[ch] renamed to spmdregions.[ch]
#
# Revision 1.73  1998/04/09 23:43:44  dkr
# in 'compile':
#   added wlpragma_funs.o
#
# Revision 1.72  1998/04/03 12:12:21  dkr
# added dir concurrent in prod, clean, ...
#
# Revision 1.71  1998/04/03 12:10:56  srs
# added $(CONCURRENT) to OBJ
#
# Revision 1.70  1998/04/03 11:39:01  dkr
# added concregions.o in dir concurrent
#
# Revision 1.69  1998/04/02 16:07:16  dkr
# added concregs.o
#
# Revision 1.68  1998/03/24 11:47:10  cg
# compilation of compile/profile.c removed
# sources of libsac moved to new directory runtime
#
# Revision 1.67  1998/03/22 18:02:26  srs
# added WLT.o, WLI.o, WLF.o
#
# Revision 1.66  1998/03/04 00:08:53  dkr
# 'clean' now deletes the subdir .sb, too
#
# Revision 1.65  1998/02/27 09:03:55  cg
# added linkage of global/resource.o
#
# Revision 1.64  1998/02/25 09:04:29  cg
# added file globals.c for linkage
#
# Revision 1.63  1998/02/06 13:18:57  srs
# added WithloopFolding.o
#
# Revision 1.62  1998/02/05 16:47:34  srs
# removed WorkReduction from OPTIMIZE
#
# Revision 1.61  1998/01/28 13:46:49  srs
# SHOW_MALLOC is now default
#
# Revision 1.60  1997/11/27 10:45:29  dkr
# added target lib/dbug.o
#
# Revision 1.59  1997/11/24 15:54:56  dkr
# target "clean" now erase sac2c.efence, too
#
# Revision 1.58  1997/11/24 15:43:39  srs
# NEWTREE is now defined by default.
# removed duplicate __EXTENSIONS__
#
# Revision 1.57  1997/11/20 15:11:50  dkr
# changed the path to Old2NewWith.o
#
# Revision 1.56  1997/11/20 14:57:47  dkr
# added Old2NewWith.o to the COMPILE-project
#
# Revision 1.55  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.54  1997/11/11 16:49:20  sbs
# make deps done
#
# Revision 1.53  1997/10/28 19:20:13  srs
# now make can have a macro definition CFALGS which adds
# parameters to the call of the c-compiler:
# e.g. gmake CFLAGS=-Dshow_malloc
#
# Revision 1.52  1997/09/05 13:44:22  cg
# The compiler flag -D__EXTENSIONS__ is now set to avoid warnings
# upon non-ANSI-compliant functions (popen, pclose, tempnam, strdup).
#
# Revision 1.51  1997/08/04 17:03:22  dkr
# added libefence support
#
# Revision 1.50  1997/05/28 12:37:35  sbs
# analyse.o -> profile.o
#
# Revision 1.49  1997/05/14  06:44:20  sbs
# analyse.o inserted
#
# Revision 1.48  1997/04/25  14:27:10  sbs
# fuer cg: libm.a deleted
#
# Revision 1.45  1997/04/24  16:09:53  sbs
# /usr/lib/debug/malloc.o commented out
#
# Revision 1.44  1996/05/24  14:23:00  sbs
# tags line inserted.
#
# Revision 1.43  1996/02/16  09:37:55  sbs
# -lm inserted; floor is needed in src/print/convert.c !
#
# Revision 1.42  1996/01/17  16:49:21  asi
# added common subexpression elimination
#
# Revision 1.40  1996/01/02  15:37:13  cg
# added linking of scnprs.o and cccall.o
#
# Revision 1.39  1995/12/29  10:20:15  cg
# added linking of readsib.o,
# switched -pedantic off to avoid cast warnings
#
# Revision 1.38  1995/12/01  16:07:59  cg
# added linking of precompile.o
#
# Revision 1.38  1995/12/01  16:07:59  cg
# added linking of precompile.o
#
# Revision 1.37  1995/11/16  19:32:03  cg
# added linking of rmvoidfun.o
#
# Revision 1.36  1995/11/06  09:21:09  cg
# added uniquecheck.o
#
# Revision 1.35  1995/11/01  08:00:38  cg
# added linking of objects.o
#
# Revision 1.34  1995/10/26  17:59:36  cg
# Makefile prepared for usage of shell script sac_gcc to avoid
# the boring warning 'casts not allowed as lvalues'.
#
# Revision 1.33  1995/10/22  17:24:56  cg
# added checkdec.o
#
# Revision 1.32  1995/10/20  09:21:38  cg
# added compilation of 'analyse.c`
#
# Revision 1.31  1995/10/16  11:59:58  cg
# objinit.o and new directory objects added.
#
# Revision 1.30  1995/10/05  15:56:11  cg
# added implicittypes.o
#
# Revision 1.29  1995/09/27  12:15:18  cg
# included tree_basic.c and tree_compound.c
#
# Revision 1.28  1995/09/01  07:44:25  cg
# linking of sib.o added
#
# Revision 1.27  1995/07/28  13:27:20  asi
# added make product
#
# Revision 1.26  1995/07/24  11:40:53  asi
# added ArrayElimination.o
#
# Revision 1.25  1995/07/07  15:00:40  asi
# added loop unswitching
#
# Revision 1.24  1995/06/02  09:53:02  sbs
# psi-opt inserted.
#
# Revision 1.23  1995/05/26  14:23:42  asi
# function inlineing and loop unrolling added
#
# Revision 1.22  1995/05/01  15:36:19  asi
# src/optimize/DupTree.o inserted
#
# Revision 1.21  1995/04/05  15:52:38  asi
# loop invariant removal added
#
# Revision 1.20  1995/04/03  10:11:30  sbs
# src/compile/icm2c.o inserted
#
# Revision 1.18  1995/03/29  11:49:43  hw
# compile inserted
#
# Revision 1.17  1995/03/28  12:07:46  hw
# added internal_lib.o
#
# Revision 1.16  1995/03/22  11:19:57  asi
# added debuging tools for malloc, free etc. - malloc.o
# should be removed in final version
#
# Revision 1.15  1995/03/17  17:39:11  asi
# WorkReduction.o added
#
# Revision 1.14  1995/03/10  10:52:39  hw
# refcount inserted
#
# Revision 1.13  1995/02/13  17:25:14  asi
# added ConstantFolding.o and DeadCodeRemoval.o
#
# Revision 1.12  1995/02/03  07:57:47  hw
# added  TYPECHECK
#
# Revision 1.11  1994/12/20  17:33:45  hw
# added free.o & tree.o
#
# Revision 1.10  1994/12/16  14:41:26  sbs
# Put the invokation of make in scanparse in front, since
# some other files include y.tab.h (via scnprs.h)
#
# Revision 1.9  1994/12/16  14:33:25  sbs
# import.o inserted
#
# Revision 1.8  1994/12/11  17:30:35  sbs
# modules inserted
#
# Revision 1.7  1994/12/09  10:42:25  sbs
# optimize inserted
#
# Revision 1.6  1994/12/05  15:07:25  hw
# added path tp convert.o to PRINT
#
# Revision 1.5  1994/12/01  17:32:02  hw
# added TYPCHECK ect.
#
# Revision 1.4  1994/11/17  11:11:37  sbs
# -Wall -pedantic forced for all makes
#
# Revision 1.3  1994/11/10  15:45:55  sbs
# RCS-header inserted
#
#

# CC=gcc -ansi -Wall -pedantic -g 
#
# -pedantic switched off to avoid those disgusting warnings about
# casts in lvalues (-erroff=E_CAST_DOESNT_YIELD_LVALUE for cc)
#
# The switch __EXTENSIONS__ must be set under Solaris in order to avoid warnings
# when using non-ANSI-compliant functions.
# Currently, these are popen(), pclose(), tempnam(), and strdup().
#

CC        := gcc

gcc_FLAGS := -ansi -Wall 
cc_FLAGS  := -xsb -erroff=E_CAST_DOESNT_YIELD_LVALUE

CCFLAGS :=$($(CC)_FLAGS) -g

override CFLAGS :=-D__EXTENSIONS__ -DNEWTREE -DSHOW_MALLOC $(CFLAGS)

CCPROD       :=gcc

gcc_PROD_FLAGS := -ansi -Wall -pedantic -O3
cc_PROD_FLAGS  := 

CCPROD_FLAGS := $($(CC)_PROD_FLAGS)
CPROD_FLAGS  :=-DDBUG_OFF $(CFLAGS)

MAKE         :=$(MAKE) CC="$(CC)" CCFLAGS="$(CCFLAGS)" CFLAGS="$(CFLAGS)"
MAKEPROD     :=$(MAKE) CC="$(CCPROD)" CCFLAGS="$(CCPROD_FLAGS)" CFLAGS="$(CPROD_FLAGS)"
MAKEFLAGS    += --no-print-directory

LEX          :=lex
YACC         :=yacc -dv
LIBS         :=-ly -ll -lm
EFLIBS       :=-L/home/dkr/gcc/lib/ElectricFence -lefence
RM           :=rm -f

LIB          :=lib/dbug.o
# /usr/lib/debug/malloc.o

GLOBAL= src/global/main.o src/global/Error.o src/global/usage.o \
        src/global/my_debug.o src/global/traverse.o  src/global/tree.o \
	src/global/tree_basic.o src/global/tree_compound.o \
        src/global/free.o src/global/internal_lib.o \
        src/global/globals.o src/global/resource.o
SCANP= src/scanparse/y.tab.o src/scanparse/lex.yy.o \
       src/scanparse/scnprs.o
PRINT= src/print/print.o src/print/convert.o
FLATTEN= src/flatten/flatten.o
TYPECHECK= src/typecheck/typecheck.o src/typecheck/prim_fun.o \
           src/typecheck/typecheck_WL.o src/typecheck/gen_pseudo_fun.o
OPTIMIZE= src/optimize/optimize.o src/optimize/ConstantFolding.o \
          src/optimize/DeadCodeRemoval.o \
	  src/optimize/LoopInvariantRemoval.o src/optimize/DupTree.o \
	  src/optimize/Inline.o src/optimize/Unroll.o src/optimize/WLUnroll.o \
          src/optimize/Unswitch.o src/optimize/CSE.o \
	  src/optimize/WithloopFolding.o src/optimize/WLT.o \
	  src/optimize/WLI.o src/optimize/WLF.o src/optimize/DataFlowMask.o
PSIOPT= src/psi-opt/index.o src/psi-opt/psi-opt.o src/psi-opt/ArrayElimination.o
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

all: dummy sac2c

efence: dummy sac2c.efence

product : clean prod sac2c

dummy:
	(cd src/scanparse; $(MAKE) )
	(cd src/global; $(MAKE) )
	(cd src/print; $(MAKE) )
	(cd src/flatten; $(MAKE) )
	(cd src/typecheck; $(MAKE) )
	(cd src/optimize; $(MAKE) )
	(cd src/modules; $(MAKE) )
	(cd src/objects; $(MAKE) )
	(cd src/refcount; $(MAKE) )       
	(cd src/concurrent; $(MAKE) )
	(cd src/compile; $(MAKE) )
	(cd src/psi-opt; $(MAKE) )
	(cd src/runtime; $(MAKEPROD) )
	(cd lib/src; $(MAKE) )

prod:
	(cd src/scanparse; $(MAKEPROD) )
	(cd src/global; $(MAKEPROD) )
	(cd src/print; $(MAKEPROD) )
	(cd src/flatten; $(MAKEPROD) )
	(cd src/typecheck; $(MAKEPROD) )
	(cd src/optimize; $(MAKEPROD) )
	(cd src/modules; $(MAKEPROD) )
	(cd src/objects; $(MAKEPROD) )
	(cd src/refcount; $(MAKEPROD) )       
	(cd src/concurrent; $(MAKEPROD) )
	(cd src/compile; $(MAKEPROD) )
	(cd src/psi-opt; $(MAKEPROD) )
	(cd src/runtime; $(MAKEPROD) )
	(cd lib/src; $(MAKEPROD) )

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

tags: 
	ctags src/*/*.[ch] >/dev/null
