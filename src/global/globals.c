/*
 *
 * $Log$
 * Revision 3.73  2004/11/23 12:52:04  cg
 * SacDevCamp update.
 *
 * Revision 3.72  2004/11/23 11:36:42  cg
 * Switched mac-file based declaration of global variables.
 *
 * Revision 3.71  2004/11/19 15:39:59  ktr
 * OPT_SRF is now activeted by default.
 *
 * Revision 3.70  2004/10/28 16:58:43  khf
 * support for max_newgens and no_fold_fusion added
 *
 * Revision 3.69  2004/10/27 15:53:14  khf
 * WLFS activated
 *
 * Revision 3.68  2004/10/23 12:00:31  ktr
 * Added switches for static reuse / static free.
 *
 * Revision 3.67  2004/10/13 15:18:54  sah
 * disabled SP in NEW_AST mode
 *
 * Revision 3.66  2004/09/30 20:14:17  sah
 * renamed newast version to 1.00-alpha
 *
 * Revision 3.65  2004/09/28 16:32:19  ktr
 * cleaned up concurrent (removed everything not working / not working with emm)
 *
 * Revision 3.64  2004/09/27 10:13:09  ktr
 * Reactivated WLS
 *
 * Revision 3.63  2004/09/21 12:39:16  sah
 * version identifier now contains newast
 * if NEW_AST flag set.
 *
 * Revision 3.62  2004/08/26 14:02:36  cg
 * Re-organized default enable/disable settings of optimizations
 * after non-ssa-based optimizations were removed.
 * Enable emm (new refcounting) by default.
 *
 * Revision 3.61  2004/08/12 12:01:31  ktr
 * EMM reuse is now activated by default.
 *
 * Revision 3.60  2004/08/10 16:13:42  ktr
 * reuse inference in EMM can now be activated using -reuse.
 *
 * Revision 3.59  2004/08/10 13:28:43  sah
 * -sbs flag enabled in NEW_AST mode by default, as
 * the new typechecker is as well.
 *
 * Revision 3.58  2004/08/09 13:15:14  khf
 * OPT_WLPG enabled
 *
 * Revision 3.57  2004/08/06 10:47:56  skt
 * executionmodes_available added
 *
 * Revision 3.56  2004/08/05 15:17:04  sbs
 * ssaform_phase added.
 *
 * Revision 3.55  2004/08/04 12:04:20  ktr
 * substituted eacc by emm
 *
 * Revision 3.54  2004/07/29 15:25:51  sah
 * reenabled array padding
 * disabled wls
 *  (due to bug #41)
 *
 * Revision 3.53  2004/07/23 15:53:50  ktr
 * - removed OPT_BLIR
 * - removed -ktr
 * - added -emm -do/noeacc
 *
 * Revision 3.52  2004/07/20 17:43:44  sah
 * disbaled array padding (OPT_AP) by default
 *
 * Revision 3.51  2004/07/17 20:07:13  sah
 * added ssa to version id
 *
 * Revision 3.50  2004/07/14 23:23:37  sah
 * removed all old ssa optimizations and the use_ssaform flag
 *
 * Revision 3.49  2004/06/09 13:28:34  skt
 * min_parallel_size_per_thread added
 *
 * Revision 3.48  2004/04/30 13:21:03  ktr
 * Nothing really changed.
 *
 * Revision 3.47  2004/03/26 14:36:23  khf
 * OPT_WLPG added
 *
 * Revision 3.46  2004/03/05 19:14:27  mwe
 * representation of conditional changed
 * using N_funcond node instead of phi
 *
 * Revision 3.45  2004/03/02 16:48:25  mwe
 * OPT_CVP added
 *
 * Revision 3.44  2004/02/25 13:02:15  khf
 * added option -khf
 *
 * Revision 3.43  2004/02/05 10:37:14  cg
 * Re-factorized handling of different modes in multithreaded code
 * generation:
 * - Added enumeration type for representation of modes
 * - Renamed mode identifiers to more descriptive names.
 *
 * Revision 3.42  2003/12/10 17:30:15  khf
 * with-loop fusion deactivated in compilation by default
 *
 * Revision 3.41  2003/12/10 16:07:14  skt
 * changed compiler flag from -mtn to -mtmode and expanded mt-versions by one
 *
 * Revision 3.40  2003/10/19 21:38:25  dkrHH
 * prf_string moved from print.[ch] to globals.[ch] (for BEtest)
 *
 * Revision 3.39  2003/09/16 16:09:35  sbs
 * spec_mode and spec_mode_str added.
 *
 * Revision 3.38  2003/09/09 14:57:00  sbs
 * act_info_chn for extended type error reporting added
 *
 * Revision 3.37  2003/08/19 17:20:41  ktr
 * SelectionPropagation os now activated by default.
 *
 * Revision 3.36  2003/08/16 08:38:03  ktr
 * SelectionPropagation added. Must currently be activated with -dosp.
 *
 * Revision 3.35  2003/08/05 11:36:19  ktr
 * Support for maxwls added.
 *
 * Revision 3.34  2003/05/21 16:38:02  ktr
 * added option -ktr
 *
 * Revision 3.33  2003/03/24 16:35:56  sbs
 * cpp_incs added.
 *
 * Revision 3.32  2003/03/20 14:01:13  sbs
 * config.h included; OS and ARCH used.
 *
 * Revision 3.31  2003/03/13 13:59:51  dkr
 * min_array_rep added
 *
 * Revision 3.30  2003/03/09 17:13:54  ktr
 * added basic support for BLIR.
 *
 * ... [eliminated]
 *
 * Revision 3.1  2000/11/20 17:59:28  sacbase
 * new release made
 *
 * ... [eliminated]
 *
 * Revision 2.2  1999/03/31 11:30:27  cg
 * added global variable cachesim.
 *
 * ... [eliminated]
 *
 */

/*
 * File : globals.c
 *
 * This file should contain the definitions of all global variables
 * used in the implementation of the sac2c compiler which are not restricted
 * to the use within one particular file or within the files of one
 * particular subdirectory. While the former ones have to be declared static,
 * the latter ones should be defined in a file called globals_xyz.c specific
 * to the respective subdirectory.
 *
 * However, the usage of global variables should be as extensive as possible
 * since a functional programming style is preferred in the SAC project. The
 * major application of global variables therefore is the storage of such
 * global information as determined by the command line arguments of a sac2c
 * compiler call.
 *
 */

#include "globals.h"

#include "ct_prf.h"
#include "constants.h"
#include "basecv.h"
#include "zipcv.h"
#include "cv2cv.h"
#include "cv2scalar.h"
#include "cv2str.h"

#if 0
#include "config.h"
#include "filemgr.h"
#include "types.h"
#include "Error.h"
#include "internal_lib.h"
#include "type_errors.h"
#endif

static const char *compiler_phase_name_init[PH_final + 1] = {
#define PH_SELtext(it_text) it_text
#include "phase_info.mac"
#undef PH_SELtext
};

static int do_lac2fun_init[PH_final + 1] = {
#define PH_SELlac2fun(it_lac2fun) it_lac2fun
#include "phase_info.mac"
#undef PH_SELlac2fun
};

static int do_fun2lac_init[PH_final + 1] = {
#define PH_SELfun2lac(it_fun2lac) it_fun2lac
#include "phase_info.mac"
#undef PH_SELfun2lac
};

static bool ATG_has_shp_init[] = {
#define SELECTshp(it_shp) it_shp
#include "argtag_info.mac"
#undef SELECTshp
};

static bool ATG_has_rc_init[] = {
#define SELECTrc(it_rc) it_rc
#include "argtag_info.mac"
#undef SELECTrc
};

static bool ATG_has_desc_init[] = {
#define SELECTdesc(it_desc) it_desc
#include "argtag_info.mac"
#undef SELECTdesc
};

static bool ATG_is_in_init[] = {
#define SELECTin(it_in) it_in
#include "argtag_info.mac"
#undef SELECTin
};

static bool ATG_is_out_init[] = {
#define SELECTout(it_out) it_out
#include "argtag_info.mac"
#undef SELECTout
};

static bool ATG_is_inout_init[] = {
#define SELECTinout(it_inout) it_inout
#include "argtag_info.mac"
#undef SELECTinout
};

static char *ATG_string_init[] = {
#define SELECTtext(it_text) it_text
#include "argtag_info.mac"
#undef SELECTtext
};

static const int basetype_size_init[] = {
#define TYP_IFsize(sz) sz
#include "type_info.mac"
#undef TYP_IFsize
};

static const char *prf_string_init[] = {
#define PRF_IF(a, b, c, d, e, f, g, h) c
#include "prf_node_info.mac"
#undef PRF_IF
};

static char *nt_shape_string_init[] = {
#define ATTRIB NT_SHAPE_INDEX
#define NTIFstr(it_str) it_str
#include "nt_info.mac"
#undef ATTRIB
#undef NTIFstr
};

static char *nt_hidden_string_init[] = {
#define ATTRIB NT_HIDDEN_INDEX
#define NTIFstr(it_str) it_str
#include "nt_info.mac"
#undef ATTRIB
#undef NTIFstr
};

static char *nt_unique_string_init[] = {
#define ATTRIB NT_UNIQUE_INDEX
#define NTIFstr(it_str) it_str
#include "nt_info.mac"
#undef ATTRIB
#undef NTIFstr
};

static const ct_funptr NTCPRF_funtab_init[] = {
#define PRF_IF(a, b, c, d, e, f, g, h) g
#include "prf_node_info.mac"
#undef PRF_IF
};

static const void *NTCPRF_cffuntab_init[] = {
#define PRF_IF(a, b, c, d, e, f, g, h) (void *)h
#include "prf_node_info.mac"
#undef PRF_IF
};

static const zipcvfunptr zipcv_plus_init[] = {
#define TYP_IFzipcv(fun) fun##Plus
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_minus_init[] = {
#define TYP_IFzipcv(fun) fun##Minus
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_mul_init[] = {
#define TYP_IFzipcv(fun) fun##Mul
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_div_init[] = {
#define TYP_IFzipcv(fun) fun##Div
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_mod_init[] = {
#define TYP_IFzipcv(fun) fun##Mod
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_min_init[] = {
#define TYP_IFzipcv(fun) fun##Min
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_max_init[] = {
#define TYP_IFzipcv(fun) fun##Max
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_and_init[] = {
#define TYP_IFzipcv(fun) fun##And
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_or_init[] = {
#define TYP_IFzipcv(fun) fun##Or
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_eq_init[] = {
#define TYP_IFzipcv(fun) fun##Eq
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_neq_init[] = {
#define TYP_IFzipcv(fun) fun##Neq
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_le_init[] = {
#define TYP_IFzipcv(fun) fun##Le
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_lt_init[] = {
#define TYP_IFzipcv(fun) fun##Lt
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_gt_init[] = {
#define TYP_IFzipcv(fun) fun##Gt
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_ge_init[] = {
#define TYP_IFzipcv(fun) fun##Ge
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_not_init[] = {
#define TYP_IFzipcv(fun) fun##Not
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_toi_init[] = {
#define TYP_IFzipcv(fun) fun##Toi
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_tof_init[] = {
#define TYP_IFzipcv(fun) fun##Tof
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_tod_init[] = {
#define TYP_IFzipcv(fun) fun##Tod
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_abs_init[] = {
#define TYP_IFzipcv(fun) fun##Abs
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_neg_init[] = {
#define TYP_IFzipcv(fun) fun##Neg
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const basecvfunptr basecv_zero_init[] = {
#define TYP_IFbasecv(fun) fun##Zero
#include "type_info.mac"
#undef TYP_IFbasecv
};

static const basecvfunptr basecv_one_init[] = {
#define TYP_IFbasecv(fun) fun##One
#include "type_info.mac"
#undef TYP_IFbasecv
};

static const cv2cvfunptr cv2cv_init[] = {
#define TYP_IFcv2cv(fun) fun
#include "type_info.mac"
#undef TYP_IFcv2cv
};

static const cv2scalarfunptr cv2scalar_init[] = {
#define TYP_IFcv2scal(fun) fun
#include "type_info.mac"
#undef TYP_IFcv2scal
};

static const cv2strfunptr cv2str_init[] = {
#define TYP_IFcv2str(fun) fun
#include "type_info.mac"
#undef TYP_IFcv2str
};

#if 0
static const char *mdb_nodetype_init[] = {
#define NIFmdb_nodetype(mdb_nodetype) mdb_nodetype
#include "node_info.mac"
#undef NIFmdb_nodetype
};
#endif

static const char *mdb_prf_init[] = {
#define PRF_IF(a, b, c, d, e, f, g, h) b
#include "prf_node_info.mac"
#undef PRF_IF
};

static const char *mdb_type_init[] = {
#define TYP_IFdb_str(str) str
#include "type_info.mac"
#undef TYP_IFdb_str
};

static const char *mdb_statustype_init[] = {
#define SELECTtext(it_text) it_text
#include "status_info.mac"
#undef SELECTtext
};

/*
 * Initialize optimization defaults from optimize.mac
 */

#ifdef PRODUCTION

static optimize_t optimize_init = {
#define OPTdevl(devl) devl
#define OPTdelim ,
#include "optimize.mac"
#undef OPTdelim
#undef OPTdevl
};

#else /* PRODUCTION */

static optimize_t optimize_init = {
#define OPTprod(prod) prod
#define OPTdelim ,
#include "optimize.mac"
#undef OPTdelim
#undef OPTprod
};

#endif /* PRODUCTION */

/*
 * Initialize global variables from globals.mac
 */

global_t global = {
#define GLOBALinit(it_init) it_init
#define GLOBALdelim ,
#include "globals.mac"
#undef GLOBALdelim
#undef GLOBALinit
};
