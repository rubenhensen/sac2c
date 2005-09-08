
#
# $Log$
# Revision 3.193  2005/09/08 11:02:12  sbs
# update_wrapper_type.o added.
#
# Revision 3.192  2005/09/04 12:48:24  ktr
# added map_fun_trav.o
#
# Revision 3.191  2005/09/02 17:45:28  sah
# added map_lac_funs.o
#
# Revision 3.190  2005/09/02 14:26:00  ktr
# added src/optimize/liftoptargs.o
#
# Revision 3.189  2005/08/24 10:31:44  ktr
# added src/compile/wlidx.o
#
# Revision 3.188  2005/08/20 12:05:31  ktr
# added src/optimize/elimtypeconv.o
#
# Revision 3.187  2005/08/19 18:15:29  ktr
# added src/optimize/inferneedcounters.o, src/optimize/associativity.o
#
# Revision 3.186  2005/08/18 07:00:57  sbs
# insert_type_conv.o added.
#
# Revision 3.185  2005/08/02 14:23:38  ktr
# added src/optimize/deadcodeinference.o
#
# Revision 3.184  2005/07/25 16:57:40  sbs
# private_heap.o added.
#
# Revision 3.183  2005/07/22 13:10:52  sah
# extracted some functionality from
# deserialize into add_function_body
#
# Revision 3.182  2005/07/21 14:18:54  sah
# introduced remove_external_code
#
# Revision 3.181  2005/07/19 16:57:40  ktr
# replaced SSADeadCodeRemoval with deadcoderemoval.
#
# Revision 3.180  2005/07/17 20:12:35  sbs
# split_wrappers.o added.
#
# Revision 3.179  2005/07/17 11:46:54  sah
# added serialize_filenames.o
#
# Revision 3.178  2005/07/16 19:08:19  sbs
# index_infer.o added.
#
# Revision 3.177  2005/07/16 18:33:36  sah
# cleanup
#
# Revision 3.176  2005/07/16 10:01:12  ktr
# added src/refcount/referencecounting.o
#
# Revision 3.175  2005/07/15 15:52:18  sah
# splitted create_wrapper_code and dispatchfuncalls
# introduced namespaces
#
# Revision 3.174  2005/07/03 17:00:43  ktr
# added some files
#
# Revision 3.173  2005/06/30 16:39:07  ktr
# added src/refcount/audscldist.o
#
# Revision 3.172  2005/06/28 20:57:36  cg
# Added while2do.[ch] and handle_condexpr.[ch]
#
# Revision 3.171  2005/06/15 10:54:19  sbs
# libsac2c rule added
# could have deps in all, sac2c.prod but considered overhead during development
#
# Revision 3.170  2005/06/14 12:27:35  sah
# use libtool to build sac2c
#
# Revision 3.169  2005/06/14 08:52:04  khf
# added wldefaultpartition.o
#
# Revision 3.168  2005/06/06 10:18:34  jhb
# added checktst.o
#
# Revision 3.167  2005/06/01 16:59:05  sah
# separated annotating namespaces and gathering dependencies in two
# phase to allow for reusing the gathering phase to print the
# dependencies of a module.
#
# Revision 3.166  2005/05/31 18:16:14  sah
# added resolve symbol types phase
#
# Revision 3.165  2005/05/22 19:45:53  sah
# added first implementation steps for import
#
# Revision 3.164  2005/05/13 16:36:11  ktr
# added flatten/lacinlining.o
#
# Revision 3.163  2005/04/29 20:31:11  khf
# added src/flatten/wlanalysis.o
#
# Revision 3.162  2005/04/20 19:10:23  ktr
# removed src/optimize/Inline.o
#
# Revision 3.161  2005/03/10 09:41:09  cg
# Added linking of phase.o and setup.o.
#
# Revision 3.160  2005/02/14 11:25:52  cg
# Added linking of prepare_inlining and inlining.
#
# Revision 3.159  2005/02/11 15:02:37  jhb
# added check.o check_lib.o
#
# Revision 3.158  2005/02/02 18:09:03  mwe
# signature_simplification added
#
# Revision 3.157  2005/01/07 16:47:26  cg
# added linking of ctinfo.o
#
# Revision 3.156  2004/12/16 14:36:16  ktr
# added src/refcount/inplacecomp.o
#
# Revision 3.155  2004/12/08 12:15:47  mwe
# type_upgrade.o added
#
# Revision 3.154  2004/11/29 15:01:50  ktr
# added xml-dir to target clean
#
# Revision 3.153  2004/11/29 14:41:57  sah
# added setlinksign traversal
#
# Revision 3.152  2004/11/29 09:35:04  sbs
# xml dir added
#
# Revision 3.151  2004/11/27 03:19:10  sbs
# AEAETSCH!!!!!
#
# Revision 3.150  2004/11/27 02:37:33  sbs
# typeconversions renamed .
#
# Revision 3.149  2004/11/27 02:22:25  sbs
# *** empty log message ***
#
# Revision 3.148  2004/11/27 00:06:54  sbs
# *** empty log message ***
#
# Revision 3.147  2004/11/27 00:03:05  sbs
# traverse_tables.o added
#
# Revision 3.146  2004/11/26 23:00:13  sbs
# adjusted to the SacDevCamp changes
#
# Revision 3.145  2004/11/24 09:52:25  sbs
# type_utils.o added
#
# Revision 3.144  2004/11/23 12:34:08  cg
# Removed some files.
#
# Revision 3.143  2004/11/21 11:22:03  sah
# removed some old ast infos
#
# Revision 3.142  2004/11/19 21:01:58  sah
# added objanalysis.o
#
# Revision 3.141  2004/11/18 15:18:03  mwe
# ChackAvis changed to ToNewTypes
#
# Revision 3.140  2004/11/17 19:50:56  sah
# made src/modules/implicittypes.o invisible in
# new ast mode
#
# Revision 3.139  2004/11/14 13:42:15  ktr
# added src/refcount/reusebranching.o
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
          src/optimize/elimtypeconv.o src/optimize/liftoptflags.o

PROFILE= src/profile/annotate_fun_calls.o

PSIOPT= src/psi-opt/index.o src/psi-opt/index_infer.o src/psi-opt/ArrayElimination.o \
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
         src/modules/namespaces.o

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
          src/refcount/rcminimize.o src/refcount/NumLookUpTable.o

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
         src/compile/remove_external_code.o src/compile/wlidxs.o

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
	$(LIBTOOL) $(CCPROD) $(CCPROD_FLAGS) $(CPROD_FLAGS) -o sac2c $(OBJ) $(LIB) $(LIBS)

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


