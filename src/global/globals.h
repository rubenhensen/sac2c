/*
 *
 * $Log$
 * Revision 3.55  2004/11/23 21:51:49  cg
 * Added genlib flags.
 *
 * Revision 3.54  2004/11/23 19:43:26  cg
 * Added flags for triggering tracing, cache simulation and runtime checks.
 *
 * Revision 3.53  2004/11/23 16:23:05  cg
 * First working revision of new representation of global variables.
 *
 * Revision 3.52  2004/11/23 12:52:04  cg
 * SacDevCamp update.
 *
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
 * Description:
 *
 * We alow exactly one global variable named 'global', which is a huge
 * structure containing all globally available information.
 *
 * Most of the work is done in types.h where the complicated type
 * global_t is generated from globals.mac and in globals.c where the
 * initialization code again is generated from globals.mac.
 *
 */

#ifndef _SAC_GLOBALS_H_
#define _SAC_GLOBALS_H_

#include "types.h"

extern global_t global;

extern void GLOBinitializeGlobal ();

#endif /* _SAC_GLOBALS_H_ */
