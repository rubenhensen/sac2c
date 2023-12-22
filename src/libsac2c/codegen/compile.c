/**
 *
 * @file compile.c
 *
 * This file implements the code generation (SAC code -> C code with ICMs) for
 * the new backend
 */

#include "compile.h"

#include <stdlib.h>
#include <math.h>
#include <dirent.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"

#include "globals.h"

#define DBUG_PREFIX "COMP"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "DataFlowMask.h"
#include "NameTuplesUtils.h"
#include "scheduling.h"
#include "wl_bounds.h"
#include "new_types.h"
#include "user_types.h"
#include "type_utils.h"
#include "shape.h"
#include "LookUpTable.h"
#include "convert.h"
#include "math_utils.h"
#include "types.h"
#include "cuda_utils.h"
#include "regression.h"
#include "namespaces.h"

#define FOLDFIX_LABEL_GENERATION_ACTIVE 1

/*
 * arg_info
 * ========
 *
 * Other:
 *
 *   INFO_MODUL       : pointer to current modul
 *   INFO_FUNDEF      : pointer to current fundef
 *
 *   INFO_LASTIDS     : pointer to IDS of current let
 *
 *   INFO_FOLDFUNS    : [flag]
 *     In order to guarantee that the special fold-funs are compiled *before*
 *     the associated with-loops, the fundef chain is traversed twice.
 *     During the first traversal (FOLDFUNS == TRUE) only special fold-funs
 *     are traversed. During the second traversal (FOLDFUNS == FALSE) only
 *     the other functions are traversed.
 *
 *   INFO_FUNPOST     : icms to be placed at the end of the function body
 *     Currently used by suballoc in the mutc backend to free descriptors
 *     at the end of a thread function.
 *
 *   INFO_CONCURRENTRANGES: used to pass down the WITH3_USECONCURRENTRANGES
 *                          during code generation
 *
 *   INFO_COND        : inside a conditional id or bool
 */

/*
 * INFO structure
 */
struct INFO {
    node *modul;
    node *fundef;
    node *lastids;
    node *assign;
    int schedid;
    node *schedinit;
    node *idxvec;
    node *offsets;
    node *lowervec;
    node *uppervec;
    node *icmchain;
    bool isfull;
    node *spmdframe;
    node *spmdbarrier;
    char *break_label;
    lut_t *foldlut;
    node *postfun;
    bool concurrentranges;
    bool cond;
    node *withloop;
    node *with3folds;
    node *let;
    node *fpframe;
    node *withops;
    node *vardec_init;
    char *with3_dist;
    node *with2_cond;
    FILE **fp_list;
    size_t *line_count;
    int nr_threads;
    int nr_files;
};

/*
 * INFO macros
 */
#define INFO_MODUL(n) ((n)->modul)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LASTIDS(n) ((n)->lastids)
#define INFO_ASSIGN(n) ((n)->assign)
#define INFO_SCHEDULERID(n) ((n)->schedid)
#define INFO_SCHEDULERINIT(n) ((n)->schedinit)
#define INFO_IDXVEC(n) ((n)->idxvec)
#define INFO_OFFSETS(n) ((n)->offsets)
#define INFO_LOWERVEC(n) ((n)->lowervec)
#define INFO_UPPERVEC(n) ((n)->uppervec)
#define INFO_ICMCHAIN(n) ((n)->icmchain)
#define INFO_ISFULL_AUDWL(n) ((n)->isfull)
#define INFO_SPMDFRAME(n) ((n)->spmdframe)
#define INFO_SPMDBARRIER(n) ((n)->spmdbarrier)
#define INFO_BREAKLABEL(n) ((n)->break_label)
#define INFO_FOLDLUT(n) ((n)->foldlut)
#define INFO_POSTFUN(n) ((n)->postfun)
#define INFO_CONCURRENTRANGES(n) ((n)->concurrentranges)
#define INFO_COND(n) ((n)->cond)
#define INFO_WITHLOOP(n) ((n)->withloop)
#define INFO_WITH3_FOLDS(n) ((n)->with3folds)
#define INFO_LET(n) ((n)->let)
#define INFO_FPFRAME(n) ((n)->fpframe)
#define INFO_WITHOPS(n) ((n)->withops)
#define INFO_VARDEC_INIT(n) ((n)->vardec_init)
#define INFO_WITH3_DIST(n) ((n)->with3_dist)
#define INFO_WITH2_COND(n) ((n)->with2_cond)
#define INFO_FP_LIST(n) ((n)->fp_list)
#define INFO_LINE_COUNT(n) ((n)->line_count)
#define INFO_NR_THREADS(n) ((n)->nr_threads)
#define INFO_NR_FILES(n) ((n)->nr_files)
/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_MODUL (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_LASTIDS (result) = NULL;
    INFO_ASSIGN (result) = NULL;
    INFO_SCHEDULERID (result) = 0;
    INFO_SCHEDULERINIT (result) = NULL;
    INFO_IDXVEC (result) = NULL;
    INFO_OFFSETS (result) = NULL;
    INFO_LOWERVEC (result) = NULL;
    INFO_UPPERVEC (result) = NULL;
    INFO_ICMCHAIN (result) = NULL;
    INFO_ISFULL_AUDWL (result) = FALSE;
    INFO_SPMDFRAME (result) = NULL;
    INFO_SPMDBARRIER (result) = NULL;
    INFO_FOLDLUT (result) = NULL;
    INFO_POSTFUN (result) = NULL;
    INFO_COND (result) = FALSE;
    INFO_WITHLOOP (result) = NULL;
    INFO_WITH3_FOLDS (result) = NULL;
    INFO_LET (result) = NULL;
    INFO_FPFRAME (result) = NULL;
    INFO_WITHOPS (result) = NULL;
    INFO_VARDEC_INIT (result) = NULL;
    INFO_WITH3_DIST (result) = NULL;
    INFO_WITH2_COND (result) = NULL;
    INFO_FP_LIST (result) = NULL;
    INFO_LINE_COUNT (result) = NULL;
    INFO_NR_THREADS (result) = 0;
    INFO_NR_FILES (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/***
 ***  RC: moved here temporarily for compatibility reasons from refcount.h
 ***/

/* value, representing an inactive reference counter */
#define RC_INACTIVE (-1)

/*
 * macros for testing the RC status
 */
#define RC_IS_INACTIVE(rc) ((rc) == RC_INACTIVE)
#define RC_IS_ACTIVE(rc) ((rc) >= 0) /* == (RC_IS_ZERO(rc) || RC_IS_VITAL(rc)) */

#define RC_IS_LEGAL(rc) ((RC_IS_INACTIVE (rc)) || (RC_IS_ACTIVE (rc)))

#define RC_IS_VITAL(rc) ((rc) > 0)

#if 0
/* These macros seem not to be used.
 * The only reason I keep them here is that they might shed a light on how RC
 * is meant to be used.... (Bodo 2016)
 */
#define RC_INIT(rc) (RC_IS_ACTIVE (rc) ? 1 : (rc))

#define RC_IS_ZERO(rc) ((rc) == 0)

#endif

/******************************************************************************
 *
 * Enum type for specifying kinds of Generic Functions
 *
 *****************************************************************************/

typedef enum { GF_copy, GF_free } generic_fun_t;

/*******************************************************************************
 *
 * Struct used by the smart decision tool
 *
 ******************************************************************************/

struct smart_decision_t {
    int64_t *nr_measurements;
    int64_t *cum_time;
    int64_t problem_size;
    float *fun_time;
    float max_time;
    float min_time;
};

/******************************************************************************
 *
 * static global variables for the compilation of the new with-loop
 *
 ******************************************************************************/

static node *wlids = NULL;
static node *wlnode = NULL;
static node *wlseg = NULL;
static node *wlstride = NULL;

/* postfix for goto labels */
#define LABEL_POSTFIX "SAC_label"

#if 0
/*
 * This macro indicates whether there are multiple segments present or not.
 * However, it seems not to be used. Again, I only keep it as it may shed some
 * light on the remaining code ... (Bodo 2016)
 */
#define MULTIPLE_SEGS(seg) ((seg != NULL) && (WLSEGX_NEXT (seg) != NULL))
#endif

/******************************************************************************
 *
 * static global variables for the compilation of primitive functions
 *
 ******************************************************************************/

static char *prf_ccode_tab[] = {
#define PRFccode(ccode) "SAC_ND_PRF_" #ccode
#include "prf_info.mac"
};

static node *
With3Folds (node *ids, node *ops)
{
    node *res = NULL;
    DBUG_ENTER ();

    if (IDS_NEXT (ids) != NULL) {
        res = With3Folds (IDS_NEXT (ids), WITHOP_NEXT (ops));
    }

    if (NODE_TYPE (ops) == N_fold) {
        res = TBmakeIds (IDS_AVIS (ids), res);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn  char *GetBasetypeStr( ntype *type)
 *
 * @brief  Returns the basetype string of the given type, i.e. "TYPES_NAME" if
 *         type represents a user-defined type and "TYPES_BASETYPE" otherwise.
 *
 ******************************************************************************/

static const char *
GetBasetypeStr (ntype *type)
{
    simpletype basetype;
    const char *str;

    DBUG_ENTER ();

    if (TUisArrayOfUser (type)) {
        str = UTgetName (TYgetUserType (TYgetScalar (type)));
        DBUG_ASSERT (str != NULL, "Name of user-defined type not found");
    } else {
        DBUG_ASSERT (TUisArrayOfSimple (type), "Expected either array of User or Simple type.");
        basetype = TUgetBaseSimpleType (type);

        if (basetype == T_bool_dev) {
            str = "bool";
        } else if (basetype == T_int_dev || basetype == T_int_shmem) {
            str = "int";
        } else if (basetype == T_long_dev || basetype == T_long_shmem) {
            str = "long";
        } else if (basetype == T_longlong_dev || basetype == T_longlong_shmem) {
            str = "long long";
        } else if (basetype == T_float_dev || basetype == T_float_shmem) {
            str = "float";
        } else if (basetype == T_double_dev || basetype == T_double_shmem
                   || basetype == T_double) {
            /* If the enforce_float flag is set,
             * we change all doubles to floats */
            if (global.enforce_float) {
                str = "float";
            } else {
                str = "double";
            }
        } else if (basetype == T_int_dist || basetype == T_long_dist
                   || basetype == T_longlong_dist || basetype == T_float_dist
                   || basetype == T_double_dist) {
            str = "struct dist_var";
        } else {
            str = global.type_string[basetype];
        }
    }

    DBUG_RETURN (str);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeBasetypeArg( ntype *type)
 *
 * @brief  Creates a new N_id node containing the basetype string of the given
 *         type.
 *
 ******************************************************************************/

static node *
MakeBasetypeArg (ntype *type)
{
    node *ret_node;
    const char *str;

    DBUG_ENTER ();

    str = GetBasetypeStr (type);

    ret_node = TCmakeIdCopyString (str);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeBasetypeArg_NT( ntype *type)
 *
 * @brief  Creates a new N_id node containing the basetype string of the given
 *         type.
 *
 ******************************************************************************/

static node *
MakeBasetypeArg_NT (ntype *type)
{
    node *ret_node;
    const char *str;

    DBUG_ENTER ();

    str = GetBasetypeStr (type);

    ret_node = TCmakeIdCopyStringNtNew (str, type);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeTypeArgs( char *name, ntype *type,
 *                          bool add_type, bool add_dim, bool add_shape,
 *                          node *exprs)
 *
 * @brief  Creates a chain of N_exprs nodes containing name, type,
 *         dimensionality and shape components of the given object.
 *
 ******************************************************************************/

static node *
MakeTypeArgs (char *name, ntype *type, bool add_type, bool add_dim, bool add_shape,
              node *exprs)
{
    int dim;
    ntype *itype;

    DBUG_ENTER ();

    itype = TUcomputeImplementationType (type);
    dim = TUgetFullDimEncoding (itype);

    /*
     * CAUTION:
     * It is important that (dim <= 0) holds for AKD and AUD arrays
     * otherwise the VARINT-interpretation of the shape-args would fail
     * during icm2c!!
     */
    if (add_shape && (dim > 0)) { // at least AKS
        exprs = TCappendExprs (SHshape2Exprs (TYgetShape (itype)), exprs);
    }

    if (add_dim) {
        exprs = TBmakeExprs (TBmakeNum (dim), exprs);
    }

    if (add_type) {
        exprs = TBmakeExprs (MakeBasetypeArg (itype), exprs);
    }

    exprs = TBmakeExprs (TCmakeIdCopyStringNtNew (name, itype), exprs);
    itype = TYfreeType (itype);

    DBUG_RETURN (exprs);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeDimArg( node *arg, bool int_only)
 *
 * @brief  Creates an ICM which computes the dimension of 'arg'.
 *         In case of nested arrays, i.e. a constant array which contains
 *         identifiers as elements, only the top-level dimension (1) is
 *         returned.
 *
 ******************************************************************************/
static node *
MakeDimArg (node *arg, bool int_only)
{
    node *ret;

    DBUG_ENTER ();

    if (NODE_TYPE (arg) == N_id) {
        int dim = TUgetDimEncoding (ID_NTYPE (arg));
        if (dim >= 0) {
            ret = TBmakeNum (dim);
        } else if (int_only) {
            ret = TBmakeNum (ARRAY_OR_SCALAR);
        } else {
            ret = TCmakeIcm1 ("ND_A_DIM", DUPdupIdNt (arg));
        }
    } else if (NODE_TYPE (arg) == N_array) {
        if (ARRAY_STRING (arg) != NULL) {
            ret = TBmakeNum (1);
        } else {
            ret = TBmakeNum (1);
            /* dimension of array elements is omitted! */
        }
    } else if (NODE_TYPE (arg) == N_num) {
        ret = TBmakeNum (0);
    } else if (NODE_TYPE (arg) == N_float) {
        ret = TBmakeNum (0);
    } else if (NODE_TYPE (arg) == N_double) {
        ret = TBmakeNum (0);
    } else if (NODE_TYPE (arg) == N_bool) {
        ret = TBmakeNum (0);
    } else if (NODE_TYPE (arg) == N_char) {
        ret = TBmakeNum (0);
    } else {
        DBUG_UNREACHABLE ("not yet implemented");
        ret = NULL;
    }

    DBUG_RETURN (ret);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeSizeArg( node *arg, bool int_only)
 *
 * @brief  ...
 *
 *****************************************************************************/
static node *
MakeSizeArg (node *arg, bool int_only)
{
    node *ret;

    DBUG_ENTER ();

    if (NODE_TYPE (arg) == N_id) {
        ntype *type = ID_NTYPE (arg);
        if (TUshapeKnown (type)) {
            ret = TBmakeNum (SHgetUnrLen (TYgetShape (type)));
        } else if (int_only) {
            ret = TBmakeNum (-1);
        } else {
            ret = TCmakeIcm1 ("ND_A_SIZE", DUPdupIdNt (arg));
        }
    } else if (NODE_TYPE (arg) == N_array) {
        ret = TBmakeNum (TCcountExprs (ARRAY_AELEMS (arg)));
        /* size of array elements is omitted! */
    } else if (NODE_TYPE (arg) == N_num) {
        ret = TBmakeNum (1);
    } else if (NODE_TYPE (arg) == N_float) {
        ret = TBmakeNum (1);
    } else if (NODE_TYPE (arg) == N_double) {
        ret = TBmakeNum (1);
    } else if (NODE_TYPE (arg) == N_bool) {
        ret = TBmakeNum (1);
    } else if (NODE_TYPE (arg) == N_char) {
        ret = TBmakeNum (1);
    } else {
        DBUG_UNREACHABLE ("not yet implemented");
        ret = NULL;
    }

    DBUG_RETURN (ret);
}

/** <!--********************************************************************-->
 *
 * @fn  char *GenericFun( generic_fun_t which, ntype *type)
 *
 * @brief  Returns the name of the specified generic function for the given
 *         type.
 *
 * @param which
 *
 ******************************************************************************/

static char *
GenericFun (generic_fun_t which, ntype *type)
{
    char *ret = NULL;
#ifndef DBUG_OFF
    char *tmp;
#endif
    usertype utype;

    DBUG_ENTER ();

    DBUG_EXECUTE_TAG ("COMP_GEN", tmp = CVtype2String (type, 0, FALSE); switch (which) {
        case GF_copy:
            DBUG_PRINT_TAG ("COMP_GEN","Looking for generic copy function for type %s", tmp);
            break;
        case GF_free:
            DBUG_PRINT_TAG ("COMP_GEN","Looking for generic free function for type %s", tmp);
            break;
    } tmp = MEMfree (tmp));

    DBUG_ASSERT (type != NULL, "no type found!");

    if (TYisUser (type)) {

        utype = TYgetUserType (type);

        DBUG_ASSERT ((utype != UT_NOT_DEFINED)
                       && (!TYisUser (TYgetScalar (UTgetBaseType (utype)))),
                     "unresolved nested user-defined type found");

        if (TYgetSimpleType (TYgetScalar (UTgetBaseType (utype))) == T_hidden) {
            switch (which) {
            case GF_copy:
                ret = TYPEDEF_COPYFUN (UTgetTdef (utype));
                break;
            case GF_free:
                ret = TYPEDEF_FREEFUN (UTgetTdef (utype));
                break;
            }
        }
    }

    DBUG_PRINT_TAG ("COMP_GEN","Found generic fun `%s'", STRonNull ("", ret));

    DBUG_RETURN (ret);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPgetFoldCode( node *fundef)
 *
 * @brief  Returns the foldop-code of the special fold-fun 'fundef'.
 *
 *   This function simply extract the assignments of the fundef-body.
 *   It is assumed that the names of the variables are the same is in the
 *   context of the corresponding with-loop!
 *   This property is *not* hold before the compilation process has been
 *   started!
 *   (Note that Precompile() calls the function AdjustFoldFundef() for each
 *   fold-fundef!!)
 *
 ******************************************************************************/

node *
COMPgetFoldCode (node *fundef)
{
    node *fold_code;
    node *tmp;

    DBUG_ENTER ();

    DBUG_ASSERT (fundef != NULL, "no fundef found!");
    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef, "fold-fun corrupted!");

    /*
     * get code of the special fold-fun
     */
    fold_code = DUPdoDupTree (FUNDEF_ASSIGNS (fundef));

    /*
     * remove declaration-ICMs ('ND_DECL_ARG') from code.
     */
    while ((NODE_TYPE (ASSIGN_STMT (fold_code)) == N_icm)
           && (STReq (ICM_NAME (ASSIGN_STMT (fold_code)), "ND_DECL__MIRROR_PARAM"))) {
        fold_code = FREEdoFreeNode (fold_code);
    }

    /*
     * remove return-ICMs ('ND_FUN_RET') from code
     * (it is the last assignment)
     */
    tmp = fold_code;
    DBUG_ASSERT (ASSIGN_NEXT (tmp) != NULL, "corrupted fold code found!");
    while (ASSIGN_NEXT (ASSIGN_NEXT (tmp)) != NULL) {
        tmp = ASSIGN_NEXT (tmp);
    }
    DBUG_ASSERT (((NODE_TYPE (ASSIGN_STMT (ASSIGN_NEXT (tmp))) == N_icm)
                  && (STReq (ICM_NAME (ASSIGN_STMT (ASSIGN_NEXT (tmp))), "ND_FUN_RET"))),
                 "no ND_FUN_RET icm found in fold code!");
    ASSIGN_NEXT (tmp) = FREEdoFreeNode (ASSIGN_NEXT (tmp));

    DBUG_RETURN (fold_code);
}

/** <!--*******************************************************************-->
 *
 * @fn node *RemoveIdxDuplicates( node *withop)
 *
 ****************************************************************************/
static node *
RemoveIdxDuplicates (node *arg_node)
{
    node *withop;

    DBUG_ENTER ();

    withop = arg_node;
    while (withop != NULL) {
        if ((NODE_TYPE (withop) == N_genarray) || (NODE_TYPE (withop) == N_modarray)) {
            node *w2 = WITHOP_NEXT (withop);

            while (w2 != NULL) {
                if (((NODE_TYPE (w2) == N_genarray) || (NODE_TYPE (w2) == N_modarray))
                    && (WITHOP_IDX (w2) == WITHOP_IDX (withop))) {
                    if (NODE_TYPE (w2) == N_genarray) {
                        GENARRAY_IDX (w2) = NULL;
                    } else {
                        MODARRAY_IDX (w2) = NULL;
                    }
                }
                w2 = WITHOP_NEXT (w2);
            }
        }
        withop = WITHOP_NEXT (withop);
    }

    DBUG_RETURN (arg_node);
}

static /* forward declaration */
  node *
  DupExprs_NT_AddReadIcms (node *exprs);

/** <!--********************************************************************-->
 *
 * @fn  node *DupExpr_NT_AddReadIcms( node *expr)
 *
 * @brief  Dublicates the whole tree 'expr' and wraps a ND_READ-icm around each
 *         N_id which denotes a scalar.
 *
 ******************************************************************************/

static node *
DupExpr_NT_AddReadIcms (node *expr)
{
    node *new_expr = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (((expr != NULL) && (NODE_TYPE (expr) != N_exprs)),
                 "Illegal argument for DupExpr_NT_AddReadIcms() found!");

    if (NODE_TYPE (expr) == N_prf) {
        new_expr = TBmakePrf (PRF_PRF (expr), DupExprs_NT_AddReadIcms (PRF_ARGS (expr)));
    } else if (NODE_TYPE (expr) == N_id) {
        new_expr = DUPdupIdNt (expr);
        if (TUgetFullDimEncoding (ID_NTYPE (expr)) == SCALAR) {
            new_expr = TCmakeIcm2 ("ND_READ", new_expr, TBmakeNum (0));
        }
    } else {
        new_expr = DUPdoDupNode (expr);
    }

    DBUG_RETURN (new_expr);
}

/** <!--********************************************************************-->
 *
 * @fn  node *DupExprs_NT_AddReadIcms( node *exprs)
 *
 * @brief  ...
 *
 ******************************************************************************/

static node *
DupExprs_NT_AddReadIcms (node *exprs)
{
    node *new_exprs = NULL;

    DBUG_ENTER ();

    if (exprs != NULL) {
        DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "no N_exprs node found!");

        new_exprs = TBmakeExprs (DupExpr_NT_AddReadIcms (EXPRS_EXPR (exprs)),
                                 DupExprs_NT_AddReadIcms (EXPRS_NEXT (exprs)));
    }

    DBUG_RETURN (new_exprs);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeAnAllocDescIcm( char *name, ntype *type, int rc,
 *                                node *get_dim,
 *                                node *assigns)
 *
 * @brief  Builds a *_ALLOC__DESC( name) icm if needed.
 *
 *****************************************************************************/
static node *
MakeAnAllocDescIcm (char *name, ntype *type, int rc, node *get_dim, node *assigns,
                    char *icm)
{
    int dim;
    DBUG_ENTER ();

    DBUG_ASSERT (RC_IS_LEGAL (rc), "illegal RC value found!");

    if (RC_IS_ACTIVE (rc)) {
        if (get_dim == NULL) {
            dim = TYgetDim (type);
            DBUG_ASSERT (dim >= 0, "dimension undefined -> size of descriptor unknown");
            get_dim = TBmakeNum (dim);
        }

        assigns
          = TCmakeAssignIcm2 (icm, TCmakeIdCopyStringNtNew (name, type), get_dim, assigns);
    }
    DBUG_RETURN (assigns);
}
/** <!--********************************************************************-->
 *
 * @fn  node *MakeAllocDescIcm( char *name, ntype *type, int rc,
 *                              node *get_dim,
 *                              node *assigns)
 *
 * @brief  Builds a ND_ALLOC__DESC( name) icm if needed.
 *
 *****************************************************************************/

static node *
MakeAllocDescIcm (char *name, ntype *type, int rc, node *get_dim, node *assigns)
{
    DBUG_ENTER ();
    assigns = MakeAnAllocDescIcm (name, type, rc, get_dim, assigns, "ND_ALLOC__DESC");
    DBUG_RETURN (assigns);
}
/** <!--********************************************************************-->
 *
 * @fn  node *MakeMutcLocalAllocDescIcm( char *name, ntype *type, int rc,
 *                                       node *get_dim,
 *                                       node *assigns)
 *
 * @brief  Builds a MUTC_LOCAL_ALLOC__DESC( name) icm if needed.
 *
 *****************************************************************************/
static node *
MakeMutcLocalAllocDescIcm (char *name, ntype *type, int rc, node *get_dim, node *assigns)
{
    DBUG_ENTER ();

    assigns
      = MakeAnAllocDescIcm (name, type, rc, get_dim, assigns, "MUTC_LOCAL_ALLOC__DESC");
    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeSetRcIcm( char *name, ntype *type, int rc, node *assigns)
 *
 * @brief  Builds a ND_SET__RC( name, num) icm if needed.
 *
 ******************************************************************************/

static node *
MakeSetRcIcm (char *name, ntype *type, int rc, node *assigns)
{
    simpletype basetype;

    DBUG_ENTER ();

    DBUG_ASSERT (RC_IS_LEGAL (rc), "illegal RC value found!");

    if (RC_IS_ACTIVE (rc)) {
        if (RC_IS_VITAL (rc)) {
            assigns = TCmakeAssignIcm2 ("ND_SET__RC", TCmakeIdCopyStringNtNew (name, type),
                                        TBmakeNum (rc), assigns);
        } else {
            basetype = TUgetSimpleImplementationType (type);
            if (CUisDeviceTypeNew (type)) {
                assigns
                  = TCmakeAssignIcm2 ("CUDA_FREE", TCmakeIdCopyStringNtNew (name, type),
                                      TCmakeIdCopyString (GenericFun (GF_free, type)),
                                      assigns);
            } else if (basetype == T_int_dist || basetype == T_long_dist
                       || basetype == T_longlong_dist || basetype == T_float_dist
                       || basetype == T_double_dist) {
                assigns
                  = TCmakeAssignIcm2 ("DIST_FREE", TCmakeIdCopyStringNtNew (name, type),
                                      TCmakeIdCopyString (GenericFun (GF_free, type)),
                                      assigns);
            } else {
                assigns
                  = TCmakeAssignIcm2 ("ND_FREE", TCmakeIdCopyStringNtNew (name, type),
                                      TCmakeIdCopyString (GenericFun (GF_free, type)),
                                      assigns);
            }
        }
    }

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeIncRcIcm( char *name, ntype *type, int num,
 *                          node *assigns)
 *
 * @brief  Builds a ND_INC_RC( name, num) icm if needed.
 *
 ******************************************************************************/

static node *
MakeIncRcIcm (char *name, ntype *type, int num, node *assigns)
{
    DBUG_ENTER ();

    DBUG_ASSERT (num >= 0, "increment for rc must be >= 0.");

    if (num > 0) {
        assigns = TCmakeAssignIcm2 ("ND_INC_RC", TCmakeIdCopyStringNtNew (name, type),
                                    TBmakeNum (num), assigns);
    }

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeDecRcIcm( char *name, ntype *type, int num,
 *                          node *assigns)
 *
 * @brief  According to 'type', 'rc' and 'num', builds a
 *         xxxx_DEC_RC_FREE( name, num, freefun) icm,
 *         or no icm at all.
 *
 ******************************************************************************/

static node *
MakeDecRcIcm (char *name, ntype *type, int num, node *assigns)
{
    const char *icm;
    simpletype elem_type;

    DBUG_ENTER ();

    DBUG_ASSERT (num >= 0, "decrement for rc must be >= 0.");


    if (num > 0) {
        elem_type = TUgetSimpleImplementationType (type);
        if (elem_type == T_int_dist || elem_type == T_long_dist
            || elem_type == T_longlong_dist
            || elem_type == T_float_dist
            || elem_type == T_double_dist) {
            icm = "DIST_DEC_RC_FREE";
        } else if (CUisDeviceTypeNew (type)) {
            icm = "CUDA_DEC_RC_FREE";
        } else {
            icm = "ND_DEC_RC_FREE";
        }
        assigns
          = TCmakeAssignIcm3 (icm, TCmakeIdCopyStringNtNew (name, type), TBmakeNum (num),
                              TCmakeIdCopyString (GenericFun (GF_free, type)), assigns);
    }

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeAllocIcm( char *name, ntype *type, int rc,
 *                          node *get_dim, node *set_shape_icm,
 *                          node *pragma, node *assign)
 *
 * @brief  Builds a ND_ALLOC icm.
 *         The given node 'assigns' is appended to the created assignment.
 *
 ******************************************************************************/

static node *
MakeAllocIcm (char *name, ntype *type, int rc, node *get_dim, node *set_shape_icm,
              node *pragma, node *assigns)
{
    node *typeArg;
    simpletype baseType;

    DBUG_ENTER ();

    DBUG_ASSERT (RC_IS_LEGAL (rc), "illegal RC value found!");
    DBUG_ASSERT (get_dim != NULL, "no dimension found!");
    DBUG_ASSERT (((set_shape_icm != NULL) && (NODE_TYPE (set_shape_icm) == N_icm)),
                 "no N_icm node found!");

    if (RC_IS_ACTIVE (rc)) {
        if (pragma == NULL) {
            baseType = TUgetSimpleImplementationType (type);

            /* This is an array that should be allocated on the device */
            if (CUisDeviceTypeNew (type)) {
#if USE_COMPACT_ALLOC
                assigns
                  = TCmakeAssignIcm3 ("ND_ALLOC", TCmakeIdCopyStringNtNew (name, type),
                                      TBmakeNum (rc), get_dim, set_shape_icm, assigns);
#else
                typeArg = MakeBasetypeArg (type);
                assigns = TCmakeAssignIcm4 (
                  "CUDA_ALLOC_BEGIN", TCmakeIdCopyStringNtNew (name, type), TBmakeNum (rc),
                  get_dim, typeArg,
                  TBmakeAssign (set_shape_icm,
                                TCmakeAssignIcm4 ("CUDA_ALLOC_END",
                                                  TCmakeIdCopyStringNtNew (name, type),
                                                  TBmakeNum (rc), DUPdoDupTree (get_dim),
                                                  DUPdoDupNode (typeArg), assigns)));
#endif
            }
            /* This is a distributed array, we allocate a control structure for it */
            else if (baseType == T_int_dist || baseType == T_long_dist
                     || baseType == T_longlong_dist || baseType == T_float_dist
                     || baseType == T_double_dist) {
                switch (baseType) {
                case T_int_dist:
                    typeArg = TCmakeIdCopyString ("int");
                    break;
                case T_long_dist:
                    typeArg = TCmakeIdCopyString ("long");
                    break;
                case T_longlong_dist:
                    typeArg = TCmakeIdCopyString ("long long");
                    break;
                case T_float_dist:
                    typeArg = TCmakeIdCopyString ("float");
                    break;
                case T_double_dist:
                    typeArg = TCmakeIdCopyString ("double");
                    break;
                default:
                    typeArg = NULL;
                    break;
                }
                assigns
                  = TCmakeAssignIcm4 ("DIST_ALLOC", TCmakeIdCopyStringNtNew (name, type),
                                      TBmakeNum (rc), get_dim, typeArg, NULL);
                FREEdoFreeTree (set_shape_icm);
            } else {
#if USE_COMPACT_ALLOC
                assigns
                  = TCmakeAssignIcm3 ("ND_ALLOC", TCmakeIdCopyStringNtNew (name, type),
                                      TBmakeNum (rc), get_dim, set_shape_icm, assigns);
#else
                assigns = TCmakeAssignIcm4 (
                  "ND_ALLOC_BEGIN", TCmakeIdCopyStringNtNew (name, type), TBmakeNum (rc),
                  get_dim, MakeBasetypeArg (type),
                  TBmakeAssign (set_shape_icm,
                                TCmakeAssignIcm4 ("ND_ALLOC_END",
                                                  TCmakeIdCopyStringNtNew (name, type),
                                                  TBmakeNum (rc), DUPdoDupTree (get_dim),
                                                  MakeBasetypeArg (type), assigns)));
#endif
            }
        } else {
            /*
             * ALLOC_PLACE does not seem to be implemented somewhere
             */
            assigns
              = TCmakeAssignIcm5 ("ND_ALLOC_PLACE", TCmakeIdCopyStringNtNew (name, type),
                                  TBmakeNum (rc),
                                  DUPdoDupNode (AP_ARG1 (PRAGMA_APL (pragma))),
                                  DUPdoDupNode (AP_ARG2 (PRAGMA_APL (pragma))),
                                  DUPdoDupNode (AP_ARG3 (PRAGMA_APL (pragma))), assigns);
        }
    } else {
        get_dim = FREEdoFreeTree (get_dim);
        set_shape_icm = FREEdoFreeTree (set_shape_icm);
    }

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeAllocIcm_IncRc( char *name, ntype *type, int rc,
 *                                node *get_dim, node *set_shape_icm,
 *                                node *pragma, node *assigns)
 *
 * @brief  Builds a ND_ALLOC and a ND_INC_RC icm.
 *         The extra ND_INC_RC is needed, if there are any ND_CHECK_REUSE ICMs
 *         above ND_ALLOC!!
 *         The given node 'assigns' is appended to the created assignment.
 *
 ******************************************************************************/

static node *
MakeAllocIcm_IncRc (char *name, ntype *type, int rc, node *get_dim, node *set_shape_icm,
                    node *pragma, node *assigns)
{
    node *new_assigns;

    DBUG_ENTER ();

    DBUG_ASSERT (RC_IS_LEGAL (rc), "illegal RC value found!");

    new_assigns = MakeAllocIcm (name, type, 0, get_dim, set_shape_icm, pragma, NULL);

    if (new_assigns != NULL) {
        DBUG_ASSERT (RC_IS_VITAL (rc), "INC_RC(rc) with (rc <= 0) found!");
        assigns = TCappendAssign (new_assigns,
                                  TCmakeAssignIcm2 ("ND_INC_RC",
                                                    TCmakeIdCopyStringNtNew (name, type),
                                                    TBmakeNum (rc), assigns));
    }

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeCheckReuseIcm( char *name, ntype *type, node *reuse_id,
 *                               node *assigns);
 *
 * @brief  Builds a CHECK_REUSE icm which checks whether reuse_id can be
 *         reused at runtime.
 *         The given node 'assigns' is appended to the created assignment.
 *
 ******************************************************************************/

static node *
MakeCheckReuseIcm (char *name, ntype *type, node *reuse_id, node *assigns)
{
    DBUG_ENTER ();

    assigns
      = TCmakeAssignIcm2 ("ND_CHECK_REUSE",
                          MakeTypeArgs (name, type, FALSE, TRUE, FALSE,
                                        MakeTypeArgs (ID_NAME (reuse_id),
                                                      ID_NTYPE (reuse_id), FALSE, TRUE,
                                                      FALSE, NULL)),
                          TCmakeIdCopyString (GenericFun (GF_copy, ID_NTYPE (reuse_id))),
                          assigns);

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeReAllocIcm( char *name, ntype *type,
 *                            char *sname, ntype *stype, int rc,
 *                            node *get_dim, node *set_shape_icm,
 *                            node *pragma, node *assign)
 *
 * @brief  Builds a ND_REALLOC icm.
 *         The given node 'assigns' is appended to the created assignment.
 *
 ******************************************************************************/

static node *
MakeReAllocIcm (char *name, ntype *type, char *sname, ntype *stype, int rc, node *get_dim,
                node *set_shape_icm, node *pragma, node *assigns)
{
    DBUG_ENTER ();

    DBUG_ASSERT (RC_IS_LEGAL (rc), "illegal RC value found!");
    DBUG_ASSERT (get_dim != NULL, "no dimension found!");
    DBUG_ASSERT (((set_shape_icm != NULL) && (NODE_TYPE (set_shape_icm) == N_icm)),
                 "no N_icm node found!");
    DBUG_ASSERT (pragma == NULL, "realloc has no pragma support");

    if (RC_IS_ACTIVE (rc)) {
        /* This is an array that should be allocated on the device */
        if (CUisDeviceTypeNew (type)) {
#if USE_COMPACT_ALLOC
            assigns = TCmakeAssignIcm3 ("ND_ALLOC", TCmakeIdCopyStringNtNew (name, type),
                                        TBmakeNum (rc), get_dim, set_shape_icm, assigns);
#else
            assigns = TCmakeAssignIcm4 (
              "CUDA_ALLOC_BEGIN", TCmakeIdCopyStringNtNew (name, type), TBmakeNum (rc),
              get_dim, MakeBasetypeArg (type),
              TBmakeAssign (set_shape_icm,
                            TCmakeAssignIcm4 ("CUDA_ALLOC_END",
                                              TCmakeIdCopyStringNtNew (name, type),
                                              TBmakeNum (rc), DUPdoDupTree (get_dim),
                                              MakeBasetypeArg (type), assigns)));
#endif
        } else {
#if USE_COMPACT_ALLOC
            assigns = TCmakeAssignIcm3 ("ND_ALLOC", TCmakeIdCopyStringNtNew (name, type),
                                        TBmakeNum (rc), get_dim, set_shape_icm, assigns);
#else
            assigns = TCmakeAssignIcm5 (
              "ND_REALLOC_BEGIN", TCmakeIdCopyStringNtNew (name, type),
              TCmakeIdCopyStringNtNew (sname, stype), TBmakeNum (rc), get_dim,
              MakeBasetypeArg (type),
              TBmakeAssign (set_shape_icm,
                            TCmakeAssignIcm5 ("ND_REALLOC_END",
                                              TCmakeIdCopyStringNtNew (name, type),
                                              TCmakeIdCopyStringNtNew (sname, stype),
                                              TBmakeNum (rc), DUPdoDupTree (get_dim),
                                              MakeBasetypeArg (type), assigns)));
#endif
        }
    } else {
        get_dim = FREEdoFreeTree (get_dim);
        set_shape_icm = FREEdoFreeTree (set_shape_icm);
    }

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeCheckResizeIcm( char *name, ntype *type, node *reuse_id,
 *                                node *assigns);
 *
 * @brief  Builds a CHECK_RESIZE icm which checks whether reuse_id can be
 *         reused via a realloc at runtime.
 *         The given node 'assigns' is appended to the created assignment.
 *
 ******************************************************************************/

static node *
MakeCheckResizeIcm (char *name, ntype *type, node *reuse_id, int rc, node *get_dim,
                    node *set_shape_icm, node *assigns)
{
    DBUG_ENTER ();

    assigns
      = TCmakeAssignIcm1 ("SAC_IS_LASTREF__BLOCK_ELSE",
                          TCmakeIdCopyStringNtNew (ID_NAME (reuse_id), ID_NTYPE (reuse_id)),
                          assigns);

    assigns = MakeReAllocIcm (name, type, ID_NAME (reuse_id), ID_NTYPE (reuse_id), rc,
                              get_dim, set_shape_icm, NULL, assigns);

    assigns
      = TCmakeAssignIcm1 ("SAC_IS_LASTREF__BLOCK_BEGIN",
                          TCmakeIdCopyStringNtNew (ID_NAME (reuse_id), ID_NTYPE (reuse_id)),
                          assigns);

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn  node MakeGetDimIcm( node *arg_node)
 *
 * @brief  Creates an ICM for calculating the dimension of an allocated
 *         array.
 *
 * Remarks:
 *   arg_node must have the special syntax used by F_alloc/F_alloc_or_reuse
 *
 ******************************************************************************/
static node *
MakeGetDimIcm (node *arg_node)
{
    node *get_dim = NULL;

    DBUG_ENTER ();

    switch (NODE_TYPE (arg_node)) {
    case N_num:
        get_dim = DUPdoDupTree (arg_node);
        break;

    case N_id:
        get_dim = TCmakeIcm1 ("ND_A_DIM", DUPdupIdNt (arg_node));
        break;

    case N_prf:
        switch (PRF_PRF (arg_node)) {
        case F_dim_A:
            get_dim = MakeDimArg (PRF_ARG1 (arg_node), FALSE);
            break;

        case F_add_SxS:
        case F_sub_SxS:
            get_dim = TCmakeIcm2 (prf_ccode_tab[PRF_PRF (arg_node)],
                                  MakeGetDimIcm (PRF_ARG1 (arg_node)),
                                  MakeGetDimIcm (PRF_ARG2 (arg_node)));
            break;
        case F_sel_VxA:
            DBUG_ASSERT ((NODE_TYPE (PRF_ARG1 (arg_node)) == N_num)
                           && (NUM_VAL (PRF_ARG1 (arg_node)) == 0)
                           && (NODE_TYPE (PRF_ARG2 (arg_node)) == N_prf)
                           && (PRF_PRF (PRF_ARG2 (arg_node)) == F_shape_A),
                         "Invalid MakeSizeArg descriptor found!");
            get_dim = MakeSizeArg (PRF_ARG1 (PRF_ARG2 (arg_node)), FALSE);
            break;

        default:
            DBUG_UNREACHABLE ("Unrecognized dim descriptor");
            break;
        }
        break;
    default:
        DBUG_UNREACHABLE ("Unrecognized dim descriptor");
        break;
    }

    DBUG_RETURN (get_dim);
}

/** <!--********************************************************************-->
 *
 * @fn  node MakeSetShapeIcm( node *arg_node, ids *let_ids)
 *
 * @brief  Creates an ICM for calculating the shape of an allocated
 *         array.
 *
 * Remarks:
 *   arg_node must have the special syntax used by F_alloc/F_alloc_or_reuse
 *
 ******************************************************************************/
static node *
MakeSetShapeIcm (node *arg_node, node *let_ids)
{
    node *arg1, *arg2;
    node *set_shape = NULL;

    DBUG_ENTER ();

    switch (NODE_TYPE (arg_node)) {
    case N_array:
        /*
         * [ a, ...]
         * => ND_SET__SHAPE_arr
         */
        set_shape
          = TCmakeIcm2 ("ND_SET__SHAPE_arr", DUPdupIdsIdNt (let_ids),
                        TBmakeExprs (TBmakeNum (TCcountExprs (ARRAY_AELEMS (arg_node))),
                                     DupExprs_NT_AddReadIcms (ARRAY_AELEMS (arg_node))));
        break;

    case N_prf:
        switch (PRF_PRF (arg_node)) {
        case F_shape_A:
            arg_node = PRF_ARG1 (arg_node);
            switch (NODE_TYPE (arg_node)) {
            case N_id:
                /*
                 * shape( a)
                 * => ND_COPY__SHAPE
                 */
                set_shape
                  = TCmakeIcm1 ("ND_COPY__SHAPE",
                                MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                              FALSE, TRUE, FALSE,
                                              MakeTypeArgs (ID_NAME (arg_node),
                                                            ID_NTYPE (arg_node), FALSE,
                                                            TRUE, FALSE, NULL)));
                break;

            case N_array:
                /*
                 * shape( [ a, ...])
                 * => ND_CREATE__ARRAY__SHAPE
                 */
                {
                    int i, dim, val0_sdim;
                    shape *shp;
                    node *icm_args, *icm_args2;

                    shp = ARRAY_FRAMESHAPE (arg_node);
                    dim = SHgetDim (shp);

                    icm_args
                      = TBmakeExprs (MakeSizeArg (arg_node, TRUE),
                                     DupExprs_NT_AddReadIcms (ARRAY_AELEMS (arg_node)));

                    icm_args2 = NULL;
                    for (i = dim - 1; i >= 0; i--) {
                        icm_args2
                          = TBmakeExprs (TBmakeNum (SHgetExtent (shp, i)), icm_args2);
                    }
                    icm_args2 = TBmakeExprs (TBmakeNum (dim), icm_args2);

                    if (ARRAY_AELEMS (arg_node) != NULL) {
                        if (NODE_TYPE (EXPRS_EXPR (ARRAY_AELEMS (arg_node))) == N_id) {
                            val0_sdim = TUgetFullDimEncoding (
                              ID_NTYPE (EXPRS_EXPR (ARRAY_AELEMS (arg_node))));
                        } else {
                            val0_sdim = 0;
                        }
                    } else {
                        val0_sdim = -815; /* array is empty */
                    }

                    set_shape
                      = TCmakeIcm3 ("ND_CREATE__ARRAY__SHAPE",
                                    MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                                  FALSE, TRUE, FALSE, icm_args2),
                                    icm_args, TBmakeNum (val0_sdim));
                }
                break;

            case N_prf:

                switch (PRF_PRF (arg_node)) {
                case F_cat_VxV:
                    arg1 = PRF_ARG1 (arg_node);
                    arg2 = PRF_ARG2 (arg_node);
                    /*
                     * shape( cat( a, b))
                     * => ND_PRF_CAT_VxV__SHAPE
                     */
                    {
                        node *icm_args;

                        DBUG_ASSERT (NODE_TYPE (arg1) == N_id,
                                     "1st arg of F_cat_VxV is no N_id!");
                        DBUG_ASSERT (NODE_TYPE (arg2) == N_id,
                                     "2nd arg of F_cat_VxV is no N_id!");

                        icm_args
                          = MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE,
                                          TRUE, FALSE,
                                          MakeTypeArgs (ID_NAME (arg1), ID_NTYPE (arg1),
                                                        FALSE, TRUE, FALSE,
                                                        MakeTypeArgs (ID_NAME (arg2),
                                                                      ID_NTYPE (arg2),
                                                                      FALSE, TRUE, FALSE,
                                                                      NULL)));

                        set_shape = TCmakeIcm1 ("ND_PRF_CAT_VxV__SHAPE", icm_args);
                    }
                    break;

                case F_drop_SxV:
                    arg1 = PRF_ARG1 (arg_node);
                    arg2 = PRF_ARG2 (arg_node);
                    /*
                     * shape( drop( a, b))
                     * => ND_PRF_DROP_SxV__SHAPE
                     */
                    {
                        node *icm_args;

                        DBUG_ASSERT (((NODE_TYPE (arg1) == N_id)
                                      || (NODE_TYPE (arg1) == N_num)),
                                     "1st arg of F_drop_SxV is neither N_id nor N_num!");
                        DBUG_ASSERT (NODE_TYPE (arg2) == N_id,
                                     "2nd arg of F_drop_SxV is no N_id!");

                        icm_args
                          = MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE,
                                          TRUE, FALSE,
                                          MakeTypeArgs (ID_NAME (arg2), ID_NTYPE (arg2),
                                                        FALSE, TRUE, FALSE,
                                                        TBmakeExprs (DUPdupNodeNt (arg1),
                                                                     NULL)));

                        set_shape = TCmakeIcm1 ("ND_PRF_DROP_SxV__SHAPE", icm_args);
                    }
                    break;

                case F_take_SxV:
                    arg1 = PRF_ARG1 (arg_node);
                    arg2 = PRF_ARG2 (arg_node);
                    /*
                     * shape( take( a, b))
                     * => ND_PRF_TAKE_SxV__SHAPE
                     */
                    {
                        node *icm_args;

                        DBUG_ASSERT (((NODE_TYPE (arg1) == N_id)
                                      || (NODE_TYPE (arg1) == N_num)),
                                     "1st arg of F_take_SxV is neither N_id nor N_num!");
                        DBUG_ASSERT (NODE_TYPE (arg2) == N_id,
                                     "2nd arg of F_take_SxV is no N_id!");

                        icm_args
                          = MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE,
                                          TRUE, FALSE,
                                          MakeTypeArgs (ID_NAME (arg2), ID_NTYPE (arg2),
                                                        FALSE, TRUE, FALSE,
                                                        TBmakeExprs (DUPdupNodeNt (arg1),
                                                                     NULL)));

                        set_shape = TCmakeIcm1 ("ND_PRF_TAKE_SxV__SHAPE", icm_args);
                    }
                    break;

                case F_sel_VxA:
                    arg1 = PRF_ARG1 (arg_node);
                    arg2 = PRF_ARG2 (arg_node);
                    switch (NODE_TYPE (arg1)) {
                    case N_array:
                        /*
                         * shape( sel( [ 1, ...], b))
                         * => ND_PRF_SEL_VxA__SHAPE_arr
                         */
                        {
                            node *icm_args;

                            icm_args = MakeTypeArgs (
                              IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE, TRUE, FALSE,
                              MakeTypeArgs (
                                ID_NAME (arg2), ID_NTYPE (arg2), FALSE, TRUE, FALSE,
                                TBmakeExprs (MakeSizeArg (arg1, TRUE),
                                             TCappendExprs (DupExprs_NT_AddReadIcms (
                                                              ARRAY_AELEMS (arg1)),
                                                            NULL))));

                            set_shape
                              = TCmakeIcm1 ("ND_PRF_SEL_VxA__SHAPE_arr", icm_args);
                        }
                        break;

                    case N_id:
                        /*
                         * shape( sel( id, b))
                         * => ND_PRF_SEL_VxA__SHAPE_id
                         */
                        {
                            node *icm_args;

                            DBUG_ASSERT ((TUgetSimpleImplementationType (ID_NTYPE (arg1)) == T_int),
                                         "1st arg of F_sel_VxA is a illegal indexing "
                                         "var!");

                            icm_args
                              = MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                              FALSE, TRUE, FALSE,
                                              MakeTypeArgs (ID_NAME (arg2),
                                                            ID_NTYPE (arg2), FALSE, TRUE,
                                                            FALSE,
                                                            TBmakeExprs (DUPdupIdNt (
                                                                           arg1),
                                                                         NULL)));

                            set_shape = TCmakeIcm1 ("ND_PRF_SEL_VxA__SHAPE_id", icm_args);
                        }
                        break;

                    default:
                        DBUG_UNREACHABLE ("Unrecognized shape descriptor");
                        break;
                    }
                    break;

                case F_idx_sel:
                    arg1 = PRF_ARG1 (arg_node);
                    arg2 = PRF_ARG2 (arg_node);
                    /*
                     * shape( idx_sel( a, b))
                     * => ND_PRF_IDX_SEL_SHAPE
                     */
                    {
                        node *icm_args;

                        DBUG_ASSERT (((NODE_TYPE (arg1) == N_id)
                                      || (NODE_TYPE (arg1) == N_num)
                                      || ((NODE_TYPE (arg1) == N_prf))),
                                     "1st arg of F_idx_sel is neither N_id nor N_num, "
                                     "N_prf!");
                        DBUG_ASSERT (NODE_TYPE (arg2) == N_id,
                                     "2nd arg of F_idx_sel is no N_id!");

                        icm_args
                          = MakeTypeArgs (ID_NAME (arg2), ID_NTYPE (arg2), FALSE, TRUE,
                                          FALSE, TBmakeExprs (DUPdupNodeNt (arg1), NULL));

                        set_shape = TCmakeIcm1 ("ND_PRF_IDX_SEL__SHAPE",
                                                MakeTypeArgs (IDS_NAME (let_ids),
                                                              IDS_NTYPE (let_ids), FALSE,
                                                              TRUE, FALSE, icm_args));
                    }
                    break;

                case F_reshape_VxA:
                    arg1 = PRF_ARG1 (arg_node);
                    arg2 = PRF_ARG2 (arg_node);
                    switch (NODE_TYPE (arg1)) {
                    case N_array:
                        /*
                         * shape( reshape( [ 1, ...], b))
                         * => ND_RESHAPE_SHAPE_arr
                         */
                        set_shape
                          = TCmakeIcm1 ("ND_PRF_RESHAPE_VxA__SHAPE_arr",
                                        MakeTypeArgs (IDS_NAME (let_ids),
                                                      IDS_NTYPE (let_ids), FALSE, TRUE,
                                                      FALSE,
                                                      TBmakeExprs (MakeSizeArg (arg1,
                                                                                TRUE),
                                                                   DupExprs_NT_AddReadIcms (
                                                                     ARRAY_AELEMS (
                                                                       arg1)))));
                        break;

                    case N_id:
                        /*
                         * shape( sel( id, b))
                         * => ND_RESHAPE_SHAPE_id
                         */
                        set_shape
                          = TCmakeIcm1 ("ND_PRF_RESHAPE_VxA__SHAPE_id",
                                        MakeTypeArgs (IDS_NAME (let_ids),
                                                      IDS_NTYPE (let_ids), FALSE, TRUE,
                                                      FALSE,
                                                      TBmakeExprs (DUPdupIdNt (arg1),
                                                                   NULL)));
                        break;

                    default:
                        DBUG_UNREACHABLE ("Unrecognized shape descriptor");
                        break;
                    }
                    break;

                case F_genarray:
                    arg1 = PRF_ARG1 (arg_node);
                    arg2 = PRF_ARG2 (arg_node);
                    switch (NODE_TYPE (arg1)) {
                    case N_id:
                        /*
                         * shape( genarray( a, b))
                         * => ND_WL_GENARRAY__SHAPE_id_id
                         */
                        set_shape
                          = TCmakeIcm1 ("ND_WL_GENARRAY__SHAPE_id_id",
                                        MakeTypeArgs (IDS_NAME (let_ids),
                                                      IDS_NTYPE (let_ids), FALSE, TRUE,
                                                      FALSE,
                                                      TBmakeExprs (DUPdupIdNt (arg1),
                                                                   MakeTypeArgs (ID_NAME (
                                                                                   arg2),
                                                                                 ID_NTYPE (
                                                                                   arg2),
                                                                                 FALSE,
                                                                                 TRUE,
                                                                                 FALSE,
                                                                                 NULL))));
                        break;
                    case N_array:
                        /*
                         * shape( genarray( [...], b))
                         * => ND_WL_GENARRAY__SHAPE_arr_id
                         */
                        set_shape
                          = TCmakeIcm4 ("ND_WL_GENARRAY__SHAPE_arr_id",
                                        MakeTypeArgs (IDS_NAME (let_ids),
                                                      IDS_NTYPE (let_ids), FALSE, TRUE,
                                                      FALSE, NULL),
                                        MakeSizeArg (arg1, TRUE),
                                        DupExprs_NT_AddReadIcms (ARRAY_AELEMS (arg1)),
                                        MakeTypeArgs (ID_NAME (arg2), ID_NTYPE (arg2),
                                                      FALSE, TRUE, FALSE, NULL));
                        break;
                    default:
                        DBUG_UNREACHABLE ("Unrecognized shape descriptor!");
                        break;
                    }

                    break;
                case F_shape_A:
                    arg1 = PRF_ARG1 (arg_node);
                    /*
                     * shape( shape( a))
                     */
                    set_shape = TCmakeIcm3 ("ND_SET__SHAPE_arr", DUPdupIdsIdNt (let_ids),
                                            TBmakeNum (1), MakeDimArg (arg1, FALSE));
                    break;

                default:
                    DBUG_UNREACHABLE ("Unrecognized shape descriptor");
                    break;
                }
                break;

            default:
                DBUG_UNREACHABLE ("Unrecognized shape descriptor");
                break;
            }
            break;

        case F_cat_VxV:
            /*
             * cat( a, [...])
             */
            arg1 = PRF_ARG1 (arg_node);
            arg2 = PRF_ARG2 (arg_node);

            DBUG_ASSERT (NODE_TYPE (arg2) == N_array, "Illegal node type!");

            switch (NODE_TYPE (arg1)) {
            case N_id:
                /*
                 * cat( a, [...])
                 *
                 * This shape descriptor is used if a with-loop's outer shape is
                 * unknown but its element shape is known.
                 *
                 * !!! This has not yet been tested !!!
                 */
                set_shape
                  = TCmakeIcm4 ("ND_WL_GENARRAY__SHAPE_id_arr",
                                MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                              FALSE, TRUE, FALSE, NULL),
                                DUPdupIdNt (arg1), MakeSizeArg (arg2, TRUE),
                                DupExprs_NT_AddReadIcms (ARRAY_AELEMS (arg2)));
                break;

            case N_array:
                /*
                 * cat( [...], [...])
                 */
                set_shape
                  = TCmakeIcm2 ("ND_SET__SHAPE_arr", DUPdupIdsIdNt (let_ids),
                                TBmakeExprs (TBmakeNum (
                                               TCcountExprs (ARRAY_AELEMS (arg1))
                                               + TCcountExprs (ARRAY_AELEMS (arg2))),
                                             TCappendExprs (DupExprs_NT_AddReadIcms (
                                                              ARRAY_AELEMS (arg1)),
                                                            DupExprs_NT_AddReadIcms (
                                                              ARRAY_AELEMS (arg2)))));
                break;

            default:
                DBUG_UNREACHABLE ("Unrecognized shape descriptor");
                break;
            }
            break;

        default:
            DBUG_UNREACHABLE ("Unrecognized shape descriptor");
            break;
        }
        break;

    default:
        DBUG_UNREACHABLE ("Unrecognized shape descriptor");
        break;
    }

    DBUG_RETURN (set_shape);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeArgNode( int idx, ntype *type, bool thread)
 *
 * @brief  ...
 *         bool thread is this arg a mutc thread fun arg?
 *
 ******************************************************************************/

static node *
MakeArgNode (size_t idx, ntype *arg_type, bool thread)
{
    node *id;
    char *name;
    ntype *type;

    DBUG_ENTER ();

    type = TYcopyType (arg_type);

    /* Set usage tag of arg */
    if (thread) {
        type = TYsetMutcUsage (type, MUTC_US_THREADPARAM);
    } else {
        type = TYsetMutcUsage (type, MUTC_US_FUNPARAM);
    }

    name = (char *)MEMmalloc (20 * sizeof (char));
    sprintf (name, "SAC_arg_%zu", idx);

    if (type != NULL) {
        id = TCmakeIdCopyStringNtNew (name, type);
    } else {
        id = TCmakeIdCopyString (name);
    }

    name = MEMfree (name);

    type = TYfreeType (type);

    DBUG_RETURN (id);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeFunctionArgsSpmd( node *fundef)
 *
 * @brief  Creates ICM argument list for SPMDFUN ICMs
 *
 ******************************************************************************/

static node *
MakeFunctionArgsSpmd (node *fundef)
{
    argtab_t *argtab;
    size_t size;
    size_t i;
    node *icm_args = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no fundef node found!");

    argtab = FUNDEF_ARGTAB (fundef);
    DBUG_ASSERT (argtab != NULL, "no argtab found!");
    DBUG_ASSERT (argtab->ptr_in[0] == NULL, "argtab inconsistent!");

    /* arguments */
    for (i = argtab->size - 1; i >= 1; i--) {
        char *name;
        node *id;
        ntype *type;

        if (argtab->ptr_in[i] != NULL) {
            DBUG_ASSERT (NODE_TYPE (argtab->ptr_in[i]) == N_arg,
                         "no N_arg node found in argtab");

            name = ARG_NAME (argtab->ptr_in[i]);
            type = ARG_NTYPE (argtab->ptr_in[i]);
            id = TCmakeIdCopyStringNtNew (STRonNull ("", name), type);
        } else {
            DBUG_ASSERT (argtab->ptr_out[i] != NULL, "argtab is uncompressed!");
            type = RET_TYPE (argtab->ptr_out[i]);
            id = MakeArgNode (i, type, FALSE);
        }

        icm_args = TBmakeExprs (TCmakeIdCopyString (global.argtag_string[argtab->tag[i]]),
                                TBmakeExprs (MakeBasetypeArg (type),
                                             TBmakeExprs (id, icm_args)));
    }
    size = argtab->size - 1;

    /* return value */
    DBUG_ASSERT (argtab->ptr_in[0] == NULL, "argtab is inconsistent!");
    if (argtab->ptr_out[0] != NULL) {
        ntype *type;
        type = RET_TYPE (argtab->ptr_out[0]);
        icm_args = TBmakeExprs (TCmakeIdCopyString (global.argtag_string[argtab->tag[0]]),
                                TBmakeExprs (MakeBasetypeArg (type),
                                             TBmakeExprs (MakeArgNode (0, type, FALSE),
                                                          icm_args)));
        size++;
    }

    icm_args = TBmakeExprs (TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                            TBmakeExprs (TBmakeNumuint (size), icm_args));

    DBUG_RETURN (icm_args);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeFunctionArgsCuda( node *fundef)
 *
 * @brief  Creates ICM argument list for CUDA GLOBALFUN ICMs
 *
 ******************************************************************************/

static node *
MakeFunctionArgsCuda (node *fundef)
{
    argtab_t *argtab;
    size_t size;
    size_t i;
    node *icm_args = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no fundef node found!");

    argtab = FUNDEF_ARGTAB (fundef);
    DBUG_ASSERT (argtab != NULL, "no argtab found!");
    DBUG_ASSERT (argtab->ptr_in[0] == NULL, "argtab inconsistent!");

    /* arguments */
    for (i = argtab->size - 1; i >= 1; i--) {
        char *name;
        node *id;
        ntype *type;

        if (argtab->ptr_in[i] != NULL) {
            DBUG_ASSERT (NODE_TYPE (argtab->ptr_in[i]) == N_arg,
                         "no N_arg node found in argtab");

            name = ARG_NAME (argtab->ptr_in[i]);
            type = ARG_NTYPE (argtab->ptr_in[i]);
            id = TCmakeIdCopyStringNtNew (STRonNull ("", name), type);
        } else {
            DBUG_ASSERT (argtab->ptr_out[i] != NULL, "argtab is uncompressed!");
            type = RET_TYPE (argtab->ptr_out[i]);
            id = MakeArgNode (i, type, FALSE);
        }

        icm_args
          = TBmakeExprs (TCmakeIdCopyString (global.argtag_string[argtab->tag[i]]),
                         TBmakeExprs (MakeBasetypeArg (type),
                                      TBmakeExprs (id, TBmakeExprs (TBmakeNum (
                                                                      TUgetFullDimEncoding (type)),
                                                                    icm_args))));
    }
    size = argtab->size - 1;

    /* return value */
    DBUG_ASSERT (argtab->ptr_in[0] == NULL, "argtab is inconsistent!");
    if (argtab->ptr_out[0] != NULL) {
        ntype *type;
        type = RET_TYPE (argtab->ptr_out[0]);
        icm_args
          = TBmakeExprs (TCmakeIdCopyString (global.argtag_string[argtab->tag[0]]),
                         TBmakeExprs (MakeBasetypeArg (type),
                                      TBmakeExprs (MakeArgNode (0, type, FALSE),
                                                   TBmakeExprs (TBmakeNum (
                                                                  TUgetFullDimEncoding (type)),
                                                                icm_args))));
        size++;
    }

    icm_args = TBmakeExprs (TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                            TBmakeExprs (TBmakeNumuint (size), icm_args));

    DBUG_RETURN (icm_args);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeFunctionArgs( node *fundef)
 *
 * @brief  Create the arguments to a function
 *         By this stage all return values are pointer arguments.
 *
 *****************************************************************************/

static node *
MakeFunctionArgs (node *fundef)
{
    node *icm_args = NULL;
    argtab_t *argtab;
    size_t i;

    DBUG_ENTER ();

    argtab = FUNDEF_ARGTAB (fundef);
    DBUG_ASSERT (argtab != NULL, "no argtab found!");

    if (FUNDEF_HASDOTARGS (fundef) || FUNDEF_HASDOTRETS (fundef)) {
        /*
         * for ... arguments the name should expand to an empty string
         *  -> replace 'tag' and 'id'
         */

        icm_args
          = TBmakeExprs (TCmakeIdCopyString (global.argtag_string[ATG_notag]),
                         TBmakeExprs (TCmakeIdCopyString ("..."),
                                      TBmakeExprs (TCmakeIdCopyString (NULL), icm_args)));
    }

    /* arguments */
    for (i = argtab->size - 1; i >= 1; i--) {
        argtag_t tag;
        ntype *type;
        char *name;
        node *id;

        if (argtab->ptr_in[i] != NULL) {
            DBUG_ASSERT (NODE_TYPE (argtab->ptr_in[i]) == N_arg,
                         "no N_arg node found in argtab");

            tag = argtab->tag[i];
            type = ARG_NTYPE (argtab->ptr_in[i]);
            name = ARG_NAME (argtab->ptr_in[i]);
            if (name != NULL) {
                id = TCmakeIdCopyStringNtNew (name, type);
            } else {
                id = MakeArgNode (i, type, FUNDEF_ISTHREADFUN (fundef));
            }
        } else {
            DBUG_ASSERT (argtab->ptr_out[i] != NULL, "argtab is uncompressed!");
            tag = argtab->tag[i];
            type = RET_TYPE (argtab->ptr_out[i]);
            id = MakeArgNode (i, type, FUNDEF_ISTHREADFUN (fundef));
        }

        icm_args = TBmakeExprs (TCmakeIdCopyString (global.argtag_string[tag]),
                                TBmakeExprs (MakeBasetypeArg (type),
                                             TBmakeExprs (id, icm_args)));
    }

    if (FUNDEF_HASDOTARGS (fundef) || FUNDEF_HASDOTRETS (fundef)) {
        icm_args = TBmakeExprs (TBmakeNumuint (argtab->size), icm_args);
    } else {
        icm_args = TBmakeExprs (TBmakeNumuint (argtab->size - 1), icm_args);
    }

    /* return value */
    DBUG_ASSERT (argtab->ptr_in[0] == NULL, "argtab inconsistent!");
    if (argtab->ptr_out[0] == NULL) {
        icm_args = TBmakeExprs (TCmakeIdCopyString (NULL), icm_args);
    } else {
        icm_args = TBmakeExprs (MakeBasetypeArg_NT (RET_TYPE (argtab->ptr_out[0])),
                                icm_args);
    }

    icm_args = TBmakeExprs (TCmakeIdCopyString (FUNDEF_NAME (fundef)), icm_args);

    DBUG_RETURN (icm_args);
}

/** <!--********************************************************************-->
 *
 * @fn  char *GetBaseTypeFromAvis( node *in)
 *
 * @brief  Get the base type of the item pointed to by avis node "in"
 *
 *****************************************************************************/
static char *
GetBaseTypeFromAvis (node *in)
{
    ntype *type = NULL;

    DBUG_ENTER ();
    DBUG_ASSERT (in != NULL, "no node found!");

    in = AVIS_DECL (in);

    if (NODE_TYPE (in) == N_vardec) {
        type = VARDEC_NTYPE (in);
    } else if (NODE_TYPE (in) == N_arg) {
        type = ARG_NTYPE (in);
    } else {
        DBUG_UNREACHABLE ("Illegal node type found!");
    }

    DBUG_RETURN (STRcpy (GetBasetypeStr (type)));
}

/** <!--********************************************************************-->
 *
 * @fn  char *GetBaseTypeFromExpr( node *in)
 *
 * @brief  Get the base type of the item pointed to by expr node "in"
 *
 *****************************************************************************/

static const char *
GetBaseTypeFromExpr (node *in)
{
    const char *ret = NULL;
    DBUG_ENTER ();
    DBUG_ASSERT (in != NULL, "no node found!");

    if (NODE_TYPE (in) == N_exprs) {
        in = EXPRS_EXPR (in);
    }

    if (NODE_TYPE (in) == N_id) {
        in = ID_AVIS (in);
        ret = GetBaseTypeFromAvis (in);
    } else if (NODE_TYPE (in) == N_ids) {
        in = IDS_AVIS (in);
        ret = GetBaseTypeFromAvis (in);
    } else if (NODE_TYPE (in) == N_globobj) {
        in = GLOBOBJ_OBJDEF (in);
        ret = GetBasetypeStr (OBJDEF_TYPE (in));
    } else {
        DBUG_UNREACHABLE ("Unexpected node type found!");
    }

    DBUG_RETURN (ret);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeFunApArgIdsNt( node *ids)
 *
 * @brief  Create a function application argument
 *         This function create a new id node and does not consume the ids node
 *
 ****************************************************************************/
static node *
MakeFunApArgIdsNt (node *ids)
{
    node *icm, *id = NULL;
    DBUG_ENTER ();

    if (TYgetMutcUsage (IDS_NTYPE (ids)) == MUTC_US_FUNPARAM) {
        id = TCmakeIdCopyString ("FPA");
    } else if (TYgetMutcUsage (IDS_NTYPE (ids)) == MUTC_US_THREADPARAM) {
        id = TCmakeIdCopyString ("FTA");
    } else {
        id = TCmakeIdCopyString ("FAG");
    }

    icm = TCmakeIcm2 ("SET_NT_USG", id, DUPdupIdsIdNt (ids));

    DBUG_RETURN (icm);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeFunApArgIdsNtThread( node *ids)
 *
 * @brief  Create a MUTC thread function application argument
 *         This function create a new id node and does not consume the ids
 *         node
 *
 ****************************************************************************/
static node *
MakeFunApArgIdsNtThread (node *ids)
{
    node *icm, *id = NULL;
    DBUG_ENTER ();

    if (TYgetMutcUsage (IDS_NTYPE (ids)) == MUTC_US_THREADPARAM) {
        id = TCmakeIdCopyString ("TPA");
    } else {
        id = TCmakeIdCopyString ("TAG");
    }

    icm = TCmakeIcm2 ("SET_NT_USG", id, DUPdupIdsIdNt (ids));

    DBUG_RETURN (icm);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeFunApArgIdNt( node *id)
 *
 * @brief  Create a function application argument
 *         This function create a new id node and does not consume the id node
 *
 ****************************************************************************/
static node *
MakeFunApArgIdNt (node *id)
{
    node *icm, *st = NULL;
    DBUG_ENTER ();

    if (TYgetMutcUsage (ID_NTYPE (id)) == MUTC_US_FUNPARAM) {
        st = TCmakeIdCopyString ("FPA");
    } else if (TYgetMutcUsage (ID_NTYPE (id)) == MUTC_US_THREADPARAM) {
        st = TCmakeIdCopyString ("FTA");
    } else {
        st = TCmakeIdCopyString ("FAG");
    }

    icm = TCmakeIcm2 ("SET_NT_USG", st, DUPdupIdNt (id));

    DBUG_RETURN (icm);
}

/** <!--*******************************************************************-->
 *
 * @fn  node *MakeFunApArgIdNtThread( node *id)
 *
 * @brief  Create a MUTC thread function application argument
 *         This function create a new id node and does not consume the id node
 *
 ****************************************************************************/
static node *
MakeFunApArgIdNtThread (node *id)
{
    node *icm, *st = NULL;
    DBUG_ENTER ();

    if (TYgetMutcUsage (ID_NTYPE (id)) == MUTC_US_THREADPARAM) {
        st = TCmakeIdCopyString ("TPA");
    } else {
        st = TCmakeIdCopyString ("TAG");
    }

    icm = TCmakeIcm2 ("SET_NT_USG", st, DUPdupIdNt (id));

    DBUG_RETURN (icm);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeFunApArgs( node *ap, info *arg_info)
 *
 * @brief  Builds N_exprs chain of ICM arguments for function applications
 *
 ******************************************************************************/

static node *
MakeFunApArgs (node *ap, info *arg_info)
{
    argtab_t *argtab;
    size_t i;
    node *icm_args = NULL;
    node *fundef;
    bool fundef_in_current_namespace;

    DBUG_ENTER ();

    DBUG_ASSERT (((ap != NULL) && (NODE_TYPE (ap) == N_ap)), "no ap node found!");

    fundef = AP_FUNDEF (ap);

    argtab = AP_ARGTAB (ap);
    DBUG_ASSERT (argtab != NULL, "no argtab found!");

    fundef_in_current_namespace
      = STReq (NSgetModule (FUNDEF_NS (fundef)),
               NSgetModule (MODULE_NAMESPACE (INFO_MODUL (arg_info))));

    /* arguments */
    for (i = argtab->size - 1; i >= 1; i--) {
        node *exprs = NULL;
        bool shared = FALSE; /* MUTC shared parameter? */
        shape_class_t shape;
        int dim = 0;

        if (argtab->ptr_out[i] != NULL) {
            if (!FUNDEF_ISTHREADFUN (fundef)) {
                exprs = TBmakeExprs (MakeFunApArgIdsNt (argtab->ptr_out[i]), icm_args);
            } else {
                exprs
                  = TBmakeExprs (MakeFunApArgIdsNtThread (argtab->ptr_out[i]), icm_args);
            }

            if (FUNDEF_RTSPECID (fundef) != NULL && global.config.rtspec
                && ((fundef_in_current_namespace && FUNDEF_ISEXPORTED (fundef))
                    || !fundef_in_current_namespace)) {
                shape = NTUgetShapeClassFromNType (IDS_NTYPE (argtab->ptr_out[i]));
                dim = TUgetDimEncoding (IDS_NTYPE (argtab->ptr_out[i]));
                exprs = TBmakeExprs (TBmakeNum (shape), exprs);
                exprs = TBmakeExprs (TBmakeNum (dim), exprs);
            }

            if (!FUNDEF_ISCUDAGLOBALFUN (fundef) && !FUNDEF_ISCUDASTGLOBALFUN (fundef)) {
                exprs = TBmakeExprs (TCmakeIdCopyString (
                                       GetBaseTypeFromExpr (argtab->ptr_out[i])),
                                     exprs);
            } else {
                exprs = TBmakeExprs (TCmakeIdCopyString (
                                       GetBaseTypeFromExpr (argtab->ptr_out[i])),
                                     TBmakeExprs (TBmakeNum (TUgetFullDimEncoding (
                                                    IDS_NTYPE (argtab->ptr_out[i]))),
                                                  exprs));
            }

        } else {
            DBUG_ASSERT (argtab->ptr_in[i] != NULL, "argtab is uncompressed!");
            DBUG_ASSERT (NODE_TYPE (argtab->ptr_in[i]) == N_exprs,
                         "no N_exprs node found in argtab");
            if (NODE_TYPE (EXPRS_EXPR (argtab->ptr_in[i])) == N_id) {
                if (!FUNDEF_ISTHREADFUN (fundef)) {
                    exprs
                      = TBmakeExprs (MakeFunApArgIdNt (EXPRS_EXPR (argtab->ptr_in[i])),
                                     icm_args);
                } else {
                    if (TYgetMutcScope (ARG_NTYPE (FUNDEF_ARGTAB (fundef)->ptr_in[i]))
                        == MUTC_SHARED) {
                        shared = TRUE;
                        exprs
                          = TBmakeExprs (TCmakeIcm2 ("SET_NT_SCO",
                                                     TCmakeIdCopyString ("SHA"),
                                                     MakeFunApArgIdNtThread (
                                                       EXPRS_EXPR (argtab->ptr_in[i]))),
                                         icm_args);
                    } else {
                        exprs = TBmakeExprs (MakeFunApArgIdNtThread (
                                               EXPRS_EXPR (argtab->ptr_in[i])),
                                             icm_args);
                    }
                }

                if (FUNDEF_RTSPECID (fundef) != NULL && global.config.rtspec
                    && ((fundef_in_current_namespace && FUNDEF_ISEXPORTED (fundef))
                        || !fundef_in_current_namespace)) {
                    shape = NTUgetShapeClassFromNType (
                      ID_NTYPE (EXPRS_EXPR (argtab->ptr_in[i])));
                    dim = TUgetDimEncoding (ID_NTYPE (EXPRS_EXPR (argtab->ptr_in[i])));
                    exprs = TBmakeExprs (TBmakeNum (shape), exprs);
                    exprs = TBmakeExprs (TBmakeNum (dim), exprs);
                }

                if (!FUNDEF_ISCUDAGLOBALFUN (fundef)
                    && !FUNDEF_ISCUDASTGLOBALFUN (fundef)) {
                    exprs = TBmakeExprs (TCmakeIdCopyString (
                                           GetBaseTypeFromExpr (argtab->ptr_in[i])),
                                         exprs);
                } else {
                    exprs = TBmakeExprs (TCmakeIdCopyString (
                                           GetBaseTypeFromExpr (argtab->ptr_in[i])),
                                         TBmakeExprs (TBmakeNum (TUgetFullDimEncoding (ID_NTYPE (
                                                        EXPRS_EXPR (argtab->ptr_in[i])))),
                                                      exprs));
                }

            } else if (NODE_TYPE (EXPRS_EXPR (argtab->ptr_in[i])) == N_globobj) {
                if (!FUNDEF_ISTHREADFUN (fundef)) {
                    if (TYgetMutcUsage (
                          OBJDEF_TYPE (GLOBOBJ_OBJDEF (EXPRS_EXPR (argtab->ptr_in[i]))))
                        == MUTC_US_THREADPARAM) {
                        exprs = TBmakeExprs (TCmakeIcm2 ("SET_NT_USG",
                                                         TCmakeIdCopyString ("FAG"),
                                                         DUPdoDupNode (EXPRS_EXPR (
                                                           argtab->ptr_in[i]))),
                                             icm_args);
                    } else {
                        exprs = TBmakeExprs (TCmakeIcm2 ("SET_NT_USG",
                                                         TCmakeIdCopyString ("TFA"),
                                                         DUPdoDupNode (EXPRS_EXPR (
                                                           argtab->ptr_in[i]))),
                                             icm_args);
                    }
                } else {
                    exprs
                      = TBmakeExprs (TCmakeIcm2 ("SET_NT_USG", TCmakeIdCopyString ("TAG"),
                                                 DUPdoDupNode (
                                                   EXPRS_EXPR (argtab->ptr_in[i]))),
                                     icm_args);
                }

                if (FUNDEF_RTSPECID (fundef) != NULL && global.config.rtspec
                    && ((fundef_in_current_namespace && FUNDEF_ISEXPORTED (fundef))
                        || !fundef_in_current_namespace)) {
                    shape = NTUgetShapeClassFromNType (
                      ID_NTYPE (EXPRS_EXPR (argtab->ptr_in[i])));
                    dim = TUgetDimEncoding (ID_NTYPE (EXPRS_EXPR (argtab->ptr_in[i])));
                    exprs = TBmakeExprs (TBmakeNum (shape), exprs);
                    exprs = TBmakeExprs (TBmakeNum (dim), exprs);
                }

                exprs = TBmakeExprs (TCmakeIdCopyString (
                                       GetBaseTypeFromExpr (argtab->ptr_in[i])),
                                     exprs);

#ifndef DBUG_OFF
            } else {
                DBUG_UNREACHABLE (
                  "arguments of N_ap should be either N_id or N_globobj!");
#endif
            }
        }

        if (shared) {
            icm_args = TBmakeExprs (TCmakeIdCopyString ("shared"), exprs);
        } else {
            icm_args
              = TBmakeExprs (TCmakeIdCopyString (global.argtag_string[argtab->tag[i]]),
                             exprs);
        }
    }

    icm_args = TBmakeExprs (TBmakeNumuint (argtab->size - 1), icm_args);

    if (!FUNDEF_ISSPMDFUN (fundef) && !FUNDEF_ISCUDAGLOBALFUN (fundef)
        && !FUNDEF_ISCUDASTGLOBALFUN (fundef)) {
        if (argtab->ptr_out[0] == NULL) {
            icm_args = TBmakeExprs (TCmakeIdCopyString (NULL), icm_args);
        } else {
            icm_args = TBmakeExprs (DUPdupIdsId (argtab->ptr_out[0]), icm_args);
        }
    }

    if (FUNDEF_ISINDIRECTWRAPPERFUN (fundef)
        || (FUNDEF_RTSPECID (fundef) != NULL && global.config.rtspec
            && ((fundef_in_current_namespace && FUNDEF_ISEXPORTED (fundef))
                || !fundef_in_current_namespace))) {
        argtab_t *fundef_argtab;

        fundef_argtab = FUNDEF_ARGTAB (fundef);

        DBUG_ASSERT (fundef_argtab != NULL, "no fundef_argtab found!");

        /* return value */
        DBUG_ASSERT (fundef_argtab->ptr_in[0] == NULL, "fundef_argtab inconsistent!");
        if (fundef_argtab->ptr_out[0] == NULL) {
            icm_args = TBmakeExprs (TCmakeIdCopyString (NULL), icm_args);
        } else {
            icm_args = TBmakeExprs (MakeBasetypeArg_NT (RET_TYPE (fundef_argtab->ptr_out[0])),
                                    icm_args);
        }
    }

    if (FUNDEF_RTSPECID (fundef) != NULL && global.config.rtspec
        && ((fundef_in_current_namespace && FUNDEF_ISEXPORTED (fundef))
            || !fundef_in_current_namespace)) {
        icm_args = TBmakeExprs (TCmakeIdCopyString (FUNDEF_RTSPECID (fundef)), icm_args);
        icm_args
          = TBmakeExprs (TCmakeIdCopyString (FUNDEF_SOURCENAME (fundef)), icm_args);
    }

    icm_args = TBmakeExprs (TCmakeIdCopyString (FUNDEF_NAME (fundef)), icm_args);

    DBUG_RETURN (icm_args);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeIcm_PROP_OBJ_OUT( node *prop_obj, node *assigns)
 *
 * @brief  Builds a N_assign node with the ND_PROP_OBJ_OUT icm.
 *
 ******************************************************************************/

static node *
MakeIcm_PROP_OBJ_OUT (node *prop_obj, node *lhs, node *assigns)
{
    node *exprs;
    node *icm_args;
    node *ret_node;
    unsigned int count;

    DBUG_ENTER ();

    exprs = PRF_ARGS (prop_obj);
    icm_args = NULL;
    count = 0;

    while (exprs != NULL) {
        icm_args = TBmakeExprs (DUPdupIdsIdNt (lhs),
                                TBmakeExprs (DUPdupIdNt (EXPRS_EXPR (exprs)), icm_args));
        count++;
        lhs = IDS_NEXT (lhs);
        exprs = EXPRS_NEXT (exprs);
    }

    icm_args = TBmakeExprs (TBmakeNumuint (count), icm_args);

    ret_node = TCmakeAssignIcm1 ("ND_PRF_PROP_OBJ_OUT", icm_args, assigns);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeIcm_PROP_OBJ_IN( node *prop_obj, node *assigns)
 *
 * @brief  Builds a N_assign node with the ND_PROP_OBJ_IN icm.
 *
 ******************************************************************************/

static node *
MakeIcm_PROP_OBJ_IN (node *prop_obj, node *lhs, node *assigns)
{
    node *exprs;
    node *icm_args;
    node *ret_node;
    unsigned int count;

    DBUG_ENTER ();

    exprs = EXPRS_NEXT (PRF_ARGS (prop_obj));
    icm_args = NULL;
    count = 0;

    while (exprs != NULL) {
        icm_args = TBmakeExprs (DUPdupIdsIdNt (lhs),
                                TBmakeExprs (DUPdupIdNt (EXPRS_EXPR (exprs)), icm_args));
        count++;
        lhs = IDS_NEXT (lhs);
        exprs = EXPRS_NEXT (exprs);
    }

    icm_args = TBmakeExprs (TBmakeNumuint (count), icm_args);

    ret_node = TCmakeAssignIcm1 ("ND_PRF_PROP_OBJ_IN", icm_args, assigns);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeIcmFPCases( int n)
 *
 * @brief  Builds a N_assign chain for FP_CASE icm's
 *
 ******************************************************************************/
static node *
MakeIcmFPCases (int n)
{
    int i;
    node *assign;

    DBUG_ENTER ();

    assign = TBmakeAssign (TBmakeIcm ("FP_SETUP_SLOW_END", NULL), NULL);

    for (i = n; i > 0; i--) {
        assign = TBmakeAssign (TBmakeIcm ("FP_CASE", TBmakeExprs (TBmakeNum (i), NULL)),
                               assign);
    }

    assign = TBmakeAssign (TBmakeIcm ("FP_SETUP_SLOW_START", NULL), assign);

    DBUG_RETURN (assign);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeFPAp( node *let, node *icm)
 *
 * @brief  Surround an N_ap node with fp start and end icms
 *
 ******************************************************************************/
static node *
MakeFPAp (node *let, node *icm, info *arg_info)
{
    node *fundef, *ids;
    node *livevars, *start, *vars, *end;
    node *assign;

    DBUG_ENTER ();

    fundef = INFO_FUNDEF (arg_info);

    livevars = NULL;
    vars = NULL;

    ids = LET_LIVEVARS (let);
    while (ids != NULL) {
        livevars = TCmakeAssignIcm2 ("SAC_FP_SET_LIVEVAR",
                                     TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                                     TCmakeIdCopyString (AVIS_NAME (LIVEVARS_AVIS (ids))),
                                     livevars);
        ids = LIVEVARS_NEXT (ids);
    }

    start = TCmakeIcm1 ("FP_SPAWN_START", TBmakeNum (LET_SPAWNSYNCINDEX (let)));

    ids = LET_SYNC_IDS (let);
    while (ids != NULL) {
        vars = TCmakeAssignIcm2 ("SAC_FP_SET_LIVEVAR",
                                 TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                                 TCmakeIdCopyString (AVIS_NAME (IDS_AVIS (ids))), vars);
        ids = IDS_NEXT (ids);
    }

    if (!FUNDEF_ISSLOWCLONE (fundef)) {
        end = TCmakeIcm3 ("FP_SPAWN_END_FAST", TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                          TCmakeIdCopyString (AVIS_NAME (IDS_AVIS (LET_IDS (let)))),
                          TBmakeNum (LET_SPAWNSYNCINDEX (let)));
    } else {
        end = TCmakeIcm4 ("FP_SPAWN_END_SLOW", TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                          TCmakeIdCopyString (AVIS_NAME (IDS_AVIS (LET_IDS (let)))),
                          TBmakeNum (LET_SPAWNSYNCINDEX (let)),
                          TBmakeNum (LET_SPAWNSYNCINDEX (LET_MATCHINGSPAWNSYNC (let))));
    }

    assign = TCappendAssign (
      livevars,
      TBmakeAssign (start,
                    TBmakeAssign (icm, TCappendAssign (vars, TBmakeAssign (end, NULL)))));

    DBUG_RETURN (assign);
}

/** <!--********************************************************************-->
 *
 * @fn  bool CheckAp( node *ap, info *arg_info)
 *
 * @brief  Checks whether no one of the externally refcounted in-arguments
 *         occurs on the LHS of the given application as well. (precompile
 *         should guarantee that!)
 *
 ******************************************************************************/

static bool
CheckAp (node *ap, info *arg_info)
{
    argtab_t *argtab;
    node *arg, *arg_id;
    node *let_ids;
    size_t ids_idx, arg_idx;
    bool ok = TRUE;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (ap) == N_ap, "no N_ap node found!");

    argtab = AP_ARGTAB (ap);
    DBUG_ASSERT (argtab != NULL, "no argtab found!");

    DBUG_ASSERT (argtab->ptr_in[0] == NULL, "argtab inconsistent");
    for (arg_idx = 1; arg_idx < argtab->size; arg_idx++) {
        arg = argtab->ptr_in[arg_idx];
        if (arg != NULL) {
            DBUG_ASSERT (NODE_TYPE (arg) == N_exprs, "no N_exprs node found in argtab!");
            arg_id = EXPRS_EXPR (arg);
            if (NODE_TYPE (arg_id) == N_id) {

                for (ids_idx = 0; ids_idx < argtab->size; ids_idx++) {
                    let_ids = argtab->ptr_out[ids_idx];
                    if ((let_ids != NULL) && (ids_idx != arg_idx)
                        && (STReq (ID_NAME (arg_id), IDS_NAME (let_ids)))) {
                        DBUG_ASSERT (global.argtag_is_in[argtab->tag[arg_idx]],
                                     "illegal tag found!");

                        if (!global.argtag_has_rc[argtab->tag[arg_idx]]) {
                            ok = FALSE;
                        }
                    }
                }
            }
        }
    }

    DBUG_RETURN (ok);
}

/** <!--********************************************************************-->
 *
 * @fn node *RhsId(node *arg_node, node *arg_info)
 *
 * @brief Compiles let expression with id on RHS.
 *        The return value is a N_assign chain of ICMs.
 *        Note, that the old 'arg_node' is removed by COMPLet.
 *
 *****************************************************************************/

static node *
RhsId (node *arg_node, info *arg_info)
{
    node *let_ids = NULL;
    node *ret_node = NULL;
    node *fundef;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    fundef = INFO_FUNDEF (arg_info);

    /*
     * 'arg_node' and 'let_ids' are both non-unique or both unique
     */
    if (!STReq (IDS_NAME (let_ids), ID_NAME (arg_node))) {
        if (fundef != NULL
            && (FUNDEF_ISCUDAGLOBALFUN (fundef) || FUNDEF_ISCUDASTGLOBALFUN (fundef))) {
            ret_node
              = TCmakeAssignIcm2 ("CUDA_ASSIGN",
                                  MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                                FALSE, TRUE, FALSE,
                                                MakeTypeArgs (ID_NAME (arg_node),
                                                              ID_NTYPE (arg_node), FALSE,
                                                              TRUE, FALSE, NULL)),
                                  TCmakeIdCopyString (
                                    GenericFun (GF_copy, ID_NTYPE (arg_node))),
                                  ret_node);
        } else {
            ret_node
              = TCmakeAssignIcm2 ("ND_ASSIGN",
                                  MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                                FALSE, TRUE, FALSE,
                                                MakeTypeArgs (ID_NAME (arg_node),
                                                              ID_NTYPE (arg_node), FALSE,
                                                              TRUE, FALSE, NULL)),
                                  TCmakeIdCopyString (
                                    GenericFun (GF_copy, ID_NTYPE (arg_node))),
                                  ret_node);
        }
    } else {
        /*
         * We are dealing with an assignment of the kind:
         *   a = a;
         * which we compile into:
         *   NOOP()
         */
        ret_node = TCmakeAssignIcm0 ("NOOP", ret_node);
    }
    DBUG_RETURN (ret_node);
}

static node *
AnnotateDescParamsWith3 (node *arg_node, info *arg_info)
{
    node *ops;
    DBUG_ENTER ();

    ops = INFO_WITHOPS (arg_info);
    INFO_WITHOPS (arg_info) = WITH3_OPERATIONS (arg_node);

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_WITHOPS (arg_info) = ops;

    DBUG_RETURN (arg_node);
}

static node *
AnnotateDescParamsAp (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_WITHOPS (AP_FUNDEF (arg_node)) == NULL) {
        FUNDEF_WITHOPS (AP_FUNDEF (arg_node)) = DUPdoDupTree (INFO_WITHOPS (arg_info));
    }

    DBUG_RETURN (arg_node);
}
/** <!--********************************************************************-->
 *
 * @fn  node *AnnotateDescParams( node *arg_node)
 *
 * @brief  Label function applications of thread functions from lifting
 *         the body of N_range into threads with a copy of the withops
 *         of the surrounding with3
 *
 *         Used to lift descriptors up one level
 *
 ******************************************************************************/
static node *
AnnotateDescParams (node *syntax_tree)
{
    anontrav_t trav[] = {{N_with3, &AnnotateDescParamsWith3},
                         {N_ap, &AnnotateDescParamsAp},
                         {(nodetype)0, NULL}};
    info *info;
    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpushAnonymous (trav, &TRAVsons);
    syntax_tree = TRAVopt (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn  static void COMPdoPrepareSmart(info *info)
 *
 * @brief  this function reads all database files that where created by the
 *         smart decision tool during the training phases. The file pointers
 *         to these files are stored in the arg_info node alongside with some
 *         important information such as the number of database files that
 *         where found, and the largest number of threads that where used during
 *         the training phase.
 *         More information can be found in mt_smart.c.
 *
 * @param info: arg_info node.
 *
 ******************************************************************************/
static void
COMPdoPrepareSmart (info *info)
{
    DBUG_ENTER ();

    FILE *fp;
    DIR *dp = opendir ("."); // pointer to current directory

    // reserve some memory to temporarly store the filenames of the database files that
    // are present in the current directory. The amount of memory that is reserved for
    // these filenames is an estimation and can be changed later.
    char *filename = MEMmalloc (
      (strlen (global.mt_smart_filename) + strlen (global.mt_smart_arch) + 15)
      * sizeof (char));
    char *file_id
      = MEMmalloc ((strlen (global.mt_smart_filename) + strlen (global.mt_smart_arch) + 2)
                   * sizeof (char));

    struct dirent *inode;
    int n, i = 0, nr_files = 0, max_nr_threads = 0;
    size_t n_ul;
    size_t filename_len
      = strlen (global.mt_smart_filename) + strlen (global.mt_smart_arch) + 14;

    if (dp == NULL) {
        CTIabort (EMPTY_LOC, "Unable to open current directory.");
    }

    // only target database files contain the following id.
    sprintf (file_id, "%s.%s", global.mt_smart_filename, global.mt_smart_arch);

    // scan current directory for database files. Only database files that contain the
    // above id are targeted. The id consists of the filename of the database and the name
    // of the architecture that one which to compile for. During this scan phase the
    // number of candidate database files as well as the largest number of threads that
    // where used during the training phase are computed.
    while ((inode = readdir (dp)) != NULL) {
        if (strstr (inode->d_name, file_id)) {
            if (strlen (inode->d_name) > filename_len) {
                MEMrealloc (filename, (strlen (inode->d_name) + 1) * sizeof (char));
                filename_len = strlen (inode->d_name);
            }
            strcpy (filename, inode->d_name);
            strtok (filename, ".");        // stat
            strtok (NULL, ".");            // <filename>
            strtok (NULL, ".");            // <architecture>
            n = atoi (strtok (NULL, ".")); // <threads>
            max_nr_threads = n > max_nr_threads ? n : max_nr_threads;
            nr_files++;
        }
    }

    if (nr_files == 0) {
        CTIabort (EMPTY_LOC,
          "No stat files found. Smart decisions can't be made without training data.");
    }

    // store largest number of threads
    INFO_NR_THREADS (info) = max_nr_threads;
    // reserve some memory to store the file pointers to the candidate
    // database files, also reserve some memory the store the size of
    // each database file.
    INFO_FP_LIST (info) = MEMmalloc (nr_files * sizeof (FILE *));
    INFO_LINE_COUNT (info) = MEMmalloc (nr_files * sizeof (int));
    // store the total number of database files
    INFO_NR_FILES (info) = nr_files;

    rewinddir (dp);

    // scan current directory a second time to store the file pointers
    // to the candidate database files as well as there file sizes.
    while ((inode = readdir (dp)) != NULL) {
        if (strstr (inode->d_name, file_id)) {
            strcpy (filename, inode->d_name);
            strtok (filename, ".");        // stat
            strtok (NULL, ".");            // <filename>
            strtok (NULL, ".");            // <architecture>
            n_ul = strtoul (strtok (NULL, "."),NULL,0); // <threads>
            INFO_LINE_COUNT (info)[i] = n_ul + 3;

            fp = fopen (inode->d_name, "r");
            if (fp == NULL) {
                CTIabort (EMPTY_LOC, "Unable to load stat files.");
            }
            INFO_FP_LIST (info)[i] = fp;
            i++;
        }
    }

    // free some memory that is no longer needed
    closedir (dp);
    MEMfree (filename);
    MEMfree (file_id);

    DBUG_RETURN ();
}

static void
COMPdoFinalizeSmart (info *info)
{
    DBUG_ENTER ();

    for (int i = 0; i < INFO_NR_FILES (info); i++) {
        fclose (INFO_FP_LIST (info)[i]);
    }
    MEMfree (INFO_FP_LIST (info));
    MEMfree (INFO_LINE_COUNT (info));

    DBUG_RETURN ();
}

static bool
CheckAUDOperators (node *withop, node **break_id)
{
    bool isfull = FALSE;
    bool hasmod=FALSE, hasgen=FALSE, hasfold=FALSE, hasbreak=FALSE;
    int numfold=0;
    DBUG_ENTER ();

    DBUG_ASSERT ((withop != NULL), "AUD with-loop without operator found");
    while (withop != NULL) {
        if (NODE_TYPE (withop) == N_genarray) {
            isfull = TRUE;
            hasgen = TRUE;
        } else if (NODE_TYPE (withop) == N_modarray) {
            isfull = TRUE;
            hasmod = TRUE;
        } else if (NODE_TYPE (withop) == N_fold) {
            hasfold = TRUE;
            numfold++;
        } else if (NODE_TYPE (withop) == N_break) {
            DBUG_ASSERT (!hasbreak, "more than one break operator in AUD WL");
            hasbreak = TRUE;
            *break_id = BREAK_MEM (withop);
        } else {
            DBUG_ASSERT( (NODE_TYPE (withop) == N_propagate),
                         "illegal operator in AUDWL found");
        }
        withop = WITHOP_NEXT (withop);
    }
    DBUG_ASSERT ((isfull && !hasfold && !hasbreak && !(hasgen && hasmod))
                 || (!hasgen && !hasmod), "illegal mix of operators in AUD WL");
    DBUG_ASSERT (isfull || !hasbreak || (numfold == 1), "illegal AUD WL operators: "
                                          "more than one fold and a single break!");

    DBUG_RETURN (isfull);
}

static node *
CreateAUDInitAccus (node *withop, node *res_ids)
{
    node *icm_chain = NULL;
    node *neutral, *let_neutral;

    DBUG_ENTER ();
    while (withop != NULL) {
        if (NODE_TYPE (withop) == N_fold) {
            neutral = FOLD_NEUTRAL (withop);
            let_neutral = TBmakeLet (DUPdoDupNode (res_ids), DUPdoDupNode (neutral));

            icm_chain = TCappendAssign (COMPdoCompile (let_neutral), icm_chain);
            res_ids = IDS_NEXT (res_ids);
        }
        withop = WITHOP_NEXT (withop);
    }
    DBUG_RETURN (icm_chain);
}

static node *
CreateAUDSuballocDescAllocChain (node *withop, node *res_ids, node *idx_id)
{
    node *icm_chain = NULL;
    node *sub_id;
    node *sub_get_dim;
    node *sub_set_shape;

    DBUG_ENTER ();
    while (withop != NULL) {
        if (((NODE_TYPE (withop) == N_modarray)
              || (NODE_TYPE (withop) == N_genarray))
            && (WITHOP_SUB (withop) != NULL)) {
            sub_id = WITHOP_SUB (withop);
            /*
             * Calculate dimension of subarray
             *
             * dim( A_sub) = dim( A) - size( iv)
             */
            sub_get_dim
              = TCmakeIcm2 (prf_ccode_tab[F_sub_SxS],
                            TCmakeIcm1 ("ND_A_DIM",
                                        TCmakeIdCopyStringNtNew (IDS_NAME (res_ids),
                                                              IDS_NTYPE (res_ids))),
                            TCmakeIcm1 ("ND_A_SIZE",
                                        TCmakeIdCopyStringNtNew (ID_NAME (idx_id),
                                                              ID_NTYPE (idx_id))));
            /*
             * Annotate shape of subarray if default present
             * (genarray only)
             */
            if ((NODE_TYPE (withop) == N_genarray)
                && (!KNOWN_SHAPE (TUgetFullDimEncoding (ID_NTYPE (sub_id))))) {
                if (GENARRAY_DEFAULT (withop) != NULL) {
                    DBUG_PRINT ("creating COPY__SHAPE for SUBALLOC var");
                    /*
                     * copy shape
                     */
                    sub_set_shape
                      = TCmakeIcm1 ("ND_COPY__SHAPE",
                                    MakeTypeArgs (ID_NAME (sub_id), ID_NTYPE (sub_id),
                                                  FALSE, TRUE, FALSE,
                                                  MakeTypeArgs (ID_NAME (
                                                                  GENARRAY_DEFAULT (
                                                                    withop)),
                                                                ID_NTYPE (
                                                                  GENARRAY_DEFAULT (
                                                                    withop)),
                                                                FALSE, TRUE, FALSE,
                                                                NULL)));

                    icm_chain = TBmakeAssign (sub_set_shape, icm_chain);
                } else {
                    DBUG_UNREACHABLE ("no default value found! "
                                      "cannot create subvar shape");
                }
            } else if ((NODE_TYPE (withop) == N_modarray)
                       && (!KNOWN_SHAPE (TUgetFullDimEncoding (ID_NTYPE (sub_id))))) {
                DBUG_PRINT ("creating WL_MODARRAY_SUBSHAPE for SUBALLOC var");
                /*
                 * set shape in modarray case based upon result
                 * and index vector
                 */
                sub_set_shape
                  = TCmakeIcm4 ("WL_MODARRAY_SUBSHAPE",
                                TCmakeIdCopyStringNtNew (ID_NAME (sub_id), ID_NTYPE (sub_id)),
                                DUPdupIdNt (idx_id),
                                TBmakeNum (TUgetDimEncoding (ID_NTYPE (sub_id))),
                                DUPdupIdsIdNt (res_ids));
                icm_chain = TBmakeAssign (sub_set_shape, icm_chain);
            }

            /*
             * Allocate descriptor of subarray
             */
            icm_chain = MakeAllocDescIcm (ID_NAME (sub_id), ID_NTYPE (sub_id), 1,
                                          sub_get_dim, icm_chain);
            res_ids = IDS_NEXT (res_ids);
        }
        withop = WITHOP_NEXT (withop);
    }
    DBUG_RETURN (icm_chain);
}

static node *
CreateAUDSuballocDescFreeChain (node *withop)
{
    node *icm_chain = NULL;
    node *sub_id;

    DBUG_ENTER ();
    while (withop != NULL) {
        if (((NODE_TYPE (withop) == N_modarray)
             || (NODE_TYPE (withop) == N_genarray))
            && (WITHOP_SUB (withop) != NULL)) {

            sub_id = WITHOP_SUB (withop);
            icm_chain = TCmakeAssignIcm1 ("ND_FREE__DESC",
                                          TCmakeIdCopyStringNtNew (ID_NAME (sub_id),
                                                                   ID_NTYPE (sub_id)),
                                          icm_chain);
        }
        withop = WITHOP_NEXT (withop);
    }
    DBUG_RETURN (icm_chain);
}

/******************************************************************************
 *
 * COMPILE traversal functions
 *
 ******************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn  node *COMPdoCompile( node *arg_node)
 *
 * @brief  Starts compilation.
 *
 ******************************************************************************/

node *
COMPdoCompile (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    if (global.mt_smart_mode == 2) {
        COMPdoPrepareSmart (info);
    }

    INFO_FOLDLUT (info) = LUTgenerateLut ();

    TRAVpush (TR_comp);

    if (global.mutc_suballoc_desc_one_level_up) {
        arg_node = AnnotateDescParams (arg_node);
    }

    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    INFO_FOLDLUT (info) = LUTremoveLut (INFO_FOLDLUT (info));

    if (global.mt_smart_mode == 2) {
        COMPdoFinalizeSmart (info);
    }

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPmodule( node *arg_node, info *arg_info)
 *
 * @brief  Compiles a N_module node: traverses sons.
 *
 ******************************************************************************/

node *
COMPmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_MODUL (arg_info) = arg_node;

    if (global.mtmode != MT_none) {
        INFO_SPMDFRAME (arg_info)
          = TBmakeAssign (TCmakeIcm0 ("MT_SPMD_FRAME_END"), INFO_SPMDFRAME (arg_info));
        INFO_SPMDBARRIER (arg_info) = TBmakeAssign (TCmakeIcm0 ("MT_SPMD_BARRIER_END"),
                                                    INFO_SPMDBARRIER (arg_info));
    }

    MODULE_OBJS (arg_node) = TRAVopt(MODULE_OBJS (arg_node), arg_info);

    MODULE_FUNDECS (arg_node) = TRAVopt(MODULE_FUNDECS (arg_node), arg_info);

    MODULE_FUNS (arg_node) = TRAVopt(MODULE_FUNS (arg_node), arg_info);

    MODULE_THREADFUNS (arg_node) = TRAVopt (MODULE_THREADFUNS (arg_node), arg_info);

    MODULE_TYPES (arg_node) = TRAVopt(MODULE_TYPES (arg_node), arg_info);

    if (global.mtmode != MT_none) {
        INFO_SPMDFRAME (arg_info)
          = TBmakeAssign (TCmakeIcm0 ("MT_SPMD_FRAME_BEGIN"), INFO_SPMDFRAME (arg_info));
        INFO_SPMDBARRIER (arg_info) = TBmakeAssign (TCmakeIcm0 ("MT_SPMD_BARRIER_BEGIN"),
                                                    INFO_SPMDBARRIER (arg_info));

        MODULE_SPMDSTORE (arg_node)
          = TCappendAssign (INFO_SPMDFRAME (arg_info), INFO_SPMDBARRIER (arg_info));

        INFO_SPMDFRAME (arg_info) = NULL;
        INFO_SPMDBARRIER (arg_info) = NULL;
    }

    if (global.fp) {
        MODULE_FPFRAMESTORE (arg_node)
          = TCmakeAssignIcm0 ("FP_FRAME_START",
                              TCappendAssign (INFO_FPFRAME (arg_info),
                                              TCmakeAssignIcm0 ("FP_FRAME_END", NULL)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPtypedef( node *arg_node, info *arg_info)
 *
 * @brief  If needed an appropriate ICM is generated and stored in TYPEDEF_ICM.
 *         The rest of the N_typdef node ist left untouched!
 *
 ******************************************************************************/

node *
COMPtypedef (node *arg_node, info *arg_info)
{
#ifndef DBUG_OFF
    char *tmp_str = NULL;
#endif
    node *icm = NULL;

    DBUG_ENTER ();

    DBUG_EXECUTE (tmp_str = TYtype2DebugString (TYPEDEF_NTYPE (arg_node), FALSE, 0); );
    DBUG_PRINT ("compiling typedef \"%s\"::%s", TYPEDEF_NAME (arg_node), tmp_str);
    DBUG_EXECUTE (tmp_str = MEMfree (tmp_str););

    icm
      = TCmakeIcm1 ("ND_TYPEDEF", MakeTypeArgs (TYPEDEF_NAME (arg_node),
                                                TYPEDEF_NTYPE (arg_node),
                                                TRUE, FALSE, FALSE, NULL));

    TYPEDEF_ICM (arg_node) = icm;

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);
    } else {
        DBUG_EXECUTE (UTprintRepository (stderr););
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPobjdef( node *arg_node, info *arg_info)
 *
 * @brief  If needed an appropriate ICM is generated and stored in OBJDEF_ICM.
 *         Furthermore, the NT TAG is added to the objdef node.
 *         The rest of the N_objdef node ist left untouched!
 *
 ******************************************************************************/

node *
COMPobjdef (node *arg_node, info *arg_info)
{
    node *icm;

    DBUG_ENTER ();

    if (!OBJDEF_ISLOCAL (arg_node)) {
        icm = TCmakeIcm1 ("ND_OBJDEF_EXTERN",
                          MakeTypeArgs (OBJDEF_NAME (arg_node),
                                        OBJDEF_TYPE (arg_node), TRUE,
                                        TRUE, FALSE, NULL));
    } else {
        icm = TCmakeIcm1 ("ND_OBJDEF",
                          MakeTypeArgs (OBJDEF_NAME (arg_node),
                                        OBJDEF_TYPE (arg_node), TRUE,
                                        TRUE, TRUE, NULL));
    }
    OBJDEF_ICM (arg_node) = icm;

    OBJDEF_NT_TAG (arg_node)
      = NTUcreateNtTagFromNType (OBJDEF_NAME (arg_node), OBJDEF_TYPE (arg_node));

    OBJDEF_NEXT (arg_node) = TRAVopt(OBJDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPFundefArgs( node *fundef, info *arg_info)
 *
 * @brief  ...
 *
 ******************************************************************************/

static node *
COMPFundefArgs (node *fundef, info *arg_info)
{
    argtab_t *argtab;
    node *arg;
    size_t i;
    node *assigns = NULL;

    DBUG_ENTER ();

    DBUG_PRINT ("processing arguments of fundef %s", FUNDEF_NAME (fundef));

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef, "no N_fundef node found!");

    argtab = FUNDEF_ARGTAB (fundef);
    DBUG_ASSERT (argtab != NULL, "no argtab found!");

    /*
     * Additional icms for the function body are generated regardless of the
     * existence of such a block, but COMPFundef() inserts them only if a
     * block already exists.
     */

    if (!FUNDEF_ISEXTERN (fundef)) {

        DBUG_ASSERT (argtab->ptr_in[0] == NULL, "argtab inconsistent");
        for (i = 1; i < argtab->size; i++) {
            arg = argtab->ptr_in[i];
            if (arg != NULL) {
                DBUG_ASSERT (NODE_TYPE (arg) == N_arg, "no N_arg node found in argtab!");

                /*
                 * put "ND_DECL__MIRROR_PARAM" ICMs at beginning of function block
                 *   AND IN FRONT OF THE DECLARATION ICMs!!!
                 */
                assigns = TCmakeAssignIcm1 ("ND_DECL__MIRROR_PARAM",
                                            MakeTypeArgs (ARG_NAME (arg), ARG_NTYPE (arg),
                                                          FALSE, TRUE, TRUE, NULL),
                                            assigns);

                /*
                 * put "ND_DECL_PARAM_inout" ICM at beginning of function block
                 *   AND IN FRONT OF THE DECLARATION ICMs!!!
                 */
                if (argtab->tag[i] == ATG_inout) {
                    assigns
                      = TCmakeAssignIcm1 ("ND_DECL_PARAM_inout",
                                          MakeTypeArgs (ARG_NAME (arg), ARG_NTYPE (arg),
                                                        TRUE, FALSE, FALSE, NULL),
                                          assigns);
                }
            }
        }
    }

    DBUG_RETURN (assigns);
}

static node *
AddDescParams (node *ops, node *params)
{
    DBUG_ENTER ();

    if (ops != NULL) {
        if (WITHOP_SUB (ops) != NULL) {
            shape_class_t shapeClass
              = NTUgetShapeClassFromNType (ID_NTYPE (WITHOP_SUB (ops)));
            if (shapeClass == C_akd || shapeClass == C_aud) {
                node *arg2
                  = TBmakeExprs (TCmakeIcm2 ("SET_NT_USG", TCmakeIdCopyString ("TPA"),
                                             DUPdupIdNt (WITHOP_SUB (ops))),
                                 NULL);
                node *newParam
                  = TBmakeExprs (TCmakeIdCopyString ("in_justdesc"),
                                 TBmakeExprs (TCmakeIdCopyString ("int"), arg2));

                params = TCappendExprs (params, newParam);
                NUM_VAL (EXPRS_EXPR3 (params)) += 1;
            }
        }
        params = AddDescParams (WITHOP_NEXT (ops), params);
    }

    DBUG_RETURN (params);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPfundef( node *arg_node, info *arg_info)
 *
 * @brief  ...
 *
 ******************************************************************************/

node *
COMPfundef (node *arg_node, info *arg_info)
{
    node *old_fundef;
    node *assigns;
    node *icm_args;

    DBUG_ENTER ();

    DBUG_PRINT ("compiling %s", FUNDEF_NAME (arg_node));

    if (!FUNDEF_ISZOMBIE (arg_node)) {
        /*
         * push 'arg_info'
         */
        old_fundef = INFO_FUNDEF (arg_info);
        INFO_FUNDEF (arg_info) = arg_node;

        /********** begin: traverse body **********/

        /*
         * Each scheduler within a single SPMD function must be associated with a
         * unique segment ID. This is realized by means of the following counter.
         */
        INFO_SCHEDULERID (arg_info) = 0;

        /*
         * For each scheduler a specific initialization ICM is created during the
         * traversal of an SPMD function. They are chained by means of N_assign
         * nodes and will later be inserted into the code which sets up the
         * environment for multithreaded execution.
         */
        INFO_SCHEDULERINIT (arg_info) = NULL;

        if (FUNDEF_BODY (arg_node) != NULL) {

            /*
             * Init FP frame before traversing body
             */
            if (FUNDEF_CONTAINSSPAWN (arg_node) && !FUNDEF_ISSLOWCLONE (arg_node)) {
                INFO_FPFRAME (arg_info)
                  = TCmakeAssignIcm1 ("FP_FRAME_FUNC_END",
                                      TCmakeIdCopyString (FUNDEF_NAME (arg_node)), NULL);
            }

            /*
             * Traverse body
             */
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            DBUG_ASSERT (INFO_POSTFUN (arg_info) == NULL,
                         "left-over postfun icms found!");
            /*
             * Store collected scheduler information.
             *
             * comment by msd:
             * Why do we store this in the first place? This BLOCK_SCHEDULER_INIT
             * property is only set here and never read. I'm commenting this out and
             * storing the assignments directly in the fundef_body.
             */
            /*BLOCK_SCHEDULER_INIT( FUNDEF_BODY( arg_node))
              = INFO_SCHEDULERINIT( arg_info);*/

            if (INFO_SCHEDULERINIT (arg_info) != NULL) {
                BLOCK_ASSIGNS (FUNDEF_BODY (arg_node))
                  = TCappendAssign (INFO_SCHEDULERINIT (arg_info),
                                    BLOCK_ASSIGNS (FUNDEF_BODY (arg_node)));

                INFO_SCHEDULERINIT (arg_info) = NULL;
            }

            if (INFO_SCHEDULERID (arg_info) > global.max_schedulers) {
                global.max_schedulers = INFO_SCHEDULERID (arg_info);
            }

            /*
             * Add init memory allocator
             */
            if (FUNDEF_NEEDSDYNAMICMEMORY (arg_node)) {
                BLOCK_ASSIGNS (FUNDEF_BODY (arg_node))
                  = TCappendAssign (TBmakeAssign (TBmakeIcm ("SAC_INIT_LOCAL_MEM", NULL),
                                                  BLOCK_ASSIGNS (FUNDEF_BODY (arg_node))),
                                    TBmakeAssign (TBmakeIcm ("SAC_CLEANUP_LOCAL_MEM",
                                                             NULL),
                                                  NULL));
            }

            /*
             * Add setup code for Functional Parallelism
             */
            if (FUNDEF_CONTAINSSPAWN (arg_node)) {
                if (FUNDEF_ISSLOWCLONE (arg_node)) {
                    BLOCK_ASSIGNS (FUNDEF_BODY (arg_node))
                      = TCappendAssign (MakeIcmFPCases (FUNDEF_NUMSPAWNSYNC (arg_node)),
                                        BLOCK_ASSIGNS (FUNDEF_BODY (arg_node)));
                } else {
                    BLOCK_ASSIGNS (FUNDEF_BODY (arg_node))
                      = TBmakeAssign (TCmakeIcm1 ("FP_SETUP_FAST",
                                                  TCmakeIdCopyString (FUNDEF_NAME (
                                                    FUNDEF_SLOWCLONE (arg_node)))),
                                      BLOCK_ASSIGNS (FUNDEF_BODY (arg_node)));
                }
            }
        }

        /********** end: traverse body **********/

        /*
         * traverse arguments
         */
        if ((FUNDEF_ARGS (arg_node) != NULL) && (FUNDEF_BODY (arg_node) != NULL)
            && !FUNDEF_ISCUDAGLOBALFUN (arg_node)
            && !FUNDEF_ISCUDASTGLOBALFUN (arg_node)) {
            assigns = COMPFundefArgs (arg_node, arg_info);

            /* new first assignment of body */
            BLOCK_ASSIGNS (FUNDEF_BODY (arg_node))
              = TCappendAssign (assigns, BLOCK_ASSIGNS (FUNDEF_BODY (arg_node)));
        }

        if (FUNDEF_ISSPMDFUN (arg_node) || FUNDEF_ISXTSPMDFUN (arg_node)) {
            /* TODO: SPMD funs and XT-SPMD funs are identical and should be merged */
            icm_args = MakeFunctionArgsSpmd (arg_node);
            FUNDEF_ICMDECL (arg_node) = TBmakeIcm ("MT_SPMDFUN_DECL", icm_args);
            FUNDEF_ICMDEFBEGIN (arg_node)
              = TBmakeIcm ("MT_SPMDFUN_DEF_BEGIN", DUPdoDupTree (icm_args));
            FUNDEF_ICMDEFEND (arg_node)
              = TBmakeIcm ("MT_SPMDFUN_DEF_END", DUPdoDupTree (icm_args));
        } else if (FUNDEF_ISMTFUN (arg_node) || FUNDEF_ISXTFUN (arg_node)) {
            /* The MT ICMs are used for both the MT and XT funs because the interface is
             * the same. However, internally the difference is that XT funs may do an SPMD
             * invocation, while the MT funs cannot. */
            icm_args = MakeFunctionArgs (arg_node);
            FUNDEF_ICMDECL (arg_node) = TBmakeIcm ("MT_MTFUN_DECL", icm_args);
            FUNDEF_ICMDEFBEGIN (arg_node)
              = TBmakeIcm ("MT_MTFUN_DEF_BEGIN", DUPdoDupTree (icm_args));
            FUNDEF_ICMDEFEND (arg_node)
              = TBmakeIcm ("MT_MTFUN_DEF_END", DUPdoDupTree (icm_args));
        } else if (FUNDEF_ISTHREADFUN (arg_node)) {
            icm_args = MakeFunctionArgs (arg_node);
            if (FUNDEF_ISSPAWNFUN (arg_node)) {
                FUNDEF_ICMDECL (arg_node) = TBmakeIcm ("MUTC_SPAWNFUN_DECL", icm_args);
                FUNDEF_ICMDEFBEGIN (arg_node)
                  = TBmakeIcm ("MUTC_SPAWNFUN_DEF_BEGIN", DUPdoDupTree (icm_args));
                FUNDEF_ICMDEFEND (arg_node)
                  = TBmakeIcm ("MUTC_SPAWNFUN_DEF_END", DUPdoDupTree (icm_args));
            } else {
                if (FUNDEF_WASWITH3BODY (arg_node)
                    && global.mutc_suballoc_desc_one_level_up
                    && FUNDEF_WITHOPS (arg_node) != NULL) {
                    icm_args = AddDescParams (FUNDEF_WITHOPS (arg_node), icm_args);
                }
                FUNDEF_ICMDECL (arg_node) = TBmakeIcm ("MUTC_THREADFUN_DECL", icm_args);
                FUNDEF_ICMDEFBEGIN (arg_node)
                  = TBmakeIcm ("MUTC_THREADFUN_DEF_BEGIN", DUPdoDupTree (icm_args));
                FUNDEF_ICMDEFEND (arg_node)
                  = TBmakeIcm ("MUTC_THREADFUN_DEF_END", DUPdoDupTree (icm_args));
            }
        } else if (FUNDEF_ISCUDAGLOBALFUN (arg_node)
                   || FUNDEF_ISCUDASTGLOBALFUN (arg_node)) {
            icm_args = MakeFunctionArgsCuda (arg_node);
            FUNDEF_ICMDECL (arg_node) = TBmakeIcm ("CUDA_GLOBALFUN_DECL", icm_args);
            FUNDEF_ICMDEFBEGIN (arg_node)
              = TBmakeIcm ("CUDA_GLOBALFUN_DEF_BEGIN", DUPdoDupTree (icm_args));
            FUNDEF_ICMDEFEND (arg_node)
              = TBmakeIcm ("CUDA_GLOBALFUN_DEF_END", DUPdoDupTree (icm_args));
        } else if (FUNDEF_ISWRAPPERENTRYFUN (arg_node)) {
            icm_args = MakeFunctionArgs (arg_node);
            FUNDEF_ICMDECL (arg_node) = TBmakeIcm ("ND_FUN_DECL", icm_args);
            FUNDEF_ICMDEFBEGIN (arg_node)
              = TBmakeIcm ("WE_FUN_DEF_BEGIN", DUPdoDupTree (icm_args));
            FUNDEF_ICMDEFEND (arg_node)
              = TBmakeIcm ("WE_FUN_DEF_END", DUPdoDupTree (icm_args));
        } else if (global.backend == BE_distmem) {
            if (FUNDEF_DISTMEMHASSIDEEFFECTS (arg_node)) {
                icm_args = MakeFunctionArgs (arg_node);
                FUNDEF_ICMDECL (arg_node)
                  = TBmakeIcm ("ND_DISTMEM_FUN_DECL_WITH_SIDE_EFFECTS", icm_args);
                FUNDEF_ICMDEFBEGIN (arg_node)
                  = TBmakeIcm ("ND_FUN_DEF_BEGIN", DUPdoDupTree (icm_args));
                FUNDEF_ICMDEFEND (arg_node)
                  = TBmakeIcm ("ND_FUN_DEF_END", DUPdoDupTree (icm_args));
            } else {
                icm_args = MakeFunctionArgs (arg_node);
                FUNDEF_ICMDECL (arg_node) = TBmakeIcm ("ND_FUN_DECL", icm_args);
                FUNDEF_ICMDEFBEGIN (arg_node)
                  = TBmakeIcm ("ND_FUN_DEF_BEGIN", DUPdoDupTree (icm_args));
                FUNDEF_ICMDEFEND (arg_node)
                  = TBmakeIcm ("ND_FUN_DEF_END", DUPdoDupTree (icm_args));
            }
        } else {
            /* ST and SEQ functions */
            icm_args = MakeFunctionArgs (arg_node);
            FUNDEF_ICMDECL (arg_node) = TBmakeIcm ("ND_FUN_DECL", icm_args);
            FUNDEF_ICMDEFBEGIN (arg_node)
              = TBmakeIcm ("ND_FUN_DEF_BEGIN", DUPdoDupTree (icm_args));
            FUNDEF_ICMDEFEND (arg_node)
              = TBmakeIcm ("ND_FUN_DEF_END", DUPdoDupTree (icm_args));
        }

        /*
         * traverse next fundef
         */
        FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

        /*
         * Create entries in SPMD frame and SPMD barrier
         */
        if (FUNDEF_ISSPMDFUN (arg_node)) {
            node *icm;

            icm = DUPdoDupNode (FUNDEF_ICMDEFBEGIN (arg_node));
            ICM_NAME (icm) = "MT_SPMD_FRAME_ELEMENT";
            INFO_SPMDFRAME (arg_info) = TBmakeAssign (icm, INFO_SPMDFRAME (arg_info));

            icm = DUPdoDupNode (FUNDEF_ICMDEFBEGIN (arg_node));
            ICM_NAME (icm) = "MT_SPMD_BARRIER_ELEMENT";
            INFO_SPMDBARRIER (arg_info) = TBmakeAssign (icm, INFO_SPMDBARRIER (arg_info));
        }

        /*
         * Create enties in FP frame
         */
        if (FUNDEF_CONTAINSSPAWN (arg_node) && !FUNDEF_ISSLOWCLONE (arg_node)) {
            node *livevars;
            node *avis;
            ntype *type;

            livevars = FUNDEF_LIVEVARS (arg_node);

            while (livevars != NULL) {
                // TODO: print the type as well
                avis = LIVEVARS_AVIS (livevars);

                type = AVIS_TYPE (avis);

                INFO_FPFRAME (arg_info)
                  = TCmakeAssignIcm2 ("FP_FRAME_LIVEVAR", MakeBasetypeArg (type),
                                      TCmakeIdCopyString (AVIS_NAME (avis)),
                                      INFO_FPFRAME (arg_info));
                livevars = LIVEVARS_NEXT (livevars);
            }

            INFO_FPFRAME (arg_info)
              = TCmakeAssignIcm0 ("FP_FRAME_FUNC_START", INFO_FPFRAME (arg_info));
        }

        /*
         * pop 'arg_info'
         */
        INFO_FUNDEF (arg_info) = old_fundef;
    } else {
        FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPvardec( node *arg_node, info *arg_info)
 *
 * @brief  Compiles a N_vardec node. The generated ICM chain is stored in
 *         VARDEC_ICM. The rest of the N_vardec node is left untouched in order
 *         to have the declarations still available. (Note, that each id
 *         contains a pointer to its vardec ...!)
 *
 ******************************************************************************/

node *
COMPvardec (node *arg_node, info *arg_info)
{
#ifndef DBUG_OFF
    char *tmp_str = NULL;
#endif

    DBUG_ENTER ();

    DBUG_EXECUTE (tmp_str = TYtype2DebugString (VARDEC_NTYPE (arg_node), FALSE, 0); );
    DBUG_PRINT ("      var \"%s\" of type %s", VARDEC_NAME (arg_node), tmp_str);
    DBUG_EXECUTE (tmp_str = MEMfree (tmp_str););

    if (TUgetSimpleImplementationType (VARDEC_NTYPE (arg_node)) == T_sync) {
        if (global.backend != BE_mutc) {
            DBUG_PRINT ("Removing sync vardec");

            VARDEC_ICM (arg_node) = TCmakeIcm0 ("NOOP");

            if (!FUNDEF_ISSLOWCLONE (INFO_FUNDEF (arg_info))) {
                INFO_FPFRAME (arg_info)
                  = TCmakeAssignIcm1 ("FP_FRAME_SYNC",
                                      TCmakeIdCopyString (
                                        AVIS_NAME (VARDEC_AVIS (arg_node))),
                                      INFO_FPFRAME (arg_info));
            }
        } else {
            VARDEC_ICM (arg_node)
              = TCmakeIcm1 ("SAC_MUTC_DECL_SYNCVAR",
                            TCmakeIdCopyString (VARDEC_NAME (arg_node)));
        }
    } else if (AVIS_ISTHREADINDEX (VARDEC_AVIS (arg_node))) {
        VARDEC_ICM (arg_node) = TCmakeIcm1 ("SAC_MUTC_DECL_INDEX",
                                            TCmakeIdCopyString (VARDEC_NAME (arg_node)));
    } else if (FUNDEF_ISCUDAGLOBALFUN (INFO_FUNDEF (arg_info)) &&
               /* !CUisDeviceTypeNew( VARDEC_NTYPE( arg_node)) && */
               AVIS_ISCUDALOCAL (VARDEC_AVIS (arg_node))
               && TUgetFullDimEncoding (VARDEC_NTYPE (arg_node)) > 0) {
        VARDEC_ICM (arg_node)
          = TCmakeIcm1 ("CUDA_DECL_KERNEL_ARRAY",
                        MakeTypeArgs (VARDEC_NAME (arg_node), VARDEC_NTYPE (arg_node),
                                      TRUE, TRUE, TRUE, NULL));
    } else if (FUNDEF_ISCUDAGLOBALFUN (INFO_FUNDEF (arg_info))
               && CUisShmemTypeNew (VARDEC_NTYPE (arg_node))
               && TUgetFullDimEncoding (VARDEC_NTYPE (arg_node)) != 0) {
        VARDEC_ICM (arg_node)
          = TCmakeIcm1 ("CUDA_DECL_SHMEM_ARRAY",
                        MakeTypeArgs (VARDEC_NAME (arg_node), VARDEC_NTYPE (arg_node),
                                      TRUE, TRUE, TRUE, NULL));
    } else {
        if (VARDEC_INIT (arg_node) != NULL) {
            VARDEC_ICM (arg_node)
              = TCmakeIcm2 ("ND_DECL_CONST__DATA",
                            MakeTypeArgs (VARDEC_NAME (arg_node), VARDEC_NTYPE (arg_node),
                                          TRUE, FALSE, FALSE, NULL),
                            VARDEC_INIT (arg_node));
            VARDEC_INIT (arg_node) = NULL;
        } else if (TUisNested (VARDEC_NTYPE (arg_node)) ) {
            VARDEC_ICM (arg_node)
              = TCmakeIcm1 ("ND_DECL_NESTED",
                            MakeTypeArgs (VARDEC_NAME (arg_node), VARDEC_NTYPE (arg_node),
                                          TRUE, TRUE, TRUE, NULL));
        } else if (global.backend == BE_distmem
                   && AVIS_DISTMEMSUBALLOC (VARDEC_AVIS (arg_node))) {
            VARDEC_ICM (arg_node)
              = TCmakeIcm1 ("ND_DSM_DECL",
                            MakeTypeArgs (VARDEC_NAME (arg_node), VARDEC_NTYPE (arg_node),
                                          TRUE, TRUE, TRUE, NULL));
        } else {
            VARDEC_ICM (arg_node)
              = TCmakeIcm1 ("ND_DECL",
                            MakeTypeArgs (VARDEC_NAME (arg_node), VARDEC_NTYPE (arg_node),
                                          TRUE, TRUE, TRUE, NULL));
        }
    }

    if (AVIS_SUBALLOC (VARDEC_AVIS (arg_node))
        && global.mutc_suballoc_desc_one_level_up) {
        INFO_VARDEC_INIT (arg_info)
          = TCmakeAssignIcm1 ("MUTC_INIT_SUBALLOC_DESC",
                              MakeTypeArgs (VARDEC_NAME (arg_node),
                                            VARDEC_NTYPE (arg_node), FALSE, FALSE, FALSE,
                                            NULL),
                              INFO_VARDEC_INIT (arg_info));
    }

    VARDEC_NEXT (arg_node) = TRAVopt(VARDEC_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPblock( node *arg_node, info *arg_info)
 *
 * @brief  Compiles a N_block node.
 *
 ******************************************************************************/

node *
COMPblock (node *arg_node, info *arg_info)
{
    node *assign;
    char *fun_name, *cs_tag;

    DBUG_ENTER ();

    if (BLOCK_CACHESIM (arg_node) != NULL) {
        fun_name = FUNDEF_NAME (INFO_FUNDEF (arg_info));
        cs_tag = (char *)MEMmalloc (STRlen (BLOCK_CACHESIM (arg_node)) + STRlen (fun_name)
                                    + 14);
        if (BLOCK_CACHESIM (arg_node)[0] == '\0') {
            sprintf (cs_tag, "\"%s(...)\"", fun_name);
        } else {
            sprintf (cs_tag, "\"%s in %s(...)\"", BLOCK_CACHESIM (arg_node), fun_name);
        }

        BLOCK_CACHESIM (arg_node) = MEMfree (BLOCK_CACHESIM (arg_node));

        BLOCK_ASSIGNS (arg_node)
          = TCmakeAssignIcm1 ("CS_START", TCmakeIdCopyString (cs_tag),
                              BLOCK_ASSIGNS (arg_node));
        assign = BLOCK_ASSIGNS (arg_node);

        while ((ASSIGN_NEXT (assign) != NULL)
               && (NODE_TYPE (ASSIGN_STMT (ASSIGN_NEXT (assign))) != N_return)) {
            assign = ASSIGN_NEXT (assign);
        }

        ASSIGN_NEXT (assign) = TCmakeAssignIcm1 ("CS_STOP", TCmakeIdCopyString (cs_tag),
                                                 ASSIGN_NEXT (assign));
    }

    DBUG_PRINT ("   traversing assignments...");
    BLOCK_ASSIGNS (arg_node) = TRAVopt(BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_PRINT ("   traversing vardecs...");
    BLOCK_VARDECS (arg_node) = TRAVopt(BLOCK_VARDECS (arg_node), arg_info);

    if (INFO_VARDEC_INIT (arg_info) != NULL) {
        BLOCK_ASSIGNS (arg_node)
          = TCappendAssign (INFO_VARDEC_INIT (arg_info), BLOCK_ASSIGNS (arg_node));
        INFO_VARDEC_INIT (arg_info) = NULL;
    }
    DBUG_PRINT ("   done...");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPassign( node *arg_node, info *arg_info)
 *
 * @brief  Compiles a N_assign node.
 *         Note, that the traversal of ASSIGN_STMT( arg_node) may return a
 *         N_assign chain instead of an expression.
 *
 ******************************************************************************/

node *
COMPassign (node *arg_node, info *arg_info)
{
    node *instr, *last, *next;

    DBUG_ENTER ();

    INFO_ASSIGN (arg_info) = arg_node;
    instr = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    next = ASSIGN_NEXT (arg_node);

    if (NODE_TYPE (instr) == N_assign) {
        /*
         * a N_assign chain was returned.
         *  -> insert N_assign chain at the current position into the tree.
         */

        /* insert head of 'instr' into AST */
        ASSIGN_STMT (arg_node) = ASSIGN_STMT (instr);

        /* insert tail of 'instr' into AST (last element) */
        last = instr;
        while (ASSIGN_NEXT (last) != NULL) {
            last = ASSIGN_NEXT (last);
        }
        ASSIGN_NEXT (last) = ASSIGN_NEXT (arg_node);

        /* free head of 'instr' */
        ASSIGN_STMT (instr) = NULL;
        instr = FREEdoFreeNode (instr);

        /* insert tail of 'instr' into AST (first element) */
        ASSIGN_NEXT (arg_node) = instr;
    } else {
        /* insert 'instr' into AST */
        ASSIGN_STMT (arg_node) = instr;
    }

    next = TRAVopt(next, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  static node *MakeFunRetArgs( node *arg_node, info *arg_info)
 *
 * @brief  Generates ICM args for N_return-node
 *
 ******************************************************************************/

static node *
MakeFunRetArgs (node *arg_node, info *arg_info)
{
    node *fundef;
    argtab_t *argtab;
    node *ret_exprs;
    node *funargs;
    node *newid;
    node *new_args;
    size_t i;
    unsigned int ret_cnt;
    node *cret_node = NULL;
    node *icm_args = NULL;
    node *last_arg = NULL;

    DBUG_ENTER ();

    fundef = INFO_FUNDEF (arg_info);
    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no fundef node found!");

    argtab = FUNDEF_ARGTAB (fundef);
    DBUG_ASSERT (argtab != NULL, "no argtab found!");

    /* return value */
    DBUG_ASSERT (argtab->ptr_in[0] == NULL, "argtab inconsistent!");
    if (RETURN_CRET (arg_node) != NULL) {
        DBUG_ASSERT (NODE_TYPE (RETURN_CRET (arg_node)) == N_exprs,
                     "no N_exprs node found in RETURN_CRET");
        DBUG_ASSERT (argtab->ptr_out[0] != NULL, "argtab inconsistent!");
        cret_node = DUPdoDupTree (EXPRS_EXPR (RETURN_CRET (arg_node)));
    } else {
        DBUG_ASSERT (argtab->ptr_out[0] == NULL, "argtab or RETURN_CRET inconsistent!");
    }

    /* regular arguments */
    ret_exprs = RETURN_EXPRS (arg_node);
    ret_cnt = 0;
    for (i = 1; i < argtab->size; i++) {
        if (argtab->ptr_out[i] != NULL) {
            DBUG_ASSERT (ret_exprs != NULL, "not enough return values found!");
            if (RETURN_CRET (arg_node) == ret_exprs) {
                ret_exprs = EXPRS_NEXT (ret_exprs);
                DBUG_ASSERT (ret_exprs != NULL, "not enough return values found!");
            }
            DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (ret_exprs)) == N_id,
                         "argument of return-statement must be a N_id node!");

            new_args
              = TBmakeExprs (TCmakeIdCopyString (global.argtag_string[argtab->tag[i]]),
                             TBmakeExprs (MakeArgNode (i,
                                                       TYcopyType (RET_TYPE (argtab->ptr_out[i])),
                                                       FUNDEF_ISTHREADFUN (fundef)),
                                          TBmakeExprs (DUPdupIdNt (
                                                         EXPRS_EXPR (ret_exprs)),
                                                       NULL)));

            if (last_arg == NULL) {
                icm_args = new_args;
            } else {
                EXPRS_NEXT (last_arg) = new_args;
            }
            last_arg = EXPRS_EXPRS3 (new_args);

            ret_exprs = EXPRS_NEXT (ret_exprs);
            ret_cnt++;
        } else {
            DBUG_ASSERT (argtab->ptr_in[i] != NULL, "argtab is uncompressed!");
        }
    }

    /* reference parameters */
    funargs = FUNDEF_ARGS (fundef);
    while (funargs != NULL) {
        if (ARG_ISREFERENCE (funargs)) {
            /*
             * this is not the most elegant way to do it, but I do
             * not want to change too much of the backend here.
             */
            newid = TBmakeId (ARG_AVIS (funargs));

            new_args = TBmakeExprs (TCmakeIdCopyString (global.argtag_string[ATG_inout]),
                                    TBmakeExprs (DUPdupIdNt (newid),
                                                 TBmakeExprs (DUPdupIdNt (newid), NULL)));

            newid = FREEdoFreeNode (newid);

            if (last_arg == NULL) {
                icm_args = new_args;
            } else {
                EXPRS_NEXT (last_arg) = new_args;
            }
            last_arg = EXPRS_EXPRS3 (new_args);

            ret_cnt++;
        }

        funargs = ARG_NEXT (funargs);
    }

    icm_args = TBmakeExprs (TBmakeNumuint (ret_cnt), icm_args);

    if (cret_node == NULL) {
        icm_args = TBmakeExprs (TCmakeIdCopyString (NULL), icm_args);
    } else {
        icm_args = TBmakeExprs (cret_node, icm_args);
    }

    DBUG_RETURN (icm_args);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeFunRetArgsSpmd( node *arg_node, info *arg_info)
 *
 * @brief  Generates ICMs for N_return-node found in body of a SPMD-function.
 *
 ******************************************************************************/

static node *
MakeFunRetArgsSpmd (node *arg_node, info *arg_info)
{
    node *fundef;
    argtab_t *argtab;
    node *ret_exprs;
    node *new_args;
    int ret_cnt;
    size_t i;
    node *icm_args = NULL;
    node *last_arg = NULL;
    node *vardecs;
    ntype *type;
    node *val_nt;
    node *foldfun_tag;
    node *foldfun_name;

    DBUG_ENTER ();

    fundef = INFO_FUNDEF (arg_info);
    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no fundef node found!");

    argtab = FUNDEF_ARGTAB (fundef);
    DBUG_ASSERT (argtab != NULL, "no argtab found!");

    /* regular arguments */
    ret_exprs = RETURN_EXPRS (arg_node);
    ret_cnt = 0;
    vardecs = FUNDEF_VARDECS (INFO_FUNDEF (arg_info));

    for (i = 0; i < argtab->size; i++) {
        if (argtab->ptr_out[i] != NULL) {
            node *foldfun;

            DBUG_ASSERT (ret_exprs != NULL, "not enough return values found!");
            DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (ret_exprs)) == N_id,
                         "no N_id node found!");

            foldfun = (node *)LUTsearchInLutPp (INFO_FOLDLUT (arg_info),
                                                ID_AVIS (EXPRS_EXPR (ret_exprs)));

            if (foldfun == ID_AVIS (EXPRS_EXPR (ret_exprs))) {
                foldfun = NULL;
            }

            DBUG_ASSERT ((foldfun == NULL) || (NODE_TYPE (foldfun) == N_fundef),
                         "Wrong fold function detected");

            type = ID_NTYPE (EXPRS_EXPR (ret_exprs));

            DBUG_ASSERT (vardecs != NULL, "Too few vardecs in SPMD function");

            val_nt = TBmakeId (VARDEC_AVIS (vardecs));
            ID_NT_TAG (val_nt) = NTUcreateNtTagFromNType (ID_NAME (val_nt), VARDEC_NTYPE (vardecs));
            vardecs = VARDEC_NEXT (vardecs);

            if (foldfun == NULL) {
                foldfun_tag = TCmakeIdCopyString ("ND");
                foldfun_name = TCmakeIdCopyString ("NONE");
            } else {
                foldfun_name = TCmakeIdCopyString (FUNDEF_NAME (foldfun));
                if (FUNDEF_ISMTFUN (foldfun)) {
                    foldfun_tag = TCmakeIdCopyString ("MT");
                } else if (FUNDEF_ISXTFUN (foldfun)) {
                    foldfun_tag = TCmakeIdCopyString ("XT");
                } else {
                    foldfun_tag = TCmakeIdCopyString ("ND");
                }
            }

            new_args = TBmakeExprs (
              TCmakeIdCopyString (global.argtag_string[argtab->tag[i]]),
              TBmakeExprs (
                DUPdupIdNt (EXPRS_EXPR (ret_exprs)),
                TBmakeExprs (val_nt, TBmakeExprs (MakeBasetypeArg (type),
                                                  TBmakeExprs (foldfun_tag,
                                                               TBmakeExprs (foldfun_name,
                                                                            NULL))))));

            if (last_arg == NULL) {
                icm_args = new_args;
            } else {
                EXPRS_NEXT (last_arg) = new_args;
            }
            last_arg = EXPRS_EXPRS6 (new_args);

            ret_exprs = EXPRS_NEXT (ret_exprs);
            ret_cnt++;
        } else {
            DBUG_ASSERT (((i == 0) || (argtab->ptr_in[i] != NULL)),
                         "argtab is uncompressed!");
        }
    }

    icm_args = TBmakeExprs (TCmakeIdCopyString (FUNDEF_NAME (INFO_FUNDEF (arg_info))),
                            TBmakeExprs (TBmakeNumuint (ret_cnt), icm_args));

    DBUG_RETURN (icm_args);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPreturn( node *arg_node, info *arg_info)
 *
 * @brief  Generates ICMs for N_return of a function.
 *
 ******************************************************************************/

node *
COMPreturn (node *arg_node, info *arg_info)
{
    node *fundef, *icm;

    DBUG_ENTER ();

    fundef = INFO_FUNDEF (arg_info);

    if (FUNDEF_ISSPMDFUN (fundef) || FUNDEF_ISXTSPMDFUN (fundef)) {
        icm = TBmakeIcm ("MT_SPMDFUN_RET", MakeFunRetArgsSpmd (arg_node, arg_info));
    } else if (FUNDEF_ISMTFUN (fundef) || FUNDEF_ISXTFUN (fundef)) {
        icm = TBmakeIcm ("MT_MTFUN_RET", MakeFunRetArgs (arg_node, arg_info));
    } else if (FUNDEF_ISTHREADFUN (fundef)) {
        icm = TBmakeIcm ("MUTC_THREADFUN_RET", MakeFunRetArgs (arg_node, arg_info));
    } else if (FUNDEF_ISCUDAGLOBALFUN (fundef) || FUNDEF_ISCUDASTGLOBALFUN (fundef)) {
        icm = TBmakeIcm ("CUDA_GLOBALFUN_RET", MakeFunRetArgs (arg_node, arg_info));
    } else {
        icm = TBmakeIcm ("ND_FUN_RET", MakeFunRetArgs (arg_node, arg_info));
    }

    FUNDEF_RETURN (fundef) = icm;

    arg_node = TBmakeAssign (icm, NULL);

    if (INFO_POSTFUN (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_POSTFUN (arg_info), arg_node);
        INFO_POSTFUN (arg_info) = NULL;
    }

    if (FUNDEF_CONTAINSSPAWN (fundef)) {
        arg_node = TCmakeAssignIcm0 ("FP_SAVE_RESULT", arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPlet( node *arg_node, info *arg_info)
 *
 * @brief  Compiles a N_let node.
 *         The return value is a RHS expression or a N_assign chain of ICMs.
 *         In the latter case the old 'arg_node' is removed.
 *
 ******************************************************************************/

node *
COMPlet (node *arg_node, info *arg_info)
{
    node *expr;
    node *ret_node;

    DBUG_ENTER ();

    INFO_LASTIDS (arg_info) = LET_IDS (arg_node);
    INFO_LET (arg_info) = arg_node;

    DBUG_PRINT ("      compiling %s, ... = \n", (LET_IDS (arg_node) != NULL ?
                                      IDS_NAME (LET_IDS (arg_node)) : "(void)"));
    expr = TRAVdo (LET_EXPR (arg_node), arg_info);

    /*
     * 'expr' is a RHS expression or a N_assign chain !!
     */

    if (expr == NULL) {
        /* Nothing left return NOOP */
        ret_node = TCmakeIcm0 ("NOOP");
        arg_node = FREEdoFreeTree (arg_node);
    } else if (NODE_TYPE (expr) == N_assign) {
        /*
         * 'expr' is a N_assign chain
         *  -> return this N_assign chain
         *  -> remove old 'arg_node'
         */
        ret_node = expr;
        arg_node = FREEdoFreeTree (arg_node);
    } else {
        /*
         * 'expr' is a RHS expression
         */
        LET_EXPR (arg_node) = expr;
        ret_node = arg_node;
    }

    INFO_LASTIDS (arg_info) = NULL;

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPApIds( node *ap, info *arg_info)
 *
 * @brief  Traverses ids on LHS of application.
 *
 ******************************************************************************/

static node *
COMPApIds (node *ap, info *arg_info)
{
    argtab_t *argtab;
    argtag_t tag;
    node *let_ids;
    size_t i;
    node *ret_node = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (ap) == N_ap, "no N_ap node found!");

    argtab = AP_ARGTAB (ap);
    DBUG_ASSERT (argtab != NULL, "no argtab found!");

    /*
     * decrement after check for > 0, safe method for reverse loop ending on 0
     * i : (size - 1) to 0
     */
    for (i = argtab->size; i-- > 0;) {
        if (argtab->ptr_out[i] != NULL) {
            let_ids = argtab->ptr_out[i];
            tag = argtab->tag[i];
            DBUG_ASSERT (((global.argtag_is_out[tag]) || (global.argtag_is_inout[tag])),
                         "illegal tag found!");

            if (global.argtag_is_out[tag]) { /* it is an out- (but no inout-) parameter */
                if (!global.argtag_has_rc[tag]) {
                    /* function does no refcounting */
                    ret_node = MakeSetRcIcm (IDS_NAME (let_ids), IDS_NTYPE (let_ids), 1,
                                             ret_node);
                }
            }

            ret_node
              = TCmakeAssignIcm1 ("ND_REFRESH__MIRROR",
                                  MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                                FALSE, TRUE, FALSE, NULL),
                                  ret_node);

            if (global.argtag_is_out[tag]) { /* it is an out- (but no inout-) parameter */
                if (!global.argtag_has_shp[tag]) {
                    /* function sets no shape information */
                    shape_class_t sc = NTUgetShapeClassFromNType (
                      IDS_NTYPE (((node *)argtab->ptr_out[i])));
                    DBUG_ASSERT (sc != C_unknowns, "illegal data class found!");
                    if ((sc == C_akd) || (sc == C_aud)) {
                        CTIabort (LINE_TO_LOC (global.linenum),
                                  "Return value with undefined shape/dimension found."
                                  " Non-AKS return values in external functions are "
                                  "only allowed when the corresponding refcounting"
                                  " pragma is set.");
                    }
                }

                if (!global.argtag_has_desc[tag]) {
                    /* function uses no descriptor at all */
                    ret_node
                      = MakeAllocDescIcm (IDS_NAME (let_ids), IDS_NTYPE (let_ids), 1,
                                          /* dim should be statically known: */
                                          NULL, ret_node);
                }
            }
        }
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPApArgs( node *ap, info *arg_info)
 *
 * @brief  ...
 *
 ******************************************************************************/

static node *
COMPApArgs (node *ap, info *arg_info)
{
    argtab_t *argtab;
    argtag_t tag;
    size_t i;
    node *ret_node = NULL;

    DBUG_ENTER ();

    argtab = AP_ARGTAB (ap);
    DBUG_ASSERT (argtab != NULL, "no argtab found!");
    /*
     * decrement after check for > 0, safe method for reverse loop ending on 0
     * i : (size - 1) to 0
     */
    for (i = argtab->size; i-- > 0;) {
        if (argtab->ptr_in[i] != NULL) {
            DBUG_ASSERT (NODE_TYPE (argtab->ptr_in[i]) == N_exprs,
                         "no N_exprs node found in argtab");
            tag = argtab->tag[i];

            DBUG_ASSERT ((global.argtag_is_in[tag] || global.argtag_is_inout[tag]),
                         "illegal tag found!");
        }
    }

    DBUG_RETURN (ret_node);
}

static node *
AddDescArgs (node *ops, node *args)
{
    DBUG_ENTER ();

    if (ops != NULL) {
        if (WITHOP_SUB (ops) != NULL) {
            shape_class_t shapeClass
              = NTUgetShapeClassFromNType (ID_NTYPE (WITHOP_SUB (ops)));
            if (shapeClass == C_akd || shapeClass == C_aud) {
                node *newArg
                  = TBmakeExprs (TCmakeIdCopyString ("in_justdesc"),
                                 TBmakeExprs (TCmakeIdCopyString ("int"),
                                              TBmakeExprs (DUPdupIdNt (WITHOP_SUB (ops)),
                                                           NULL)));
                args = TCappendExprs (args, newArg);
                NUM_VAL (EXPRS_EXPR3 (args)) += 1;
            }
        }
        args = AddDescArgs (WITHOP_NEXT (ops), args);
    }

    DBUG_RETURN (args);
}

/** <!--********************************************************************-->
 *
 * @fn  int rank(int64_t y, struct smart_decision_t ** X, int n)
 *
 * @brief  Sequential algorithm that computes the rank of y in X, where n = |X|.
 *
 * @param y: integer y that is used to compute the rank for.
 * @param X: integer pointer to an sorted integer array.
 * @param n: integer which is the length of X.
 *
 ******************************************************************************/
int
rank (int64_t y, struct smart_decision_t **X, int n)
{
    DBUG_ENTER ();

    int idx = n / 2;
    if (n == 0) {
        DBUG_RETURN (0);
    }
    while (n > 1) {
        if (X[idx]->problem_size <= y) {
            n -= n / 2;
            idx += n / 2;
        } else {
            n /= 2;
            idx -= n - n / 2;
        }
    }

    DBUG_RETURN (X[idx]->problem_size < y ? idx + 1 : idx);
}

/** <!--********************************************************************-->
 *
 * @fn  struct smart_decision_t * create_smart_decision_data( info * info )
 *
 * @brief  this function creates a data structure that is used to analyze
 *         the performance profile stored in the database file that was
 *         created by the smart decision tool. For more information see
 *         mt_smart.c.
 *
 * @param info: arg_info node.
 *
 ******************************************************************************/
struct smart_decision_t *
create_smart_decision_data (info *info)
{
    DBUG_ENTER ();

    struct smart_decision_t *data = MEMmalloc (1 * sizeof (struct smart_decision_t));

    data->max_time = 0.0;
    data->min_time = 0.0;
    data->problem_size = 0;
    data->nr_measurements = MEMmalloc (INFO_NR_THREADS (info) * sizeof (int64_t));
    data->cum_time = MEMmalloc (INFO_NR_THREADS (info) * sizeof (int64_t));
    data->fun_time = MEMmalloc (INFO_NR_THREADS (info) * sizeof (float));

    for (int i = 0; i < INFO_NR_THREADS (info); i++) {
        data->nr_measurements[i] = 0;
        data->cum_time[i] = 0;
        data->fun_time[i] = 0.0;
    }

    DBUG_RETURN (data);
}

/** <!--********************************************************************-->
 *
 * @fn  void destroy_smart_decision_data(struct smart_decision_t * data)
 *
 * @brief  this function frees the memory that was used by the data structure
 *         that was created by the create_smart_decision_data function.
 *
 * @param data: pointer to the data structure.
 *
 ******************************************************************************/
void
destroy_smart_decision_data (struct smart_decision_t *data)
{
    DBUG_ENTER ();

    MEMfree (data->nr_measurements);
    MEMfree (data->cum_time);
    MEMfree (data->fun_time);
    MEMfree (data);

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn  static int * COMPdoDecideSmart( info *info, int spmd_id)
 *
 * @brief  This function computes a recommendation table for each with loop
 *         based on the performance profile stored in the database files that
 *         where created by the smart decision tool during the training phase.
 *         The recommendation file is needed on runtime during the decision
 *         phase. The COMPap function compiles the recommendation table in the
 *         SAC code for use on runtime by using a series of ICM's. For more
 *         information see the COMPap function and mt_smart.c.
 *
 * @param info: arg_info node.
 * @param spmd_id: unique identifier for the current with loop.
 *
 ******************************************************************************/
static int64_t *
COMPdoDecideSmart (info *info, int spmd_id)
{
    DBUG_ENTER ();

    bool moved = false;
    int nr_measurements = 0;
    int measurements_array_size = 128;
    int idx;
    FILE *fp;
    int64_t *line;
    struct smart_decision_t **measurements;
    int64_t *recommendations;
    float *y;
    float *reg;
    float **X;

    float slope, angle;
    float pX, pY, diff;
    const float t_angle = (float)global.mt_smart_gradient * ((float)M_PI / 180.0f);

    // Handle trivial cases
    if (global.mt_smart_gradient < 0 || global.mt_smart_gradient > 90) {
        CTIabort (EMPTY_LOC, "The gradient must be a value between 0 and 90 degrees.");
    } else if (global.mt_smart_gradient == 90) {
        DBUG_PRINT ("SAC will use 1 thread to compute SPMD function %i.\n", spmd_id);
        recommendations = MEMmalloc (3 * sizeof (int));
        recommendations[0] = 1; // one recommendation
        recommendations[1] = 0; // for any problem size of 1 or larger
        recommendations[2] = 1; // use 1 thread
        DBUG_RETURN (recommendations);
    }

    measurements
      = MEMmalloc (measurements_array_size * sizeof (struct smart_decision_t *));

    // Combine and sort the smart decision profiles from the candidate database files that
    // where found by the COMPdoPrepareSmart function. The combined profiles from the
    // candidate database files are stored in the smart_decision_t structs. These structs
    // are designed to make it easy to analyse the profiles. Complexity: O(n^2) + O(log
    // n!) = O(n^2) Start by looping over all candidate database files.
    for (int i = 0; i < INFO_NR_FILES (info); i++) {
        line = MEMmalloc (INFO_LINE_COUNT (info)[i] * sizeof (int64_t));
        fp = INFO_FP_LIST (info)[i];
        rewind (fp);
        // Read all lines in the database
        while (fread (line, sizeof (int64_t), INFO_LINE_COUNT (info)[i], fp)
               == (unsigned)INFO_LINE_COUNT (info)[i]) {
            // We are only interested in those lines that contain information about the
            // current with loop, because we want to construct the recommendation table
            // for the current with loop.
            if (line[0] == spmd_id) {
                // make sure that there is enough memory available to store the next
                // profile
                if (nr_measurements >= measurements_array_size) {
                    measurements_array_size *= 2;
                    MEMrealloc (measurements, measurements_array_size
                                                * sizeof (struct smart_decision_t *));
                }
                // find the memory location for the current profile
                idx = rank (line[1], measurements, nr_measurements);
                // check if profiles can be combined, if not create some space for a new
                // profile.
                if (idx < nr_measurements && measurements[idx]->problem_size != line[1]) {
                    memmove (&measurements[idx + 1], &measurements[idx],
                             (size_t)(nr_measurements - idx)
                               * sizeof (struct smart_decision_t *));
                    moved = true;
                }
                // create a new profile
                if (idx == nr_measurements || moved == true) {
                    measurements[idx] = create_smart_decision_data (info);
                    measurements[idx]->problem_size = line[1];
                    measurements[idx]->max_time = 0.0;
                    measurements[idx]->min_time = INFINITY;
                    nr_measurements++;
                    moved = false;
                }
                // combine profiles
                for (size_t j = 0; j < INFO_LINE_COUNT (info)[i] - 3; j++) {
                    measurements[idx]->nr_measurements[j] += line[2];
                    measurements[idx]->cum_time[j] += line[j + 3];
                    measurements[idx]->fun_time[j]
                      = (float)measurements[idx]->cum_time[j]
                        / (float)measurements[idx]->nr_measurements[j];
                    if (measurements[idx]->fun_time[j] > measurements[idx]->max_time) {
                        measurements[idx]->max_time = measurements[idx]->fun_time[j];
                    }
                    if (measurements[idx]->fun_time[j] < measurements[idx]->min_time) {
                        measurements[idx]->min_time = measurements[idx]->fun_time[j];
                    }
                }
            }
        }
        MEMfree (line);
    }

    // Handle case when there are no measurements
    if (nr_measurements == 0) {
        DBUG_PRINT ("SAC will use all threads to compute SPMD function %i.\n", spmd_id);
        recommendations = (int64_t *)MEMmalloc (3 * sizeof (int));
        recommendations[0] = 1; // one recommendation
        recommendations[1] = 0; // for any problem size of 0 or larger
        recommendations[2] = 0; // use all threads
    }
    // Compute recommendation table when there are 1 or more measurements
    else {
        DBUG_PRINT ("SAC will use a recommendation table to decide how many threads "
                    "should be used to compute SPMD function %i.\n",
                    spmd_id);
        recommendations = (int64_t *)MEMmalloc (sizeof (int64_t)* (2 * (size_t)nr_measurements + 1) );
        recommendations[0]
          = nr_measurements; // N recommendations (N is defined by 'nr_measurements')

        // Convert sorted decision data in a recommendation table
        for (int i = 0; i < nr_measurements; i++) {
            recommendations[2 * i + 1]
              = measurements[i]->problem_size; // for problem size x (x is defined by
                                               // 'measurements[i]->problem_size')
            recommendations[2 * (i + 1)] = 0;

            if (INFO_NR_THREADS (info) <= 4) {
                for (int j = 1; j < INFO_NR_THREADS (info); j++) {
                    diff = measurements[i]->max_time - measurements[i]->min_time;
                    pX
                      = (measurements[i]->max_time - measurements[i]->fun_time[j]) / diff;
                    pY = (measurements[i]->max_time - measurements[i]->fun_time[j - 1])
                         / diff;
                    slope = pX - pY;
                    angle = atanf (slope);
                    if (angle <= t_angle || angle <= 0) {
                        recommendations[2 * (i + 1)] = j; // use j threads
                        break;
                    }
                }
            }
            // 4th order polynomial regression
            // this makes the recommendation process less sensitive for outliers
            else {
                reg = MEMmalloc (4 * sizeof (float));
                y = MEMmalloc (INFO_NR_THREADS (info) * sizeof (float));
                X = Matrix (INFO_NR_THREADS (info), 4);

                // collect data for regression
                for (int g = 0; g < INFO_NR_THREADS (info); g++) {
                    float jj = (float)(g + 1);
                    diff = measurements[i]->max_time - measurements[i]->min_time;
                    X[g][0] = 1.0;
                    X[g][1] = jj;
                    X[g][2] = X[g][1] * jj;
                    X[g][3] = X[g][2] * jj;
                    y[g]
                      = (measurements[i]->max_time - measurements[i]->fun_time[g]) / diff;

                }

                // apply regression
                PolyRegression (X, INFO_NR_THREADS (info), 4, y, reg);

                // do recommendation
                for (int g = 1; g < INFO_NR_THREADS (info); g++) {
                    float j = (float)g;
                    float jj = (float)(g - 1);
                    pX = reg[0] + j * reg[1] + j * j * reg[2] + j * j * j * reg[3];
                    pY = reg[0] + jj * reg[1] + jj * jj * reg[2]
                         + jj * jj * jj * reg[3];
                    slope = pX - pY;
                    angle = atanf (slope);
                    if (angle <= t_angle || angle <= 0) {
                        recommendations[2 * (i + 1)] = g; // use j threads
                        break;
                    }
                }

                MEMfree (reg);
                MEMfree (y);
                DelMatrix (X, INFO_NR_THREADS (info), 4);
            }
        }
    }

    // Clean-up phase
    for (int i = 0; i < nr_measurements; i++) {
        destroy_smart_decision_data (measurements[i]);
    }
    MEMfree (measurements);

    DBUG_RETURN (recommendations);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPap( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_ap node.
 *   Creates an ICM for function application and insert ICMs to decrement the
 *   RC of function arguments. (The return value is a N_assign chain of ICMs.)
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   The flattening phase assures that no one of the arguments occurs on the LHS
 *   of the application (a = fun(a) is impossible).
 *
 *   INFO_LASTIDS contains pointer to previous let-ids.
 *
 ******************************************************************************/

node *
COMPap (node *arg_node, info *arg_info)
{
    node *icm, *icm_conf = NULL, *icm_args, *icm_pre = NULL, *icm_post = NULL;
    node *fundef, *with2, *cond, *seg, *array_inf, *array_sup;
    node *assigns, *assigns1 = NULL, *assigns2 = NULL, *assigns4 = NULL;

    node **icm_data = NULL, **icm_conf_expr = NULL;
    int64_t *recommendations;
    bool fundef_in_current_namespace;
    size_t data_size = 0;
    int dims, seg_dim, op_offset, op, nr_segs, idx = 0;

    static int spmdfun_count = 0;

    DBUG_ENTER ();

    fundef = AP_FUNDEF (arg_node);

    fundef_in_current_namespace
      = STReq (NSgetModule (FUNDEF_NS (fundef)),
               NSgetModule (MODULE_NAMESPACE (INFO_MODUL (arg_info))));

    DBUG_ASSERT (CheckAp (arg_node, arg_info),
                 "application of a user-defined function without own"
                 " refcounting: refcounted argument occurs also on LHS!");

    /*
     * traverse ids on LHS of application
     */
    if (!(FUNDEF_ISTHREADFUN (fundef) && AP_ISSPAWNED (arg_node))) {
        /* Do update results ect, as this should be done after sync */
        assigns1 = COMPApIds (arg_node, arg_info);
    }

    /*
     * traverse arguments of application
     */
    assigns2 = COMPApArgs (arg_node, arg_info);

    assigns = TCappendAssign (assigns1, assigns2);

    icm_args = MakeFunApArgs (arg_node, arg_info);

    if (FUNDEF_ISSPMDFUN (fundef)) {
        if (global.mt_smart_mode > 0) {

            spmdfun_count++;

            assigns4 = BLOCK_ASSIGNS (FUNDEF_BODY (fundef));

            while (assigns4 != NULL
                   && (NODE_TYPE (ASSIGN_STMT (assigns4)) != N_let
                       || NODE_TYPE (LET_EXPR (ASSIGN_STMT (assigns4))) != N_with2)) {
                assigns4 = ASSIGN_NEXT (assigns4);
            }

            DBUG_ASSERT (assigns4 != NULL,
                         "A SPMD function without a with loop in the body was detected!");

            with2 = LET_EXPR (ASSIGN_STMT (assigns4));

            cond = INFO_WITH2_COND (arg_info);

            if (WITH2_SIZE (with2) < 0 && cond != NULL && COND_ELSE (cond) != NULL) {
                assigns4 = BLOCK_ASSIGNS (COND_ELSE (cond));

                while (assigns4 != NULL
                       && (NODE_TYPE (ASSIGN_STMT (assigns4)) != N_let
                           || NODE_TYPE (LET_EXPR (ASSIGN_STMT (assigns4))) != N_with2)) {
                    assigns4 = ASSIGN_NEXT (assigns4);
                }

                if (assigns4 != NULL) {
                    with2 = LET_EXPR (ASSIGN_STMT (assigns4));
                }

                INFO_WITH2_COND (arg_info) = NULL;
            }

            if (WITH2_SIZE (with2) > -1) {
                icm_conf
                  = TCmakeIcm1 ("MT_SMART_PROBLEM_SIZE", TBmakeNum (WITH2_SIZE (with2)));
            } else if (WITH2_MEMID (with2) != NULL) {
                icm_conf = TCmakeIcm1 ("MT_SMART_VAR_PROBLEM_SIZE",
                                       DUPdupNodeNt (WITH2_MEMID (with2)));
            } else if (WITH2_SEGS (with2) != NULL) {
                seg = WITH2_SEGS (with2);
                nr_segs = 0;
                op_offset = -2;

                while (seg != NULL) {
                    seg = WLSEG_NEXT (seg);
                    nr_segs++;
                }

                idx = 0;
                seg = WITH2_SEGS (with2);
                dims = WITH2_DIMS (with2);
                icm_conf_expr = MEMmalloc ((nr_segs * dims + 2) * sizeof (node *));
                icm_conf_expr[idx++]
                  = TBmakeIcm ("MT_SMART_EXPR_PROBLEM_SIZE_BEGIN", NULL);

                while (seg != NULL) {
                    seg_dim = WLSEG_DIMS (seg);
                    if (seg_dim > dims) {
                        MEMrealloc (icm_conf_expr,
                                    (nr_segs * seg_dim + 2) * sizeof (node *));
                        dims = seg_dim;
                    }
                    if (seg_dim > 0) {
                        array_inf = ARRAY_AELEMS (WLSEG_IDXINF (seg));
                        array_sup = ARRAY_AELEMS (WLSEG_IDXSUP (seg));

                        for (int i = 0; i < seg_dim; i++) {
                            op = op_offset + (i == 0 ? 2 : 1);
                            DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (array_inf)) == N_num
                                           || NODE_TYPE (EXPRS_EXPR (array_inf)) == N_id,
                                         "The array IdxInf of N_wlseg does contain an "
                                         "attribute that is not a N_num neither a N_id!");
                            DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (array_sup)) == N_num
                                           || NODE_TYPE (EXPRS_EXPR (array_sup)) == N_id,
                                         "The array IdxSup of N_wlseg does contain an "
                                         "attribute that is not a N_num neither a N_id!");
                            if (NODE_TYPE (EXPRS_EXPR (array_inf)) == N_num
                                && NODE_TYPE (EXPRS_EXPR (array_sup)) == N_num) {
                                icm_conf_expr[idx++]
                                  = TCmakeIcm3 ("MT_SMART_EXPR_PROBLEM_SIZE_IxI",
                                                TBmakeNum (
                                                  NUM_VAL (EXPRS_EXPR (array_inf))),
                                                TBmakeNum (
                                                  NUM_VAL (EXPRS_EXPR (array_sup))),
                                                TBmakeNum (op));
                            } else if (NODE_TYPE (EXPRS_EXPR (array_inf)) == N_num
                                       && NODE_TYPE (EXPRS_EXPR (array_sup)) == N_id) {
                                icm_conf_expr[idx++]
                                  = TCmakeIcm3 ("MT_SMART_EXPR_PROBLEM_SIZE_IxC",
                                                TBmakeNum (
                                                  NUM_VAL (EXPRS_EXPR (array_inf))),
                                                DUPdupNodeNt (EXPRS_EXPR (array_sup)),
                                                TBmakeNum (op));
                            } else if (NODE_TYPE (EXPRS_EXPR (array_inf)) == N_id
                                       && NODE_TYPE (EXPRS_EXPR (array_sup)) == N_num) {
                                icm_conf_expr[idx++]
                                  = TCmakeIcm3 ("MT_SMART_EXPR_PROBLEM_SIZE_CxI",
                                                DUPdupNodeNt (EXPRS_EXPR (array_inf)),
                                                TBmakeNum (
                                                  NUM_VAL (EXPRS_EXPR (array_sup))),
                                                TBmakeNum (op));
                            } else {
                                icm_conf_expr[idx++]
                                  = TCmakeIcm3 ("MT_SMART_EXPR_PROBLEM_SIZE_CxC",
                                                DUPdupNodeNt (EXPRS_EXPR (array_inf)),
                                                DUPdupNodeNt (EXPRS_EXPR (array_sup)),
                                                TBmakeNum (op));
                            }
                            array_inf = EXPRS_NEXT (array_inf);
                            array_sup = EXPRS_NEXT (array_sup);
                            op_offset = 0;
                        }
                    }
                    seg = WLSEG_NEXT (seg);
                }

                icm_conf_expr[idx++]
                  = TCmakeIcm1 ("MT_SMART_EXPR_PROBLEM_SIZE_END", TBmakeNum (op_offset));
            } else {
                icm_conf = TCmakeIcm1 ("MT_SMART_PROBLEM_SIZE", TBmakeNum (0));
            }

            // compute and compile recommendation table in SAC code if one has to compile
            // for the decision phase.
            if (global.mt_smart_mode == 2) {
                // compute recommendation table
                recommendations = COMPdoDecideSmart (arg_info, spmdfun_count);
                // create set of ICM's to compile the recommendation tabel in the SAC
                // code.
                data_size = (size_t) recommendations[0]; //used in malloc therefore can't be negative
                icm_data = MEMmalloc ((data_size + 2) * sizeof (node *));
                icm_data[0] = TCmakeIcm1 ("MT_SMART_DATA_BEGIN", TBmakeNum (data_size));
                for (size_t i = 0; i < data_size; i++) {
                    icm_data[i + 1]
                      = TCmakeIcm2 ("MT_SMART_DATA_ADD",
                                    TBmakeNum (recommendations[2 * i + 1]),
                                    TBmakeNum (recommendations[2 * (i + 1)]));
                }
                icm_data[data_size + 1] = TBmakeIcm ("MT_SMART_DATA_END", NULL);
            }
            // create ICM's to compile the calls for the execution of the runtime
            // components of the smart decision tool in the SAC code.
            icm_pre = TCmakeIcm1 ("MT_SMART_BEGIN", TBmakeNum (spmdfun_count));
            icm = TBmakeIcm ("MT_SPMDFUN_AP", icm_args);
            icm_post = TBmakeIcm ("MT_SMART_END", NULL);

        } else {
            icm = TBmakeIcm ("MT_SPMDFUN_AP", icm_args);
        }
    } else if (FUNDEF_ISMTFUN (fundef) || FUNDEF_ISXTFUN (fundef)) {
        /* The interface of the MT and XT funs is the same, hence use the same ICM. */
        icm = TBmakeIcm ("MT_MTFUN_AP", icm_args);
    } else if (FUNDEF_ISTHREADFUN (fundef)) {
        if (AP_ISSPAWNED (arg_node)) {
            const char *place = AP_SPAWNPLACE (arg_node) == NULL
                                  ? (AP_ISREMOTE (arg_node) ? "" : "PLACE_LOCAL")
                                  : AP_SPAWNPLACE (arg_node);
            icm_args = TBmakeExprs (TCmakeIdCopyString (place), icm_args);
            icm_args = TBmakeExprs (DUPdupIdsIdNt (INFO_LASTIDS (arg_info)), icm_args);
            icm = TBmakeIcm ("MUTC_SPAWNFUN_AP", icm_args);
        } else if (!FUNDEF_WASWITH3BODY (fundef)) {
            icm = TBmakeIcm ("MUTC_FUNTHREADFUN_AP", icm_args);
        } else {
            if (global.mutc_suballoc_desc_one_level_up) {
                icm_args = AddDescArgs (INFO_WITHOPS (arg_info), icm_args);
            }
            icm = TBmakeIcm ("MUTC_THREADFUN_AP", icm_args);
        }
    } else if (FUNDEF_ISCUDAGLOBALFUN (fundef)) {
        icm = TBmakeIcm ("CUDA_GLOBALFUN_AP", icm_args);
    } else if (FUNDEF_ISCUDASTGLOBALFUN (fundef)) {
        icm = TBmakeIcm ("CUDA_ST_GLOBALFUN_AP", icm_args);
    } else if (FUNDEF_ISINDIRECTWRAPPERFUN (fundef)) {
        icm = TBmakeIcm ("WE_FUN_AP", icm_args);
    } else if (FUNDEF_RTSPECID (fundef) != NULL && global.config.rtspec
               && ((fundef_in_current_namespace && FUNDEF_ISEXPORTED (fundef))
                   || !fundef_in_current_namespace)) {
        icm_args
          = TBmakeExprs (TCmakeIdCopyString (NSgetModule (FUNDEF_NS (fundef))), icm_args);
        icm = TBmakeIcm ("RTSPEC_FUN_AP", icm_args);
    } else if (global.backend == BE_distmem && AP_DISTMEMHASSIDEEFFECTS (arg_node)) {
        /* This function application has side effects. We have to treat it in a special
         * way. */

        /* Append return type and return NT to ICM arguments. */
        if (AP_ARGTAB (arg_node)->ptr_out[0] == NULL) {
            icm_args = TBmakeExprs (TCmakeIdCopyString (NULL), icm_args);
            icm_args = TBmakeExprs (TCmakeIdCopyString (NULL), icm_args);
        } else {
            icm_args
              = TBmakeExprs (DUPdupIdsIdNt (AP_ARGTAB (arg_node)->ptr_out[0]), icm_args);
            icm_args = TBmakeExprs (TCmakeIdCopyString (GetBaseTypeFromExpr (
                                      AP_ARGTAB (arg_node)->ptr_out[0])),
                                    icm_args);
        }

        /* Append NT of out arguments to ICM arguments. */
        for (size_t i = AP_ARGTAB (arg_node)->size - 1; i >= 1; i--) {
            if (AP_ARGTAB (arg_node)->ptr_out[i] == NULL) {
                /* in/inout argument, append NT. */
                if (NODE_TYPE (EXPRS_EXPR (AP_ARGTAB (arg_node)->ptr_in[i])) == N_id) {
                    icm_args = TBmakeExprs (DUPdupIdNt (EXPRS_EXPR (
                                              AP_ARGTAB (arg_node)->ptr_in[i])),
                                            icm_args);
                } else { /* N_globobj */
                    icm_args = TBmakeExprs (DUPdoDupNode (EXPRS_EXPR (
                                              AP_ARGTAB (arg_node)->ptr_in[i])),
                                            icm_args);
                }
            } else {
                /* out argument, append NT. */
                icm_args = TBmakeExprs (DUPdupIdsIdNt (AP_ARGTAB (arg_node)->ptr_out[i]),
                                        icm_args);
            }
        }

        icm_args = TBmakeExprs (TBmakeNumuint (AP_ARGTAB (arg_node)->size - 1), icm_args);

        icm = TBmakeIcm ("ND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS", icm_args);
    } else {
        icm = TBmakeIcm ("ND_FUN_AP", icm_args);
    }

    // If called function contains spawn, check result
    if (AP_TOSPAWN (arg_node)) {
        assigns = TCmakeAssignIcm1 ("FP_FUN_AP_CHECK", TCmakeIdCopyString (TRAVtmpVar ()),
                                    assigns);
    }

    // If ap is spaned, wrap the ap in spawn start and end icm's
    if (AP_ISSPAWNED (arg_node) && (global.backend != BE_mutc)) {
        arg_node
          = TCappendAssign (MakeFPAp (INFO_LET (arg_info), icm, arg_info), assigns);
    } else if (AP_ISSPAWNED (arg_node) && AP_ISREMOTE (arg_node)
               && (global.backend == BE_mutc)) {
        if (AP_ARGS (arg_node) != NULL) {
            assigns = TBmakeAssign (icm, assigns);
            icm = TCmakeIcm1 ("SAC_MUTC_RC_BARRIER", DUPdupIdNt (AP_ARG1 (arg_node)));
        }
        arg_node = TBmakeAssign (icm, assigns);
    } else {
        if (FUNDEF_ISSPMDFUN (fundef) && global.mt_smart_mode > 0) {
            arg_node
              = TBmakeAssign (icm_pre,
                              TBmakeAssign (icm, TCappendAssign (assigns,
                                                                 TBmakeAssign (icm_post,
                                                                               NULL))));
            if (global.mt_smart_mode == 2) {
                /*
                 * decrement after check for > 0, safe method for reverse loop ending on 0
                 * i : (data_size + 1) to 0
                 */
                for (size_t i = data_size + 2; i-- > 0;) {
                    arg_node = TBmakeAssign (icm_data[i], arg_node);
                }
                MEMfree (icm_data);
            }
            if (icm_conf_expr != NULL) {
                for (int i = idx - 1; i >= 0; i--) {
                    arg_node = TBmakeAssign (icm_conf_expr[i], arg_node);
                }
                MEMfree (icm_conf_expr);
            } else {
                arg_node = TBmakeAssign (icm_conf, arg_node);
            }
        }
        /*
         * We return an N_assign chain here rather than an N_ap node.
         * The surrounding COMPlet takes care of this.
         */
        else {
            arg_node = TBmakeAssign (icm, assigns);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPid( node *arg_node, info *arg_info)
 *
 * @brief  Handle ids from two locations:
 *
 *         From in the conditional of a cond or do node
 *
 *         All other ids from the rhs
 *
 ******************************************************************************/

node *
COMPid (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ();

    if (INFO_COND (arg_info)) {
        if (NODE_TYPE (arg_node) == N_id) {
            ret_node
              = TBmakeIcm ("SAC_ND_GETVAR",
                           TBmakeExprs (DUPdupIdNt (arg_node),
                                        TBmakeExprs (DUPdoDupTree (arg_node), NULL)));
            FREEdoFreeTree (arg_node);
        } else {
            ret_node = arg_node;
        }
    } else {
        ret_node = RhsId (arg_node, arg_info);
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeIcm_PRF_TYPE_CONV_AKD( node *let_ids, node *id,
 *                                       char *error)
 *
 * @brief  Produce all of the ICMs needed to implement PRF_TYPE_CONV_AKD
 *
 *****************************************************************************/
static node *
MakeIcm_PRF_TYPE_CONV_AKD (char *error, node *let_ids, node *id)
{
    int i;
    node *ret_node = NULL;

    DBUG_ENTER ();

    ret_node
      = TCmakeAssignIcm3 ("SAC_ND_PRF_TYPE_CONV__AKD_END", TBmakeStr (STRcpy (error)),
                          TCmakeIdCopyStringNtNew (IDS_NAME (let_ids), IDS_NTYPE (let_ids)),
                          TCmakeIdCopyStringNtNew (ID_NAME (id), ID_NTYPE (id)),
                          ret_node);

    for (i = DIM_NO_OFFSET (TUgetFullDimEncoding (IDS_NTYPE (let_ids))) - 1; i >= 0; i--) {
        ret_node = TCmakeAssignIcm3 ("SAC_ND_PRF_TYPE_CONV__AKD_SHAPE", TBmakeNum (i),
                                     TCmakeIdCopyStringNtNew (IDS_NAME (let_ids),
                                                              IDS_NTYPE (let_ids)),
                                     TCmakeIdCopyStringNtNew (ID_NAME (id),
                                                           ID_NTYPE (id)),
                                     ret_node);
    }

    ret_node
      = TCmakeAssignIcm3 ("SAC_ND_PRF_TYPE_CONV__AKD_START", TBmakeStr (STRcpy (error)),
                          TCmakeIdCopyStringNtNew (IDS_NAME (let_ids),
                                                   IDS_NTYPE (let_ids)),
                          TCmakeIdCopyStringNtNew (ID_NAME (id), ID_NTYPE (id)),
                          ret_node);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeIcm_PRF_TYPE_CONV_AKS( node *let_ids, node *id,
 *                                       char *error)
 *
 * @brief  Produce all of the ICMs needed to implement PRF_TYPE_CONV_AKS
 *
 *****************************************************************************/
static node *
MakeIcm_PRF_TYPE_CONV_AKS (char *error, node *let_ids, node *id)
{
    int i;
    node *ret_node = NULL;

    DBUG_ENTER ();

    ret_node
      = TCmakeAssignIcm3 ("SAC_ND_PRF_TYPE_CONV__AKS_END", TBmakeStr (STRcpy (error)),
                          TCmakeIdCopyStringNtNew (IDS_NAME (let_ids), IDS_NTYPE (let_ids)),
                          TCmakeIdCopyStringNtNew (ID_NAME (id), ID_NTYPE (id)),
                          ret_node);

    for (i = DIM_NO_OFFSET (TUgetFullDimEncoding (IDS_NTYPE (let_ids))) - 1; i >= 0; i--) {
        ret_node = TCmakeAssignIcm3 ("SAC_ND_PRF_TYPE_CONV__AKS_COND", TBmakeNum (i),
                                     TCmakeIdCopyStringNtNew (IDS_NAME (let_ids),
                                                              IDS_NTYPE (let_ids)),
                                     TCmakeIdCopyStringNtNew (ID_NAME (id),
                                                              ID_NTYPE (id)),
                                     ret_node);
    }

    ret_node
      = TCmakeAssignIcm3 ("SAC_ND_PRF_TYPE_CONV__AKS_START", TBmakeStr (STRcpy (error)),
                          TCmakeIdCopyStringNtNew (IDS_NAME (let_ids),
                                                   IDS_NTYPE (let_ids)),
                          TCmakeIdCopyStringNtNew (ID_NAME (id), ID_NTYPE (id)),
                          ret_node);

    DBUG_RETURN (ret_node);
}

static node *
COMPprfSecond (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;
    DBUG_ENTER ();
    ret_node
      = TCmakeAssignIcm2 ("SAC_ND_PRF_SECOND", DUPdupIdsIdNt (INFO_LASTIDS (arg_info)),
                          DUPdupIdNt (PRF_ARG2 (arg_node)), ret_node);
    DBUG_RETURN (ret_node);
}

static node *
COMPprfSyncIn (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;
    DBUG_ENTER ();

    if (global.backend == BE_mutc) {
        ret_node = TCmakeAssignIcm1 ("ND_REFRESH__MIRROR",
                                     MakeTypeArgs (IDS_NAME (INFO_LASTIDS (arg_info)),
                                                   IDS_NTYPE (INFO_LASTIDS (arg_info)),
                                                   FALSE, TRUE, FALSE, NULL),
                                     ret_node);

        ret_node = TCmakeAssignIcm2 ("SAC_ND_PRF_SYNCIN",
                                     DUPdupIdsIdNt (INFO_LASTIDS (arg_info)),
                                     DUPdupIdNt (PRF_ARG2 (arg_node)), ret_node);
    } else if (global.backend == BE_cuda || global.backend == BE_cudahybrid) {
        ret_node = TCmakeAssignIcm2 ("SAC_CUDA_PRF_SYNCIN",
                                     DUPdupIdsIdNt (INFO_LASTIDS (arg_info)),
                                     DUPdupIdNt (PRF_ARG2 (arg_node)), ret_node);
    } else {
        DBUG_UNREACHABLE ("syncin is not supported for this backend!");
    }

    DBUG_RETURN (ret_node);
}

static node *
COMPprfSyncOut (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;
    DBUG_ENTER ();

    if (global.backend == BE_mutc) {
        ret_node
          = TCmakeAssignIcm2 ("SAC_ND_PRF_SYNCOUT", DUPdupIdNt (PRF_ARG1 (arg_node)),
                              DUPdupIdNt (PRF_ARG2 (arg_node)), ret_node);
    } else if (global.backend == BE_cuda || global.backend == BE_cudahybrid) {
        ret_node
          = TCmakeAssignIcm2 ("SAC_CUDA_PRF_SYNCOUT", DUPdupIdNt (PRF_ARG2 (arg_node)),
                              DUPdupIdNt (PRF_ARG1 (arg_node)), ret_node);
    } else {
        DBUG_UNREACHABLE ("syncout is not supported for this backend!");
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfTypeConv( node *arg_node, info *arg_info)
 *
 * @brief  Produce an icm to check if a type conversion can be performed
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 *****************************************************************************/
static node *
COMPprfTypeConv (node *arg_node, info *arg_info)
{
    node *ret_node = NULL, *let_ids = NULL;
    char *lhs_type_string = NULL, *rhs_type_string = NULL;
    char *error = NULL;
    const char *fmt = "%s:%d\\nCan not assign %s %s to %s %s type mismatch\\n";
    size_t error_len = 0;
    node *id = NULL;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    id = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node)));

    lhs_type_string = CVtype2String (IDS_NTYPE (let_ids), 0, FALSE);
    rhs_type_string = CVtype2String (ID_NTYPE (id), 0, FALSE);

    error_len = STRlen (fmt) - (2 * 6) + STRlen (NODE_FILE (arg_node)) + STRsizeInt ()
                + STRlen (rhs_type_string) + STRlen (AVIS_NAME (ID_AVIS (id)))
                + STRlen (lhs_type_string) + STRlen (AVIS_NAME (IDS_AVIS (let_ids)));
    error = (char *)MEMmalloc (sizeof (char) * error_len);

    sprintf (error, fmt, NODE_FILE (arg_node), NODE_LINE (arg_node), rhs_type_string,
             AVIS_NAME (ID_AVIS (id)), lhs_type_string, AVIS_NAME (IDS_AVIS (let_ids)));

    if ((SCALAR != TUgetFullDimEncoding (IDS_NTYPE (let_ids)))
        && (KNOWN_SHAPE (TUgetFullDimEncoding (IDS_NTYPE (let_ids)))
            && (global.min_array_rep <= MAR_scl_aks))) {
        /* ASK needs the mirror */
        ret_node = MakeIcm_PRF_TYPE_CONV_AKS (error, let_ids, id);

    } else if (SCALAR != TUgetFullDimEncoding (IDS_NTYPE (let_ids))
               && KNOWN_DIMENSION (TUgetFullDimEncoding (IDS_NTYPE (let_ids)))
               && (global.min_array_rep <= MAR_scl_akd)) {
        /* ASK needs the mirror */
        ret_node = MakeIcm_PRF_TYPE_CONV_AKD (error, let_ids, id);

    } else {
        ret_node = TCmakeAssignIcm3 ("SAC_ND_PRF_TYPE_CONV", TBmakeStr (STRcpy (error)),
                                     TCmakeIdCopyStringNtNew (AVIS_NAME (IDS_AVIS (let_ids)),
                                                           IDS_NTYPE (let_ids)),
                                     TCmakeIdCopyStringNtNew (AVIS_NAME (ID_AVIS (id)),
                                                           ID_NTYPE (id)),
                                     NULL);
    }
    MEMfree (lhs_type_string);
    MEMfree (rhs_type_string);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfFromUnq( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a N_id node representing an application
 *         of the from_class() conversion function on RHS.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 ******************************************************************************/

static node *
COMPprfFromUnq (node *arg_node, info *arg_info)
{
    node *let_ids;
    ntype *lhs_type, *rhs_type;
    node *ret_node, *arg;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    arg = PRF_ARG1 (arg_node);

    /*
     * 'arg' is unique and 'let_ids' is non-unique
     *
     * Although this is an assignment  A = from_unq( B);  the type of B is
     * possible non-unique, e.g. if this assignment has been added during
     * precompilation due to the fact that B is used as update-arg for a
     * C-function!!
     */

    lhs_type = IDS_NTYPE (let_ids);
    DBUG_ASSERT (!TUisUniqueUserType (lhs_type), "from_unq() with unique LHS found!");
    rhs_type = ID_NTYPE (arg);

    if (!TUisUniqueUserType (rhs_type)) {
        /*
         * non-unique type
         *   -> ignore from_unq() in order to get a simpler ICM code
         */
        ret_node = COMPid (arg, arg_info);
    } else {
        ret_node
          = TCmakeAssignIcm1 ("ND_ASSIGN",
                              MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE,
                                            TRUE, FALSE,
                                            MakeTypeArgs (ID_NAME (arg), ID_NTYPE (arg),
                                                          FALSE, TRUE, FALSE, NULL)),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_NTYPE (arg))));
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfToUnq( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a N_id node representing an application
 *   of the to_class() conversion function on RHS.
 *   The return value is a (possibly empty) N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 ******************************************************************************/

static node *
COMPprfToUnq (node *arg_node, info *arg_info)
{
    node *let_ids;
    ntype *lhs_type, *rhs_type;
    node *icm_args;
    node *ret_node, *arg;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    arg = PRF_ARG1 (arg_node);

    DBUG_ASSERT (!STReq (IDS_NAME (let_ids), ID_NAME (arg)),
                 ".=to_unq(.) on identical objects is not allowed!");

    /*
     * 'arg' is non-unique and 'let_ids' is unique
     *
     * Although this is an assignment  A = to_unq( B);  the type of A is
     * possible non-unique, e.g. if this assignment has been added during
     * precompilation due to the fact that A is used as update-arg for a
     * C-function.
     */

    lhs_type = IDS_NTYPE (let_ids);
    rhs_type = ID_NTYPE (arg);
    DBUG_ASSERT (!TUisUniqueUserType (rhs_type), "to_unq() with unique RHS found!");

    icm_args
      = MakeTypeArgs (IDS_NAME (let_ids), lhs_type, FALSE, TRUE, FALSE,
                      MakeTypeArgs (ID_NAME (arg), rhs_type, FALSE, TRUE, FALSE, NULL));

    /*
     * No RC manipulation requires as MAKE_UNIQUE always yields rc == 1
     */
    ret_node
      = TCmakeAssignIcm3 ("ND_MAKE_UNIQUE", icm_args, MakeBasetypeArg (lhs_type),
                          TCmakeIdCopyString (GenericFun (GF_copy, rhs_type)), NULL);

    DBUG_RETURN (ret_node);
}

/*
 * TODO: IMPLEMENT
 */
static node *
COMPprfEnclose (node *arg_node, info *arg_info)
{
    node *let_ids;
    ntype *lhs_type, *rhs_type;
    node *icm_args;
    node *ret_node, *arg;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    arg = PRF_ARG3 (arg_node);

    lhs_type = IDS_NTYPE (let_ids);
    rhs_type = ID_NTYPE (arg);

    icm_args
      = MakeTypeArgs (IDS_NAME (let_ids), lhs_type, FALSE, TRUE, TRUE,
                      MakeTypeArgs (ID_NAME (arg), rhs_type, FALSE, TRUE, FALSE, NULL));

    ret_node = TCmakeAssignIcm1 ("ND_ENCLOSE", icm_args, NULL);

    ret_node = MakeIncRcIcm (ID_NAME (arg), ID_NTYPE (arg), 1, ret_node);

    DBUG_RETURN (ret_node);
}

static node *
COMPprfDisclose (node *arg_node, info *arg_info)
{
    node *let_ids;
    ntype *lhs_type, *rhs_type;
    node *icm_args;
    node *ret_node, *arg;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    arg = PRF_ARG3 (arg_node);

    lhs_type = IDS_NTYPE (let_ids);
    rhs_type = ID_NTYPE (arg);

    icm_args
      = MakeTypeArgs (IDS_NAME (let_ids), lhs_type, FALSE, TRUE, FALSE,
                      MakeTypeArgs (ID_NAME (arg), rhs_type, FALSE, TRUE, FALSE, NULL));

    /*
     * No RC manipulation requires as MAKE_UNIQUE always yields rc == 1
     */
    ret_node
      = TCmakeAssignIcm3 ("ND_DISCLOSE", icm_args, MakeBasetypeArg (lhs_type),
                          TCmakeIdCopyString (GenericFun (GF_copy, rhs_type)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPscalar( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a constant scalar on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/
static node *
COMPscalar (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm2 ("ND_CREATE__SCALAR__DATA", DUPdupIdsIdNt (let_ids),
                                 DUPdoDupNode (arg_node), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPnumbyte( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a constant scalar on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/
node *
COMPnumbyte (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = COMPscalar (arg_node, arg_info);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPnumshort( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a constant scalar on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/
node *
COMPnumshort (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = COMPscalar (arg_node, arg_info);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPnum( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a constant scalar on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/
node *
COMPnum (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = COMPscalar (arg_node, arg_info);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPnumint( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a constant scalar on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/
node *
COMPnumint (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = COMPscalar (arg_node, arg_info);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPnumlong( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a constant scalar on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/
node *
COMPnumlong (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = COMPscalar (arg_node, arg_info);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPnumlonglong( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a constant scalar on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/
node *
COMPnumlonglong (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = COMPscalar (arg_node, arg_info);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPnumubyte( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a constant scalar on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/
node *
COMPnumubyte (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = COMPscalar (arg_node, arg_info);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPnumushort( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a constant scalar on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/
node *
COMPnumushort (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = COMPscalar (arg_node, arg_info);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPnumuint( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a constant scalar on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/
node *
COMPnumuint (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = COMPscalar (arg_node, arg_info);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPnumulong( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a constant scalar on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/
node *
COMPnumulong (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = COMPscalar (arg_node, arg_info);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPnumulonglong( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a constant scalar on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/
node *
COMPnumulonglong (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = COMPscalar (arg_node, arg_info);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPchar( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a constant scalar on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/
node *
COMPchar (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = COMPscalar (arg_node, arg_info);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPbool( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a constant scalar on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/
node *
COMPbool (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = COMPscalar (arg_node, arg_info);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPfloat( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a constant scalar on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/
node *
COMPfloat (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = COMPscalar (arg_node, arg_info);

    DBUG_RETURN (ret_node);
}

node *
COMPfloatvec (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = COMPscalar (arg_node, arg_info);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPdouble( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a constant scalar on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/
node *
COMPdouble (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = COMPscalar (arg_node, arg_info);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPArray( node *arg_node, info *arg_info)
 *
 * @brief  Compiles let expression with a constant array on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/

node *
COMParray (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    if (ARRAY_STRING (arg_node) != NULL) {
        /* array is a string */
        ret_node
          = TCmakeAssignIcm2 ("ND_CREATE__STRING__DATA", DUPdupIdsIdNt (let_ids),
                              TBmakeStr (STRcpy (ARRAY_STRING (arg_node))), ret_node);
    } else {
        node *icm_args;
        char *copyfun;

        icm_args = TBmakeExprs (MakeSizeArg (arg_node, TRUE),
                                DUPdupExprsNt (ARRAY_AELEMS (arg_node)));

        if (ARRAY_AELEMS (arg_node) != NULL) {
            node *val0 = EXPRS_EXPR (ARRAY_AELEMS (arg_node));
            if (NODE_TYPE (val0) == N_id) {
                copyfun = GenericFun (GF_copy, ID_NTYPE (val0));
            } else {
                copyfun = NULL;
            }
        } else {
            /*
             * A = [:type];
             * where type \in AKS
             */
            copyfun = NULL;
        }

        ret_node
          = TCmakeAssignIcm2 ("ND_CREATE__ARRAY__DATA",
                              MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE,
                                            TRUE, FALSE, DUPdoDupTree (icm_args)),
                              TCmakeIdCopyString (copyfun), ret_node);
    }

    DBUG_RETURN (ret_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn  node COMPprfPropObjIn( node *arg_node, info *arg_info)
 *
 * @brief Compiles N_prf node of type F_prop_obj_in.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 ******************************************************************************/

node *
COMPprfPropObjIn (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;
    DBUG_ENTER ();

    ret_node = MakeIcm_PROP_OBJ_IN (arg_node, INFO_LASTIDS (arg_info), NULL);
    DBUG_RETURN (ret_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn  node COMPprfPropObjIn( node *arg_node, info *arg_info)
 *
 * @brief Compiles N_prf node of type F_prop_obj_in.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 ******************************************************************************/

node *
COMPprfPropObjOut (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;
    DBUG_ENTER ();

    ret_node = MakeIcm_PROP_OBJ_OUT (arg_node, INFO_LASTIDS (arg_info), NULL);
    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node COMPprfIncRC( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_inc_rc.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/
static node *
COMPprfIncRC (node *arg_node, info *arg_info)
{
    char *name;
    ntype *type;
    node *ret_node = NULL;
    int num;

    DBUG_ENTER ();

    switch (NODE_TYPE (PRF_ARG1 (arg_node))) {
    case N_id:
        name = ID_NAME (PRF_ARG1 (arg_node));
        type = ID_NTYPE (PRF_ARG1 (arg_node));
        num = NUM_VAL (PRF_ARG2 (arg_node));

        ret_node = MakeIncRcIcm (name, type, num, NULL);
        break;

    case N_globobj:
        name = OBJDEF_NAME (GLOBOBJ_OBJDEF (PRF_ARG1 (arg_node)));
        type = OBJDEF_TYPE (GLOBOBJ_OBJDEF (PRF_ARG1 (arg_node)));
        num = NUM_VAL (PRF_ARG2 (arg_node));

        ret_node = MakeIncRcIcm (name, type, num, NULL);

        type = TYfreeType (type);
        break;
    default:
        DBUG_UNREACHABLE ("1. Argument of inc_rc has wrong node type.");
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node COMPprfDecRC( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_dec_rc.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/
static node *
COMPprfDecRC (node *arg_node, info *arg_info)
{
    char *name;
    ntype *type;
    node *ret_node = NULL;
    int num;

    DBUG_ENTER ();

    switch (NODE_TYPE (PRF_ARG1 (arg_node))) {
    case N_id:
        name = ID_NAME (PRF_ARG1 (arg_node));
        type = ID_NTYPE (PRF_ARG1 (arg_node));
        num = NUM_VAL (PRF_ARG2 (arg_node));

        ret_node = MakeDecRcIcm (name, type, num, NULL);
        break;

    case N_globobj:
        name = OBJDEF_NAME (GLOBOBJ_OBJDEF (PRF_ARG1 (arg_node)));
        type = OBJDEF_TYPE (GLOBOBJ_OBJDEF (PRF_ARG1 (arg_node)));
        num = NUM_VAL (PRF_ARG2 (arg_node));

        ret_node = MakeDecRcIcm (name, type, num, NULL);

        type = TYfreeType (type);

        break;
    default:
        DBUG_UNREACHABLE ("1. Argument of dec_rc has wrong node type.");
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node COMPprfRestorerc( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_restorerc.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/
static node *
COMPprfRestorerc (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;
    DBUG_ENTER ();

    DBUG_ASSERT (ID_AVIS (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))))
                   == IDS_AVIS (INFO_LASTIDS (arg_info)),
                 "Second arg to Restorerc and lhs must have same avis");

    ret_node
      = TCmakeAssignIcm2 ("ND_PRF_RESTORERC",
                          TCmakeIdCopyStringNtNew (AVIS_NAME (
                                                   IDS_AVIS (INFO_LASTIDS (arg_info))),
                                                   IDS_NTYPE (INFO_LASTIDS (arg_info))),
                          TCmakeIdCopyStringNtNew (AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                                                   ID_NTYPE (PRF_ARG1 (arg_node))),
                          NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node COMPprf2norc( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_2norc.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/
static node *
COMPprf2norc (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;
    DBUG_ENTER ();

    ret_node
      = TCmakeAssignIcm2 ("ND_PRF_2NORC",
                          TCmakeIdCopyStringNtNew (IDS_NAME (INFO_LASTIDS (arg_info)),
                                                   IDS_NTYPE (INFO_LASTIDS (arg_info))),
                          TCmakeIdCopyStringNtNew (ID_NAME (PRF_ARG1 (arg_node)),
                                                   ID_NTYPE (PRF_ARG1 (arg_node))),
                          NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node COMPprf2asyncrc( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_2asyncrc.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/
static node *
COMPprf2asyncrc (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;
    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm1 ("ND_REFRESH__MIRROR",
                                 MakeTypeArgs (IDS_NAME (INFO_LASTIDS (arg_info)),
                                               IDS_NTYPE (INFO_LASTIDS (arg_info)), FALSE,
                                               TRUE, FALSE, NULL),
                                 ret_node);

    ret_node
      = TCmakeAssignIcm2 ("ND_PRF_2ASYNC",
                          TCmakeIdCopyStringNtNew (IDS_NAME (INFO_LASTIDS (arg_info)),
                                                   IDS_NTYPE (INFO_LASTIDS (arg_info))),
                          TCmakeIdCopyStringNtNew (ID_NAME (PRF_ARG1 (arg_node)),
                                                   ID_NTYPE (PRF_ARG1 (arg_node))),
                          ret_node);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node COMPprfAlloc( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_alloc.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfAlloc (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *ret_node;
    int rc;
    node *get_dim;
    node *set_shape;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    rc = NUM_VAL (PRF_ARG1 (arg_node));
    get_dim = MakeGetDimIcm (PRF_ARG2 (arg_node));
    set_shape = MakeSetShapeIcm (PRF_ARG3 (arg_node), let_ids);

    ret_node = MakeAllocIcm (IDS_NAME (let_ids), IDS_NTYPE (let_ids), rc, get_dim,
                             set_shape, NULL, NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node COMPprfAllocOrReuse( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_alloc_or_reuse.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfAllocOrReuse (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *ret_node;
    int rc;
    node *get_dim;
    node *set_shape;
    node *cand;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    rc = NUM_VAL (PRF_ARG1 (arg_node));
    get_dim = MakeGetDimIcm (PRF_ARG2 (arg_node));
    set_shape = MakeSetShapeIcm (PRF_ARG3 (arg_node), let_ids);

    ret_node = MakeAllocIcm_IncRc (IDS_NAME (let_ids), IDS_NTYPE (let_ids), rc, get_dim,
                                   set_shape, NULL, NULL);

    cand = EXPRS_EXPRS4 (PRF_ARGS (arg_node));
    while (cand != NULL) {
        ret_node = MakeCheckReuseIcm (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                      EXPRS_EXPR (cand), ret_node);
        cand = EXPRS_NEXT (cand);
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node COMPprfResize( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_resize.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfResize (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *ret_node;
    int rc;
    node *get_dim;
    node *set_shape;
    node *resizecand;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    rc = NUM_VAL (PRF_ARG1 (arg_node));
    get_dim = MakeGetDimIcm (PRF_ARG2 (arg_node));
    set_shape = MakeSetShapeIcm (PRF_ARG3 (arg_node), let_ids);
    resizecand = PRF_ARG4 (arg_node);

    DBUG_ASSERT (resizecand != NULL, "no source for resize found!");

    ret_node
      = MakeReAllocIcm (IDS_NAME (let_ids), IDS_NTYPE (let_ids), ID_NAME (resizecand),
                        ID_NTYPE (resizecand), rc, get_dim, set_shape, NULL, NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node COMPprfAllocOrResize( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_alloc_or_resize.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfAllocOrResize (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *ret_node = NULL;
    int rc;
    node *get_dim;
    node *set_shape;
    node *cand;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    rc = NUM_VAL (PRF_ARG1 (arg_node));
    get_dim = MakeGetDimIcm (PRF_ARG2 (arg_node));
    set_shape = MakeSetShapeIcm (PRF_ARG3 (arg_node), let_ids);
    cand = EXPRS_EXPRS4 (PRF_ARGS (arg_node));

    /*
     * We have to do the incrc explicitly, as we do not know whether
     * we use the allocated or resized data.
     */
    ret_node = MakeIncRcIcm (IDS_NAME (let_ids), IDS_NTYPE (let_ids), rc, ret_node);

    if (cand != NULL) {
        ret_node = TCmakeAssignIcm1 ("SAC_IS_LASTREF__BLOCK_END",
                                     TCmakeIdCopyStringNtNew (ID_NAME (EXPRS_EXPR (cand)),
                                                           ID_NTYPE (EXPRS_EXPR (cand))),
                                     ret_node);
    }

    ret_node = MakeAllocIcm (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                             0, /* done by explicit incrc */
                             get_dim, set_shape, NULL, ret_node);

    while (cand != NULL) {
        ret_node = MakeCheckResizeIcm (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                       EXPRS_EXPR (cand), rc, DUPdoDupTree (get_dim),
                                       DUPdoDupTree (set_shape), ret_node);
        cand = EXPRS_NEXT (cand);
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node COMPprfFree( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_free
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *
 ******************************************************************************/

static node *
COMPprfFree (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (PRF_ARG1 (arg_node)) != N_globobj,
                 "Application of F_free to N_globobj detected!");

    ret_node = MakeSetRcIcm (ID_NAME (PRF_ARG1 (arg_node)), ID_NTYPE (PRF_ARG1 (arg_node)),
                             0, NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node COMPprfSuballoc( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_suballoc
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfSuballoc (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *ret_node = NULL;
    node *mem_id;
    shape_class_t sc;
    node *sub_get_dim = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT ((NODE_TYPE (PRF_ARG1 (arg_node)) == N_num),
                 "first arg of suballoc should be rc-value introduced in phase rc;"
                 " node type found: %s", global.mdb_nodetype[NODE_TYPE (PRF_ARG1 (arg_node))]);

    let_ids = INFO_LASTIDS (arg_info);
    mem_id = PRF_ARG2 (arg_node);
    sc = NTUgetShapeClassFromNType (IDS_NTYPE (let_ids));

    DBUG_ASSERT (sc != C_scl, "scalars cannot be suballocated\n");

    if (INFO_WITHLOOP (arg_info) != NULL && WITH_CUDARIZABLE (INFO_WITHLOOP (arg_info))) {
        ret_node
          = TCmakeAssignIcm5 ("CUDA_WL_SUBALLOC", DUPdupIdsIdNt (let_ids),
                              TBmakeNum (TUgetFullDimEncoding (IDS_NTYPE (let_ids))),
                              DUPdupIdNt (PRF_ARG2 (arg_node)),
                              TBmakeNum (TUgetFullDimEncoding (ID_NTYPE (PRF_ARG1 (arg_node)))),
                              DUPdupIdNt (PRF_ARG3 (arg_node)), NULL);
    } else if (global.backend == BE_distmem) {
        ret_node = TCmakeAssignIcm3 ("WL_DISTMEM_SUBALLOC", DUPdupIdsIdNt (let_ids),
                                     DUPdupIdNt (PRF_ARG2 (arg_node)),
                                     DUPdupIdNt (PRF_ARG3 (arg_node)), NULL);
    } else {
        ret_node = TCmakeAssignIcm3 ("WL_SUBALLOC", DUPdupIdsIdNt (let_ids),
                                     DUPdupIdNt (PRF_ARG2 (arg_node)),
                                     DUPdupIdNt (PRF_ARG3 (arg_node)), NULL);
    }

    if ((global.backend == BE_mutc)
        && !AVIS_SUBALLOC (IDS_AVIS (INFO_LASTIDS (arg_info)))) {
#if 0 /* Still may be present if not canonical */
    DBUG_ASSERT (PRF_ARG3( arg_node) != NULL, "suballoc lacking default information in mutc backend");
#endif
        /*
         * We need to allocate a descriptor and set the shape
         *
         * 1) compute dimensionality. If the range is not chunked,
         *    its the dimensionality of the memvar - 1, else its
         *    the dimensionality of the memvar. The third arg
         *    of the suballoc holds this information.
         */
        if (TCcountExprs (PRF_ARGS (arg_node)) >= 4) {
            sub_get_dim = TCmakeIcm2 (prf_ccode_tab[F_sub_SxS],
                                      TCmakeIcm1 ("ND_A_DIM", DUPdupIdNt (mem_id)),
                                      DUPdoDupNode (PRF_ARG4 (arg_node)));
        }
        /*
         * 2) annotate shape for suballoc, if information present and
         *    dynamic shape required.
         *    TODO MUTC: As we only support genarray for now, this
         *               information always has to be present!
         */
        if (TCcountExprs (PRF_ARGS (arg_node)) >= 5) {
            if (!KNOWN_SHAPE (TUgetFullDimEncoding (IDS_NTYPE (let_ids)))) {
#if 0 /* Still may be present if not canonical */
        DBUG_ASSERT (PRF_ARG5( arg_node) != NULL, "missing shape information for suballoc");
#endif

                ret_node = TBmakeAssign (MakeSetShapeIcm (PRF_ARG5 (arg_node), let_ids),
                                         ret_node);
            }
        }

        /*
         * 3) produce the descriptor allocation
         *
         * Allocate the desc local as it will not go out of scope
         */
        ret_node = MakeMutcLocalAllocDescIcm (IDS_NAME (let_ids), IDS_NTYPE (let_ids), 1,
                                              sub_get_dim, ret_node);
#if FREE_LOCAL /* Do not free local alloced stuff */
        /*
         * 4) add a corresponding descriptor free to the
         *    postfun icm chain
         */
        INFO_POSTFUN (arg_info)
          = TCmakeAssignIcm1 ("ND_FREE__DESC",
                              TCmakeIdCopyStringNtNew (IDS_NAME (let_ids),
                                                    IDS_NTYPE (let_ids)),
                              INFO_POSTFUN (arg_info));
#endif
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node COMPprfWLAssign( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_wl_assign
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfWLAssign (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *arg3;
    DBUG_ENTER ();

    if (NODE_TYPE (PRF_ARG3 (arg_node)) == N_id) {
        arg3 = DUPdupIdNt (PRF_ARG3 (arg_node));
    } else {
        arg3 = DUPdupIdNt (EXPRS_EXPR (ARRAY_AELEMS (PRF_ARG3 (arg_node))));
    }

    ret_node
      = TCmakeAssignIcm6 ("WL_ASSIGN",
                          MakeTypeArgs (ID_NAME (PRF_ARG1 (arg_node)),
                                        ID_NTYPE (PRF_ARG1 (arg_node)), FALSE, TRUE, FALSE,
                                        NULL),
                          MakeTypeArgs (ID_NAME (PRF_ARG2 (arg_node)),
                                        ID_NTYPE (PRF_ARG2 (arg_node)), FALSE, TRUE, FALSE,
                                        NULL),
                          arg3,
                          TBmakeExprs (MakeSizeArg (PRF_ARG3 (arg_node), TRUE), NULL),
                          DUPdupIdNt (PRF_ARG4 (arg_node)),
                          TCmakeIdCopyString (
                            GenericFun (GF_copy, ID_NTYPE (PRF_ARG1 (arg_node)))),
                          NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node COMPprfCUDAWLAssign( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_cuda_wl_assign
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfCUDAWLAssign (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm3 ("CUDA_WL_ASSIGN",
                                 MakeTypeArgs (ID_NAME (PRF_ARG1 (arg_node)),
                                               ID_NTYPE (PRF_ARG1 (arg_node)), FALSE, TRUE,
                                               FALSE, NULL),
                                 MakeTypeArgs (ID_NAME (PRF_ARG2 (arg_node)),
                                               ID_NTYPE (PRF_ARG2 (arg_node)), FALSE, TRUE,
                                               FALSE, NULL),
                                 DUPdupIdNt (PRF_ARG3 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node COMPprfCondWLAssign( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_cond_wl_assign
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/
static node *
COMPprfCondWLAssign (node *arg_node, info *arg_info)
{
    node *cond, *shmemidx, *shmem, *devidx, *devmem;
    node *icm_args;

    node *ret_node = NULL;

    DBUG_ENTER ();

    cond = PRF_ARG1 (arg_node);
    shmemidx = PRF_ARG2 (arg_node);
    shmem = PRF_ARG3 (arg_node);
    devidx = PRF_ARG4 (arg_node);
    devmem = PRF_ARG5 (arg_node);

    icm_args
      = TBmakeExprs (DUPdupNodeNt (cond),
                     TBmakeExprs (DUPdupNodeNt (shmemidx),
                                  TBmakeExprs (DUPdupNodeNt (shmem),
                                               TBmakeExprs (DUPdupNodeNt (devidx),
                                                            TBmakeExprs (DUPdupNodeNt (
                                                                           devmem),
                                                                         NULL)))));

    ret_node = TCmakeAssignIcm1 ("CUDA_COND_WL_ASSIGN", icm_args, NULL);

    DBUG_RETURN (ret_node);
}

static node *
COMPprfCUDAGridBlock (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm2 ("CUDA_GRID_BLOCK",
                                 TBmakeNumuint (TCcountExprs (PRF_ARGS (arg_node))),
                                 DupExprs_NT_AddReadIcms (PRF_ARGS (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

static node *
COMPprfCUDAThreadSpace (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm3 ("CUDA_THREAD_SPACE",
                                 DUPdoDupTree( PRF_ARG1 (arg_node)),
                                 TBmakeNumuint (TCcountExprs (EXPRS_NEXT (PRF_ARGS (arg_node)))),
                                 DupExprs_NT_AddReadIcms (EXPRS_NEXT (PRF_ARGS (arg_node))), NULL);

    DBUG_RETURN (ret_node);
}

static node *
COMPprfCUDAIndexSpace (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm3 ("CUDA_INDEX_SPACE",
                                 DUPdoDupTree( PRF_ARG1 (arg_node)),
                                 TBmakeNumuint (TCcountExprs (EXPRS_NEXT (PRF_ARGS (arg_node)))),
                                 DupExprs_NT_AddReadIcms (EXPRS_NEXT (PRF_ARGS (arg_node))), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node COMPprfWLBreak( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_wl_break
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfWLBreak (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm3 ("ND_ASSIGN__DATA", DUPdupIdNt (PRF_ARG2 (arg_node)),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 TCmakeIdCopyString (
                                   GenericFun (GF_copy, ID_NTYPE (PRF_ARG1 (arg_node)))),
                                 NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node COMPprfCopy( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_copy
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *
 ******************************************************************************/

static node *
COMPprfCopy (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *ret_node;
    simpletype src_basetype, dst_basetype;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    if (global.backend != BE_cuda && global.backend != BE_cudahybrid) {
        ret_node
          = TCmakeAssignIcm3 ("ND_COPY__DATA", DUPdupIdsIdNt (let_ids),
                              DUPdupIdNt (PRF_ARG1 (arg_node)),
                              TCmakeIdCopyString (
                                GenericFun (GF_copy, ID_NTYPE (PRF_ARG1 (arg_node)))),
                              NULL);
    } else {
        src_basetype = TUgetSimpleImplementationType (ID_NTYPE (PRF_ARG1 (arg_node)));
        dst_basetype = TUgetSimpleImplementationType (IDS_NTYPE (let_ids));

        if (CUisDeviceTypeNew (ID_NTYPE (PRF_ARG1 (arg_node)))
            && (src_basetype == dst_basetype)
            && !FUNDEF_ISCUDAGLOBALFUN (INFO_FUNDEF (arg_info))) {
            ret_node
              = TCmakeAssignIcm4 ("CUDA_COPY__ARRAY", DUPdupIdsIdNt (let_ids),
                                  DUPdupIdNt (PRF_ARG1 (arg_node)),
                                  MakeBasetypeArg (ID_NTYPE (PRF_ARG1 (arg_node))),
                                  TCmakeIdCopyString (
                                    GenericFun (GF_copy, ID_NTYPE (PRF_ARG1 (arg_node)))),
                                  NULL);
        } else {
            ret_node
              = TCmakeAssignIcm3 ("ND_COPY__DATA", DUPdupIdsIdNt (let_ids),
                                  DUPdupIdNt (PRF_ARG1 (arg_node)),
                                  TCmakeIdCopyString (
                                    GenericFun (GF_copy, ID_NTYPE (PRF_ARG1 (arg_node)))),
                                  NULL);
        }
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node COMPprfIsReused( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_isreused
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *
 ******************************************************************************/

static node *
COMPprfIsReused (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm3 ("ND_PRF_IS_REUSED", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 DUPdupIdNt (PRF_ARG2 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfDim( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_dim_A.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfDim (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *arg;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    DBUG_ASSERT (NODE_TYPE (arg) == N_id, "arg of F_dim_A is no N_id!");

    ret_node = TCmakeAssignIcm1 ("ND_PRF_DIM_A__DATA",
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                               FALSE, TRUE, FALSE,
                                               MakeTypeArgs (ID_NAME (arg), ID_NTYPE (arg),
                                                             FALSE, TRUE, FALSE, NULL)),
                                 NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfIsDist( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_isDist_A.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfIsDist (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *arg;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    DBUG_ASSERT (NODE_TYPE (arg) == N_id, "arg of F_isDist_A is no N_id!");

    ret_node = TCmakeAssignIcm1 ("ND_PRF_IS_DIST_A__DATA",
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                               FALSE, FALSE, FALSE,
                                               MakeTypeArgs (ID_NAME (arg), ID_NTYPE (arg),
                                                             FALSE, FALSE, FALSE, NULL)),
                                 NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfFirstElems( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_firstElems_A.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfFirstElems (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *arg;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    DBUG_ASSERT (NODE_TYPE (arg) == N_id, "arg of F_firstElems_A is no N_id!");

    ret_node = TCmakeAssignIcm1 ("ND_PRF_FIRST_ELEMS_A__DATA",
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                               FALSE, FALSE, FALSE,
                                               MakeTypeArgs (ID_NAME (arg), ID_NTYPE (arg),
                                                             FALSE, FALSE, FALSE, NULL)),
                                 NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfLocalFrom( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_localFrom_A.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfLocalFrom (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *arg;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    DBUG_ASSERT (NODE_TYPE (arg) == N_id, "arg of F_localFrom_A is no N_id!");

    ret_node = TCmakeAssignIcm1 ("ND_PRF_LOCAL_FROM_A__DATA",
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                               FALSE, FALSE, FALSE,
                                               MakeTypeArgs (ID_NAME (arg), ID_NTYPE (arg),
                                                             FALSE, FALSE, FALSE, NULL)),
                                 NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfLocalCount( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_localCount_A.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfLocalCount (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *arg;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    DBUG_ASSERT (NODE_TYPE (arg) == N_id, "arg of F_localCount_A is no N_id!");

    ret_node = TCmakeAssignIcm1 ("ND_PRF_LOCAL_COUNT_A__DATA",
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                               FALSE, FALSE, FALSE,
                                               MakeTypeArgs (ID_NAME (arg), ID_NTYPE (arg),
                                                             FALSE, FALSE, FALSE, NULL)),
                                 NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfOffs( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_offs_A.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfOffs (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *arg;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    DBUG_ASSERT (NODE_TYPE (arg) == N_id, "arg of F_offs_A is no N_id!");

    ret_node = TCmakeAssignIcm1 ("ND_PRF_OFFS_A__DATA",
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                               FALSE, FALSE, FALSE,
                                               MakeTypeArgs (ID_NAME (arg), ID_NTYPE (arg),
                                                             FALSE, FALSE, FALSE, NULL)),
                                 NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfShape( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_shape_A.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfShape (node *arg_node, info *arg_info)
{
    node *arg;
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    DBUG_ASSERT (NODE_TYPE (arg) == N_id, "arg of F_shape_A is no N_id!");

    ret_node = TCmakeAssignIcm1 ("ND_PRF_SHAPE_A__DATA",
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                               FALSE, TRUE, FALSE,
                                               MakeTypeArgs (ID_NAME (arg), ID_NTYPE (arg),
                                                             FALSE, TRUE, FALSE, NULL)),
                                 NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfSize( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_size_A.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfSize (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *arg;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    DBUG_ASSERT (NODE_TYPE (arg) == N_id, "arg of F_size_A is no N_id!");

    ret_node = TCmakeAssignIcm1 ("ND_PRF_SIZE_A__DATA",
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                               FALSE, TRUE, FALSE,
                                               MakeTypeArgs (ID_NAME (arg), ID_NTYPE (arg),
                                                             FALSE, TRUE, FALSE, NULL)),
                                 NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfReshape( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_reshape_VxA.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfReshape (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *set_shape_icm = NULL;
    int rc;
    char *copyfun;
    int dim_new, dim_old;
    node *ret_node = NULL;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    /*
     *   B = reshape( sv, A);
     *
     * If this is the last reference to 'A', 'B' can reuse some parts of 'A':
     *   'B' can reuse the data vector of 'A'.
     *   'B' can reuse the descriptor of 'A' if the dimension of 'B' is smaller
     *     or equal the dimension of 'A'.
     *
     ****************************************************************************
     *
     * For efficiency reasons, constant arrays are excepted as arguments of
     * reshape() as well:
     *
     *   A = fun( ...);
     *   B = reshape( [3,4], A);
     *
     * Here, the backend can avoid the creation of the array containing the shape
     * [3,4].
     *
     */
    DBUG_ASSERT (NODE_TYPE (PRF_ARG4 (arg_node)) == N_id,
                 "Fourth arg of reshape must be N_id");

    rc = NUM_VAL (PRF_ARG1 (arg_node));
    set_shape_icm = MakeSetShapeIcm (PRF_ARG3 (arg_node), let_ids);

#if 1
    /* Is this correct? Or do we have to take the rhs instead? */
    copyfun = GenericFun (GF_copy, IDS_NTYPE (let_ids));
#endif

    ret_node
      = MakeSetRcIcm (IDS_NAME (let_ids), IDS_NTYPE (let_ids), rc,
                      TBmakeAssign (set_shape_icm,
                                    TCmakeAssignIcm3 ("ND_ASSIGN__DATA",
                                                      DUPdupIdsIdNt (let_ids),
                                                      DUPdupIdNt (PRF_ARG4 (arg_node)),
                                                      TCmakeIdCopyString (copyfun),
                                                      ret_node)));

    dim_new = TUgetDimEncoding (IDS_NTYPE (let_ids));
    dim_old = TUgetDimEncoding (ID_NTYPE (PRF_ARG4 (arg_node)));

    if ((dim_new >= 0) && (dim_old >= 0) && (dim_new <= dim_old)) {
        /*
         * the old descriptor is large enough to be reused :-)
         */
        ret_node = TCmakeAssignIcm2 ("ND_ASSIGN__DESC", DUPdupIdsIdNt (let_ids),
                                     DUPdupIdNt (PRF_ARG4 (arg_node)), ret_node);
    } else if (global.backend == BE_distmem) {
        ret_node = MakeAllocDescIcm (
          IDS_NAME (let_ids), IDS_NTYPE (let_ids), rc, MakeGetDimIcm (PRF_ARG2 (arg_node)),
          TCmakeAssignIcm2 ("ND_COPY__DESC_DIS_FIELDS", DUPdupIdNt (PRF_ARG4 (arg_node)),
                            DUPdupIdsIdNt (let_ids),
                            TCmakeAssignIcm1 ("ND_FREE__DESC",
                                              DUPdupIdNt (PRF_ARG4 (arg_node)),
                                              TCmakeAssignIcm2 ("ND_ASSIGN__DESC",
                                                                DUPdupIdNt (
                                                                  PRF_ARG4 (arg_node)),
                                                                DUPdupIdsIdNt (let_ids),
                                                                ret_node))));
    } else {
        ret_node
          = MakeAllocDescIcm (IDS_NAME (let_ids), IDS_NTYPE (let_ids), rc,
                              MakeGetDimIcm (PRF_ARG2 (arg_node)),
                              TCmakeAssignIcm1 ("ND_FREE__DESC",
                                                DUPdupIdNt (PRF_ARG4 (arg_node)),
                                                TCmakeAssignIcm2 ("ND_ASSIGN__DESC",
                                                                  DUPdupIdNt (
                                                                    PRF_ARG4 (arg_node)),
                                                                  DUPdupIdsIdNt (let_ids),
                                                                  ret_node)));
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfAllocOrReshape( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_alloc_or_reshape.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfAllocOrReshape (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *get_dim = NULL, *set_shape_icm = NULL;
    int rc;
    node *ret_node = NULL;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    rc = NUM_VAL (PRF_ARG1 (arg_node));
    NUM_VAL (PRF_ARG1 (arg_node)) = 1;

    get_dim = MakeGetDimIcm (PRF_ARG2 (arg_node));
    set_shape_icm = MakeSetShapeIcm (PRF_ARG3 (arg_node), let_ids);

    ret_node = TCmakeAssignIcm1 (
      "IS_LASTREF__BLOCK_BEGIN", DUPdupIdNt (PRF_ARG4 (arg_node)),
      TCappendAssign (
        COMPprfReshape (arg_node, arg_info),
        MakeIncRcIcm (IDS_NAME (let_ids), IDS_NTYPE (let_ids), rc,
                      TCmakeAssignIcm1 (
                        "IS_LASTREF__BLOCK_ELSE", DUPdupIdNt (PRF_ARG4 (arg_node)),
                        MakeAllocIcm (IDS_NAME (let_ids), IDS_NTYPE (let_ids), rc, get_dim,
                                      set_shape_icm, NULL,
                                      TCmakeAssignIcm1 ("IS_LASTREF__BLOCK_END",
                                                        DUPdupIdNt (PRF_ARG4 (arg_node)),
                                                        ret_node))))));

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfIdxSel( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_idx_sel.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/
static node *
COMPprfIdxSel (node *arg_node, info *arg_info)
{
    node *arg1, *arg2;
    node *let_ids;
    int dim;
    node *icm_args;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    /*
     * CAUTION: AE, IVE generate unflattened code!
     * The 1st argument of idx_sel() may be a N_prf node with N_num arguments
     */
    DBUG_ASSERT (((NODE_TYPE (arg1) == N_id) || (NODE_TYPE (arg1) == N_num)
                  || ((NODE_TYPE (arg1) == N_prf))),
                 "1st arg of F_idx_sel is neither N_id nor N_num, N_prf!");
    DBUG_ASSERT (NODE_TYPE (arg2) == N_id, "2nd arg of F_idx_sel is no N_id!");

    icm_args = MakeTypeArgs (ID_NAME (arg2), ID_NTYPE (arg2), FALSE, TRUE, FALSE,
                             TBmakeExprs (DUPdupNodeNt (arg1), NULL));

    /* idx_sel() works only for arrays with known dimension!!! */
    dim = TUgetDimEncoding (IDS_NTYPE (let_ids));
    DBUG_ASSERT (dim >= 0, "unknown dimension found!");

    /* The ICM depends on whether we use the distributed memory backend
     * and the read access is known to be local. */
    char *icm_name = "ND_PRF_IDX_SEL__DATA";
    if (global.backend == BE_distmem && PRF_DISTMEMISLOCALREAD (arg_node)) {
        icm_name = "ND_PRF_IDX_SEL__DATA_Local";
    }

    ret_node
      = TCmakeAssignIcm2 (icm_name,
                          MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE,
                                        TRUE, FALSE, DUPdoDupTree (icm_args)),
                          TCmakeIdCopyString (GenericFun (GF_copy, ID_NTYPE (arg2))),
                          NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfIdxModarray_AxSxS( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_idx_modarray.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfIdxModarray_AxSxS (node *arg_node, info *arg_info)
{
    node *arg1, *arg2, *arg3;
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    arg3 = PRF_ARG3 (arg_node);

    DBUG_ASSERT (NODE_TYPE (arg1) == N_id, "1st arg of F_idx_modarray_AxSxS is no N_id!");
    /*
     * Because of IVE, at the 2nd argument position of F_idx_modarray might occur
     * an arithmetical expression (see function IdxArray)!!!
     */
    DBUG_ASSERT (((NODE_TYPE (arg2) == N_id) || (NODE_TYPE (arg2) == N_num)
                  || (NODE_TYPE (arg2) == N_prf)),
                 "2nd arg of F_idx_modarray_AxSxS is neither N_id nor N_num, N_prf!");
    DBUG_ASSERT (((NODE_TYPE (arg2) != N_id)
                  || (TUgetSimpleImplementationType (ID_NTYPE (arg2)) == T_int)),
                 "2nd arg of F_idx_modarray_AxSxS is a illegal indexing var!");
    DBUG_ASSERT (NODE_TYPE (arg3) != N_array,
                 "3rd arg of F_idx_modarray_AxSxS is a N_array!");

    /*
      if( global.backend == BE_cuda &&
          ( TUgetSimpleImplementationType (ID_NTYPE( arg1)) == T_float_dev ||
            TUgetSimpleImplementationType (ID_NTYPE( arg1)) == T_int_dev) &&
          !FUNDEF_ISCUDAGLOBALFUN( INFO_FUNDEF( arg_info))) {
        ret_node = TCmakeAssignIcm4( "CUDA_PRF_IDX_MODARRAY_AxSxS__DATA",
                                     MakeTypeArgs( IDS_NAME( let_ids),
                                                   IDS_NTYPE( let_ids),
                                                   FALSE, TRUE, FALSE,
                                     MakeTypeArgs( ID_NAME( arg1),
                                                   ID_NTYPE( arg1),
                                                   FALSE, TRUE, FALSE,
                                     NULL)),
                                     DUPdupNodeNt( arg2),
                                     DUPdupNodeNt( arg3),
                                     MakeBasetypeArg( ID_NTYPE(arg1)),
                   NULL);
      }
      else {
    */
    ret_node
      = TCmakeAssignIcm4 ("ND_PRF_IDX_MODARRAY_AxSxS__DATA",
                          MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE,
                                        TRUE, FALSE,
                                        MakeTypeArgs (ID_NAME (arg1), ID_NTYPE (arg1),
                                                      FALSE, TRUE, FALSE, NULL)),
                          DUPdupNodeNt (arg2), DUPdupNodeNt (arg3),
                          TCmakeIdCopyString (GenericFun (GF_copy, ID_NTYPE (arg1))),
                          NULL);
    /*
      }
    */
    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfIdxModarray_AxSxA( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_idx_modarray.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfIdxModarray_AxSxA (node *arg_node, info *arg_info)
{
    node *arg1, *arg2, *arg3;
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    arg3 = PRF_ARG3 (arg_node);

    DBUG_ASSERT (NODE_TYPE (arg1) == N_id, "1st arg of F_idx_modarray_AxSxA is no N_id!");
    /*
     * Because of IVE, at the 2nd argument position of F_idx_modarray might occur
     * an arithmetical expression (see function IdxArray)!!!
     */
    DBUG_ASSERT (((NODE_TYPE (arg2) == N_id) || (NODE_TYPE (arg2) == N_num)
                  || (NODE_TYPE (arg2) == N_prf)),
                 "2nd arg of F_idx_modarray_AxSxA is neither N_id nor N_num, N_prf!");
    DBUG_ASSERT (((NODE_TYPE (arg2) != N_id)
                  || (TUgetSimpleImplementationType (ID_NTYPE (arg2)) == T_int)),
                 "2nd arg of F_idx_modarray_AxSxA is a illegal indexing var!");
    DBUG_ASSERT (NODE_TYPE (arg3) != N_array,
                 "3rd arg of F_idx_modarray_AxSxA is a N_array!");

    if ((global.backend == BE_cuda || global.backend == BE_cudahybrid)
        && CUisDeviceTypeNew (ID_NTYPE (arg1)) && CUisDeviceTypeNew (ID_NTYPE (arg3))
        && !FUNDEF_ISCUDAGLOBALFUN (INFO_FUNDEF (arg_info))) {
        ret_node
          = TCmakeAssignIcm4 ("CUDA_PRF_IDX_MODARRAY_AxSxA__DATA",
                              MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE,
                                            TRUE, FALSE,
                                            MakeTypeArgs (ID_NAME (arg1), ID_NTYPE (arg1),
                                                          FALSE, TRUE, FALSE, NULL)),
                              DUPdupNodeNt (arg2), DUPdupNodeNt (arg3),
                              MakeBasetypeArg (ID_NTYPE (arg1)), NULL);
    } else {
        ret_node
          = TCmakeAssignIcm4 ("ND_PRF_IDX_MODARRAY_AxSxA__DATA",
                              MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE,
                                            TRUE, FALSE,
                                            MakeTypeArgs (ID_NAME (arg1), ID_NTYPE (arg1),
                                                          FALSE, TRUE, FALSE, NULL)),
                              DUPdupNodeNt (arg2), DUPdupNodeNt (arg3),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_NTYPE (arg1))),
                              NULL);
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfIdxShapeSel( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_idx_shape_sel.
 *         The return value is a N_assign chain of ICMs.
 *         Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfIdxShapeSel (node *arg_node, info *arg_info)
{
    node *arg1, *arg2;
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT (NODE_TYPE (arg1) == N_num, "1st arg of F_idx_shape_sel is no N_num!");
    DBUG_ASSERT (NODE_TYPE (arg2) == N_id, "2nd arg of F_idx_shape_sel is no N_id!");

    ret_node = TCmakeAssignIcm3 ("ND_PRF_IDX_SHAPE_SEL__DATA",
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                               FALSE, TRUE, FALSE, NULL),
                                 MakeTypeArgs (ID_NAME (arg2), ID_NTYPE (arg2), FALSE,
                                               TRUE, FALSE, NULL),
                                 TBmakeExprs (DUPdupNodeNt (arg1), NULL), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfSel( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_sel_VxA.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfSel (node *arg_node, info *arg_info)
{
    node *arg1, *arg2;
    node *let_ids;
    node *icm_args;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    /*
     *   B = sel( iv, A);
     *
     ****************************************************************************
     *
     * For efficiency reasons, constant arrays are excepted as 1st argument of
     * sel() as well:
     *
     *   A = fun( ...);
     *   B = sel( [3,4], A);
     *
     * Here, the backend can avoid the creation of the array containing the shape
     * [3,4].
     */

    DBUG_ASSERT (NODE_TYPE (arg2) == N_id, "2nd arg of F_sel_VxA is no N_id!");

    if (NODE_TYPE (arg1) == N_id) {
        DBUG_ASSERT (TUgetSimpleImplementationType (ID_NTYPE (arg1)) == T_int,
                     "1st arg of F_sel_VxA is a illegal indexing var!");

        icm_args
          = MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE, TRUE, FALSE,
                          MakeTypeArgs (ID_NAME (arg2), ID_NTYPE (arg2), FALSE, TRUE,
                                        FALSE, TBmakeExprs (DUPdupIdNt (arg1), NULL)));

        /* The ICM depends on whether we use the distributed memory backend
         * and the read access is known to be local. */
        char *icm_name = "ND_PRF_SEL_VxA__DATA_id";
        if (global.backend == BE_distmem && PRF_DISTMEMISLOCALREAD (arg_node)) {
            icm_name = "ND_PRF_SEL_VxA__DATA_id_Local";
        }

        ret_node
          = TCmakeAssignIcm3 (icm_name, DUPdoDupTree (icm_args), MakeSizeArg (arg1, TRUE),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_NTYPE (arg2))),
                              NULL);
    } else {
        node *type_args;

        DBUG_ASSERT (NODE_TYPE (arg1) == N_array,
                     "1st arg of F_sel_VxA is neither N_id nor N_array!");

        type_args
          = MakeTypeArgs (ID_NAME (arg2), ID_NTYPE (arg2), FALSE, TRUE, FALSE,
                          TBmakeExprs (MakeSizeArg (arg1, TRUE),
                                       TCappendExprs (DUPdupExprsNt (ARRAY_AELEMS (arg1)),
                                                      NULL)));

        icm_args = MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE, TRUE,
                                 FALSE, type_args);

        /* The ICM depends on whether we use the distributed memory backend
         * and the read access is known to be local. */
        char *icm_name = "ND_PRF_SEL_VxA__DATA_arr";
        if (global.backend == BE_distmem && PRF_DISTMEMISLOCALREAD (arg_node)) {
            icm_name = "ND_PRF_SEL_VxA__DATA_arr_Local";
        }

        ret_node
          = TCmakeAssignIcm2 (icm_name, DUPdoDupTree (icm_args),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_NTYPE (arg2))),
                              NULL);
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * @fn node *COMPprfAll (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
static node *
COMPprfAll (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm2 ("ND_PRF_ALL",
                                 DUPdupIdsIdNt (INFO_LASTIDS (arg_info)),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 NULL);

    DBUG_RETURN (ret_node);
}

/* This is as SIMD version of the selection.  It selects a SIMD verctor,
 * rathter than a scalar.
 */
static node *
COMPsimd_prfSel (node *arg_node, info *arg_info)
{
    node *simd_length, *arg1, *arg2;
    node *let_ids;
    node *icm_args;
    node *ret_node;

    node *base_type_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    simd_length = DUPdoDupTree (PRF_ARG1 (arg_node));
    arg1 = PRF_ARG2 (arg_node);
    arg2 = PRF_ARG3 (arg_node);

    /*
     *   B = sel( iv, A);
     *
     ****************************************************************************
     *
     * For efficiency reasons, constant arrays are excepted as 1st argument of
     * sel() as well:
     *
     *   A = fun( ...);
     *   B = sel( [3,4], A);
     *
     * Here, the backend can avoid the creation of the array containing the shape
     * [3,4].
     */

    DBUG_ASSERT (NODE_TYPE (arg2) == N_id, "2nd arg of F_sel_VxA is no N_id!");

    /* Name of the type of the array, to pass it in the macro.  */
    base_type_node = TCmakeIdCopyString (GetBaseTypeFromExpr (arg2));

    if (NODE_TYPE (arg1) == N_id) {
        DBUG_ASSERT (TUgetSimpleImplementationType (ID_NTYPE (arg1)) == T_int,
                     "1st arg of F_sel_VxA is a illegal indexing var!");

        icm_args
          = MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE, TRUE, FALSE,
                          MakeTypeArgs (ID_NAME (arg2), ID_NTYPE (arg2), FALSE, TRUE,
                                        FALSE, TBmakeExprs (DUPdupIdNt (arg1), NULL)));

        ret_node
          = TCmakeAssignIcm5 ("ND_PRF_SIMD_SEL_VxA__DATA_id", DUPdoDupTree (icm_args),
                              MakeSizeArg (arg1, TRUE),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_NTYPE (arg2))),
                              simd_length, base_type_node, NULL);
    } else {
        node *type_args;

        DBUG_ASSERT (NODE_TYPE (arg1) == N_array,
                     "1st arg of F_sel_VxA is neither N_id nor N_array!");

        type_args
          = MakeTypeArgs (ID_NAME (arg2), ID_NTYPE (arg2), FALSE, TRUE, FALSE,
                          TBmakeExprs (MakeSizeArg (arg1, TRUE),
                                       TCappendExprs (DUPdupExprsNt (ARRAY_AELEMS (arg1)),
                                                      NULL)));

        icm_args = MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE, TRUE,
                                 FALSE, type_args);

        ret_node
          = TCmakeAssignIcm4 ("ND_PRF_SIMD_SEL_VxA__DATA_arr", DUPdoDupTree (icm_args),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_NTYPE (arg2))),
                              simd_length, base_type_node, NULL);
    }

    DBUG_RETURN (ret_node);
}

static node *
COMPsimd_sel_SxS (node *arg_node, info *arg_info)
{
    node *arg2;
    node *let_ids;
    node *ret_node;

    const char *base_type;
    node *id_wrapper;

    DBUG_ENTER ();

    /* Assure that the prf has exactly three arguments */
    // DBUG_ASSERT (prf_arg_number_correct (arg_node, 3),
    //             "Wrong number of arguments found");

    let_ids = INFO_LASTIDS (arg_info);
    arg2 = PRF_ARG2 (arg_node);

    base_type = GetBaseTypeFromExpr (arg2);
    id_wrapper = TBmakeSpid (NULL, STRcpy (base_type));

    ret_node
      = TCmakeAssignIcm4 ("ND_PRF_SIMD_SELSxS__DATA", DUPdupIdsIdNt (let_ids), id_wrapper,
                          /*TCmakeIdCopyString(
                           * prf_ccode_tab[PRF_PRF(arg_node)])*/
                          TBmakeSpid (NULL, STRcpy ("XXX")),
                          DupExprs_NT_AddReadIcms (PRF_ARGS (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

static node *
COMPsimd_modarray (node *arg_node, info *arg_info)
{
    node *arg1;
    node *let_ids;
    node *ret_node;

    const char *base_type;
    node *id_wrapper;

    DBUG_ENTER ();

    /* Assure that the prf has exactly three arguments */
    // DBUG_ASSERT (prf_arg_number_correct (arg_node, 3),
    //             "Wrong number of arguments found");

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);

    base_type = GetBaseTypeFromExpr (arg1);
    id_wrapper = TBmakeSpid (NULL, STRcpy (base_type));

    ret_node
      = TCmakeAssignIcm4 ("ND_PRF_SIMD_MODARRAY", DUPdupIdsIdNt (let_ids), id_wrapper,
                          /*TCmakeIdCopyString(
                           * prf_ccode_tab[PRF_PRF(arg_node)])*/
                          TBmakeSpid (NULL, STRcpy ("XXX")),
                          DupExprs_NT_AddReadIcms (PRF_ARGS (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfSelI( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_sel_VxAI.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfSelI (node *arg_node, info *arg_info)
{
    node *arg1, *arg2;
    node *icm_args;
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    if (NODE_TYPE (arg1) == N_id) {
        DBUG_ASSERT ((TUgetSimpleImplementationType (ID_NTYPE (arg1)) == T_int),
                     "1st arg of F_sel_VxA is a illegal indexing var!");

        icm_args
          = MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE, TRUE, FALSE,
                          MakeTypeArgs (ID_NAME (arg2), ID_NTYPE (arg2), FALSE, TRUE,
                                        FALSE, TBmakeExprs (DUPdupIdNt (arg1), NULL)));

        ret_node
          = TCmakeAssignIcm3 ("ND_PRF_SEL_VxIA__DATA_id", DUPdoDupTree (icm_args),
                              MakeSizeArg (arg1, TRUE),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_NTYPE (arg2))),
                              NULL);
    } else {
        ret_node = (node *)NULL;
        DBUG_UNREACHABLE ("Not an N_id!!");
    }
    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfModarray( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_modarray_AxVxS.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfModarray_AxVxS (node *arg_node, info *arg_info)
{
    node *arg1, *arg2, *arg3;
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    arg3 = PRF_ARG3 (arg_node);

    /*
     *   B = modarray( A, iv, val);
     *
     ****************************************************************************
     *
     * For efficiency reasons, constant arrays are accepted as 2nd argument of
     * modarray() as well:
     *
     *   A = fun( ...);
     *   B = modarray( A, [3,4], val);
     *
     * Here, the backend can avoid the creation of the array containing the shape
     * [3,4].
     */

    DBUG_ASSERT (NODE_TYPE (arg1) == N_id, "1st arg of F_modarray_AxVxS is no N_id!");
    DBUG_ASSERT (NODE_TYPE (arg3) != N_array,
                 "3rd arg of F_modarray_AxVxS is a N_array!");

    if (NODE_TYPE (arg2) == N_id) {
        DBUG_ASSERT ((TUgetSimpleImplementationType (ID_NTYPE (arg2)) == T_int),
                     "2nd arg of F_modarray_AxVxS is a illegal indexing var!");

        ret_node
          = TCmakeAssignIcm5 ("ND_PRF_MODARRAY_AxVxS__DATA_id",
                              MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE,
                                            TRUE, FALSE,
                                            MakeTypeArgs (ID_NAME (arg1), ID_NTYPE (arg1),
                                                          FALSE, TRUE, FALSE, NULL)),
                              DUPdupNodeNt (arg2), MakeSizeArg (arg2, TRUE),
                              DUPdupNodeNt (arg3),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_NTYPE (arg1))),
                              NULL);
    } else {
        DBUG_ASSERT (NODE_TYPE (arg2) == N_array,
                     "2nd arg of F_modarray_AxVxS is neither N_id nor N_array!");

        ret_node
          = TCmakeAssignIcm5 ("ND_PRF_MODARRAY_AxVxS__DATA_arr",
                              MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE,
                                            TRUE, FALSE,
                                            MakeTypeArgs (ID_NAME (arg1), ID_NTYPE (arg1),
                                                          FALSE, TRUE, FALSE, NULL)),
                              MakeSizeArg (arg2, TRUE),
                              DUPdupExprsNt (ARRAY_AELEMS (arg2)), DUPdupNodeNt (arg3),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_NTYPE (arg1))),
                              NULL);
    }

    DBUG_RETURN (ret_node);
}

static node *
COMPprfModarray_AxVxA (node *arg_node, info *arg_info)
{
    node *arg1, *arg2, *arg3;
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    arg3 = PRF_ARG3 (arg_node);

    /*
     *   B = modarray( A, iv, val);
     *
     ****************************************************************************
     *
     * For efficiency reasons, constant arrays are accepted as 2nd argument of
     * modarray() as well:
     *
     *   A = fun( ...);
     *   B = modarray( A, [3,4], val);
     *
     * Here, the backend can avoid the creation of the array containing the shape
     * [3,4].
     */

    DBUG_ASSERT (NODE_TYPE (arg1) == N_id, "1st arg of F_modarray_AxVxA is no N_id!");
    DBUG_ASSERT (NODE_TYPE (arg3) != N_array,
                 "3rd arg of F_modarray_AxVxA is a N_array!");

    if (NODE_TYPE (arg2) == N_id) {
        DBUG_ASSERT ((TUgetSimpleImplementationType (ID_NTYPE (arg2)) == T_int),
                     "2nd arg of F_modarray_AxVxA is a illegal indexing var!");

        ret_node
          = TCmakeAssignIcm5 ("ND_PRF_MODARRAY_AxVxA__DATA_id",
                              MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE,
                                            TRUE, FALSE,
                                            MakeTypeArgs (ID_NAME (arg1), ID_NTYPE (arg1),
                                                          FALSE, TRUE, FALSE, NULL)),
                              DUPdupNodeNt (arg2), MakeSizeArg (arg2, TRUE),
                              DUPdupNodeNt (arg3),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_NTYPE (arg1))),
                              NULL);
    } else {
        DBUG_ASSERT (NODE_TYPE (arg2) == N_array,
                     "2nd arg of F_modarray_AxVxA is neither N_id nor N_array!");

        ret_node
          = TCmakeAssignIcm5 ("ND_PRF_MODARRAY_AxVxA__DATA_arr",
                              MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE,
                                            TRUE, FALSE,
                                            MakeTypeArgs (ID_NAME (arg1), ID_NTYPE (arg1),
                                                          FALSE, TRUE, FALSE, NULL)),
                              MakeSizeArg (arg2, TRUE),
                              DUPdupExprsNt (ARRAY_AELEMS (arg2)), DUPdupNodeNt (arg3),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_NTYPE (arg1))),
                              NULL);
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfGenarray( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_genarray.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfGenarray (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    DBUG_UNREACHABLE ("prf F_genarray not implemented yet!");

    ret_node = NULL;

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfTake( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_take_SxV.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfTake (node *arg_node, info *arg_info)
{
    node *arg1, *arg2;
    node *let_ids;
    node *icm_args;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT (((NODE_TYPE (arg1) == N_id) || (NODE_TYPE (arg1) == N_num)),
                 "1st arg of F_take_SxV is neither N_id nor N_num!");
    DBUG_ASSERT (NODE_TYPE (arg2) == N_id, "2nd arg of F_take_SxV is no N_id!");

    icm_args
      = MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE, TRUE, FALSE,
                      MakeTypeArgs (ID_NAME (arg2), ID_NTYPE (arg2), FALSE, TRUE, FALSE,
                                    TBmakeExprs (DUPdupNodeNt (arg1), NULL)));

    ret_node
      = TCmakeAssignIcm2 ("ND_PRF_TAKE_SxV__DATA", DUPdoDupTree (icm_args),
                          TCmakeIdCopyString (GenericFun (GF_copy, ID_NTYPE (arg2))),
                          NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfDrop( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_drop_SxV.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfDrop (node *arg_node, info *arg_info)
{
    node *arg1, *arg2;
    node *let_ids;
    node *icm_args;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT (((NODE_TYPE (arg1) == N_id) || (NODE_TYPE (arg1) == N_num)),
                 "1st arg of F_drop_SxV is neither N_id nor N_num!");
    DBUG_ASSERT (NODE_TYPE (arg2) == N_id, "2nd arg of F_drop_SxV is no N_id!");

    icm_args
      = MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE, TRUE, FALSE,
                      MakeTypeArgs (ID_NAME (arg2), ID_NTYPE (arg2), FALSE, TRUE, FALSE,
                                    TBmakeExprs (DUPdupNodeNt (arg1), NULL)));

    ret_node
      = TCmakeAssignIcm2 ("ND_PRF_DROP_SxV__DATA", DUPdoDupTree (icm_args),
                          TCmakeIdCopyString (GenericFun (GF_copy, ID_NTYPE (arg2))),
                          NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfCat( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_cat_VxV.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfCat (node *arg_node, info *arg_info)
{
    node *arg1, *arg2;
    node *let_ids;
    node *icm_args;
    char *copyfun1, *copyfun2;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT (NODE_TYPE (arg1) == N_id, "1st arg of F_cat_VxV is no N_id!");
    DBUG_ASSERT (NODE_TYPE (arg2) == N_id, "2nd arg of F_cat_VxV is no N_id!");

    icm_args
      = MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids), FALSE, TRUE, FALSE,
                      MakeTypeArgs (ID_NAME (arg1), ID_NTYPE (arg1), FALSE, TRUE, FALSE,
                                    MakeTypeArgs (ID_NAME (arg2), ID_NTYPE (arg2), FALSE,
                                                  TRUE, FALSE, NULL)));

    copyfun1 = GenericFun (GF_copy, ID_NTYPE (arg1));
    copyfun2 = GenericFun (GF_copy, ID_NTYPE (arg2));
    DBUG_ASSERT ((((copyfun1 == NULL) && (copyfun2 == NULL))
                  || STReq (copyfun1, copyfun2)),
                 "F_cat_VxV: different copyfuns found!");

    ret_node = TCmakeAssignIcm2 ("ND_PRF_CAT_VxV__DATA", DUPdoDupTree (icm_args),
                                 TCmakeIdCopyString (copyfun1), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfOp_S( node *arg_node, info *arg_info)
 *
 * @brief  Compiles a unary scalar N_prf node into a ND_PRF_S__DATA-icm.
 *         The return value is a N_assign chain of ICMs.
 *         Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfOp_S (node *arg_node, info *arg_info)
{
    node *arg;
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    /* assure that the prf has exactly one argument */
    DBUG_ASSERT (PRF_EXPRS2 (arg_node) == NULL, "more than a single argument found!");
    DBUG_ASSERT (NODE_TYPE (arg) != N_id || TUgetFullDimEncoding (ID_NTYPE (arg)) == SCALAR,
                 "non-scalar argument `%s' found!", global.prf_name[PRF_PRF (arg_node)]);

    /* If enforce float flag is set, we change all tods to tofs */
    if (global.enforce_float && PRF_PRF (arg_node) == F_tod_S) {
        ret_node = TCmakeAssignIcm3 ("ND_PRF_S__DATA", DUPdupIdsIdNt (let_ids),
                                     TCmakeIdCopyString (prf_ccode_tab[F_tof_S]),
                                     DupExprs_NT_AddReadIcms (PRF_ARGS (arg_node)), NULL);
    } else {
        if (PRF_PRF (arg_node) == F_abs_S && TUisUnsigned (ID_NTYPE (arg))) {
            ret_node
              = TCmakeAssignIcm3 ("ND_PRF_S__DATA", DUPdupIdsIdNt (let_ids),
                                  TCmakeIdCopyString ("SAC_ND_PRF_ID"),
                                  DupExprs_NT_AddReadIcms (PRF_ARGS (arg_node)), NULL);
        } else {
            ret_node
              = TCmakeAssignIcm3 ("ND_PRF_S__DATA", DUPdupIdsIdNt (let_ids),
                                  TCmakeIdCopyString (prf_ccode_tab[PRF_PRF (arg_node)]),
                                  DupExprs_NT_AddReadIcms (PRF_ARGS (arg_node)), NULL);
        }
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfOp_V( char *icm_name,
 *                         node *arg_node, info *arg_info)
 *
 * @brief  Compiles a unary scalar N_prf node into a ND_PRF_V__DATA-icm.
 *         The return value is a N_assign chain of ICMs.
 *         Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfOp_V (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    /* assure that the prf has exactly one argument */
    DBUG_ASSERT (PRF_EXPRS2 (arg_node) == NULL, "more than a single argument found!");
    ret_node = TCmakeAssignIcm3 ("ND_PRF_V__DATA", DUPdupIdsIdNt (let_ids),
                                 TCmakeIdCopyString (prf_ccode_tab[PRF_PRF (arg_node)]),
                                 DupExprs_NT_AddReadIcms (PRF_ARGS (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/* This function suppose to catch all the cases when the binary prf was called
 * with SIMD arguments.  Hopefully all the cases...
 */
static inline bool
is_simd_type (node *n)
{
    if (NODE_TYPE (n) == N_floatvec)
        return TRUE;

    if (NODE_TYPE (n) == N_exprs)
        n = EXPRS_EXPR (n);

    if (NODE_TYPE (n) == N_id) {
        node *av = AVIS_DECL (ID_AVIS (n));
        ntype *type = NULL;
        if (NODE_TYPE (av) == N_vardec)
            type = VARDEC_NTYPE (av);
        else if (NODE_TYPE (av) == N_arg)
            type = ARG_NTYPE (av);
        else
            DBUG_UNREACHABLE ("unexpected node type of avis");
        return TUgetSimpleImplementationType (type) == T_floatvec;
    }

    DBUG_ASSERT (NODE_TYPE (n) != N_ids, "N_ids in binary prf -- WTF?  O_o");
    return FALSE;
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfOp_SxS( node *arg_node, info *arg_info)
 *
 * @brief  Compiles a binary N_prf node into a ND_PRF_?x?__DATA-icm.
 *         The return value is a N_assign chain of ICMs.
 *         Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfOp_SxS (node *arg_node, info *arg_info)
{
    node *arg1, *arg2;
    node *let_ids;
    node *ret_node;
    simpletype stype;
    char *prf_orig_name = prf_ccode_tab[PRF_PRF (arg_node)];
    char *prf_name = prf_orig_name;
    char *ty_str;

    DBUG_ENTER ();

    /* assure that the prf has exactly two arguments */
    DBUG_ASSERT (((PRF_EXPRS1 (arg_node) != NULL) && (PRF_EXPRS2 (arg_node) != NULL)
                  && (PRF_EXPRS3 (arg_node) == NULL)),
                 "illegal number of args found!");

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT (((NODE_TYPE (arg1) != N_id)
                  || (TUgetFullDimEncoding (ID_NTYPE (arg1)) == SCALAR)),
                 "%s: non-scalar first argument found!",
                 global.prf_name[PRF_PRF (arg_node)]);

    DBUG_ASSERT (((NODE_TYPE (arg2) != N_id)
                  || (TUgetFullDimEncoding (ID_NTYPE (arg2)) == SCALAR)),
                 "%s: non-scalar second argument found!",
                 global.prf_name[PRF_PRF (arg_node)]);

    /* Here we have a special case for floatvec.  We are going to append
     * _SIMD prefix to the name of prf we generate.  The current implementation
     * is more of a hack.  FIXME implement it properly.
     */
    if (is_simd_type (arg1)) {
        prf_name = MEMmalloc (strlen (prf_orig_name) + strlen ("_SIMD") + 1);
        sprintf (prf_name, "%s%s", prf_orig_name, "_SIMD");
    }

    /*
     * For -profile o, we need to make sure that we provide two pieces
     * of information: the basic type the operation is working on and
     * the operation itself.
     * The operation itself is encoded in TCmakeIdCopyString (prf_name)
     * already, which is used to evetually expand a macro of that name
     * for the actual code generation.
     * Here, we extract the type from the first argument.
     * NB: bear in mind, we are on the old type representation here :-(
     * The encoding of the type here is similar BUT NOT IDENTICAL
     * to simpletype. simpletype does not exist in the runtime system.
     * Instead, we have a custom enum type that lives in
     * libsac/profile/profile_ops.h !
     */
    if (NODE_TYPE (arg1) == N_num) {
        ty_str = "T_int";
    } else if (NODE_TYPE (arg1) == N_float) {
        ty_str = "T_float";
    } else if (NODE_TYPE (arg1) == N_double) {
        ty_str = "T_double";
    } else if (NODE_TYPE (arg1) == N_id) {
      stype = TUgetSimpleImplementationType (ID_NTYPE (arg1));
      if (stype == T_int) {
        ty_str = "T_int";
      } else if (stype == T_float) {
        ty_str = "T_float";
      } else if (stype == T_double) {
        ty_str = "T_double";
      } else {
        ty_str = "T_ignore";
      }
    } else {
      ty_str = "T_ignore";
    }

    ret_node = TCmakeAssignIcm4 ("ND_PRF_SxS__DATA", DUPdupIdsIdNt (let_ids),
                                 TCmakeIdCopyString (ty_str),
                                 TCmakeIdCopyString (prf_name),
                                 DupExprs_NT_AddReadIcms (PRF_ARGS (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfOp_SxV( node *arg_node, info *arg_info)
 *
 * @brief  Compiles a binary N_prf node into a ND_PRF_?x?__DATA-icm.
 *         The return value is a N_assign chain of ICMs.
 *         Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfOp_SxV (node *arg_node, info *arg_info)
{
    node *arg1;
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ();

    /* assure that the prf has exactly two arguments */
    DBUG_ASSERT (((PRF_EXPRS1 (arg_node) != NULL) && (PRF_EXPRS2 (arg_node) != NULL)
                  && (PRF_EXPRS3 (arg_node) == NULL)),
                 "illegal number of args found!");

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);

    DBUG_ASSERT (((NODE_TYPE (arg1) != N_id)
                  || (TUgetFullDimEncoding (ID_NTYPE (arg1)) == SCALAR)),
                 "%s: non-scalar first argument found!",
                 global.prf_name[PRF_PRF (arg_node)]);

    ret_node = TCmakeAssignIcm3 ("ND_PRF_SxV__DATA", DUPdupIdsIdNt (let_ids),
                                 TCmakeIdCopyString (prf_ccode_tab[PRF_PRF (arg_node)]),
                                 DupExprs_NT_AddReadIcms (PRF_ARGS (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfOp_VxS( node *arg_node, info *arg_info)
 *
 * @brief  Compiles a binary N_prf node into a ND_PRF_?x?__DATA-icm.
 *         The return value is a N_assign chain of ICMs.
 *         Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfOp_VxS (node *arg_node, info *arg_info)
{
    node *arg2;
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ();

    /* assure that the prf has exactly two arguments */
    DBUG_ASSERT (((PRF_EXPRS1 (arg_node) != NULL) && (PRF_EXPRS2 (arg_node) != NULL)
                  && (PRF_EXPRS3 (arg_node) == NULL)),
                 "illegal number of args found!");

    let_ids = INFO_LASTIDS (arg_info);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT (((NODE_TYPE (arg2) != N_id)
                  || (TUgetFullDimEncoding (ID_NTYPE (arg2)) == SCALAR)),
                 "%s: non-scalar second argument found!",
                 global.prf_name[PRF_PRF (arg_node)]);
    ret_node = TCmakeAssignIcm3 ("ND_PRF_VxS__DATA", DUPdupIdsIdNt (let_ids),
                                 TCmakeIdCopyString (prf_ccode_tab[PRF_PRF (arg_node)]),
                                 DupExprs_NT_AddReadIcms (PRF_ARGS (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfOp_VxV( node *arg_node, info *arg_info)
 *
 * @brief  Compiles a binary N_prf node into a ND_PRF_?x?__DATA-icm.
 *         The return value is a N_assign chain of ICMs.
 *         Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfOp_VxV (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ();

    /* assure that the prf has exactly two arguments */
    DBUG_ASSERT (((PRF_EXPRS1 (arg_node) != NULL) && (PRF_EXPRS2 (arg_node) != NULL)
                  && (PRF_EXPRS3 (arg_node) == NULL)),
                 "illegal number of args found!");

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm3 ("ND_PRF_VxV__DATA", DUPdupIdsIdNt (let_ids),
                                 TCmakeIdCopyString (prf_ccode_tab[PRF_PRF (arg_node)]),
                                 DupExprs_NT_AddReadIcms (PRF_ARGS (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/* FIXME Enforce using of the following function across the entire file.  */
static inline bool
prf_arg_number_correct (node *arg_node, size_t num_args)
{
    size_t i;
    node *args = PRF_ARGS (arg_node);

    for (i = 0; i < num_args; i++) {
        if (!args)
            return FALSE;

        args = EXPRS_NEXT (args);
    }

    return args == NULL;
}

static node *
COMPprfOp_SMxSM (node *arg_node, info *arg_info)
{
    node *arg2;
    node *let_ids;
    node *ret_node;

    const char *base_type;
    node *id_wrapper;

    DBUG_ENTER ();

    /* Assure that the prf has exactly three arguments */
    DBUG_ASSERT (prf_arg_number_correct (arg_node, 3), "Wrong number of arguments found");

    let_ids = INFO_LASTIDS (arg_info);
    arg2 = PRF_ARG2 (arg_node);

    base_type = GetBaseTypeFromExpr (arg2);
    id_wrapper = TBmakeSpid (NULL, STRcpy (base_type));

    ret_node
      = TCmakeAssignIcm4 ("ND_PRF_SMxSM__DATA", DUPdupIdsIdNt (let_ids), id_wrapper,
                          TCmakeIdCopyString (prf_ccode_tab[PRF_PRF (arg_node)]),
                          DupExprs_NT_AddReadIcms (PRF_ARGS (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfTypeError( node *arg_node, info *arg_info)
 *
 * @brief  ...
 *
 ******************************************************************************/

static node *
COMPprfTypeError (node *arg_node, info *arg_info)
{
    node *bottom;
    node *message;
    node *ret_node;

    DBUG_ENTER ();

    DBUG_ASSERT (PRF_ARGS (arg_node) != NULL, "1st argument of F_type_error not found!");

    DBUG_ASSERT (NODE_TYPE (PRF_ARG1 (arg_node)) == N_type,
                 "1st argument of F_type_error  not a N_type node!");

    bottom = EXPRS_EXPR (PRF_ARGS (arg_node));

    DBUG_ASSERT (TYisBottom (TYPE_TYPE (bottom)),
                 "1st argument of F_type_error contains non bottom type!");

    message = TBmakeStr (STRstring2SafeCEncoding (TYgetBottomError (TYPE_TYPE (bottom))));

    ret_node = TCmakeAssignIcm1 ("TYPE_ERROR", message, NULL);

    DBUG_RETURN (ret_node);
}

/** <!--*******************************************************************-->
 *
 * @fn COMPprfWrapperShapeEncode( node *arg_node, info *arg_info)
 *
 * @brief Creates the ICM's for encoding shape information at runtime. This is
 * needed by the wrapper entry function when using runtime specialization.
 *
 * @return A chain of assignment nodes.
 *
 ****************************************************************************/
static node *
COMPprfWrapperShapeEncode (node *arg_node, info *arg_info)
{
    node *assigns = NULL;
    node *args;

    DBUG_ENTER ();

    args = PRF_ARGS (arg_node);

    if (args == NULL) {
        DBUG_PRINT_TAG ("RTSPEC", "Arguments are NULL!");
        assigns = TCmakeAssignIcm1 ("WE_NO_SHAPE_ENCODE", TBmakeNumuint (0), NULL);
    } else {
        assigns = TCmakeAssignIcm1 ("WE_SHAPE_ENCODE",
                                    TBmakeExprs (TBmakeNumuint (TCcountExprs (args)),
                                                 DUPdupExprsNt (args)),
                                    NULL);
    }

    DBUG_RETURN (assigns);
}

/** <!--*******************************************************************-->
 *
 * @fn COMPprfWrapperModFunInfo( node *arg_node, info *arg_info)
 *
 * @brief Creates the ICM's that hold the name of the function and module. This
 * is needed by the wrapper entry when using runtime specialization.
 *
 * @return A chain of assignment nodes.
 *
 ****************************************************************************/
static node *
COMPprfWrapperModFunInfo (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *args;
    node *funname;
    node *modname;

    DBUG_ENTER ();

    args = PRF_ARGS (arg_node);
    funname = EXPRS_EXPR (args);
    modname = EXPRS_EXPR (EXPRS_NEXT (args));

    DBUG_ASSERT (funname != NULL && modname != NULL, "Missing naming information!!!");

    ret_node
      = TCmakeAssignIcm2 ("WE_MODFUN_INFO", TBmakeStr (STRcpy (STR_STRING (funname))),
                          TBmakeStr (STRcpy (STR_STRING (modname))), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfDispatchError( node *arg_node, info *arg_info)
 *
 * @brief  ...
 *
 ******************************************************************************/

static node *
COMPprfDispatchError (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *args;
    int skip;
    node *funname, *funargs;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    DBUG_ASSERT (PRF_ARGS (arg_node) != NULL,
                 "1st argument of F_dispatch_error not found!");

    /*
     * ARG1 is the number of types to ignore, so we skip ARG1 + 1
     * arguments...
     */
    args = PRF_ARGS (arg_node);

    DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (args)) == N_num, "N_num expected as 1st arg!");

    skip = NUM_VAL (EXPRS_EXPR (args)) + 1;

    while (skip != 0) {
        skip--;
        args = EXPRS_NEXT (args);
    }

    funname = EXPRS_EXPR (args);
    funargs = EXPRS_NEXT (args);

    ret_node = TCmakeAssignIcm5 ("DISPATCH_ERROR", TBmakeNumuint (TCcountIds (let_ids)),
                                 TCids2ExprsNt (let_ids), DUPdoDupNode (funname),
                                 TBmakeNumuint (TCcountExprs (funargs)),
                                 DUPdupExprsNt (funargs), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfNoop( node *arg_node, info *arg_info)
 *
 * @brief  ...
 *
 ******************************************************************************/

static node *
COMPprfNoop (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm0 ("NOOP", NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfUnshare( node *arg_node, info *arg_info)
 *
 * @brief  Compile the F_unshare primitive function.
 *
 *  Input:
 *      v = unshare( a, iv1,...,ivn);
 *
 *  is compiled into (approx):
 *      ND_UNSHARE( a, ivn);
 *      ...
 *      ND_UNSHARE( a, iv1);
 *      ND_ASSIGN( v, a);
 *
 ******************************************************************************/

static node *
COMPprfUnshare (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ();

    /* generate the assignment ND_ASSIGN( v, a); */
    ret_node = RhsId (PRF_ARG1 (arg_node), arg_info);

    node *accu_id = PRF_ARG1 (arg_node);

    /* walk over the args2..n and generate ND_UNSHARE icm nodes */
    for (node *prfargs = PRF_EXPRS2 (arg_node); prfargs != NULL;
         prfargs = EXPRS_NEXT (prfargs)) {
        /* get the indev-vector id */
        node *iv_id = EXPRS_EXPR (prfargs);

        ret_node
          = TCmakeAssignIcm4 ("ND_UNSHARE", /* C-ICM */
                              MakeTypeArgs (ID_NAME (accu_id), ID_NTYPE (accu_id), FALSE,
                                            TRUE, FALSE, NULL),
                              MakeTypeArgs (ID_NAME (iv_id), ID_NTYPE (iv_id), FALSE, TRUE,
                                            FALSE, NULL),
                              MakeBasetypeArg (ID_NTYPE (iv_id)),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_NTYPE (iv_id))),
                              ret_node);
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfIdxs2Offset( node *arg_node, info *arg_info)
 *
 * @brief  ...
 *
 ******************************************************************************/

static node *
COMPprfIdxs2Offset (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *icm = NULL;
    node *idxs_exprs;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    idxs_exprs = EXPRS_NEXT (PRF_ARGS (arg_node));

    /*
     * either pass the ID of the vector or the
     * IDs of the shape elements.
     */
    if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_array) {
        icm
          = TCmakeIcm5 ("ND_IDXS2OFFSET_arr", DUPdupIdsIdNt (let_ids),
                        TBmakeNum (TCcountExprs (idxs_exprs)), DUPdupExprsNt (idxs_exprs),
                        MakeSizeArg (PRF_ARG1 (arg_node), TRUE),
                        DupExprs_NT_AddReadIcms (ARRAY_AELEMS (PRF_ARG1 (arg_node))));
    } else if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id) {
        icm
          = TCmakeIcm5 ("ND_IDXS2OFFSET_id", DUPdupIdsIdNt (let_ids),
                        TBmakeNum (TCcountExprs (idxs_exprs)), DUPdupExprsNt (idxs_exprs),
                        MakeSizeArg (PRF_ARG1 (arg_node), TRUE),
                        DUPdupIdNt (PRF_ARG1 (arg_node)));
#ifndef DBUG_OFF
    } else {
        DBUG_UNREACHABLE ("unexpected 1st arg to idxs2offset");
#endif
    }

    DBUG_RETURN (TBmakeAssign (icm, NULL));
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfArrayIdxs2Offset( node *arg_node, info *arg_info)
 *
 * @brief  ...
 *
 ******************************************************************************/

static node *
COMPprfArrayIdxs2Offset (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *icm = NULL;
    node *array;
    node *idxs_exprs;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    array = PRF_ARG1 (arg_node);
    idxs_exprs = EXPRS_NEXT (PRF_ARGS (arg_node));

    DBUG_ASSERT (NODE_TYPE (array) == N_id,
                 "First argument of F_array_idxs2offset must be an N_id Node!");

    icm = TCmakeIcm5 ("ND_ARRAY_IDXS2OFFSET_id", DUPdupIdsIdNt (let_ids),
                      TBmakeNum (TCcountExprs (idxs_exprs)), DUPdupExprsNt (idxs_exprs),
                      MakeDimArg (PRF_ARG1 (arg_node), TRUE),
                      DUPdupIdNt (PRF_ARG1 (arg_node)));

    DBUG_RETURN (TBmakeAssign (icm, NULL));
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfArrayIdxs2Offset( node *arg_node, info *arg_info)
 *
 * @brief  ...
 *
 ******************************************************************************/

static node *
COMPprfArrayVect2Offset (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *icm = NULL;
    node *array;
    node *iv_vect;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    array = PRF_ARG1 (arg_node);
    iv_vect = PRF_ARG2 (arg_node);

    DBUG_ASSERT (NODE_TYPE (array) == N_id,
                 "First argument of F_array_vect2offset must be an N_id Node!");

    icm = TCmakeIcm5 ("ND_ARRAY_VECT2OFFSET_id", DUPdupIdsIdNt (let_ids),
                      TBmakeNum (TUgetLengthEncoding (ID_NTYPE (iv_vect))),
                      DUPdupIdNt (iv_vect), MakeDimArg (PRF_ARG1 (arg_node), TRUE),
                      DUPdupIdNt (PRF_ARG1 (arg_node)));
    /*
        icm = TCmakeIcm5( "ND_VECT2OFFSET_id",
                          DUPdupIdsIdNt( let_ids),
                          TBmakeNum( TUgetLengthEncoding (ID_NTYPE( iv_vect))),
                          DUPdupIdNt( iv_vect),
                          MakeSizeArg( PRF_ARG1( arg_node), TRUE),
                          DUPdupIdNt( PRF_ARG1( arg_node)));
    */

    DBUG_RETURN (TBmakeAssign (icm, NULL));
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfVect2Offset( node *arg_node, info *arg_info)
 *
 * @brief  ...
 *
 ******************************************************************************/

static node *
COMPprfVect2Offset (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *iv_vect;
    node *icm = NULL;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    iv_vect = PRF_ARG2 (arg_node);

    /*
     * either pass the ID of the vector or the
     * IDs of the shape elements.
     */
    if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_array) {
        icm = TCmakeIcm5 ("ND_VECT2OFFSET_arr", DUPdupIdsIdNt (let_ids),
                          TBmakeNum (TUgetLengthEncoding (ID_NTYPE (iv_vect))),
                          DUPdupIdNt (iv_vect), MakeSizeArg (PRF_ARG1 (arg_node), TRUE),
                          DupExprs_NT_AddReadIcms (ARRAY_AELEMS (PRF_ARG1 (arg_node))));
    } else if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id) {
        icm = TCmakeIcm5 ("ND_VECT2OFFSET_id", DUPdupIdsIdNt (let_ids),
                          TBmakeNum (TUgetLengthEncoding (ID_NTYPE (iv_vect))),
                          DUPdupIdNt (iv_vect), MakeSizeArg (PRF_ARG1 (arg_node), TRUE),
                          DUPdupIdNt (PRF_ARG1 (arg_node)));
#ifndef DBUG_OFF
    } else {
        DBUG_UNREACHABLE ("unexpected 1st arg to vect2offset");
#endif
    }

    DBUG_RETURN (TBmakeAssign (icm, NULL));
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfRunMtGenarray( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_run_mt_genarray, F_run_mt_modarray
 *   and F_run_mt_fold.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfRunMtGenarray (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *ret_node;
    node *mem_id, *thresh_num;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    mem_id = EXPRS_EXPR (PRF_EXPRS1 (arg_node));
    thresh_num = EXPRS_EXPR (PRF_EXPRS2 (arg_node));

    ret_node = TCmakeAssignIcm3 ("ND_PRF_RUNMT_GENARRAY__DATA",
                                 /* result of the test: bool scalar */
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                               FALSE, FALSE, FALSE, NULL),
                                 /* N_id from GENARRAY_MEM: */
                                 MakeTypeArgs (ID_NAME (mem_id), ID_NTYPE (mem_id), FALSE,
                                               FALSE, FALSE, NULL),
                                 /* minimal parallel size.
                                    This is just global.min_parallel_size */
                                 DUPdoDupNode (thresh_num), NULL);
    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfRunMtModarray( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_run_mt_genarray, F_run_mt_modarray
 *   and F_run_mt_fold.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfRunMtModarray (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *ret_node;
    node *mem_id, *thresh_num;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    mem_id = EXPRS_EXPR (PRF_EXPRS1 (arg_node));
    thresh_num = EXPRS_EXPR (PRF_EXPRS2 (arg_node));

    ret_node = TCmakeAssignIcm3 ("ND_PRF_RUNMT_MODARRAY__DATA",
                                 /* result of the test: bool scalar */
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                               FALSE, FALSE, FALSE, NULL),
                                 /* N_id from MODARRAY_MEM: */
                                 MakeTypeArgs (ID_NAME (mem_id), ID_NTYPE (mem_id), FALSE,
                                               FALSE, FALSE, NULL),
                                 /* minimal parallel size.
                                    This is just global.min_parallel_size */
                                 DUPdoDupNode (thresh_num), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprfRunMtFold( node *arg_node, info *arg_info)
 *
 * @brief  Compiles N_prf node of type F_run_mt_genarray, F_run_mt_modarray
 *   and F_run_mt_fold.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPprfRunMtFold (node *arg_node, info *arg_info)
{
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm1 ("ND_PRF_RUNMT_FOLD__DATA",
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_NTYPE (let_ids),
                                               FALSE, TRUE, FALSE, NULL),
                                 NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfMask_SxSxS( node *arg_node, info *arg_info)
 *
 * @brief This mask() entry looks more like ND_PRF_SxS, because the arguments
 *        and results are scalars.
 *
 *****************************************************************************/
node *
COMPprfMask_SxSxS (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm4 ("ND_PRF_MASK_SxSxS__DATA", DUPdupIdsIdNt (let_ids),
                                 DUPdupNodeNt (PRF_ARG1 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG2 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG3 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfMask_SxSxV( node *arg_node, info *arg_info)
 *
 *
 *****************************************************************************/
node *
COMPprfMask_SxSxV (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm4 ("ND_PRF_MASK_SxSxV__DATA", DUPdupIdsIdNt (let_ids),
                                 DUPdupNodeNt (PRF_ARG1 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG2 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG3 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfMask_SxVxS( node *arg_node, info *arg_info)
 *
 *
 *****************************************************************************/
node *
COMPprfMask_SxVxS (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm4 ("ND_PRF_MASK_SxVxS__DATA", DUPdupIdsIdNt (let_ids),
                                 DUPdupNodeNt (PRF_ARG1 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG2 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG3 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfMask SxVxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COMPprfMask_SxVxV (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm4 ("ND_PRF_MASK_SxVxV__DATA", DUPdupIdsIdNt (let_ids),
                                 DUPdupNodeNt (PRF_ARG1 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG2 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG3 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfMask_VxSxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COMPprfMask_VxSxS (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm4 ("ND_PRF_MASK_VxSxS__DATA", DUPdupIdsIdNt (let_ids),
                                 DUPdupNodeNt (PRF_ARG1 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG2 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG3 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfMask_VxSxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COMPprfMask_VxSxV (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm4 ("ND_PRF_MASK_VxSxV__DATA", DUPdupIdsIdNt (let_ids),
                                 DUPdupNodeNt (PRF_ARG1 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG2 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG3 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfMask_VxVxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COMPprfMask_VxVxS (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm4 ("ND_PRF_MASK_VxVxS__DATA", DUPdupIdsIdNt (let_ids),
                                 DUPdupNodeNt (PRF_ARG1 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG2 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG3 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfMask_VxVxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COMPprfMask_VxVxV (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm4 ("ND_PRF_MASK_VxVxV__DATA", DUPdupIdsIdNt (let_ids),
                                 DUPdupNodeNt (PRF_ARG1 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG2 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG3 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * @fn node *COMPprfGuard (node *arg_node, info *arg_info)
 *
 * @brief markmemvals has already modified guards such that the returned
 * arguments are named the same as the inputs, e.g.
 *   x1, .., xn = (n, x1, .., xn, p1, .., pm);
 * Therefore an assignment is no longer necessary, so all we need to do is to
 * check whether all predicates pi hold, and give an error otherwise.
 *
 ******************************************************************************/
static node *
COMPprfGuard (node *arg_node, info *arg_info)
{
    size_t num_rets;
    node *preds, *guard;
    node *ret_node = NULL;

    DBUG_ENTER ();

    num_rets = PRF_NUMVARIABLERETS (arg_node);
    DBUG_ASSERT (num_rets > 0, "guard has no return values");
    preds = TCgetNthExprs (num_rets, PRF_ARGS (arg_node));

    while (preds != NULL) {
        guard = TCmakeAssignIcm1 ("ND_PRF_GUARD",
                                  DUPdupNodeNt (EXPRS_EXPR (preds)),
                                  NULL);
        ret_node = TCappendAssign (ret_node, guard);
        preds = EXPRS_NEXT (preds);
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfTypeConstraint( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COMPprfTypeConstraint (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;
    ntype *arg_type;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg_type = TYPE_TYPE (PRF_ARG1 (arg_node));

    if (TYisAKV (arg_type)) {
        DBUG_UNREACHABLE ("Type constraint with AKV type not implemented");

        ret_node = NULL;
    } else if (TYisAKS (arg_type)) {
        ret_node = SHshape2Array (TYgetShape (arg_type));
        ret_node
          = TCmakeAssignIcm4 ("ND_PRF_TYPE_CONSTRAINT_AKS", DUPdupIdsIdNt (let_ids),
                              DUPdupIdNt (PRF_ARG2 (arg_node)),
                              MakeSizeArg (ret_node, TRUE), ARRAY_AELEMS (ret_node),
                              NULL);
    } else if (TYisAKD (arg_type)) {
        ret_node
          = TCmakeAssignIcm3 ("ND_PRF_TYPE_CONSTRAINT_AKD", DUPdupIdsIdNt (let_ids),
                              DUPdupIdNt (PRF_ARG2 (arg_node)),
                              TBmakeNum (TYgetDim (arg_type)), NULL);
    } else if (TYisAUDGZ (arg_type)) {
        ret_node
          = TCmakeAssignIcm2 ("ND_PRF_TYPE_CONSTRAINT_AUDGZ", DUPdupIdsIdNt (let_ids),
                              DUPdupIdNt (PRF_ARG2 (arg_node)), NULL);

    } else {
        /* TYisAUD is always true */
        ret_node = TCmakeAssignIcm2 ("ND_CREATE__SCALAR", DUPdupIdsIdNt (let_ids),
                                     TBmakeBool (TRUE), NULL);
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfSameShape( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COMPprfSameShape (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node
      = TCmakeAssignIcm5 ("ND_PRF_SAME_SHAPE", DUPdupIdsIdNt (let_ids),
                          DUPdupIdNt (PRF_ARG1 (arg_node)),
                          TBmakeNum (TUgetFullDimEncoding (ID_NTYPE (PRF_ARG1 (arg_node)))),
                          DUPdupIdNt (PRF_ARG2 (arg_node)),
                          TBmakeNum (TUgetFullDimEncoding (ID_NTYPE (PRF_ARG2 (arg_node)))),
                          NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfShapeMatchesDim( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COMPprfShapeMatchesDim (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm3 ("ND_PRF_SHAPE_MATCHES_DIM", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 DUPdupIdNt (PRF_ARG2 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfNonNegVal_S( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COMPprfNonNegVal_S (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm2 ("ND_PRF_NON_NEG_VAL_S", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfNonNegVal_V( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COMPprfNonNegVal_V (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm2 ("ND_PRF_NON_NEG_VAL_V", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfValLtShape( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COMPprfValLtShape (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node
      = TCmakeAssignIcm4 ("ND_PRF_VAL_LT_SHAPE_VxA", DUPdupIdsIdNt (let_ids),
                          DUPdupIdNt (PRF_ARG1 (arg_node)),
                          DUPdupIdNt (PRF_ARG2 (arg_node)),
                          TBmakeNum (TUgetFullDimEncoding (ID_NTYPE (PRF_ARG2 (arg_node)))),
                          NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfValLtVal_SxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COMPprfValLtVal_SxS (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm3 ("ND_PRF_VAL_LT_VAL_SxS", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 DUPdupIdNt (PRF_ARG2 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfValLeVal_SxS( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COMPprfValLeVal_SxS (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm3 ("ND_PRF_VAL_LE_VAL_SxS", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 DUPdupIdNt (PRF_ARG2 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfValLeVal_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COMPprfValLeVal_VxV (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm3 ("ND_PRF_VAL_LE_VAL_VxV", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 DUPdupIdNt (PRF_ARG2 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfProdMatchesShape( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COMPprfProdMatchesProdShape (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node
      = TCmakeAssignIcm4 ("ND_PRF_PROD_MATCHES_PROD_SHAPE", DUPdupIdsIdNt (let_ids),
                          DUPdupIdNt (PRF_ARG1 (arg_node)),
                          DUPdupIdNt (PRF_ARG2 (arg_node)),
                          TBmakeNum (TUgetFullDimEncoding (ID_NTYPE (PRF_ARG2 (arg_node)))),
                          NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfPrefetch2Host (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    /* As the cudamemprefetch primitive aliases it's input,
     * we need to perform an assignment of the lhr by the
     * rhs, so we generate the assignment ND_ASSIGN( v, a);
     */
    ret_node = RhsId (PRF_ARG1 (arg_node), arg_info);

    /* and precede this by the actual prefetch ICM call */
    ret_node = TCmakeAssignIcm3 ("CUDA_MEM_PREFETCH",
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 MakeBasetypeArg (ID_NTYPE (PRF_ARG1 (arg_node))),
                                 TBmakeNum (-1), ret_node);

    DBUG_RETURN (ret_node);
}

node *
COMPprfPrefetch2Device (node *arg_node, info *arg_info)
{
    /**
     * At the moment we assume that the device is *always* at
     * ordinal 0. In a multi-GPU setup this may not be the case.
     *
     * Additionally, if CUDA_VISIBLE_DEVICES env var is set, then
     * setting 0 here is the zero-th device in the env var list.
     */
    node *ret_node;

    DBUG_ENTER ();

    /* As the cudamemprefetch primitive aliases it's input,
     * we need to perform an assignment of the lhr by the
     * rhs, so we generate the assignment ND_ASSIGN( v, a);
     */
    ret_node = RhsId (PRF_ARG1 (arg_node), arg_info);

    /* and precede this by the actual prefetch ICM call */
    ret_node = TCmakeAssignIcm3 ("CUDA_MEM_PREFETCH",
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 MakeBasetypeArg (ID_NTYPE (PRF_ARG1 (arg_node))),
                                 TBmakeNum (0), ret_node);

    DBUG_RETURN (ret_node);
}

node *
COMPprfDevice2Host (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm4 ("CUDA_MEM_TRANSFER", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 MakeBasetypeArg (ID_NTYPE (PRF_ARG1 (arg_node))),
                                 TCmakeIdCopyString ("cudaMemcpyDeviceToHost"), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfHost2Device (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm4 ("CUDA_MEM_TRANSFER", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 MakeBasetypeArg (ID_NTYPE (PRF_ARG1 (arg_node))),
                                 TCmakeIdCopyString ("cudaMemcpyHostToDevice"), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfDevice2HostStart (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm4 ("CUDA_MEM_TRANSFER_START", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 MakeBasetypeArg (ID_NTYPE (PRF_ARG1 (arg_node))),
                                 TCmakeIdCopyString ("cudaMemcpyDeviceToHost"), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfHost2DeviceStart (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm4 ("CUDA_MEM_TRANSFER_START", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 MakeBasetypeArg (ID_NTYPE (PRF_ARG1 (arg_node))),
                                 TCmakeIdCopyString ("cudaMemcpyHostToDevice"), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfDevice2HostEnd (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    /**
     * This PRF aliases its second argument, meaning we need to do an assign
     * from this to LHS.
     */
    ret_node = RhsId (PRF_ARG1 (arg_node), arg_info);

    ret_node = TCmakeAssignIcm1 ("CUDA_MEM_TRANSFER_END",
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 ret_node);

    DBUG_RETURN (ret_node);
}

node *
COMPprfHost2DeviceEnd (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    /**
     * This PRF aliases its second argument, meaning we need to do an assign
     * from this to LHS.
     */
    ret_node = RhsId (PRF_ARG1 (arg_node), arg_info);

    ret_node = TCmakeAssignIcm1 ("CUDA_MEM_TRANSFER_END",
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 ret_node);

    DBUG_RETURN (ret_node);
}

node *
COMPprfDevice2Device (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm4 ("CUDA_MEM_TRANSFER", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 MakeBasetypeArg (ID_NTYPE (PRF_ARG1 (arg_node))),
                                 TCmakeIdCopyString ("cudaMemcpyDeviceToDevice"), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfCUDAThreadIdxX (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm1 ("SAC_CUDA_THREADIDX_X",
                                 DUPdupIdsIdNt (INFO_LASTIDS (arg_info)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfCUDAThreadIdxY (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm1 ("SAC_CUDA_THREADIDX_Y",
                                 DUPdupIdsIdNt (INFO_LASTIDS (arg_info)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfCUDAThreadIdxZ (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm1 ("SAC_CUDA_THREADIDX_Z",
                                 DUPdupIdsIdNt (INFO_LASTIDS (arg_info)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfCUDABlockIdxX (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm1 ("SAC_CUDA_BLOCKIDX_X",
                                 DUPdupIdsIdNt (INFO_LASTIDS (arg_info)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfCUDABlockIdxY (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm1 ("SAC_CUDA_BLOCKIDX_Y",
                                 DUPdupIdsIdNt (INFO_LASTIDS (arg_info)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfCUDABlockDimX (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm1 ("SAC_CUDA_BLOCKDIM_X",
                                 DUPdupIdsIdNt (INFO_LASTIDS (arg_info)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfCUDABlockDimY (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm1 ("SAC_CUDA_BLOCKDIM_Y",
                                 DUPdupIdsIdNt (INFO_LASTIDS (arg_info)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfCUDABlockDimZ (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm1 ("SAC_CUDA_BLOCKDIM_Z",
                                 DUPdupIdsIdNt (INFO_LASTIDS (arg_info)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfCUDAGridDimX (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm1 ("SAC_CUDA_GRIDDIM_X",
                                 DUPdupIdsIdNt (INFO_LASTIDS (arg_info)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfCUDAGridDimY (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm1 ("SAC_CUDA_GRIDDIM_Y",
                                 DUPdupIdsIdNt (INFO_LASTIDS (arg_info)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfShmemBoundaryLoad (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_UNREACHABLE ("Illegal primitive function found!");

    DBUG_RETURN (arg_node);
}

node *
COMPprfShmemBoundaryCheck (node *arg_node, info *arg_info)
{
    int dim_pos, offset;
    node *let_ids, *idx, *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    dim_pos = NUM_VAL (PRF_ARG1 (arg_node));
    idx = PRF_ARG2 (arg_node);
    offset = NUM_VAL (PRF_ARG3 (arg_node));

    ret_node = TCmakeAssignIcm4 ("CUDA_SHMEM_BOUNDARY_CHECK", DUPdupIdsIdNt (let_ids),
                                 TBmakeNum (dim_pos), DUPdupIdNt (idx),
                                 TBmakeNum (offset), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfSyncthreads (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm0 ("SAC_CUDA_SYNCTHREADS", NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfKernelTerminate (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = TCmakeAssignIcm0 ("SAC_CUDA_KERNEL_TERMINATE", NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfCond (node *arg_node, info *arg_info)
{
    node *arg1, *arg2, *arg3;
    node *let_ids;
    node *icm_args;
    node *ret_node;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    arg3 = PRF_ARG3 (arg_node);

    DBUG_ASSERT (NODE_TYPE (arg1) == N_id, "1st arg of F_cond is not N_id!");
    DBUG_ASSERT (NODE_TYPE (arg2) == N_id, "2nd arg of F_cond is not N_id!");
    DBUG_ASSERT (NODE_TYPE (arg3) == N_id, "3rd arg of F_cond is not N_id!");

    /*
      DBUG_ASSERT( TYisScalar( ID_NTYPE( arg1)),
                   "1st arg of F_cond is not scalar!");
      DBUG_ASSERT( TYisScalar( ID_NTYPE( arg2)),
                   "2nd arg of F_cond is not scalar!");
      DBUG_ASSERT( TYisScalar( ID_NTYPE( arg3)),
                   "3rd arg of F_cond is not scalar!");
    */

    icm_args
      = TBmakeExprs (DUPdupIdsIdNt (let_ids),
                     TBmakeExprs (DUPdupNodeNt (arg1),
                                  TBmakeExprs (DUPdupNodeNt (arg2),
                                               TBmakeExprs (DUPdupNodeNt (arg3), NULL))));

    ret_node = TCmakeAssignIcm1 ("ND_PRF_COND", icm_args, NULL);

    DBUG_RETURN (ret_node);
}

/*
 * Distributed Variable Primitive Functions
 */
node *
COMPprfGetCudaThread (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm1 ("SAC_DIST_GETCUDATHREAD", DUPdupIdsIdNt (let_ids), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfHost2Dist_st (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm4 ("DIST_HOST2DIST_ST", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG2 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG3 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfHost2Dist_spmd (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm2 ("DIST_HOST2DIST_SPMD", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfDevice2Dist (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm5 ("DIST_DEV2DIST", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 DUPdupIdNt (PRF_ARG2 (arg_node)),
                                 DUPdupIdNt (PRF_ARG3 (arg_node)),
                                 DUPdupIdNt (PRF_ARG4 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfDist2Host_Rel (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm6 ("DIST_DIST2HOST_REL", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG2 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG3 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG4 (arg_node)),
                                 MakeBasetypeArg (IDS_NTYPE (let_ids)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfDist2Dev_Rel (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm6 ("DIST_DIST2DEV_REL", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG2 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG3 (arg_node)),
                                 DUPdupIdNt (PRF_ARG4 (arg_node)),
                                 MakeBasetypeArg (IDS_NTYPE (let_ids)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfDist2Host_Abs (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm6 ("DIST_DIST2HOST_ABS", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG2 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG3 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG4 (arg_node)),
                                 MakeBasetypeArg (IDS_NTYPE (let_ids)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfDist2Dev_Abs (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm6 ("DIST_DIST2DEV_ABS", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG2 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG3 (arg_node)),
                                 DUPdupIdNt (PRF_ARG4 (arg_node)),
                                 MakeBasetypeArg (IDS_NTYPE (let_ids)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfDist2Dev_Avail (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm8 ("DIST_DIST2DEV_AVAIL", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG2 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG3 (arg_node)),
                                 DUPdupIdNt (PRF_ARG4 (arg_node)),
                                 DUPdupIdNt (PRF_ARG5 (arg_node)),
                                 DUPdupIdNt (PRF_ARG6 (arg_node)),
                                 MakeBasetypeArg (IDS_NTYPE (let_ids)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfDistContBlock (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm7 ("DIST_DISTCONTBLOCK", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG2 (arg_node)),
                                 DUPdupNodeNt (PRF_ARG3 (arg_node)),
                                 DUPdupIdNt (PRF_ARG4 (arg_node)),
                                 DUPdupIdNt (PRF_ARG5 (arg_node)),
                                 DUPdupIdNt (PRF_ARG6 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfCudaGetStream (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node
      = TCmakeAssignIcm1 ("CUDA_GET_STREAM", DUPdupIdNt (PRF_ARG1 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfCudaSetDevice (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node
      = TCmakeAssignIcm1 ("CUDA_SET_DEVICE", DUPdupIdNt (PRF_ARG1 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfCudaDeviceSync (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node
      = TCmakeAssignIcm1 ("CUDA_DEVICE_SYNC", DUPdupIdNt (PRF_ARG1 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfSched_Start (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm2 ("SCHED_START", DUPdupIdsIdNt (let_ids),
                                 DUPdupNodeNt (PRF_ARG2 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfSched_Stop (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ();

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm2 ("SCHED_STOP", DUPdupIdsIdNt (let_ids),
                                 DUPdupNodeNt (PRF_ARG2 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/*
 * Refresh mirrors of results of spawned function
 */
node *
COMPprfSyncIds (node *ids, node *chain)
{
    DBUG_ENTER ();

    if (ids != NULL) {
        chain = COMPprfSyncIds (IDS_NEXT (ids), chain);
        chain = TCmakeAssignIcm1 ("ND_REFRESH__MIRROR",
                                  MakeTypeArgs (IDS_NAME (ids), IDS_NTYPE (ids), FALSE,
                                                TRUE, FALSE, NULL),
                                  chain);
    }

    DBUG_RETURN (chain);
}

node *
COMPprfSync (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;
    node *fundef, *let;
    node *vars;

    DBUG_ENTER ();

    fundef = INFO_FUNDEF (arg_info);

    if (global.backend != BE_mutc) {
        if (!FUNDEF_ISSLOWCLONE (fundef)) {
            // TODO: make better empty statement
            ret_node = TCmakeAssignIcm0 ("SAC_NOOP", NULL);
        } else {
            ret_node = TCmakeAssignIcm0 ("SAC_FP_SYNC_END", NULL);

            let = INFO_LET (arg_info);
            vars = LET_LIVEVARS (let);

            while (vars != NULL) {
                ret_node = TCmakeAssignIcm2 ("SAC_FP_GET_LIVEVAR",
                                             TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                                             TCmakeIdCopyString (
                                               AVIS_NAME (LIVEVARS_AVIS (vars))),
                                             ret_node);
                vars = LIVEVARS_NEXT (vars);
            }

            ret_node = TCmakeAssignIcm3 ("SAC_FP_SYNC_START",
                                         TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                                         TCmakeIdCopyString (
                                           AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node)))),
                                         TBmakeNum (LET_SPAWNSYNCINDEX (let)), ret_node);

            // TODO: set only livevars minus results from sync
            vars = LET_LIVEVARS (let);

            while (vars != NULL) {
                ret_node = TCmakeAssignIcm2 ("SAC_FP_SET_LIVEVAR",
                                             TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                                             TCmakeIdCopyString (
                                               AVIS_NAME (LIVEVARS_AVIS (vars))),
                                             ret_node);
                vars = LIVEVARS_NEXT (vars);
            }
        }
    } else {
        ret_node = COMPprfSyncIds (INFO_LASTIDS (arg_info), ret_node);
        ret_node = TCmakeAssignIcm1 ("SAC_MUTC_SPAWNSYNC",
                                     DUPdupIdNt (PRF_ARG1 (arg_node)), ret_node);
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * static global variables for the compilation of primitive functions
 *
 * This function table cannot be defined earlier because we must be in the
 * scope of the respective functions, which are all static and defined above.
 *
 ******************************************************************************/

static const travfun_p prf_comp_funtab[] = {
#define PRFcomp_fun(comp_fun) comp_fun
#include "prf_info.mac"
};

/** <!--********************************************************************-->
 *
 * @fn  node *COMPprf( node *arg_node, info *arg_info)
 *
 * @brief  Compilation of a N_prf node.
 *         The return value is a N_assign chain of ICMs.
 *         Note, that the old 'arg_node' is removed by COMPLet.
 *
 ******************************************************************************/

node *
COMPprf (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (prf_comp_funtab[PRF_PRF (arg_node)] != NULL,
                 "prf found that lacks code generation!");

    ret_node = prf_comp_funtab[PRF_PRF (arg_node)](arg_node, arg_info);

    DBUG_ASSERT (((ret_node != NULL) && (NODE_TYPE (ret_node) == N_assign)),
                 "no assignment chain found!");

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPdo( node *arg_node, info *arg_info)
 *
 * @brief  Compilation of do-loops
 *
 *   The result will look like this:
 *
 *   goto DO_LABEL;
 *   do {
 *     DO_SKIP;
 *
 *   DO_LABEL:
 *     DO_BODY:
 *   } while ( DO_COND);
 *
 ******************************************************************************/

node *
COMPdo (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *cond, *body;
    node *forloop_begin, *forloop_end;
    char *label_str = NULL;

    DBUG_ENTER ();

    if (!DO_ISFORLOOP (arg_node)) {
        DBUG_ASSERT (((NODE_TYPE (DO_COND (arg_node)) == N_id)
                      || (NODE_TYPE (DO_COND (arg_node)) == N_bool)),
                     "loop condition is neither a N_id nor a N_bool node!");

        INFO_COND (arg_info) = TRUE;
        DO_COND (arg_node) = TRAVdo (DO_COND (arg_node), arg_info);
        INFO_COND (arg_info) = FALSE;

        DO_BODY (arg_node) = TRAVdo (DO_BODY (arg_node), arg_info);
        DO_SKIP (arg_node) = TRAVopt(DO_SKIP (arg_node), arg_info);

        /*
         * We will return a N_assign chain!!
         * Therefore we first build a new N_assign node containing the loop.
         */
        cond = DO_COND (arg_node);
        body = DO_BODY (arg_node);
        DO_COND (arg_node) = NULL;
        DO_BODY (arg_node) = NULL;

        ret_node = TBmakeAssign (TBmakeDo (cond, body), NULL);

        /*
         * Build ND-label-icm before body.
         *
         * Needed to avoid the above DEC_RCs in the first pass of a do_loop.
         * See explanations above.
         */
        if (DO_LABEL (arg_node) == 0) {
            label_str = TRAVtmpVarName (LABEL_POSTFIX);
        } else {
            label_str = DO_LABEL (arg_node);
            DO_LABEL (arg_node) = NULL;
        }
        BLOCK_ASSIGNS (body)
          = TCmakeAssignIcm1 ("ND_LABEL", TCmakeIdCopyString (label_str),
                              BLOCK_ASSIGNS (body));

        /*
         * Insert code from DO_SKIP containing DEC_RCs into body
         */
        if (DO_SKIP (arg_node) != NULL) {
            BLOCK_ASSIGNS (body)
              = TCappendAssign (BLOCK_ASSIGNS (DO_SKIP (arg_node)), BLOCK_ASSIGNS (body));

            DO_SKIP (arg_node) = NULL;
        }

        /*
         * Insert GOTO before do-loop.
         */
        ret_node = TCmakeAssignIcm1 ("ND_GOTO", TCmakeIdCopyString (label_str), ret_node);
    } else {
        DO_BODY (arg_node) = TRAVdo (DO_BODY (arg_node), arg_info);
        body = DO_BODY (arg_node);

        ret_node = BLOCK_ASSIGNS (body);
        BLOCK_ASSIGNS (body) = NULL;

        forloop_begin
          = TCmakeAssignIcm2 ("SAC_CUDA_FORLOOP_BEGIN",
                              TCmakeIdCopyString (AVIS_NAME (DO_ITERATOR (arg_node))),
                              TCmakeIdCopyString (AVIS_NAME (DO_UPPER_BOUND (arg_node))),
                              NULL);
        forloop_end = TCmakeAssignIcm0 ("SAC_CUDA_FORLOOP_END", NULL);

        ret_node = TCappendAssign (forloop_begin, ret_node);
        ret_node = TCappendAssign (ret_node, forloop_end);
    }

    /*
     * The old 'arg_node' can be removed now.
     */
    arg_node = FREEdoFreeNode (arg_node);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPcond( node *arg_node, info *arg_info)
 *
 * @brief  Compiling a conditional.
 *
 ******************************************************************************/
node *
COMPcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* FIXME: This assertion fails sometimes when compiling for -mt and the condition is a
     * N_prf node.  */
    DBUG_ASSERT (((NODE_TYPE (COND_COND (arg_node)) == N_id)
                  || (NODE_TYPE (COND_COND (arg_node)) == N_bool)),
                 "if-clause condition is neither a N_id nor a N_bool but node type %d!",
                 NODE_TYPE (COND_COND (arg_node)));

    INFO_WITH2_COND (arg_info) = arg_node;
    INFO_COND (arg_info) = TRUE;
    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    INFO_COND (arg_info) = FALSE;

    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**************************
 *
 *  with-loop
 *
 */

/** <!--********************************************************************-->
 *
 * @fn  node *MakeIcmArgs_WL_LOOP1( node *arg_node)
 *
 * @brief  ICM args without 'step'.
 *
 ******************************************************************************/

static node *
MakeIcmArgs_WL_LOOP1 (node *arg_node)
{
    node *args;
    int dim;

    DBUG_ENTER ();

    dim = WLNODE_DIM (arg_node);
    args = TBmakeExprs (
      TBmakeNum (dim),
      TBmakeExprs (
        DUPdupIdNt (WITH2_VEC (wlnode)),
        TBmakeExprs (DUPdupIdNt (TCgetNthExprsExpr (dim, WITH2_IDS (wlnode))),
                     TBmakeExprs (WLBidOrNumMakeIndex (WLNODE_BOUND1 (arg_node), dim,
                                                       wlids),
                                  TBmakeExprs (WLBidOrNumMakeIndex (WLNODE_BOUND2 (
                                                                      arg_node),
                                                                    dim, wlids),
                                               NULL)))));

    DBUG_RETURN (args);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeIcmArgs_WL_LOOP2( node *arg_node)
 *
 * @brief  ICM args with 'step'.
 *
 ******************************************************************************/

static node *
MakeIcmArgs_WL_LOOP2 (node *arg_node)
{
    node *args;

    DBUG_ENTER ();

    args = TBmakeExprs (MakeIcmArgs_WL_LOOP1 (arg_node),
                        TBmakeExprs (WLBidOrNumMakeIndex (WLNODE_STEP (arg_node),
                                                          WLNODE_DIM (arg_node), wlids),
                                     NULL));

    DBUG_RETURN (args);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeIcmArgs_WL_OP1( node *arg_node, node *_ids)
 *
 * @brief  ICM args without names of loop variables.
 *
 ******************************************************************************/

static node *
MakeIcmArgs_WL_OP1 (node *arg_node, node *_ids)
{
    node *args;

    DBUG_ENTER ();

    args
      = MakeTypeArgs (IDS_NAME (_ids), IDS_NTYPE (_ids), FALSE, TRUE, FALSE,
                      TBmakeExprs (DUPdupIdNt (WITH2_VEC (wlnode)),
                                   TBmakeExprs (TBmakeNum (WITH2_DIMS (wlnode)), NULL)));

    DBUG_RETURN (args);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeIcmArgs_WL_OP2( node *arg_node, node *_ids)
 *
 * @brief  ICM args with names of loop variables.
 *
 ******************************************************************************/

static node *
MakeIcmArgs_WL_OP2 (node *arg_node, node *_ids)
{
    node *args;
    node *last_arg;
    node *withid_ids;
    int num_args;

    DBUG_ENTER ();

    args = MakeIcmArgs_WL_OP1 (arg_node, _ids);
    DBUG_ASSERT (args != NULL, "no ICM args found!");
    last_arg = args;
    while (EXPRS_NEXT (last_arg) != NULL) {
        last_arg = EXPRS_NEXT (last_arg);
    }

    DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (last_arg)) == N_num, "wrong ICM arg found!");
    num_args = NUM_VAL (EXPRS_EXPR (last_arg));

    withid_ids = WITH2_IDS (wlnode);
    while (withid_ids != NULL) {
        last_arg = EXPRS_NEXT (last_arg)
          = TBmakeExprs (DUPdupIdNt (EXPRS_EXPR (withid_ids)), NULL);
        num_args--;
        withid_ids = EXPRS_NEXT (withid_ids);
    }
    DBUG_ASSERT (num_args == 0, "wrong number of ids in WITHID_IDS found!");

    DBUG_RETURN (args);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeIcm_MT_ADJUST_SCHEDULER( node *arg_node, node *assigns)
 *
 * @brief  ...
 *
 ******************************************************************************/

static node *
MakeIcm_MT_ADJUST_SCHEDULER (node *arg_node, node *assigns)
{
    node *withop;
    node *begin_icm = NULL;
    node *end_icm = NULL;
    node *offset_icms = NULL;
    node *tmp_ids;
    node *idxs_exprs;

    int dim;

    DBUG_ENTER ();

    DBUG_ASSERT (((NODE_TYPE (arg_node) == N_wlblock)
                  || (NODE_TYPE (arg_node) == N_wlublock)
                  || ((NODE_TYPE (arg_node) == N_wlstride))),
                 "illegal WL-node found!");

    dim = WLNODE_DIM (arg_node);

    if ((!WLNODE_ISNOOP (arg_node)) && (WLNODE_LEVEL (arg_node) == 0)
        && WITH2_PARALLELIZE (wlnode) && (SCHadjustmentRequired (dim, wlseg))) {

        begin_icm
          = TCmakeAssignIcm6 ("MT_ADJUST_SCHEDULER__BEGIN", DUPdupIdsIdNt (wlids),
                              TBmakeNum (WLSEG_DIMS (wlseg)), TBmakeNum (dim),
                              WLBidOrNumMakeIndex (WLNODE_BOUND1 (arg_node), dim, wlids),
                              WLBidOrNumMakeIndex (WLNODE_BOUND2 (arg_node), dim, wlids),
                              WLBidOrNumMakeIndex (WLNODE_STEP (arg_node), dim, wlids),
                              begin_icm);

        /* for every ids of wlids (multioperator WL) */
        tmp_ids = wlids;
        idxs_exprs = WITH2_IDXS (wlnode);
        withop = WITH2_WITHOP (wlnode);

        while (withop != NULL) {
            if (WITHOP_IDX (withop) != NULL) {
                offset_icms = TCmakeAssignIcm3 ("MT_ADJUST_SCHEDULER__OFFSET",
                                                DUPdupIdNt (EXPRS_EXPR (idxs_exprs)),
                                                DUPdupIdsIdNt (tmp_ids), TBmakeNum (dim),
                                                offset_icms);
                idxs_exprs = EXPRS_NEXT (idxs_exprs);
            }

            tmp_ids = IDS_NEXT (tmp_ids);
            withop = WITHOP_NEXT (withop);
        }

        end_icm
          = TCmakeAssignIcm6 ("MT_ADJUST_SCHEDULER__END", DUPdupIdsIdNt (wlids),
                              TBmakeNum (WLSEG_DIMS (wlseg)), TBmakeNum (dim),
                              WLBidOrNumMakeIndex (WLNODE_BOUND1 (arg_node), dim, wlids),
                              WLBidOrNumMakeIndex (WLNODE_BOUND2 (arg_node), dim, wlids),
                              WLBidOrNumMakeIndex (WLNODE_STEP (arg_node), dim, wlids),
                              end_icm);

        assigns = TCmakeAssigns4 (begin_icm, offset_icms, end_icm, assigns);
    }

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeIcm_WL_INIT_OFFSET( node *arg_node, node *assigns)
 *
 * @brief  ...
 *
 ******************************************************************************/

static node *
MakeIcm_WL_INIT_OFFSET (node *arg_node, node *assigns)
{
    node *withop;
    node *tmp_ids;
    node *idxs_exprs;

    DBUG_ENTER ();

    /* for every ids of wlids (multioperator WL) */
    idxs_exprs = WITH2_IDXS (wlnode);
    tmp_ids = wlids;
    withop = WITH2_WITHOP (wlnode);

    while (withop != NULL) {
        if (WITHOP_IDX (withop) != NULL) {
            assigns
              = TCmakeAssignIcm2 ("WL_INIT_OFFSET", DUPdupIdNt (EXPRS_EXPR (idxs_exprs)),
                                  MakeIcmArgs_WL_OP1 (arg_node, tmp_ids), assigns);

            idxs_exprs = EXPRS_NEXT (idxs_exprs);
        }

        tmp_ids = IDS_NEXT (tmp_ids);
        withop = WITHOP_NEXT (withop);
    }

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeIcm_GETVAR_ifNeeded( node *arg_node)
 *
 * @brief create a GETVAR icm if arg_node is an id
 *****************************************************************************/
static node *
MakeIcm_GETVAR_ifNeeded (node *arg_node)
{
    DBUG_ENTER ();

    if (NODE_TYPE (arg_node) == N_id) {
        node *res
          = TCmakeIcm2 ("SAC_ND_GETVAR",
                        TCmakeIdCopyStringNtNew (ID_NAME (arg_node), ID_NTYPE (arg_node)),
                        TCmakeIdCopyString (ID_NAME (arg_node)));
        arg_node = FREEdoFreeTree (arg_node);
        arg_node = res;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeIcm_WL_ADJUST_OFFSET( node *arg_node, node *assigns)
 *
 * @brief  ...
 *
 ******************************************************************************/

static node *
MakeIcm_WL_ADJUST_OFFSET (node *arg_node, node *assigns)
{
    node *withop;
    node *tmp_ids;
    node *idxs_exprs;

    DBUG_ENTER ();

    /* for every ids of wlids (multioperator WL) */
    tmp_ids = wlids;
    idxs_exprs = WITH2_IDXS (wlnode);
    withop = WITH2_WITHOP (wlnode);

    while (withop != NULL) {
        if (WITHOP_IDX (withop) != NULL) {
            assigns = TCmakeAssignIcm3 ("WL_ADJUST_OFFSET",
                                        DUPdupIdNt (EXPRS_EXPR (idxs_exprs)),
                                        TBmakeNum (WLNODE_DIM (arg_node)),
                                        MakeIcmArgs_WL_OP2 (arg_node, tmp_ids), assigns);

            idxs_exprs = EXPRS_NEXT (idxs_exprs);
        }
        tmp_ids = IDS_NEXT (tmp_ids);
        withop = WITHOP_NEXT (withop);
    }

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeIcm_WL_SET_OFFSET( node *arg_node, node *assigns)
 *
 * @brief  Inserts the ICM WL_SET_OFFSET if needed.
 *
 *   Blocking is inactive:
 *     The WL_SET_OFFSET-icm is needed, if the offset is needed in the wl-code
 *     and the next dimension is the last one, for which the segment's domain
 *     is not the full index range.
 *
 *   Blocking is active:
 *     The WL_SET_OFFSET-icm is needed, if the offset is needed in the wl-code
 *     and the next dimension is the last one.
 *
 ******************************************************************************/

static node *
MakeIcm_WL_SET_OFFSET (node *arg_node, node *assigns)
{
    int first_block_dim, first_ublock_dim, last_frac_dim;
    int dims, dim;
    node *idx_min, *idx_max;
    int d;
    size_t d_u;
    shape *shp;
    int icm_dim = (-1);
    node *withop;
    node *tmp_ids;
    node *idxs_exprs;

    DBUG_ENTER ();

    /* for every ids of wlids (multioperator WL) */
    tmp_ids = wlids;
    withop = WITH2_WITHOP (wlnode);
    idxs_exprs = WITH2_IDXS (wlnode);

    while (withop != NULL) {
        if (WITHOP_IDX (withop) != NULL) {
            dim = WLNODE_DIM (arg_node);
            dims = WLSEG_DIMS (wlseg);

            if (!WLSEG_ISDYNAMIC (wlseg)) {
                /*
                 * infer first unrolling-blocking dimension
                 * (== 'dims', if no unrolling-blocking is done)
                 */
                d = 0;
                while ((d < dims)
                       && (TCgetIntVectorNthValue (d, WLSEG_UBV (wlseg)) == 1)) {
                    d++;
                }
                first_ublock_dim = d;

                /*
                 * infer first blocking dimension
                 * (== 'dims', if no blocking is done)
                 */
                d = 0;
                while (
                  (d < dims)
                  && (TCgetIntVectorNthValue (d, EXPRS_EXPR (WLSEG_BV (wlseg))) == 1)) {
                    d++;
                }
                first_block_dim = d;

                first_block_dim = MATHmin (first_block_dim, first_ublock_dim);
            } else {
                first_block_dim = dims;
            }

            /*
             * infer the last dimension for which the segment's domain is not the
             * full range (== -1, if the segment's domain equals the full index
             * vector space)
             */
            if (TUshapeKnown (IDS_NTYPE (tmp_ids))) {
                shp = TYgetShape (IDS_NTYPE (tmp_ids));
            } else {
                shp = NULL;
            }
            d = dims - 1;
            d_u = d;
            while (d >= 0) {

                idx_min = TCgetNthExprs (d_u, ARRAY_AELEMS (WLSEG_IDXINF (wlseg)));
                idx_max = TCgetNthExprs (d_u, ARRAY_AELEMS (WLSEG_IDXINF (wlseg)));

                if ((NODE_TYPE (idx_min) == N_num) && (NODE_TYPE (idx_max) == N_num)
                    && (((NUM_VAL (idx_min) == 0) && (NUM_VAL (idx_max) == IDX_SHAPE))
                        || ((shp != NULL)
                            && (NUM_VAL (idx_max) == SHgetExtent (shp, d_u))))) {
                    d--;
                    d_u--;
                } else {
                    break;
                }
            }
            last_frac_dim = d;

            /*
             * check whether 'WL_SET_OFFSET' is needed in the current dimension
             * or not
             */
            if (first_block_dim < dims) {
                /*
                 * blocking is active
                 *  -> insert ICM at the most inner position
                 */
                if (dim + 1 == dims - 1) {
                    /* the next dim is the last one */
                    icm_dim = first_block_dim;
                }
            } else {
                /*
                 * blocking is inactive
                 *  -> insert ICM at the computed position
                 */
                if (dim + 1 == last_frac_dim) {
                    icm_dim = first_block_dim;
                }
            }

            if (icm_dim >= 0) {
                assigns
                  = TCmakeAssignIcm4 ("WL_SET_OFFSET",
                                      DUPdupIdNt (EXPRS_EXPR (idxs_exprs)),
                                      TBmakeNum (dim), TBmakeNum (icm_dim),
                                      MakeIcmArgs_WL_OP2 (arg_node, tmp_ids), assigns);
            }
            idxs_exprs = EXPRS_NEXT (idxs_exprs);
        }
        tmp_ids = IDS_NEXT (tmp_ids);
        withop = WITHOP_NEXT (withop);
    }

    DBUG_RETURN (assigns);
}

/** <!--*******************************************************************-->
 *
 * @fn  node *COMPwith( node *arg_node, info *arg_info)
 *
 * @brief
 *   The return value is a N_assign chain of ICMs.
 *   The old 'arg_node' is removed by COMPLet.
 *
 ****************************************************************************/

node *
COMPwith (node *arg_node, info *arg_info)
{
    node *icm_chain = NULL, *body_icms, *default_icms = NULL;
    node *generator_icms;
    node *res_ids, *ids, *idx_id, *lower_id, *upper_id;
    node *break_id = NULL;
    node *offs_exprs, *exprs;
    bool isfull;
    node *old_withloop;
    char *break_label_str;
    int i;

    DBUG_ENTER ();

    res_ids = INFO_LASTIDS (arg_info);

    old_withloop = INFO_WITHLOOP (arg_info);
    INFO_WITHLOOP (arg_info) = arg_node;

    /**
     * This is an aud WL! We assume the following here:
     * 1) There is exactly 1 partition (fold/propagate)
     *                  or 2 partitions (genarray/modarray)
     * 2) We can have multiple operators. If we do, the following
     *    restrictions apply:
     *    a) we do NOT have a mix between fold, genarray, or modarray
     *    b) if we have multiple genarrays, they all have the same shape
     *    c) if we have multiple modarrays, they all modify the same array
     *       NB: identical shapes might suffice, but currently (2022)
     *       these cannot arrive here...
     *    d) propagate operators are allowed both, as a stand-alone WL
     *       or in accompanyment of genarray, modarray, or fold WLs.
     *    e) not sure, we can deal with break- operators????
     *
     * For the code generation, we have to distinguish two cases:
     * 1) isfull: we are dealing with modarray or genarray
     *            and need to iterate over the entire shape
     * 2) ! isfull: we are iterating from lower bound to upper
     *              bound of the generator only.
     *
     * First we DBUG_ASSERT the restrictions explained above, then we
     * set isfull appropriately. The checks regarding the MOWls
     * happen in the helper function CheckAUDOperators.
     */
    DBUG_PRINT ("generating code for AUD with-loop...");
    DBUG_ASSERT (WITH_PART (arg_node) != NULL, "missing part in AUD with loop!");

    isfull = CheckAUDOperators (WITH_WITHOP (arg_node), &break_id);
    if (isfull) {
        DBUG_PRINT ("  genarray/modarray WL found => FULL range");
        DBUG_ASSERT (TCcountParts (WITH_PART (arg_node)) == 2,
                     "AUD genarray / modarray WL does not have"
                     " partition and default partition only");
    } else {
        DBUG_PRINT ("  fold/propagate only WL found => partition range");
        DBUG_ASSERT (TCcountParts (WITH_PART (arg_node)) == 1,
                     "AUD fold / propagate WL does not have"
                     " exactly one partition");
    }

    /*
     * XXX not sure what this does (SBS 2022):
     */
    WITH_WITHOP (arg_node) = RemoveIdxDuplicates (WITH_WITHOP (arg_node));

    /**
     * First, we traverse the partition.
     * This will yield the index vector var in INFO_IDXVEC (cf. COMPwithid),
     * the offsets for all operators in INFO_OFFSETS (cf. COMPwithid),
     * the lower and upper bounds in INFO_LOWERVEC, INFO_UPPERVEC
     * (cf. COMPgenerator),
     * and the generator-check icms in INFO_ICMCHAIN (cf. COMPgenerator).
     * Whereas the former are back-links only (and thus has to be copied!), the
     * latter has been produced for insertion here and thus can be used as is!.
     * Furthermore, note that the index vector comes as N_id as we are
     * after EMM!!
     * Another aspect to be noticed here is that COMPpart relies on
     * INFO_ISFULL_AUDWL to be set properly since for genarray and modarray
     * With-Loops require checks against the lower and upper bounds whereas
     * fold-WLs do not!
     * For single propagates, i.e., withloops that have only a propagate
     * operator, we use the fold compilation scheme, as well.
     */
    INFO_ISFULL_AUDWL (arg_info) = isfull;

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    generator_icms = INFO_ICMCHAIN (arg_info);
    idx_id = INFO_IDXVEC (arg_info);
    offs_exprs = INFO_OFFSETS (arg_info);
    lower_id = INFO_LOWERVEC (arg_info);
    upper_id = INFO_UPPERVEC (arg_info);

    INFO_ICMCHAIN (arg_info) = NULL;
    INFO_IDXVEC (arg_info) = NULL;
    INFO_OFFSETS (arg_info) = NULL;
    INFO_IDXVEC (arg_info) = NULL;
    INFO_LOWERVEC (arg_info) = NULL;
    INFO_UPPERVEC (arg_info) = NULL;


    /**
     * Now, we compile the actual code and put a copy of the resulting icms
     * into body_icm and (in case of isfull) into default_icms.
     */
    DBUG_ASSERT (WITH_CODE (arg_node) != NULL, "missing code in AUD with loop!");
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    body_icms = DUPdoDupTree (BLOCK_ASSIGNS (WITH_CBLOCK (arg_node)));
    // just in case we need the label. Stupid C compilers complain otherwise :-)
    break_label_str = TRAVtmpVarName (LABEL_POSTFIX);

    if (isfull) {
        if (CODE_NEXT (WITH_CODE (arg_node)) == NULL) {
                CTIabort (NODE_LOCATION (arg_node),
                          "Cannot infer default element for with-loop.");
        }
        default_icms
          = DUPdoDupTree (BLOCK_ASSIGNS (CODE_CBLOCK (CODE_NEXT (WITH_CODE (arg_node)))));

        /*
         * After the AUD-WL, we need to free any potential suballoc descriptors.
         * This code is generated in CreateAUDSuballocDescFreeChain.
         */
        icm_chain = CreateAUDSuballocDescFreeChain (WITH_WITHOP (arg_node));
    } else {
        // create label iff CheckAUDOperators found an N_break:
        if (break_id != NULL) {
            icm_chain = TCmakeAssignIcm1 ("ND_LABEL",
                            TCmakeIdCopyString ( break_label_str),
                            icm_chain);
        }
    }

    /**
     * Finally, we generate the code for this with-loop. Note that we build
     * the code bottom up to avoid appending assign chains all the time.
     */
    icm_chain = TCmakeAssignIcm0 ("SAC_AUD_WL_END", icm_chain);
    icm_chain = TCmakeAssignIcm0 ("SAC_AUD_WL_END_ITERATE", icm_chain);
    if (isfull) {
        icm_chain
          = TCmakeAssignIcm2 ("SAC_AUD_WL_INC_IDX_VEC_FULL",
                TCmakeIdCopyStringNtNew (ID_NAME (idx_id), ID_NTYPE (idx_id)),
                TCmakeIdCopyStringNtNew (IDS_NAME (res_ids), IDS_NTYPE (res_ids)),
                icm_chain);
        i = 0;
        exprs = offs_exprs;
        while (exprs != NULL) {
            icm_chain
              = TCmakeAssignIcm2 ("SAC_AUD_WL_INC_OFFSET",
                    TCmakeIdCopyStringNtNew (ID_NAME (EXPRS_EXPR (exprs)),
                                             ID_NTYPE (EXPRS_EXPR (exprs))),
                    TBmakeNum (i),
                    icm_chain);
            i++;
            exprs = EXPRS_NEXT (exprs);
        }
    } else {
        icm_chain
          = TCmakeAssignIcm3 ("SAC_AUD_WL_INC_IDX_VEC_LOWER",
                TCmakeIdCopyStringNtNew (ID_NAME (idx_id), ID_NTYPE (idx_id)),
                TCmakeIdCopyStringNtNew (ID_NAME (lower_id), ID_NTYPE (lower_id)),
                TCmakeIdCopyStringNtNew (ID_NAME (upper_id), ID_NTYPE (upper_id)),
                icm_chain);
    }
    icm_chain = TCmakeAssignIcm0 ("AUD_WL_COND_END", icm_chain);
    if (isfull) {
        icm_chain = TCappendAssign (default_icms, icm_chain);
        icm_chain = TCmakeAssignIcm0 ("AUD_WL_COND_DEFAULT", icm_chain);
    }
    // create exit iff CheckAUDOperators found an N_break:
    if (break_id != NULL) {
        icm_chain = TCmakeAssignIcm2 ("BREAK_ON_GUARD",
                        TCmakeIdCopyString ( ID_NAME (break_id)),
                        TCmakeIdCopyString ( break_label_str),
                        icm_chain);
    }
    icm_chain = TCappendAssign (body_icms, icm_chain);
    icm_chain = TCmakeAssignIcm1 ("AUD_WL_COND_BODY",
                    TCmakeIdCopyStringNtNew (ID_NAME (idx_id), ID_NTYPE (idx_id)),
                    icm_chain);
    icm_chain = TCappendAssign (generator_icms, icm_chain);
    icm_chain = TCmakeAssignIcm0 ("AUD_WL_START_ITERATE", icm_chain);

    if (isfull) {
        i = 0;
        exprs = offs_exprs;
        ids = res_ids;
        while (exprs != NULL) {
            icm_chain
              = TCmakeAssignIcm3 ("SAC_AUD_WL_INIT_OFFSET",
                    TCmakeIdCopyStringNtNew (ID_NAME (EXPRS_EXPR (exprs)),
                                             ID_NTYPE (EXPRS_EXPR (exprs))),
                    TCmakeIdCopyStringNtNew (IDS_NAME (ids), IDS_NTYPE (ids)),
                    TBmakeNum (i),
                    icm_chain);
            i++;
            exprs = EXPRS_NEXT (exprs);
            ids = IDS_NEXT (ids);
        }
        icm_chain
          = TCmakeAssignIcm2 ("SAC_AUD_WL_INIT_IDX_VEC_FULL",
                TCmakeIdCopyStringNtNew (ID_NAME (idx_id), ID_NTYPE (idx_id)),
                TCmakeIdCopyStringNtNew (IDS_NAME (res_ids), IDS_NTYPE (res_ids)),
                icm_chain);
    } else {
        icm_chain
          = TCmakeAssignIcm3 ("SAC_AUD_WL_INIT_IDX_VEC_LOWER",
                TCmakeIdCopyStringNtNew (ID_NAME (idx_id), ID_NTYPE (idx_id)),
                TCmakeIdCopyStringNtNew (ID_NAME (lower_id), ID_NTYPE (lower_id)),
                TCmakeIdCopyStringNtNew (ID_NAME (upper_id), ID_NTYPE (upper_id)),
                icm_chain);
    }
    icm_chain = TCmakeAssignIcm1 ("AUD_WL_BEGIN",
                                  TCmakeIdCopyStringNtNew (ID_NAME (idx_id), ID_NTYPE (idx_id)),
                                  icm_chain);

    if (isfull) {
        icm_chain = TCappendAssign (
                        CreateAUDSuballocDescAllocChain (WITH_WITHOP (arg_node),
                                                         res_ids, idx_id),
                        icm_chain);
    } else {
        icm_chain = TCappendAssign (
                        CreateAUDInitAccus (WITH_WITHOP (arg_node), res_ids),
                        icm_chain);
    }

    break_label_str = MEMfree (break_label_str);
    INFO_WITHLOOP (arg_info) = old_withloop;

    DBUG_RETURN (icm_chain);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPpart( node *arg_node, info *arg_info)
 *
 * @brief
 *    make sure the N_withid node is visited BEFORE the N_generator!
 *     ATTENTION: this is used in case of AUD only! i.e. when being called
 *     from N_with but NOT from N_with2!
 *
 *****************************************************************************/

node *
COMPpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_ASSERT (PART_WITHID (arg_node) != NULL, "N_part without N_withid!");
    PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);

    DBUG_ASSERT (PART_GENERATOR (arg_node) != NULL, "N_part without N_generator!");
    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPwithid( node *arg_node, info *arg_info)
 *
 * @brief
 *     passes up pointer to the generator vector variable via INFO_IDXVEC.
 *     ATTENTION: this is used in case of AUD only! i.e. when being called
 *     from N_with but NOT from N_with2!
 *
 *****************************************************************************/

node *
COMPwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_ASSERT (WITHID_IDS (arg_node) == NULL,
                 "AUD with loop with WITHID_IDS found!"
                 "Should have been transformed into N_with2 (AKD)!");
    INFO_IDXVEC (arg_info) = WITHID_VEC (arg_node);
    INFO_OFFSETS (arg_info) = WITHID_IDXS (arg_node);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPgenerator( node *arg_node, info *arg_info)
 *
 * @brief
 *     creates an ICM for the generator-check and returns it via
 *     INFO_ICMCHAIN. returns lower and upper bound in INFO_LOWERVEC
 *     and INFO_UPPERVEC, respectively.
 *     expects INFO_IDXVEC and INFO_ISFULL_AUDWL to be set properly!
 *     ATTENTION: this is used in case of AUD only! i.e. when being called
 *     from N_with but NOT from N_with2!
 *
 *****************************************************************************/

node *
COMPgenerator (node *arg_node, info *arg_info)
{
    node *lower, *upper, *step, *width, *idx;

    DBUG_ENTER ();
    lower = GENERATOR_BOUND1 (arg_node);
    upper = GENERATOR_BOUND2 (arg_node);
    step = GENERATOR_STEP (arg_node);
    width = GENERATOR_WIDTH (arg_node);

    idx = INFO_IDXVEC (arg_info);
    INFO_LOWERVEC (arg_info) = lower;
    INFO_UPPERVEC (arg_info) = upper;

    if (INFO_ISFULL_AUDWL (arg_info)) {
        INFO_ICMCHAIN (arg_info)
          = TCmakeAssignIcm3 ("SAC_AUD_WL_CHECK_LU",
                TCmakeIdCopyStringNtNew (ID_NAME (idx), ID_NTYPE (idx)),
                TCmakeIdCopyStringNtNew (ID_NAME (lower), ID_NTYPE (lower)),
                TCmakeIdCopyStringNtNew (ID_NAME (upper), ID_NTYPE (upper)),
                NULL);
    } else {
        INFO_ICMCHAIN (arg_info) = NULL;
    }
    if (step != NULL) {
        if (width == NULL) {
            INFO_ICMCHAIN (arg_info)
              = TCmakeAssignIcm3 ("SAC_AUD_WL_CHECK_STEP",
                    TCmakeIdCopyStringNtNew (ID_NAME (idx), ID_NTYPE (idx)),
                    TCmakeIdCopyStringNtNew (ID_NAME (lower), ID_NTYPE (lower)),
                    TCmakeIdCopyStringNtNew (ID_NAME (step), ID_NTYPE (step)),
                    INFO_ICMCHAIN (arg_info));
        } else {
            INFO_ICMCHAIN (arg_info)
              = TCmakeAssignIcm4 ("SAC_AUD_WL_CHECK_STEPWIDTH",
                                  TCmakeIdCopyStringNtNew (ID_NAME (idx), ID_NTYPE (idx)),
                                  TCmakeIdCopyStringNtNew (ID_NAME (lower), ID_NTYPE (lower)),
                                  TCmakeIdCopyStringNtNew (ID_NAME (step), ID_NTYPE (step)),
                                  TCmakeIdCopyStringNtNew (ID_NAME (width), ID_NTYPE (width)),
                                  INFO_ICMCHAIN (arg_info));
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPwith2( node *arg_node, info *arg_info)
 *
 * @brief  Compilation of a N_with2 node.
 *   If this is a fold-with-loop, we append the vardecs of all special fold-funs
 *    to the vardec-chain of the current function.
 *   The return value is a N_assign chain of ICMs.
 *   The old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   - 'wlids' points always to LET_IDS of the current with-loop.
 *   - 'wlnode' points always to the N_Nwith2-node.
 *
 ******************************************************************************/

node *
COMPwith2 (node *arg_node, info *arg_info)
{
    node *old_wlnode;
    node *old_wlids;
    node *icm_args;
    char *profile_name = NULL;
    node *ret_node;
    node *alloc_icms = NULL;
    node *free_icms = NULL;
    node *fold_icms = NULL;
    node *fold_rc_icms = NULL;
    node *shpfac_decl_icms = NULL;
    node *shpfac_def_icms = NULL;
    node *tmp_ids;
    node *idxs_exprs;
    node *withop;
    node *let_neutral;
    char *break_label_str;
    int num_with_ops = 0;

    DBUG_ENTER ();

    /*
     * we must store the with-loop ids *before* compiling the codes
     *  because INFO_LASTIDS is possibly updated afterwards!!!
     */
    old_wlids = wlids; /* stack 'wlids' */
    wlids = INFO_LASTIDS (arg_info);
    old_wlnode = wlnode; /* stack 'wlnode' */
    wlnode = arg_node;

    /*******************************************
     * build WL_... ICMs                       *
     *******************************************/

    /*
     * build arguments for  'WL_SCHEDULE__BEGIN'-ICM and 'WL_SCHEDULE__END'-ICM
     */

    WITH2_WITHOP (arg_node) = RemoveIdxDuplicates (WITH2_WITHOP (arg_node));

    /*
     * build all required(!) shape factors
     */
    tmp_ids = wlids;
    idxs_exprs = WITH2_IDXS (wlnode);
    withop = WITH2_WITHOP (wlnode);

    while (withop != NULL) {
        num_with_ops++;

        if (WITHOP_IDX (withop) != NULL) {
            shpfac_decl_icms
              = TCmakeAssignIcm3 ("WL_DECLARE_SHAPE_FACTOR",
                                  MakeTypeArgs (IDS_NAME (tmp_ids), IDS_NTYPE (tmp_ids),
                                                FALSE, TRUE, FALSE, NULL),
                                  DUPdupIdNt (WITH2_VEC (wlnode)),
                                  TBmakeNum (WITH2_DIMS (arg_node)), shpfac_decl_icms);

            shpfac_def_icms
              = TCmakeAssignIcm3 ("WL_DEFINE_SHAPE_FACTOR",
                                  MakeTypeArgs (IDS_NAME (tmp_ids), IDS_NTYPE (tmp_ids),
                                                FALSE, TRUE, FALSE, NULL),
                                  DUPdupIdNt (WITH2_VEC (wlnode)),
                                  TBmakeNum (WITH2_DIMS (arg_node)), shpfac_def_icms);

            idxs_exprs = EXPRS_NEXT (idxs_exprs);
        }

        tmp_ids = IDS_NEXT (tmp_ids);
        withop = WITHOP_NEXT (withop);
    }

    icm_args = TBmakeExprs (TBmakeNum (WITH2_DIMS (arg_node)), NULL);

    tmp_ids = wlids;
    withop = WITH2_WITHOP (wlnode);

    while (tmp_ids != NULL) {

        /*
         * The descriptor of A_sub must be built
         * as otherwise suballoc cannot calculate
         * the index correctly!
         */
        if ((NODE_TYPE (withop) == N_genarray) || (NODE_TYPE (withop) == N_modarray)) {

            if (WITHOP_SUB (withop) != NULL) {
                node *sub_id = WITHOP_SUB (withop);
                node *sub_get_dim;
                node *sub_set_shape;

                /*
                 * Calculate dimension of subarray
                 *
                 * dim( A_sub) = dim( A) - size( iv)
                 */
                sub_get_dim
                  = TCmakeIcm2 (prf_ccode_tab[F_sub_SxS],
                                TCmakeIcm1 ("ND_A_DIM", DUPdupIdsIdNt (tmp_ids)),
                                TBmakeNum (WITH2_DIMS (arg_node)));

                /*
                 * Annotate shape of subarray if default present
                 * (genarray only)
                 */
                if ((NODE_TYPE (withop) == N_genarray)
                    && (!KNOWN_SHAPE (TUgetFullDimEncoding (ID_NTYPE (sub_id))))) {
                    if (GENARRAY_DEFAULT (withop) != NULL) {
                        DBUG_PRINT ("creating COPY__SHAPE for SUBALLOC var");
                        /*
                         * copy shape
                         */
                        sub_set_shape
                          = TCmakeIcm1 ("ND_COPY__SHAPE",
                                        MakeTypeArgs (ID_NAME (sub_id), ID_NTYPE (sub_id),
                                                      FALSE, TRUE, FALSE,
                                                      MakeTypeArgs (ID_NAME (
                                                                      GENARRAY_DEFAULT (
                                                                        withop)),
                                                                    ID_NTYPE (
                                                                      GENARRAY_DEFAULT (
                                                                        withop)),
                                                                    FALSE, TRUE, FALSE,
                                                                    NULL)));

                        alloc_icms = TBmakeAssign (sub_set_shape, alloc_icms);

                    } else {
                        DBUG_UNREACHABLE ("no default value found! "
                                          "cannot create subvar shape");
                    }
                } else if ((NODE_TYPE (withop) == N_modarray)
                           && (!KNOWN_SHAPE (TUgetFullDimEncoding (ID_NTYPE (sub_id))))) {
                    DBUG_PRINT ("creating WL_MODARRAY_SUBSHAPE for SUBALLOC var");
                    /*
                     * set shape in modarray case based upon result
                     * and index vector
                     */
                    sub_set_shape
                      = TCmakeIcm4 ("WL_MODARRAY_SUBSHAPE",
                                    TCmakeIdCopyStringNtNew (ID_NAME (sub_id),
                                                          ID_NTYPE (sub_id)),
                                    DUPdupIdNt (WITHID_VEC (WITH2_WITHID (arg_node))),
                                    TBmakeNum (TUgetDimEncoding (ID_NTYPE (sub_id))),
                                    DUPdupIdsIdNt (tmp_ids));

                    alloc_icms = TBmakeAssign (sub_set_shape, alloc_icms);
                }

                /*
                 * Allocate descriptor of subarray
                 */
                alloc_icms = MakeAllocDescIcm (ID_NAME (sub_id), ID_NTYPE (sub_id), 1,
                                               sub_get_dim, alloc_icms);

                /*
                 * Free descriptor of subarray
                 */
                free_icms = TCmakeAssignIcm1 ("ND_FREE__DESC",
                                              TCmakeIdCopyStringNtNew (ID_NAME (sub_id),
                                                                    ID_NTYPE (sub_id)),
                                              free_icms);
            }
        }

        if (NODE_TYPE (withop) == N_fold) {
            /*
             * put (tmp_ids, foldop) into FOLDLUT
             */
            INFO_FOLDLUT (arg_info)
              = LUTinsertIntoLutP (INFO_FOLDLUT (arg_info), IDS_AVIS (tmp_ids),
                                   FOLD_FUNDEF (withop));

            /*
             * compile 'tmp_ids = neutral' !!!
             */
            let_neutral
              = TBmakeLet (DUPdoDupNode (tmp_ids), DUPdoDupNode (FOLD_NEUTRAL (withop)));

            fold_icms = TCappendAssign (COMPdoCompile (let_neutral), fold_icms);
        }

        tmp_ids = IDS_NEXT (tmp_ids);
        withop = WITHOP_NEXT (withop);
    }

    /**
     * Take type of first operator as profile name
     * even it is a multioperator WL
     */
    if (WITH2_WITHOP (arg_node) == NULL) {
        profile_name = "void";
    } else {
        switch (WITH2_TYPE (arg_node)) {
        case N_genarray:
            profile_name = "genarray";
            break;

        case N_modarray:
            profile_name = "modarray";
            break;

        case N_fold:
            profile_name = "fold";
            break;

        case N_propagate:
            profile_name = "propagate";
            break;

        case N_break:
            DBUG_UNREACHABLE ("Break must not appear as a first withop");

        default:
            DBUG_UNREACHABLE ("illegal withop type found");
            break;
        }
    }

    /************************************************
     * compile all code blocks                      *
     *                                              *
     * NB: The code generated here is put into      *
     *     the chain of ICMs in COMPwlseg_xx, i.e., *
     *     when traversing through WITH2_SEGS       *
     *     down below!                              *
     ************************************************/

    WITH2_CODE (arg_node) = TRAVopt(WITH2_CODE (arg_node), arg_info);

    /*******************************************
     * put it all together                     *
     *******************************************/

    break_label_str = TRAVtmpVarName (LABEL_POSTFIX);
    INFO_BREAKLABEL (arg_info) = break_label_str;

    node *begin_icm;
    if (global.backend == BE_distmem) {
        withop = WITH2_WITHOP (wlnode);

        if (num_with_ops > 1) {
            CTIwarn (EMPTY_LOC, "The distributed memory backend does not yet support distributed "
                     "multi-operator with-loops "
                     "(first target: %s, first operator: %s, number of operators: %d).",
                     IDS_NAME (wlids), profile_name, num_with_ops);
        }

        /*
         * The with-loop is distributable iff it is a single-operator
         * genarray or modarray with-loop.
         */
        bool is_distributable
          = (num_with_ops == 1)
            && ((NODE_TYPE (withop) == N_genarray) || (NODE_TYPE (withop) == N_modarray));

        begin_icm = TCmakeAssignIcm3 ("WL_DIST_SCHEDULE__BEGIN", icm_args,
                                      TBmakeBool (is_distributable),
                                      MakeTypeArgs (IDS_NAME (wlids), IDS_NTYPE (wlids),
                                                    TRUE, FALSE, FALSE, NULL),
                                      NULL);

    } else {
        begin_icm = TCmakeAssignIcm1 ("WL_SCHEDULE__BEGIN", icm_args, NULL);
    }

    ret_node = TCmakeAssigns9 (
      alloc_icms, fold_icms,
      TCmakeAssignIcm1 ("PF_BEGIN_WITH", TCmakeIdCopyString (profile_name), begin_icm),
      shpfac_decl_icms, shpfac_def_icms, TRAVdo (WITH2_SEGS (arg_node), arg_info),
      TCmakeAssignIcm1 ("WL_SCHEDULE__END", DUPdoDupTree (icm_args),
                        TCmakeAssignIcm1 ("PF_END_WITH",
                                          TCmakeIdCopyString (profile_name),
#if FOLDFIX_LABEL_GENERATION_ACTIVE
                                          TCmakeAssignIcm1 ("ND_LABEL",
                                                            TCmakeIdCopyString (
                                                              break_label_str),
                                                            NULL))),
#else
                                          NULL)),
#endif
      fold_rc_icms, free_icms);
    INFO_BREAKLABEL (arg_info) = NULL;

    /*
     * pop 'wlids', 'wlnode'
     */
    wlids = old_wlids;
    wlnode = old_wlnode;

    DBUG_RETURN (ret_node);
}

static void
COMPwith3AllocDesc (node *ops, node **pre, node **post)
{
    DBUG_ENTER ();

    if (global.mutc_suballoc_desc_one_level_up) {
        if (WITHOP_NEXT (ops) != NULL) {
            COMPwith3AllocDesc (WITHOP_NEXT (ops), pre, post);
        }

        if (((NODE_TYPE (ops) == N_genarray) && (GENARRAY_SUB (ops) != NULL))
            || ((NODE_TYPE (ops) == N_modarray) && (MODARRAY_SUB (ops) != NULL))) {
            node *sub
              = NODE_TYPE (ops) == N_genarray ? GENARRAY_SUB (ops) : MODARRAY_SUB (ops);
            int dim = TUgetDimEncoding (ID_NTYPE (WITHOP_MEM (ops)));
            DBUG_ASSERT (dim >= 0, "Can only handle AKD or better");
            *pre = MakeMutcLocalAllocDescIcm (ID_NAME (sub), ID_NTYPE (sub), 1,
                                              TBmakeNum (dim), *pre);
            *pre = TCmakeAssignIcm2 ("ND_DECL__DESC",
                                     TCmakeIdCopyStringNtNew (ID_NAME (sub), ID_NTYPE (sub)),
                                     TCmakeIdCopyString (""), *pre);
#if FREE_LOCAL
            *post = TCmakeAssignIcm1 ("ND_FREE__DESC",
                                      TCmakeIdCopyStringNtNew (ID_NAME (sub), ID_NTYPE (sub)),
                                      *post);
#endif
        }
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPwith3( node *arg_node, info *arg_info)
 *
 * @brief Compiliation of with3 node
 *        Create family ids
 *        Create creats
 *        Create syncs
 *****************************************************************************/

node *
COMPwith3 (node *arg_node, info *arg_info)
{
    bool old_concurrentranges = INFO_CONCURRENTRANGES (arg_info);
    node *pre = NULL;
    node *post = NULL;
    node *ops = NULL;
    char *with3_dist = NULL;
    DBUG_ENTER ();

    if (global.backend == BE_mutc) {
        INFO_CONCURRENTRANGES (arg_info) = WITH3_USECONCURRENTRANGES (arg_node);
        INFO_WITH3_FOLDS (arg_info)
          = With3Folds (INFO_LASTIDS (arg_info), WITH3_OPERATIONS (arg_node));

        ops = INFO_WITHOPS (arg_info);
        INFO_WITHOPS (arg_info) = WITH3_OPERATIONS (arg_node);
        with3_dist = INFO_WITH3_DIST (arg_info);
        INFO_WITH3_DIST (arg_info) = WITH3_DIST (arg_node);

        COMPwith3AllocDesc (INFO_WITHOPS (arg_info), &pre, &post);
        arg_node = TRAVopt (WITH3_RANGES (arg_node), arg_info);

        INFO_WITHOPS (arg_info) = ops;
        INFO_WITH3_DIST (arg_info) = with3_dist;

        if (pre != NULL) {
            arg_node = TCappendAssign (pre, arg_node);
        }
        if (post != NULL) {
            arg_node = TCappendAssign (arg_node, post);
        }

        INFO_WITH3_FOLDS (arg_info) = FREEoptFreeTree(INFO_WITH3_FOLDS (arg_info));

        INFO_CONCURRENTRANGES (arg_info) = old_concurrentranges;
    } else if (global.backend == BE_cuda || global.backend == BE_cudahybrid) {
        arg_node = TRAVopt (WITH3_RANGES (arg_node), arg_info);
    } else {
        DBUG_UNREACHABLE ("With3 not defined for this backend");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPrange( node *arg_node, info *arg_info)
 *
 * @brief Compilation of range node creating family create sync
 *****************************************************************************/

node *
COMPrange (node *arg_node, info *arg_info)
{
    node *res = NULL;

    node *family, *create, *next, *sync;
    node *thread_fun;
    node *block;
    char *familyName;
    str_buf *buffer;

    node *loopnests;

    DBUG_ENTER ();

    if (global.backend == BE_mutc) {
        buffer = SBUFcreate (1024);

        buffer = SBUFprintf (buffer, "family_%s",
                             FUNDEF_NAME (AP_FUNDEF (RANGE_RESULTS (arg_node))));

        familyName = SBUF2str (buffer);
        buffer = SBUFfree (buffer);

        family = TCmakeAssignIcm1 ("SAC_MUTC_DECL_FAMILY",
                                   TCmakeIdCopyString (familyName), NULL);

        thread_fun = TRAVdo (RANGE_RESULTS (arg_node), arg_info);

        RANGE_LOWERBOUND (arg_node)
          = MakeIcm_GETVAR_ifNeeded (RANGE_LOWERBOUND (arg_node));
        RANGE_UPPERBOUND (arg_node)
          = MakeIcm_GETVAR_ifNeeded (RANGE_UPPERBOUND (arg_node));
        if (RANGE_CHUNKSIZE (arg_node) != NULL) {
            RANGE_CHUNKSIZE (arg_node)
              = MakeIcm_GETVAR_ifNeeded (RANGE_CHUNKSIZE (arg_node));
        }

        if (global.mutc_force_block_size >= 0) {
            block = TBmakeNum (global.mutc_force_block_size);
        } else if (global.mutc_static_resource_management) {
            block = TBmakeNum (RANGE_BLOCKSIZE (arg_node));
        } else {
            block = TCmakeIdCopyString ("");
        }

        create
          = TCmakeAssignIcm7 ("SAC_MUTC_CREATE", TCmakeIdCopyString (familyName),
                              TCmakeIdCopyString (
                                INFO_WITH3_DIST (arg_info) != NULL
                                  ? INFO_WITH3_DIST (arg_info)
                                  : (RANGE_ISGLOBAL (arg_node) ? "" : "PLACE_LOCAL")),
                              DUPdoDupTree (RANGE_LOWERBOUND (arg_node)),
                              DUPdoDupTree (RANGE_UPPERBOUND (arg_node)),
                              (RANGE_CHUNKSIZE (arg_node) == NULL)
                                ? TCmakeIdCopyString ("1")
                                : DUPdoDupTree (RANGE_CHUNKSIZE (arg_node)),
                              block, DUPdoDupTree (ASSIGN_STMT (thread_fun)), NULL);

        sync = TCmakeAssignIcm1 ("SAC_MUTC_SYNC", TCmakeIdCopyString (familyName), NULL);

        next = TRAVopt (RANGE_NEXT (arg_node), arg_info);

        family = TCappendAssign (family, create);
#if 0
    if (INFO_CONCURRENTRANGES( arg_info)) {
      DBUG_ASSERT ( INFO_WITH3_FOLDS( arg_info) == NULL, "Fold and concurrent not supported");
      family = TCappendAssign(family, next);
      family = TCappendAssign(family, sync);
    } else {
#endif
        node *start, *end;
        family = TCappendAssign (family, sync);
        start = TCmakeAssignIcm0 ("MUTC_CREATE_BLOCK_START", NULL);
        end = TCmakeAssignIcm0 ("MUTC_CREATE_BLOCK_END", NULL);

        family = TCappendAssign (start, family);

        if (INFO_WITH3_FOLDS (arg_info) != NULL) {
            node *save;
            DBUG_ASSERT (IDS_NEXT (INFO_WITH3_FOLDS (arg_info)) == NULL,
                         "Only single fold with3 loops supported");
            save = TCmakeAssignIcm1 ("SAC_MUTC_SAVE",
                                     TCmakeIdCopyStringNtNew (IDS_NAME (
                                                             INFO_WITH3_FOLDS (arg_info)),
                                                           IDS_NTYPE (INFO_WITH3_FOLDS (
                                                             arg_info))),
                                     NULL);
            family = TCappendAssign (family, save);
        }
        family = TCappendAssign (family, end);
        family = TCappendAssign (family, next);
#if 0
    }
#endif

        /* FREEdoFreeTree(arg_node); */ /* Done by COMPlet for us */
        FREEdoFreeTree (thread_fun);

        res = family;
    } else if (global.backend == BE_cuda || global.backend == BE_cudahybrid) {

        RANGE_LOWERBOUND (arg_node)
          = MakeIcm_GETVAR_ifNeeded (RANGE_LOWERBOUND (arg_node));
        RANGE_UPPERBOUND (arg_node)
          = MakeIcm_GETVAR_ifNeeded (RANGE_UPPERBOUND (arg_node));
        if (RANGE_CHUNKSIZE (arg_node) != NULL) {
            RANGE_CHUNKSIZE (arg_node)
              = MakeIcm_GETVAR_ifNeeded (RANGE_CHUNKSIZE (arg_node));
        }

        loopnests = TCmakeAssignIcm5 ("WL3_SCHEDULE__BEGIN",
                                      DUPdoDupTree (RANGE_LOWERBOUND (arg_node)),
                                      DUPdupIdNt (RANGE_INDEX (arg_node)),
                                      DUPdoDupTree (RANGE_UPPERBOUND (arg_node)),
                                      (RANGE_CHUNKSIZE (arg_node) == NULL)
                                        ? TCmakeIdCopyString ("1")
                                        : DUPdoDupTree (RANGE_CHUNKSIZE (arg_node)),
                                      TBmakeNum (RANGE_NEEDCUDAUNROLL (arg_node)), NULL);

        RANGE_BODY (arg_node) = TRAVopt (RANGE_BODY (arg_node), arg_info);
        loopnests = TCappendAssign (loopnests,
                                    DUPdoDupTree (BLOCK_ASSIGNS (RANGE_BODY (arg_node))));
        loopnests
          = TCappendAssign (loopnests,
                            TCmakeAssignIcm1 ("WL3_SCHEDULE__END",
                                              DUPdupIdNt (RANGE_INDEX (arg_node)), NULL));

        next = TRAVopt (RANGE_NEXT (arg_node), arg_info);
        res = TCappendAssign (loopnests, next);
    } else {
        DBUG_UNREACHABLE ("N_range not defined in this backend");
        res = NULL;
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPwlseg( node *arg_node, info *arg_info)
 *
 * @brief  Compilation of a N_wlseg- or N_wlsegvar-node:
 *   Returns a N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *   (The whole with-loop-tree should be freed by COMPWith2() only!!)
 *
 * remark:
 *   - 'wlseg' points to the current with-loop segment.
 *     (N_wlseg- *or* N_wlsegVar-node!)
 *
 ******************************************************************************/
node *
COMPwlseg (node *arg_node, info *arg_info)
{
    node *old_wlseg;
    node *ret_node;
    node *next_icms = NULL;

    DBUG_ENTER ();

    /*
     * stack old 'wlseg'.
     * store pointer to current segment in 'wlseg'.
     */
    old_wlseg = wlseg;
    wlseg = arg_node;

    /*
     * create ICMs for successor
     */
    if (WLSEG_NEXT (arg_node) != NULL) {
        next_icms = TRAVdo (WLSEG_NEXT (arg_node), arg_info);
    }

    ret_node
      = TCmakeAssigns4 (SCHcompileSchedulingWithTaskselBegin (INFO_SCHEDULERID (arg_info),
                                                              wlids,
                                                              WLSEG_SCHEDULING (arg_node),
                                                              WLSEG_TASKSEL (arg_node),
                                                              arg_node),
                        MakeIcm_WL_INIT_OFFSET (arg_node,
                                                TRAVdo (WLSEG_CONTENTS (arg_node),
                                                        arg_info)),
                        SCHcompileSchedulingWithTaskselEnd (INFO_SCHEDULERID (arg_info),
                                                            wlids,
                                                            WLSEG_SCHEDULING (arg_node),
                                                            WLSEG_TASKSEL (arg_node),
                                                            arg_node),
                        next_icms);

    /*
     * Collect initialization ICMs for schedulers.
     * This is only done for 'real' schedulers, not for the pseudo schedulers
     * used during sequential execution.
     */
    if (WLSEG_SCHEDULING (arg_node) != NULL) {
        INFO_SCHEDULERINIT (arg_info)
          = TBmakeAssign (SCHcompileSchedulingWithTaskselInit (INFO_SCHEDULERID (
                                                                 arg_info),
                                                               wlids,
                                                               WLSEG_SCHEDULING (
                                                                 arg_node),
                                                               WLSEG_TASKSEL (arg_node),
                                                               arg_node),
                          INFO_SCHEDULERINIT (arg_info));

        (INFO_SCHEDULERID (arg_info))++;
    }

    /*
     * pop 'wlseg'.
     */
    wlseg = old_wlseg;

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPwlxblock( node *arg_node, info *arg_info)
 *
 * @brief  Compilation of a N_wlblock- or N_wlublock-node:
 *     Returns a N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by COMPWith2() only!!)
 *
 * remarks:
 *   - 'wlids' points always to LET_IDS of the current with-loop.
 *   - 'wlnode' points always to the N_Nwith2-node.
 *   - 'wlseg' points to the current with-loop segment.
 *     (N_wlseg- *or* N_wlsegVar-node!)
 *
 ******************************************************************************/
static node *
COMPwlxblock (node *arg_node, info *arg_info)
{
    int level;
    bool is_block, mt_active, offset_needed;
    node *ret_node;
    char *icm_name_begin = NULL;
    char *icm_name_end = NULL;
    node *begin_icm = NULL;
    node *end_icm = NULL;
    node *node_icms = NULL;
    node *next_icms = NULL;

    DBUG_ENTER ();

    level = WLXBLOCK_LEVEL (arg_node);

    is_block = (NODE_TYPE (arg_node) == N_wlblock);
    mt_active = WITH2_PARALLELIZE (wlnode);
    offset_needed = WITH2_NEEDSOFFSET (wlnode);

    /*******************************************
     * create ICMs for next dim / contents     *
     *******************************************/

    if (WLXBLOCK_ISNOOP (arg_node)) {
        node_icms = MakeIcm_WL_ADJUST_OFFSET (arg_node, NULL);
    } else {
        if (WLXBLOCK_NEXTDIM (arg_node) != NULL) {
            DBUG_ASSERT (WLXBLOCK_CONTENTS (arg_node) == NULL,
                         "CONTENTS and NEXTDIM used simultaneous!");

            node_icms = TRAVdo (WLXBLOCK_NEXTDIM (arg_node), arg_info);
        }

        if (WLXBLOCK_CONTENTS (arg_node) != NULL) {
            DBUG_ASSERT (WLXBLOCK_NEXTDIM (arg_node) == NULL,
                         "CONTENTS and NEXTDIM used simultaneous!");

            node_icms = TRAVdo (WLXBLOCK_CONTENTS (arg_node), arg_info);
        }
    }

    /*******************************************
     * create ICMs for block loop              *
     *******************************************/

    if (WLXBLOCK_ISNOOP (arg_node)) {
        DBUG_ASSERT (level == 0, "inner NOOP N_wl...-node found!");

        if (offset_needed) {
            if (is_block) {
                if (mt_active) {
                    icm_name_begin = "WL_MT_BLOCK_NOOP_BEGIN";
                    icm_name_end = "WL_MT_BLOCK_NOOP_END";
                } else {
                    icm_name_begin = "WL_BLOCK_NOOP_BEGIN";
                    icm_name_end = "WL_BLOCK_NOOP_END";
                }
            } else {
                if (mt_active) {
                    icm_name_begin = "WL_MT_UBLOCK_NOOP_BEGIN";
                    icm_name_end = "WL_MT_UBLOCK_NOOP_END";
                } else {
                    icm_name_begin = "WL_UBLOCK_NOOP_BEGIN";
                    icm_name_end = "WL_UBLOCK_NOOP_END";
                }
            }
        }
    } else {
        if (is_block) {
            if (mt_active) {
                if (level == 0) {
                    icm_name_begin = "WL_MT_BLOCK_LOOP0_BEGIN";
                } else {
                    icm_name_begin = "WL_MT_BLOCK_LOOP_BEGIN";
                }
                icm_name_end = "WL_MT_BLOCK_LOOP_END";
            } else {
                if (level == 0) {
                    icm_name_begin = "WL_BLOCK_LOOP0_BEGIN";
                } else {
                    icm_name_begin = "WL_BLOCK_LOOP_BEGIN";
                }
                icm_name_end = "WL_BLOCK_LOOP_END";
            }
        } else {
            if (mt_active) {
                if (level == 0) {
                    icm_name_begin = "WL_MT_UBLOCK_LOOP0_BEGIN";
                } else {
                    icm_name_begin = "WL_MT_UBLOCK_LOOP_BEGIN";
                }
                icm_name_end = "WL_MT_UBLOCK_LOOP_END";
            } else {
                if (level == 0) {
                    icm_name_begin = "WL_UBLOCK_LOOP0_BEGIN";
                } else {
                    icm_name_begin = "WL_UBLOCK_LOOP_BEGIN";
                }
                icm_name_end = "WL_UBLOCK_LOOP_END";
            }
        }
    }

    if (icm_name_begin != NULL) {
        begin_icm
          = TCmakeAssignIcm1 (icm_name_begin, MakeIcmArgs_WL_LOOP2 (arg_node), NULL);
    }

    if (icm_name_end != NULL) {
        end_icm = TCmakeAssignIcm1 (icm_name_end, MakeIcmArgs_WL_LOOP2 (arg_node), NULL);
    }

    /*******************************************
     * create ICMs for successor               *
     *******************************************/

    if (WLXBLOCK_NEXT (arg_node) != NULL) {
        next_icms = TRAVdo (WLXBLOCK_NEXT (arg_node), arg_info);
    }

    /*******************************************
     * put it all together                     *
     *******************************************/

    ret_node = TCmakeAssigns5 (MakeIcm_MT_ADJUST_SCHEDULER (arg_node, NULL), begin_icm,
                               node_icms, end_icm, next_icms);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPwlblock( node *arg_node, info *arg_info)
 *
 * @brief  Compilation of a N_wlblock-node:
 *     Returns a N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by COMPWith2() only!!)
 *
 * remarks:
 *   - 'wlids' points always to LET_IDS of the current with-loop.
 *   - 'wlnode' points always to the N_Nwith2-node.
 *   - 'wlseg' points to the current with-loop segment.
 *     (N_wlseg- *or* N_wlsegVar-node!)
 *
 ******************************************************************************/

node *
COMPwlblock (node *arg_node, info *arg_info)
{
    node *res;

    DBUG_ENTER ();

    res = COMPwlxblock (arg_node, arg_info);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPwlublock( node *arg_node, info *arg_info)
 *
 * @brief  Compilation of a N_wlublock-node:
 *     Returns a N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by COMPWith2() only!!)
 *
 * remarks:
 *   - 'wlids' points always to LET_IDS of the current with-loop.
 *   - 'wlnode' points always to the N_Nwith2-node.
 *   - 'wlseg' points to the current with-loop segment.
 *     (N_wlseg- *or* N_wlsegVar-node!)
 *
 ******************************************************************************/

node *
COMPwlublock (node *arg_node, info *arg_info)
{
    node *res;

    DBUG_ENTER ();

    res = COMPwlxblock (arg_node, arg_info);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPwlstride( node *arg_node, info *arg_info)
 *
 * @brief  Compilation of a N_wlstride- or N_wlstridevar-node:
 *     Returns a N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by COMPWith2() only!!)
 *
 * remarks:
 *   - 'wlids' points always to LET_IDS of the current with-loop.
 *   - 'wlnode' points always to the N_Nwith2-node.
 *   - 'wlseg' points to the current with-loop segment.
 *     (N_wlseg- *or* N_wlsegVar-node!)
 *   - 'wlstride' points to the current with-loop stride.
 *     (N_wlstride- *or* N_wlstrideVar-node!)
 *
 ******************************************************************************/
node *
COMPwlstride (node *arg_node, info *arg_info)
{
    node *old_wlstride;
    int level;
    bool mt_active, offset_needed;
    node *ret_node;
    const char *icm_name_begin = NULL;
    char *icm_name_end = NULL;
    node *begin_icm = NULL;
    node *end_icm = NULL;
    node *node_icms = NULL;
    node *next_icms = NULL;

    DBUG_ENTER ();

    /*
     * stack old 'wlstride'.
     * store pointer to current segment in 'wlstride'.
     */
    old_wlstride = wlstride;
    wlstride = arg_node;

    level = WLSTRIDE_LEVEL (arg_node);

    mt_active = WITH2_PARALLELIZE (wlnode);
    offset_needed = WITH2_NEEDSOFFSET (wlnode);

    /*******************************************
     * create ICMs for next dim / contents     *
     *******************************************/

    if (WLSTRIDE_ISNOOP (arg_node)) {
        node_icms = MakeIcm_WL_ADJUST_OFFSET (arg_node, NULL);
    } else {
        if (WLSTRIDE_CONTENTS (arg_node) != NULL) {
            node_icms = TRAVdo (WLSTRIDE_CONTENTS (arg_node), arg_info);
        }
    }

    /*******************************************
     * create ICMs for stride loop             *
     *******************************************/

    if (WLSTRIDE_ISNOOP (arg_node)) {
        DBUG_ASSERT (level == 0, "inner NOOP N_wl...-node found!");

        if (offset_needed) {
            if (mt_active) {
                icm_name_begin = "WL_MT_STRIDE_NOOP_BEGIN";
                icm_name_end = "WL_MT_STRIDE_NOOP_END";
            } else {
                icm_name_begin = "WL_STRIDE_NOOP_BEGIN";
                icm_name_end = "WL_STRIDE_NOOP_END";
            }
        }
    } else if ((!WLSTRIDE_ISDYNAMIC (arg_node)) && (WLSTRIDE_DOUNROLL (arg_node))) {
        int cnt_unroll;
        node *new_icms = NULL;

        /*
         * unrolling
         */
        DBUG_ASSERT (level > 0, "outer UNROLLING stride found!");

        if (mt_active) {
            icm_name_begin = "WL_MT_STRIDE_UNROLL_BEGIN";
            icm_name_end = "WL_MT_STRIDE_UNROLL_END";
        } else {
            icm_name_begin = "WL_STRIDE_UNROLL_BEGIN";
            icm_name_end = "WL_STRIDE_UNROLL_END";
        }

        DBUG_ASSERT (((NUM_VAL (WLSTRIDE_BOUND2 (arg_node))
                       - NUM_VAL (WLSTRIDE_BOUND1 (arg_node)))
                      % NUM_VAL (WLSTRIDE_STEP (arg_node)))
                       == 0,
                     "'step' is not a divisor of 'bound2 - bound1'!");

        for (cnt_unroll = (NUM_VAL (WLSTRIDE_BOUND2 (arg_node))
                           - NUM_VAL (WLSTRIDE_BOUND1 (arg_node)))
                          / NUM_VAL (WLSTRIDE_STEP (arg_node));
             cnt_unroll > 1; cnt_unroll--) {
            new_icms = TCappendAssign (DUPdoDupTree (node_icms), new_icms);
        }
        node_icms = TCappendAssign (node_icms, new_icms);
    } else {
        /*
         * no unrolling
         */

        if (mt_active || global.backend == BE_distmem) {
            if (level == 0) {
                icm_name_begin = "WL_MT_STRIDE_LOOP0_BEGIN";
            } else {
                icm_name_begin = (WLSTRIDE_NEXT (arg_node) != NULL)
                                   ? "WL_MT_STRIDE_LOOP_BEGIN"
                                   : "WL_MT_STRIDE_LAST_LOOP_BEGIN";
            }
            icm_name_end = "WL_MT_STRIDE_LOOP_END";
        } else {
            if (level == 0) {
                icm_name_begin = "WL_STRIDE_LOOP0_BEGIN";
            } else {
                icm_name_begin = (WLSTRIDE_NEXT (arg_node) != NULL)
                                   ? "WL_STRIDE_LOOP_BEGIN"
                                   : "WL_STRIDE_LAST_LOOP_BEGIN";
            }
            icm_name_end = "WL_STRIDE_LOOP_END";
        }
    }

    if (icm_name_begin != NULL) {
        begin_icm
          = TCmakeAssignIcm1 (icm_name_begin, MakeIcmArgs_WL_LOOP2 (arg_node), NULL);
    }

    if (icm_name_end != NULL) {
        end_icm = TCmakeAssignIcm1 (icm_name_end, MakeIcmArgs_WL_LOOP2 (arg_node), NULL);
    }

    /*******************************************
     * create ICMs for successor               *
     *******************************************/

    if (WLSTRIDE_NEXT (arg_node) != NULL) {
        next_icms = TRAVdo (WLSTRIDE_NEXT (arg_node), arg_info);
    }

    /*******************************************
     * put it all together                     *
     *******************************************/

    ret_node = TCmakeAssigns5 (MakeIcm_MT_ADJUST_SCHEDULER (arg_node, NULL), begin_icm,
                               node_icms, end_icm, next_icms);

    /*
     * restore old 'wlstride'.
     */
    wlstride = old_wlstride;

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPwlgrid( node *arg_node, info *arg_info)
 *
 * @brief  Compilation of a N_wlgrid- or N_wlgridvar-node:
 *     Returns a N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by COMPWith2() only!!)
 *
 * remarks:
 *   - 'wlids' points always to LET_IDS of the current with-loop.
 *   - 'wlnode' points always to the N_with-node.
 *   - 'wlseg' points to the current with-loop segment.
 *     (N_wlseg- *or* N_wlsegVar-node!)
 *   - 'wlstride' points to the current with-loop stride.
 *     (N_wlstride- *or* N_wlstrideVar-node!)
 *
 ******************************************************************************/
node *
COMPwlgrid (node *arg_node, info *arg_info)
{
    bool mt_active, is_fitted;
    node *ret_node;
    char *icm_name_begin = NULL;
    char *icm_name_end = NULL;
    node *begin_icm = NULL;
    node *end_icm = NULL;
    node *node_icms = NULL;
    node *next_icms = NULL;

    DBUG_ENTER ();

    mt_active = WITH2_PARALLELIZE (wlnode);
    is_fitted = WLGRID_ISFITTED (arg_node);

    if (WLGRID_ISNOOP (arg_node)) {
        node_icms = MakeIcm_WL_ADJUST_OFFSET (arg_node, NULL);
    } else {
        /*******************************************
         * create ICMs for next dim                *
         *******************************************/

        if (WLGRID_NEXTDIM (arg_node) != NULL) {
            DBUG_ASSERT (WLGRID_CODE (arg_node) == NULL,
                         "CODE and NEXTDIM used simultaneous!");

            node_icms
              = MakeIcm_WL_SET_OFFSET (arg_node,
                                       TRAVdo (WLGRID_NEXTDIM (arg_node), arg_info));
        } else {

            /*******************************************
             * create ICMs for code block              *
             *******************************************/

            node *icm_args = NULL;
            char *icm_name = NULL;
            node *cexpr;
            node *tmp_ids;
            node *withop;
            node *idxs_exprs;
            node *cexprs;

            DBUG_ASSERT (WLGRID_CODE (arg_node) != NULL,
                         "WLGRIDX_CODE must not be NULL!");

            /*
             * insert compiled code.
             */
            cexprs = CODE_CEXPRS (WLGRID_CODE (arg_node));

            DBUG_ASSERT (WLGRID_CBLOCK (arg_node) != NULL,
                         "no code block found in N_Ncode node");
            DBUG_ASSERT (WLGRID_CBLOCK_ASSIGNS (arg_node) != NULL,
                         "first instruction of block is NULL"
                         " (should be a N_empty node)");

            node_icms = DUPdoDupTree (WLGRID_CBLOCK_ASSIGNS (arg_node));

            /* for every ids of wlids (multioperator WL) */
            tmp_ids = wlids;
            idxs_exprs = WITH2_IDXS (wlnode);
            withop = WITH2_WITHOP (wlnode);

            while (tmp_ids != NULL && cexprs != NULL) {
                cexpr = EXPRS_EXPR (cexprs);
                /*
                 * choose right ICM
                 */
                switch (NODE_TYPE (withop)) {
                case N_genarray:
                    /* here is no break missing! */
                case N_modarray:
                    DBUG_ASSERT (NODE_TYPE (cexpr) == N_id, "code expr is not a id");

                    if (WITHOP_IDX (withop) != NULL) {
                        icm_name = "WL_INC_OFFSET";
                        icm_args = TBmakeExprs (DUPdupIdNt (EXPRS_EXPR (idxs_exprs)),
                                                TBmakeExprs (DUPdupIdNt (cexpr), NULL));
                        idxs_exprs = EXPRS_NEXT (idxs_exprs);
                    }
                    break;

                case N_fold:
                    icm_name = "WL_FOLD";
                    icm_args = MakeIcmArgs_WL_OP2 (arg_node, tmp_ids);
                    break;

                case N_propagate:
                    break;

                case N_break:
                    icm_name = "BREAK_ON_GUARD";
                    icm_args = TBmakeExprs (DUPdoDupTree (BREAK_MEM (withop)),
                                            TBmakeExprs (TCmakeIdCopyString (
                                                           INFO_BREAKLABEL (arg_info)),
                                                         NULL));
                    break;

                default:
                    DBUG_UNREACHABLE ("illegal withop type found");
                    icm_name = NULL;
                    icm_args = NULL;
                    break;
                }

                if (icm_name != NULL) {
                    node_icms
                      = TCappendAssign (node_icms,
                                        TCmakeAssignIcm1 (icm_name, icm_args, NULL));
                    icm_name = NULL;
                }

                cexprs = EXPRS_NEXT (cexprs);
                tmp_ids = IDS_NEXT (tmp_ids);
                withop = WITHOP_NEXT (withop);
            }
        }
    }

    /*******************************************
     * create ICMs for grid loop               *
     *******************************************/

    if ((WITHID_VECNEEDED (WITH2_WITHID (wlnode))) && (!WLGRID_ISNOOP (arg_node))
        && ((WLGRID_CODE (arg_node) != NULL) || (WLGRID_NEXTDIM (arg_node) != NULL))) {
        DBUG_PRINT ("IV %s is built! :(", ID_NAME (WITH2_VEC (wlnode)));
        node_icms = TCmakeAssignIcm1 ("WL_SET_IDXVEC", MakeIcmArgs_WL_LOOP1 (arg_node),
                                      node_icms);
    }

    if (WLGRID_ISNOOP (arg_node)) {
        if (mt_active) {
            if (is_fitted) {
                icm_name_begin = "WL_MT_GRID_NOOP_BEGIN";
                icm_name_end = "WL_MT_GRID_NOOP_END";
            } else {
                icm_name_begin = "WL_MT_GRID_FIT_NOOP_BEGIN";
                icm_name_end = "WL_MT_GRID_FIT_NOOP_END";
            }
        } else {
            if (is_fitted) {
                icm_name_begin = "WL_GRID_NOOP_BEGIN";
                icm_name_end = "WL_GRID_NOOP_END";
            } else {
                icm_name_begin = "WL_GRID_FIT_NOOP_BEGIN";
                icm_name_end = "WL_GRID_FIT_NOOP_END";
            }
        }
    } else {
        if ((!WLGRID_ISDYNAMIC (arg_node))
            && ((WLGRID_DOUNROLL (arg_node))
                || ((NUM_VAL (WLGRID_BOUND2 (arg_node))
                       - NUM_VAL (WLGRID_BOUND1 (arg_node))
                     == 1)
                    && is_fitted))) {
            int cnt_unroll;
            node *new_icms = NULL;

            /*
             * unrolling  or  (width == 1) and fitted already
             */

            if (mt_active) {
                icm_name_begin = "WL_MT_GRID_UNROLL_BEGIN";
                icm_name_end = "WL_MT_GRID_UNROLL_END";
            } else {
                icm_name_begin = "WL_GRID_UNROLL_BEGIN";
                icm_name_end = "WL_GRID_UNROLL_END";
            }

            for (cnt_unroll = NUM_VAL (WLGRID_BOUND2 (arg_node))
                              - NUM_VAL (WLGRID_BOUND1 (arg_node));
                 cnt_unroll > 1; cnt_unroll--) {
                new_icms = TCappendAssign (DUPdoDupTree (node_icms), new_icms);
            }
            node_icms = TCappendAssign (node_icms, new_icms);
        } else {
            /*
             * no unrolling   and   (width > 1) or not fitted
             */

            if (mt_active) {
                if (is_fitted) {
                    icm_name_begin = "WL_MT_GRID_LOOP_BEGIN";
                    icm_name_end = "WL_MT_GRID_LOOP_END";
                } else {
                    icm_name_begin = "WL_MT_GRID_FIT_LOOP_BEGIN";
                    icm_name_end = "WL_MT_GRID_FIT_LOOP_END";
                }
            } else {
                if (is_fitted) {
                    icm_name_begin = "WL_GRID_LOOP_BEGIN";
                    icm_name_end = "WL_GRID_LOOP_END";
                } else {
                    icm_name_begin = "WL_GRID_FIT_LOOP_BEGIN";
                    icm_name_end = "WL_GRID_FIT_LOOP_END";
                }
            }
        }
    }

    if (icm_name_begin != NULL) {
        begin_icm
          = TCmakeAssignIcm1 (icm_name_begin, MakeIcmArgs_WL_LOOP1 (arg_node), NULL);
    }

    if (icm_name_end != NULL) {
        end_icm = TCmakeAssignIcm1 (icm_name_end, MakeIcmArgs_WL_LOOP1 (arg_node), NULL);
    }

    /*******************************************
     * create ICMs for successor               *
     *******************************************/

    if (WLGRID_NEXT (arg_node) != NULL) {
        next_icms = TRAVdo (WLGRID_NEXT (arg_node), arg_info);
    }

    /*******************************************
     * put it all together                     *
     *******************************************/

    ret_node = TCmakeAssigns4 (begin_icm, node_icms, end_icm, next_icms);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPcode( node *arg_node, info *arg_info)
 *
 * @brief  Compiles a N_ncode node.
 *
 ******************************************************************************/

node *
COMPcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    CODE_NEXT (arg_node) = TRAVopt(CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
