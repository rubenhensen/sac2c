/*
 *
 * $Log$
 * Revision 1.4  2003/09/18 15:11:14  dkr
 * SAC_ND_ALLOC__DESC_AND_DATA added
 *
 * Revision 1.3  2003/06/17 18:54:15  dkr
 * comment in header modified
 *
 * Revision 1.2  2003/06/17 18:49:01  dkr
 * RCS header added
 *
 */

/**
 **  This file can be used to test the expansion of H-ICMs
 **
 **  Apply the CC preprocessor to this file and verify the generated output:
 **    gcc -I$SACHOME/runtime -E icm_test.C > tutu
 **    grep CAT tutu
 **    grep NT tutu
 **
 **  It exists a Makefile target for this purpose (icm_test) !!!
 **/

#ifndef _SHP_
#define _SHP_ SCL /* SCL, AKS, AKD, AUD */
#endif

#ifndef _HID_
#define _HID_ HID /* NHD, HID */
#endif

#ifndef _UNQ_
#define _UNQ_ NUQ /* NUQ, UNQ */
#endif

#define TAGGED_ARRAYS
#define SAC_DO_MULTITHREAD 1
#include "sac.h"

SAC_ND_A_DESC ((nt, (_SHP_, (_HID_, (_UNQ_, )))))
SAC_ND_A_DESC_DIM ((nt, (_SHP_, (_HID_, (_UNQ_, )))))
SAC_ND_A_DESC_SIZE ((nt, (_SHP_, (_HID_, (_UNQ_, )))))
SAC_ND_A_DESC_SHAPE ((nt, (_SHP_, (_HID_, (_UNQ_, )))), 0)
SAC_ND_A_FIELD ((nt, (_SHP_, (_HID_, (_UNQ_, )))))
SAC_ND_A_RC ((nt, (_SHP_, (_HID_, (_UNQ_, )))))
SAC_ND_A_DIM ((nt, (_SHP_, (_HID_, (_UNQ_, )))))
SAC_ND_A_SIZE ((nt, (_SHP_, (_HID_, (_UNQ_, )))))
SAC_ND_A_SHAPE ((nt, (_SHP_, (_HID_, (_UNQ_, )))), 0)

SAC_ND_READ ((nt, (_SHP_, (_HID_, (_UNQ_, )))), 0)
SAC_ND_WRITE ((nt, (_SHP_, (_HID_, (_UNQ_, )))), 0)

SAC_ND_TYPEDEF ((nt, (_SHP_, (_HID_, (_UNQ_, )))), int)

SAC_ND_DECL__DATA ((nt, (_SHP_, (_HID_, (_UNQ_, )))), int, )
SAC_ND_DECL__DESC ((nt, (_SHP_, (_HID_, (_UNQ_, )))), )

SAC_ND_PARAM_ (dummy, int)
SAC_ND_PARAM_in ((nt, (_SHP_, (_HID_, (_UNQ_, )))), int) SAC_ND_PARAM_in_nodesc (
  (nt, (_SHP_, (_HID_, (_UNQ_, )))),
  int) SAC_ND_PARAM_out ((nt, (_SHP_, (_HID_, (_UNQ_, )))),
                         int) SAC_ND_PARAM_out_nodesc ((nt, (_SHP_, (_HID_, (_UNQ_, )))),
                                                       int)
  SAC_ND_PARAM_inout ((nt, (_SHP_, (_HID_, (_UNQ_, )))), int) SAC_ND_PARAM_inout_nodesc (
    (nt, (_SHP_, (_HID_, (_UNQ_, )))),
    int) SAC_ND_PARAM_inout_nodesc_bx ((nt, (_SHP_, (_HID_, (_UNQ_, )))),
                                       int) SAC_ND_ARG_in ((nt,
                                                            (_SHP_, (_HID_, (_UNQ_, )))))
    SAC_ND_ARG_in_nodesc ((nt, (_SHP_, (_HID_, (_UNQ_, )))))
      SAC_ND_ARG_out ((nt, (_SHP_, (_HID_, (_UNQ_, ))))) SAC_ND_ARG_out_nodesc ((
        nt,
        (_SHP_, (_HID_, (_UNQ_, ))))) SAC_ND_ARG_inout ((nt, (_SHP_, (_HID_, (_UNQ_, )))))
        SAC_ND_ARG_inout_nodesc ((nt, (_SHP_, (_HID_, (_UNQ_, )))))
          SAC_ND_ARG_inout_nodesc_bx ((nt, (_SHP_, (_HID_, (_UNQ_, ))))) SAC_ND_RET_out (
            (nt, (_SHP_, (_HID_, (_UNQ_, )))),
            (ntp,
             (_SHP_,
              (_HID_, (_UNQ_, ))))) SAC_ND_RET_inout ((nt, (_SHP_, (_HID_, (_UNQ_, )))),
                                                      (ntp, (_SHP_, (_HID_, (_UNQ_, )))))
            SAC_ND_DECL_PARAM_inout ((nt, (_SHP_, (_HID_, (_UNQ_, )))), int)

              SAC_ND_ALLOC ((nt, (_SHP_, (_HID_, (_UNQ_, )))), rc, get_dim, set_shape_icm)
                SAC_ND_ALLOC_BEGIN (
                  (nt, (_SHP_, (_HID_, (_UNQ_, )))),
                  rc, dim) SAC_ND_ALLOC_END ((nt, (_SHP_, (_HID_, (_UNQ_, )))), rc, dim)
                  SAC_ND_ALLOC__DESC ((nt, (_SHP_, (_HID_, (_UNQ_, )))), dim)
                    SAC_ND_ALLOC__DATA ((nt, (_SHP_, (_HID_, (_UNQ_, )))))
                      SAC_ND_ALLOC__DESC_AND_DATA ((nt, (_SHP_, (_HID_, (_UNQ_, )))), dim)

                        SAC_ND_FREE ((nt, (_SHP_, (_HID_, (_UNQ_, )))), freefun)
                          SAC_ND_FREE__DESC ((nt, (_SHP_, (_HID_, (_UNQ_, )))))
                            SAC_ND_FREE__DATA ((nt, (_SHP_, (_HID_, (_UNQ_, )))), freefun)

                              SAC_ND_ASSIGN__DATA ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))),
                                                   (from_nt, (_SHP_, (_HID_, (_UNQ_, )))),
                                                   copyfun)
                                SAC_ND_COPY__DATA ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))),
                                                   (from_nt, (_SHP_, (_HID_, (_UNQ_, )))),
                                                   copyfun)

                                  SAC_ND_SET__RC ((nt, (_SHP_, (_HID_, (_UNQ_, )))), rc)
                                    SAC_ND_INC_RC ((nt, (_SHP_, (_HID_, (_UNQ_, )))), rc)
                                      SAC_ND_DEC_RC ((nt, (_SHP_, (_HID_, (_UNQ_, )))),
                                                     rc)
                                        SAC_ND_DEC_RC_FREE ((nt,
                                                             (_SHP_, (_HID_, (_UNQ_, )))),
                                                            rc, freefun)

                                          SAC_ND_CREATE__SCALAR__DATA (
                                            (nt, (_SHP_, (_HID_, (_UNQ_, )))), 17)
                                            SAC_ND_CREATE__STRING__DATA (
                                              (nt, (_SHP_, (_HID_, (_UNQ_, )))), "hallo")

                                              SAC_IS_LASTREF__BLOCK_BEGIN (
                                                (to_nt, (_SHP_, (_HID_, (_UNQ_, )))))
                                                SAC_IS_LASTREF__BLOCK_ELSE (
                                                  (to_nt, (_SHP_, (_HID_, (_UNQ_, )))))
SAC_IS_LASTREF__BLOCK_END
((to_nt, (_SHP_, (_HID_, (_UNQ_, )))))

  SAC_IS_REUSED__BLOCK_BEGIN ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))),
                              (from_nt, (_SHP_, (_HID_, (_UNQ_, )))))
    SAC_IS_REUSED__BLOCK_ELSE ((to_nt, (_SHP_, (_HID_, (_UNQ_, )))),
                               (from_nt, (_SHP_, (_HID_, (_UNQ_, )))))
SAC_IS_REUSED__BLOCK_END
((to_nt, (_SHP_, (_HID_, (_UNQ_, )))), (from_nt, (_SHP_, (_HID_, (_UNQ_, )))))

  SAC_MT_SPMD_ARG_in (int, (nt, (_SHP_, (_HID_, (_UNQ_, )))))
    SAC_MT_SPMD_ARG_out (int, (nt, (_SHP_, (_HID_, (_UNQ_, )))))
