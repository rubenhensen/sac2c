/*
 *
 * $Log$
 * Revision 3.51  2004/11/23 11:36:42  cg
 * Switched mac-file based declaration of global variables.
 *
 * Revision 3.50  2004/10/28 16:58:43  khf
 * support for max_newgens and no_fold_fusion added
 *
 * Revision 3.49  2004/10/23 12:00:31  ktr
 * Added switches for static reuse / static free.
 *
 * Revision 3.48  2004/09/28 16:32:19  ktr
 * cleaned up concurrent (removed everything not working / not working with emm)
 *
 * Revision 3.47  2004/08/10 16:13:42  ktr
 * reuse inference in EMM can now be activated using -reuse.
 *
 * Revision 3.46  2004/08/06 10:47:56  skt
 * executionmodes_available added
 *
 * Revision 3.45  2004/08/05 15:17:04  sbs
 * ssaform_phase added.
 *
 * Revision 3.44  2004/08/04 12:04:20  ktr
 * substituted eacc by emm
 *
 * Revision 3.43  2004/07/23 15:53:50  ktr
 * - removed OPT_BLIR
 * - removed -ktr
 * - added -emm -do/noeacc
 *
 * Revision 3.42  2004/07/14 23:23:37  sah
 * removed all old ssa optimizations and the use_ssaform flag
 *
 * Revision 3.41  2004/06/09 13:28:34  skt
 * min_parallel_size_per_thread added
 *
 * Revision 3.40  2004/03/26 14:36:23  khf
 * OPT_WLPG added
 *
 * Revision 3.39  2004/03/10 00:10:17  dkrHH
 * old backend removed
 *
 * Revision 3.38  2004/03/02 16:48:25  mwe
 * OPT_CVP added
 *
 * Revision 3.37  2004/02/25 13:02:15  khf
 * added option -khf
 *
 * Revision 3.36  2004/02/05 10:37:14  cg
 * Re-factorized handling of different modes in multithreaded code
 * generation:
 * - Added enumeration type for representation of modes
 * - Renamed mode identifiers to more descriptive names.
 *
 * Revision 3.35  2003/12/10 17:33:16  khf
 * OPT_WLFS added for with-loop fusion
 *
 * Revision 3.34  2003/12/10 16:07:14  skt
 * changed compiler flag from -mtn to -mtmode and expanded mt-versions by one
 *
 * Revision 3.33  2003/10/19 21:38:25  dkrHH
 * prf_string moved from print.[ch] to globals.[ch] (for BEtest)
 *
 * Revision 3.32  2003/09/17 18:12:34  dkr
 * RCAO renamed into DAO for new backend
 *
 * Revision 3.31  2003/09/16 16:09:35  sbs
 * spec_mode and spec_mode_str added.
 *
 * Revision 3.30  2003/09/09 14:57:00  sbs
 * act_info_chn for extended type error reporting added
 *
 * Revision 3.29  2003/08/16 08:38:03  ktr
 * SelectionPropagation added. Must currently be activated with -dosp.
 *
 * Revision 3.28  2003/08/05 11:36:19  ktr
 * Support for maxwls added.
 *
 * [...]
 *
 * Revision 2.20  1999/10/22 14:16:41  sbs
 * made simpletype_size global, since it is needed in compile,
 * tile_size_inference AND constants already!
 *
 * [...]
 *
 */

/*
 * File : globals.h
 *
 * Declaration of global variables
 * which are all defined and initialized in globals.c
 *
 * This file is mostly generated from globals.mac.
 */

#ifndef _SAC_GLOBALS_H_
#define _SAC_GLOBALS_H_

#include "types.h"

/*
 * The following macro definitions should be replaced by flags.
 */

#define TRACE_NONE 0x0000 /* don't trace at all */
#define TRACE_ALL 0xffff  /* enable all implemented trace options */

#define TRACE_FUN 0x0001  /* trace user-defined fun apps */
#define TRACE_PRF 0x0002  /* trace prim fun apps */
#define TRACE_REF 0x0004  /* trace reference counting operations */
#define TRACE_MEM 0x0008  /* trace malloc/free operations */
#define TRACE_WL 0x0010   /* trace with-loop execution */
#define TRACE_AA 0x0020   /* trace array accesses */
#define TRACE_MT 0x0040   /* trace multi-threading specific operations */
#define TRACE_CENV 0x0080 /* trace c runtime enviroment init/exit */

#define PROFILE_NONE 0x0000
#define PROFILE_ALL 0xffff

#define PROFILE_FUN 0x0001
#define PROFILE_INL 0x0002
#define PROFILE_LIB 0x0004
#define PROFILE_WITH 0x0008

#define CACHESIM_NO 0x0000

#define CACHESIM_YES 0x0001
#define CACHESIM_ADVANCED 0x0002
#define CACHESIM_FILE 0x0004
#define CACHESIM_PIPE 0x0008
#define CACHESIM_IMMEDIATE 0x0010
#define CACHESIM_BLOCK 0x0020

#define RUNTIMECHECK_NONE 0x0000
#define RUNTIMECHECK_ALL 0xffff

#define RUNTIMECHECK_BOUNDARY 0x0001
#define RUNTIMECHECK_TYPE 0x0002
#define RUNTIMECHECK_MALLOC 0x0004
#define RUNTIMECHECK_ERRNO 0x0008
#define RUNTIMECHECK_HEAP 0x0010

#define INTRINSIC_NONE 0x0000
#define INTRINSIC_ALL 0xffff

#define INTRINSIC_ADD 0x0001
#define INTRINSIC_SUB 0x0002
#define INTRINSIC_MUL 0x0004
#define INTRINSIC_DIV 0x0008
#define INTRINSIC_TO 0x0010
#define INTRINSIC_TAKE 0x0020
#define INTRINSIC_DROP 0x0040
#define INTRINSIC_ROT 0x0080
#define INTRINSIC_CAT 0x0100
#define INTRINSIC_SEL 0x0200
#define INTRINSIC_MODA 0x0400

#define GENERATELIBRARY_NOTHING 0x0000 /* generate no library - dummy value for init */
#define GENERATELIBRARY_SAC 0x0001     /* generate SAC library from module */
#define GENERATELIBRARY_C 0x0002       /* generate C library and headerfile from module */

#define MIN_ARRAY_REP_SCL_AKS 0x0001
#define MIN_ARRAY_REP_SCL_AKD 0x0002
#define MIN_ARRAY_REP_SCL_AUD 0x0004
#define MIN_ARRAY_REP_AUD 0x0008

extern globals_t globals;

#endif /* _SAC_GLOBALS_H_ */
