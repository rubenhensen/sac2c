
#
# $Log$
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

GLOBAL= src/global/main.o src/global/Error.o src/global/usage.o \
        src/global/internal_lib.o src/global/globals.o \
        src/global/resource.o src/global/build.o src/global/interrupt.o \
        src/global/options.o src/global/NameTuplesUtils.o\
        src/global/filemgr.o

TREE= src/tree/traverse.o src/tree/tree_basic.o src/tree/free.o \
      src/tree/tree_compound.o src/tree/DupTree.o src/tree/LookUpTable.o \
      src/tree/DataFlowMask.o src/tree/DataFlowMaskUtils.o \
      src/tree/InferDFMs.o src/tree/cleanup_decls.o src/tree/adjust_ids.o \
      src/tree/change_signature.o src/tree/compare_tree.o \
      src/tree/scheduling.o src/tree/wl_bounds.o \
      src/tree/node_basic.o src/tree/free_node.o \
      src/tree/free_attribs.o src/tree/traverse_tables.o src/tree/traverse_helper.o

SERIALIZE= src/serialize/serialize_node.o \
           src/serialize/serialize_attribs.o src/serialize/deserialize.o \
           src/serialize/serialize_buildstack.o src/serialize/serialize_helper.o \
           src/serialize/serialize_link.o src/serialize/serialize.o \
           src/serialize/serialize_stack.o

SCANP= src/scanparse/y.tab.o src/scanparse/lex.yy.o \
       src/scanparse/scnprs.o src/scanparse/handle_dots.o \
       src/scanparse/resolvepragma.o

PRINT= src/print/print.o src/print/convert.o

FLATTEN= src/flatten/flatten.o src/flatten/lac2fun.o \
         src/flatten/fun2lac.o src/flatten/insert_vardec.o \
         src/flatten/handle_mops.o src/flatten/UndoSSATransform.o \
         src/flatten/ToNewTypes.o src/flatten/SSATransform.o \
         src/flatten/ssa.o src/flatten/WLPartitionGeneration.o \
	 src/flatten/WLEnhancement.o src/flatten/ExplicitAccumulate.o

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
           src/typecheck/type_utils.o

OPTIMIZE= src/optimize/optimize.o \
          src/optimize/DeadFunctionRemoval.o \
	  src/optimize/Inline.o \
          src/optimize/AssociativeLaw.o \
          src/optimize/SSADeadCodeRemoval.o src/optimize/SSACSE.o \
          src/optimize/SSAConstantFolding.o src/optimize/SSALIR.o \
          src/optimize/SSALUR.o src/optimize/SSAInferLI.o \
          src/optimize/SSAWLUnroll.o src/optimize/rmcasts.o \
          src/optimize/ElimSubDiv.o \
          src/optimize/UndoElimSubDiv.o \
          src/optimize/ConstVarPropagation.o src/optimize/DistributiveLaw.o

PROFILE= src/profile/annotate_fun_calls.o

PSIOPT= src/psi-opt/index.o src/psi-opt/ArrayElimination.o \
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
         src/modules/prepareinline.o src/modules/dependencies.o 

OBJECTS= src/objects/objinit.o src/objects/objects.o \
         src/objects/uniquecheck.o src/objects/objanalysis.o

REFCOUNT= src/refcount/allocation.o src/refcount/rcopt.o \
          src/refcount/rcphase.o src/refcount/filterrc.o \
          src/refcount/alloc.o src/refcount/refcounting.o \
          src/refcount/reuse.o src/refcount/aliasanalysis.o \
          src/refcount/staticreuse.o src/refcount/reuseelimination.o \
          src/refcount/interfaceanalysis.o src/refcount/loopreuseopt.o \
          src/refcount/datareuse.o src/refcount/explicitcopy.o \
          src/refcount/reusebranching.o src/refcount/ReuseWithArrays.o

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
         src/compile/typeconv_precompile.o 

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
	$(CC) $(CCFLAGS) $(CFLAGS) -o sac2c $(OBJ) $(LIB) $(LIBS) $(LDDYNFLAG)

sac2c.efence: $(OBJ) $(LIB)
	$(CC) $(CCFLAGS) $(CFLAGS) -o sac2c.efence $(OBJ) $(LIB) $(LIBS) $(EFLIBS) $(LDDYNFLAG)

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


