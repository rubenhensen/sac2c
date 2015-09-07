IF (NOT ENABLE_MUTC)
  SET (LIBSAC_SRC
    cachesim/access_detailed.c
    cachesim/access_simple.c
    cachesim/basic.c
    essentials/commandline.c
    essentials/message.c
    essentials/misc.c
    essentials/profile.c
    essentials/trace.c
    interface/sacarg.c
    interface/sacargconvert.c
    interface/sacmt.c
    mt/mt.c
    mt_beehive/mt_beehive.c
    mt_lpel/mt_lpel_std.c
    mt_lpel/mt_lpel_trace.c
    mt_omp/mt_omp.c
    mt_pth/mt_pth_std.c
    mt_pth/mt_pth_trace.c
    phm/nophm.c
    rtspec/controller.c
    rtspec/persistence.c
    rtspec/registry.c
    rtspec/reqqueue.c
  )
ELSE ()
  SET (LIBSAC_SRC
    essentials/commandline.c
    essentials/message.c
    essentials/misc.c
    essentials/profile.c
    essentials/trace.c
    interface/sacarg.c
    interface/sacargconvert.c
    interface/sacmt.c
    mutc/reference_counting.c
    phm/nophm.c
  )
ENDIF (NOT ENABLE_MUTC)


ADD_LIBRARY (sacShared SHARED ${LIBSAC_SRC})
ADD_LIBRARY (sacStatic STATIC ${LIBSAC_SRC})
ADD_DEPENDENCIES (sacStatic sac_h)
ADD_DEPENDENCIES (sacShared sac_h)
SET_TARGET_PROPERTIES(sacStatic sacShared PROPERTIES OUTPUT_NAME sac)
