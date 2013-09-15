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

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "str.h"
#include "memory.h"

#include "constants.h"
#include "basecv.h"
#include "zipcv.h"
#include "cv2cv.h"
#include "cv2scalar.h"
#include "cv2str.h"
#include "str.h"
#include "memory.h"
#include "check_mem.h"
#include "constant_folding.h"
#include "symbolic_constant_simplification.h"
#include "structural_constant_constant_folding.h"
#include "saa_constant_folding.h"
#include "ctinfo.h"
#include "free.h"
#include "namespaces.h"
#include "resource.h"

#include <limits.h>

/*
 * global variable to hold the global variables
 */

global_t global;

/*
 * auxiliary data structures for initialisation
 */

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

static char *nt_mutc_storage_class_string_init[] = {
#define ATTRIB NT_MUTC_STORAGE_CLASS_INDEX
#define NTIFstr(it_str) it_str
#include "nt_info.mac"
#undef ATTRIB
#undef NTIFstr
};

static char *nt_mutc_scope_string_init[] = {
#define ATTRIB NT_MUTC_SCOPE_INDEX
#define NTIFstr(it_str) it_str
#include "nt_info.mac"
#undef ATTRIB
#undef NTIFstr
};

static char *nt_mutc_usage_string_init[] = {
#define ATTRIB NT_MUTC_USAGE_INDEX
#define NTIFstr(it_str) it_str
#include "nt_info.mac"
#undef ATTRIB
#undef NTIFstr
};

static char *nt_bitarray_string_init[] = {
#define ATTRIB NT_BITARRAY_INDEX
#define NTIFstr(it_str) it_str
#include "nt_info.mac"
#undef ATTRIB
#undef NTIFstr
};

static const char *backend_string[] = {
#define BACKENDstring(string) string,
#include "backends.mac"
#undef BACKENDstring
  "UNKNOWN"};

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

static const zipcvfunptr zipcv_tob_init[] = {
#define TYP_IFzipcv(fun) fun##Tob
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_tos_init[] = {
#define TYP_IFzipcv(fun) fun##Tos
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_tobool[] = {
#define TYP_IFzipcv(fun) fun##Tobool
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_toc_init[] = {
#define TYP_IFzipcv(fun) fun##Toc
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_toi_init[] = {
#define TYP_IFzipcv(fun) fun##Toi
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_tol_init[] = {
#define TYP_IFzipcv(fun) fun##Tol
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_toll_init[] = {
#define TYP_IFzipcv(fun) fun##Toll
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_toub_init[] = {
#define TYP_IFzipcv(fun) fun##Toub
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_tous_init[] = {
#define TYP_IFzipcv(fun) fun##Tous
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_toui_init[] = {
#define TYP_IFzipcv(fun) fun##Toui
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_toul_init[] = {
#define TYP_IFzipcv(fun) fun##Toul
#include "type_info.mac"
#undef TYP_IFzipcv
};

static const zipcvfunptr zipcv_toull_init[] = {
#define TYP_IFzipcv(fun) fun##Toull
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

static const basecvfunptr basecv_negativeone_init[] = {
#define TYP_IFbasecv(fun) fun##NegativeOne
#include "type_info.mac"
#undef TYP_IFbasecv
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

/*static configuration_t */
#define config_init                                                                      \
    (configuration_t)                                                                    \
    {                                                                                    \
        NULL,   /* cc			  */                                                             \
          NULL, /* ccflags		  */                                                         \
          NULL, /* ccincdir		  */                                                        \
          NULL, /* cclibdir		  */                                                        \
          NULL, /* ldflags		  */                                                         \
          NULL, /* cclink		  */                                                          \
          NULL, /* ccmtlink		  */                                                        \
          NULL, /* ccdllink		  */                                                        \
          NULL, /* cext		  */                                                            \
          NULL, /* rc_method		  */                                                       \
          NULL, /* backend		  */                                                         \
          NULL, /* mt_lib               */                                               \
          NULL, /* tree_cc		  */                                                         \
          NULL, /* tree_ld		  */                                                         \
          NULL, /* tree_ld_path	  */                                                     \
          NULL, /* lib_variant	  */                                                      \
          NULL, /* tree_cext		  */                                                       \
          NULL, /* opt_O0		  */                                                          \
          NULL, /* opt_O1		  */                                                          \
          NULL, /* opt_O2		  */                                                          \
          NULL, /* opt_O3		  */                                                          \
          NULL, /* opt_g		  */                                                           \
          NULL, /* opt_D		  */                                                           \
          NULL, /* opt_I		  */                                                           \
          NULL, /* cpp_stdin		  */                                                       \
          NULL, /* cpp_file		  */                                                        \
          NULL, /* tar_create		  */                                                      \
          NULL, /* tar_extract	  */                                                      \
          NULL, /* ar_create		  */                                                       \
          NULL, /* ld_dynamic		  */                                                      \
          NULL, /* genpic		  */                                                          \
          NULL, /* ld_path		  */                                                         \
          NULL, /* ranlib		  */                                                          \
          NULL, /* mkdir		  */                                                           \
          NULL, /* rmdir		  */                                                           \
          NULL, /* chdir		  */                                                           \
          NULL, /* cat		  */                                                             \
          NULL, /* move		  */                                                            \
          NULL, /* rsh		  */                                                             \
          NULL, /* dump_output	  */                                                      \
          NULL, /* cuda_arch		  */                                                       \
          NULL, /* libpath		  */                                                         \
          NULL, /* imppath		  */                                                         \
          NULL, /* extlibpath		  */                                                      \
          NULL, /* tmpdir		  */                                                          \
          0,    /* cache1_size	  */                                                      \
          0,    /* cache1_line	  */                                                      \
          0,    /* cache1_assoc	  */                                                     \
          NULL, /* cache1_writepol	  */                                                  \
          0,    /* cache1_msca_factor	  */                                               \
          0,    /* cache2_size	  */                                                      \
          0,    /* cache2_line	  */                                                      \
          0,    /* cache2_assoc	  */                                                     \
          NULL, /* cache2_writepol	  */                                                  \
          0,    /* cache2_msca_factor	  */                                               \
          0,    /* cache3_size	  */                                                      \
          0,    /* cache3_line	  */                                                      \
          0,    /* cache3_assoc	  */                                                     \
          NULL, /* cache3_writepol	  */                                                  \
          0,    /* cache3_msca_factor;  */                                               \
    }
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
 * Initialize printfunsets flags from flags.mac
 */

static printfunsets_flags_t printfunsets_init = {
#define PRINTFUNSETSdefault(default) default,
#include "flags.mac"
};

static printfunsets_flags_t printfunsets_all_init = {
#define PRINTFUNSETSdefault(default) TRUE,
#include "flags.mac"
};

static printfunsets_flags_t printfunsets_none_init = {
#define PRINTFUNSETSdefault(default) FALSE,
#include "flags.mac"
};

/*
 * Initialize visualizefunsets flags from flags.mac
 */

static visualizefunsets_flags_t visualizefunsets_init = {
#define VISUALIZEFUNSETSdefault(default) default,
#include "flags.mac"
};

static visualizefunsets_flags_t visualizefunsets_all_init = {
#define VISUALIZEFUNSETSdefault(default) TRUE,
#include "flags.mac"
};

static visualizefunsets_flags_t visualizefunsets_none_init = {
#define VISUALIZEFUNSETSdefault(default) FALSE,
#include "flags.mac"
};

/*
 * Initialize print flags from flags.mac
 */

static print_flags_t print_init = {
#define PRINTdefault(default) default,
#include "flags.mac"
};

static print_flags_t print_all_init = {
#define PRINTdefault(default) TRUE,
#include "flags.mac"
};

static print_flags_t print_none_init = {
#define PRINTdefault(default) FALSE,
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
 * Initialize  primitive function tables
 */

static const char *prf_name_init[] = {
#define PRFname(name) "_" #name "_"
#include "prf_info.mac"
};

static const arg_encoding_t prf_arg_encoding_init[] = {
#define PRFarg_encoding_h(a, b, c) a, b, c
#define PRFarg_encoding(arg_encoding) PRFarg_encoding_h arg_encoding
#include "prf_info.mac"
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

    DBUG_ENTER ();

    aps = (int **)MEMmalloc (maxfun * sizeof (int *));

    for (i = 0; i < maxfunap; i++) {
        aps[i] = (int *)MEMmalloc (maxfunap * sizeof (int));
    }

    DBUG_RETURN (aps);
}

/*
 * Initialize global variables from globals.mac
 */

void
GLOBinitializeGlobal (int argc, char *argv[], tool_t tool, char *toolname)
{
    DBUG_ENTER ();

#define GLOBAL(type, name, val, ...) global.name = val;
#include "globals.mac"

    global.argc = argc;
    global.argv = argv;

    global.tool = tool;
    global.toolname = toolname;

    /* Make sure that dynamically allocated strings are actually
     * empty strings, otherwise there might be some surprises
     * later :)
     */

    global.ccflags[0] = '\0';
    global.cachesim_host[0] = '\0';
    global.cachesim_file[0] = '\0';
    global.cachesim_dir[0] = '\0';

    memset (global.profile_funnme, 0, sizeof (char *) * PF_MAXFUN);
    memset (global.profile_funapcntr, 0, sizeof (int) * PF_MAXFUN);

    DBUG_RETURN ();
}

/*
 * Initialize global variables from resource file entries
 */

void
GLOBsetupBackend (void)
{
    DBUG_ENTER ();

    if (STReq (global.config.backend, "")) {
        global.backend = BE_c99;
    }
#define BACKEND(type, string)                                                            \
    else if (STReqci (global.config.backend, string))                                    \
    {                                                                                    \
        global.backend = type;                                                           \
    }
#include "backends.mac"
    else {
        CTIabort ("Unknown compiler backend in sac2crc file: %s", global.config.backend);
    }

    DBUG_RETURN ();
}

#define xfree_dummy(...)

static void
xfree_char_ptr (char *ptr)
{
    if (ptr && !__builtin_constant_p (ptr))
        MEMfree (ptr);
}

static void
xfree_int_ptr (int *ptr)
{
    if (ptr && !__builtin_constant_p (ptr))
        MEMfree (ptr);
}

static void
xfree_char_ptr_ptr (char **ptr, size_t size)
{
    size_t i;
    if (!ptr || __builtin_constant_p (ptr))
        return;

    for (i = 0; i < size; i++)
        if (ptr[i] && !__builtin_constant_p (ptr[i]))
            MEMfree (ptr[i]);
    if (ptr)
        MEMfree (ptr);
}

static UNUSED void
xfree_node (node *node)
{
    if (node)
        FREEdoFreeTree (node);
}

static void
xfree_apline (int **ptr, size_t maxfun)
{
    size_t i;

    for (i = 0; i < maxfun; i++)
        if (ptr[i])
            MEMfree (ptr[i]);

    if (ptr)
        MEMfree (ptr);
}

void
GLOBfinalizeGlobal (void)
{
    DBUG_ENTER ();

    xfree_namespace_pool ();

#define GLOBAL(type, name, val, func, ...) func (global.name __VA_ARGS__);
#include "globals.mac"
    DBUG_RETURN ();
}

#undef DBUG_PREFIX
