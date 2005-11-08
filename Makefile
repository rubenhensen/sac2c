
#
# $Id$
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
# Collection of object files
#

GLOBAL= src/global/main.o src/global/setup.o src/global/usage.o \
        src/global/internal_lib.o src/global/globals.o \
        src/global/resource.o src/global/build.o \
        src/global/options.o src/global/NameTuplesUtils.o\
        src/global/filemgr.o src/global/ctinfo.o src/global/phase.o \
        src/global/private_heap.o

TREE= src/tree/traverse.o src/tree/tree_basic.o src/tree/free.o \
      src/tree/tree_compound.o src/tree/DupTree.o src/tree/LookUpTable.o \
      src/tree/DataFlowMask.o src/tree/DataFlowMaskUtils.o \
      src/tree/InferDFMs.o src/tree/cleanup_decls.o src/tree/adjust_ids.o \
      src/tree/change_signature.o src/tree/compare_tree.o \
      src/tree/scheduling.o src/tree/wl_bounds.o \
      src/tree/node_basic.o src/tree/free_node.o \
      src/tree/free_attribs.o src/tree/traverse_tables.o \
      src/tree/traverse_helper.o src/tree/check.o src/tree/check_lib.o \
      src/tree/checktst.o src/tree/prepare_inlining.o \
      src/tree/map_lac_funs.o src/tree/map_fun_trav.o

SERIALIZE= src/serialize/serialize_node.o \
           src/serialize/serialize_attribs.o src/serialize/deserialize.o \
           src/serialize/serialize_buildstack.o \
           src/serialize/serialize_helper.o \
           src/serialize/serialize_link.o src/serialize/serialize.o \
           src/serialize/serialize_stack.o \
           src/serialize/serialize_symboltable.o \
           src/serialize/serialize_filenames.o \
           src/serialize/add_function_body.o

SCANP= src/scanparse/y.tab.o src/scanparse/lex.yy.o \
       src/scanparse/scnprs.o src/scanparse/handle_dots.o \
       src/scanparse/resolvepragma.o

PRINT= src/print/print.o src/print/convert.o

FLATTEN= src/flatten/flatten.o src/flatten/lac2fun.o \
         src/flatten/fun2lac.o src/flatten/insert_vardec.o \
         src/flatten/handle_mops.o src/flatten/UndoSSATransform.o \
         src/flatten/ToNewTypes.o src/flatten/SSATransform.o \
         src/flatten/ssa.o src/flatten/WLPartitionGeneration.o \
         src/flatten/WLEnhancement.o src/flatten/ExplicitAccumulate.o \
         src/flatten/ToOldTypes.o src/flatten/wlanalysis.o \
         src/flatten/lacinlining.o src/flatten/wldefaultpartition.o \
         src/flatten/while2do.o src/flatten/handle_condexpr.o \
         src/flatten/codesimplification.o

CONSTANTS= src/constants/shape.o src/constants/constants_basic.o \
           src/constants/constants_struc_ops.o \
           src/constants/constants_ari_ops.o  src/constants/cv2cv.o \
           src/constants/cv2scalar.o src/constants/cv2str.o \
           src/constants/zipcv.o src/constants/basecv.o \
           src/constants/constants_serialize.o

TYPECHECK= src/typecheck/gen_pseudo_fun.o \
           src/typecheck/new_typecheck.o src/typecheck/new_types.o \
           src/typecheck/user_types.o src/typecheck/create_wrappers.o \
           src/typecheck/ssi.o src/typecheck/sig_deps.o src/typecheck/ct_prf.o \
           src/typecheck/ct_basic.o src/typecheck/ct_fun.o \
           src/typecheck/ct_with.o src/typecheck/type_errors.o \
           src/typecheck/specialize.o src/typecheck/new2old.o \
           src/typecheck/create_wrapper_code.o \
           src/typecheck/type_statistics.o \
           src/typecheck/type_utils.o \
           src/typecheck/resolvesymboltypes.o \
           src/typecheck/split_wrappers.o \
           src/typecheck/dispatchfuncalls.o \
           src/typecheck/insert_type_conv.o \
           src/typecheck/update_wrapper_type.o

OPTIMIZE= src/optimize/optimize.o \
          src/optimize/DeadFunctionRemoval.o \
          src/optimize/AssociativeLaw.o src/optimize/deadcodeinference.o \
          src/optimize/deadcoderemoval.o src/optimize/SSACSE.o \
          src/optimize/SSAConstantFolding.o src/optimize/SSALIR.o \
          src/optimize/SSALUR.o src/optimize/SSAInferLI.o \
          src/optimize/SSAWLUnroll.o src/optimize/rmcasts.o \
          src/optimize/ElimSubDiv.o \
          src/optimize/UndoElimSubDiv.o src/optimize/type_upgrade.o \
          src/optimize/ConstVarPropagation.o src/optimize/DistributiveLaw.o \
          src/optimize/signature_simplification.o src/optimize/inlining.o \
          src/optimize/inferneedcounters.o src/optimize/associativity.o \
          src/optimize/elimtypeconv.o src/optimize/liftoptflags.o \
          src/optimize/wlsimplification.o src/optimize/prfunroll.o

PROFILE= src/profile/annotate_fun_calls.o

PSIOPT= src/psi-opt/index_eliminate.o src/psi-opt/index_infer.o \
        src/psi-opt/index_optimize.o \
        src/psi-opt/wl_access_analyze.o src/psi-opt/tile_size_inference.o \
        src/psi-opt/wls.o src/psi-opt/wlscheck.o \
        src/psi-opt/wlsbuild.o src/psi-opt/wlswithloopification.o \
        src/psi-opt/SSAWithloopFolding.o src/psi-opt/SSAWLT.o \
        src/psi-opt/SSAWLI.o src/psi-opt/SSAWLF.o \
        src/psi-opt/pad.o src/psi-opt/pad_collect.o src/psi-opt/pad_infer.o \
        src/psi-opt/pad_transform.o src/psi-opt/pad_info.o \
        src/psi-opt/WithloopFusion.o src/psi-opt/detectdependencies.o \
        src/psi-opt/tagdependencies.o src/psi-opt/resolvedependencies.o

MODULES= src/modules/symboltable.o \
         src/modules/stringset.o src/modules/libstat.o \
         src/modules/modulemanager.o src/modules/libmanager.o \
         src/modules/export.o src/modules/ccmanager.o \
         src/modules/libbuilder.o src/modules/resolveall.o \
         src/modules/annotatenamespace.o src/modules/usesymbols.o \
         src/modules/prepareinline.o src/modules/dependencies.o \
         src/modules/importsymbols.o src/modules/gatherdependencies.o \
         src/modules/namespaces.o src/modules/addspecialfuns.o

OBJECTS= src/objects/objinit.o src/objects/objects.o \
         src/objects/uniquecheck.o src/objects/objanalysis.o

REFCOUNT= src/refcount/allocation.o src/refcount/rcopt.o \
          src/refcount/rcphase.o src/refcount/filterrc.o \
          src/refcount/alloc.o src/refcount/referencecounting.o \
          src/refcount/reuse.o src/refcount/aliasanalysis.o \
          src/refcount/staticreuse.o src/refcount/reuseelimination.o \
          src/refcount/interfaceanalysis.o src/refcount/loopreuseopt.o \
          src/refcount/datareuse.o src/refcount/explicitcopy.o \
          src/refcount/reusebranching.o src/refcount/ReuseWithArrays.o \
          src/refcount/inplacecomp.o src/refcount/audscldist.o \
          src/refcount/rcminimize.o src/refcount/NumLookUpTable.o \
          src/refcount/wrci.o src/refcount/reusewithoffset.o

CONCURRENT= src/concurrent/concurrent.o src/concurrent/spmd_init.o  \
            src/concurrent/spmd_lift.o src/concurrent/sync_init.o \
            src/concurrent/sync_opt.o src/concurrent/schedule.o \
            src/concurrent/spmd_trav.o src/concurrent/spmd_emm.o

MULTITHREAD= src/multithread/multithread.o \
             src/multithread/multithread_lib.o \
             src/multithread/tag_executionmode.o \
             src/multithread/propagate_executionmode.o \
             src/multithread/create_cells.o \
             src/multithread/create_dataflowgraph.o \
             src/multithread/assignments_rearrange.o \
             src/multithread/cell_growth.o \
             src/multithread/create_withinwith.o \
             src/multithread/replicate_functions.o \
             src/multithread/consolidate_cells.o

COMPILE= src/compile/wlpragma_funs.o src/compile/wltransform.o \
         src/compile/precompile.o src/compile/gen_startup_code.o \
         src/compile/compile.o src/compile/icm2c.o src/compile/icm2c_basic.o \
         src/compile/icm2c_utils.o src/compile/icm2c_std.o \
         src/compile/icm2c_prf.o src/compile/markmemvals.o \
         src/compile/icm2c_mt.o src/compile/icm2c_sched.o \
         src/compile/icm2c_wl.o src/compile/icm2c_error.o \
         src/compile/renameidentifiers.o src/compile/functionprecompile.o \
         src/compile/typeconv_precompile.o  src/compile/setlinksign.o \
         src/compile/remove_external_code.o src/compile/wlidxs.o \
         src/compile/simd_infer.o src/compile/mark_noop_grids.o

CINTERFACE=
#CINTERFACE= src/c-interface/map_cwrapper.o \
                #src/c-interface/print_interface.o \
                #src/c-interface/import_specialization.o \
                #src/c-interface/print_interface_header.o \
                #src/c-interface/print_interface_wrapper.o

OBJ:= $(GLOBAL) $(TREE) $(SCANP) $(PRINT) $(FLATTEN) $(TYPECHECK) $(OPTIMIZE) \
     $(MODULES) $(OBJECTS) $(REFCOUNT) $(COMPILE) $(PSIOPT) $(CONCURRENT) \
     $(MULTITHREAD) $(CINTERFACE) $(CONSTANTS) $(PROFILE) $(SERIALIZE)

#
#  Rules section
#

.PHONY: all efence product check_os dummy prod clean tar floppy distrib distrib_product linux 

all: check_os tools/bin/cse dummy sac2c

efence: check_os tools/bin/cse dummy sac2c.efence

product: check_os tools/bin/cse clean prod sac2c.prod

distrib_product: check_os tools/bin/cse prod sac2c.prod

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
	(cd src/xml; $(MAKE_NORM) )
	(cd src/scanparse; $(MAKE_NORM) )
	(cd src/global; $(MAKE_NORM) )
	(cd src/tree; $(MAKE_NORM) )
	(cd src/serialize; $(MAKE_NORM) )
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
	(cd src/xml; $(MAKE_PROD) )
	(cd src/scanparse; $(MAKE_PROD) )
	(cd src/global; $(MAKE_PROD) )
	(cd src/tree; $(MAKE_PROD) )
	(cd src/serialize; $(MAKE_PROD) )
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
	$(LIBTOOL) $(CC) $(CCFLAGS) $(CFLAGS) -o sac2c $(OBJ) $(LIB) $(LIBS) $(LDDYNFLAG)

libsac2c: sac2c
	$(MAKE) -C src/libsac libsac2c.so

sac2c.efence: $(OBJ) $(LIB)
	$(LIBTOOL) $(CC) $(CCFLAGS) $(CFLAGS) -o sac2c.efence $(OBJ) $(LIB) $(LIBS) $(EFLIBS) $(LDDYNFLAG)

sac2c.prod:  $(OBJ) $(LIB)
	$(LIBTOOL) $(CCPROD) $(CCPROD_FLAGS) $(CPROD_FLAGS) -o sac2c $(OBJ) $(LIB) $(LIBS) $(LDDYNFLAG)

doxygen:
	doxygen sac2cdoxy

deps:
	$(ECHO) "make deps is obsolete"

clean:
	(cd lib/src; $(MAKE_CLEAN) )
	(cd src/xml; $(MAKE_CLEAN) )
	(cd src/scanparse; $(MAKE_CLEAN) )
	(cd src/global; $(MAKE_CLEAN) )
	(cd src/tree; $(MAKE_CLEAN) )
	(cd src/print; $(MAKE_CLEAN) )
	(cd src/flatten; $(MAKE_CLEAN) )
	(cd src/typecheck; $(MAKE_CLEAN) )
	(cd src/serialize; $(MAKE_CLEAN) )
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


tar: src.tar.gz

src.tar.gz: 
	$(TAR) -cvf src.tar $(shell svn -R list)
	$(GZIP) src.tar

distrib:
	(cd distrib/src; $(MAKE))

tags: 
	ctags src/*/*.[ch] >/dev/null

$(PROJECT_ROOT)/Makefile.Config: $(PROJECT_ROOT)/Makefile.Config.in
	(cd $(PROJECT_ROOT); ./configure)

configure: configure.ac
	svn lock configure src/global/config.h.in
	autoconf
	autoheader
	svn commit configure src/global/config.h.in



