#
#
# $Log$
# Revision 3.94  2004/07/26 16:14:03  skt
# support for create_cells added
#
# Revision 3.93  2004/07/21 16:56:10  ktr
# added src/compile/markmemval.o
#
# Revision 3.92  2004/07/21 12:40:38  khf
# added WLEnhancement.o and ExplicitAccumulate.o to FLATTEN
#
# Revision 3.91  2004/07/18 08:50:09  ktr
# added src/refcount/refcounting.o to REFCOUNT.
#
# Revision 3.90  2004/07/14 23:58:41  sah
# removed old non-ssa optimizations
#
# Revision 3.89  2004/07/14 15:32:49  ktr
# refcount/alloc.o added.
#
# Revision 3.88  2004/07/06 12:41:19  skt
# propagate_executionmode added
#
# Revision 3.87  2004/07/01 14:35:30  sah
# added newast target
#
# Revision 3.86  2004/06/08 14:21:52  skt
# tag_executionmode.o added
#
# Revision 3.85  2004/04/27 10:17:11  skt
# target assignments_rearrange.o added
#
# Revision 3.84  2004/04/21 16:42:30  ktr
# added SSARefCount.o
#
# Revision 3.83  2004/04/08 08:06:42  khf
# WithloopFusion.o added
#
# Revision 3.82  2004/03/09 23:47:40  dkrHH
# compile.tagged.o removed
#
# Revision 3.81  2004/03/02 16:39:40  mwe
# ConstVarPropagation added
#
# Revision 3.80  2004/02/26 13:04:21  khf
# WLPartitionGeneration.o added
#
# Revision 3.79  2004/02/25 08:17:44  cg
# Elimination of while-loops by conversion into do-loops with
# leading conditional integrated into flatten.
# Separate compiler phase while2do eliminated.
# NO while-loops may occur after flatten.
# While-loop specific code eliminated.
#
# Revision 3.78  2004/02/02 15:49:35  skt
# ssa.o added
#
# Revision 3.77  2004/01/28 18:02:41  skt
# some adaptions, because of moving from CheckAvis.[hc]
# SSATransform.[hc] and UndoSSATransform.[hs] from
# src/tree to src/flatten
#
# Revision 3.76  2003/11/07 15:17:35  sbs
# CLOCK_SKEW_ELIMINATION run on config.h as well now...
#
# Revision 3.75  2003/11/07 14:45:50  sbs
# CLOCK_SKEW_ELIMINATION now applied to Makefile.Config as well.
#
# Revision 3.74  2003/09/20 14:15:43  dkr
# icm2c_prf.o added
#
# Revision 3.73  2003/09/17 12:34:55  sbs
# type_statistics.o added
#
# Revision 3.72  2003/08/18 11:28:48  sbs
# src/optimize/SelectionPropagation.o is indeed linked now as well 8-((
#
# Revision 3.71  2003/04/26 20:49:26  mwe
# ElimSubDiv and UndoElimSubDiv added
#
# Revision 3.70  2003/03/25 12:00:42  sbs
# target configure changed
#
# Revision 3.69  2003/03/25 11:58:15  sbs
# configure target added.
#
# Revision 3.68  2003/03/21 16:40:07  sbs
# if DISABLE_PHM is set, libsac_heapmgr_xxx.a will not be created anymore!
#
# Revision 3.67  2003/03/20 14:07:38  sbs
# auto-configure-mechanism created; check_os adjusted.
#
# Revision 3.66  2003/03/09 17:13:09  ktr
# blir.o added to COMPILE
#
# Revision 3.65  2003/02/12 18:44:20  ktr
# Added target 'doxygen' to create documentation in $(SAC_PUBLIC_HTML)/sac2cdoc.
#
# Revision 3.64  2003/02/11 17:33:14  sbs
# call of make tagged changed to a simple touch operation
# pattern rule for .tagged eliminated (depreciated due to
# implicit mechanism in Makefile.Config).
#
# Revision 3.63  2003/02/08 16:01:26  mwe
# DistributiveLaw.o for linker added
#
# Revision 3.62  2002/10/28 06:35:56  sbs
# new target "tagged" added and new pattern rule for suffix ".tagged"
# added. You make use  make tagged  or make twice.tagged now; but make sure
# you do not mix with the ordinary one and make sure you make clean before
# doing it the first time since the object files have identical names
# with the ordinary ones!!!!
#
# Revision 3.61  2002/10/18 13:36:48  sbs
# new targets   twice    and sac2c.twice   added
#
# Revision 3.60  2002/10/08 22:09:17  dkr
# SSAWLUnroll.o added
#
# Revision 3.59  2002/09/09 14:19:26  dkr
# icm2c_error.o added
#
# Revision 3.58  2002/08/13 10:24:23  sbs
# handle_mops.o added.
#
# Revision 3.57  2002/08/09 13:09:38  dkr
# typecheck/create_wrapper_code.o added
#
# Revision 3.56  2002/08/05 17:01:40  sbs
# some files for the new type checker added.
#
# Revision 3.55  2002/07/09 12:45:21  sbs
# handle_dots.o added
#
# Revision 3.54  2002/07/02 13:01:50  dkr
# icm2c_basic.o added
#
# Revision 3.52  2002/06/07 17:20:09  mwe
# Added AssociativeLaw.o for linking
#
# Revision 3.51  2002/05/31 17:22:23  dkr
# NameTuplesUtils.o and icm2c_utils.o added
#
# Revision 3.50  2002/03/13 16:03:20  ktr
# Added Withloop-Scalarization for linking
#
# Revision 3.49  2002/03/10 16:41:19  sbs
# ssi.o included
#
# Revision 3.48  2002/03/05 15:35:08  sbs
# create_wrappers.o added
#
# Revision 3.47  2002/02/21 15:14:34  sbs
# src/flatten/insert_vardec.o added
#
# Revision 3.46  2002/01/18 12:00:23  sacbase
# dependencies check_os and tools/bin/cse added for distrib_product!
#
# Revision 3.45  2001/12/10 15:33:14  dkr
# compile.tagged.o added
#
# Revision 3.44  2001/12/06 16:17:08  dkr
# no changes done
#
# Revision 3.43  2001/11/20 12:24:54  sbs
# cse moved 8-)
#
# Revision 3.41  2001/11/15 14:43:16  sbs
# PROJECT_ROOT added
#
# Revision 3.40  2001/11/14 13:19:06  sbs
# TARGET clean added for avoiding deps mechanism on clean.
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

PROJECT_ROOT := ./
include $(PROJECT_ROOT)/Makefile.Config


LIB          := lib/dbug.o lib/main_args.o

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
        src/global/options.o src/global/NameTuples.o \
        src/global/NameTuplesUtils.o
TREE= src/tree/traverse.o src/tree/tree_basic.o src/tree/free.o \
      src/tree/tree_compound.o src/tree/DupTree.o src/tree/LookUpTable.o \
      src/tree/DataFlowMask.o src/tree/DataFlowMaskUtils.o \
      src/tree/InferDFMs.o src/tree/cleanup_decls.o src/tree/adjust_ids.o \
      src/tree/change_signature.o src/tree/compare_tree.o \
      src/tree/scheduling.o src/tree/wl_bounds.o

SCANP= src/scanparse/y.tab.o src/scanparse/lex.yy.o \
       src/scanparse/scnprs.o src/scanparse/handle_dots.o
PRINT= src/print/print.o src/print/convert.o
FLATTEN= src/flatten/flatten.o src/flatten/lac2fun.o \
         src/flatten/fun2lac.o src/flatten/insert_vardec.o \
         src/flatten/handle_mops.o src/flatten/UndoSSATransform.o \
         src/flatten/CheckAvis.o src/flatten/SSATransform.o \
         src/flatten/ssa.o src/flatten/WLPartitionGeneration.o \
	 src/flatten/WLEnhancement.o src/flatten/ExplicitAccumulate.o

CONSTANTS= src/constants/shape.o src/constants/constants_basic.o \
           src/constants/constants_struc_ops.o \
           src/constants/constants_ari_ops.o  src/constants/cv2cv.o \
           src/constants/cv2scalar.o src/constants/cv2str.o \
           src/constants/zipcv.o src/constants/basecv.o
TYPECHECK= src/typecheck/typecheck.o src/typecheck/prim_fun.o \
           src/typecheck/typecheck_WL.o src/typecheck/gen_pseudo_fun.o \
           src/typecheck/new_typecheck.o src/typecheck/new_types.o \
           src/typecheck/user_types.o src/typecheck/create_wrappers.o \
           src/typecheck/ssi.o src/typecheck/sig_deps.o src/typecheck/ct_prf.o \
           src/typecheck/ct_basic.o src/typecheck/ct_fun.o \
           src/typecheck/ct_with.o src/typecheck/type_errors.o \
           src/typecheck/specialize.o src/typecheck/new2old.o \
           src/typecheck/create_wrapper_code.o src/typecheck/type_statistics.o
OPTIMIZE= src/optimize/optimize.o \
          src/optimize/generatemasks.o \
          src/optimize/DeadFunctionRemoval.o \
	  src/optimize/Inline.o \
          src/optimize/AssociativeLaw.o \
          src/optimize/SSADeadCodeRemoval.o src/optimize/SSACSE.o \
          src/optimize/SSAConstantFolding.o src/optimize/SSALIR.o \
          src/optimize/SSALUR.o src/optimize/SSAInferLI.o \
          src/optimize/SSAWLUnroll.o src/optimize/rmcasts.o \
          src/optimize/DistributiveLaw.o src/optimize/ElimSubDiv.o \
          src/optimize/UndoElimSubDiv.o src/optimize/SelectionPropagation.o \
          src/optimize/ConstVarPropagation.o

PROFILE= src/profile/annotate_fun_calls.o
PSIOPT= src/psi-opt/index.o src/psi-opt/ArrayElimination.o \
        src/psi-opt/wl_access_analyze.o src/psi-opt/tile_size_inference.o \
        src/psi-opt/WithloopScalarization.o \
        src/psi-opt/SSAWithloopFolding.o src/psi-opt/SSAWLT.o \
        src/psi-opt/SSAWLI.o src/psi-opt/SSAWLF.o \
        src/psi-opt/pad.o src/psi-opt/pad_collect.o src/psi-opt/pad_infer.o \
        src/psi-opt/pad_transform.o src/psi-opt/pad_info.o \
	src/psi-opt/WithloopFusion.o
MODULES= src/modules/filemgr.o src/modules/import.o src/modules/writesib.o \
         src/modules/implicittypes.o src/modules/analysis.o \
         src/modules/checkdec.o src/modules/readsib.o src/modules/cccall.o
OBJECTS= src/objects/objinit.o src/objects/objects.o src/objects/uniquecheck.o
REFCOUNT= src/refcount/refcount.o \
          src/refcount/alloc.o src/refcount/refcounting.o
CONCURRENT= src/concurrent/concurrent.o \
            src/concurrent/spmd_init.o src/concurrent/spmd_opt.o \
            src/concurrent/spmd_lift.o src/concurrent/sync_init.o \
            src/concurrent/sync_opt.o src/concurrent/schedule.o \
            src/concurrent/spmd_trav.o src/concurrent/spmd_cons.o \
            src/concurrent/concurrent_lib.o
MULTITHREAD= src/multithread/multithread.o src/multithread/schedule_init.o \
             src/multithread/repfuns_init.o src/multithread/blocks_init.o \
             src/multithread/blocks_expand.o src/multithread/multithread_lib.o \
             src/multithread/mtfuns_init.o src/multithread/blocks_cons.o \
             src/multithread/blocks_propagate.o \
             src/multithread/dataflow_analysis.o \
             src/multithread/barriers_init.o src/multithread/blocks_lift.o \
             src/multithread/adjust_calls.o src/multithread/assignments_rearrange.o \
             src/multithread/tag_executionmode.o src/multithread/propagate_executionmode.o \
            src/multithread/create_cells.o
COMPILE= src/compile/wltransform.o src/compile/wlpragma_funs.o \
         src/compile/precompile.o src/compile/gen_startup_code.o src/compile/compile.o \
         src/compile/icm2c.o src/compile/icm2c_basic.o \
         src/compile/icm2c_utils.o src/compile/icm2c_std.o src/compile/icm2c_prf.o \
         src/compile/icm2c_mt.o src/compile/icm2c_sched.o \
         src/compile/icm2c_wl.o src/compile/icm2c_error.o \
         src/compile/ReuseWithArrays.o src/compile/PatchWith.o \
         src/compile/markmemvals.o

CINTERFACE= src/c-interface/map_cwrapper.o src/c-interface/print_interface.o \
            src/c-interface/import_specialization.o \
            src/c-interface/print_interface_header.o \
            src/c-interface/print_interface_wrapper.o

OBJ= $(GLOBAL) $(TREE) $(SCANP) $(PRINT) $(FLATTEN) $(TYPECHECK) $(OPTIMIZE) \
     $(MODULES) $(OBJECTS) $(REFCOUNT) $(COMPILE) $(PSIOPT) $(CONCURRENT) \
     $(MULTITHREAD) $(CINTERFACE) $(CONSTANTS) $(PROFILE)


#
#  Rules section
#

.PHONY: all efence product check_os dummy prod clean tar floppy distrib distrib_product linux newast

all: check_os tools/bin/cse dummy sac2c

efence: check_os tools/bin/cse dummy sac2c.efence

product: check_os tools/bin/cse clean prod sac2c.prod

distrib_product: check_os tools/bin/cse prod sac2c.prod

twice: check_os tools/bin/cse dummy sac2c sac2c.twice

tools/bin/cse:
	$(MAKE) -C tools
	$(CLOCK_SKEW_ELIMINATION) Makefile.Config
	$(CLOCK_SKEW_ELIMINATION) src/global/config.h


check_os:
	@ if [ "$(OS)" = "" -o "$(ARCH)" = "" ]; \
	  then $(ECHO) "*** Unknown OS or unknown ARCH! Please specify!"; \
	       exit 1; \
	  fi
	@ $(ECHO)
	@ $(ECHO) "Building for $(OS) on $(ARCH).";
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
ifneq ($(DISABLE_PHM),yes)
	(cd src/heapmgr; $(MAKE_PROD) )
endif
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
ifneq ($(DISABLE_PHM),yes)
	(cd src/heapmgr; $(MAKE_PROD) )
endif
	(cd src/runtime; $(MAKE_PROD) )
	(cd src/tools; $(MAKE_PROD) )
	(cd src/c-interface; $(MAKE_PROD) )

sac2c: $(OBJ) $(LIB)
	$(CC) $(CCFLAGS) $(CFLAGS) -o sac2c $(OBJ) $(LIB) $(LIBS)

sac2c.efence: $(OBJ) $(LIB)
	$(CC) $(CCFLAGS) $(CFLAGS) -o sac2c.efence $(OBJ) $(LIB) $(LIBS) $(EFLIBS)

sac2c.prod:  $(OBJ) $(LIB)
	$(CCPROD) $(CCPROD_FLAGS) $(CPROD_FLAGS) -o sac2c $(OBJ) $(LIB) $(LIBS)

sac2c.twice: $(OBJ) $(LIB)
	ssh twice \
	  "cd $(RCSROOT); \
           setenv PATH /opt/gnu/bin\:$$$$PATH; \
           $(CC) $(CCFLAGS) $(CFLAGS) -o sac2c.twice $(OBJ) $(LIB) $(LIBS)"

doxygen:
	doxygen sac2cdoxy

deps:
	$(ECHO) "make deps is obsolete"

clean:
	(cd lib/src; $(MAKE_CLEAN) )
	(cd src/scanparse; $(MAKE_CLEAN) )
	(cd src/global; $(MAKE_CLEAN) )
	(cd src/tree; $(MAKE_CLEAN) )
	(cd src/print; $(MAKE_CLEAN) )
	(cd src/flatten; $(MAKE_CLEAN) )
	(cd src/typecheck; $(MAKE_CLEAN) )
	(cd src/constants; $(MAKE_CLEAN) )
	(cd src/optimize; $(MAKE_CLEAN) )
	(cd src/modules; $(MAKE_CLEAN) )
	(cd src/objects; $(MAKE_CLEAN) )
	(cd src/refcount; $(MAKE_CLEAN) )
	(cd src/concurrent; $(MAKE_CLEAN) )
	(cd src/multithread; $(MAKE_CLEAN) )
	(cd src/compile; $(MAKE_CLEAN) )
	(cd src/profile; $(MAKE_CLEAN) )
	(cd src/psi-opt; $(MAKE_CLEAN) )
	(cd src/libsac; $(MAKE_CLEAN) )
	(cd src/heapmgr; $(MAKE_CLEAN) )
	(cd src/tools; $(MAKE_CLEAN) )
	(cd src/runtime; $(MAKE_CLEAN) )
	(cd src/c-interface; $(MAKE_CLEAN) )
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

$(PROJECT_ROOT)/Makefile.Config: $(PROJECT_ROOT)/Makefile.Config.in
	(cd $(PROJECT_ROOT); ./configure)

configure: configure.ac
	co -l configure
	co -l src/global/config.h.in
	autoconf
	autoheader
	ci -u configure src/global/config.h.in


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
            'make OS=LINUX_X86'

newast:
	touch _NEW_AST

