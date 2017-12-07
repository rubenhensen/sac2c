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
    mt_h/mt_gen.h
    mt_h/fp_gen.h
    mutc_h/mutc_gen.h
    mutc_h/mutc_rc_gen.h
    nested_h/nested_gen.h
)


SET (M4_HEADERS)
FOREACH(name ${M4_FILES})
  SET(src "${CMAKE_CURRENT_SOURCE_DIR}/${name}.m4")
  SET(dst "${CMAKE_CURRENT_BINARY_DIR}/${name}")
  ADD_CUSTOM_COMMAND(
    OUTPUT "${dst}"
    DEPENDS "${src}"  "${CMAKE_CURRENT_SOURCE_DIR}/m4/icm.m4"
    COMMAND ${M4_EXEC} -I"${CMAKE_CURRENT_SOURCE_DIR}/m4" ${src} > ${dst}
    COMMENT "Generating ${dst}"
  )
  LIST (APPEND M4_HEADERS ${dst})
ENDFOREACH (name)


FILE (GLOB_RECURSE RUNTIME_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

SET (ORDERED_RUNTIME_HEADERS
essentials_h/rc_methods.h
essentials_h/types.h
essentials_h/audwl.h
essentials_h/rt_commandline.h
essentials_h/icm.h
essentials_h/prf.h
essentials_h/std.h
essentials_h/std_gen.h
essentials_h/rc_impl.h
essentials_h/rcm_local_async_norc_ptr.h
essentials_h/rcm_local_pasync.h
essentials_h/rcm_local_pasync_norc_desc.h
essentials_h/rc_stubs.h
essentials_h/rc_impl_gen.h
essentials_h/bool.h
essentials_h/errors.h
essentials_h/idx.h
essentials_h/rt_misc.h
essentials_h/wl.h
essentials_h/tuple.h
extras_h/rt_cachesim.h
extras_h/rt_profile.h
../libsac/essentials/profile.h
extras_h/runtimecheck.h
extras_h/rt_trace.h
mt_h/rt_mt.h
../libsac/mt/mt.h
mt_h/rt_mt_omp.h
../libsac/mt/mt_omp.h
mt_h/schedule.h
mt_h/rt_mt_beehive.h
../libsac/mt/mt_beehive.h
mt_h/mt_gen.h
mt_h/mt_barriers.h
mt_h/rt_mt_pth.h
../libsac/mt/mt_pth.h
mt_h/fp.h
mt_h/fp_gen.h
mt_h/rt_mt_smart.h
../libsac/mt/mt_smart.h
mt_h/mt_lpel.h
phm_h/phm.h
mutc_h/mutc.h
mutc_h/mutc_world.h
mutc_h/mutc_gen.h
mutc_h/mutc_tostring.h
mutc_h/mutc_startup.h
mutc_h/mutc_rc.h
mutc_h/mutc_rc_gen.h
cuda_h/cuda.h
cuda_h/cuda_gen.h
rtspec_h/rtspec.h
rtspec_h/registry.h
rtspec_h/simple_reqqueue.h
rtspec_h/uuid_reqqueue.h
rtspec_h/supervisor.h
rtspec_h/simple_controller.h
rtspec_h/uuid_controller.h
rtspec_h/persistence.h
distmem_h/distmem.h
cudahybrid_h/cudahybrid.h
cudahybrid_h/alloc_cache.h
cudahybrid_h/dist_var_nomemsave.h
cudahybrid_h/basic.h
nested_h/nested.h
nested_h/nested_gen.h
distmemphm_h/distmemphm.h
)

SET (CMAKE_LIBSAC_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libsac")

FILE (GLOB_RECURSE LIBSAC_HEADERS RELATIVE "${CMAKE_LIBSAC_SOURCE_DIR}" "${CMAKE_LIBSAC_SOURCE_DIR}/*.h")

SET (SAC_H  "${PROJECT_BINARY_DIR}/include/sac.h")

# copy RUNTIME_HEADERS into ${PROJECT_BINARY_DIR}/include/runtime
SET (RUNTIME_HEADERS_DONE)
FOREACH (name ${RUNTIME_HEADERS})
  SET(src "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
  SET(dst "${PROJECT_BINARY_DIR}/include/runtime/${name}")
  ADD_CUSTOM_COMMAND(
    OUTPUT "${dst}"
    DEPENDS "${src}"
    COMMAND ${CMAKE_COMMAND} -E copy ${src} ${dst}
    COMMENT "Copying header to ${dst}"
  )
  LIST (APPEND RUNTIME_HEADERS_DONE ${dst})
ENDFOREACH (name)

# copy M4_HEADERS headers into ${PROJECT_BINARY_DIR}/include/runtime
SET (M4_HEADERS_DONE)
FOREACH (name ${M4_FILES})
  SET(src "${CMAKE_CURRENT_BINARY_DIR}/${name}")
  SET(dst "${PROJECT_BINARY_DIR}/include/runtime/${name}")
  ADD_CUSTOM_COMMAND(
    OUTPUT "${dst}"
    DEPENDS "${src}"
    COMMAND ${CMAKE_COMMAND} -E copy ${src} ${dst}
    COMMENT "Copying header to ${dst}"
  )
  LIST (APPEND M4_HEADERS_DONE ${dst})
ENDFOREACH (name)

# copy LIBSAC_HEADERS into ${PROJECT_BINARY_DIR}/include/libsac

SET (LIBSAC_HEADERS_DONE)
FOREACH (name ${LIBSAC_HEADERS})
  SET(src "${CMAKE_LIBSAC_SOURCE_DIR}/${name}")
  SET(dst "${PROJECT_BINARY_DIR}/include/libsac/${name}")
  ADD_CUSTOM_COMMAND(
    OUTPUT "${dst}"
    DEPENDS "${src}"
    COMMAND ${CMAKE_COMMAND} -E copy ${src} ${dst}
    COMMENT "Copying header to ${dst}"
  )
  LIST (APPEND LIBSAC_HEADERS_DONE ${dst})
ENDFOREACH (name)

# generate sac.h from M4_HEADERS, RUNTIME_HEADERS, and LIBSAC_HEADERS:
ADD_CUSTOM_COMMAND (
  OUTPUT    ${SAC_H}
  DEPENDS   ${M4_HEADERS} ${RUNTIME_HEADERS}
  COMMAND   ${CMAKE_COMMAND} -DOUTPUT="${SAC_H}"
                             -DREL_HEADERS="${M4_FILES};${RUNTIME_HEADERS}"
# however, currently the M4 files are included manually to maintain order...
#                            -DREL_HEADERS="${ORDERED_RUNTIME_HEADERS}"
                             -DREL_LIB_HEADERS="${LIBSAC_HEADERS}"
                             -P ${PROJECT_SOURCE_DIR}/cmake/generate-sac-h.cmake
)

ADD_CUSTOM_TARGET (sac_h ALL DEPENDS ${SAC_H} ${RUNTIME_HEADERS_DONE} ${M4_HEADERS_DONE} ${LIBSAC_HEADERS_DONE})


# install include files
XINSTALL (DIRECTORY "${PROJECT_BINARY_DIR}/include/runtime"
  DESTINATION "${INCPATH_INSTALL}"
  COMPONENT headers)
XINSTALL (FILES "${SAC_H}"
  DESTINATION "${INCPATH_INSTALL}"
  COMPONENT headers)
