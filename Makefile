#
#
# $Log$
# Revision 3.38  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.37  2001/11/13 21:31:39  dkr
# OSX_MAC_LIBS corrected
#
# Revision 3.36  2001/11/13 20:42:42  dkr
# OSX_MAC added in 'check_os'
#
# Revision 3.35  2001/11/13 20:02:35  dkr
# new system OSX_MAC added
#
# Revision 3.34  2001/07/18 12:57:45  cg
# Linking of file tree.o eliminated.
#
# Revision 3.33  2001/05/30 14:00:22  nmw
# SSAInferLI.o added
#
# Revision 3.32  2001/05/22 14:51:57  nmw
# rmcasts.o added
#
# Revision 3.31  2001/05/15 16:38:01  nmw
# SSAWLI.o and SSAWLF.o in src/psi-opt added
#
# Revision 3.30  2001/05/14 15:59:18  nmw
# SSAWLT.o and SSAWithloopFOlding.o added
#
# Revision 3.29  2001/05/11 14:42:36  cg
# Added linking of new file icm2c_sched.o
#
# Revision 3.28  2001/05/02 07:57:34  nmw
# basecv.o added
#
# Revision 3.27  2001/04/26 17:07:37  dkr
# rmvoidfun.o removed
#
# Revision 3.26  2001/04/20 11:18:21  nmw
# SSALUR.o added to project
#
# Revision 3.25  2001/04/18 15:37:02  nmw
# while2do added
#
# Revision 3.24  2001/04/02 11:45:46  dkr
# wl_bounds.o added
#
# Revision 3.23  2001/03/26 15:41:19  nmw
# SSALIR.o added
#
# Revision 3.22  2001/03/20 16:17:16  nmw
# SSAConstantFolding.o added
#
# Revision 3.21  2001/03/09 11:14:44  sbs
# PROFILE and annotate_fun_calls.o added.
#
# Revision 3.20  2001/03/06 13:19:47  nmw
# compare_tree added
#
# Revision 3.19  2001/03/05 17:01:33  sbs
# zipcv.o and SSACSE.o added.
#
# Revision 3.18  2001/03/02 15:52:56  nmw
# change_signature added
#
# Revision 3.17  2001/03/02 14:55:55  sbs
# integrated constants!
#
# Revision 3.16  2001/03/02 14:42:34  sbs
# integrated constants?
#
# Revision 3.15  2001/02/23 13:40:15  nmw
# SSADeadCodeRemoval added
#
# Revision 3.14  2001/02/22 14:46:10  sbs
# cv2str.o added.
#
# Revision 3.13  2001/02/22 12:52:22  nmw
# UndoSSATransform added
#
# Revision 3.12  2001/02/13 15:19:45  nmw
# SSATransform added
#
# Revision 3.11  2001/02/12 17:08:33  nmw
# CheckAvis in /src/tree added to sac2c
#
#
#
# ... [eliminated] 
#
# Revision 1.81  1998/05/13 07:12:25  cg
# added linking of file icm2c_mt.o
#
#

#######################################################################################
#
# include general setup:
#

include Makefile.Config


LIB          :=lib/dbug.o lib/main_args.o

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
TREE= src/tree/traverse.o src/tree/tree_basic.o src/tree/free.o \
      src/tree/tree_compound.o src/tree/DupTree.o src/tree/LookUpTable.o \
      src/tree/DataFlowMask.o src/tree/DataFlowMaskUtils.o src/tree/InferDFMs.o \
      src/tree/cleanup_decls.o src/tree/adjust_ids.o src/tree/scheduling.o \
      src/tree/CheckAvis.o src/tree/SSATransform.o src/tree/UndoSSATransform.o \
      src/tree/change_signature.o src/tree/compare_tree.o src/tree/wl_bounds.o
SCANP= src/scanparse/y.tab.o src/scanparse/lex.yy.o \
       src/scanparse/scnprs.o
PRINT= src/print/print.o src/print/convert.o
FLATTEN= src/flatten/flatten.o src/flatten/lac2fun.o src/flatten/fun2lac.o \
         src/flatten/while2do.o
CONSTANTS= src/constants/shape.o src/constants/constants_basic.o \
           src/constants/constants_struc_ops.o src/constants/constants_ari_ops.o \
           src/constants/cv2cv.o src/constants/cv2scalar.o src/constants/cv2str.o \
           src/constants/zipcv.o src/constants/basecv.o
TYPECHECK= src/typecheck/typecheck.o src/typecheck/prim_fun.o \
           src/typecheck/typecheck_WL.o src/typecheck/gen_pseudo_fun.o \
           src/typecheck/new_typecheck.o src/typecheck/new_types.o \
           src/typecheck/user_types.o
OPTIMIZE= src/optimize/optimize.o src/optimize/ConstantFolding.o \
          src/optimize/generatemasks.o src/optimize/DeadCodeRemoval.o \
          src/optimize/DeadFunctionRemoval.o src/optimize/freemasks.o \
	  src/optimize/LoopInvariantRemoval.o src/optimize/Inline.o \
          src/optimize/Unroll.o src/optimize/WLUnroll.o src/optimize/Unswitch.o \
          src/optimize/CSE.o \
          src/optimize/SSADeadCodeRemoval.o src/optimize/SSACSE.o \
          src/optimize/SSAConstantFolding.o src/optimize/SSALIR.o \
          src/optimize/SSALUR.o src/optimize/rmcasts.o src/optimize/SSAInferLI.o
PROFILE= src/profile/annotate_fun_calls.o
PSIOPT= src/psi-opt/index.o src/psi-opt/ArrayElimination.o \
	src/psi-opt/wl_access_analyze.o src/psi-opt/tile_size_inference.o \
	src/psi-opt/WithloopFolding.o src/psi-opt/WLT.o src/psi-opt/WLI.o \
	src/psi-opt/WLF.o \
        src/psi-opt/SSAWithloopFolding.o src/psi-opt/SSAWLT.o \
        src/psi-opt/SSAWLI.o src/psi-opt/SSAWLF.o \
	src/psi-opt/pad.o src/psi-opt/pad_collect.o src/psi-opt/pad_infer.o \
	src/psi-opt/pad_transform.o src/psi-opt/pad_info.o
MODULES= src/modules/filemgr.o src/modules/import.o src/modules/writesib.o  \
         src/modules/implicittypes.o src/modules/analysis.o \
         src/modules/checkdec.o src/modules/readsib.o \
         src/modules/cccall.o
OBJECTS= src/objects/objinit.o src/objects/objects.o src/objects/uniquecheck.o
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
          src/compile/icm2c_sched.o src/compile/icm2c_wl.o  \
          src/compile/ReuseWithArrays.o src/compile/PatchWith.o

CINTERFACE= src/c-interface/map_cwrapper.o src/c-interface/print_interface.o \
            src/c-interface/import_specialization.o \
            src/c-interface/print_interface_header.o \
            src/c-interface/print_interface_wrapper.o

OBJ=$(GLOBAL) $(TREE) $(SCANP) $(PRINT) $(FLATTEN) $(TYPECHECK) $(OPTIMIZE) \
    $(MODULES) $(OBJECTS) $(REFCOUNT) $(COMPILE) $(PSIOPT) $(CONCURRENT) \
    $(MULTITHREAD) $(CINTERFACE) $(CONSTANTS) $(PROFILE)


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
               -a "$(OS)" != "OSF_ALPHA" -a "$(OS)" != "OSX_MAC" ]; \
	  then $(ECHO) "*** Unknown OS! Please specify:"; \
               $(ECHO) "SOLARIS_SPARC (default)"; \
               $(ECHO) "LINUX_X86"; \
               $(ECHO) "OSF_ALPHA"; \
               $(ECHO) "OSX_MAC"; \
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
	(cd src/constants; $(MAKE_NORM) )
	(cd src/optimize; $(MAKE_NORM) )
	(cd src/modules; $(MAKE_NORM) )
	(cd src/objects; $(MAKE_NORM) )
	(cd src/refcount; $(MAKE_NORM) )       
	(cd src/concurrent; $(MAKE_NORM) )
	(cd src/multithread; $(MAKE_NORM) )
	(cd src/compile; $(MAKE_NORM) )
	(cd src/profile; $(MAKE_NORM) )
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
	(cd src/constants; $(MAKE_PROD) )
	(cd src/optimize; $(MAKE_PROD) )
	(cd src/modules; $(MAKE_PROD) )
	(cd src/objects; $(MAKE_PROD) )
	(cd src/refcount; $(MAKE_PROD) )       
	(cd src/concurrent; $(MAKE_PROD) )
	(cd src/multithread; $(MAKE_PROD) )
	(cd src/compile; $(MAKE_PROD) )
	(cd src/profile; $(MAKE_PROD) )
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

clean:
	(cd lib/src; $(MAKE) clean)
	(cd src/scanparse; $(MAKE) clean)
	(cd src/global; $(MAKE) clean)
	(cd src/tree; $(MAKE) clean)
	(cd src/print; $(MAKE) clean)
	(cd src/flatten; $(MAKE) clean)
	(cd src/typecheck; $(MAKE) clean)
	(cd src/constants; $(MAKE) clean)
	(cd src/optimize; $(MAKE) clean)
	(cd src/modules; $(MAKE) clean)
	(cd src/objects; $(MAKE) clean)
	(cd src/refcount; $(MAKE) clean)
	(cd src/concurrent; $(MAKE) clean)
	(cd src/multithread; $(MAKE) clean)
	(cd src/compile; $(MAKE) clean )
	(cd src/profile; $(MAKE) clean )
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

