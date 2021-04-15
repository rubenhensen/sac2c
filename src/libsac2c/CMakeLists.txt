
# Get a list of subdirectories containing *.h files into the variable HEADER_LIST
HEADER_DIRECTORIES (HEADER_LIST)

# Add each directory to include_dirs.
FOREACH (d ${HEADER_LIST})
  INCLUDE_DIRECTORIES ("${d}")
ENDFOREACH (d)

INCLUDE_DIRECTORIES ("${CMAKE_CURRENT_BINARY_DIR}")
INCLUDE_DIRECTORIES (${ISL_INC_PATH} ${BARVINOK_INC_PATH})


SET (COMMON_XSL
  ${CMAKE_CURRENT_SOURCE_DIR}/xml/common_accessor_macros.xsl
  ${CMAKE_CURRENT_SOURCE_DIR}/xml/common_node_access.xsl
  ${CMAKE_CURRENT_SOURCE_DIR}/xml/common_make_assertion.xsl
  ${CMAKE_CURRENT_SOURCE_DIR}/xml/common_make_head.xsl
  ${CMAKE_CURRENT_SOURCE_DIR}/xml/common_travfun.xsl
  ${CMAKE_CURRENT_SOURCE_DIR}/xml/common_make_head_checkmem.xsl
  ${CMAKE_CURRENT_SOURCE_DIR}/xml/common_key_tables.xsl
  ${CMAKE_CURRENT_SOURCE_DIR}/xml/common_name_to_nodeenum.xsl
  ${CMAKE_CURRENT_SOURCE_DIR}/xml/common_make_body.xsl
  ${CMAKE_CURRENT_SOURCE_DIR}/xml/common_c_code.xsl
)

SET (XSL_FILES
  types/types_trav.h
  types/types_nodetype.h
  tree/free_node.c
  tree/check_reset.h
  tree/node_basic.h
  tree/free_attribs.h
  tree/free_node.h
  tree/attribs.h
  tree/traverse_helper.c
  tree/sons.h
  tree/traverse_tables.h
  tree/check.c
  tree/node_alloc.h
  tree/traverse_tables.c
  tree/check.h
  tree/check_reset.c
  #tree/check_attribs.h
  tree/check_node.h
  tree/check_node.c
  tree/node_basic.c
  serialize/serialize_node.c
  serialize/serialize_link.c
  serialize/serialize_node.h
  serialize/serialize_buildstack.h
  serialize/serialize_helper.c
  serialize/serialize_link.h
  serialize/serialize_attribs.h
  serialize/serialize_buildstack.c
  global/node_info.mac
)

# For every *.{c,h,mac}.xsl file add a custom command to generate this file
# and store the name of generated files in the XSL_DONE_FILES
SET(XSL_DONE_FILES)
FOREACH(name ${XSL_FILES})
  SET(src "${CMAKE_CURRENT_SOURCE_DIR}/${name}.xsl")
  GET_FILENAME_COMPONENT (dst ${name} NAME)
  SET (dst "${CMAKE_CURRENT_BINARY_DIR}/${dst}")
  ADD_CUSTOM_COMMAND(
    OUTPUT "${dst}"
    # FIXME(artem) it also depends on the bunch of xsl files...
    DEPENDS "${CB}" "${src}" "${CMAKE_CURRENT_SOURCE_DIR}/xml/ast.xml"
    COMMAND
    ${XSLT_EXEC} ${src}
                 ${CMAKE_CURRENT_SOURCE_DIR}/xml/ast.xml
                 | ${CB} > ${dst}
    COMMENT "Generating ${dst}"
  )
  LIST (APPEND XSL_DONE_FILES "${dst}")
ENDFOREACH(name)


# Add custom command to produce fancy_icm_vars.c
ADD_CUSTOM_COMMAND(
  OUTPUT
  "${CMAKE_CURRENT_BINARY_DIR}/fancy_icm_vars.c"
  DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm_vars.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm.data"
  COMMAND
    # FIXME sort and uniq are unix-specific, we can do use CMake tools for sort
    # and uniq.
    ${CMAKE_C_COMPILER} -E -P ${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm_vars.c  -DICM_ALL
    | sort
    | uniq > "${CMAKE_CURRENT_BINARY_DIR}/fancy_icm_vars.c"
  COMMENT "Generating ${CMAKE_CURRENT_BINARY_DIR}/fancy_icm_vars.c"
)


# Create a dependency for the file where fancy_icm_vars are used.
SET_SOURCE_FILES_PROPERTIES(
    "${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c.c"
    PROPERTIES
    OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/fancy_icm_vars.c")

SET (SAC2C_SRC
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/SSAWLF.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/SSAWLI.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/SSAWithloopFolding.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/WithloopFusion.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/algebraic_wlf.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/algebraic_wlfi.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/copywlelim.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/cubeslicer.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/detectdependencies.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/eliminate_duplicate_fundef_args.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/indexvectorutils.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/isl_utilities.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/ive_reusewl_and_scalarize.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/ive_split_loop_invariants.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/ive_split_selections.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/ivexcleanup.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/ivexpropagation.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/ivextrema.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/lacfun_utilities.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/loop_and_cond_scalarization_in.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/loop_and_cond_scalarization_out.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/loop_scalarization.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/narray_utilities.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/pad.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/pad_collect.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/pad_infer.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/pad_info.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/pad_transform.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/polyhedral_loop_unrolling.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/polyhedral_setup.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/polyhedral_utilities.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/polyhedral_wlf.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/propagate_extrema_thru_lacfuns.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/resolvedependencies.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/set_withloop_depth.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/tagdependencies.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/tile_size_inference.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/with_loop_utilities.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/withloop_flattening.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/wl_access_analyze.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/wl_cost_check.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/wl_needcount.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/wlpropagation.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/wls.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/wlsbuild.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/wlscheck.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/wlsimplification.c
${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/wlswithloopification.c
${CMAKE_CURRENT_SOURCE_DIR}/cinterface/bundle_to_fundef.c
${CMAKE_CURRENT_SOURCE_DIR}/cinterface/construct_bundles.c
${CMAKE_CURRENT_SOURCE_DIR}/cinterface/create_c_wrapper_body.c
${CMAKE_CURRENT_SOURCE_DIR}/cinterface/create_f_wrapper_header.c
${CMAKE_CURRENT_SOURCE_DIR}/cinterface/generate_copy_and_free.c
${CMAKE_CURRENT_SOURCE_DIR}/cinterface/load_module_contents.c
${CMAKE_CURRENT_SOURCE_DIR}/cinterface/print_ccflags.c
${CMAKE_CURRENT_SOURCE_DIR}/cinterface/print_ldflags.c
${CMAKE_CURRENT_SOURCE_DIR}/codegen/compile.c
${CMAKE_CURRENT_SOURCE_DIR}/codegen/gen_startup_code.c
${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c.c
${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_basic.c
${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_cuda.c
${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_error.c
${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_fp.c
${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_ia.c
${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_mt.c
${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_nested.c
${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_prf.c
${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_rtspec.c
${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_sched.c
${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_std.c
${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_utils.c
${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_wl.c
${CMAKE_CURRENT_SOURCE_DIR}/codegen/regression.c
${CMAKE_CURRENT_SOURCE_DIR}/codegen/tag_preparation.c
${CMAKE_CURRENT_SOURCE_DIR}/concurrent/annotate_scheduling.c
${CMAKE_CURRENT_SOURCE_DIR}/concurrent/cost_model.c
${CMAKE_CURRENT_SOURCE_DIR}/concurrent/create_mtst_funs.c
${CMAKE_CURRENT_SOURCE_DIR}/concurrent/create_mtst_funs_module.c
${CMAKE_CURRENT_SOURCE_DIR}/concurrent/create_spmd_funs.c
${CMAKE_CURRENT_SOURCE_DIR}/concurrent/mtdcr.c
${CMAKE_CURRENT_SOURCE_DIR}/concurrent/remove_propagates.c
${CMAKE_CURRENT_SOURCE_DIR}/concurrent/restore_mem_instr.c
${CMAKE_CURRENT_SOURCE_DIR}/concurrent/restore_mtst_funs.c
${CMAKE_CURRENT_SOURCE_DIR}/concurrent/scheduling.c
${CMAKE_CURRENT_SOURCE_DIR}/concurrent/set_spmd_linksign.c
${CMAKE_CURRENT_SOURCE_DIR}/constants/basecv.c
${CMAKE_CURRENT_SOURCE_DIR}/constants/constants_ari_ops.c
${CMAKE_CURRENT_SOURCE_DIR}/constants/constants_basic.c
${CMAKE_CURRENT_SOURCE_DIR}/constants/constants_serialize.c
${CMAKE_CURRENT_SOURCE_DIR}/constants/constants_struc_ops.c
${CMAKE_CURRENT_SOURCE_DIR}/constants/cv2cv.c
${CMAKE_CURRENT_SOURCE_DIR}/constants/cv2scalar.c
${CMAKE_CURRENT_SOURCE_DIR}/constants/cv2str.c
${CMAKE_CURRENT_SOURCE_DIR}/constants/shape.c
${CMAKE_CURRENT_SOURCE_DIR}/constants/zipcv.c
${CMAKE_CURRENT_SOURCE_DIR}/constraints/constraint_statistics.c
${CMAKE_CURRENT_SOURCE_DIR}/constraints/generate_guard_solvers.c
${CMAKE_CURRENT_SOURCE_DIR}/constraints/insert_conformity_checks.c
${CMAKE_CURRENT_SOURCE_DIR}/constraints/insert_domain_constraints.c
${CMAKE_CURRENT_SOURCE_DIR}/constraints/polyhedral_guard_optimization.c
${CMAKE_CURRENT_SOURCE_DIR}/constraints/strip_conformity_checks.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/adjust_shmem_access.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/adjust_stknl_rets.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/async_delay.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/async_delay_expand.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/annotate_cond_transfers.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/annotate_cuda_partition.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/annotate_cuda_withloop2.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/annotate_memory_transfers.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/create_cond_fun.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/create_constant_assignments.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/create_cuda_kernels.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/create_loop_fun.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/cuda_create_cells.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/cuda_data_reuse.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/cuda_sink_code.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/cuda_tag_executionmode.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/cuda_utils.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/data_access_analysis.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/expand_shmem_boundary_load.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/infer_reusable_arrays.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/insert_cudast_memtran.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/insert_withloop_memtran.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/kernel_post_processing.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/minimize_block_transfers2.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/minimize_cond_transfers.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/minimize_cudast_transfers.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/minimize_loop_transfers.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/minimize_emr_transfers.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/minimize_transfers.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/managed_memory.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/partial_fold.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/prepare_forloop_generation.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/prepare_kernel_generation.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/remove_unused_lac.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/shared_memory_reuse.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/single_thread_kernels.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/split_cuda_fold.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/split_cuda_withloop.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/split_partitions.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/gpukernel_check_funs.c
${CMAKE_CURRENT_SOURCE_DIR}/cuda/gpukernel_comp_funs.c
${CMAKE_CURRENT_SOURCE_DIR}/cudahybrid/change_arrays_to_dist.c
${CMAKE_CURRENT_SOURCE_DIR}/cudahybrid/create_dist_wl_cond.c
${CMAKE_CURRENT_SOURCE_DIR}/cudahybrid/cuda_cost_model.c
${CMAKE_CURRENT_SOURCE_DIR}/cudahybrid/infer_memory_accesses.c
${CMAKE_CURRENT_SOURCE_DIR}/cudahybrid/insert_memtrans_dist.c
${CMAKE_CURRENT_SOURCE_DIR}/cudahybrid/introduce_availability_loop.c
${CMAKE_CURRENT_SOURCE_DIR}/cudahybrid/prepare_dist_scheduler.c
${CMAKE_CURRENT_SOURCE_DIR}/distmem/identify_side_effect_function_aps.c
${CMAKE_CURRENT_SOURCE_DIR}/distmem/identify_side_effect_functions.c
${CMAKE_CURRENT_SOURCE_DIR}/distmem/identify_suballocated_arrays.c
${CMAKE_CURRENT_SOURCE_DIR}/distmem/mark_local_selects.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/ExplicitAccumulate.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/SSATransform.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/UndoSSATransform.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/WLPartitionGeneration.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/flatten.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/flattengenerators.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/fun2lac.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/handle_condexpr.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/handle_mops.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/insert_udc_funbody.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/insert_vardec.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/lac2fun.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/lacinlining.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/undossaivtransform.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/while2do.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/wlanalysis.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/wlbounds2structconsts.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/wldefaultpartition.c
${CMAKE_CURRENT_SOURCE_DIR}/flatten/ssawithid.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/binheap.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/ctransitive.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/dag.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/dfwalk.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/dynarray.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/dynelem.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/dynmatrix.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/elemstack.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/graphtypes.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/graphutils.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/lub.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/lubcross.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/lubtree.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/mineq.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/query.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/reachhelper.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/reachlabel.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/tfprintutils.c
${CMAKE_CURRENT_SOURCE_DIR}/flexsub/topo.c
${CMAKE_CURRENT_SOURCE_DIR}/funcpara/add_sync.c
${CMAKE_CURRENT_SOURCE_DIR}/funcpara/count_spawn_sync.c
${CMAKE_CURRENT_SOURCE_DIR}/funcpara/live_variable_analysis.c
${CMAKE_CURRENT_SOURCE_DIR}/funcpara/make_slow_clones.c
${CMAKE_CURRENT_SOURCE_DIR}/funcpara/move_sync_statement.c
${CMAKE_CURRENT_SOURCE_DIR}/funcpara/tag_fp_aps.c
${CMAKE_CURRENT_SOURCE_DIR}/funcpara/tag_fp_fundefs.c
${CMAKE_CURRENT_SOURCE_DIR}/generics/check_and_simplify_generic_definitions.c
${CMAKE_CURRENT_SOURCE_DIR}/generics/generate_generic_type_conversions.c
${CMAKE_CURRENT_SOURCE_DIR}/generics/remove_generic_definitions.c
${CMAKE_CURRENT_SOURCE_DIR}/global/NameTuplesUtils.c
${CMAKE_CURRENT_SOURCE_DIR}/global/ctinfo.c
${CMAKE_CURRENT_SOURCE_DIR}/global/filemgr.c
${CMAKE_CURRENT_SOURCE_DIR}/global/globals.c
${CMAKE_CURRENT_SOURCE_DIR}/global/main.c
${CMAKE_CURRENT_SOURCE_DIR}/global/memory.c
${CMAKE_CURRENT_SOURCE_DIR}/global/options.c
${CMAKE_CURRENT_SOURCE_DIR}/global/phase.c
${CMAKE_CURRENT_SOURCE_DIR}/global/phase_drivers.c
${CMAKE_CURRENT_SOURCE_DIR}/global/phase_info.c
${CMAKE_CURRENT_SOURCE_DIR}/global/phase_options.c
${CMAKE_CURRENT_SOURCE_DIR}/global/private_heap.c
${CMAKE_CURRENT_SOURCE_DIR}/global/profiler.c
${CMAKE_CURRENT_SOURCE_DIR}/global/resource.c
${CMAKE_CURRENT_SOURCE_DIR}/global/printable_target_functions.c
${CMAKE_CURRENT_SOURCE_DIR}/global/usage.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/NumLookUpTable.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/ReuseWithArrays.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/aliasanalysis.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/alloc.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/audscldist.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/datareuse.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/emr_candidate_inference.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/emr_loop_optimisation.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/emr_utils.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/explicitcopy.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/filter_partial_reuse_candidates.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/filterrc.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/identify_noop_branch.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/inplacecomp.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/interfaceanalysis.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/loopallocopt.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/loopreuseopt.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/movesharedmeminstr.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/polyhedral_reuse_analysis.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/rcminimize.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/rcopt.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/referencecounting.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/reuse.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/reusebranching.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/reuseelimination.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/reusewithoffset.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/rm_alias_results_cc.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/scoperce.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/staticreuse.c
${CMAKE_CURRENT_SOURCE_DIR}/memory/wrci.c
${CMAKE_CURRENT_SOURCE_DIR}/modules/addspecialfuns.c
${CMAKE_CURRENT_SOURCE_DIR}/modules/annotatenamespace.c
${CMAKE_CURRENT_SOURCE_DIR}/modules/cctools.c
${CMAKE_CURRENT_SOURCE_DIR}/modules/dependencies.c
${CMAKE_CURRENT_SOURCE_DIR}/modules/export.c
${CMAKE_CURRENT_SOURCE_DIR}/modules/gatherdependencies.c
${CMAKE_CURRENT_SOURCE_DIR}/modules/importsymbols.c
${CMAKE_CURRENT_SOURCE_DIR}/modules/libmanager.c
${CMAKE_CURRENT_SOURCE_DIR}/modules/libstat.c
${CMAKE_CURRENT_SOURCE_DIR}/modules/modulemanager.c
${CMAKE_CURRENT_SOURCE_DIR}/modules/namespaces.c
${CMAKE_CURRENT_SOURCE_DIR}/modules/prepareinline.c
${CMAKE_CURRENT_SOURCE_DIR}/modules/resolveall.c
${CMAKE_CURRENT_SOURCE_DIR}/modules/stringset.c
${CMAKE_CURRENT_SOURCE_DIR}/modules/symboltable.c
${CMAKE_CURRENT_SOURCE_DIR}/modules/usesymbols.c
${CMAKE_CURRENT_SOURCE_DIR}/multithread/abort.c
${CMAKE_CURRENT_SOURCE_DIR}/multithread/assignments_rearrange.c
${CMAKE_CURRENT_SOURCE_DIR}/multithread/cell_growth.c
${CMAKE_CURRENT_SOURCE_DIR}/multithread/consolidate_cells.c
${CMAKE_CURRENT_SOURCE_DIR}/multithread/create_cells.c
${CMAKE_CURRENT_SOURCE_DIR}/multithread/create_dataflowgraph.c
${CMAKE_CURRENT_SOURCE_DIR}/multithread/create_withinwith.c
${CMAKE_CURRENT_SOURCE_DIR}/multithread/multithread_lib.c
${CMAKE_CURRENT_SOURCE_DIR}/multithread/propagate_executionmode.c
${CMAKE_CURRENT_SOURCE_DIR}/multithread/replicate_functions.c
${CMAKE_CURRENT_SOURCE_DIR}/multithread/tag_executionmode.c
${CMAKE_CURRENT_SOURCE_DIR}/mutc/add_rc_mode_prf.c
${CMAKE_CURRENT_SOURCE_DIR}/mutc/create_function_pairs.c
${CMAKE_CURRENT_SOURCE_DIR}/mutc/create_spawn_funs.c
${CMAKE_CURRENT_SOURCE_DIR}/mutc/move_syncs.c
${CMAKE_CURRENT_SOURCE_DIR}/mutc/tag_fun_thread.c
${CMAKE_CURRENT_SOURCE_DIR}/nested/generate_nested_type_conversions.c
${CMAKE_CURRENT_SOURCE_DIR}/objects/check_uniqueness.c
${CMAKE_CURRENT_SOURCE_DIR}/objects/check_uniqueness_annotations.c
${CMAKE_CURRENT_SOURCE_DIR}/objects/extend_wrapper_types.c
${CMAKE_CURRENT_SOURCE_DIR}/objects/generate_object_initialiser.c
${CMAKE_CURRENT_SOURCE_DIR}/objects/manage_object_initialisers.c
${CMAKE_CURRENT_SOURCE_DIR}/objects/object_analysis.c
${CMAKE_CURRENT_SOURCE_DIR}/objects/object_init.c
${CMAKE_CURRENT_SOURCE_DIR}/objects/resolve_objects.c
${CMAKE_CURRENT_SOURCE_DIR}/objects/resolve_reference_args.c
${CMAKE_CURRENT_SOURCE_DIR}/objects/restore_objects.c
${CMAKE_CURRENT_SOURCE_DIR}/objects/restore_reference_args.c
${CMAKE_CURRENT_SOURCE_DIR}/objects/spmdfun_fix.c
${CMAKE_CURRENT_SOURCE_DIR}/objects/strip_external_signatures.c
${CMAKE_CURRENT_SOURCE_DIR}/platform/macos.c
${CMAKE_CURRENT_SOURCE_DIR}/precompile/dead_vardec_removal.c
${CMAKE_CURRENT_SOURCE_DIR}/precompile/distribute_threads.c
${CMAKE_CURRENT_SOURCE_DIR}/precompile/dynamic_memory_usage_inference.c
${CMAKE_CURRENT_SOURCE_DIR}/precompile/functionprecompile.c
${CMAKE_CURRENT_SOURCE_DIR}/precompile/lift_with3_bodies.c
${CMAKE_CURRENT_SOURCE_DIR}/precompile/mark_noop_grids.c
${CMAKE_CURRENT_SOURCE_DIR}/precompile/markmemvals.c
${CMAKE_CURRENT_SOURCE_DIR}/precompile/marksuballoc.c
${CMAKE_CURRENT_SOURCE_DIR}/precompile/move_const.c
${CMAKE_CURRENT_SOURCE_DIR}/precompile/omp_find_private.c
${CMAKE_CURRENT_SOURCE_DIR}/precompile/remove_external_code.c
${CMAKE_CURRENT_SOURCE_DIR}/precompile/remove_noop_branch.c
${CMAKE_CURRENT_SOURCE_DIR}/precompile/renameidentifiers.c
${CMAKE_CURRENT_SOURCE_DIR}/precompile/resolve_code_sharing.c
${CMAKE_CURRENT_SOURCE_DIR}/precompile/setlinksign.c
${CMAKE_CURRENT_SOURCE_DIR}/precompile/tag_main_fun_thread.c
${CMAKE_CURRENT_SOURCE_DIR}/precompile/typeconv_precompile.c
${CMAKE_CURRENT_SOURCE_DIR}/print/convert.c
${CMAKE_CURRENT_SOURCE_DIR}/print/print.c
${CMAKE_CURRENT_SOURCE_DIR}/profile/annotate_fun_calls.c
${CMAKE_CURRENT_SOURCE_DIR}/rtspec/runtime_compiler.c
${CMAKE_CURRENT_SOURCE_DIR}/rtspec/runtime_filtering.c
${CMAKE_CURRENT_SOURCE_DIR}/rtspec/runtime_function_id.c
${CMAKE_CURRENT_SOURCE_DIR}/rtspec/runtime_specialization.c
${CMAKE_CURRENT_SOURCE_DIR}/scanparse/handle_with_loop_dots.c
${CMAKE_CURRENT_SOURCE_DIR}/scanparse/handle_selection_dots.c
${CMAKE_CURRENT_SOURCE_DIR}/scanparse/handle_set_expression_dots.c
${CMAKE_CURRENT_SOURCE_DIR}/scanparse/set_expression_utils.c
${CMAKE_CURRENT_SOURCE_DIR}/scanparse/set_expression_range_inference.c
${CMAKE_CURRENT_SOURCE_DIR}/scanparse/handle_set_expressions.c
${CMAKE_CURRENT_SOURCE_DIR}/scanparse/handle_with_loop_generators.c
${CMAKE_CURRENT_SOURCE_DIR}/scanparse/handle_with_loop_operators.c
${CMAKE_CURRENT_SOURCE_DIR}/scanparse/handle_zero_generator_with_loops.c
${CMAKE_CURRENT_SOURCE_DIR}/scanparse/hidestructs.c
${CMAKE_CURRENT_SOURCE_DIR}/scanparse/introduce_user_tracing_calls.c
${CMAKE_CURRENT_SOURCE_DIR}/scanparse/lex.c
${CMAKE_CURRENT_SOURCE_DIR}/scanparse/parser.c
${CMAKE_CURRENT_SOURCE_DIR}/scanparse/resolvepragma.c
${CMAKE_CURRENT_SOURCE_DIR}/scanparse/scnprs.c
${CMAKE_CURRENT_SOURCE_DIR}/scanparse/trie.c
${CMAKE_CURRENT_SOURCE_DIR}/serialize/add_function_body.c
${CMAKE_CURRENT_SOURCE_DIR}/serialize/deserialize.c
${CMAKE_CURRENT_SOURCE_DIR}/serialize/serialize.c
${CMAKE_CURRENT_SOURCE_DIR}/serialize/serialize_attribs.c
#${CMAKE_CURRENT_SOURCE_DIR}/serialize/serialize_buildstack.c
${CMAKE_CURRENT_SOURCE_DIR}/serialize/serialize_filenames.c
#${CMAKE_CURRENT_SOURCE_DIR}/serialize/serialize_helper.c
#${CMAKE_CURRENT_SOURCE_DIR}/serialize/serialize_link.c
#${CMAKE_CURRENT_SOURCE_DIR}/serialize/serialize_node.c
${CMAKE_CURRENT_SOURCE_DIR}/serialize/serialize_stack.c
${CMAKE_CURRENT_SOURCE_DIR}/serialize/serialize_symboltable.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/DeadFunctionRemoval.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/dead_local_function_removal.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/ElimSubDiv.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/SSACSE.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/SSALUR.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/SSAWLUnroll.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/UndoCondZeroCmp.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/UndoElimSubDiv.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/arithmetic_simplification.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/associative_law.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/auto_inlining.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/comparison_to_zero.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/comparison_to_zero_guards.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/constant_folding.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/constant_propagation.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/deadcodeinference.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/deadcoderemoval.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/distributive_law.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/elim_shapevars.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/elimtypeconv.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/inferneedcounters.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/inlining.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/insert_symb_arrayattr.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/iteration_invariant_parameter_inference.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/liftoptflags.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/loop_invariant_removal.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/makedimexpr.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/makeshapeexpr.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/prfunroll.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/remove_vardecs.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/reorder_equalityprf_arguments.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/rmcasts.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/saa_constant_folding.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/setfundefwasoptimized.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/signature_simplification.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/sortassociativefunctionarguments.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/statistics.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/structural_constant_constant_folding.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/symbolic_constant_simplification.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/transform_gtge_to_ltle.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/variable_propagation.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/withloop_invariant_removal.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/wl_lock_optimization_marking.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/wl_lock_optimization_shifting.c
${CMAKE_CURRENT_SOURCE_DIR}/stdopt/wlselcount.c
${CMAKE_CURRENT_SOURCE_DIR}/support/debug.c
${CMAKE_CURRENT_SOURCE_DIR}/support/gdb_utils.c
${CMAKE_CURRENT_SOURCE_DIR}/support/int_matrix.c
${CMAKE_CURRENT_SOURCE_DIR}/support/math_utils.c
${CMAKE_CURRENT_SOURCE_DIR}/support/ptr_buffer.c
${CMAKE_CURRENT_SOURCE_DIR}/support/str.c
${CMAKE_CURRENT_SOURCE_DIR}/support/str_buffer.c
${CMAKE_CURRENT_SOURCE_DIR}/support/str_vec.c
${CMAKE_CURRENT_SOURCE_DIR}/support/system.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/DataFlowMask.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/DataFlowMaskUtils.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/DupTree.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/LookUpTable.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/change_signature.c
#${CMAKE_CURRENT_SOURCE_DIR}/tree/check.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/check_lacfuns.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/check_lib.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/check_mem.c
#${CMAKE_CURRENT_SOURCE_DIR}/tree/check_node.c
#${CMAKE_CURRENT_SOURCE_DIR}/tree/check_reset.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/cleanup_decls.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/compare_tree.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/free.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/free_attribs.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/free_lhs_avis_sons.c
#${CMAKE_CURRENT_SOURCE_DIR}/tree/free_node.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/group_local_funs.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/infer_dfms.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/map_avis_trav.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/map_call_graph.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/map_fun_trav.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/move_assigns.c
#${CMAKE_CURRENT_SOURCE_DIR}/tree/node_basic.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/pattern_match.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/pattern_match_attribs.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/pattern_match_build_lut.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/pattern_match_modes.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/pattern_match_old.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/prepare_inlining.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/prf_pvs_info.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/remove_dfms.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/rename.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/sanity_checks.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/traverse.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/traverse_inject.c
#${CMAKE_CURRENT_SOURCE_DIR}/tree/traverse_helper.c
#${CMAKE_CURRENT_SOURCE_DIR}/tree/traverse_tables.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/tree_basic.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/tree_compound.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/tree_utils.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/ungroup_local_funs.c
${CMAKE_CURRENT_SOURCE_DIR}/tree/wl_bounds.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/builtin_types.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/create_wrapper_code.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/create_wrappers.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/ct_basic.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/ct_fun.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/ct_prf.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/ct_with.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/destruct.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/dispatchfuncalls.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/elim_alpha_types.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/elim_bottom_types.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/enforce_specialization.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/extend_lac_funs.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/free_dispatch_information.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/insert_type_conv.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/new_typecheck.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/new_types.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/resolvesymboltypes.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/sig_deps.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/specialization_oracle_static_shape_knowledge.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/specialize.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/split_wrappers.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/ssi.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/type_errors.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/type_statistics.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/type_utils.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/update_wrapper_type.c
${CMAKE_CURRENT_SOURCE_DIR}/typecheck/user_types.c
${CMAKE_CURRENT_SOURCE_DIR}/visualize/visualize.c
${CMAKE_CURRENT_SOURCE_DIR}/wltransform/addSyncs.c
${CMAKE_CURRENT_SOURCE_DIR}/wltransform/prune_empty_with3.c
${CMAKE_CURRENT_SOURCE_DIR}/wltransform/remove_with3.c
${CMAKE_CURRENT_SOURCE_DIR}/wltransform/unroll_with3.c
${CMAKE_CURRENT_SOURCE_DIR}/wltransform/unshare_fold_iv.c
${CMAKE_CURRENT_SOURCE_DIR}/wltransform/wl_split_dimensions.c
${CMAKE_CURRENT_SOURCE_DIR}/wltransform/wlidxs.c
${CMAKE_CURRENT_SOURCE_DIR}/wltransform/wlpragma_funs.c
${CMAKE_CURRENT_SOURCE_DIR}/wltransform/wltransform.c
# Jupyter interface
${CMAKE_CURRENT_SOURCE_DIR}/global/jupyter.c
# Generated files now
#${CMAKE_CURRENT_BINARY_DIR}/codegen/fancy_icm_vars.c
${XSL_DONE_FILES}
${PROJECT_BINARY_DIR}/src/build.c
)


# XXX (artem): For the time being we exclude the files that do not yet adhere
#	       with -Wconversion.
SET_SOURCE_FILES_PROPERTIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/algebraic_wlfi.c #SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/isl_utilities.c #isl_val_get_num_si is long, FUNDEF_LOOPCOUNT int
    ${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/ivexpropagation.c #SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/loop_and_cond_scalarization_in.c #SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/loop_and_cond_scalarization_out.c #SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/pad_info.c #SHPSEG_SHAPE
    ${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/polyhedral_wlf.c #SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/SSAWithloopFolding.c #SHgetExtent, UnrLen
    ${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/SSAWLF.c #SHgetUnrlen for intern_gen->shape
    ${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/SSAWLI.c #SHgetExtent
    ${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/WithloopFusion.c #SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/wlswithloopification.c #INFO_INNERDIMS
    ${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/wlsbuild.c #SHtakeFromShape
    ${CMAKE_CURRENT_SOURCE_DIR}/arrayopt/wlscheck.c #TYgetDim
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/compile.c #TBmakeNum___ and MEMmalloc
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm_icm2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_cuda.c
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_error.c
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_fp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_mt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_prf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_sched.c
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_std.c
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/icm2c_rtspec.c
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/regression.c #Matrix int with MEMmalloc
    ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/cost_model.c #SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/scheduling.c       #TBmakeNum warnings
    ${CMAKE_CURRENT_SOURCE_DIR}/constants/basecv.c #SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/constants/constants_basic.c #SHgetDim & SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/constants/constants_struc_ops.c #SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/constants/shape.c #MEMmalloc with dim
    ${CMAKE_CURRENT_SOURCE_DIR}/cudahybrid/cuda_cost_model.c #SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/cuda/data_access_analysis.c #TYgetDim
    ${CMAKE_CURRENT_SOURCE_DIR}/cuda/infer_reusable_arrays.c #TYgetDim
    ${CMAKE_CURRENT_SOURCE_DIR}/cuda/partial_fold.c #SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/cuda/shared_memory_reuse.c #F_shmem_boundary_load TBmakeNum
    ${CMAKE_CURRENT_SOURCE_DIR}/flatten/wlanalysis.c #SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/flatten/wldefaultpartition.c #TYgetDim
    ${CMAKE_CURRENT_SOURCE_DIR}/flexsub/dynarray.c #DYNARRAY_TOTALELEMS (70 uses) and ALLOCELEMS linked
    ${CMAKE_CURRENT_SOURCE_DIR}/flexsub/dynmatrix.c #total rows and col int - MEMmalloc warning
    ${CMAKE_CURRENT_SOURCE_DIR}/flexsub/lubtree.c #LUBINFO_NUMINTRA and LUBgetBlockId
    ${CMAKE_CURRENT_SOURCE_DIR}/memory/alloc.c #SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/memory/polyhedral_reuse_analysis.c #TYgetDim
    ${CMAKE_CURRENT_SOURCE_DIR}/print/print.c                 #SHgetDim dim warning
    ${CMAKE_CURRENT_SOURCE_DIR}/print/convert.c               #CVshpseg2String dim warning
    ${CMAKE_CURRENT_SOURCE_DIR}/precompile/functionprecompile.c #RET & ARG_LINKSIGN
    ${CMAKE_CURRENT_SOURCE_DIR}/scanparse/handle_dots.c       #TBmakeNum warnings
    ${CMAKE_CURRENT_SOURCE_DIR}/scanparse/parser.c            #SHmakeShape (n) - n is size_t
    ${CMAKE_CURRENT_SOURCE_DIR}/scanparse/resolvepragma.c     #PRAGMA_NUMPARAMS
    ${CMAKE_CURRENT_SOURCE_DIR}/stdopt/makedimexpr.c   #TBmakeNum with TCcountExprs
    ${CMAKE_CURRENT_SOURCE_DIR}/stdopt/makeshapeexpr.c #TBmakeNum with TCcountExprs
    ${CMAKE_CURRENT_SOURCE_DIR}/stdopt/prfunroll.c #SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/stdopt/SSAWLUnroll.c #SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/stdopt/saa_constant_folding.c #int COgetDataVec with TCgetNthExprsExpr
    ${CMAKE_CURRENT_SOURCE_DIR}/stdopt/structural_constant_constant_folding.c #SHgetUnrlen and Idx2OffsetArray
    ${CMAKE_CURRENT_SOURCE_DIR}/support/str.c #STRstring2Array TBmakeNum (cnt)
    ${CMAKE_CURRENT_SOURCE_DIR}/tree/pattern_match_attribs.c #SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/tree/tree_utils.c #SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/typecheck/ct_prf.c #SHgetExtent, UnrLen and COgetDataVec with TYmakeAKD
    ${CMAKE_CURRENT_SOURCE_DIR}/typecheck/new_types.c         #DIM warnings from SH and TY
    ${CMAKE_CURRENT_SOURCE_DIR}/typecheck/ct_with.c #SHgetExtent
    ${CMAKE_CURRENT_SOURCE_DIR}/typecheck/specialization_oracle_static_shape_knowledge.c #SHgetExtent
    ${CMAKE_CURRENT_SOURCE_DIR}/typecheck/type_errors.c #SHgetUnrlen
    ${CMAKE_CURRENT_SOURCE_DIR}/wltransform/wltransform.c #TYgetDim
    ${CMAKE_CURRENT_SOURCE_DIR}/wltransform/wl_split_dimensions.c #TYgetDim
    # Generated files
    ${CMAKE_CURRENT_BINARY_DIR}/node_basic.c
    ${CMAKE_CURRENT_BINARY_DIR}/serialize_helper.c
    PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -Wno-conversion -Wno-sign-compare"
)


# Generate lList of needed libs for linker

SET (LIB_LIST ${M_LIB} ${DL_LIB})

IF (ENABLE_RTSPEC AND NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    LIST (APPEND LIB_LIST ${UUID_LIB} ${CRYPT_LIB})
ENDIF ()

# ISL needed for POGO, PWLF
IF (ENABLE_ISL)
    LIST (APPEND LIB_LIST ${LIB_ISL})
ENDIF ()

# BARVINOK needed for PLUT
IF (ENABLE_BARVINOK)
    LIST (APPEND LIB_LIST ${LIB_BARVINOK})
ENDIF ()

# Add INTERNAL_MALLOC definition that makes it possible to use `malloc'
# as an alias fo MEMmalloc within libsac2c code.
ADD_DEFINITIONS (-DINTERNAL_MALLOC)

ADD_LIBRARY (sac2cShared SHARED ${SAC2C_SRC})
SET_TARGET_PROPERTIES (sac2cShared
  PROPERTIES
    OUTPUT_NAME sac2c
)

TARGET_LINK_LIBRARIES (sac2cShared ${LIB_LIST})


XINSTALL (TARGETS sac2cShared LIBRARY DESTINATION "${TREEPATH_INSTALL}" COMPONENT libraries)
#ADD_CUSTOM_COMMAND (TARGET sac2cShared POST_BUILD
#  COMMAND "${CMAKE_COMMAND}" -E copy
#	$<TARGET_FILE:sac2cShared> $<TARGET_FILE_DIR:sac2cShared>/libsac2c-local${SHARED_LIB_EXT}
#  COMMENT "Copying libsac2c to libsac2c-local.")
