/*
 *
 * $Id$
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

#include "dbug.h"
#include "internal_lib.h"

#include "ct_prf.h"
#include "constants.h"
#include "basecv.h"
#include "zipcv.h"
#include "cv2cv.h"
#include "cv2scalar.h"
#include "cv2str.h"
#include "check_mem.h"

#include <limits.h>

static bool argtag_has_shp_init[] = {
#define SELECTshp(it_shp) it_shp
#include "argtag_info.mac"
#undef SELECTshp
};

static bool argtag_has_rc_init[] = {
#define SELECTrc(it_rc) it_rc
#include "argtag_info.mac"
#undef SELECTrc
};

static bool argtag_has_desc_init[] = {
#define SELECTdesc(it_desc) it_desc
#include "argtag_info.mac"
#undef SELECTdesc
};

static bool argtag_is_in_init[] = {
#define SELECTin(it_in) it_in
#include "argtag_info.mac"
#undef SELECTin
};

static bool argtag_is_out_init[] = {
#define SELECTout(it_out) it_out
#include "argtag_info.mac"
#undef SELECTout
};

static bool argtag_is_inout_init[] = {
#define SELECTinout(it_inout) it_inout
#include "argtag_info.mac"
#undef SELECTinout
};

static const char *argtag_string_init[] = {
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
#define PRF_IF(a, b, c, d, e, f, g, h, i) c
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

static const ct_funptr ntc_funtab_init[] = {
#define PRF_IF(a, b, c, d, e, f, g, h, i) g
#include "prf_node_info.mac"
#undef PRF_IF
};

static const void *ntc_cffuntab_init[] = {
#define PRF_IF(a, b, c, d, e, f, g, h, i) (void *)h
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

static const char *mdb_nodetype_init[] = {
#define NIFname(name) name
#include "node_info.mac"
#undef NIFname
};

static const char *mdb_prf_init[] = {
#define PRF_IF(a, b, c, d, e, f, g, h, i) b
#include "prf_node_info.mac"
#undef PRF_IF
};

static const char *mdb_type_init[] = {
#define TYP_IFdb_str(str) str
#include "type_info.mac"
#undef TYP_IFdb_str
};

static const char *type_string_init[] = {
#define TYP_IFpr_str(str) str
#include "type_info.mac"
};

static const char *rename_type_init[] = {
#define TYP_IFfunr_str(str) str
#include "type_info.mac"
};

static configuration_t config_init;
/*
 * This is only a dirty trick to fake an a-priori initialization
 * of config, Which is not possible otherwise without a major code
 * rewrite.
 */

/*
 * Initialize optimization defaults from optimize.mac
 */

static optimize_flags_t optimize_developer_init = {
#define OPTIMIZEdevl(devl) devl,
#include "optimize.mac"
};

static optimize_flags_t optimize_production_init = {
#define OPTIMIZEprod(prod) prod,
#include "optimize.mac"
};

static optimize_flags_t optimize_all_init = {
#define OPTIMIZEprod(prod) TRUE,
#include "optimize.mac"
};

static optimize_flags_t optimize_none_init = {
#define OPTIMIZEprod(prod) FALSE,
#include "optimize.mac"
};

static optimize_counter_t optimize_counter_init = {
#define OPTCOUNTERid(id) 0,
#include "optimize.mac"
};

/*
 * Initialize trace flags from flags.mac
 */

static trace_flags_t trace_init = {
#define TRACEdefault(default) default,
#include "flags.mac"
};

static trace_flags_t trace_all_init = {
#define TRACEdefault(default) TRUE,
#include "flags.mac"
};

static trace_flags_t trace_none_init = {
#define TRACEdefault(default) FALSE,
#include "flags.mac"
};

/*
 * Initialize cache simulation flags from flags.mac
 */

static cachesim_flags_t cachesim_init = {
#define CSdefault(default) default,
#include "flags.mac"
};

static cachesim_flags_t cachesim_all_init = {
#define CSdefault(default) TRUE,
#include "flags.mac"
};

static cachesim_flags_t cachesim_none_init = {
#define CSdefault(default) FALSE,
#include "flags.mac"
};

/*
 * Initialize profile flags from flags.mac
 */

static profile_flags_t profile_init = {
#define PROFILEdefault(default) default,
#include "flags.mac"
};

static profile_flags_t profile_all_init = {
#define PROFILEdefault(default) TRUE,
#include "flags.mac"
};

static profile_flags_t profile_none_init = {
#define PROFILEdefault(default) FALSE,
#include "flags.mac"
};

/*
 * Initialize runtime check flags from flags.mac
 */

static runtimecheck_flags_t runtimecheck_init = {
#define RTCdefault(default) default,
#include "flags.mac"
};

static runtimecheck_flags_t runtimecheck_all_init = {
#define RTCdefault(default) TRUE,
#include "flags.mac"
};

static runtimecheck_flags_t runtimecheck_none_init = {
#define RTCdefault(default) FALSE,
#include "flags.mac"
};

/*
 * Initialize library generation flags from flags.mac
 */

static genlib_flags_t genlib_init = {
#define GENLIBdefault(default) default,
#include "flags.mac"
};

/*
 * Initialize  primitive function tables
 */

static char *prf_symbol_init[] = {
#define PRF_IF(a, b, c, d, e, f, g, h, i) d
#include "prf_node_info.mac"
#undef PRF_IF
};

static bool prf_is_infix_init[] = {
#define PRF_IF(a, b, c, d, e, f, g, h, i) e
#include "prf_node_info.mac"
#undef PRF_IF
};

/*
 * Initialize specialization mode string representations
 */

static char *spec_mode_str_init[] = {"aks", "akd", "aud"};

/*
 * Initialize signature specialization mode string representations
 */

static char *sigspec_mode_str_init[] = {"akv", "aks", "akd", "aud"};

/*
 * Initialization helper functions
 */

static int **
BuildFunApLine (int maxfun, int maxfunap)
{
    int i, **aps;

    DBUG_ENTER ("BuildFunApLine");

    aps = (int **)ILIBmalloc (maxfun * sizeof (int *));

    for (i = 0; i < maxfunap; i++) {
        aps[i] = (int *)ILIBmalloc (maxfunap * sizeof (int));
    }

    DBUG_RETURN (aps);
}

/*
 * Initialize global variables from globals.mac
 */

global_t global;

void
GLOBinitializeGlobal ()
{
    DBUG_ENTER ("GLOBinitializeGlobal");

#define GLOBALname(name) global.name =
#define GLOBALinit(init) init;
#include "globals.mac"

    DBUG_VOID_RETURN;
}
