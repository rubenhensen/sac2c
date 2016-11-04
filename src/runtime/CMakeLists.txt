# Create the list of directories.
FILE (GLOB tmplst RELATIVE  "${CMAKE_CURRENT_SOURCE_DIR}" *)
SET (DIR_LIST)
FOREACH (dir ${tmplst})
  SET (mask "CMakeLists.txt|Makefile|m4")
  IF (NOT ${dir} MATCHES ${mask})
    LIST (APPEND DIR_LIST ${dir}) 
  ENDIF (NOT ${dir} MATCHES ${mask})
ENDFOREACH (dir)

# Create a list of directories in the project binary dir
# in order to generate .h.m4 files.
FOREACH (dir ${DIR_LIST}) 
  FILE (MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${dir}")
ENDFOREACH (dir)

SET (M4_FILES
    cuda_h/cuda_gen.h
    essentials_h/rc_impl_gen.h
    essentials_h/std_gen.h
    mt_beehive_h/mt_gen.h
    mt_pth_h/fp_gen.h
    mutc_h/mutc_gen.h
    mutc_h/mutc_rc_gen.h
    nested_h/nested_gen.h
)


SET (M4_LIST_DONE)
FOREACH(name ${M4_FILES})
  SET(src "${CMAKE_CURRENT_SOURCE_DIR}/${name}.m4")
  SET(dst "${CMAKE_CURRENT_BINARY_DIR}/${name}")
  ADD_CUSTOM_COMMAND(
    OUTPUT "${dst}"
    DEPENDS "${src}"  "${CMAKE_CURRENT_SOURCE_DIR}/m4/icm.m4"
    COMMAND ${M4_EXEC} -I"${CMAKE_CURRENT_SOURCE_DIR}/m4" ${src} > ${dst}
    COMMENT "Generating ${dst}"
  )
  LIST (APPEND M4_LIST_DONE ${dst})
ENDFOREACH (name)


FILE (GLOB_RECURSE HEADERS "*.h")

# XXX This is a stupid hack, as the order of the definitions in the following list
# *makes a difference*.  If the order is different, the sac2c will not
# compile.
SET (HEADERS_ALL
${PROJECT_BINARY_DIR}/include/config.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/rc_methods.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/types.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/audwl.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/commandline.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/icm.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/prf.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/std.h
${CMAKE_CURRENT_BINARY_DIR}/essentials_h/std_gen.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/rc_impl.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/rcm_local_async_norc_ptr.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/rcm_local_pasync.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/rcm_local_pasync_norc_desc.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/rc_stubs.h
${CMAKE_CURRENT_BINARY_DIR}/essentials_h/rc_impl_gen.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/message.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/bool.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/errors.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/idx.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/misc.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/wl.h
${CMAKE_CURRENT_SOURCE_DIR}/essentials_h/tuple.h
${CMAKE_CURRENT_SOURCE_DIR}/extras_h/cachesim.h
${CMAKE_CURRENT_SOURCE_DIR}/extras_h/profile.h
${CMAKE_CURRENT_SOURCE_DIR}/extras_h/runtimecheck.h
${CMAKE_CURRENT_SOURCE_DIR}/extras_h/trace.h
${CMAKE_CURRENT_SOURCE_DIR}/mt_h/mt.h
${CMAKE_CURRENT_SOURCE_DIR}/mt_omp_h/mt_omp.h
${CMAKE_CURRENT_SOURCE_DIR}/mt_beehive_h/schedule.h
${CMAKE_CURRENT_SOURCE_DIR}/mt_beehive_h/mt_beehive.h
${CMAKE_CURRENT_BINARY_DIR}/mt_beehive_h/mt_gen.h
${CMAKE_CURRENT_SOURCE_DIR}/mt_pth_h/mt_barriers.h
${CMAKE_CURRENT_SOURCE_DIR}/mt_pth_h/mt_pth.h
${CMAKE_CURRENT_SOURCE_DIR}/mt_pth_h/fp.h
${CMAKE_CURRENT_BINARY_DIR}/mt_pth_h/fp_gen.h
${CMAKE_CURRENT_SOURCE_DIR}/mt_smart_h/mt_smart.h
${CMAKE_CURRENT_SOURCE_DIR}/mt_lpel_h/mt_lpel.h
${CMAKE_CURRENT_SOURCE_DIR}/phm_h/phm.h
${CMAKE_CURRENT_SOURCE_DIR}/mutc_h/mutc.h
${CMAKE_CURRENT_SOURCE_DIR}/mutc_h/mutc_world.h
${CMAKE_CURRENT_BINARY_DIR}/mutc_h/mutc_gen.h
${CMAKE_CURRENT_SOURCE_DIR}/mutc_h/mutc_tostring.h
${CMAKE_CURRENT_SOURCE_DIR}/mutc_h/mutc_startup.h
${CMAKE_CURRENT_SOURCE_DIR}/mutc_h/mutc_rc.h
${CMAKE_CURRENT_BINARY_DIR}/mutc_h/mutc_rc_gen.h
${CMAKE_CURRENT_SOURCE_DIR}/cuda_h/cuda.h
${CMAKE_CURRENT_BINARY_DIR}/cuda_h/cuda_gen.h
${CMAKE_CURRENT_SOURCE_DIR}/rtspec_h/rtspec.h
${CMAKE_CURRENT_SOURCE_DIR}/rtspec_h/registry.h
${CMAKE_CURRENT_SOURCE_DIR}/rtspec_h/simple_reqqueue.h
${CMAKE_CURRENT_SOURCE_DIR}/rtspec_h/uuid_reqqueue.h
${CMAKE_CURRENT_SOURCE_DIR}/rtspec_h/supervisor.h
${CMAKE_CURRENT_SOURCE_DIR}/rtspec_h/simple_controller.h
${CMAKE_CURRENT_SOURCE_DIR}/rtspec_h/uuid_controller.h
${CMAKE_CURRENT_SOURCE_DIR}/rtspec_h/persistence.h
${CMAKE_CURRENT_SOURCE_DIR}/distmem_h/distmem.h
${CMAKE_CURRENT_SOURCE_DIR}/cudahybrid_h/cudahybrid.h
${CMAKE_CURRENT_SOURCE_DIR}/cudahybrid_h/alloc_cache.h
${CMAKE_CURRENT_SOURCE_DIR}/cudahybrid_h/dist_var_nomemsave.h
${CMAKE_CURRENT_SOURCE_DIR}/cudahybrid_h/basic.h
${CMAKE_CURRENT_SOURCE_DIR}/nested_h/nested.h
${CMAKE_CURRENT_BINARY_DIR}/nested_h/nested_gen.h
${CMAKE_CURRENT_SOURCE_DIR}/distmemphm_h/distmemphm.h
#${M4_LIST_DONE}
)


SET (SAC_H  "${PROJECT_BINARY_DIR}/include/sac.h")
SET (HZIP "${PROJECT_BINARY_DIR}/hzip")
ADD_CUSTOM_COMMAND (
  OUTPUT    ${SAC_H}
  DEPENDS   ${M4_LIST_DONE} ${HEADERS_ALL} ${HZIP}
  COMMAND   ${HZIP} ${HEADERS_ALL} > ${SAC_H}
)

ADD_CUSTOM_TARGET (sac_h ALL DEPENDS ${SAC_H})
XINSTALL (FILES "${SAC_H}" DESTINATION "${INCPATH_CONF}")
