/*
 * $Id$
 *
 * @file compile.c
 *
 * This file implements the code generation (SAC code -> C code with ICMs) for
 * the new backend
 */

#include "compile.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"

#include "globals.h"
#include "dbug.h"
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
#include "shape.h"
#include "LookUpTable.h"
#include "convert.h"
#include "math_utils.h"
#include "types.h"

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
    bool isfold;
    node *spmdframe;
    node *spmdbarrier;
    char *break_label;
    lut_t *foldlut;
    node *postfun;
    bool concurrentranges;
    bool cond;
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
#define INFO_ISFOLD(n) ((n)->isfold)
#define INFO_SPMDFRAME(n) ((n)->spmdframe)
#define INFO_SPMDBARRIER(n) ((n)->spmdbarrier)
#define INFO_BREAKLABEL(n) ((n)->break_label)
#define INFO_FOLDLUT(n) ((n)->foldlut)
#define INFO_POSTFUN(n) ((n)->postfun)
#define INFO_CONCURRENTRANGES(n) ((n)->concurrentranges)
#define INFO_COND(n) ((n)->cond)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

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
    INFO_ISFOLD (result) = FALSE;
    INFO_SPMDFRAME (result) = NULL;
    INFO_SPMDBARRIER (result) = NULL;
    INFO_FOLDLUT (result) = NULL;
    INFO_POSTFUN (result) = NULL;
    INFO_COND (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

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

#define RC_INIT(rc) (RC_IS_ACTIVE (rc) ? 1 : (rc))

#define RC_IS_LEGAL(rc) ((RC_IS_INACTIVE (rc)) || (RC_IS_ACTIVE (rc)))

#define RC_IS_ZERO(rc) ((rc) == 0)
#define RC_IS_VITAL(rc) ((rc) > 0)

/******************************************************************************
 *
 * Enum type for specifying kinds of Generic Functions
 *
 *****************************************************************************/

typedef enum { GF_copy, GF_free } generic_fun_t;

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

/*
 * This macro indicates whether there are multiple segments present or not.
 */
#define MULTIPLE_SEGS(seg) ((seg != NULL) && (WLSEGX_NEXT (seg) != NULL))

/******************************************************************************
 *
 * static global variables for the compilation of primitive functions
 *
 ******************************************************************************/

static char *prf_ccode_tab[] = {
#define PRFccode(ccode) "SAC_ND_PRF_" #ccode
#include "prf_info.mac"
};

/** <!--********************************************************************-->
 *
 * @fn  char *GetBasetypeStr( types *type)
 *
 * @brief  Returns the basetype string of the given type, i.e. "TYPES_NAME" if
 *         type represents a user-defined type and "TYPES_BASETYPE" otherwise.
 *
 ******************************************************************************/

static const char *
GetBasetypeStr (types *type)
{
    simpletype basetype;
    const char *str;

    DBUG_ENTER ("GetBasetypeStr");

    basetype = TCgetBasetype (type);

    if (basetype == T_user) {
        str = TYPES_NAME (type);
        DBUG_ASSERT ((str != NULL), "Name of user-defined type not found");
    } else if (basetype == T_float_dev) {
        str = "float";
    } else if (basetype == T_int_dev) {
        str = "int";
    } else {
        str = global.type_string[basetype];
    }

    DBUG_RETURN (str);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeBasetypeArg( types *type)
 *
 * @brief  Creates a new N_id node containing the basetype string of the given
 *         type.
 *
 ******************************************************************************/

static node *
MakeBasetypeArg (types *type)
{
    node *ret_node;
    const char *str;

    DBUG_ENTER ("MakeBasetypeArg");

    str = GetBasetypeStr (type);

    ret_node = TCmakeIdCopyString (str);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeBasetypeArg_NT( types *type)
 *
 * @brief  Creates a new N_id node containing the basetype string of the given
 *         type.
 *
 ******************************************************************************/

static node *
MakeBasetypeArg_NT (types *type)
{
    node *ret_node;
    const char *str;

    DBUG_ENTER ("MakeBasetypeArg_NT");

    str = GetBasetypeStr (type);

    ret_node = TCmakeIdCopyStringNt (str, type);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeTypeArgs( char *name, types *type,
 *                          bool add_type, bool add_dim, bool add_shape,
 *                          node *exprs)
 *
 * @brief  Creates a chain of N_exprs nodes containing name, type,
 *         dimensionality and shape components of the given object.
 *
 ******************************************************************************/

static node *
MakeTypeArgs (char *name, types *type, bool add_type, bool add_dim, bool add_shape,
              node *exprs)
{
    int dim;

    DBUG_ENTER ("MakeTypeArgs");

    dim = TCgetShapeDim (type);

    /*
     * CAUTION:
     * It is important that (dim <= 0) holds for AKD and AUD arrays
     * otherwise the VARINT-interpretation of the shape-args would fail
     * during icm2c!!
     */
    if (add_shape) {
        if (dim == 0) {
            /* SCL */
        } else if (dim > 0) {
            /* AKS */
            exprs = TCappendExprs (TCtype2Exprs (type), exprs);
        } else if (dim < KNOWN_DIM_OFFSET) {
            /* AKD */
        } else {
            /* AUD */
        }
    }

    if (add_dim) {
        exprs = TBmakeExprs (TBmakeNum (dim), exprs);
    }

    if (add_type) {
        exprs = TBmakeExprs (MakeBasetypeArg (type), exprs);
    }

    exprs = TBmakeExprs (TCmakeIdCopyStringNt (name, type), exprs);

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

    DBUG_ENTER ("MakeDimArg");

    if (NODE_TYPE (arg) == N_id) {
        int dim = TCgetDim (ID_TYPE (arg));
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
        DBUG_ASSERT ((0), "not yet implemented");
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

    DBUG_ENTER ("MakeSizeArg");

    if (NODE_TYPE (arg) == N_id) {
        types *type = ID_TYPE (arg);
        if (TCgetShapeDim (type) >= 0) {
            ret = TBmakeNum (TCgetTypesLength (type));
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
        DBUG_ASSERT ((0), "not yet implemented");
        ret = NULL;
    }

    DBUG_RETURN (ret);
}

/** <!--********************************************************************-->
 *
 * @fn  char *GenericFun( generic_fun_t which, types *type)
 *
 * @brief  Returns the name of the specified generic function for the given
 *         type.
 *
 * @param which
 *
 ******************************************************************************/

static char *
GenericFun (generic_fun_t which, types *type)
{
    node *tdef;
    char *ret = NULL;
#ifndef DBUG_OFF
    char *tmp;
#endif
    usertype utype;

    DBUG_ENTER ("GenericFun");

    DBUG_EXECUTE ("COMP", tmp = CVtype2String (type, 0, FALSE); switch (which) {
        case GF_copy:
            DBUG_PRINT ("COMP", ("Looking for generic copy functioni for type %s", tmp));
            break;
        case GF_free:
            DBUG_PRINT ("COMP", ("Looking for generic free function for type %s", tmp));
            break;
    } tmp = MEMfree (tmp););

    DBUG_ASSERT ((type != NULL), "no type found!");

    if (TYPES_BASETYPE (type) == T_user) {
        tdef = TYPES_TDEF (type);
        DBUG_ASSERT ((tdef != NULL), "Failed attempt to look up typedef");

        utype = UTfindUserType (TYPEDEF_NAME (tdef), TYPEDEF_NS (tdef));

        DBUG_ASSERT ((utype != UT_NOT_DEFINED)
                       && (!TYisUser (TYgetScalar (UTgetBaseType (utype)))),
                     "unresolved nested user-defined type found");

        if (TYgetSimpleType (TYgetScalar (UTgetBaseType (utype))) == T_hidden) {
            switch (which) {
            case GF_copy:
                ret = TYPEDEF_COPYFUN (tdef);
                break;
            case GF_free:
                ret = TYPEDEF_FREEFUN (tdef);
                break;
            }
        }
    }

    DBUG_PRINT ("COMP", ("Found generic fun `%s'", STRonNull ("", ret)));

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

    DBUG_ENTER ("COMPgetFoldCode");

    DBUG_ASSERT ((fundef != NULL), "no fundef found!");
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "fold-fun corrupted!");

    /*
     * get code of the special fold-fun
     */
    fold_code = DUPdoDupTree (FUNDEF_INSTR (fundef));

    /*
     * remove declaration-ICMs ('ND_DECL_ARG') from code.
     */
    while ((NODE_TYPE (ASSIGN_INSTR (fold_code)) == N_icm)
           && (STReq (ICM_NAME (ASSIGN_INSTR (fold_code)), "ND_DECL__MIRROR_PARAM"))) {
        fold_code = FREEdoFreeNode (fold_code);
    }

    /*
     * remove return-ICMs ('ND_FUN_RET') from code
     * (it is the last assignment)
     */
    tmp = fold_code;
    DBUG_ASSERT ((ASSIGN_NEXT (tmp) != NULL), "corrupted fold code found!");
    while (ASSIGN_NEXT (ASSIGN_NEXT (tmp)) != NULL) {
        tmp = ASSIGN_NEXT (tmp);
    }
    DBUG_ASSERT (((NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (tmp))) == N_icm)
                  && (STReq (ICM_NAME (ASSIGN_INSTR (ASSIGN_NEXT (tmp))), "ND_FUN_RET"))),
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

    DBUG_ENTER ("RemoveIdxDuplicates");

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

    DBUG_ENTER ("DupExpr_NT_AddReadIcms");

    DBUG_ASSERT (((expr != NULL) && (NODE_TYPE (expr) != N_exprs)),
                 "Illegal argument for DupExpr_NT_AddReadIcms() found!");

    if (NODE_TYPE (expr) == N_prf) {
        new_expr = TBmakePrf (PRF_PRF (expr), DupExprs_NT_AddReadIcms (PRF_ARGS (expr)));
    } else if (NODE_TYPE (expr) == N_id) {
        new_expr = DUPdupIdNt (expr);
        if (TCgetShapeDim (ID_TYPE (expr)) == SCALAR) {
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

    DBUG_ENTER ("DupExprs_NT_AddReadIcms");

    if (exprs != NULL) {
        // DBUG_ASSERT( (NODE_TYPE( exprs) == N_exprs), "no N_exprs node found!");

        if ((NODE_TYPE (exprs) == N_exprs)) {

        } else {
            printf ("no N_exprs node found!\n");
        }

        new_exprs = TBmakeExprs (DupExpr_NT_AddReadIcms (EXPRS_EXPR (exprs)),
                                 DupExprs_NT_AddReadIcms (EXPRS_NEXT (exprs)));
    }

    DBUG_RETURN (new_exprs);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeAllocDescIcm( char *name, types *type, int rc,
 *                              node *get_dim,
 *                              node *assigns)
 *
 * @brief  Builds a ND_ALLOC__DESC( name) icm if needed.
 *
 ******************************************************************************/

static node *
MakeAllocDescIcm (char *name, types *type, int rc, node *get_dim, node *assigns)
{
    int dim;

    DBUG_ENTER ("MakeAllocDescIcm");

    DBUG_ASSERT ((RC_IS_LEGAL (rc)), "illegal RC value found!");

    if (RC_IS_ACTIVE (rc)) {
        if (get_dim == NULL) {
            dim = TCgetDim (type);
            DBUG_ASSERT ((dim >= 0), "dimension undefined -> size of descriptor unknown");
            get_dim = TBmakeNum (dim);
        }

        assigns = TCmakeAssignIcm2 ("ND_ALLOC__DESC", TCmakeIdCopyStringNt (name, type),
                                    get_dim, assigns);
    }

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeSetRcIcm( char *name, types *type, int rc, node *assigns)
 *
 * @brief  Builds a ND_SET__RC( name, num) icm if needed.
 *
 ******************************************************************************/

static node *
MakeSetRcIcm (char *name, types *type, int rc, node *assigns)
{
    DBUG_ENTER ("MakeSetRcIcm");

    DBUG_ASSERT ((RC_IS_LEGAL (rc)), "illegal RC value found!");

    if (RC_IS_ACTIVE (rc)) {
        if (RC_IS_VITAL (rc)) {
            assigns = TCmakeAssignIcm2 ("ND_SET__RC", TCmakeIdCopyStringNt (name, type),
                                        TBmakeNum (rc), assigns);
        } else {
            if (TCgetBasetype (type) != T_float_dev
                && TCgetBasetype (type) != T_int_dev) {
                assigns
                  = TCmakeAssignIcm2 ("ND_FREE", TCmakeIdCopyStringNt (name, type),
                                      TCmakeIdCopyString (GenericFun (GF_free, type)),
                                      assigns);
            } else {
                assigns
                  = TCmakeAssignIcm2 ("CUDA_FREE", TCmakeIdCopyStringNt (name, type),
                                      TCmakeIdCopyString (GenericFun (GF_free, type)),
                                      assigns);
            }
        }
    }

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeIncRcIcm( char *name, types *type, int num,
 *                          node *assigns)
 *
 * @brief  Builds a ND_INC_RC( name, num) icm if needed.
 *
 ******************************************************************************/

static node *
MakeIncRcIcm (char *name, types *type, int num, node *assigns)
{
    DBUG_ENTER ("MakeIncRcIcm");

    DBUG_ASSERT ((num >= 0), "increment for rc must be >= 0.");

    if (num > 0) {
        assigns = TCmakeAssignIcm2 ("ND_INC_RC", TCmakeIdCopyStringNt (name, type),
                                    TBmakeNum (num), assigns);
    }

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeDecRcIcm( char *name, types *type, int num,
 *                          node *assigns)
 *
 * @brief  According to 'type', 'rc' and 'num', builds a
 *         ND_DEC_RC_FREE( name, num, freefun) icm,
 *         or no icm at all.
 *
 ******************************************************************************/

static node *
MakeDecRcIcm (char *name, types *type, int num, node *assigns)
{
    DBUG_ENTER ("MakeDecRcIcm");

    DBUG_ASSERT ((num >= 0), "decrement for rc must be >= 0.");

    if (num > 0) {
        if (TCgetBasetype (type) != T_float_dev && TCgetBasetype (type) != T_int_dev) {
            assigns
              = TCmakeAssignIcm3 ("ND_DEC_RC_FREE", TCmakeIdCopyStringNt (name, type),
                                  TBmakeNum (num),
                                  TCmakeIdCopyString (GenericFun (GF_free, type)),
                                  assigns);
        } else {
            assigns
              = TCmakeAssignIcm3 ("CUDA_DEC_RC_FREE", TCmakeIdCopyStringNt (name, type),
                                  TBmakeNum (num),
                                  TCmakeIdCopyString (GenericFun (GF_free, type)),
                                  assigns);
        }
    }

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeAllocIcm( char *name, types *type, int rc,
 *                          node *get_dim, node *set_shape_icm,
 *                          node *pragma, node *assign)
 *
 * @brief  Builds a ND_ALLOC icm.
 *         The given node 'assigns' is appended to the created assignment.
 *
 ******************************************************************************/

static node *
MakeAllocIcm (char *name, types *type, int rc, node *get_dim, node *set_shape_icm,
              node *pragma, node *assigns)
{
    DBUG_ENTER ("MakeAllocIcm");

    DBUG_ASSERT ((RC_IS_LEGAL (rc)), "illegal RC value found!");
    DBUG_ASSERT ((get_dim != NULL), "no dimension found!");
    DBUG_ASSERT (((set_shape_icm != NULL) && (NODE_TYPE (set_shape_icm) == N_icm)),
                 "no N_icm node found!");

    if (RC_IS_ACTIVE (rc)) {
        if (pragma == NULL) {
            /* This is an array that should be allocated on the device */
            if (TCgetBasetype (type) == T_float_dev
                || TCgetBasetype (type) == T_int_dev) {
#if USE_COMPACT_ALLOC
                assigns
                  = TCmakeAssignIcm3 ("ND_ALLOC", TCmakeIdCopyStringNt (name, type),
                                      TBmakeNum (rc), get_dim, set_shape_icm, assigns);
#else
                assigns = TCmakeAssignIcm4 (
                  "CUDA_ALLOC_BEGIN", TCmakeIdCopyStringNt (name, type), TBmakeNum (rc),
                  get_dim, MakeBasetypeArg (type),
                  TBmakeAssign (set_shape_icm,
                                TCmakeAssignIcm4 ("CUDA_ALLOC_END",
                                                  TCmakeIdCopyStringNt (name, type),
                                                  TBmakeNum (rc), DUPdoDupTree (get_dim),
                                                  MakeBasetypeArg (type), assigns)));
#endif
            } else {
#if USE_COMPACT_ALLOC
                assigns
                  = TCmakeAssignIcm3 ("ND_ALLOC", TCmakeIdCopyStringNt (name, type),
                                      TBmakeNum (rc), get_dim, set_shape_icm, assigns);
#else
                assigns = TCmakeAssignIcm4 (
                  "ND_ALLOC_BEGIN", TCmakeIdCopyStringNt (name, type), TBmakeNum (rc),
                  get_dim, MakeBasetypeArg (type),
                  TBmakeAssign (set_shape_icm,
                                TCmakeAssignIcm4 ("ND_ALLOC_END",
                                                  TCmakeIdCopyStringNt (name, type),
                                                  TBmakeNum (rc), DUPdoDupTree (get_dim),
                                                  MakeBasetypeArg (type), assigns)));
#endif
            }
        } else {
            /*
             * ALLOC_PLACE does not seem to be implemented somewhere
             */
            assigns
              = TCmakeAssignIcm5 ("ND_ALLOC_PLACE", TCmakeIdCopyStringNt (name, type),
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
 * @fn  node *MakeAllocIcm_IncRc( char *name, types *type, int rc,
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
MakeAllocIcm_IncRc (char *name, types *type, int rc, node *get_dim, node *set_shape_icm,
                    node *pragma, node *assigns)
{
    node *new_assigns;

    DBUG_ENTER ("MakeAllocIcm_IncRc");

    DBUG_ASSERT ((RC_IS_LEGAL (rc)), "illegal RC value found!");

    new_assigns = MakeAllocIcm (name, type, 0, get_dim, set_shape_icm, pragma, NULL);

    if (new_assigns != NULL) {
        DBUG_ASSERT ((RC_IS_VITAL (rc)), "INC_RC(rc) with (rc <= 0) found!");
        assigns = TCappendAssign (new_assigns,
                                  TCmakeAssignIcm2 ("ND_INC_RC",
                                                    TCmakeIdCopyStringNt (name, type),
                                                    TBmakeNum (rc), assigns));
    }

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeCheckReuseIcm( char *name, types *type, node *reuse_id,
 *                               node *assigns);
 *
 * @brief  Builds a CHECK_REUSE icm which checks whether reuse_id can be
 *         reused at runtime.
 *         The given node 'assigns' is appended to the created assignment.
 *
 ******************************************************************************/

static node *
MakeCheckReuseIcm (char *name, types *type, node *reuse_id, node *assigns)
{
    DBUG_ENTER ("MakeCheckReuseIcm");

    assigns
      = TCmakeAssignIcm2 ("ND_CHECK_REUSE",
                          MakeTypeArgs (name, type, FALSE, TRUE, FALSE,
                                        MakeTypeArgs (ID_NAME (reuse_id),
                                                      ID_TYPE (reuse_id), FALSE, TRUE,
                                                      FALSE, NULL)),
                          TCmakeIdCopyString (GenericFun (GF_copy, ID_TYPE (reuse_id))),
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

    DBUG_ENTER ("MakeGetDimIcm");

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
            DBUG_ASSERT ((0), "Unrecognized dim descriptor");
            break;
        }
        break;
    default:
        DBUG_ASSERT ((0), "Unrecognized dim descriptor");
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

    DBUG_ENTER ("MakeSetShapeIcm");

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
                                MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                              FALSE, TRUE, FALSE,
                                              MakeTypeArgs (ID_NAME (arg_node),
                                                            ID_TYPE (arg_node), FALSE,
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
                            val0_sdim = TCgetShapeDim (
                              ID_TYPE (EXPRS_EXPR (ARRAY_AELEMS (arg_node))));
                        } else {
                            val0_sdim = 0;
                        }
                    } else {
                        val0_sdim = -815; /* array is empty */
                    }

                    set_shape
                      = TCmakeIcm3 ("ND_CREATE__ARRAY__SHAPE",
                                    MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids),
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

                        DBUG_ASSERT ((NODE_TYPE (arg1) == N_id),
                                     "1st arg of F_cat_VxV is no N_id!");
                        DBUG_ASSERT ((NODE_TYPE (arg2) == N_id),
                                     "2nd arg of F_cat_VxV is no N_id!");

                        icm_args
                          = MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE,
                                          TRUE, FALSE,
                                          MakeTypeArgs (ID_NAME (arg1), ID_TYPE (arg1),
                                                        FALSE, TRUE, FALSE,
                                                        MakeTypeArgs (ID_NAME (arg2),
                                                                      ID_TYPE (arg2),
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
                        DBUG_ASSERT ((NODE_TYPE (arg2) == N_id),
                                     "2nd arg of F_drop_SxV is no N_id!");

                        icm_args
                          = MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE,
                                          TRUE, FALSE,
                                          MakeTypeArgs (ID_NAME (arg2), ID_TYPE (arg2),
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
                        DBUG_ASSERT ((NODE_TYPE (arg2) == N_id),
                                     "2nd arg of F_take_SxV is no N_id!");

                        icm_args
                          = MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE,
                                          TRUE, FALSE,
                                          MakeTypeArgs (ID_NAME (arg2), ID_TYPE (arg2),
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
                              IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE, TRUE, FALSE,
                              MakeTypeArgs (
                                ID_NAME (arg2), ID_TYPE (arg2), FALSE, TRUE, FALSE,
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

                            DBUG_ASSERT (((TCgetBasetype (ID_TYPE (arg1)) == T_int)),
                                         "1st arg of F_sel_VxA is a illegal indexing "
                                         "var!");

                            icm_args
                              = MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                              FALSE, TRUE, FALSE,
                                              MakeTypeArgs (ID_NAME (arg2),
                                                            ID_TYPE (arg2), FALSE, TRUE,
                                                            FALSE,
                                                            TBmakeExprs (DUPdupIdNt (
                                                                           arg1),
                                                                         NULL)));

                            set_shape = TCmakeIcm1 ("ND_PRF_SEL_VxA__SHAPE_id", icm_args);
                        }
                        break;

                    default:
                        DBUG_ASSERT ((0), "Unrecognized shape descriptor");
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
                        DBUG_ASSERT ((NODE_TYPE (arg2) == N_id),
                                     "2nd arg of F_idx_sel is no N_id!");

                        icm_args
                          = MakeTypeArgs (ID_NAME (arg2), ID_TYPE (arg2), FALSE, TRUE,
                                          FALSE, TBmakeExprs (DUPdupNodeNt (arg1), NULL));

                        set_shape = TCmakeIcm1 ("ND_PRF_IDX_SEL__SHAPE",
                                                MakeTypeArgs (IDS_NAME (let_ids),
                                                              IDS_TYPE (let_ids), FALSE,
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
                                                      IDS_TYPE (let_ids), FALSE, TRUE,
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
                                                      IDS_TYPE (let_ids), FALSE, TRUE,
                                                      FALSE,
                                                      TBmakeExprs (DUPdupIdNt (arg1),
                                                                   NULL)));
                        break;

                    default:
                        DBUG_ASSERT ((0), "Unrecognized shape descriptor");
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
                                                      IDS_TYPE (let_ids), FALSE, TRUE,
                                                      FALSE,
                                                      TBmakeExprs (DUPdupIdNt (arg1),
                                                                   MakeTypeArgs (ID_NAME (
                                                                                   arg2),
                                                                                 ID_TYPE (
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
                                                      IDS_TYPE (let_ids), FALSE, TRUE,
                                                      FALSE, NULL),
                                        MakeSizeArg (arg1, TRUE),
                                        DupExprs_NT_AddReadIcms (ARRAY_AELEMS (arg1)),
                                        MakeTypeArgs (ID_NAME (arg2), ID_TYPE (arg2),
                                                      FALSE, TRUE, FALSE, NULL));
                        break;
                    default:
                        DBUG_ASSERT ((0), "Unrecognized shape descriptor!");
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
                    DBUG_ASSERT ((0), "Unrecognized shape descriptor");
                    break;
                }
                break;

            default:
                DBUG_ASSERT ((0), "Unrecognized shape descriptor");
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
                                MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids),
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
                DBUG_ASSERT ((0), "Unrecognized shape descriptor");
                break;
            }
            break;

        default:
            DBUG_ASSERT ((0), "Unrecognized shape descriptor");
            break;
        }
        break;

    default:
        DBUG_ASSERT ((0), "Unrecognized shape descriptor");
        break;
    }

    DBUG_RETURN (set_shape);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeArgNode( int idx, types *type, bool thread)
 *
 * @brief  ...
 *         bool thread is this arg a mutc thread fun arg?
 *
 ******************************************************************************/

static node *
MakeArgNode (int idx, types *arg_type, bool thread)
{
    node *id;
    char *name;
    types *type;

    DBUG_ENTER ("MakeArgNode");

    type = DUPdupAllTypes (arg_type);

    /* Set usage tag of arg */
    if (thread) {
        TYPES_MUTC_USAGE (type) = MUTC_US_THREADPARAM;
    } else {
        TYPES_MUTC_USAGE (type) = MUTC_US_FUNPARAM;
    }

    name = MEMmalloc (20 * sizeof (char));
    sprintf (name, "SAC_arg_%d", idx);

    if (type != NULL) {
        id = TCmakeIdCopyStringNt (name, type);
    } else {
#if 1
        id = TCmakeIdCopyString (name);
#endif
    }

    name = MEMfree (name);

    type = FREEfreeAllTypes (type);

    DBUG_RETURN (id);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeIcm_MT_SPMD_FUN_DEC( node *fundef)
 *
 * @brief  Creates a MT_SPMD_FUN_DEC ICM.
 *
 ******************************************************************************/

static node *
MakeIcm_MT_SPMD_FUN_DEC (node *fundef)
{
    argtab_t *argtab;
    node *icm;
    int size;
    int i;
    node *icm_args = NULL;

    DBUG_ENTER ("MakeIcm_MT_SPMD_FUN_DEC");

    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no fundef node found!");

    argtab = FUNDEF_ARGTAB (fundef);
    DBUG_ASSERT ((argtab != NULL), "no argtab found!");
    DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab inconsistent!");

    /* arguments */
    for (i = argtab->size - 1; i >= 1; i--) {
        char *name;
        node *id;
        types *type;

        if (argtab->ptr_in[i] != NULL) {
            DBUG_ASSERT ((NODE_TYPE (argtab->ptr_in[i]) == N_arg),
                         "no N_arg node found in argtab");

            name = ARG_NAME (argtab->ptr_in[i]);
            type = ARG_TYPE (argtab->ptr_in[i]);
            id = TCmakeIdCopyStringNt (STRonNull ("", name), type);
        } else {
            DBUG_ASSERT ((argtab->ptr_out[i] != NULL), "argtab is uncompressed!");
            type = TYtype2OldType (RET_TYPE (argtab->ptr_out[i]));
            id = MakeArgNode (i, type, FALSE);
        }

        icm_args = TBmakeExprs (TCmakeIdCopyString (global.argtag_string[argtab->tag[i]]),
                                TBmakeExprs (MakeBasetypeArg (type),
                                             TBmakeExprs (id, icm_args)));
    }
    size = argtab->size - 1;

    /* return value */
    DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab is inconsistent!");
    if (argtab->ptr_out[0] != NULL) {
        types *type;
        type = TYtype2OldType (RET_TYPE (argtab->ptr_out[0]));
        icm_args = TBmakeExprs (TCmakeIdCopyString (global.argtag_string[argtab->tag[0]]),
                                TBmakeExprs (MakeBasetypeArg (type),
                                             TBmakeExprs (MakeArgNode (0, type, FALSE),
                                                          icm_args)));
        size++;
    }

    icm = TCmakeIcm3 ("MT_SPMD_FUN_DEC", TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                      TBmakeNum (size), icm_args);

    DBUG_RETURN (icm);
}

static node *
MakeIcm_CUDA_FUN_DEC (node *fundef)
{
    argtab_t *argtab;
    node *icm;
    int size;
    int i;
    node *icm_args = NULL;

    DBUG_ENTER ("MakeIcm_CUDA_FUN_DEC");

    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no fundef node found!");

    argtab = FUNDEF_ARGTAB (fundef);
    DBUG_ASSERT ((argtab != NULL), "no argtab found!");
    DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab inconsistent!");

    /* arguments */
    for (i = argtab->size - 1; i >= 1; i--) {
        char *name;
        node *id;
        types *type;

        if (argtab->ptr_in[i] != NULL) {
            DBUG_ASSERT ((NODE_TYPE (argtab->ptr_in[i]) == N_arg),
                         "no N_arg node found in argtab");

            name = ARG_NAME (argtab->ptr_in[i]);
            type = ARG_TYPE (argtab->ptr_in[i]);
            id = TCmakeIdCopyStringNt (STRonNull ("", name), type);
        } else {
            DBUG_ASSERT ((argtab->ptr_out[i] != NULL), "argtab is uncompressed!");
            type = TYtype2OldType (RET_TYPE (argtab->ptr_out[i]));
            id = MakeArgNode (i, type, FALSE);
        }

        icm_args
          = TBmakeExprs (TCmakeIdCopyString (global.argtag_string[argtab->tag[i]]),
                         TBmakeExprs (MakeBasetypeArg (type),
                                      TBmakeExprs (id, TBmakeExprs (TBmakeNum (
                                                                      TYPES_DIM (type)),
                                                                    icm_args))));
    }
    size = argtab->size - 1;

    /* return value */
    DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab is inconsistent!");
    if (argtab->ptr_out[0] != NULL) {
        types *type;
        type = TYtype2OldType (RET_TYPE (argtab->ptr_out[0]));
        icm_args
          = TBmakeExprs (TCmakeIdCopyString (global.argtag_string[argtab->tag[0]]),
                         TBmakeExprs (MakeBasetypeArg (type),
                                      TBmakeExprs (MakeArgNode (0, type, FALSE),
                                                   TBmakeExprs (TBmakeNum (
                                                                  TYPES_DIM (type)),
                                                                icm_args))));
        size++;
    }

    icm = TCmakeIcm3 ("CUDA_FUN_DEC", TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                      TBmakeNum (size), icm_args);

    DBUG_RETURN (icm);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeFunctionArgs( node *fundef, bool decl)
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
    int i;

    DBUG_ENTER ("MakeFunctionArgs");

    argtab = FUNDEF_ARGTAB (fundef);
    DBUG_ASSERT ((argtab != NULL), "no argtab found!");

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
        types *type;
        char *name;
        node *id;

        if (argtab->ptr_in[i] != NULL) {
            DBUG_ASSERT ((NODE_TYPE (argtab->ptr_in[i]) == N_arg),
                         "no N_arg node found in argtab");

            tag = argtab->tag[i];
            type = ARG_TYPE (argtab->ptr_in[i]);
            name = ARG_NAME (argtab->ptr_in[i]);
            if (name != NULL) {
                id = TCmakeIdCopyStringNt (name, type);
            } else {
                id = MakeArgNode (i, type, FALSE);
            }
        } else {
            DBUG_ASSERT ((argtab->ptr_out[i] != NULL), "argtab is uncompressed!");
            tag = argtab->tag[i];
            type = TYtype2OldType (RET_TYPE (argtab->ptr_out[i]));
            id = MakeArgNode (i, type, FALSE);
        }

        icm_args = TBmakeExprs (TCmakeIdCopyString (global.argtag_string[tag]),
                                TBmakeExprs (MakeBasetypeArg (type),
                                             TBmakeExprs (id, icm_args)));
    }

    if (FUNDEF_HASDOTARGS (fundef) || FUNDEF_HASDOTRETS (fundef)) {
        icm_args = TBmakeExprs (TBmakeNum (argtab->size), icm_args);
    } else {
        icm_args = TBmakeExprs (TBmakeNum (argtab->size - 1), icm_args);
    }

    /* return value */
    DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab inconsistent!");
    if (argtab->ptr_out[0] == NULL) {
        icm_args = TBmakeExprs (TCmakeIdCopyString (NULL), icm_args);
    } else {
        icm_args = TBmakeExprs (MakeBasetypeArg_NT (
                                  TYtype2OldType (RET_TYPE (argtab->ptr_out[0]))),
                                icm_args);
    }

    DBUG_RETURN (icm_args);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeFunctionSignature( node *fundef, bool decl)
 *
 * @brief  Create ICMs for the signature of a function
 *         The result is normally needed for FUNDEF_ICMDECL and
 *         FUNDEF_ICMDEFBEGIN
 *
 * @param fundef
 * @param decl  Is this a function declaration or definition?
 *
 ******************************************************************************/

static node *
MakeFunctionSignature (node *fundef, bool decl)
{
    node *ret_node;

    DBUG_ENTER ("MakeFunctionSignature");

    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no fundef node found!");

    if (decl) {
        if (FUNDEF_ISTHREADFUN (fundef)) {
            ret_node = TCmakeIcm2 ("MUTC_THREAD_FUN_DECL",
                                   TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                                   MakeFunctionArgs (fundef));
        } else if (FUNDEF_ISSPMDFUN (fundef)) {
            ret_node = MakeIcm_MT_SPMD_FUN_DEC (fundef);
        } else if (FUNDEF_ISCUDAGLOBALFUN (fundef)) {
            ret_node = MakeIcm_CUDA_FUN_DEC (fundef);
        } else {
            ret_node
              = TCmakeIcm2 ("ND_FUN_DECL", TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                            MakeFunctionArgs (fundef));
        }
    } else {
        if (FUNDEF_ISTHREADFUN (fundef)) {
            ret_node = TCmakeIcm2 ("MUTC_THREAD_FUN_DEF_BEGIN",
                                   TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                                   MakeFunctionArgs (fundef));
        } else if (FUNDEF_ISCUDAGLOBALFUN (fundef)) {
            ret_node = MakeIcm_CUDA_FUN_DEC (fundef);
        } else if (FUNDEF_ISSPMDFUN (fundef)) {
            ret_node = MakeIcm_MT_SPMD_FUN_DEC (fundef);
        } else {
            ret_node
              = TCmakeIcm2 ("ND_FUN_DEF_BEGIN", TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                            MakeFunctionArgs (fundef));
        }
    }
    DBUG_RETURN (ret_node);
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
    types *type = NULL;

    DBUG_ENTER ("GetBaseTypeFromAvis");
    DBUG_ASSERT ((in != NULL), "no node found!");

    in = AVIS_DECL (in);

    if (NODE_TYPE (in) == N_vardec) {
        type = VARDEC_TYPE (in);
    } else if (NODE_TYPE (in) == N_arg) {
        type = ARG_TYPE (in);
    } else {
        DBUG_ASSERT (FALSE, "Illegal node type found!");
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
    DBUG_ENTER ("GetBaseTypeFromExpr");
    DBUG_ASSERT ((in != NULL), "no node found!");

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
        ret = GetBasetypeStr (TYtype2OldType (OBJDEF_TYPE (in)));
    } else {
        DBUG_ASSERT (FALSE, "Unexpected node type found!");
    }

    DBUG_RETURN (ret);
}

/** <!--********************************************************************-->
 *
 * @fn  node *MakeIcm_FUN_AP( node *ap, node *fundef, node *assigns)
 *
 * @brief  Builds a N_assign node with the ND_FUN_AP icm.
 *
 ******************************************************************************/

static node *
MakeIcm_FUN_AP (node *ap, node *fundef, node *assigns)
{
    node *ret_node;
    argtab_t *argtab;
    int i;
    node *icm_args = NULL;

    DBUG_ENTER ("MakeIcm_FUN_AP");

    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no fundef node found!");

    DBUG_ASSERT (((ap != NULL) && (NODE_TYPE (ap) == N_ap)), "no ap node found!");

    argtab = AP_ARGTAB (ap);
    DBUG_ASSERT ((argtab != NULL), "no argtab found!");

    /* arguments */
    for (i = argtab->size - 1; i >= 1; i--) {
        node *exprs = NULL;

        if (argtab->ptr_out[i] != NULL) {
            exprs = TBmakeExprs (DUPdupIdsIdNt (argtab->ptr_out[i]), icm_args);

            if (!FUNDEF_ISCUDAGLOBALFUN (fundef)) {
                exprs = TBmakeExprs (TCmakeIdCopyString (
                                       GetBaseTypeFromExpr (argtab->ptr_out[i])),
                                     exprs);
            } else {
                exprs = TBmakeExprs (TCmakeIdCopyString (
                                       GetBaseTypeFromExpr (argtab->ptr_out[i])),
                                     TBmakeExprs (TBmakeNum (TYPES_DIM (
                                                    IDS_TYPE (argtab->ptr_out[i]))),
                                                  exprs));
            }
        } else {
            DBUG_ASSERT ((argtab->ptr_in[i] != NULL), "argtab is uncompressed!");
            DBUG_ASSERT ((NODE_TYPE (argtab->ptr_in[i]) == N_exprs),
                         "no N_exprs node found in argtab");
            if (NODE_TYPE (EXPRS_EXPR (argtab->ptr_in[i])) == N_id) {
                exprs
                  = TBmakeExprs (DUPdupIdNt (EXPRS_EXPR (argtab->ptr_in[i])), icm_args);

                if (!FUNDEF_ISCUDAGLOBALFUN (fundef)) {
                    exprs = TBmakeExprs (TCmakeIdCopyString (
                                           GetBaseTypeFromExpr (argtab->ptr_in[i])),
                                         exprs);
                } else {
                    exprs = TBmakeExprs (TCmakeIdCopyString (
                                           GetBaseTypeFromExpr (argtab->ptr_in[i])),
                                         TBmakeExprs (TBmakeNum (TYPES_DIM (ID_TYPE (
                                                        EXPRS_EXPR (argtab->ptr_in[i])))),
                                                      exprs));
                }
            } else if (NODE_TYPE (EXPRS_EXPR (argtab->ptr_in[i])) == N_globobj) {
                exprs
                  = TBmakeExprs (DUPdoDupNode (EXPRS_EXPR (argtab->ptr_in[i])), icm_args);
                exprs = TBmakeExprs (TCmakeIdCopyString (
                                       GetBaseTypeFromExpr (argtab->ptr_in[i])),
                                     exprs);
#ifndef DBUG_OFF
            } else {
                DBUG_ASSERT (0, "arguments of N_ap should be either N_id or N_globobj!");
#endif
            }
        }
        icm_args = TBmakeExprs (TCmakeIdCopyString (global.argtag_string[argtab->tag[i]]),
                                exprs);
    }

    icm_args = TBmakeExprs (TBmakeNum (argtab->size - 1), icm_args);

    /* return value */
    DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab inconsistent!");

    if (FUNDEF_ISSPMDFUN (fundef)) {
        ret_node
          = TCmakeAssignIcm2 ("MT_SPMD_FUN_AP", TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                              icm_args, assigns);
    } else if (FUNDEF_ISCUDAGLOBALFUN (fundef)) {
        ret_node
          = TCmakeAssignIcm2 ("CUDA_FUN_AP", TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                              icm_args, assigns);
    } else {
        if (argtab->ptr_out[0] == NULL) {
            icm_args = TBmakeExprs (TCmakeIdCopyString (NULL), icm_args);
        } else {
            icm_args = TBmakeExprs (DUPdupIdsId (argtab->ptr_out[0]), icm_args);
        }
        if (FUNDEF_ISTHREADFUN (fundef)) {
            ret_node = TCmakeAssignIcm2 ("MUTC_THREAD_AP",
                                         TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                                         icm_args, assigns);
        } else {
            ret_node
              = TCmakeAssignIcm2 ("ND_FUN_AP", TCmakeIdCopyString (FUNDEF_NAME (fundef)),
                                  icm_args, assigns);
        }
    }

    DBUG_RETURN (ret_node);
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
    int count;

    DBUG_ENTER ("MakeIcm_PROP_OBJ_OUT");

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

    icm_args = TBmakeExprs (TBmakeNum (count), icm_args);

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
    int count;

    DBUG_ENTER ("MakeIcm_PROP_OBJ_IN");

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

    icm_args = TBmakeExprs (TBmakeNum (count), icm_args);

    ret_node = TCmakeAssignIcm1 ("ND_PRF_PROP_OBJ_IN", icm_args, assigns);

    DBUG_RETURN (ret_node);
}

#ifndef DBUG_OFF

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
    int ids_idx, arg_idx;
    bool ok = TRUE;

    DBUG_ENTER ("CheckAp");

    DBUG_ASSERT ((NODE_TYPE (ap) == N_ap), "no N_ap node found!");

    argtab = AP_ARGTAB (ap);
    DBUG_ASSERT ((argtab != NULL), "no argtab found!");

    DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab inconsistent");
    for (arg_idx = 1; arg_idx < argtab->size; arg_idx++) {
        arg = argtab->ptr_in[arg_idx];
        if (arg != NULL) {
            DBUG_ASSERT ((NODE_TYPE (arg) == N_exprs),
                         "no N_exprs node found in argtab!");
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

#endif

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

    DBUG_ENTER ("RhsId");

    let_ids = INFO_LASTIDS (arg_info);

    /*
     * 'arg_node' and 'let_ids' are both non-unique or both unique
     */
    if (!STReq (IDS_NAME (let_ids), ID_NAME (arg_node))) {
        ret_node
          = TCmakeAssignIcm2 ("ND_ASSIGN",
                              MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE,
                                            TRUE, FALSE,
                                            MakeTypeArgs (ID_NAME (arg_node),
                                                          ID_TYPE (arg_node), FALSE, TRUE,
                                                          FALSE, NULL)),
                              TCmakeIdCopyString (
                                GenericFun (GF_copy, ID_TYPE (arg_node))),
                              ret_node);
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

    DBUG_ENTER ("Compile");

    info = MakeInfo ();

    INFO_FOLDLUT (info) = LUTgenerateLut ();

    TRAVpush (TR_comp);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    INFO_FOLDLUT (info) = LUTremoveLut (INFO_FOLDLUT (info));

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
    DBUG_ENTER ("COMPmodule");

    INFO_MODUL (arg_info) = arg_node;

    if (global.mtmode != MT_none) {
        INFO_SPMDFRAME (arg_info)
          = TBmakeAssign (TCmakeIcm0 ("MT_SPMD_FRAME_END"), INFO_SPMDFRAME (arg_info));
        INFO_SPMDBARRIER (arg_info) = TBmakeAssign (TCmakeIcm0 ("MT_SPMD_BARRIER_END"),
                                                    INFO_SPMDBARRIER (arg_info));
    }

    if (MODULE_OBJS (arg_node) != NULL) {
        MODULE_OBJS (arg_node) = TRAVdo (MODULE_OBJS (arg_node), arg_info);
    }

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    MODULE_THREADFUNS (arg_node) = TRAVopt (MODULE_THREADFUNS (arg_node), arg_info);

    if (MODULE_TYPES (arg_node) != NULL) {
        MODULE_TYPES (arg_node) = TRAVdo (MODULE_TYPES (arg_node), arg_info);
    }

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
    node *icm = NULL;

    DBUG_ENTER ("COMPtypedef");

    DBUG_PRINT ("COMP", ("compiling typedef '%s'", TYPEDEF_NAME (arg_node)));

    icm
      = TCmakeIcm1 ("ND_TYPEDEF", MakeTypeArgs (TYPEDEF_NAME (arg_node),
                                                TYtype2OldType (TYPEDEF_NTYPE (arg_node)),
                                                TRUE, FALSE, FALSE, NULL));

    TYPEDEF_ICM (arg_node) = icm;

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);
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

    DBUG_ENTER ("COMPobjdef");

    if (!OBJDEF_ISLOCAL (arg_node)) {
        icm = TCmakeIcm1 ("ND_OBJDEF_EXTERN",
                          MakeTypeArgs (OBJDEF_NAME (arg_node),
                                        TYtype2OldType (OBJDEF_TYPE (arg_node)), TRUE,
                                        TRUE, FALSE, NULL));
    } else {
        icm = TCmakeIcm1 ("ND_OBJDEF",
                          MakeTypeArgs (OBJDEF_NAME (arg_node),
                                        TYtype2OldType (OBJDEF_TYPE (arg_node)), TRUE,
                                        TRUE, TRUE, NULL));
    }
    OBJDEF_ICM (arg_node) = icm;

    OBJDEF_NT_TAG (arg_node)
      = NTUcreateNtTagFromNType (OBJDEF_NAME (arg_node), OBJDEF_TYPE (arg_node));

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = TRAVdo (OBJDEF_NEXT (arg_node), arg_info);
    }

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
    int i;
    node *assigns = NULL;

    DBUG_ENTER ("COMPFundefArgs");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "no N_fundef node found!");

    argtab = FUNDEF_ARGTAB (fundef);
    DBUG_ASSERT ((argtab != NULL), "no argtab found!");

    /*
     * Additional icms for the function body are generated regardless of the
     * existence of such a block, but COMPFundef() inserts them only if a
     * block already exists.
     */

    if (!FUNDEF_ISEXTERN (fundef)) {

        DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab inconsistent");
        for (i = 1; i < argtab->size; i++) {
            arg = argtab->ptr_in[i];
            if (arg != NULL) {
                DBUG_ASSERT ((NODE_TYPE (arg) == N_arg),
                             "no N_arg node found in argtab!");

                /*
                 * put "ND_DECL__MIRROR_PARAM" ICMs at beginning of function block
                 *   AND IN FRONT OF THE DECLARATION ICMs!!!
                 */
                assigns = TCmakeAssignIcm1 ("ND_DECL__MIRROR_PARAM",
                                            MakeTypeArgs (ARG_NAME (arg), ARG_TYPE (arg),
                                                          FALSE, TRUE, TRUE, NULL),
                                            assigns);

                /*
                 * put "ND_DECL_PARAM_inout" ICM at beginning of function block
                 *   AND IN FRONT OF THE DECLARATION ICMs!!!
                 */
                if (argtab->tag[i] == ATG_inout) {
                    assigns
                      = TCmakeAssignIcm1 ("ND_DECL_PARAM_inout",
                                          MakeTypeArgs (ARG_NAME (arg), ARG_TYPE (arg),
                                                        TRUE, FALSE, FALSE, NULL),
                                          assigns);
                }
            }
        }
    }

    DBUG_RETURN (assigns);
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

    DBUG_ENTER ("COMPfundef");

    DBUG_PRINT ("COMP", ("compiling %s", FUNDEF_NAME (arg_node)));

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
             * Traverse body
             */
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            DBUG_ASSERT ((INFO_POSTFUN (arg_info) == NULL),
                         "left-over postfun icms found!");
            /*
             * Store collected scheduler information.
             */
            BLOCK_SCHEDULER_INIT (FUNDEF_BODY (arg_node)) = INFO_SCHEDULERINIT (arg_info);

            if (INFO_SCHEDULERID (arg_info) > global.max_schedulers) {
                global.max_schedulers = INFO_SCHEDULERID (arg_info);
            }
        }

        /********** end: traverse body **********/

        /*
         * traverse arguments
         */
        if ((FUNDEF_ARGS (arg_node) != NULL) && (FUNDEF_BODY (arg_node) != NULL)
            && !FUNDEF_ISCUDAGLOBALFUN (arg_node)) {
            assigns = COMPFundefArgs (arg_node, arg_info);

            /* new first assignment of body */
            BLOCK_INSTR (FUNDEF_BODY (arg_node))
              = TCappendAssign (assigns, BLOCK_INSTR (FUNDEF_BODY (arg_node)));
        }

        FUNDEF_ICMDEFBEGIN (arg_node) = MakeFunctionSignature (arg_node, FALSE);
        FUNDEF_ICMDECL (arg_node) = MakeFunctionSignature (arg_node, TRUE);

        FUNDEF_ICMDEFEND (arg_node) = TBmakeIcm ("SAC_ND_FUN_DEF_END", NULL);

        /*
         * traverse next fundef
         */
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }

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
         * pop 'arg_info'
         */
        INFO_FUNDEF (arg_info) = old_fundef;
    } else {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
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
    DBUG_ENTER ("COMPvardec");

    if (AVIS_ISTHREADINDEX (VARDEC_AVIS (arg_node))) {
        VARDEC_ICM (arg_node) = TCmakeIcm1 ("SAC_MUTC_DECL_INDEX",
                                            TCmakeIdCopyString (VARDEC_NAME (arg_node)));
    } else {
        VARDEC_ICM (arg_node)
          = TCmakeIcm1 ("ND_DECL",
                        MakeTypeArgs (VARDEC_NAME (arg_node), VARDEC_TYPE (arg_node),
                                      TRUE, TRUE, TRUE, NULL));
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

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

    DBUG_ENTER ("COMPblock");

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

        DBUG_ASSERT ((BLOCK_INSTR (arg_node) != NULL),
                     "first instruction of block is NULL"
                     " (should be a N_empty node)");
        assign = BLOCK_INSTR (arg_node);

        BLOCK_INSTR (arg_node)
          = TCmakeAssignIcm1 ("CS_START", TCmakeIdCopyString (cs_tag),
                              BLOCK_INSTR (arg_node));

        while ((ASSIGN_NEXT (assign) != NULL)
               && (NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (assign))) != N_return)) {
            assign = ASSIGN_NEXT (assign);
        }

        ASSIGN_NEXT (assign) = TCmakeAssignIcm1 ("CS_STOP", TCmakeIdCopyString (cs_tag),
                                                 ASSIGN_NEXT (assign));
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPassign( node *arg_node, info *arg_info)
 *
 * @brief  Compiles a N_assign node.
 *         Note, that the traversal of ASSIGN_INSTR( arg_node) may return a
 *         N_assign chain instead of an expression.
 *
 ******************************************************************************/

node *
COMPassign (node *arg_node, info *arg_info)
{
    node *instr, *last, *next;

    DBUG_ENTER ("COMPassign");

    INFO_ASSIGN (arg_info) = arg_node;
    instr = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    next = ASSIGN_NEXT (arg_node);

    if (NODE_TYPE (instr) == N_assign) {
        /*
         * a N_assign chain was returned.
         *  -> insert N_assign chain at the current position into the tree.
         */

        /* insert head of 'instr' into AST */
        ASSIGN_INSTR (arg_node) = ASSIGN_INSTR (instr);

        /* insert tail of 'instr' into AST (last element) */
        last = instr;
        while (ASSIGN_NEXT (last) != NULL) {
            last = ASSIGN_NEXT (last);
        }
        ASSIGN_NEXT (last) = ASSIGN_NEXT (arg_node);

        /* free head of 'instr' */
        ASSIGN_INSTR (instr) = NULL;
        instr = FREEdoFreeNode (instr);

        /* insert tail of 'instr' into AST (first element) */
        ASSIGN_NEXT (arg_node) = instr;
    } else {
        /* insert 'instr' into AST */
        ASSIGN_INSTR (arg_node) = instr;
    }

    if (next != NULL) {
        next = TRAVdo (next, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPNormalFunReturn( node *arg_node, info *arg_info)
 *
 * @brief  Generates ICMs for N_return-node found in body of a
 *         non-SPMD-function.
 *
 ******************************************************************************/

static node *
COMPNormalFunReturn (node *arg_node, info *arg_info)
{
    node *fundef;
    argtab_t *argtab;
    node *ret_exprs;
    node *funargs;
    node *newid;
    node *new_args;
    int i;
    int ret_cnt;
    node *cret_node = NULL;
    node *icm_args = NULL;
    node *last_arg = NULL;

    DBUG_ENTER ("COMPNormalFunReturn");

    fundef = INFO_FUNDEF (arg_info);
    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no fundef node found!");

    argtab = FUNDEF_ARGTAB (fundef);
    DBUG_ASSERT ((argtab != NULL), "no argtab found!");

    /* return value */
    DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab inconsistent!");
    if (RETURN_CRET (arg_node) != NULL) {
        DBUG_ASSERT ((NODE_TYPE (RETURN_CRET (arg_node)) == N_exprs),
                     "no N_exprs node found in RETURN_CRET");
        DBUG_ASSERT ((argtab->ptr_out[0] != NULL), "argtab inconsistent!");
        cret_node = DUPdoDupTree (EXPRS_EXPR (RETURN_CRET (arg_node)));
    } else {
        DBUG_ASSERT ((argtab->ptr_out[0] == NULL), "argtab or RETURN_CRET inconsistent!");
    }

    /* regular arguments */
    ret_exprs = RETURN_EXPRS (arg_node);
    ret_cnt = 0;
    for (i = 1; i < argtab->size; i++) {
        if (argtab->ptr_out[i] != NULL) {
            DBUG_ASSERT ((ret_exprs != NULL), "not enough return values found!");
            if (RETURN_CRET (arg_node) == ret_exprs) {
                ret_exprs = EXPRS_NEXT (ret_exprs);
                DBUG_ASSERT ((ret_exprs != NULL), "not enough return values found!");
            }
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (ret_exprs)) == N_id),
                         "argument of return-statement must be a N_id node!");

            new_args
              = TBmakeExprs (TCmakeIdCopyString (global.argtag_string[argtab->tag[i]]),
                             TBmakeExprs (MakeArgNode (i,
                                                       ID_TYPE (EXPRS_EXPR (ret_exprs)),
                                                       FALSE),
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
            DBUG_ASSERT ((argtab->ptr_in[i] != NULL), "argtab is uncompressed!");
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

    /*
     * replace N_return node by a new N_icm node
     */

    DBUG_ASSERT ((FUNDEF_RETURN (fundef) == arg_node),
                 "FUNDEF_RETURN not found via 'arg_info'!");

    arg_node = FREEdoFreeTree (arg_node);
    if (cret_node == NULL) {
        cret_node = TCmakeIdCopyString (NULL);
    }
    arg_node = TCmakeIcm3 ("ND_FUN_RET", cret_node, TBmakeNum (ret_cnt), icm_args);

    FUNDEF_RETURN (fundef) = arg_node;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPSpmdFunReturn( node *arg_node, info *arg_info)
 *
 * @brief  Generates ICMs for N_return-node found in body of a SPMD-function.
 *
 ******************************************************************************/

static node *
COMPSpmdFunReturn (node *arg_node, info *arg_info)
{
    node *fundef;
    argtab_t *argtab;
    node *ret_exprs;
    node *new_args;
    int ret_cnt;
    int i;
    node *icm_args = NULL;
    node *last_arg = NULL;

    DBUG_ENTER ("COMPSpmdFunReturn");

    fundef = INFO_FUNDEF (arg_info);
    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no fundef node found!");

    argtab = FUNDEF_ARGTAB (fundef);
    DBUG_ASSERT ((argtab != NULL), "no argtab found!");

    /* regular arguments */
    ret_exprs = RETURN_EXPRS (arg_node);
    ret_cnt = 0;
    for (i = 0; i < argtab->size; i++) {
        if (argtab->ptr_out[i] != NULL) {
            char *foldfunname;

            DBUG_ASSERT ((ret_exprs != NULL), "not enough return values found!");
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (ret_exprs)) == N_id),
                         "no N_id node found!");

            foldfunname = LUTsearchInLutSs (INFO_FOLDLUT (arg_info),
                                            ID_NAME (EXPRS_EXPR (ret_exprs)));

            foldfunname
              = (foldfunname == ID_NAME (EXPRS_EXPR (ret_exprs)) ? "NONE" : foldfunname);

            new_args
              = TBmakeExprs (TCmakeIdCopyString (global.argtag_string[argtab->tag[i]]),
                             TBmakeExprs (TCmakeIdCopyString (foldfunname),
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
            DBUG_ASSERT (((i == 0) || (argtab->ptr_in[i] != NULL)),
                         "argtab is uncompressed!");
        }
    }

    arg_node = TCmakeIcm3 ("MT_SPMD_FUN_RET",
                           TCmakeIdCopyString (FUNDEF_NAME (INFO_FUNDEF (arg_info))),
                           TBmakeNum (ret_cnt), icm_args);

    FUNDEF_RETURN (fundef) = arg_node;

    DBUG_RETURN (arg_node);
}

static node *
COMPCudaFunReturn (node *arg_node, info *arg_info)
{
    node *fundef;

    DBUG_ENTER ("COMPCudaFunReturn");

    fundef = INFO_FUNDEF (arg_info);
    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no fundef node found!");

    arg_node = TCmakeIcm0 ("SAC_NOOP");

    FUNDEF_RETURN (fundef) = arg_node;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPreturn( node *arg_node, info *arg_info)
 *
 * @brief  Generates ICMs for N_return of a function (ND or MT).
 *
 ******************************************************************************/

node *
COMPreturn (node *arg_node, info *arg_info)
{
    node *fundef;

    DBUG_ENTER ("COMPreturn");

    fundef = INFO_FUNDEF (arg_info);

    if (FUNDEF_ISSPMDFUN (fundef)) {
        arg_node = COMPSpmdFunReturn (arg_node, arg_info);
    } else if (FUNDEF_ISCUDAGLOBALFUN (fundef)) {
        arg_node = COMPCudaFunReturn (arg_node, arg_info);
    } else {
        arg_node = COMPNormalFunReturn (arg_node, arg_info);
    }

    if (INFO_POSTFUN (arg_info) != NULL) {
        /*
         * Fuse in POSTFUN icms. COMPassign takes care of the rest
         * when finding an N_assign chain.
         */
        arg_node
          = TCappendAssign (INFO_POSTFUN (arg_info), TBmakeAssign (arg_node, NULL));
        INFO_POSTFUN (arg_info) = NULL;
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

    DBUG_ENTER ("COMPlet");

    INFO_LASTIDS (arg_info) = LET_IDS (arg_node);

    expr = TRAVdo (LET_EXPR (arg_node), arg_info);

    /*
     * 'expr' is a RHS expression or a N_assign chain !!
     */

    if (NODE_TYPE (expr) == N_assign) {
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
    int i;
    node *ret_node = NULL;

    DBUG_ENTER ("COMPApIds");

    DBUG_ASSERT ((NODE_TYPE (ap) == N_ap), "no N_ap node found!");

    argtab = AP_ARGTAB (ap);
    DBUG_ASSERT ((argtab != NULL), "no argtab found!");

    for (i = argtab->size - 1; i >= 0; i--) {
        if (argtab->ptr_out[i] != NULL) {
            let_ids = argtab->ptr_out[i];
            tag = argtab->tag[i];
            DBUG_ASSERT (((global.argtag_is_out[tag]) || (global.argtag_is_inout[tag])),
                         "illegal tag found!");

            if (global.argtag_is_out[tag]) { /* it is an out- (but no inout-) parameter */
                if (!global.argtag_has_rc[tag]) {
                    /* function does no refcounting */
                    ret_node = MakeSetRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids), 1,
                                             ret_node);
                }
            }

            ret_node
              = TCmakeAssignIcm1 ("ND_REFRESH__MIRROR",
                                  MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                                FALSE, TRUE, FALSE, NULL),
                                  ret_node);

            if (global.argtag_is_out[tag]) { /* it is an out- (but no inout-) parameter */
                if (!global.argtag_has_shp[tag]) {
                    /* function sets no shape information */
                    shape_class_t sc = NTUgetShapeClassFromTypes (
                      IDS_TYPE (((node *)argtab->ptr_out[i])));
                    DBUG_ASSERT ((sc != C_unknowns), "illegal data class found!");
                    if ((sc == C_akd) || (sc == C_aud)) {
                        CTIabortLine (global.linenum,
                                      "Return value with undefined shape/dimension found."
                                      " Non-AKS return values in external functions are "
                                      "only allowed when the corresponding refcounting"
                                      " pragma is set.");
                    }
                }

                if (!global.argtag_has_desc[tag]) {
                    /* function uses no descriptor at all */
                    ret_node
                      = MakeAllocDescIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids), 1,
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
    node *arg;
    int i;
    node *ret_node = NULL;

    DBUG_ENTER ("COMPApArgs");

    argtab = AP_ARGTAB (ap);
    DBUG_ASSERT ((argtab != NULL), "no argtab found!");

    for (i = argtab->size - 1; i >= 0; i--) {
        if (argtab->ptr_in[i] != NULL) {
            DBUG_ASSERT ((NODE_TYPE (argtab->ptr_in[i]) == N_exprs),
                         "no N_exprs node found in argtab");
            arg = EXPRS_EXPR (argtab->ptr_in[i]);
            tag = argtab->tag[i];
            DBUG_ASSERT ((global.argtag_is_in[tag] || global.argtag_is_inout[tag]),
                         "illegal tag found!");
        }
    }

    DBUG_RETURN (ret_node);
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
    node *let_ids;
    node *fundef;
    node *ret_node;
    node *assigns1, *assigns2;

    DBUG_ENTER ("COMPap");

    let_ids = INFO_LASTIDS (arg_info);
    fundef = AP_FUNDEF (arg_node);

    DBUG_ASSERT ((CheckAp (arg_node, arg_info)),
                 "application of a user-defined function without own"
                 " refcounting: refcounted argument occurs also on LHS!");

    /*
     * traverse ids on LHS of application
     */
    assigns1 = COMPApIds (arg_node, arg_info);

    /*
     * traverse arguments of application
     */
    assigns2 = COMPApArgs (arg_node, arg_info);

    ret_node = TCappendAssign (assigns1, assigns2);

    /* insert ND_FUN_AP icm at head of assignment chain */
    ret_node = MakeIcm_FUN_AP (arg_node, fundef, ret_node);

    DBUG_RETURN (ret_node);
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

    DBUG_ENTER ("COMPid");

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
 * @fn  types *GetType( node *arg_node)
 *
 * @brief  Return the type of an id or ids.
 *
 *****************************************************************************/

/*
 * This code is very bad style!!!!!
 * Because of the way types are handled in this phases it is needed :(
 * Hopfully with the removal of old types this can also be removed.
 */
static types *
GetType (node *arg_node)
{
    types *type = NULL;
    node *decl = NULL;

    DBUG_ENTER ("GetType");

    if (NODE_TYPE (arg_node) == N_ids) {
        decl = IDS_DECL (arg_node);
    } else if (NODE_TYPE (arg_node) == N_id) {
        decl = ID_DECL (arg_node);
    } else {
        DBUG_ASSERT (0 == 1, "Unexpected node type\n");
    }

    if (NODE_TYPE (decl) == N_vardec) {
        type = VARDEC_TYPE (decl);
    } else if (NODE_TYPE (decl) == N_arg) {
        type = ARG_TYPE (decl);
    } else {
        DBUG_ASSERT (0 == 1, "Unexpected node type\n");
    }

    DBUG_RETURN (type);
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

    DBUG_ENTER ("MakeIcm_PRF_TYPE_CONV__AKD");

    ret_node
      = TCmakeAssignIcm3 ("SAC_ND_PRF_TYPE_CONV__AKD_END", TBmakeStr (STRcpy (error)),
                          TCmakeIdCopyStringNt (AVIS_NAME (IDS_AVIS (let_ids)),
                                                GetType (let_ids)),
                          TCmakeIdCopyStringNt (AVIS_NAME (ID_AVIS (id)), GetType (id)),
                          ret_node);

    for (i = TCgetShapeDim (GetType (let_ids)) - 1; i >= 0; i--) {
        ret_node = TCmakeAssignIcm3 ("SAC_ND_PRF_TYPE_CONV__AKD_SHAPE", TBmakeNum (i),
                                     TCmakeIdCopyStringNt (AVIS_NAME (IDS_AVIS (let_ids)),
                                                           GetType (let_ids)),
                                     TCmakeIdCopyStringNt (AVIS_NAME (ID_AVIS (id)),
                                                           GetType (id)),
                                     ret_node);
    }

    ret_node
      = TCmakeAssignIcm3 ("SAC_ND_PRF_TYPE_CONV__AKD_START", TBmakeStr (STRcpy (error)),
                          TCmakeIdCopyStringNt (AVIS_NAME (IDS_AVIS (let_ids)),
                                                GetType (let_ids)),
                          TCmakeIdCopyStringNt (AVIS_NAME (ID_AVIS (id)), GetType (id)),
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

    DBUG_ENTER ("MakeIcm_PRF_TYPE_CONV__AKS");

    ret_node
      = TCmakeAssignIcm3 ("SAC_ND_PRF_TYPE_CONV__AKS_END", TBmakeStr (STRcpy (error)),
                          TCmakeIdCopyStringNt (AVIS_NAME (IDS_AVIS (let_ids)),
                                                GetType (let_ids)),
                          TCmakeIdCopyStringNt (AVIS_NAME (ID_AVIS (id)), GetType (id)),
                          ret_node);

    for (i = TCgetShapeDim (GetType (let_ids)) - 1; i >= 0; i--) {
        ret_node = TCmakeAssignIcm3 ("SAC_ND_PRF_TYPE_CONV__AKS_COND", TBmakeNum (i),
                                     TCmakeIdCopyStringNt (AVIS_NAME (IDS_AVIS (let_ids)),
                                                           GetType (let_ids)),
                                     TCmakeIdCopyStringNt (AVIS_NAME (ID_AVIS (id)),
                                                           GetType (id)),
                                     ret_node);
    }

    ret_node
      = TCmakeAssignIcm3 ("SAC_ND_PRF_TYPE_CONV__AKS_START", TBmakeStr (STRcpy (error)),
                          TCmakeIdCopyStringNt (AVIS_NAME (IDS_AVIS (let_ids)),
                                                GetType (let_ids)),
                          TCmakeIdCopyStringNt (AVIS_NAME (ID_AVIS (id)), GetType (id)),
                          ret_node);

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
    int error_len = 0;
    node *id = NULL;

    DBUG_ENTER ("COMPprfTypeConv");

    let_ids = INFO_LASTIDS (arg_info);
    id = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node)));

    lhs_type_string = CVtype2String (GetType (let_ids), 0, FALSE);
    rhs_type_string = CVtype2String (GetType (id), 0, FALSE);

    error_len = STRlen (fmt) - (2 * 6) + STRlen (NODE_FILE (arg_node)) + STRsizeInt ()
                + STRlen (rhs_type_string) + STRlen (AVIS_NAME (ID_AVIS (id)))
                + STRlen (lhs_type_string) + STRlen (AVIS_NAME (IDS_AVIS (let_ids)));
    error = (char *)MEMmalloc (sizeof (char) * error_len);

    sprintf (error, fmt, NODE_FILE (arg_node), NODE_LINE (arg_node), rhs_type_string,
             AVIS_NAME (ID_AVIS (id)), lhs_type_string, AVIS_NAME (IDS_AVIS (let_ids)));

    if ((SCALAR != TCgetShapeDim (GetType (let_ids)))
        && (KNOWN_SHAPE (TCgetShapeDim (GetType (let_ids)))
            && (global.min_array_rep <= MAR_scl_aks))) {
        /* ASK needs the mirror */
        ret_node = MakeIcm_PRF_TYPE_CONV_AKS (error, let_ids, id);

    } else if (SCALAR != TCgetShapeDim (GetType (let_ids))
               && KNOWN_DIMENSION (TCgetShapeDim (GetType (let_ids)))
               && (global.min_array_rep <= MAR_scl_akd)) {
        /* ASK needs the mirror */
        ret_node = MakeIcm_PRF_TYPE_CONV_AKD (error, let_ids, id);

    } else {
        ret_node = TCmakeAssignIcm3 ("SAC_ND_PRF_TYPE_CONV", TBmakeStr (STRcpy (error)),
                                     TCmakeIdCopyStringNt (AVIS_NAME (IDS_AVIS (let_ids)),
                                                           GetType (let_ids)),
                                     TCmakeIdCopyStringNt (AVIS_NAME (ID_AVIS (id)),
                                                           GetType (id)),
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
    types *lhs_type, *rhs_type;
    node *ret_node, *arg;

    DBUG_ENTER ("COMPprfFromUnq");

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

    lhs_type = IDS_TYPE (let_ids);
    DBUG_ASSERT ((!TCisUnique (lhs_type)), "from_unq() with unique LHS found!");
    rhs_type = ID_TYPE (arg);

    if (!TCisUnique (rhs_type)) {
        /*
         * non-unique type
         *   -> ignore from_unq() in order to get a simpler ICM code
         */
        ret_node = COMPid (arg, arg_info);
    } else {
        ret_node
          = TCmakeAssignIcm1 ("ND_ASSIGN",
                              MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE,
                                            TRUE, FALSE,
                                            MakeTypeArgs (ID_NAME (arg), ID_TYPE (arg),
                                                          FALSE, TRUE, FALSE, NULL)),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_TYPE (arg))));
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
    types *lhs_type, *rhs_type;
    node *icm_args;
    node *ret_node, *arg;

    DBUG_ENTER ("COMPprfToUnq");

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

    lhs_type = IDS_TYPE (let_ids);
    rhs_type = ID_TYPE (arg);
    DBUG_ASSERT ((!TCisUnique (rhs_type)), "to_unq() with unique RHS found!");

    icm_args
      = MakeTypeArgs (IDS_NAME (let_ids), lhs_type, FALSE, TRUE, FALSE,
                      MakeTypeArgs (ID_NAME (arg), rhs_type, FALSE, TRUE, FALSE, NULL));

    /*
     * No RC manipulation requires as MAKE_UNIQUE always yields rc == 1
     */
    ret_node
      = TCmakeAssignIcm2 ("ND_MAKE_UNIQUE", icm_args,
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

    DBUG_ENTER ("COMPscalar");

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm2 ("ND_CREATE__SCALAR__DATA", DUPdupIdsIdNt (let_ids),
                                 DUPdoDupNode (arg_node), NULL);

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

    DBUG_ENTER ("COMPnum");

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

    DBUG_ENTER ("COMPchar");

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

    DBUG_ENTER ("COMPbool");

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

    DBUG_ENTER ("COMPfloat");

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

    DBUG_ENTER ("COMPdouble");

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

    DBUG_ENTER ("COMParray");

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
                copyfun = GenericFun (GF_copy, ID_TYPE (val0));
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
                              MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE,
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
    DBUG_ENTER ("COMPprfPropObjIn");

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
    DBUG_ENTER ("COMPprfPropObjOut");

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
    types *type;
    node *ret_node = NULL;
    int num;

    DBUG_ENTER ("COMPprfIncRC");

    switch (NODE_TYPE (PRF_ARG1 (arg_node))) {
    case N_id:
        name = ID_NAME (PRF_ARG1 (arg_node));
        type = ID_TYPE (PRF_ARG1 (arg_node));
        num = NUM_VAL (PRF_ARG2 (arg_node));

        ret_node = MakeIncRcIcm (name, type, num, NULL);
        break;

    case N_globobj:
        name = OBJDEF_NAME (GLOBOBJ_OBJDEF (PRF_ARG1 (arg_node)));
        type = TYtype2OldType (OBJDEF_TYPE (GLOBOBJ_OBJDEF (PRF_ARG1 (arg_node))));
        num = NUM_VAL (PRF_ARG2 (arg_node));

        ret_node = MakeIncRcIcm (name, type, num, NULL);

        type = FREEfreeAllTypes (type);
        break;
    default:
        DBUG_ASSERT (FALSE, "1. Argument of inc_rc has wrong node type.");
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
    types *type;
    node *ret_node = NULL;
    int num;

    DBUG_ENTER ("COMPprfDecRC");

    switch (NODE_TYPE (PRF_ARG1 (arg_node))) {
    case N_id:
        name = ID_NAME (PRF_ARG1 (arg_node));
        type = ID_TYPE (PRF_ARG1 (arg_node));
        num = NUM_VAL (PRF_ARG2 (arg_node));

        ret_node = MakeDecRcIcm (name, type, num, NULL);
        break;

    case N_globobj:
        name = OBJDEF_NAME (GLOBOBJ_OBJDEF (PRF_ARG1 (arg_node)));
        type = TYtype2OldType (OBJDEF_TYPE (GLOBOBJ_OBJDEF (PRF_ARG1 (arg_node))));
        num = NUM_VAL (PRF_ARG2 (arg_node));

        ret_node = MakeDecRcIcm (name, type, num, NULL);

        type = FREEfreeAllTypes (type);
        break;
    default:
        DBUG_ASSERT (FALSE, "1. Argument of dec_rc has wrong node type.");
    }

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

    DBUG_ENTER ("COMPprfAlloc");

    let_ids = INFO_LASTIDS (arg_info);

    rc = NUM_VAL (PRF_ARG1 (arg_node));
    get_dim = MakeGetDimIcm (PRF_ARG2 (arg_node));
    set_shape = MakeSetShapeIcm (PRF_ARG3 (arg_node), let_ids);

    ret_node = MakeAllocIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids), rc, get_dim,
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

    DBUG_ENTER ("COMPprfAllocOrReuse");

    let_ids = INFO_LASTIDS (arg_info);

    rc = NUM_VAL (PRF_ARG1 (arg_node));
    get_dim = MakeGetDimIcm (PRF_ARG2 (arg_node));
    set_shape = MakeSetShapeIcm (PRF_ARG3 (arg_node), let_ids);

    ret_node = MakeAllocIcm_IncRc (IDS_NAME (let_ids), IDS_TYPE (let_ids), rc, get_dim,
                                   set_shape, NULL, NULL);

    cand = EXPRS_EXPRS4 (PRF_ARGS (arg_node));
    while (cand != NULL) {
        ret_node = MakeCheckReuseIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                      EXPRS_EXPR (cand), ret_node);
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

    DBUG_ENTER ("COMPprfFree");

    DBUG_ASSERT ((NODE_TYPE (PRF_ARG1 (arg_node)) != N_globobj),
                 "Application of F_free to N_globobj detected!");

    ret_node = MakeSetRcIcm (ID_NAME (PRF_ARG1 (arg_node)), ID_TYPE (PRF_ARG1 (arg_node)),
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
    node *sub_get_dim;

    DBUG_ENTER ("COMPprfSuballoc");

    let_ids = INFO_LASTIDS (arg_info);
    mem_id = PRF_ARG1 (arg_node);
    sc = NTUgetShapeClassFromTypes (IDS_TYPE (let_ids));

    DBUG_ASSERT (sc != C_scl, "scalars cannot be suballocated\n");

    if (global.backend == BE_cuda) {
        ret_node
          = TCmakeAssignIcm5 ("CUDA_WL_SUBALLOC", DUPdupIdsIdNt (let_ids),
                              TBmakeNum (TCgetShapeDim (IDS_TYPE (let_ids))),
                              DUPdupIdNt (PRF_ARG1 (arg_node)),
                              TBmakeNum (TCgetShapeDim (ID_TYPE (PRF_ARG1 (arg_node)))),
                              DUPdupIdNt (PRF_ARG2 (arg_node)), NULL);
    } else {
        ret_node = TCmakeAssignIcm3 ("WL_SUBALLOC", DUPdupIdsIdNt (let_ids),
                                     DUPdupIdNt (PRF_ARG1 (arg_node)),
                                     DUPdupIdNt (PRF_ARG2 (arg_node)), NULL);
    }

    if (global.backend == BE_mutc) {
        DBUG_ASSERT ((PRF_ARG3 (arg_node) != NULL),
                     "suballoc lacking default information in mutc backend");
        /*
         * We need to allocate a descriptor and set the shape
         *
         * 1) compute dimensionality. If the range is not chunked,
         *    its the dimensionality of the memvar - 1, else its
         *    the dimensionality of the memvar. The third arg
         *    of the suballoc holds this information.
         */
        sub_get_dim = TCmakeIcm2 (prf_ccode_tab[F_sub_SxS],
                                  TCmakeIcm1 ("ND_A_DIM", DUPdupIdNt (mem_id)),
                                  DUPdoDupNode (PRF_ARG3 (arg_node)));

        /*
         * 2) annotate shape for suballoc, if information present and
         *    dynamic shape required.
         *    TODO MUTC: As we only support genarray for now, this
         *               information always has to be present!
         */
        if (TCgetShapeDim (IDS_TYPE (let_ids)) < 0) {
            DBUG_ASSERT ((PRF_ARG4 (arg_node) != NULL),
                         "missing shape information for suballoc");

            ret_node
              = TBmakeAssign (MakeSetShapeIcm (PRF_ARG4 (arg_node), let_ids), ret_node);
        }

#if 0
    /*
     * (CAJ) I do not think that points 3 & 4 are needed.
     *
     * If they are then they can not be implemented like this as they
     * always results in a call to malloc which is unacceptable in a
     * loop.  If these are needed then the descriptor should be put on
     * the stack.
     */

    /*
     * 3) produce the descriptor allocation
     */
    ret_node = MakeAllocDescIcm( IDS_NAME( let_ids),
                                 IDS_TYPE( let_ids),
                                 1,
                                 sub_get_dim,
                                 ret_node);

    /*
     * 4) add a corresponding descriptor free to the 
     *    postfun icm chain
     */
    INFO_POSTFUN( arg_info) = TCmakeAssignIcm1( "ND_FREE__DESC",
                                TCmakeIdCopyStringNt( IDS_NAME( let_ids),
                                                      IDS_TYPE( let_ids)),
                                INFO_POSTFUN( arg_info));
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
    DBUG_ENTER ("COMPprfWLAssign");

    if (NODE_TYPE (PRF_ARG3 (arg_node)) == N_id) {
        arg3 = DUPdupIdNt (PRF_ARG3 (arg_node));
    } else {
        arg3 = DUPdupIdNt (EXPRS_EXPR (ARRAY_AELEMS (PRF_ARG3 (arg_node))));
    }

    ret_node
      = TCmakeAssignIcm6 ("WL_ASSIGN",
                          MakeTypeArgs (ID_NAME (PRF_ARG1 (arg_node)),
                                        ID_TYPE (PRF_ARG1 (arg_node)), FALSE, TRUE, FALSE,
                                        NULL),
                          MakeTypeArgs (ID_NAME (PRF_ARG2 (arg_node)),
                                        ID_TYPE (PRF_ARG2 (arg_node)), FALSE, TRUE, FALSE,
                                        NULL),
                          arg3,
                          TBmakeExprs (MakeSizeArg (PRF_ARG3 (arg_node), TRUE), NULL),
                          DUPdupIdNt (PRF_ARG4 (arg_node)),
                          TCmakeIdCopyString (
                            GenericFun (GF_copy, ID_TYPE (PRF_ARG1 (arg_node)))),
                          NULL);

    DBUG_RETURN (ret_node);
}

/* return the nth arg */
static node *
nthArg (node *args, int n)
{
    node *nth_arg;
    int i = 0;

    DBUG_ENTER ("nthArg");

    nth_arg = args;

    while (i != n) {
        nth_arg = ARG_NEXT (nth_arg);
        i++;
    }

    DBUG_RETURN (nth_arg);
}

static node *
COMPprfCUDAWLAssign (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ("COMPprfCUDAWLAssign");

    ret_node = TCmakeAssignIcm3 ("CUDA_WL_ASSIGN",
                                 MakeTypeArgs (ID_NAME (PRF_ARG1 (arg_node)),
                                               ID_TYPE (PRF_ARG1 (arg_node)), FALSE, TRUE,
                                               FALSE, NULL),
                                 MakeTypeArgs (ID_NAME (PRF_ARG2 (arg_node)),
                                               ID_TYPE (PRF_ARG2 (arg_node)), FALSE, TRUE,
                                               FALSE, NULL),
                                 DUPdupIdNt (PRF_ARG3 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

static node *
COMPprfCUDAWLIdxs (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;
    int array_dim;

    DBUG_ENTER ("COMPprfCUDAWLIdxs");

    array_dim = NUM_VAL (PRF_ARG3 (arg_node));
    DBUG_ASSERT ((array_dim > 0), "Dimension of result CUDA array must be > 0");

    if (array_dim <= 2) {
        ret_node = TCmakeAssignIcm4 ("CUDA_WLIDXS",
                                     MakeTypeArgs (ID_NAME (PRF_ARG1 (arg_node)),
                                                   ID_TYPE (PRF_ARG1 (arg_node)), FALSE,
                                                   FALSE, FALSE, NULL),
                                     MakeTypeArgs (ID_NAME (PRF_ARG2 (arg_node)),
                                                   ID_TYPE (PRF_ARG2 (arg_node)), FALSE,
                                                   FALSE, FALSE, NULL),
                                     TBmakeNum (array_dim),
                                     DupExprs_NT_AddReadIcms (
                                       EXPRS_EXPRS4 (PRF_ARGS (arg_node))),
                                     NULL);
    } else {
        ret_node = TCmakeAssignIcm4 ("CUDA_WLIDXS",
                                     MakeTypeArgs (ID_NAME (PRF_ARG1 (arg_node)),
                                                   ID_TYPE (PRF_ARG1 (arg_node)), FALSE,
                                                   FALSE, FALSE, NULL),
                                     MakeTypeArgs (ID_NAME (PRF_ARG2 (arg_node)),
                                                   ID_TYPE (PRF_ARG2 (arg_node)), FALSE,
                                                   FALSE, FALSE, NULL),
                                     TBmakeNum (array_dim),
                                     DupExprs_NT_AddReadIcms (
                                       EXPRS_EXPRS4 (PRF_ARGS (arg_node))),
                                     NULL);
    }

    DBUG_RETURN (ret_node);
}

static node *
COMPprfCUDAWLIds (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;
    node *args;
    int array_dim, dim;

    DBUG_ENTER ("COMPprfCUDAWLIds");

    array_dim = NUM_VAL (PRF_ARG3 (arg_node));
    DBUG_ASSERT ((array_dim > 0), "Dimension of result CUDA array must be > 0");

    args = FUNDEF_ARGS (INFO_FUNDEF (arg_info));

    if (array_dim <= 2) {
        dim = NUM_VAL (PRF_ARG2 (arg_node));
        ret_node = TCmakeAssignIcm4 ("CUDA_WLIDS",
                                     MakeTypeArgs (ID_NAME (PRF_ARG1 (arg_node)),
                                                   ID_TYPE (PRF_ARG1 (arg_node)), FALSE,
                                                   TRUE, FALSE, NULL),
                                     TBmakeNum (array_dim), TBmakeNum (dim),
                                     TBmakeBool (
                                       FUNDEF_HASSTEPWIDTHARGS (INFO_FUNDEF (arg_info))),
                                     NULL);
    } else {
        dim = NUM_VAL (PRF_ARG2 (arg_node));
        ret_node = TCmakeAssignIcm4 ("CUDA_WLIDS",
                                     MakeTypeArgs (ID_NAME (PRF_ARG1 (arg_node)),
                                                   ID_TYPE (PRF_ARG1 (arg_node)), FALSE,
                                                   TRUE, FALSE, NULL),
                                     TBmakeNum (array_dim), TBmakeNum (dim),
                                     TBmakeBool (
                                       FUNDEF_HASSTEPWIDTHARGS (INFO_FUNDEF (arg_info))),
                                     NULL);
    }

    DBUG_RETURN (ret_node);
}

static node *
COMPprfCUDAGridBlock (node *arg_node, info *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ("COMPprfCUDAGridBlock");

    ret_node = TCmakeAssignIcm2 ("CUDA_GRID_BLOCK",
                                 TBmakeNum (TCcountExprs (PRF_ARGS (arg_node))),
                                 DupExprs_NT_AddReadIcms (PRF_ARGS (arg_node)), NULL);

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

    DBUG_ENTER ("COMPprfWLBreak");

    ret_node = TCmakeAssignIcm3 ("ND_ASSIGN__DATA", DUPdupIdNt (PRF_ARG2 (arg_node)),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 TCmakeIdCopyString (
                                   GenericFun (GF_copy, ID_TYPE (PRF_ARG1 (arg_node)))),
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

    DBUG_ENTER ("COMPprfCopy");

    let_ids = INFO_LASTIDS (arg_info);

    if (global.backend != BE_cuda) {
        ret_node
          = TCmakeAssignIcm3 ("ND_COPY__DATA", DUPdupIdsIdNt (let_ids),
                              DUPdupIdNt (PRF_ARG1 (arg_node)),
                              TCmakeIdCopyString (
                                GenericFun (GF_copy, ID_TYPE (PRF_ARG1 (arg_node)))),
                              NULL);
    } else {
        src_basetype = TCgetBasetype (ID_TYPE (PRF_ARG1 (arg_node)));
        dst_basetype = TCgetBasetype (IDS_TYPE (let_ids));

        if ((src_basetype == T_float_dev && dst_basetype == T_float_dev)
            || (src_basetype == T_int_dev && dst_basetype == T_int_dev)) {
            ret_node
              = TCmakeAssignIcm4 ("CUDA_COPY__ARRAY", DUPdupIdsIdNt (let_ids),
                                  DUPdupIdNt (PRF_ARG1 (arg_node)),
                                  MakeBasetypeArg (ID_TYPE (PRF_ARG1 (arg_node))),
                                  TCmakeIdCopyString (
                                    GenericFun (GF_copy, ID_TYPE (PRF_ARG1 (arg_node)))),
                                  NULL);
        } else {
            ret_node
              = TCmakeAssignIcm3 ("ND_COPY__DATA", DUPdupIdsIdNt (let_ids),
                                  DUPdupIdNt (PRF_ARG1 (arg_node)),
                                  TCmakeIdCopyString (
                                    GenericFun (GF_copy, ID_TYPE (PRF_ARG1 (arg_node)))),
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

    DBUG_ENTER ("COMPprfIsReused");

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

    DBUG_ENTER ("COMPprfDim");

    let_ids = INFO_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg) == N_id), "arg of F_dim_A is no N_id!");

    ret_node = TCmakeAssignIcm1 ("ND_PRF_DIM_A__DATA",
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                               FALSE, TRUE, FALSE,
                                               MakeTypeArgs (ID_NAME (arg), ID_TYPE (arg),
                                                             FALSE, TRUE, FALSE, NULL)),
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

    DBUG_ENTER ("COMPprfShape");

    let_ids = INFO_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg) == N_id), "arg of F_shape_A is no N_id!");

    ret_node = TCmakeAssignIcm1 ("ND_PRF_SHAPE_A__DATA",
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                               FALSE, TRUE, FALSE,
                                               MakeTypeArgs (ID_NAME (arg), ID_TYPE (arg),
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

    DBUG_ENTER ("COMPprfSize");

    let_ids = INFO_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg) == N_id), "arg of F_size_A is no N_id!");

    ret_node = TCmakeAssignIcm1 ("ND_PRF_SIZE_A__DATA",
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                               FALSE, TRUE, FALSE,
                                               MakeTypeArgs (ID_NAME (arg), ID_TYPE (arg),
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

    DBUG_ENTER ("COMPprfReshape");

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
    copyfun = GenericFun (GF_copy, IDS_TYPE (let_ids));
#endif

    ret_node
      = MakeSetRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids), rc,
                      TBmakeAssign (set_shape_icm,
                                    TCmakeAssignIcm3 ("ND_ASSIGN__DATA",
                                                      DUPdupIdsIdNt (let_ids),
                                                      DUPdupIdNt (PRF_ARG4 (arg_node)),
                                                      TCmakeIdCopyString (copyfun),
                                                      ret_node)));

    dim_new = TCgetDim (IDS_TYPE (let_ids));
    dim_old = TCgetDim (ID_TYPE (PRF_ARG4 (arg_node)));

    if ((dim_new >= 0) && (dim_old >= 0) && (dim_new <= dim_old)) {
        /*
         * the old descriptor is large enough to be reused :-)
         */
        ret_node = TCmakeAssignIcm2 ("ND_ASSIGN__DESC", DUPdupIdsIdNt (let_ids),
                                     DUPdupIdNt (PRF_ARG4 (arg_node)), ret_node);
    } else {
        ret_node
          = MakeAllocDescIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids), rc,
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

    DBUG_ENTER ("COMPprfAllocOrReshape");

    let_ids = INFO_LASTIDS (arg_info);

    rc = NUM_VAL (PRF_ARG1 (arg_node));
    NUM_VAL (PRF_ARG1 (arg_node)) = 1;

    get_dim = MakeGetDimIcm (PRF_ARG2 (arg_node));
    set_shape_icm = MakeSetShapeIcm (PRF_ARG3 (arg_node), let_ids);

    ret_node = TCmakeAssignIcm1 (
      "IS_LASTREF__BLOCK_BEGIN", DUPdupIdNt (PRF_ARG4 (arg_node)),
      TCappendAssign (
        COMPprfReshape (arg_node, arg_info),
        MakeIncRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids), rc,
                      TCmakeAssignIcm1 (
                        "IS_LASTREF__BLOCK_ELSE", DUPdupIdNt (PRF_ARG4 (arg_node)),
                        MakeAllocIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids), rc, get_dim,
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

    DBUG_ENTER ("COMPprfIdxSel");

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
    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id), "2nd arg of F_idx_sel is no N_id!");

    icm_args = MakeTypeArgs (ID_NAME (arg2), ID_TYPE (arg2), FALSE, TRUE, FALSE,
                             TBmakeExprs (DUPdupNodeNt (arg1), NULL));

    /* idx_sel() works only for arrays with known dimension!!! */
    dim = TCgetDim (IDS_TYPE (let_ids));
    DBUG_ASSERT ((dim >= 0), "unknown dimension found!");

    /*
      if( global.backend == BE_cuda &&
          ( TCgetBasetype( ID_TYPE( arg2)) == T_float_dev ||
            TCgetBasetype( ID_TYPE( arg2)) == T_int_dev) &&
          !FUNDEF_ISCUDAGLOBALFUN( INFO_FUNDEF( arg_info))) {
        ret_node = TCmakeAssignIcm2( "CUDA_PRF_IDX_SEL__DATA",
                                     MakeTypeArgs( IDS_NAME( let_ids),
                                                   IDS_TYPE( let_ids),
                                                   FALSE, TRUE, FALSE,
                                                   DUPdoDupTree( icm_args)),
                                     MakeBasetypeArg( ID_TYPE(arg2)),
                                     NULL);
      }
      else {
    */
    ret_node
      = TCmakeAssignIcm2 ("ND_PRF_IDX_SEL__DATA",
                          MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE,
                                        TRUE, FALSE, DUPdoDupTree (icm_args)),
                          TCmakeIdCopyString (GenericFun (GF_copy, ID_TYPE (arg2))),
                          NULL);
    /*
      }
    */

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

    DBUG_ENTER ("COMPprfIdxModarray_AxSxS");

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    arg3 = PRF_ARG3 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg1) == N_id),
                 "1st arg of F_idx_modarray_AxSxS is no N_id!");
    /*
     * Because of IVE, at the 2nd argument position of F_idx_modarray might occur
     * an arithmetical expression (see function IdxArray)!!!
     */
    DBUG_ASSERT (((NODE_TYPE (arg2) == N_id) || (NODE_TYPE (arg2) == N_num)
                  || (NODE_TYPE (arg2) == N_prf)),
                 "2nd arg of F_idx_modarray_AxSxS is neither N_id nor N_num, N_prf!");
    DBUG_ASSERT (((NODE_TYPE (arg2) != N_id)
                  || (TCgetBasetype (ID_TYPE (arg2)) == T_int)),
                 "2nd arg of F_idx_modarray_AxSxS is a illegal indexing var!");
    DBUG_ASSERT ((NODE_TYPE (arg3) != N_array),
                 "3rd arg of F_idx_modarray_AxSxS is a N_array!");

    /*
      if( global.backend == BE_cuda &&
          ( TCgetBasetype( ID_TYPE( arg1)) == T_float_dev ||
            TCgetBasetype( ID_TYPE( arg1)) == T_int_dev) &&
          !FUNDEF_ISCUDAGLOBALFUN( INFO_FUNDEF( arg_info))) {
        ret_node = TCmakeAssignIcm4( "CUDA_PRF_IDX_MODARRAY_AxSxS__DATA",
                                     MakeTypeArgs( IDS_NAME( let_ids),
                                                   IDS_TYPE( let_ids),
                                                   FALSE, TRUE, FALSE,
                                     MakeTypeArgs( ID_NAME( arg1),
                                                   ID_TYPE( arg1),
                                                   FALSE, TRUE, FALSE,
                                     NULL)),
                                     DUPdupNodeNt( arg2),
                                     DUPdupNodeNt( arg3),
                                     MakeBasetypeArg( ID_TYPE(arg1)),
                   NULL);
      }
      else {
    */
    ret_node
      = TCmakeAssignIcm4 ("ND_PRF_IDX_MODARRAY_AxSxS__DATA",
                          MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE,
                                        TRUE, FALSE,
                                        MakeTypeArgs (ID_NAME (arg1), ID_TYPE (arg1),
                                                      FALSE, TRUE, FALSE, NULL)),
                          DUPdupNodeNt (arg2), DUPdupNodeNt (arg3),
                          TCmakeIdCopyString (GenericFun (GF_copy, ID_TYPE (arg1))),
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

    DBUG_ENTER ("COMPprfIdxModarray_AxSxA");

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    arg3 = PRF_ARG3 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg1) == N_id),
                 "1st arg of F_idx_modarray_AxSxA is no N_id!");
    /*
     * Because of IVE, at the 2nd argument position of F_idx_modarray might occur
     * an arithmetical expression (see function IdxArray)!!!
     */
    DBUG_ASSERT (((NODE_TYPE (arg2) == N_id) || (NODE_TYPE (arg2) == N_num)
                  || (NODE_TYPE (arg2) == N_prf)),
                 "2nd arg of F_idx_modarray_AxSxA is neither N_id nor N_num, N_prf!");
    DBUG_ASSERT (((NODE_TYPE (arg2) != N_id)
                  || (TCgetBasetype (ID_TYPE (arg2)) == T_int)),
                 "2nd arg of F_idx_modarray_AxSxA is a illegal indexing var!");
    DBUG_ASSERT ((NODE_TYPE (arg3) != N_array),
                 "3rd arg of F_idx_modarray_AxSxA is a N_array!");

    if (global.backend == BE_cuda
        && (TCgetBasetype (ID_TYPE (arg1)) == T_float_dev
            || TCgetBasetype (ID_TYPE (arg1)) == T_int_dev)
        && (TCgetBasetype (ID_TYPE (arg3)) == T_float_dev
            || TCgetBasetype (ID_TYPE (arg3)) == T_int_dev)
        && !FUNDEF_ISCUDAGLOBALFUN (INFO_FUNDEF (arg_info))) {
        ret_node
          = TCmakeAssignIcm4 ("CUDA_PRF_IDX_MODARRAY_AxSxA__DATA",
                              MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE,
                                            TRUE, FALSE,
                                            MakeTypeArgs (ID_NAME (arg1), ID_TYPE (arg1),
                                                          FALSE, TRUE, FALSE, NULL)),
                              DUPdupNodeNt (arg2), DUPdupNodeNt (arg3),
                              MakeBasetypeArg (ID_TYPE (arg1)), NULL);
    } else {
        ret_node
          = TCmakeAssignIcm4 ("ND_PRF_IDX_MODARRAY_AxSxA__DATA",
                              MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE,
                                            TRUE, FALSE,
                                            MakeTypeArgs (ID_NAME (arg1), ID_TYPE (arg1),
                                                          FALSE, TRUE, FALSE, NULL)),
                              DUPdupNodeNt (arg2), DUPdupNodeNt (arg3),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_TYPE (arg1))),
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

    DBUG_ENTER ("COMPprfIdxShapeSel");

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg1) == N_num), "1st arg of F_idx_shape_sel is no N_num!");
    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id), "2nd arg of F_idx_shape_sel is no N_id!");

    ret_node = TCmakeAssignIcm3 ("ND_PRF_IDX_SHAPE_SEL__DATA",
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                               FALSE, TRUE, FALSE, NULL),
                                 MakeTypeArgs (ID_NAME (arg2), ID_TYPE (arg2), FALSE,
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

    DBUG_ENTER ("COMPprfSel");

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

    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id), "2nd arg of F_sel_VxA is no N_id!");

    if (NODE_TYPE (arg1) == N_id) {
        DBUG_ASSERT (((TCgetBasetype (ID_TYPE (arg1)) == T_int)),
                     "1st arg of F_sel_VxA is a illegal indexing var!");

        icm_args
          = MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE, TRUE, FALSE,
                          MakeTypeArgs (ID_NAME (arg2), ID_TYPE (arg2), FALSE, TRUE,
                                        FALSE, TBmakeExprs (DUPdupIdNt (arg1), NULL)));

        ret_node
          = TCmakeAssignIcm3 ("ND_PRF_SEL_VxA__DATA_id", DUPdoDupTree (icm_args),
                              MakeSizeArg (arg1, TRUE),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_TYPE (arg2))),
                              NULL);
    } else {
        DBUG_ASSERT ((NODE_TYPE (arg1) == N_array),
                     "1st arg of F_sel_VxA is neither N_id nor N_array!");

        icm_args
          = MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE, TRUE, FALSE,
                          MakeTypeArgs (ID_NAME (arg2), ID_TYPE (arg2), FALSE, TRUE,
                                        FALSE,
                                        TBmakeExprs (MakeSizeArg (arg1, TRUE),
                                                     TCappendExprs (DUPdupExprsNt (
                                                                      ARRAY_AELEMS (
                                                                        arg1)),
                                                                    NULL))));

        ret_node
          = TCmakeAssignIcm2 ("ND_PRF_SEL_VxA__DATA_arr", DUPdoDupTree (icm_args),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_TYPE (arg2))),
                              NULL);
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

    DBUG_ENTER ("COMPprfModarray_AxVxS");

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    arg3 = PRF_ARG3 (arg_node);

    /*
     *   B = modarray( A, iv, val);
     *
     ****************************************************************************
     *
     * For efficiency reasons, constant arrays are excepted as 2nd argument of
     * modarray() as well:
     *
     *   A = fun( ...);
     *   B = modarray( A, [3,4], val);
     *
     * Here, the backend can avoid the creation of the array containing the shape
     * [3,4].
     */

    DBUG_ASSERT ((NODE_TYPE (arg1) == N_id), "1st arg of F_modarray_AxVxS is no N_id!");
    DBUG_ASSERT ((NODE_TYPE (arg3) != N_array),
                 "3rd arg of F_modarray_AxVxS is a N_array!");

    if (NODE_TYPE (arg2) == N_id) {
        DBUG_ASSERT (((TCgetBasetype (ID_TYPE (arg2)) == T_int)),
                     "2nd arg of F_modarray_AxVxS is a illegal indexing var!");

        ret_node
          = TCmakeAssignIcm5 ("ND_PRF_MODARRAY_AxVxS__DATA_id",
                              MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE,
                                            TRUE, FALSE,
                                            MakeTypeArgs (ID_NAME (arg1), ID_TYPE (arg1),
                                                          FALSE, TRUE, FALSE, NULL)),
                              DUPdupNodeNt (arg2), MakeSizeArg (arg2, TRUE),
                              DUPdupNodeNt (arg3),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_TYPE (arg1))),
                              NULL);
    } else {
        DBUG_ASSERT ((NODE_TYPE (arg2) == N_array),
                     "2nd arg of F_modarray_AxVxS is neither N_id nor N_array!");

        ret_node
          = TCmakeAssignIcm5 ("ND_PRF_MODARRAY_AxVxS__DATA_arr",
                              MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE,
                                            TRUE, FALSE,
                                            MakeTypeArgs (ID_NAME (arg1), ID_TYPE (arg1),
                                                          FALSE, TRUE, FALSE, NULL)),
                              MakeSizeArg (arg2, TRUE),
                              DUPdupExprsNt (ARRAY_AELEMS (arg2)), DUPdupNodeNt (arg3),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_TYPE (arg1))),
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

    DBUG_ENTER ("COMPprfModarray_AxVxA");

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    arg3 = PRF_ARG3 (arg_node);

    /*
     *   B = modarray( A, iv, val);
     *
     ****************************************************************************
     *
     * For efficiency reasons, constant arrays are excepted as 2nd argument of
     * modarray() as well:
     *
     *   A = fun( ...);
     *   B = modarray( A, [3,4], val);
     *
     * Here, the backend can avoid the creation of the array containing the shape
     * [3,4].
     */

    DBUG_ASSERT ((NODE_TYPE (arg1) == N_id), "1st arg of F_modarray_AxVxA is no N_id!");
    DBUG_ASSERT ((NODE_TYPE (arg3) != N_array),
                 "3rd arg of F_modarray_AxVxA is a N_array!");

    if (NODE_TYPE (arg2) == N_id) {
        DBUG_ASSERT (((TCgetBasetype (ID_TYPE (arg2)) == T_int)),
                     "2nd arg of F_modarray_AxVxA is a illegal indexing var!");

        ret_node
          = TCmakeAssignIcm5 ("ND_PRF_MODARRAY_AxVxA__DATA_id",
                              MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE,
                                            TRUE, FALSE,
                                            MakeTypeArgs (ID_NAME (arg1), ID_TYPE (arg1),
                                                          FALSE, TRUE, FALSE, NULL)),
                              DUPdupNodeNt (arg2), MakeSizeArg (arg2, TRUE),
                              DUPdupNodeNt (arg3),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_TYPE (arg1))),
                              NULL);
    } else {
        DBUG_ASSERT ((NODE_TYPE (arg2) == N_array),
                     "2nd arg of F_modarray_AxVxA is neither N_id nor N_array!");

        ret_node
          = TCmakeAssignIcm5 ("ND_PRF_MODARRAY_AxVxA__DATA_arr",
                              MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE,
                                            TRUE, FALSE,
                                            MakeTypeArgs (ID_NAME (arg1), ID_TYPE (arg1),
                                                          FALSE, TRUE, FALSE, NULL)),
                              MakeSizeArg (arg2, TRUE),
                              DUPdupExprsNt (ARRAY_AELEMS (arg2)), DUPdupNodeNt (arg3),
                              TCmakeIdCopyString (GenericFun (GF_copy, ID_TYPE (arg1))),
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
    node *arg1, *arg2;
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ("COMPprfGenarray");

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT ((0), "prf F_genarray not implemented yet!");

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

    DBUG_ENTER ("COMPprfTake");

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT (((NODE_TYPE (arg1) == N_id) || (NODE_TYPE (arg1) == N_num)),
                 "1st arg of F_take_SxV is neither N_id nor N_num!");
    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id), "2nd arg of F_take_SxV is no N_id!");

    icm_args
      = MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE, TRUE, FALSE,
                      MakeTypeArgs (ID_NAME (arg2), ID_TYPE (arg2), FALSE, TRUE, FALSE,
                                    TBmakeExprs (DUPdupNodeNt (arg1), NULL)));

    ret_node
      = TCmakeAssignIcm2 ("ND_PRF_TAKE_SxV__DATA", DUPdoDupTree (icm_args),
                          TCmakeIdCopyString (GenericFun (GF_copy, ID_TYPE (arg2))),
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

    DBUG_ENTER ("COMPprfDrop");

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT (((NODE_TYPE (arg1) == N_id) || (NODE_TYPE (arg1) == N_num)),
                 "1st arg of F_drop_SxV is neither N_id nor N_num!");
    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id), "2nd arg of F_drop_SxV is no N_id!");

    icm_args
      = MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE, TRUE, FALSE,
                      MakeTypeArgs (ID_NAME (arg2), ID_TYPE (arg2), FALSE, TRUE, FALSE,
                                    TBmakeExprs (DUPdupNodeNt (arg1), NULL)));

    ret_node
      = TCmakeAssignIcm2 ("ND_PRF_DROP_SxV__DATA", DUPdoDupTree (icm_args),
                          TCmakeIdCopyString (GenericFun (GF_copy, ID_TYPE (arg2))),
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

    DBUG_ENTER ("COMPprfCat");

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg1) == N_id), "1st arg of F_cat_VxV is no N_id!");
    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id), "2nd arg of F_cat_VxV is no N_id!");

    icm_args
      = MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids), FALSE, TRUE, FALSE,
                      MakeTypeArgs (ID_NAME (arg1), ID_TYPE (arg1), FALSE, TRUE, FALSE,
                                    MakeTypeArgs (ID_NAME (arg2), ID_TYPE (arg2), FALSE,
                                                  TRUE, FALSE, NULL)));

    copyfun1 = GenericFun (GF_copy, ID_TYPE (arg1));
    copyfun2 = GenericFun (GF_copy, ID_TYPE (arg2));
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

    DBUG_ENTER ("COMPprfOp_S");

    let_ids = INFO_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    /* assure that the prf has exactly one argument */
    DBUG_ASSERT ((PRF_EXPRS2 (arg_node) == NULL), "more than a single argument found!");
    DBUG_ASSERTF (((NODE_TYPE (arg) != N_id)
                   || (TCgetShapeDim (ID_TYPE (arg)) == SCALAR)),
                  ("non-scalar argument found!", global.prf_name[PRF_PRF (arg_node)]));

    ret_node = TCmakeAssignIcm3 ("ND_PRF_S__DATA", DUPdupIdsIdNt (let_ids),
                                 TCmakeIdCopyString (prf_ccode_tab[PRF_PRF (arg_node)]),
                                 DupExprs_NT_AddReadIcms (PRF_ARGS (arg_node)), NULL);

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
    node *arg;
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ("COMPprfOp_V");

    let_ids = INFO_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    /* assure that the prf has exactly one argument */
    DBUG_ASSERT ((PRF_EXPRS2 (arg_node) == NULL), "more than a single argument found!");
    ret_node = TCmakeAssignIcm3 ("ND_PRF_V__DATA", DUPdupIdsIdNt (let_ids),
                                 TCmakeIdCopyString (prf_ccode_tab[PRF_PRF (arg_node)]),
                                 DupExprs_NT_AddReadIcms (PRF_ARGS (arg_node)), NULL);

    DBUG_RETURN (ret_node);
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

    DBUG_ENTER ("COMPprfOp_SxS");

    /* assure that the prf has exactly two arguments */
    DBUG_ASSERT (((PRF_EXPRS1 (arg_node) != NULL) && (PRF_EXPRS2 (arg_node) != NULL)
                  && (PRF_EXPRS3 (arg_node) == NULL)),
                 "illegal number of args found!");

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERTF (((NODE_TYPE (arg1) != N_id)
                   || (TCgetShapeDim (ID_TYPE (arg1)) == SCALAR)),
                  ("%s: non-scalar first argument found!",
                   global.prf_name[PRF_PRF (arg_node)]));

    DBUG_ASSERTF (((NODE_TYPE (arg2) != N_id)
                   || (TCgetShapeDim (ID_TYPE (arg2)) == SCALAR)),
                  ("%s: non-scalar second argument found!",
                   global.prf_name[PRF_PRF (arg_node)]));
    ret_node = TCmakeAssignIcm3 ("ND_PRF_SxS__DATA", DUPdupIdsIdNt (let_ids),
                                 TCmakeIdCopyString (prf_ccode_tab[PRF_PRF (arg_node)]),
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
    node *arg1, *arg2;
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ("COMPprfOp_SxV");

    /* assure that the prf has exactly two arguments */
    DBUG_ASSERT (((PRF_EXPRS1 (arg_node) != NULL) && (PRF_EXPRS2 (arg_node) != NULL)
                  && (PRF_EXPRS3 (arg_node) == NULL)),
                 "illegal number of args found!");

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERTF (((NODE_TYPE (arg1) != N_id)
                   || (TCgetShapeDim (ID_TYPE (arg1)) == SCALAR)),
                  ("%s: non-scalar first argument found!",
                   global.prf_name[PRF_PRF (arg_node)]));

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
    node *arg1, *arg2;
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ("COMPprfOp_VxS");

    /* assure that the prf has exactly two arguments */
    DBUG_ASSERT (((PRF_EXPRS1 (arg_node) != NULL) && (PRF_EXPRS2 (arg_node) != NULL)
                  && (PRF_EXPRS3 (arg_node) == NULL)),
                 "illegal number of args found!");

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERTF (((NODE_TYPE (arg2) != N_id)
                   || (TCgetShapeDim (ID_TYPE (arg2)) == SCALAR)),
                  ("%s: non-scalar second argument found!",
                   global.prf_name[PRF_PRF (arg_node)]));
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
    node *arg1, *arg2;
    node *let_ids;
    node *ret_node;

    DBUG_ENTER ("COMPprfOp_VxV");

    /* assure that the prf has exactly two arguments */
    DBUG_ASSERT (((PRF_EXPRS1 (arg_node) != NULL) && (PRF_EXPRS2 (arg_node) != NULL)
                  && (PRF_EXPRS3 (arg_node) == NULL)),
                 "illegal number of args found!");

    let_ids = INFO_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    ret_node = TCmakeAssignIcm3 ("ND_PRF_VxV__DATA", DUPdupIdsIdNt (let_ids),
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

    DBUG_ENTER ("COMPprfTypeError");

    DBUG_ASSERT ((PRF_ARGS (arg_node) != NULL),
                 "1st argument of F_type_error not found!");

    DBUG_ASSERT ((NODE_TYPE (PRF_ARG1 (arg_node)) == N_type),
                 "1st argument of F_type_error  not a N_type node!");

    bottom = EXPRS_EXPR (PRF_ARGS (arg_node));

    DBUG_ASSERT ((TYisBottom (TYPE_TYPE (bottom))),
                 "1st argument of F_type_error contains non bottom type!");

    message = TBmakeStr (STRstring2SafeCEncoding (TYgetBottomError (TYPE_TYPE (bottom))));

    ret_node = TCmakeAssignIcm1 ("TYPE_ERROR", message, NULL);

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

    DBUG_ENTER ("COMPprfDispatchError");

    let_ids = INFO_LASTIDS (arg_info);

    DBUG_ASSERT ((PRF_ARGS (arg_node) != NULL),
                 "1st argument of F_dispatch_error not found!");

    /*
     * ARG1 is the number of types to ignore, so we skip ARG1 + 1
     * arguments...
     */
    args = PRF_ARGS (arg_node);

    DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (args)) = N_num), "N_num expected as 1st arg!");

    skip = NUM_VAL (EXPRS_EXPR (args)) + 1;

    while (skip != 0) {
        skip--;
        args = EXPRS_NEXT (args);
    }

    funname = EXPRS_EXPR (args);
    funargs = EXPRS_NEXT (args);

    ret_node = TCmakeAssignIcm5 ("DISPATCH_ERROR", TBmakeNum (TCcountIds (let_ids)),
                                 TCids2ExprsNt (let_ids), DUPdoDupNode (funname),
                                 TBmakeNum (TCcountExprs (funargs)),
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

    DBUG_ENTER ("COMPprfNoop");

    ret_node = TCmakeAssignIcm0 ("NOOP", NULL);

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

    DBUG_ENTER ("COMPprfIdxs2Offset");

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
        DBUG_ASSERT ((0), "unexpected 1st arg to idxs2offset");
#endif
    }

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

    DBUG_ENTER ("COMPprfVect2Offset");

    let_ids = INFO_LASTIDS (arg_info);
    iv_vect = PRF_ARG2 (arg_node);

    /*
     * either pass the ID of the vector or the
     * IDs of the shape elements.
     */
    if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_array) {
        icm = TCmakeIcm5 ("ND_VECT2OFFSET_arr", DUPdupIdsIdNt (let_ids),
                          TBmakeNum (TCgetTypesLength (ID_TYPE (iv_vect))),
                          DUPdupIdNt (iv_vect), MakeSizeArg (PRF_ARG1 (arg_node), TRUE),
                          DupExprs_NT_AddReadIcms (ARRAY_AELEMS (PRF_ARG1 (arg_node))));
    } else if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id) {
        icm = TCmakeIcm5 ("ND_VECT2OFFSET_id", DUPdupIdsIdNt (let_ids),
                          TBmakeNum (TCgetTypesLength (ID_TYPE (iv_vect))),
                          DUPdupIdNt (iv_vect), MakeSizeArg (PRF_ARG1 (arg_node), TRUE),
                          DUPdupIdNt (PRF_ARG1 (arg_node)));
#ifndef DBUG_OFF
    } else {
        DBUG_ASSERT ((0), "unexpected 1st arg to vect2offset");
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

    DBUG_ENTER ("COMPprfRunMtGenarray");

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm1 ("ND_PRF_RUNMT_GENARRAY__DATA",
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                               FALSE, TRUE, FALSE, NULL),
                                 NULL);

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

    DBUG_ENTER ("COMPprfRunMtModarray");

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm1 ("ND_PRF_RUNMT_MODARRAY__DATA",
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                               FALSE, TRUE, FALSE, NULL),
                                 NULL);

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

    DBUG_ENTER ("COMPprfRunMtFold");

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm1 ("ND_PRF_RUNMT_FOLD__DATA",
                                 MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                               FALSE, TRUE, FALSE, NULL),
                                 NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfGuard( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
static node *
COMPprfGuard (node *arg_node, info *arg_info)
{
    node *ret_node;

    DBUG_ENTER ("COMPprfGuard");

    ret_node = TCmakeAssignIcm1 ("ND_PRF_GUARD",
                                 DupExpr_NT_AddReadIcms (PRF_ARG1 (arg_node)), NULL);

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

    DBUG_ENTER ("COMPprfTypeConstraint");

    let_ids = INFO_LASTIDS (arg_info);
    arg_type = TYPE_TYPE (PRF_ARG1 (arg_node));

    if (TYisAKV (arg_type)) {
        DBUG_ASSERT (FALSE, "Type constraint with AKV type not implemented");

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

    DBUG_ENTER ("COMPprfSameShape");

    let_ids = INFO_LASTIDS (arg_info);

    ret_node
      = TCmakeAssignIcm5 ("ND_PRF_SAME_SHAPE", DUPdupIdsIdNt (let_ids),
                          DUPdupIdNt (PRF_ARG1 (arg_node)),
                          TBmakeNum (TCgetShapeDim (ID_TYPE (PRF_ARG1 (arg_node)))),
                          DUPdupIdNt (PRF_ARG2 (arg_node)),
                          TBmakeNum (TCgetShapeDim (ID_TYPE (PRF_ARG2 (arg_node)))),
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

    DBUG_ENTER ("COMPprfShapeMatchesDim");

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm3 ("ND_PRF_SHAPE_MATCHES_DIM", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 DUPdupIdNt (PRF_ARG2 (arg_node)), NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfNonNegVal( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COMPprfNonNegVal (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ("COMPprfNonNegVal");

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm2 ("ND_PRF_NON_NEG_VAL", DUPdupIdsIdNt (let_ids),
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

    DBUG_ENTER ("COMPprfValLtShape");

    let_ids = INFO_LASTIDS (arg_info);

    ret_node
      = TCmakeAssignIcm4 ("ND_PRF_VAL_LT_SHAPE", DUPdupIdsIdNt (let_ids),
                          DUPdupIdNt (PRF_ARG1 (arg_node)),
                          DUPdupIdNt (PRF_ARG2 (arg_node)),
                          TBmakeNum (TCgetShapeDim (ID_TYPE (PRF_ARG2 (arg_node)))),
                          NULL);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COMPprfValLeVal( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COMPprfValLeVal (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ("COMPprfValLeVal");

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm3 ("ND_PRF_VAL_LE_VAL", DUPdupIdsIdNt (let_ids),
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

    DBUG_ENTER ("COMPprfProdMatchesProdShape");

    let_ids = INFO_LASTIDS (arg_info);

    ret_node
      = TCmakeAssignIcm4 ("ND_PRF_PROD_MATCHES_PROD_SHAPE", DUPdupIdsIdNt (let_ids),
                          DUPdupIdNt (PRF_ARG1 (arg_node)),
                          DUPdupIdNt (PRF_ARG2 (arg_node)),
                          TBmakeNum (TCgetShapeDim (ID_TYPE (PRF_ARG2 (arg_node)))),
                          NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfDevice2Host (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ("COMPprfDevice2Host");

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm4 ("CUDA_MEM_TRANSFER", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 MakeBasetypeArg (ID_TYPE (PRF_ARG1 (arg_node))),
                                 TCmakeIdCopyString ("cudaMemcpyDeviceToHost"), NULL);

    DBUG_RETURN (ret_node);
}

node *
COMPprfHost2Device (node *arg_node, info *arg_info)
{
    node *ret_node;
    node *let_ids;

    DBUG_ENTER ("COMPprfHost2Device");

    let_ids = INFO_LASTIDS (arg_info);

    ret_node = TCmakeAssignIcm4 ("CUDA_MEM_TRANSFER", DUPdupIdsIdNt (let_ids),
                                 DUPdupIdNt (PRF_ARG1 (arg_node)),
                                 MakeBasetypeArg (ID_TYPE (PRF_ARG1 (arg_node))),
                                 TCmakeIdCopyString ("cudaMemcpyHostToDevice"), NULL);

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

    DBUG_ENTER ("COMPprf");

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
    char *label_str = NULL;

    DBUG_ENTER ("COMPdo");

    DBUG_ASSERT (((NODE_TYPE (DO_COND (arg_node)) == N_id)
                  || (NODE_TYPE (DO_COND (arg_node)) == N_bool)),
                 "loop condition is neither a N_id nor a N_bool node!");

    INFO_COND (arg_info) = TRUE;
    DO_COND (arg_node) = TRAVdo (DO_COND (arg_node), arg_info);
    INFO_COND (arg_info) = FALSE;

    DO_BODY (arg_node) = TRAVdo (DO_BODY (arg_node), arg_info);
    if (DO_SKIP (arg_node) != NULL) {
        DO_SKIP (arg_node) = TRAVdo (DO_SKIP (arg_node), arg_info);
    }

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
    BLOCK_INSTR (body)
      = TCmakeAssignIcm1 ("ND_LABEL", TCmakeIdCopyString (label_str), BLOCK_INSTR (body));

    /*
     * Insert code from DO_SKIP containing DEC_RCs into body
     */
    if (DO_SKIP (arg_node) != NULL) {
        BLOCK_INSTR (body)
          = TCappendAssign (BLOCK_INSTR (DO_SKIP (arg_node)), BLOCK_INSTR (body));

        DO_SKIP (arg_node) = NULL;
    }

    /*
     * Insert GOTO before do-loop.
     */
    ret_node = TCmakeAssignIcm1 ("ND_GOTO", TCmakeIdCopyString (label_str), ret_node);

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
    DBUG_ENTER ("COMPcond");

    DBUG_ASSERT (((NODE_TYPE (COND_COND (arg_node)) == N_id)
                  || (NODE_TYPE (COND_COND (arg_node)) == N_bool)),
                 "if-clause condition is neither a N_id nor a N_bool node!");

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

    DBUG_ENTER ("MakeIcmArgs_WL_LOOP1");

    dim = WLNODE_DIM (arg_node);
    args = TBmakeExprs (
      TBmakeNum (dim),
      TBmakeExprs (
        DUPdupIdNt (WITH2_VEC (wlnode)),
        TBmakeExprs (
          DUPdupIdNt (TCgetNthExprsExpr (dim, WITH2_IDS (wlnode))),
          TBmakeExprs (WLBnodeOrIntMakeIndex (NODE_TYPE (arg_node),
                                              WLNODE_GET_ADDR (arg_node, BOUND1), dim,
                                              wlids),
                       TBmakeExprs (WLBnodeOrIntMakeIndex (NODE_TYPE (arg_node),
                                                           WLNODE_GET_ADDR (arg_node,
                                                                            BOUND2),
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

    DBUG_ENTER ("MakeIcmArgs_WL_LOOP2");

    args = TBmakeExprs (MakeIcmArgs_WL_LOOP1 (arg_node),
                        TBmakeExprs (WLBnodeOrIntMakeIndex (NODE_TYPE (arg_node),
                                                            WLBLOCKSTR_GET_ADDR (arg_node,
                                                                                 STEP),
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

    DBUG_ENTER ("MakeIcmArgs_WL_OP1");

    args
      = MakeTypeArgs (IDS_NAME (_ids), IDS_TYPE (_ids), FALSE, TRUE, FALSE,
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

    DBUG_ENTER ("MakeIcmArgs_WL_OP2");

    args = MakeIcmArgs_WL_OP1 (arg_node, _ids);
    DBUG_ASSERT ((args != NULL), "no ICM args found!");
    last_arg = args;
    while (EXPRS_NEXT (last_arg) != NULL) {
        last_arg = EXPRS_NEXT (last_arg);
    }

    DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (last_arg)) == N_num), "wrong ICM arg found!");
    num_args = NUM_VAL (EXPRS_EXPR (last_arg));

    withid_ids = WITH2_IDS (wlnode);
    while (withid_ids != NULL) {
        last_arg = EXPRS_NEXT (last_arg)
          = TBmakeExprs (DUPdupIdNt (EXPRS_EXPR (withid_ids)), NULL);
        num_args--;
        withid_ids = EXPRS_NEXT (withid_ids);
    }
    DBUG_ASSERT ((num_args == 0), "wrong number of ids in WITHID_IDS found!");

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

    DBUG_ENTER ("MakeIcm_MT_ADJUST_SCHEDULER");

    DBUG_ASSERT (((NODE_TYPE (arg_node) == N_wlblock)
                  || (NODE_TYPE (arg_node) == N_wlublock)
                  || (NODE_TYPE (arg_node) == N_wlstride)
                  || (NODE_TYPE (arg_node) == N_wlstridevar)),
                 "illegal WL-node found!");

    dim = WLNODE_DIM (arg_node);

    if ((!WLNODE_ISNOOP (arg_node)) && (WLNODE_LEVEL (arg_node) == 0)
        && WITH2_PARALLELIZE (wlnode) && (SCHadjustmentRequired (dim, wlseg))) {

        begin_icm
          = TCmakeAssignIcm6 ("MT_ADJUST_SCHEDULER__BEGIN", DUPdupIdsIdNt (wlids),
                              TBmakeNum (WLSEGX_DIMS (wlseg)), TBmakeNum (dim),
                              WLBnodeOrIntMakeIndex (NODE_TYPE (arg_node),
                                                     WLBLOCKSTR_GET_ADDR (arg_node,
                                                                          BOUND1),
                                                     dim, wlids),
                              WLBnodeOrIntMakeIndex (NODE_TYPE (arg_node),
                                                     WLBLOCKSTR_GET_ADDR (arg_node,
                                                                          BOUND2),
                                                     dim, wlids),
                              WLBnodeOrIntMakeIndex (NODE_TYPE (arg_node),
                                                     WLBLOCKSTR_GET_ADDR (arg_node, STEP),
                                                     dim, wlids),
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
                              TBmakeNum (WLSEGX_DIMS (wlseg)), TBmakeNum (dim),
                              WLBnodeOrIntMakeIndex (NODE_TYPE (arg_node),
                                                     WLBLOCKSTR_GET_ADDR (arg_node,
                                                                          BOUND1),
                                                     dim, wlids),
                              WLBnodeOrIntMakeIndex (NODE_TYPE (arg_node),
                                                     WLBLOCKSTR_GET_ADDR (arg_node,
                                                                          BOUND2),
                                                     dim, wlids),
                              WLBnodeOrIntMakeIndex (NODE_TYPE (arg_node),
                                                     WLBLOCKSTR_GET_ADDR (arg_node, STEP),
                                                     dim, wlids),
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

    DBUG_ENTER ("MakeIcm_WL_INIT_OFFSET");

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

    DBUG_ENTER ("MakeIcm_WL_ADJUST_OFFSET");

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
    int idx_min, idx_max;
    int d;
    shpseg *shape;
    int icm_dim = (-1);
    node *withop;
    node *tmp_ids;
    node *idxs_exprs;

    DBUG_ENTER ("MakeIcm_WL_SET_OFFSET");

    /* for every ids of wlids (multioperator WL) */
    tmp_ids = wlids;
    withop = WITH2_WITHOP (wlnode);
    idxs_exprs = WITH2_IDXS (wlnode);

    while (withop != NULL) {
        if (WITHOP_IDX (withop) != NULL) {
            dim = WLNODE_DIM (arg_node);
            dims = WLSEGX_DIMS (wlseg);

            if (NODE_TYPE (wlseg) == N_wlseg) {
                /*
                 * infer first unrolling-blocking dimension
                 * (== 'dims', if no unrolling-blocking is done)
                 */
                d = 0;
                while ((d < dims) && ((WLSEG_UBV (wlseg))[d] == 1)) {
                    d++;
                }
                first_ublock_dim = d;

                /*
                 * infer first blocking dimension
                 * (== 'dims', if no blocking is done)
                 */
                d = 0;
                while ((d < dims) && ((WLSEG_BV (wlseg, 0))[d] == 1)) {
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
            shape = TYPES_SHPSEG (IDS_TYPE (tmp_ids));
            d = dims - 1;

            while (d >= 0) {
                WLBnodeOrIntGetNameOrVal (NULL, &idx_min, NODE_TYPE (wlseg),
                                          WLSEGX_IDX_GET_ADDR (wlseg, IDX_MIN, d));
                WLBnodeOrIntGetNameOrVal (NULL, &idx_max, NODE_TYPE (wlseg),
                                          WLSEGX_IDX_GET_ADDR (wlseg, IDX_MAX, d));

                if ((idx_min == 0)
                    && ((idx_max == IDX_SHAPE)
                        || ((shape != NULL) && (idx_max == SHPSEG_SHAPE (shape, d))))) {
                    d--;
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
    node *icm_chain = NULL, *body_icms, *default_icms;
    node *generator_icms;
    node *let_neutral, *neutral;
    node *res_ids, *idx_id, *lower_id, *upper_id;
    node *offs_id = NULL;
    bool isfold;

    DBUG_ENTER ("COMPwith");

    res_ids = INFO_LASTIDS (arg_info);

    /**
     * First, we traverse the partition.
     * This will yield the index vector var in INFO_IDXVEC (cf. COMPwithid)
     * and the generator-check icms in INFO_ICMCHAIN (cf. COMPgenerator).
     * Whereas the former is a back-link only (and thus has to be copied!), the
     * latter has been produced for insertion here and thus can be used as is!.
     * Furthermore, note that the index vector comes as N_id as we are
     * after EMM!!
     * Another aspect to be noticed here is that COMPpart relies on
     * INFO_ISFOLD to be set properly since for With-Loops different code
     * has to be created.
     * For single propagates, i.e., withloops that have only a propagate
     * operator, we use the fold compilation scheme, as well.
     */
    DBUG_ASSERT ((WITH_PARTS (arg_node) < 2),
                 "with-loop with non-AKS withid and multiple generators found!");

    isfold = ((NODE_TYPE (WITH_WITHOP (arg_node)) == N_fold)
              || (NODE_TYPE (WITH_WITHOP (arg_node)) == N_propagate));
    INFO_ISFOLD (arg_info) = isfold;
    DBUG_ASSERT (WITH_PART (arg_node) != NULL, "missing part in AUD with loop!");
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    WITH_WITHOP (arg_node) = RemoveIdxDuplicates (WITH_WITHOP (arg_node));

    /*
     * save the icm_chain for the generators in a local variable
     * to prevent it from being overwritten by nested with-loops
     * when traversing the code block
     */
    generator_icms = INFO_ICMCHAIN (arg_info);
    INFO_ICMCHAIN (arg_info) = NULL;

    idx_id = INFO_IDXVEC (arg_info);
    if (!isfold) {
        offs_id = EXPRS_EXPR (INFO_OFFSETS (arg_info));
    }
    lower_id = INFO_LOWERVEC (arg_info);
    upper_id = INFO_UPPERVEC (arg_info);

    INFO_IDXVEC (arg_info) = NULL;
    INFO_OFFSETS (arg_info) = NULL;

    DBUG_ASSERT (WITH_CODE (arg_node) != NULL, "missing code in AUD with loop!");
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    body_icms = DUPdoDupTree (BLOCK_INSTR (WITH_CBLOCK (arg_node)));

    if (isfold) {
        icm_chain
          = TCmakeAssignIcm3 ("AUD_WL_FOLD_END",
                              TCmakeIdCopyStringNt (ID_NAME (idx_id), ID_TYPE (idx_id)),
                              TCmakeIdCopyStringNt (ID_NAME (lower_id),
                                                    ID_TYPE (lower_id)),
                              TCmakeIdCopyStringNt (ID_NAME (upper_id),
                                                    ID_TYPE (upper_id)),
                              icm_chain);
        icm_chain = TCmakeAssignIcm0 ("AUD_WL_COND_END", icm_chain);
        icm_chain = TCappendAssign (body_icms, icm_chain);
        icm_chain = TCmakeAssignIcm0 ("AUD_WL_COND_BODY", icm_chain);
        icm_chain = TCappendAssign (generator_icms, icm_chain);
        icm_chain
          = TCmakeAssignIcm3 ("AUD_WL_FOLD_BEGIN",
                              TCmakeIdCopyStringNt (ID_NAME (idx_id), ID_TYPE (idx_id)),
                              TCmakeIdCopyStringNt (ID_NAME (lower_id),
                                                    ID_TYPE (lower_id)),
                              TCmakeIdCopyStringNt (ID_NAME (upper_id),
                                                    ID_TYPE (upper_id)),
                              icm_chain);

        if (NODE_TYPE (WITH_WITHOP (arg_node)) == N_fold) {
            /*
             * only fold withloops need this initialisation. For
             * propagate WLs, the object system enfores that res
             * and default are the same identifier.
             */
            neutral = FOLD_NEUTRAL (WITH_WITHOP (arg_node));
            let_neutral = TBmakeLet (DUPdoDupNode (res_ids), DUPdoDupNode (neutral));

            icm_chain = TCappendAssign (COMPdoCompile (let_neutral), icm_chain);
        }
    } else {

        /*
         * Free descriptor of subarray (IF it exists)
         */
        if (WITHOP_SUB (WITH_WITHOP (arg_node)) != NULL) {
            node *sub_id = WITHOP_SUB (WITH_WITHOP (arg_node));
            icm_chain = TCmakeAssignIcm1 ("ND_FREE__DESC",
                                          TCmakeIdCopyStringNt (ID_NAME (sub_id),
                                                                ID_TYPE (sub_id)),
                                          icm_chain);
        }

        if (CODE_NEXT (WITH_CODE (arg_node)) == NULL) {
            CTIabortLine (NODE_LINE (arg_node),
                          "cannot infer default element for with-loop");
        }

        default_icms
          = DUPdoDupTree (BLOCK_INSTR (CODE_CBLOCK (CODE_NEXT (WITH_CODE (arg_node)))));

        icm_chain
          = TCmakeAssignIcm3 ("AUD_WL_END",
                              TCmakeIdCopyStringNt (ID_NAME (idx_id), ID_TYPE (idx_id)),
                              TCmakeIdCopyStringNt (ID_NAME (offs_id), ID_TYPE (offs_id)),
                              TCmakeIdCopyStringNt (IDS_NAME (res_ids),
                                                    IDS_TYPE (res_ids)),
                              icm_chain);
        icm_chain = TCmakeAssignIcm0 ("AUD_WL_COND_END", icm_chain);
        icm_chain = TCappendAssign (default_icms, icm_chain);
        icm_chain = TCmakeAssignIcm0 ("AUD_WL_COND_DEFAULT", icm_chain);
        icm_chain = TCappendAssign (body_icms, icm_chain);
        icm_chain = TCmakeAssignIcm0 ("AUD_WL_COND_BODY", icm_chain);
        icm_chain = TCappendAssign (generator_icms, icm_chain);
        icm_chain
          = TCmakeAssignIcm3 ("AUD_WL_BEGIN",
                              TCmakeIdCopyStringNt (ID_NAME (idx_id), ID_TYPE (idx_id)),
                              TCmakeIdCopyStringNt (ID_NAME (offs_id), ID_TYPE (offs_id)),
                              TCmakeIdCopyStringNt (IDS_NAME (res_ids),
                                                    IDS_TYPE (res_ids)),
                              icm_chain);

        if (WITHOP_SUB (WITH_WITHOP (arg_node)) != NULL) {
            node *sub_get_dim;
            node *sub_set_shape;
            node *sub_id;

            sub_id = WITHOP_SUB (WITH_WITHOP (arg_node));

            /*
             * Calculate dimension of subarray
             *
             * dim( A_sub) = dim( A) - size( iv)
             */
            sub_get_dim
              = TCmakeIcm2 (prf_ccode_tab[F_sub_SxS],
                            TCmakeIcm1 ("ND_A_DIM",
                                        TCmakeIdCopyStringNt (IDS_NAME (res_ids),
                                                              IDS_TYPE (res_ids))),
                            TCmakeIcm1 ("ND_A_SIZE",
                                        TCmakeIdCopyStringNt (ID_NAME (idx_id),
                                                              ID_TYPE (idx_id))));

            /*
             * Annotate shape of subarray if default present
             * (genarray only)
             */
            if ((NODE_TYPE (WITH_WITHOP (arg_node)) == N_genarray)
                && (TCgetShapeDim (ID_TYPE (sub_id)) < 0)) {
                if (GENARRAY_DEFAULT (WITH_WITHOP (arg_node)) != NULL) {
                    DBUG_PRINT ("COMP", ("creating COPY__SHAPE for SUBALLOC var"));
                    /*
                     * copy shape
                     */
                    sub_set_shape
                      = TCmakeIcm1 ("ND_COPY__SHAPE",
                                    MakeTypeArgs (ID_NAME (sub_id), ID_TYPE (sub_id),
                                                  FALSE, TRUE, FALSE,
                                                  MakeTypeArgs (ID_NAME (
                                                                  GENARRAY_DEFAULT (
                                                                    WITH_WITHOP (
                                                                      arg_node))),
                                                                ID_TYPE (
                                                                  GENARRAY_DEFAULT (
                                                                    WITH_WITHOP (
                                                                      arg_node))),
                                                                FALSE, TRUE, FALSE,
                                                                NULL)));

                    icm_chain = TBmakeAssign (sub_set_shape, icm_chain);

                } else {
                    DBUG_ASSERT (0, "no default value found! "
                                    "cannot create subvar shape");
                }
            } else if ((NODE_TYPE (WITH_WITHOP (arg_node)) == N_modarray)
                       && (TCgetShapeDim (ID_TYPE (sub_id)) < 0)) {
                DBUG_PRINT ("COMP", ("creating WL_MODARRAY_SUBSHAPE for SUBALLOC var"));
                /*
                 * set shape in modarray case based upon result
                 * and index vector
                 */
                sub_set_shape
                  = TCmakeIcm4 ("WL_MODARRAY_SUBSHAPE",
                                TCmakeIdCopyStringNt (ID_NAME (sub_id), ID_TYPE (sub_id)),
                                DUPdupIdNt (WITHID_VEC (WITH_WITHID (arg_node))),
                                TBmakeNum (TCgetDim (ID_TYPE (sub_id))),
                                DUPdupIdsIdNt (res_ids));
                icm_chain = TBmakeAssign (sub_set_shape, icm_chain);
            }

            /*
             * Allocate descriptor of subarray
             */
            icm_chain = MakeAllocDescIcm (ID_NAME (sub_id), ID_TYPE (sub_id), 1,
                                          sub_get_dim, icm_chain);
        }
    }

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
    DBUG_ENTER ("COMPpart");
    DBUG_ASSERT ((PART_WITHID (arg_node) != NULL), "N_part without N_withid!");
    PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);

    DBUG_ASSERT ((PART_GENERATOR (arg_node) != NULL), "N_part without N_generator!");
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
    DBUG_ENTER ("COMPwithid");
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
 *     expects INFO_IDXVEC and INFO_ISFOLD to be set properly!
 *     ATTENTION: this is used in case of AUD only! i.e. when being called
 *     from N_with but NOT from N_with2!
 *
 *****************************************************************************/

node *
COMPgenerator (node *arg_node, info *arg_info)
{
    node *lower, *upper, *step, *width, *idx;

    DBUG_ENTER ("COMPgenerator");
    lower = GENERATOR_BOUND1 (arg_node);
    upper = GENERATOR_BOUND2 (arg_node);
    step = GENERATOR_STEP (arg_node);
    width = GENERATOR_WIDTH (arg_node);

    idx = INFO_IDXVEC (arg_info);
    INFO_LOWERVEC (arg_info) = lower;
    INFO_UPPERVEC (arg_info) = upper;

    if (step == NULL) {
        INFO_ICMCHAIN (arg_info)
          = TCmakeAssignIcm3 ((INFO_ISFOLD (arg_info) ? "AUD_WL_FOLD_LU_GEN"
                                                      : "AUD_WL_LU_GEN"),
                              TCmakeIdCopyStringNt (ID_NAME (lower), ID_TYPE (lower)),
                              TCmakeIdCopyStringNt (ID_NAME (idx), ID_TYPE (idx)),
                              TCmakeIdCopyStringNt (ID_NAME (upper), ID_TYPE (upper)),
                              NULL);
    } else if (width == NULL) {
        INFO_ICMCHAIN (arg_info)
          = TCmakeAssignIcm4 ((INFO_ISFOLD (arg_info) ? "AUD_WL_FOLD_LUS_GEN"
                                                      : "AUD_WL_LUS_GEN"),
                              TCmakeIdCopyStringNt (ID_NAME (lower), ID_TYPE (lower)),
                              TCmakeIdCopyStringNt (ID_NAME (idx), ID_TYPE (idx)),
                              TCmakeIdCopyStringNt (ID_NAME (upper), ID_TYPE (upper)),
                              TCmakeIdCopyStringNt (ID_NAME (step), ID_TYPE (step)),
                              NULL);
    } else {
        INFO_ICMCHAIN (arg_info)
          = TCmakeAssignIcm5 ((INFO_ISFOLD (arg_info) ? "AUD_WL_FOLD_LUSW_GEN"
                                                      : "AUD_WL_LUSW_GEN"),
                              TCmakeIdCopyStringNt (ID_NAME (lower), ID_TYPE (lower)),
                              TCmakeIdCopyStringNt (ID_NAME (idx), ID_TYPE (idx)),
                              TCmakeIdCopyStringNt (ID_NAME (upper), ID_TYPE (upper)),
                              TCmakeIdCopyStringNt (ID_NAME (step), ID_TYPE (step)),
                              TCmakeIdCopyStringNt (ID_NAME (width), ID_TYPE (width)),
                              NULL);
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

    DBUG_ENTER ("COMPwith2");

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
        if (WITHOP_IDX (withop) != NULL) {
            shpfac_decl_icms
              = TCmakeAssignIcm3 ("WL_DECLARE_SHAPE_FACTOR",
                                  MakeTypeArgs (IDS_NAME (tmp_ids), IDS_TYPE (tmp_ids),
                                                FALSE, TRUE, FALSE, NULL),
                                  DUPdupIdNt (WITH2_VEC (wlnode)),
                                  TBmakeNum (WITH2_DIMS (arg_node)), shpfac_decl_icms);

            shpfac_def_icms
              = TCmakeAssignIcm3 ("WL_DEFINE_SHAPE_FACTOR",
                                  MakeTypeArgs (IDS_NAME (tmp_ids), IDS_TYPE (tmp_ids),
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
                    && (TCgetShapeDim (ID_TYPE (sub_id)) < 0)) {
                    if (GENARRAY_DEFAULT (withop) != NULL) {
                        DBUG_PRINT ("COMP", ("creating COPY__SHAPE for SUBALLOC var"));
                        /*
                         * copy shape
                         */
                        sub_set_shape
                          = TCmakeIcm1 ("ND_COPY__SHAPE",
                                        MakeTypeArgs (ID_NAME (sub_id), ID_TYPE (sub_id),
                                                      FALSE, TRUE, FALSE,
                                                      MakeTypeArgs (ID_NAME (
                                                                      GENARRAY_DEFAULT (
                                                                        withop)),
                                                                    ID_TYPE (
                                                                      GENARRAY_DEFAULT (
                                                                        withop)),
                                                                    FALSE, TRUE, FALSE,
                                                                    NULL)));

                        alloc_icms = TBmakeAssign (sub_set_shape, alloc_icms);

                    } else {
                        DBUG_ASSERT (0, "no default value found! "
                                        "cannot create subvar shape");
                    }
                } else if ((NODE_TYPE (withop) == N_modarray)
                           && (TCgetShapeDim (ID_TYPE (sub_id)) < 0)) {
                    DBUG_PRINT ("COMP",
                                ("creating WL_MODARRAY_SUBSHAPE for SUBALLOC var"));
                    /*
                     * set shape in modarray case based upon result
                     * and index vector
                     */
                    sub_set_shape
                      = TCmakeIcm4 ("WL_MODARRAY_SUBSHAPE",
                                    TCmakeIdCopyStringNt (ID_NAME (sub_id),
                                                          ID_TYPE (sub_id)),
                                    DUPdupIdNt (WITHID_VEC (WITH2_WITHID (arg_node))),
                                    TBmakeNum (TCgetDim (ID_TYPE (sub_id))),
                                    DUPdupIdsIdNt (tmp_ids));

                    alloc_icms = TBmakeAssign (sub_set_shape, alloc_icms);
                }

                /*
                 * Allocate descriptor of subarray
                 */
                alloc_icms = MakeAllocDescIcm (ID_NAME (sub_id), ID_TYPE (sub_id), 1,
                                               sub_get_dim, alloc_icms);

                /*
                 * Free descriptor of subarray
                 */
                free_icms = TCmakeAssignIcm1 ("ND_FREE__DESC",
                                              TCmakeIdCopyStringNt (ID_NAME (sub_id),
                                                                    ID_TYPE (sub_id)),
                                              free_icms);
            }
        }

        if (NODE_TYPE (withop) == N_fold) {
            /*
             * put (tmp_ids, foldop) into FOLDLUT
             */
            INFO_FOLDLUT (arg_info)
              = LUTinsertIntoLutS (INFO_FOLDLUT (arg_info), IDS_NAME (tmp_ids),
                                   FUNDEF_NAME (FOLD_FUNDEF (withop)));

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
            DBUG_ASSERT ((0), "Break must not appear as a first withop");

        default:
            DBUG_ASSERT ((0), "illegal withop type found");
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

    if (WITH2_CODE (arg_node) != NULL) {
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    }

    /*******************************************
     * put it all together                     *
     *******************************************/

    break_label_str = TRAVtmpVarName (LABEL_POSTFIX);
    INFO_BREAKLABEL (arg_info) = break_label_str;

    ret_node = TCmakeAssigns9 (
      alloc_icms, fold_icms,
      TCmakeAssignIcm1 ("PF_BEGIN_WITH", TCmakeIdCopyString (profile_name),
                        TCmakeAssignIcm1 ("WL_SCHEDULE__BEGIN", icm_args, NULL)),
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

    DBUG_ENTER ("COMPwith3");

    INFO_CONCURRENTRANGES (arg_info) = WITH3_USECONCURRENTRANGES (arg_node);
    arg_node = TRAVdo (WITH3_RANGES (arg_node), arg_info);
    INFO_CONCURRENTRANGES (arg_info) = old_concurrentranges;

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
    node *family, *create, *next, *sync;
    node *thread_fun;
    char *familyName;
    str_buf *buffer;

    DBUG_ENTER ("COMPrange");

    buffer = SBUFcreate (1024);

    buffer = SBUFprintf (buffer, "family_%s",
                         FUNDEF_NAME (AP_FUNDEF (RANGE_RESULTS (arg_node))));

    familyName = SBUF2str (buffer);
    buffer = SBUFfree (buffer);

    family
      = TCmakeAssignIcm1 ("SAC_MUTC_DECL_FAMILY", TCmakeIdCopyString (familyName), NULL);

    thread_fun = TRAVdo (RANGE_RESULTS (arg_node), arg_info);

    if (NODE_TYPE (RANGE_LOWERBOUND (arg_node)) == N_id) {
        RANGE_LOWERBOUND (arg_node)
          = TCmakeAssignIcm2 ("SAC_ND_GETVAR",
                              TCmakeIdCopyStringNt (ID_NAME (RANGE_LOWERBOUND (arg_node)),
                                                    ID_TYPE (
                                                      RANGE_LOWERBOUND (arg_node))),
                              TCmakeIdCopyString (ID_NAME (RANGE_LOWERBOUND (arg_node))),
                              NULL);
    }

    if (NODE_TYPE (RANGE_UPPERBOUND (arg_node)) == N_id) {
        RANGE_UPPERBOUND (arg_node)
          = TCmakeAssignIcm2 ("SAC_ND_GETVAR",
                              TCmakeIdCopyStringNt (ID_NAME (RANGE_UPPERBOUND (arg_node)),
                                                    ID_TYPE (
                                                      RANGE_UPPERBOUND (arg_node))),
                              TCmakeIdCopyString (ID_NAME (RANGE_UPPERBOUND (arg_node))),
                              NULL);
    }

    create = TCmakeAssignIcm7 ("SAC_MUTC_CREATE", TCmakeIdCopyString (familyName),
                               TCmakeIdCopyString (
                                 RANGE_ISGLOBAL (arg_node) ? "" : "PLACE_LOCAL"),
                               DUPdoDupTree (RANGE_LOWERBOUND (arg_node)),
                               DUPdoDupTree (RANGE_UPPERBOUND (arg_node)),
                               (RANGE_CHUNKSIZE (arg_node) == NULL)
                                 ? TCmakeIdCopyString ("1")
                                 : DUPdoDupTree (RANGE_CHUNKSIZE (arg_node)),
#ifdef USE_STATIC_RESOURCE_ANNOTATIONS
                               TBmakeNum (RANGE_BLOCKSIZE (arg_node)),
#else
                               TCmakeIdCopyString (""),
#endif /* USE_STATIC_RESOURCE_ANNOTATIONS */
                               DUPdoDupTree (ASSIGN_INSTR (thread_fun)), NULL);

    sync = TCmakeAssignIcm1 ("SAC_MUTC_SYNC", TCmakeIdCopyString (familyName), NULL);

    next = TRAVopt (RANGE_NEXT (arg_node), arg_info);

    family = TCappendAssign (family, create);
    if (INFO_CONCURRENTRANGES (arg_info)) {
        family = TCappendAssign (family, next);
        family = TCappendAssign (family, sync);
    } else {
        family = TCappendAssign (family, sync);
        family = TCappendAssign (family, next);
    }

    /* FREEdoFreeTree(arg_node); */ /* Done by COMPlet for us */
    FREEdoFreeTree (thread_fun);

    DBUG_RETURN (family);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPwlsegx( node *arg_node, info *arg_info)
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
static node *
COMPwlsegx (node *arg_node, info *arg_info)
{
    node *old_wlseg;
    node *ret_node;
    node *next_icms = NULL;

    DBUG_ENTER ("COMPWLsegx");

    /*
     * stack old 'wlseg'.
     * store pointer to current segment in 'wlseg'.
     */
    old_wlseg = wlseg;
    wlseg = arg_node;

    /*
     * create ICMs for successor
     */
    if (WLSEGX_NEXT (arg_node) != NULL) {
        next_icms = TRAVdo (WLSEGX_NEXT (arg_node), arg_info);
    }

    /*
     * Collect initialization ICMs for schedulers.
     * This is only done for 'real' schedulers, not for the pseudo schedulers
     * used during sequential execution.
     */
    if (WLSEGX_SCHEDULING (arg_node) != NULL) {
        INFO_SCHEDULERINIT (arg_info)
          = TBmakeAssign (SCHcompileSchedulingWithTaskselInit (INFO_SCHEDULERID (
                                                                 arg_info),
                                                               wlids,
                                                               WLSEGX_SCHEDULING (
                                                                 arg_node),
                                                               WLSEGX_TASKSEL (arg_node),
                                                               arg_node),
                          INFO_SCHEDULERINIT (arg_info));

        (INFO_SCHEDULERID (arg_info))++;
    }

    ret_node
      = TCmakeAssigns4 (SCHcompileSchedulingWithTaskselBegin (INFO_SCHEDULERID (arg_info),
                                                              wlids,
                                                              WLSEGX_SCHEDULING (
                                                                arg_node),
                                                              WLSEGX_TASKSEL (arg_node),
                                                              arg_node),
                        MakeIcm_WL_INIT_OFFSET (arg_node,
                                                TRAVdo (WLSEGX_CONTENTS (arg_node),
                                                        arg_info)),
                        SCHcompileSchedulingWithTaskselEnd (INFO_SCHEDULERID (arg_info),
                                                            wlids,
                                                            WLSEGX_SCHEDULING (arg_node),
                                                            WLSEGX_TASKSEL (arg_node),
                                                            arg_node),
                        next_icms);

    /*
     * pop 'wlseg'.
     */
    wlseg = old_wlseg;

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPwlseg( node *arg_node, info *arg_info)
 *
 * @brief  Compilation of a N_wlseg
 *   Returns a N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *   (The whole with-loop-tree should be freed by COMPWith2() only!!)
 *
 * remark:
 *   - 'wlseg' points to the current with-loop segment.
 *
 ******************************************************************************/
node *
COMPwlseg (node *arg_node, info *arg_info)
{
    node *res;

    DBUG_ENTER ("COMPwlseg");

    res = COMPwlsegx (arg_node, arg_info);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPwlsegvar( node *arg_node, info *arg_info)
 *
 * @brief  Compilation of a N_wlsegvar-node:
 *   Returns a N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *   (The whole with-loop-tree should be freed by COMPWith2() only!!)
 *
 * remark:
 *   - 'wlseg' points to the current with-loop segment.
 *
 ******************************************************************************/
node *
COMPwlsegvar (node *arg_node, info *arg_info)
{
    node *res;

    DBUG_ENTER ("COMPwlsegvar");

    res = COMPwlsegx (arg_node, arg_info);

    DBUG_RETURN (res);
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
    int level, dim;
    bool is_block, mt_active, offset_needed;
    node *ret_node;
    char *icm_name_begin = NULL;
    char *icm_name_end = NULL;
    node *begin_icm = NULL;
    node *end_icm = NULL;
    node *node_icms = NULL;
    node *next_icms = NULL;

    DBUG_ENTER ("COMPwlxblock");

    level = WLXBLOCK_LEVEL (arg_node);
    dim = WLXBLOCK_DIM (arg_node);

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
            DBUG_ASSERT ((WLXBLOCK_CONTENTS (arg_node) == NULL),
                         "CONTENTS and NEXTDIM used simultaneous!");

            node_icms = TRAVdo (WLXBLOCK_NEXTDIM (arg_node), arg_info);
        }

        if (WLXBLOCK_CONTENTS (arg_node) != NULL) {
            DBUG_ASSERT ((WLXBLOCK_NEXTDIM (arg_node) == NULL),
                         "CONTENTS and NEXTDIM used simultaneous!");

            node_icms = TRAVdo (WLXBLOCK_CONTENTS (arg_node), arg_info);
        }
    }

    /*******************************************
     * create ICMs for block loop              *
     *******************************************/

    if (WLXBLOCK_ISNOOP (arg_node)) {
        DBUG_ASSERT ((level == 0), "inner NOOP N_wl...-node found!");

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

    DBUG_ENTER ("COMPwlblock");

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

    DBUG_ENTER ("COMPwlublock");

    res = COMPwlxblock (arg_node, arg_info);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPwlstridex( node *arg_node, info *arg_info)
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
static node *
COMPwlstridex (node *arg_node, info *arg_info)
{
    node *old_wlstride;
    int level, dim;
    bool mt_active, offset_needed;
    node *ret_node;
    char *icm_name_begin = NULL;
    char *icm_name_end = NULL;
    node *begin_icm = NULL;
    node *end_icm = NULL;
    node *node_icms = NULL;
    node *next_icms = NULL;

    static int simd_counter = 0;

    DBUG_ENTER ("COMPwlstridex");

    /*
     * stack old 'wlstride'.
     * store pointer to current segment in 'wlstride'.
     */
    old_wlstride = wlstride;
    wlstride = arg_node;

    level = WLSTRIDEX_LEVEL (arg_node);
    dim = WLSTRIDEX_DIM (arg_node);

    mt_active = WITH2_PARALLELIZE (wlnode);
    offset_needed = WITH2_NEEDSOFFSET (wlnode);

    /*******************************************
     * create ICMs for next dim / contents     *
     *******************************************/

    if (WLSTRIDEX_ISNOOP (arg_node)) {
        node_icms = MakeIcm_WL_ADJUST_OFFSET (arg_node, NULL);
    } else {
        if (WLSTRIDEX_CONTENTS (arg_node) != NULL) {
            node_icms = TRAVdo (WLSTRIDEX_CONTENTS (arg_node), arg_info);
        }
    }

    /*******************************************
     * create ICMs for stride loop             *
     *******************************************/

    if (WLSTRIDEX_ISNOOP (arg_node)) {
        DBUG_ASSERT ((level == 0), "inner NOOP N_wl...-node found!");

        if (offset_needed) {
            if (mt_active) {
                icm_name_begin = "WL_MT_STRIDE_NOOP_BEGIN";
                icm_name_end = "WL_MT_STRIDE_NOOP_END";
            } else {
                icm_name_begin = "WL_STRIDE_NOOP_BEGIN";
                icm_name_end = "WL_STRIDE_NOOP_END";
            }
        }
    } else if ((NODE_TYPE (arg_node) == N_wlstride) && (WLSTRIDE_DOUNROLL (arg_node))) {
        int cnt_unroll;
        node *new_icms = NULL;

        /*
         * unrolling
         */
        DBUG_ASSERT ((level > 0), "outer UNROLLING stride found!");

        if (mt_active) {
            icm_name_begin = "WL_MT_STRIDE_UNROLL_BEGIN";
            icm_name_end = "WL_MT_STRIDE_UNROLL_END";
        } else {
            icm_name_begin = "WL_STRIDE_UNROLL_BEGIN";
            icm_name_end = "WL_STRIDE_UNROLL_END";
        }

        DBUG_ASSERT ((((WLSTRIDE_BOUND2 (arg_node) - WLSTRIDE_BOUND1 (arg_node))
                       % WLSTRIDE_STEP (arg_node))
                      == 0),
                     "'step' is not a divisor of 'bound2 - bound1'!");

        for (cnt_unroll = (WLSTRIDE_BOUND2 (arg_node) - WLSTRIDE_BOUND1 (arg_node))
                          / WLSTRIDE_STEP (arg_node);
             cnt_unroll > 1; cnt_unroll--) {
            new_icms = TCappendAssign (DUPdoDupTree (node_icms), new_icms);
        }
        node_icms = TCappendAssign (node_icms, new_icms);
    } else {
        /*
         * no unrolling
         */

        if (mt_active) {
            if (level == 0) {
                icm_name_begin = "WL_MT_STRIDE_LOOP0_BEGIN";
            } else {
                icm_name_begin = (WLSTRIDEX_NEXT (arg_node) != NULL)
                                   ? "WL_MT_STRIDE_LOOP_BEGIN"
                                   : "WL_MT_STRIDE_LAST_LOOP_BEGIN";
            }
            icm_name_end = "WL_MT_STRIDE_LOOP_END";
        } else {
            if (level == 0) {
                icm_name_begin = "WL_STRIDE_LOOP0_BEGIN";
            } else {
                icm_name_begin = (WLSTRIDEX_NEXT (arg_node) != NULL)
                                   ? "WL_STRIDE_LOOP_BEGIN"
                                   : "WL_STRIDE_LAST_LOOP_BEGIN";
            }
            icm_name_end = "WL_STRIDE_LOOP_END";
        }
    }

    if (icm_name_begin != NULL) {
        begin_icm
          = TCmakeAssignIcm1 (icm_name_begin, MakeIcmArgs_WL_LOOP2 (arg_node), NULL);
        if ((NODE_TYPE (arg_node) == N_wlstride) && WLSTRIDE_ISSIMDSUITABLE (arg_node)) {
            begin_icm
              = TCmakeAssignIcm1 ("WL_SIMD_BEGIN", TBmakeNum (simd_counter), begin_icm);
        }
    }

    if (icm_name_end != NULL) {
        if ((NODE_TYPE (arg_node) == N_wlstride) && WLSTRIDE_ISSIMDSUITABLE (arg_node)) {
            end_icm = TCmakeAssignIcm1 ("WL_SIMD_END", TBmakeNum (simd_counter), NULL);
            simd_counter++;
        } else {
            end_icm = NULL;
        }
        end_icm
          = TCmakeAssignIcm1 (icm_name_end, MakeIcmArgs_WL_LOOP2 (arg_node), end_icm);
    }

    /*******************************************
     * create ICMs for successor               *
     *******************************************/

    if (WLSTRIDEX_NEXT (arg_node) != NULL) {
        next_icms = TRAVdo (WLSTRIDEX_NEXT (arg_node), arg_info);
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
 * @fn  node *COMPwlstride( node *arg_node, info *arg_info)
 *
 * @brief  Compilation of a N_wlstride-node:
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
    node *res;

    DBUG_ENTER ("COMPwlstride");

    res = COMPwlstridex (arg_node, arg_info);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPwlstridevar( node *arg_node, info *arg_info)
 *
 * @brief  Compilation of a N_wlstridevar-node:
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
COMPwlstridevar (node *arg_node, info *arg_info)
{
    node *res;

    DBUG_ENTER ("COMPwlstride");

    res = COMPwlstridex (arg_node, arg_info);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPwlgridx( node *arg_node, info *arg_info)
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
static node *
COMPwlgridx (node *arg_node, info *arg_info)
{
    int dim;
    bool mt_active, is_fitted;
    node *ret_node;
    char *icm_name_begin = NULL;
    char *icm_name_end = NULL;
    node *begin_icm = NULL;
    node *end_icm = NULL;
    node *node_icms = NULL;
    node *next_icms = NULL;

    DBUG_ENTER ("COMPwlgridx");

    dim = WLGRIDX_DIM (arg_node);

    mt_active = WITH2_PARALLELIZE (wlnode);
    is_fitted = WLGRIDX_ISFITTED (arg_node);

    if (WLGRIDX_ISNOOP (arg_node)) {
        node_icms = MakeIcm_WL_ADJUST_OFFSET (arg_node, NULL);
    } else {
        /*******************************************
         * create ICMs for next dim                *
         *******************************************/

        if (WLGRIDX_NEXTDIM (arg_node) != NULL) {
            DBUG_ASSERT ((WLGRIDX_CODE (arg_node) == NULL),
                         "CODE and NEXTDIM used simultaneous!");

            node_icms
              = MakeIcm_WL_SET_OFFSET (arg_node,
                                       TRAVdo (WLGRIDX_NEXTDIM (arg_node), arg_info));
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

            DBUG_ASSERT (WLGRIDX_CODE (arg_node) != NULL,
                         "WLGRIDX_CODE must not be NULL!");

            /*
             * insert compiled code.
             */
            cexprs = CODE_CEXPRS (WLGRIDX_CODE (arg_node));

            DBUG_ASSERT ((WLGRIDX_CBLOCK (arg_node) != NULL),
                         "no code block found in N_Ncode node");
            DBUG_ASSERT ((WLGRIDX_CBLOCK_INSTR (arg_node) != NULL),
                         "first instruction of block is NULL"
                         " (should be a N_empty node)");

            if (NODE_TYPE (WLGRIDX_CBLOCK_INSTR (arg_node)) != N_empty) {
                node_icms = DUPdoDupTree (WLGRIDX_CBLOCK_INSTR (arg_node));
            }

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
                    DBUG_ASSERT ((NODE_TYPE (cexpr) == N_id), "code expr is not a id");

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
                    DBUG_ASSERT ((0), "illegal withop type found");
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

    if ((WITHID_VECNEEDED (WITH2_WITHID (wlnode))) && (!WLGRIDX_ISNOOP (arg_node))
        && ((WLGRIDX_CODE (arg_node) != NULL) || (WLGRIDX_NEXTDIM (arg_node) != NULL))) {
        DBUG_PRINT ("COMP", ("IV %s is built! :(", ID_NAME (WITH2_VEC (wlnode))));
        node_icms = TCmakeAssignIcm1 ("WL_SET_IDXVEC", MakeIcmArgs_WL_LOOP1 (arg_node),
                                      node_icms);
    }

    if (WLGRIDX_ISNOOP (arg_node)) {
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
        if ((NODE_TYPE (arg_node) == N_wlgrid)
            && ((WLGRID_DOUNROLL (arg_node))
                || ((WLGRID_BOUND2 (arg_node) - WLGRID_BOUND1 (arg_node) == 1)
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

            for (cnt_unroll = WLGRID_BOUND2 (arg_node) - WLGRID_BOUND1 (arg_node);
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

    if (WLGRIDX_NEXT (arg_node) != NULL) {
        next_icms = TRAVdo (WLGRIDX_NEXT (arg_node), arg_info);
    }

    /*******************************************
     * put it all together                     *
     *******************************************/

    ret_node = TCmakeAssigns4 (begin_icm, node_icms, end_icm, next_icms);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPwlgrid( node *arg_node, info *arg_info)
 *
 * @brief  Compilation of a N_wlgrid-node.
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
    node *res;

    DBUG_ENTER ("COMPwlgrid");

    res = COMPwlgridx (arg_node, arg_info);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn  node *COMPWLgridvar( node *arg_node, info *arg_info)
 *
 * @brief  Compilation of a N_wlgridvar-node:
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
COMPwlgridvar (node *arg_node, info *arg_info)
{
    node *res;

    DBUG_ENTER ("COMPwlgridvar");

    res = COMPwlgridx (arg_node, arg_info);

    DBUG_RETURN (res);
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
    DBUG_ENTER ("COMPcode");

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
