/*
 * $Log$
 * Revision 2.64  2000/07/04 14:55:57  jhs
 * Added mtn-stuff.
 * ./
 *
 * Revision 2.63  2000/06/30 15:22:51  jhs
 * BuildParamsByDFM debugged.
 * COMPmt uses BuildParamsByDFM now.
 *
 * Revision 2.62  2000/06/23 15:10:33  dkr
 * signature of DupTree changed
 *
 * Revision 2.61  2000/06/21 13:31:35  jhs
 * Parts of N_mt-compilation added.
 *
 * Revision 2.60  2000/06/05 12:34:56  dkr
 * minor corrections in comments done
 *
 * Revision 2.59  2000/05/31 14:09:09  dkr
 * INFO_COMP_VARDEC is now handled correctly in COMPBlock()
 *
 * Revision 2.58  2000/05/26 21:58:15  dkr
 * signature of GetAdjustedFoldCode() changed
 *
 * Revision 2.57  2000/05/26 11:32:57  dkr
 * Function GetFoldCode() renamed into GetUnadjustedFoldCode()
 * Function GetAdjustedFoldCode() added
 * Code for with-loop brushed
 * Names of variables in code for fold-with-loops are always correct now
 * :-)
 * new compound macros for with-loop used
 *
 * Revision 2.56  2000/05/26 11:11:39  jhs
 * Added infrastructure for mt2-compiling
 *
 * Revision 2.55  2000/05/24 19:02:46  dkr
 * Inference for WL_ADJUST_OFFSET-icm lifted into a function
 * macro NCODE_CBLOCK_INSTR used
 * some DBUG_ASSERTs added
 *   (BLOCK_INSTR and NCODE_CBLOCK_INSTR have to be != NULL)
 *
 * Revision 2.54  2000/05/24 13:30:23  nmw
 * (jhs) Added workarounds for BLOCK_INSTR == NULL.
 *
 * Revision 2.53  2000/04/27 13:16:31  dkr
 * Bug in COMPArg fixed:
 * In SPMD-functions no DEC_RC_... and INC_RC_... ICMs are added anymore!
 * This bug causes the memory leak in the NAS-MG benchmark :-(
 *
 * Revision 2.52  2000/04/20 11:35:20  jhs
 * Comments at CreateIcmND_FUN_DEC added.
 * Made static: RenameVar, RenameReturn, ShapeToArray.
 *
 * Revision 2.51  2000/04/18 14:00:48  jhs
 * Added COMPSt and COMPMt.
 *
 * Revision 2.50  2000/04/13 09:00:03  jhs
 * Beautified.
 *
 * Revision 2.49  2000/03/29 16:06:27  jhs
 * Brushing.
 *
 * Revision 2.48  2000/03/27 14:53:53  dkr
 * ICMs SAC_WL_..._END and SAC_PF_END_WITH are in correct order now
 *
 * Revision 2.47  2000/03/21 15:46:03  dkr
 * ICM_INDENT explicitly set to 0 if MakeNode is used instead of MakeIcm
 *
 * Revision 2.46  2000/02/23 20:16:34  cg
 * Node status ST_imported replaced by ST_imported_mod and
 * ST_imported_class in order to allow distinction between enteties
 * that are imported from a module and those that are imported from a
 * class.
 *
 * Revision 2.45  2000/02/23 17:47:34  cg
 * Header file refcount.h no longer included.
 * Type property functions IsUnique(<type>), IsBoxed(<type>)
 * moved from refcount.c to tree_compound.c.
 *
 * Revision 2.44  2000/02/23 17:27:01  cg
 * The entry TYPES_TDEF of the TYPES data structure now contains a
 * reference to the corresponding N_typedef node for all user-defined
 * types.
 * Therefore, most calls to LookupType() are eliminated.
 * Please, keep the back references up to date!!
 *
 * Revision 2.43  2000/02/21 13:36:12  jhs
 * Removed some direct (->) accesses to nodes.
 * nodeline >> NODE_LINE
 * nodetype >> NODE_TYPE
 *
 * Revision 2.42  2000/02/11 21:17:28  dkr
 * fixed a bug in MakeAllocArrayICMs()
 *
 * Revision 2.41  2000/02/11 19:21:06  dkr
 * code brushed.
 * fixed a bug: correction of refcounts in conditionals works correctly
 * even with hidden object now :)
 *
 * Revision 2.40  2000/01/25 13:31:19  dkr
 * include of optimize.h removed
 *
 * Revision 2.39  2000/01/17 16:25:58  cg
 * Removed static and dynamic versions of the ICMs
 * MT_SPMD_[STATIC|DYNAMIC]_MODE_[BEGIN|ALTSEQ|END].
 * General version now is identical with the former dynamic
 * version.
 *
 * Revision 2.38  1999/11/16 15:26:23  dkr
 * some minor changes in COMPWlgrid:
 *   initializing of 'insert_icm' added
 *   some comments added
 *
 * Revision 2.37  1999/11/09 21:21:12  dkr
 * support for FREE_HIDDEN, DEC_RC_FREE_HIDDEN added.
 * hidden objects are no longer handled by FREE_ARRAY, DEC_RC_FREE_ARRAY ICMs 8-))
 *
 * Revision 2.36  1999/10/29 21:38:39  sbs
 * changed ND_DEC_RC into ND_DEC_RC_FREE_ARRAY after loops!
 * for details see comment in COMPloop!
 *
 * Revision 2.35  1999/10/28 09:53:22  sbs
 * re-coded compilation of F_reshape!
 * Now, the shape vector will be freed properly!
 *
 * Revision 2.34  1999/10/26 19:03:52  dkr
 * Fixed a bug in COMPPrf: ICMs are now correctly wrapped by an
 * assign-node
 *
 * Revision 2.33  1999/10/19 13:11:00  sbs
 * inclusion of type_info.mac adapted to the new .mac style
 *
 * Revision 2.32  1999/09/20 11:35:10  jhs
 * Reduced the whole thing from 9000 to 8000 lines.
 *
 * Revision 2.31  1999/09/10 14:29:46  jhs
 * Removed those ugly MAKENODE_xxx macros.
 * Extremly brushed COMPLoop (now I unterstand at least half of it, before I had not
 * even an idea what could happen there).
 * Commented COMPLoop and COMPCond a "little" bit.
 *
 * Revision 2.30  1999/09/01 17:11:53  jhs
 * Expanded COMPSync to refcounters in barriers.
 *
 * Revision 2.29  1999/08/30 16:33:06  cg
 * Bug fixed in handling of foldfuns:
 * Their local vardecs are no longer compiled! This will be done
 * automatically after they have been added to the local vardecs
 * of the regular function a foldfun belongs to.
 *
 * Revision 2.28  1999/08/30 16:07:19  jhs
 * Handling of non-with-loop-code in sync-blocks added.
 *
 * [ eliminated ]
 *
 * Revision 1.1  1995/03/29  12:38:10  hw
 * Initial revision
 */

#include <stdlib.h>
#include <string.h>

#include "tree.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "print.h"
#include "dbug.h"
#include "my_debug.h"
#include "internal_lib.h"
#include "Error.h"
#include "traverse.h"
#include "DataFlowMask.h"
#include "compile.h"
#include "convert.h"
#include "DupTree.h"
#include "ReuseWithArrays.h"
#include "free.h"
#include "scheduling.h"
#include "precompile.h"
#include "typecheck.h" /* to use some ugly old macros ... */

/******************************************************************************
 *
 * global variable:  int barrier_id
 *
 * description:
 *   An unambigious barrier identification is required because of the use of
 *   labels in the barrier implementation and the fact that several
 *   synchronisation blocks may be found within a single spmd block which
 *   means that several synchronisation barriers are situated in one function.
 *
 ******************************************************************************/

static int barrier_id = 0;

/******************************************************************************
 *
 * global variables for the compilation of the new with-loop
 *
 ******************************************************************************/

static ids *wl_ids = NULL;
static node *wl_node = NULL;
static node *wl_seg = NULL;

#define TYP_IFsize(sz) sz
int basetype_size[] = {
#include "type_info.mac"
};

#define DUMMY_NAME "__OUT_"
#define LABEL_NAME "__Label" /* basic-name for goto label */

/*
 * the following macros are used while generation of N_icm-nodes
 */

#define MUST_REFCOUNT(item, arg) (item##_REFCNT (arg) >= 0)

#define TYPE_REFCNT(type) (IsArray (type) + IsNonUniqueHidden (type) - 1)

#define FUN_DOES_REFCOUNT(fundef, i)                                                     \
    (FUNDEF_STATUS (fundef) != ST_Cfun)                                                  \
      ? 1                                                                                \
      : (FUNDEF_PRAGMA (fundef) == NULL)                                                 \
          ? 0                                                                            \
          : (FUNDEF_REFCOUNTING (fundef) == NULL)                                        \
              ? 0                                                                        \
              : (i >= PRAGMA_NUMPARAMS (FUNDEF_PRAGMA (fundef)))                         \
                  ? 0                                                                    \
                  : FUNDEF_REFCOUNTING (fundef)[i]

#define ICMPARAM_TAG(expr) ID_NAME (EXPRS_EXPR (expr))
#define ICMPARAM_ARG1(expr) EXPRS_EXPR (EXPRS_NEXT (expr))

/*
 * This macro indicates whether there are multiple segments present or not.
 * It uses the global variable 'wl_seg'.
 */
#define MULTIPLE_SEGS                                                                    \
    ((wl_seg != NULL)                                                                    \
     && ((NODE_TYPE (wl_seg) == N_WLseg) ? (WLSEG_NEXT (wl_seg) != NULL)                 \
                                         : (WLSEGVAR_NEXT (wl_seg) != NULL)))

/*
 ********** PLEASE DO NOT USE THE FOLLOWING MACROS FOR NEW CODE!!!! ***********
 */

#define SET_VARS_FOR_MORE_ICMS                                                           \
    first_assign = CURR_ASSIGN (arg_info);                                               \
    old_arg_node = arg_node;                                                             \
    last_assign = NEXT_ASSIGN (arg_info);                                                \
    arg_node = LET_EXPR (INFO_COMP_LASTLET (arg_info))

#define INSERT_ASSIGN                                                                    \
    if (NULL != last_assign) {                                                           \
        APPEND_ASSIGNS (first_assign, last_assign);                                      \
    }

#define CHECK_REUSE__ALLOC_ARRAY_ND(new, stype_new, old, stype_old)                      \
    if ((1 == old->refcnt) && (stype_new == stype_old)) {                                \
        /* create ND_CHECK_REUSE_ARRAY  */                                               \
        node *num;                                                                       \
        BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_CHECK_REUSE_ARRAY", old, new);  \
        first_assign = CURR_ASSIGN (arg_info);                                           \
        DBUG_PRINT ("COMP", ("first:" P_FORMAT, first_assign));                          \
        old_arg_node = arg_node;                                                         \
        arg_node = LET_EXPR (INFO_COMP_LASTLET (arg_info));                              \
        /* create ND_ALLOC_ARRAY */                                                      \
        num = MakeNum (0);                                                               \
        CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, new, num);        \
        APPEND_ASSIGNS (first_assign, next_assign);                                      \
    } else {                                                                             \
        /* create ND_ALLOC_ARRAY */                                                      \
        node *num;                                                                       \
        DBUG_ASSERT (((old->refcnt == -1) || (!(optimize & OPT_RCO))),                   \
                     "Refcnt of prf-arg neither -1 nor 1 !");                            \
        num = MakeNum (0);                                                               \
        BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY", type_id_node,     \
                       new);                                                             \
        MAKE_NEXT_ICM_ARG (icm_arg, num);                                                \
        first_assign = CURR_ASSIGN (arg_info);                                           \
        DBUG_PRINT ("COMP", ("first:" P_FORMAT, first_assign));                          \
        old_arg_node = arg_node;                                                         \
        arg_node = LET_EXPR (INFO_COMP_LASTLET (arg_info));                              \
    }

#define DEC_OR_FREE_RC_ND(array, num_node)                                               \
    if (1 < array->refcnt) { /* create ND_DEC_RC */                                      \
        DEC_RC_ND (array, num_node);                                                     \
    } else {                                                                             \
        if (array->refcnt > 0) {                                                         \
            DEC_RC_FREE_ND (array, num_node);                                            \
        }                                                                                \
    }

#define DEC_RC_ND(array, num_node)                                                       \
    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC", array, num_node);                        \
    APPEND_ASSIGNS (first_assign, next_assign)

#define DEC_RC_FREE_ND(array, num_node) /* create ND_DEC_RC_FREE_ARRAY */                \
    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE_ARRAY", array, num_node);             \
    APPEND_ASSIGNS (first_assign, next_assign)

#define INC_RC_ND(array, num_node) /* create ND_INC_RC */                                \
    CREATE_2_ARY_ICM (next_assign, "ND_INC_RC", array, num_node);                        \
    APPEND_ASSIGNS (first_assign, next_assign)

#define SET_RC_ND(array, num_node) /* create ND_SET_RC */                                \
    CREATE_2_ARY_ICM (next_assign, "ND_SET_RC", array, num_node);                        \
    APPEND_ASSIGNS (first_assign, next_assign)

#define DEC_OR_FREE_RC_ND_IDS(ids, num_node)                                             \
    if (1 < ids->refcnt) { /* create ND_DEC_RC */                                        \
        DEC_RC_ND_IDS (ids, num_node);                                                   \
    } else {                                                                             \
        if (ids->refcnt > 0) {                                                           \
            DEC_RC_FREE_ND_IDS (ids, num_node);                                          \
        }                                                                                \
    }

#define DEC_RC_ND_IDS(ids, num_node) /* create ND_DEC_RC */                              \
    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC", MakeId2 (DupOneIds (ids, NULL)),         \
                      num_node);                                                         \
    APPEND_ASSIGNS (first_assign, next_assign)

#define DEC_RC_FREE_ND_IDS(ids, num_node) /* create ND_DEC_RC_FREE_ARRAY */              \
    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE_ARRAY",                               \
                      MakeId2 (DupOneIds (ids, NULL)), num_node);                        \
    APPEND_ASSIGNS (first_assign, next_assign)

#ifdef TAGGED_ARRAYS
#define CREATE_TMP_CONST_ARRAY(array, rc)                                                \
    array_is_const = 1;                                                                  \
    old_arg_node = arg_node;                                                             \
    COUNT_ELEMS (n_elems, ARRAY_AELEMS (array))                                          \
    tmp_array1 = MakeId1 ("__TMP");                                                      \
    NODE_TYPE (INFO_COMP_LASTLET (arg_info)) = N_block; /* reuse previous N_let*/        \
    CREATE_4_ARY_ICM (first_assign, "ND_DECL_AKS", type_id_node, tmp_array1,             \
                      MakeNum (1), MakeNum (n_elems));                                   \
    arg_node = first_assign;                                                             \
    CREATE_CONST_ARRAY (array, tmp_array1, type_id_node, rc);                            \
    array = tmp_array1 /* set array to __TMP */

#else /* TAGGED_ARRAYS */

#define CREATE_TMP_CONST_ARRAY(array, rc)                                                \
    array_is_const = 1;                                                                  \
    old_arg_node = arg_node;                                                             \
    COUNT_ELEMS (n_elems, ARRAY_AELEMS (array))                                          \
    tmp_array1 = MakeId1 ("__TMP");                                                      \
    NODE_TYPE (INFO_COMP_LASTLET (arg_info)) = N_block; /* reuse previous N_let*/        \
    CREATE_4_ARY_ICM (first_assign, "ND_KS_DECL_ARRAY", type_id_node, tmp_array1,        \
                      MakeNum (1), MakeNum (n_elems));                                   \
    arg_node = first_assign;                                                             \
    CREATE_CONST_ARRAY (array, tmp_array1, type_id_node, rc);                            \
    array = tmp_array1 /* set array to __TMP */
#endif                 /* TAGGED_ARRAYS */

#define CREATE_CONST_ARRAY(array, name, type, rc)                                        \
    CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type, name, rc);                    \
    APPEND_ASSIGNS (first_assign, next_assign);                                          \
    COUNT_ELEMS (n_elems, ARRAY_AELEMS (array));                                         \
    CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_S", name, MakeNum (n_elems));  \
    EXPRS_NEXT (icm_arg) = ARRAY_AELEMS (array);                                         \
    APPEND_ASSIGNS (first_assign, next_assign)

#ifdef TAGGED_ARRAYS
#define DECL_ARRAY(assign, node, var_str, var_str_node)                                  \
    COUNT_ELEMS (n_elems, node);                                                         \
    var_str_node = MakeId1 (var_str);                                                    \
    CREATE_4_ARY_ICM (assign, "ND_DECL_AKS", type_id_node, var_str_node, MakeNum (1),    \
                      MakeNum (n_elems))

#else /* TAGGED_ARRAYS */

#define DECL_ARRAY(assign, node, var_str, var_str_node)                                  \
    COUNT_ELEMS (n_elems, node);                                                         \
    var_str_node = MakeId1 (var_str);                                                    \
    CREATE_4_ARY_ICM (assign, "ND_KS_DECL_ARRAY", type_id_node, var_str_node,            \
                      MakeNum (1), MakeNum (n_elems))
#endif /* TAGGED_ARRAYS */

#define INSERT_ID_NODE(no, last, str) EXPRS_NEXT (last) = MakeExprs (MakeId1 (str), no);

/*
 * following macros are used to compute last but one or next N_assign from
 *  a 'arg_info' node
 */

#define CURR_ASSIGN(arg_info)                                                            \
    ((N_block == NODE_TYPE (INFO_COMP_LASTASSIGN (arg_info)))                            \
       ? BLOCK_INSTR (INFO_COMP_LASTASSIGN (arg_info))                                   \
       : ASSIGN_NEXT (INFO_COMP_LASTASSIGN (arg_info)))

#define NEXT_ASSIGN(arg_info) ASSIGN_NEXT (CURR_ASSIGN (arg_info))

#define INSERT_BEFORE(arg_info, node)                                                    \
    if (N_block == NODE_TYPE (INFO_COMP_LASTASSIGN (arg_info))) {                        \
        BLOCK_INSTR (INFO_COMP_LASTASSIGN (arg_info)) = node;                            \
    } else {                                                                             \
        ASSIGN_NEXT (INFO_COMP_LASTASSIGN (arg_info)) = node;                            \
    }

#define FREE_TYPE(a)                                                                     \
    if (NULL != a->shpseg) {                                                             \
        FREE (a->shpseg)                                                                 \
    }                                                                                    \
    FREE (a)

#define FREE_VARDEC(a)                                                                   \
    FREE_TYPE (VARDEC_TYPE (a))                                                          \
    FREE (a)

static int label_nr = 0;

/******************************************************************************
 *
 * Function:
 *   char *GenericFun(int which, types *type)
 *
 * Description:
 *   Returns the name of the specified generic function of the given type
 *
 ******************************************************************************/

static char *
GenericFun (int which, types *type)
{
    node *tdef;
    char *ret = NULL;

    DBUG_ENTER ("GenericFun");

    DBUG_PRINT ("COMP", ("Looking for generic fun %d (0==copy/1==free)"));

    if (TYPES_BASETYPE (type) == T_user) {
        tdef = TYPES_TDEF (type);
        DBUG_ASSERT ((tdef != NULL), "Failed attempt to look up typedef");

        if ((TYPEDEF_BASETYPE (tdef) == T_hidden)
            || (TYPEDEF_BASETYPE (tdef) == T_user)) {
            if (TYPEDEF_TNAME (tdef) != NULL) {
                tdef = TYPES_TDEF (TYPEDEF_TYPE (tdef));
                DBUG_ASSERT ((tdef != NULL), "Failed attempt to look up typedef");
            }

            switch (which) {
            case 0:
                ret = TYPEDEF_COPYFUN (tdef);
                break;
            case 1:
                ret = TYPEDEF_FREEFUN (tdef);
                break;
            default:
                DBUG_ASSERT ((0), "Unknown kind if generic function requested");
            }
        }
    }

    DBUG_PRINT ("COMP", ("Found generic fun %s", ret));

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   node *MakeAdjustRcICM(ids *varname, int num)
 *
 * description:
 *   According to num, either ND_INC_RC( varname, num),
 *                            ----,
 *                       or   ND_DEC_RC_FREE_ARRAY( varname, -num)
 *   is created.
 *
 ******************************************************************************/

static node *
MakeAdjustRcICM (ids *varname, int num)
{
    node *result;

    DBUG_ENTER ("MakeAdjustRcICM");

    if (num > 0) {
        result = MakeAssignIcm2 ("ND_INC_RC", MakeId2 (DupOneIds (varname, NULL)),
                                 MakeNum (num));
    } else {
        if (num == 0) {
            result = NULL;
        } else {
            /* num < 0 */
            result = MakeAssignIcm2 ("ND_DEC_RC_FREE_ARRAY",
                                     MakeId2 (DupOneIds (varname, NULL)), MakeNum (-num));
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *GetUnadjustedFoldCode( node *fundef)
 *
 * description:
 *   Returns the foldop-code of the pseudo fold-fun 'fundef'.
 *
 *   This function simply extract the assignments of the fundef-body.
 *   It is assumed that the names of the variables are the same is in the
 *   context of the corresponding with-loop!
 *   This property is *not* hold before the compilation has been started!
 *   (Note that Precompile() calls the function AdjustFoldFundef() for each
 *   fold-fundef)
 *
 *   Before the compilation phase the function GetAdjustedFoldCode() should be
 *   used instead!
 *
 ******************************************************************************/

node *
GetUnadjustedFoldCode (node *fundef)
{
    node *fold_code, *tmp;

    DBUG_ENTER ("GetUnadjustedFoldCode");

    DBUG_ASSERT ((fundef != NULL), "fundef is NULL!");
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "no fundef found!");

    /*
     * get code of the pseudo fold-fun
     */
    fold_code = DupTree (FUNDEF_INSTR (fundef));

    /*
     * remove declaration-ICMs ('ND_KS_DECL_ARRAY_ARG') from code.
     */
    while (
      (NODE_TYPE (ASSIGN_INSTR (fold_code)) == N_icm)
      && (strcmp (ICM_NAME (ASSIGN_INSTR (fold_code)), "ND_KS_DECL_ARRAY_ARG") == 0)) {
        fold_code = FreeNode (fold_code);
    }

    /*
     * we must remove the return node of the code (it is the last node)
     */
    tmp = fold_code;
    DBUG_ASSERT ((ASSIGN_NEXT (tmp) != NULL), "no assign found");
    while (ASSIGN_NEXT (ASSIGN_NEXT (tmp)) != NULL) {
        tmp = ASSIGN_NEXT (tmp);
    }
    ASSIGN_NEXT (tmp) = FreeNode (ASSIGN_NEXT (tmp));

    DBUG_RETURN (fold_code);
}

/******************************************************************************
 *
 * function:
 *   node *GetAdjustedFoldCode( node *fundef,
 *                              ids *acc, node *cexpr)
 *
 * description:
 *   Returns the foldop-code of the pseudo fold-fun 'fundef' with adjusted
 *   var-names: returns-value and the first formal argument of the fold-fun
 *   are renamed according to 'acc', the second formal argument is renamed
 *   according to 'cexpr'.
 *
 * note:
 *   Besides the renaming of variables the fold-fun code is simply extracted
 *   from the definition. All back-references (e.g. vardecs) are NOT adjusted.
 *   Therefore this routine should NOT be used in a compiler phase before
 *   the pre-compilation (especially NOT during with-loop-folding) !!!
 *
 * parameters:
 *   'acc' is the accumulator variable.
 *   'cexpr' is the expression in the operation part.
 *
 ******************************************************************************/

node *
GetAdjustedFoldCode (node *fundef, ids *acc, node *cexpr)
{
    node *fold_code;

    DBUG_ENTER ("GetAdjustedFoldCode");

    fundef = AdjustFoldFundef (fundef, acc, cexpr);
    fold_code = GetUnadjustedFoldCode (fundef);

    DBUG_RETURN (fold_code);
}

/******************************************************************************
 *
 * function:
 *   node *GetFoldVardecs( node *fundef)
 *
 * description:
 *   returns the vardecs of the pseudo fold-fun 'fundef'.
 *
 ******************************************************************************/

node *
GetFoldVardecs (node *fundef)
{
    node *fold_vardecs;

    DBUG_ENTER ("GetFoldVardecs");

    /*
     * get vardecs of the pseudo fold-fun
     */
    fold_vardecs = DupTree (FUNDEF_VARDEC (fundef));

    DBUG_RETURN (fold_vardecs);
}

/******************************************************************************
 *
 * function:
 *   ids *GetIndexIds(ids *index_ids, int dim)
 *
 * description:
 *   returns the index-ids for dimension 'dim' found in 'index_ids'.
 *   'index_ids' is a vector of index-ids (e.g. NWITHID_IDS(...)) containing
 *    at least 'dim' elements.
 *
 ******************************************************************************/

static ids *
GetIndexIds (ids *index_ids, int dim)
{
    int i;

    DBUG_ENTER ("GetIndexIds");

    for (i = 0; i < dim; i++) {
        DBUG_ASSERT ((index_ids != NULL), "not enough ids found");
        index_ids = IDS_NEXT (index_ids);
    }

    DBUG_RETURN (index_ids);
}

/******************************************************************************
 *
 * function:
 *   node *MakeAllocArrayICMs( ids *mm_ids, node *next)
 *
 * description:
 *   Builds a ND_ALLOC_ARRAY icm for each RC-ids in 'mm_ids'.
 *   The given node 'next' is appended to the created assign-chain.
 *
 *   CAUTION: Do not use this function in conjunction with a
 *            'ND_CHECK_REUSE_ARRAY' icm.
 *            Use 'MakeAllocArrayICMs_reuse()' instead!!
 *
 ******************************************************************************/

static node *
MakeAllocArrayICMs (ids *mm_ids, node *next)
{
    simpletype s_type;
    node *assign;
    node *last_assign = NULL;
    node *assigns = NULL;

    DBUG_ENTER ("MakeAllocArrayICMs");

    while (mm_ids != NULL) {
        if (IDS_REFCNT (mm_ids) >= 0) {
            GET_BASIC_SIMPLETYPE (s_type, IDS_TYPE (mm_ids));
            assign = MakeAssignIcm3 ("ND_ALLOC_ARRAY", MakeId1 (type_string[s_type]),
                                     MakeId2 (DupOneIds (mm_ids, NULL)),
                                     MakeNum (IDS_REFCNT (mm_ids)));

            if (assigns == NULL) {
                assigns = assign;
            } else {
                ASSIGN_NEXT (last_assign) = assign;
            }
            last_assign = assign;
        }
        mm_ids = IDS_NEXT (mm_ids);
    }

    if (last_assign != NULL) {
        ASSIGN_NEXT (last_assign) = next;
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *MakeAllocArrayICMs_reuse( ids *mm_ids, node *next)
 *
 * description:
 *   builds a 'ND_ALLOC_ARRAY, ND_INC_RC' icm for each RC-ids in 'mm_ids':
 *     ND_ALLOC_ARRAY( <type>, mm_ids, 0);
 *     ND_INC_RC( mm_ids, IDS_RC( mm_ids));
 *   The extra 'ND_INC_RC' is needed, if there are any 'ND_CHECK_REUSE_ARRAY'
 *   ICMs above 'ND_ALLOC_ARRAY' !!!
 *
 *   The given node 'next' is appended to the created assign-chain.
 *
 ******************************************************************************/

static node *
MakeAllocArrayICMs_reuse (ids *mm_ids, node *next)
{
    simpletype s_type;
    node *assign;
    node *last_assign = NULL;
    node *assigns = NULL;

    DBUG_ENTER ("MakeAllocArrayICMs_reuse");

    while (mm_ids != NULL) {
        if (IDS_REFCNT (mm_ids) >= 0) {
            GET_BASIC_SIMPLETYPE (s_type, IDS_TYPE (mm_ids));
            assign
              = MakeAssign (MakeIcm3 ("ND_ALLOC_ARRAY", MakeId1 (type_string[s_type]),
                                      MakeId2 (DupOneIds (mm_ids, NULL)), MakeNum (0)),
                            MakeAssignIcm2 ("ND_INC_RC",
                                            MakeId2 (DupOneIds (mm_ids, NULL)),
                                            MakeNum (IDS_REFCNT (mm_ids))));

            if (assigns == NULL) {
                assigns = assign;
            } else {
                ASSIGN_NEXT (last_assign) = assign;
            }
            last_assign = ASSIGN_NEXT (assign);
        }
        mm_ids = IDS_NEXT (mm_ids);
    }

    if (last_assign != NULL) {
        ASSIGN_NEXT (last_assign) = next;
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *MakeIncRcICMs( ids *mm_ids, node *next)
 *
 * description:
 *   Builds a 'ND_INC_RC' icm for each ids in 'mm_ids', which rc is >=0.
 *   The given node 'next' is appended to the created assign-chain.
 *
 ******************************************************************************/

static node *
MakeIncRcICMs (ids *mm_ids, node *next)
{
    node *assign;
    node *last_assign = NULL;
    node *assigns = NULL;

    DBUG_ENTER ("MakeIncRcICMs");

    while (mm_ids != NULL) {
        if (IDS_REFCNT (mm_ids) >= 0) {
            assign = MakeAssignIcm2 ("ND_INC_RC", MakeId2 (DupOneIds (mm_ids, NULL)),
                                     MakeNum (IDS_REFCNT (mm_ids)));

            if (assigns == NULL) {
                assigns = assign;
            } else {
                ASSIGN_NEXT (last_assign) = assign;
            }
            last_assign = assign;
        }
        mm_ids = IDS_NEXT (mm_ids);
    }

    if (last_assign != NULL) {
        ASSIGN_NEXT (last_assign) = next;
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *MakeDecRcICMs( ids *mm_ids, node *next)
 *
 * description:
 *   According to RC and type, builds either
 *             a ND_DEC_RC( varname, num),
 *          or a ND_DEC_RC_FREE_ARRAY( varname, num)
 *          or a ND_DEC_RC_FREE_HIDDEN( varname, num, freefun)
 *   icm for each ids in 'mm_ids'.
 *   The given node 'next' is appended to the created assign-chain.
 *
 ******************************************************************************/

static node *
MakeDecRcICMs (ids *mm_ids, node *next)
{
    node *assign;
    node *last_assign = NULL;
    node *assigns = NULL;

    DBUG_ENTER ("MakeDecRcICMs");

    while (mm_ids != NULL) {
        if (IDS_REFCNT (mm_ids) >= 0) {
            if (IDS_REFCNT (mm_ids) > 1) {
                assign = MakeAssignIcm2 ("ND_DEC_RC", MakeId2 (DupOneIds (mm_ids, NULL)),
                                         MakeNum (1));
            } else {
                if (IsNonUniqueHidden (IDS_TYPE (mm_ids))) {
                    assign
                      = MakeAssignIcm3 ("ND_DEC_RC_FREE_HIDDEN",
                                        MakeId2 (DupOneIds (mm_ids, NULL)), MakeNum (1),
                                        MakeId1 (GenericFun (1, IDS_TYPE (mm_ids))));
                } else {
                    assign
                      = MakeAssignIcm2 ("ND_DEC_RC_FREE_ARRAY",
                                        MakeId2 (DupOneIds (mm_ids, NULL)), MakeNum (1));
                }
            }

            if (assigns == NULL) {
                assigns = assign;
            } else {
                ASSIGN_NEXT (last_assign) = assign;
            }
            last_assign = assign;
        }
        mm_ids = IDS_NEXT (mm_ids);
    }

    if (last_assign != NULL) {
        ASSIGN_NEXT (last_assign) = next;
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *IdOrNumToIndex( node *id_or_num, int dim)
 *
 * description:
 *   'id_or_num' is a N_id- or N_num-node.
 *   If 'id_or_num' is a N_id-node, this is an identifier of an array, and
 *    we are interested in 'id_or_num[ dim]'.
 *   Therefore we build a new N_id-node with this array-access as name:
 *    'SAC_ND_A_FIELD( id_or_num)[ dim]'.
 *
 *   This function is needed for compilation of N_WLstriVar, N_WLgridVar,
 *    because the params of these nodes are N_id- or N_id-nodes...
 *
 ******************************************************************************/

static node *
IdOrNumToIndex (node *id_or_num, int dim)
{
    node *index;
    char *str;

    DBUG_ENTER ("IdOrNumToIndex");

    if (NODE_TYPE (id_or_num) == N_id) {
        str = (char *)Malloc ((strlen (ID_NAME (id_or_num)) + 25) * sizeof (char));
        sprintf (str, "SAC_ND_READ_ARRAY( %s,  %d)", ID_NAME (id_or_num), dim);
        index = MakeId1 (str);
    } else {
        DBUG_ASSERT ((NODE_TYPE (id_or_num) == N_num), "wrong node type found");
        index = DupNode (id_or_num);
    }

    DBUG_RETURN (index);
}

/******************************************************************************
 *
 * Function:
 *   node *AddVardec( node *vardec, types *type, char *name, node *fundef)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
AddVardec (node *vardec, types *type, char *name, node *fundef)
{
    node *tmp;
    int insert;

    DBUG_ENTER ("AddVardec");

    /* look if there is already a matching vardec */
    insert = 1;
    tmp = vardec;
    if (tmp != NULL) {
        while ((VARDEC_NEXT (tmp) != NULL) && (insert == 1)) {
            if (!strcmp (VARDEC_NAME (tmp), name)) {
                insert = 0;
            }
            tmp = VARDEC_NEXT (tmp);
        }
    }

    if ((insert == 1) && ((tmp == NULL) || strcmp (VARDEC_NAME (tmp), name))) {
        /*
         * now insert new vardec node
         */
        types *new_type = DuplicateTypes (type, 0);
        node *new_vardec = MakeVardec (StringCopy (name), new_type, NULL);

        if (vardec != NULL) {
            DBUG_ASSERT ((NULL == VARDEC_NEXT (tmp)), "VARDEC_NEXT(tmp) != NULL");
            VARDEC_NEXT (tmp) = new_vardec;
        } else {
            vardec = new_vardec;

            /*
             * if there is no vardec yet,
             * store the new one in 'FUNDEF_VARDEC( fundef)'
             */
            FUNDEF_VARDEC (fundef) = vardec;
        }

        /*
         * we must update FUNDEF_DFM_BASE!!
         */
        FUNDEF_DFM_BASE (fundef)
          = DFMUpdateMaskBase (FUNDEF_DFM_BASE (fundef), FUNDEF_ARGS (fundef),
                               FUNDEF_VARDEC (fundef));
    }

    DBUG_RETURN (vardec);
}

/*
 *
 *  functionname  : AdjustAddedAssigns
 *  description   :
 *  remarks       :
 *
 */

static void
AdjustAddedAssigns (node *before, node *after)
{
    char *new_id, *old_id;
    node *tmp, *last;

    DBUG_ENTER ("AdjustAddedAssigns");

    before = ASSIGN_NEXT (before);

    while (before != NULL) {
        if (NODE_TYPE (ASSIGN_INSTR (before)) == N_icm) {
            if ((0
                 == strcmp (ICM_NAME (ASSIGN_INSTR (before)), "ND_KS_MAKE_UNIQUE_ARRAY"))
                || (0
                    == strcmp (ICM_NAME (ASSIGN_INSTR (before)),
                               "ND_MAKE_UNIQUE_HIDDEN"))) {
                tmp = ASSIGN_NEXT (after);
                last = after;
                old_id = ID_NAME (ICM_ARG1 (ASSIGN_INSTR (before)));
                new_id = ID_NAME (ICM_ARG2 (ASSIGN_INSTR (before)));

                while (tmp != NULL) {
                    if (NODE_TYPE (ASSIGN_INSTR (tmp)) == N_icm) {
                        if (((0
                              == strcmp (ICM_NAME (ASSIGN_INSTR (tmp)),
                                         "ND_DEC_RC_FREE_HIDDEN"))
                             && (0
                                 == strcmp (ID_NAME (ICM_ARG1 (ASSIGN_INSTR (tmp))),
                                            old_id)))
                            || ((0
                                 == strcmp (ICM_NAME (ASSIGN_INSTR (tmp)),
                                            "ND_DEC_RC_FREE_ARRAY"))
                                && (0
                                    == strcmp (ID_NAME (ICM_ARG1 (ASSIGN_INSTR (tmp))),
                                               old_id)))
                            || ((0
                                 == strcmp (ICM_NAME (ASSIGN_INSTR (tmp)), "ND_ALLOC_RC"))
                                && (0
                                    == strcmp (ID_NAME (ICM_ARG1 (ASSIGN_INSTR (tmp))),
                                               new_id)))) {
                            ASSIGN_NEXT (last) = ASSIGN_NEXT (tmp);
                        } else if ((0
                                    == strcmp (ICM_NAME (ASSIGN_INSTR (tmp)),
                                               "ND_NO_RC_FREE_ARRAY"))
                                   && (0
                                       == strcmp (ID_NAME (ICM_ARG1 (ASSIGN_INSTR (tmp))),
                                                  new_id))) {
                            ICM_NAME (ASSIGN_INSTR (before))
                              = "ND_KS_NO_RC_MAKE_UNIQUE_ARRAY";
                        } else if ((0
                                    == strcmp (ICM_NAME (ASSIGN_INSTR (tmp)),
                                               "ND_NO_RC_FREE_HIDDEN"))
                                   && (0
                                       == strcmp (ID_NAME (ICM_ARG1 (ASSIGN_INSTR (tmp))),
                                                  new_id))) {
                            ICM_NAME (ASSIGN_INSTR (before))
                              = "ND_NO_RC_MAKE_UNIQUE_HIDDEN";
                        } else if (((0
                                     == strcmp (ICM_NAME (ASSIGN_INSTR (tmp)),
                                                "ND_NO_RC_ASSIGN_HIDDEN"))
                                    || (0
                                        == strcmp (ICM_NAME (ASSIGN_INSTR (tmp)),
                                                   "ND_KS_NO_RC_ASSIGN_ARRAY")))
                                   && (0
                                       == strcmp (ID_NAME (ICM_ARG1 (ASSIGN_INSTR (tmp))),
                                                  new_id))
                                   && (0
                                       == strcmp (ID_NAME (ICM_ARG2 (ASSIGN_INSTR (tmp))),
                                                  old_id))) {
                            new_id = old_id;
                        }
                    }

                    last = tmp;
                    tmp = ASSIGN_NEXT (tmp);
                }
            }
        }

        before = ASSIGN_NEXT (before);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : BasetypeSize
 *  description   :
 *  remarks       :
 *
 */

static int
BasetypeSize (types *type)
{
    int ret;
    node *tdef;

    DBUG_ENTER ("BasetypeSize");

    if (TYPES_BASETYPE (type) == T_user) {
        tdef = TYPES_TDEF (type);
        DBUG_ASSERT (tdef != NULL, "Failed attempt to look up typedef");

        ret = basetype_size[TYPEDEF_BASETYPE (tdef)];
    } else {
        ret = basetype_size[TYPES_BASETYPE (type)];
    }

    DBUG_RETURN (ret);
}

/*
 *
 *  functionname  : MakeTypeString
 *  arguments     : 1) types structure
 *  description   : converts type to the respective string used for
 *                  compilation, which means the basetype as string
 *                  followed by * in the case of arrays.
 *  global vars   : type_string
 *  remarks       :
 *
 *
 */

static char *
MakeTypeString (types *fulltype)
{
    char *ret;

    DBUG_ENTER ("MakeTypeString");

    if (TYPES_DIM (fulltype) != 0) {
        ret = (char *)Malloc (sizeof (char)
                              * (strlen (type_string[TYPES_BASETYPE (fulltype)]) + 3));
        strcpy (ret, type_string[TYPES_BASETYPE (fulltype)]);
        strcat (ret, " *");
    } else {
        ret = type_string[TYPES_BASETYPE (fulltype)];
    }

    DBUG_RETURN (ret);
}

/*
 *
 *  functionname  : MergeIcmsAp
 *  arguments     : 1) icm for out-parameter which is already situated
 *                     in the table
 *                  2) icm for in-parameter which was to be added to the
 *                     table when the mapping was detected.
 *                  3) type of parameter
 *                  4) refcount of variable
 *  description   :
 *  remarks       :
 *
 */

static node *
MergeIcmsAp (node *out_icm, node *in_icm, types *type, int rc)
{
    node *new_assign, *icm_arg;

    DBUG_ENTER ("MergeIcmsAp");

    if (IsBoxed (type)) {
        ICMPARAM_TAG (out_icm) = "upd_bx";

        if (IsArray (type)) {
            if (IsUnique (type)) {
                DBUG_PRINT ("COMP", ("Merging ICM-args: unique array %s - %s",
                                     ID_NAME (ICMPARAM_ARG1 (in_icm)),
                                     ID_NAME (ICMPARAM_ARG1 (out_icm))));

                if (0
                    != strcmp (ID_NAME (ICMPARAM_ARG1 (in_icm)),
                               ID_NAME (ICMPARAM_ARG1 (out_icm)))) {
                    CREATE_2_ARY_ICM (new_assign, "ND_KS_NO_RC_ASSIGN_ARRAY",
                                      MakeId1 (ID_NAME (ICMPARAM_ARG1 (in_icm))),
                                      MakeId1 (ID_NAME (ICMPARAM_ARG1 (out_icm))));
                } else {
                    new_assign = NULL;
                }
            } else if (rc == 1) {
                DBUG_PRINT ("COMP", ("Merging ICM-args non-unique array with rc==1"
                                     " %s - %s",
                                     ID_NAME (ICMPARAM_ARG1 (in_icm)),
                                     ID_NAME (ICMPARAM_ARG1 (out_icm))));

                CREATE_3_ARY_ICM (new_assign, "ND_KS_MAKE_UNIQUE_ARRAY",
                                  MakeId1 (ID_NAME (ICMPARAM_ARG1 (in_icm))),
                                  MakeId1 (ID_NAME (ICMPARAM_ARG1 (out_icm))),
                                  MakeNum (BasetypeSize (type)));
            } else {
                DBUG_PRINT ("COMP", ("Merging ICM-args non-unique array with rc>1"
                                     " %s - %s",
                                     ID_NAME (ICMPARAM_ARG1 (in_icm)),
                                     ID_NAME (ICMPARAM_ARG1 (out_icm))));

                CREATE_3_ARY_ICM (new_assign, "ND_KS_COPY_ARRAY",
                                  MakeId1 (ID_NAME (ICMPARAM_ARG1 (in_icm))),
                                  MakeId1 (ID_NAME (ICMPARAM_ARG1 (out_icm))),
                                  MakeNum (BasetypeSize (type)));
            }
        } else if (IsUnique (type)) {
            DBUG_PRINT ("COMP", ("Merging ICM-args unique hidden %s - %s",
                                 ID_NAME (ICMPARAM_ARG1 (in_icm)),
                                 ID_NAME (ICMPARAM_ARG1 (out_icm))));

            if (0
                != strcmp (ID_NAME (ICMPARAM_ARG1 (in_icm)),
                           ID_NAME (ICMPARAM_ARG1 (out_icm)))) {
                CREATE_2_ARY_ICM (new_assign, "ND_NO_RC_ASSIGN_HIDDEN",
                                  MakeId1 (ID_NAME (ICMPARAM_ARG1 (in_icm))),
                                  MakeId1 (ID_NAME (ICMPARAM_ARG1 (out_icm))));
            } else {
                new_assign = NULL;
            }
        } else if (rc == 1) {
            DBUG_PRINT ("COMP", ("Merging ICM-args non-unique hidden %s - %s"
                                 " with rc==1",
                                 ID_NAME (ICMPARAM_ARG1 (in_icm)),
                                 ID_NAME (ICMPARAM_ARG1 (out_icm))));

            CREATE_3_ARY_ICM (new_assign, "ND_MAKE_UNIQUE_HIDDEN",
                              MakeId1 (ID_NAME (ICMPARAM_ARG1 (in_icm))),
                              MakeId1 (ID_NAME (ICMPARAM_ARG1 (out_icm))),
                              MakeId1 (GenericFun (0, type)));
        } else {
            DBUG_PRINT ("COMP", ("Merging ICM-args non-unique hidden %s - %s"
                                 " with rc>1",
                                 ID_NAME (ICMPARAM_ARG1 (in_icm)),
                                 ID_NAME (ICMPARAM_ARG1 (out_icm))));

            CREATE_3_ARY_ICM (new_assign, "ND_KS_COPY_HIDDEN",
                              MakeId1 (ID_NAME (ICMPARAM_ARG1 (in_icm))),
                              MakeId1 (ID_NAME (ICMPARAM_ARG1 (out_icm))),
                              MakeId1 (GenericFun (0, type)));
        }
    } else {
        ICMPARAM_TAG (out_icm) = "upd";

        DBUG_PRINT ("COMP",
                    ("Merging ICM-args unboxed %s", ID_NAME (ICMPARAM_ARG1 (out_icm))));

        if ((NODE_TYPE (ICMPARAM_ARG1 (in_icm)) == N_id)
            && (0
                == strcmp (ID_NAME (ICMPARAM_ARG1 (in_icm)),
                           ID_NAME (ICMPARAM_ARG1 (out_icm))))) {
            new_assign = NULL;
        } else {
            new_assign = MakeAssign (MakeLet (ICMPARAM_ARG1 (in_icm),
                                              MakeIds (StringCopy (ID_NAME (
                                                         ICMPARAM_ARG1 (out_icm))),
                                                       NULL, ST_regular)),
                                     NULL);
        }
    }

    DBUG_PRINT ("COMP", ("Merging icm args of \"ND_FUN_AP\", new tag=\"%s\"",
                         ID_NAME (EXPRS_EXPR (out_icm))));

    DBUG_RETURN (new_assign);
}

/*
 *
 *  functionname  : MergeIcmsFundef
 *  description   :
 *  remarks       :
 *
 */

static void
MergeIcmsFundef (node *out_icm, node *in_icm, types *out_type, types *in_type, int line)
{
    DBUG_ENTER ("MergeIcmsFundef");

    if ((0 != strcmp ("out", ID_NAME (EXPRS_EXPR (out_icm))))
        || (0 != strcmp ("in", ID_NAME (EXPRS_EXPR (in_icm))))) {
        ERROR (line, ("Pragma 'linksign` illegal"));
        CONT_ERROR (("Mappings allowed exclusively between one parameter"
                     " and one return value on which both the function"
                     " does no refcounting !"));
        ABORT_ON_ERROR;
    }

    if (CMP_equal == CmpTypes (out_type, in_type)) {
        if (IsBoxed (out_type)) {
            ID_NAME (EXPRS_EXPR (out_icm)) = "upd_bx";
        } else {
            ID_NAME (EXPRS_EXPR (out_icm)) = "upd";
        }

        DBUG_PRINT ("COMP", ("Merging icm args of \"ND_FUN_DEC\", new tag=\"%s\"",
                             ID_NAME (EXPRS_EXPR (out_icm))));
    } else {
        ERROR (line, ("Pragma 'linksign` illegal"));
        CONT_ERROR (("Mappings allowed exclusively between parameters"
                     " with identical types !"));
        ABORT_ON_ERROR;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *ReorganizeReturnIcm(node *icm_arg)
 *
 * description:
 *
 *   The ND_FUN_DEC / ND_FUN_RET ICMs handle the first out-parameter of a
 *   function different from all others since this one is compiled to the
 *   original single return value of a C function. The decision actually
 *   is made when compiling the function definition. Because the function
 *   body is compiled before, the already existing ND_FUN_RET ICM has to
 *   be adjusted according to the ND_FUN_DEC ICM.
 *
 ******************************************************************************/

static node *
ReorganizeReturnIcm (node *icm_arg)
{
    node *tmp, *last;
    int first_out = 0, cnt = 0;

    DBUG_ENTER ("ReorganizeReturnIcm");

    if (icm_arg != NULL) {
        tmp = EXPRS_NEXT (EXPRS_NEXT (icm_arg));
        last = EXPRS_NEXT (icm_arg);

        while (tmp != NULL) {
            if ((!first_out) && (0 == strcmp ("out", ID_NAME (EXPRS_EXPR (tmp))))) {
                first_out = 1;
                ID_NAME (EXPRS_EXPR (icm_arg)) = ID_NAME (EXPRS_EXPR (EXPRS_NEXT (tmp)));
                tmp = EXPRS_NEXT (EXPRS_NEXT (tmp));
                EXPRS_NEXT (last) = tmp;
            } else {
                last = EXPRS_NEXT (EXPRS_NEXT (last));
                tmp = EXPRS_NEXT (EXPRS_NEXT (tmp));
                cnt++;
            }
        }
        NUM_VAL (EXPRS_EXPR (EXPRS_NEXT (icm_arg))) = cnt;
    }

    DBUG_RETURN (icm_arg);
}

/*
 *
 *  functionname  : CreateApIcm
 *  description   :
 *  remarks       : last let-node is reused for icm node
 *
 */

static node *
CreateApIcm (node *icm, char *name, node **icm_tab, int tab_size)
{
    int i, cnt_icm = 0;
    node *icm_arg, *tmp;

    DBUG_ENTER ("CreateApIcm");

    DBUG_PRINT ("COMP", ("Creating ICM \"ND_FUN_AP\""));

    NODE_TYPE (icm) = N_icm;
    ICM_NAME (icm) = "ND_FUN_AP";
    ICM_INDENT (icm) = 0;

    /* put function_name at beginning of ICM_ARGS */

    icm_arg = MakeExprs (MakeId1 (name), NULL);
    ICM_ARGS (icm) = icm_arg;

    if (icm_tab[1] == NULL) {
        MAKE_NEXT_ICM_ARG (icm_arg, MakeId1 (""));
    } else {
        APPEND_ICM_ARG (icm_arg, EXPRS_NEXT (icm_tab[1]));
    }

    /*
     *  Now, the number of icm arguments is counted.
     */

    for (i = 2; i < tab_size; i++) {
        if (icm_tab[i] != NULL) {
            cnt_icm++;
        }
    }

    tmp = icm_tab[0];

    while (tmp != NULL) {
        cnt_icm++;
        tmp = EXPRS_NEXT (EXPRS_NEXT (tmp));
    }

    /*
     *  The number od icm arguments is stored within the icm.
     */

    MAKE_NEXT_ICM_ARG (icm_arg, MakeNum (cnt_icm));

    /*
     *  The icm arguments are extracted from the table and concatenated
     *  to a list.
     */

    for (i = 2; i < tab_size; i++) {
        if (icm_tab[i] != NULL) {
            APPEND_ICM_ARG (icm_arg, icm_tab[i]);
            icm_arg = EXPRS_NEXT (icm_arg);
        }
    }

    if (icm_tab[0] != NULL) {
        APPEND_ICM_ARG (icm_arg, icm_tab[0]);
    }

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * function:
 *   node *CreateIcmND_FUN_DEC(char *name, node **icm_tab, int tab_size)
 *
 * description:
 *   creates a ND_FUN_DEC ICM, which has the following format:
 *     ND_FUN_DEC( name, rettype, narg, [TAG, type, arg]*)
 *   name provides the name of the function.
 *   icm_tab[0], icm_tab[0] != NULL      T_dots
 *   icm_tab[1]                          provides the rettype
 *   icm_tab[n], n>1, icm_tab[n] != NULL provides the arguments
 *                                       as icm_args: TAG, type arg
 *
 ******************************************************************************/

static node *
CreateIcmND_FUN_DEC (char *name, node **icm_tab, int tab_size)
{
    node *icm, *icm_arg;
    int i;
    int cnt_icm = 0;

    DBUG_ENTER ("CreateIcmND_FUN_DEC");

    DBUG_PRINT ("COMP", ("Creating ICM \"ND_FUN_DEC\""));

    icm = MakeIcm0 ("ND_FUN_DEC");

    /*
     *  First arg is name of function.
     *  The args will be builded from left to right.
     */
    icm_arg = MakeExprs (MakeId1 (name), NULL);
    ICM_ARGS (icm) = icm_arg;

    /*
     *  Second argument is returntype, void if none given.
     *  returntype is stored in icm_tab[1]
     */
    if (icm_tab[1] == NULL) {
        MAKE_NEXT_ICM_ARG (icm_arg, MakeId1 ("void"));
    } else {
        APPEND_ICM_ARG (icm_arg, EXPRS_NEXT (icm_tab[1]));
    }

    /*
     *  Count number of arguments
     *  including optional dots, but excluding returnvalue
     */
    for (i = 0; i < tab_size; i++) {
        /*
         *  ignore actual returnvalue and not given arguments
         */
        if ((i != 1) && (icm_tab[i] != NULL)) {
            cnt_icm++;
        }
    }

    /*
     *  Third argument is number of following args,
     *  where each arg willl contain 3 parts: tag, type and arg_name.
     */
    MAKE_NEXT_ICM_ARG (icm_arg, MakeNum (cnt_icm));

    /*
     *  From 2 on we find the arguments of the function
     *  we add tag, type and arg_name as icm_args
     */
    for (i = 2; i < tab_size; i++) {
        if (icm_tab[i] != NULL) {
            APPEND_ICM_ARG (icm_arg, icm_tab[i]);
            icm_arg = EXPRS_NEXT (EXPRS_NEXT (icm_arg));
        }
    }

    /*
     *  If there is an optional dots we add this one as the last arg.
     */
    if (icm_tab[0] != NULL) {
        APPEND_ICM_ARG (icm_arg, icm_tab[0]);
    }

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * function:
 *   node *CreateIcmMT_SPMD_FUN_DEC(char *name, char *from,
 *                                  node **icm_tab, int tab_size)
 *
 * description:
 *   creates a MT_SPMD_FUN_DEC ICM.
 *
 ******************************************************************************/

static node *
CreateIcmMT_SPMD_FUN_DEC (char *name, char *from, node **icm_tab, int tab_size)
{
    node *icm, *icm_arg;
    int i;
    int cnt_icm = 0;

    DBUG_ENTER ("CreateIcmMT_SPMD_FUN_DEC");

    DBUG_PRINT ("COMP", ("Creating ICM \"MT_SPMD_FUN_DEC\""));

    icm = MakeIcm0 ("MT_SPMD_FUN_DEC");

    icm_arg = MakeExprs (MakeId1 (name), NULL);
    ICM_ARGS (icm) = icm_arg;

    MAKE_NEXT_ICM_ARG (icm_arg, MakeId1 (from));

    for (i = 1; i < tab_size; i++) {
        if (icm_tab[i] != NULL) {
            cnt_icm++;
        }
    }

    MAKE_NEXT_ICM_ARG (icm_arg, MakeNum (cnt_icm));

    for (i = 1; i < tab_size; i++) {
        if (icm_tab[i] != NULL) {
            APPEND_ICM_ARG (icm_arg, icm_tab[i]);
            icm_arg = EXPRS_NEXT (EXPRS_NEXT (icm_arg));
        }
    }

    DBUG_RETURN (icm);
}

/*
 *
 *  functionname  : InsertApDotsParam
 *  description   :
 *  remarks       :
 *
 */

static void
InsertApDotsParam (node **icm_tab, node *icm_arg)
{
    DBUG_ENTER ("InsertApDotsParam");

    DBUG_PRINT ("COMP", ("Inserting ...-param in application"));

    icm_tab[0] = icm_arg;

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : InsertApArgParam
 *  description   :
 *  remarks       :
 *
 */

static void
InsertApArgParam (node **icm_tab, node *icm_arg, types *type, int rc,
                  node *collect_assigns, int *linksign, int cnt_param)
{
    node *new_assign = NULL;

    DBUG_ENTER ("InsertApArgParam");

    DBUG_PRINT ("COMP", ("Inserting arg #%d, tag=\"%s\"", cnt_param,
                         ID_NAME (EXPRS_EXPR (icm_arg))));

    if (linksign == NULL) {
        /*
         *  create standard icm table
         */
        icm_tab[cnt_param + 2] = icm_arg;
    } else {
        /*
         *  create special icm table depending on pragma liksign
         */

        if (icm_tab[linksign[cnt_param] + 1] == NULL) {
            icm_tab[linksign[cnt_param] + 1] = icm_arg;
        } else {
            new_assign
              = MergeIcmsAp (icm_tab[linksign[cnt_param] + 1], icm_arg, type, rc);

            if (new_assign != NULL) {
                ASSIGN_NEXT (new_assign) = ASSIGN_NEXT (collect_assigns);
                ASSIGN_NEXT (collect_assigns) = new_assign;
            }
        }
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : InsertApReturnParam
 *  description   :
 *  remarks       :
 *
 */

static void
InsertApReturnParam (node **icm_tab, node *icm_arg, types *type, int *linksign,
                     int cnt_param)
{
    DBUG_ENTER ("InsertApReturnParam");

    DBUG_PRINT ("COMP", ("Inserting arg #%d, tag=\"%s\"", cnt_param,
                         ID_NAME (EXPRS_EXPR (icm_arg))));

    if (linksign == NULL) {
        /*
         *  create standard icm table
         */

        if ((0 == strcmp (ID_NAME (EXPRS_EXPR (icm_arg)), "out"))
            && (icm_tab[1] == NULL)) {
            icm_tab[1] = icm_arg;
        } else {
            icm_tab[cnt_param + 2] = icm_arg;
        }
    } else {
        /*
         *  create special icm table depending on pragma liksign
         */

        icm_tab[linksign[cnt_param] + 1] = icm_arg;
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : InsertDefDotsParam
 *  description   :
 *  remarks       :
 *
 */

static void
InsertDefDotsParam (node **icm_tab, node *icm_arg)
{
    DBUG_ENTER ("InsertDefDotsParam");

    DBUG_PRINT ("COMP", ("Inserting ...-param in function definition"));

    icm_tab[0] = icm_arg;

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : InsertDefArgParam
 *  description   :
 *  remarks       :
 *
 */

static void
InsertDefArgParam (node **icm_tab, node *icm_arg, types **type_tab, types *type_arg,
                   int *linksign, int cnt_param, int line)
{
    DBUG_ENTER ("InsertDefArgParam");

    DBUG_PRINT ("COMP", ("Inserting arg #%d, tag=\"%s\"", cnt_param,
                         ID_NAME (EXPRS_EXPR (icm_arg))));

    if (linksign == NULL) {
        /*
         *  create standard icm table
         */
        icm_tab[cnt_param + 2] = icm_arg;
    } else {
        /*
         *  create special icm table depending on pragma liksign
         */

        if (linksign[cnt_param] == 0) {
            ERROR (line, ("Pragma 'linksign` illegal"));
            CONT_ERROR (("Argument parameter cannot be mapped to return position"));
            ABORT_ON_ERROR;
        }

        if (icm_tab[linksign[cnt_param] + 1] == NULL) {
            icm_tab[linksign[cnt_param] + 1] = icm_arg;
        } else {
            MergeIcmsFundef (icm_tab[linksign[cnt_param] + 1], icm_arg,
                             type_tab[linksign[cnt_param] + 1], type_arg, line);
        }
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : InsertDefReturnParam
 *  description   :
 *  remarks       :
 *
 */

static void
InsertDefReturnParam (node **icm_tab, node *icm_arg, types **type_tab, types *type_arg,
                      int *linksign, int cnt_param, int line)
{
    DBUG_ENTER ("InsertDefReturnParam");

    DBUG_PRINT ("COMP", ("Inserting arg #%d, tag=\"%s\"", cnt_param,
                         ID_NAME (EXPRS_EXPR (icm_arg))));

    if (linksign == NULL) {
        /*
         *  create standard icm table
         */

        if ((0 == strcmp (ID_NAME (EXPRS_EXPR (icm_arg)), "out"))
            && (icm_tab[1] == NULL)) {
            icm_tab[1] = icm_arg;
        } else {
            icm_tab[cnt_param + 2] = icm_arg;
        }
    } else {
        /*
         *  create special icm table depending on pragma liksign
         */

        if (icm_tab[linksign[cnt_param] + 1] == NULL) {
            icm_tab[linksign[cnt_param] + 1] = icm_arg;
            type_tab[linksign[cnt_param] + 1] = type_arg;
        } else {
            ERROR (line, ("Pragma 'linksign` illegal"));
            CONT_ERROR (("2 return parameters mapped to same position !"));
            ABORT_ON_ERROR;
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   char *RenameVar( char *string, int i)
 *
 * description:
 *   puts '__tmp' behind 'string'.
 *
 ******************************************************************************/

static char *
RenameVar (char *string, int i)
{
    char *new_name;

    DBUG_ENTER ("RenameVar");

    if (0 == i) {
        new_name = (char *)Malloc (sizeof (char) * (strlen (string) + 6));
        sprintf (new_name, "%s__tmp", string);
    } else {
        new_name = (char *)Malloc (sizeof (char) * (strlen (string) + 12));
        sprintf (new_name, "%s__tmp%d", string, i);
    }

    DBUG_RETURN (new_name);
}

/*
 *
 *  functionname  : RenameReturn
 *  arguments     : 1) N_return node
 *                  2) arg_info
 *  description   : - renames the variables in a return_statement,
 *                  - adds variable declaration
 *                  - inserts new assignments (after CURR_ASSIGN(arg_info))
 *  remarks       : - pointer to variable declaration is stored in
 *                    INFO_COMP_VARDECS(arg_info)
 *                  - returns N_let of last inserted new assign if
 *                     there renameing had to be done
 *                    returns N_return if no renaming had to be done
 *                  - puts new assignments after CURR_ASSIGN(arg_info).
 *                    node[0] of CURR_ASSIGN(arg_info) will be set in
 *                    last COMPAssign (return value of COMPReturn)) again
 *
 */
static node *
RenameReturn (node *return_node, node *arg_info)
{
    node *exprs, *tmp_exprs, *assign, *let, *next_assign, *vardec;
    int i;
    char *old_id, *new_id;

    DBUG_ENTER ("RenameReturn");

    exprs = RETURN_EXPRS (return_node);
    next_assign = MakeAssign (return_node, NULL);
    vardec = INFO_COMP_VARDECS (arg_info);

    while (NULL != exprs) {
        tmp_exprs = EXPRS_NEXT (exprs);
        i = 1;
        old_id = ID_NAME (EXPRS_EXPR (exprs));
        while (NULL != tmp_exprs) {
            if (0 == strcmp (ID_NAME (EXPRS_EXPR (tmp_exprs)), old_id)) {
                /* generates new nodes */
                new_id = RenameVar (old_id, i);
                let = MakeLet (MakeId (ID_NAME (EXPRS_EXPR (tmp_exprs)), NULL, 0),
                               MakeIds (new_id, NULL, 0));
                assign = MakeAssign (let, next_assign);
                next_assign = assign;
                vardec = AddVardec (vardec, ID_TYPE (EXPRS_EXPR (tmp_exprs)), new_id,
                                    INFO_COMP_FUNDEF (arg_info));

                /* rename variable in return-statement */
                ID_NAME (EXPRS_EXPR (tmp_exprs)) = StringCopy (new_id);
            }
            tmp_exprs = EXPRS_NEXT (tmp_exprs);
            i++;
        }
        exprs = EXPRS_NEXT (exprs);
    }

    if (ASSIGN_INSTR (next_assign) != return_node) {
        /* new nodes have been inserted */
        node *last_assign = CURR_ASSIGN (arg_info);
        ASSIGN_INSTR (last_assign) = ASSIGN_INSTR (assign);
        ASSIGN_NEXT (last_assign) = ASSIGN_NEXT (assign);
        INFO_COMP_VARDECS (arg_info) = vardec;

        return_node = ASSIGN_INSTR (assign);
    }

    DBUG_RETURN (return_node);
}

/*
 *
 *  functionname  : GenName
 *  arguments     : 1) number of variable
 *                  2) name
 *  description   : allocates memory for a new string , copies name to this
 *                  string and appends a number 1) to it
 *  remarks       : 1) <= 99
 *
 */
static char *
GenName (int i, char *name)
{
    char *new_name;

    DBUG_ENTER ("GenName");

    new_name = (char *)Malloc (sizeof (char) * (strlen (name) + 3));
    sprintf (new_name, "%s%d", name, i);

    DBUG_RETURN (new_name);
}

/*
 *
 *  functionname  : ShapeToArray
 *  arguments     : 1) N_vardec node
 *  description   : computes the shape of corresponding type and stores it
 *                  as N_exprs - chain
 *  remarks       : ----
 *
 */
static node *
ShapeToArray (node *vardec_node)
{
    node *ret_node = NULL, *tmp, *basic_type_node;
    int i;

    DBUG_ENTER ("ShapeToArray");
    if (T_user != TYPES_BASETYPE (VARDEC_TYPE (vardec_node))) {
        ret_node = MakeNode (N_exprs);
        EXPRS_EXPR (ret_node)
          = MakeNum (SHPSEG_SHAPE (TYPES_SHPSEG (VARDEC_TYPE (vardec_node)), 0));
        tmp = ret_node;
        for (i = 1; i < TYPES_DIM (VARDEC_TYPE (vardec_node)); i++) {
            EXPRS_NEXT (tmp) = MakeNode (N_exprs);
            EXPRS_EXPR (EXPRS_NEXT (tmp))
              = MakeNum (SHPSEG_SHAPE (TYPES_SHPSEG (VARDEC_TYPE (vardec_node)), i));
            tmp = EXPRS_NEXT (tmp);
        }
    } else {
        basic_type_node = TYPES_TDEF (VARDEC_TYPE (vardec_node));
        DBUG_ASSERT ((basic_type_node != NULL), "Failed attempt to look up typedef");

        if (1 <= TYPES_DIM (VARDEC_TYPE (vardec_node))) {
            ret_node = MakeNode (N_exprs);
            EXPRS_EXPR (ret_node)
              = MakeNum (SHPSEG_SHAPE (TYPES_SHPSEG (VARDEC_TYPE (vardec_node)), 0));
            tmp = ret_node;
            for (i = 1; i < TYPES_DIM (VARDEC_TYPE (vardec_node)); i++) {
                EXPRS_NEXT (tmp) = MakeNode (N_exprs);

                EXPRS_EXPR (EXPRS_NEXT (tmp))
                  = MakeNum (SHPSEG_SHAPE (TYPES_SHPSEG (VARDEC_TYPE (vardec_node)), i));
                tmp = EXPRS_NEXT (tmp);
            }
            for (i = 0; i < TYPES_DIM (VARDEC_TYPE (basic_type_node)); i++) {
                EXPRS_NEXT (tmp) = MakeNode (N_exprs);
                EXPRS_EXPR (EXPRS_NEXT (tmp)) = MakeNum (
                  SHPSEG_SHAPE (TYPES_SHPSEG (VARDEC_TYPE (basic_type_node)), i));
                tmp = EXPRS_NEXT (tmp);
            }
        } else {
            ret_node = MakeNode (N_exprs);
            EXPRS_EXPR (ret_node)
              = MakeNum (SHPSEG_SHAPE (TYPES_SHPSEG (VARDEC_TYPE (basic_type_node)), 0));
            tmp = ret_node;
            for (i = 1; i < TYPES_DIM (VARDEC_TYPE (basic_type_node)); i++) {
                EXPRS_NEXT (tmp) = MakeNode (N_exprs);
                EXPRS_EXPR (EXPRS_NEXT (tmp)) = MakeNum (
                  SHPSEG_SHAPE (TYPES_SHPSEG (VARDEC_TYPE (basic_type_node)), i));
                tmp = EXPRS_NEXT (tmp);
            }
        }
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * function:
 *   node *Compile( node *arg_node)
 *
 * description:
 *   starts compilation.
 *
 ******************************************************************************/

node *
Compile (node *arg_node)
{
    node *info;

    DBUG_ENTER ("Compile");

    act_tab = comp_tab;
    info = MakeInfo ();

    arg_node = Trav (arg_node, info);

    FREE (info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPModul( node *arg_node, node *arg_info)
 *
 * description:
 *   compiles an N_modul node:
 *     - traverses sons.
 *
 ******************************************************************************/

node *
COMPModul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("COMPModul");

    INFO_COMP_MODUL (arg_info) = arg_node;

    if (MODUL_OBJS (arg_node) != NULL) {
        MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), arg_info);
    }

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    if (MODUL_TYPES (arg_node) != NULL) {
        MODUL_TYPES (arg_node) = Trav (MODUL_TYPES (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPFundef( node *arg_node, node *arg_info)
 *
 * description:
 *   Compiles a N_fundef node.
 *
 ******************************************************************************/

node *
COMPFundef (node *arg_node, node *arg_info)
{
    node *return_node, *return_icm, *icm_arg, *type_id_node, *var_name_node, *tag_node,
      *icm_tab_entry, *old_fundef, **icm_tab;
    types *rettypes, *fulltype, **type_tab;
    statustype old_actualattrib;
    int cnt_param, tab_size, i;

    DBUG_ENTER ("COMPFundef");

    DBUG_PRINT ("COMP", ("compiling %s", FUNDEF_NAME (arg_node)));

    DBUG_PRINT ("COMPjhs", ("compiling %s attrib: %s status: %s", FUNDEF_NAME (arg_node),
                            mdb_statustype[FUNDEF_ATTRIB (arg_node)],
                            mdb_statustype[FUNDEF_STATUS (arg_node)]));

    /*
     *  "push arg_info"
     *
     *  informations changed at arg_info are saved before,
     *  INFO_COMP_ACTUALATTRIB will be used while compiling mt2 stuff
     */
    old_fundef = INFO_COMP_FUNDEF (arg_info);
    old_actualattrib = INFO_COMP_ACTUALATTRIB (arg_info);
    INFO_COMP_FUNDEF (arg_info) = arg_node;
    INFO_COMP_ACTUALATTRIB (arg_info) = FUNDEF_ATTRIB (arg_node);

    /********** begin: traverse body **********/

    /*
     * During compilation of a N_sync, the prioir N_sync (if exists) is needed.
     * INFO_COMP_LAST_SYNC provides these information, it is initialized here with
     * NULL and will be updated by each compilation of a N_sync (one needs to
     * compile them ordered!), this includes the destruction of such an N_sync-tree.
     * After compilation of the function the last known sync is destroyed then.
     */
    INFO_COMP_LAST_SYNC (arg_info) = NULL;

    /*
     * INFO_COMP_VARDECS points to the vardecs of the current fundef (*not* block!!)
     * Nevertheless the VARDECs are a son of the N_block-node therefore
     * INFO_COMP_VARDECS is set and evaluated in COMPBlock().
     * Here, this pointer is just initialized with NULL.
     */
    INFO_COMP_VARDECS (arg_info) = NULL;

    /*
     * traverse body
     */
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    /*
     *  Destruction of last known N_sync is done here, all others have be killed
     *  while traversing.
     */
    if (INFO_COMP_LAST_SYNC (arg_info) != NULL) {
        INFO_COMP_LAST_SYNC (arg_info) = FreeTree (INFO_COMP_LAST_SYNC (arg_info));
        INFO_COMP_LAST_SYNC (arg_info) = NULL;
    }

    /********** end: traverse body **********/

    /*
     * compile return types
     */
    rettypes = FUNDEF_TYPES (arg_node);

    if ((NULL != FUNDEF_RETURN (arg_node)) && (TYPES_BASETYPE (rettypes) != T_void)) {
        /*
         * FUNDEF_RETURN(arg_node) points to a N_icm (ND_FUN_RET, MT_SPMD_FUN_RET).
         * 'return_node' will point to the first N_exprs belonging to a return_value.
         * This exists only for functions with at least one return value.
         */

        return_icm = FUNDEF_RETURN (arg_node);
        DBUG_ASSERT ((NODE_TYPE (return_icm) == N_icm), "no ICM found");

        if (strcmp (ICM_NAME (return_icm), "ND_FUN_RET") == 0) {
            return_node = EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (ICM_ARGS (return_icm))));
        } else if (strcmp (ICM_NAME (return_icm), "MT_SPMD_FUN_RET") == 0) {
            return_node = EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (ICM_ARGS (return_icm))));
        } else {
            DBUG_ASSERT ((0), "wrong ICM found");
        }
    }

    cnt_param = 0;

    tab_size = CountFunctionParams (arg_node) + 2;
    icm_tab = (node **)Malloc (sizeof (node *) * tab_size);
    type_tab = (types **)Malloc (sizeof (types *) * tab_size);

    for (i = 0; i < tab_size; i++) {
        icm_tab[i] = NULL;
        type_tab[i] = NULL;
    }

    while ((NULL != rettypes) && (TYPES_BASETYPE (rettypes) != T_void)
           && (TYPES_BASETYPE (rettypes) != T_dots)) {

        if ((MUST_REFCOUNT (TYPE, rettypes))
            && (FUN_DOES_REFCOUNT (arg_node, cnt_param))) {
            tag_node = MakeId1 ("out_rc");
        } else {
            tag_node = MakeId1 ("out");
        }

        icm_arg = MakeExprs (tag_node, NULL);
        icm_tab_entry = icm_arg;

        GET_BASIC_TYPE (fulltype, rettypes, 042);

        type_id_node = MakeId1 (MakeTypeString (fulltype));
        MAKE_NEXT_ICM_ARG (icm_arg, type_id_node);

        if (NULL == FUNDEF_BODY (arg_node)) {
            /* it is an extern declaration */
#ifdef IMPORTED_WITH_NAME
            var_name_node = MakeId1 (GenName (i, DUMMY_NAME));
#else
            var_name_node = MakeId1 ("");
#endif

        } else {
            DBUG_ASSERT ((return_node != NULL), "no return icm found");
            DBUG_ASSERT ((N_id == NODE_TYPE (EXPRS_EXPR (return_node))),
                         "wrong nodetype != N_id");
            var_name_node = MakeId3 (LET_IDS (EXPRS_EXPR (return_node)));
            if (EXPRS_NEXT (return_node) != NULL) {
                /*
                 * put return_node to next N_exprs where a function return_value
                 * is behind
                 */
                return_node = EXPRS_NEXT (EXPRS_NEXT (return_node));
            }
        }

        MAKE_NEXT_ICM_ARG (icm_arg, var_name_node);

        InsertDefReturnParam (icm_tab, icm_tab_entry, type_tab, rettypes,
                              (FUNDEF_PRAGMA (arg_node) == NULL)
                                ? NULL
                                : FUNDEF_LINKSIGN (arg_node),
                              cnt_param, NODE_LINE (arg_node));

        rettypes = TYPES_NEXT (rettypes);
        cnt_param++;
    } /* while */

    if ((rettypes != NULL) && (TYPES_BASETYPE (rettypes) == T_dots)) {
        tag_node = MakeId1 ("in");
        icm_arg = MakeExprs (tag_node, NULL);
        icm_tab_entry = icm_arg;
        type_id_node = MakeId1 (MakeTypeString (rettypes));
        MAKE_NEXT_ICM_ARG (icm_arg, type_id_node);
        var_name_node = MakeId1 ("");
        MAKE_NEXT_ICM_ARG (icm_arg, var_name_node);

        InsertDefDotsParam (icm_tab, icm_tab_entry);
    }

    if (NULL != FUNDEF_ARGS (arg_node)) {
        if (NULL != FUNDEF_BODY (arg_node)) {
            /* first assign of body */
            INFO_COMP_FIRSTASSIGN (arg_info) = BLOCK_INSTR (FUNDEF_BODY (arg_node));
        }

        INFO_COMP_CNTPARAM (arg_info) = (cnt_param == 0) ? 1 : cnt_param;
        INFO_COMP_ICMTAB (arg_info) = icm_tab;
        INFO_COMP_TYPETAB (arg_info) = type_tab;

        /*
         *  the arg_node is needed while compiling args as argument for
         *  FUN_DOES_REFCOUNT
         */

        /* traverse formal parameters (N_arg) */
        Trav (FUNDEF_ARGS (arg_node), arg_info);

        if (NULL != FUNDEF_BODY (arg_node)) {
            /* new first assign of body */
            BLOCK_INSTR (FUNDEF_BODY (arg_node)) = INFO_COMP_FIRSTASSIGN (arg_info);
            INFO_COMP_FIRSTASSIGN (arg_info) = NULL;
        }

        INFO_COMP_CNTPARAM (arg_info) = 0;
        INFO_COMP_ICMTAB (arg_info) = NULL;
        INFO_COMP_TYPETAB (arg_info) = NULL;
    }

    if ((FUNDEF_RETURN (arg_node) != NULL)
        && (ICM_ARGS (FUNDEF_RETURN (arg_node)) != NULL)
        && (FUNDEF_STATUS (arg_node) != ST_spmdfun)) {
        ReorganizeReturnIcm (ICM_ARGS (FUNDEF_RETURN (arg_node)));
    }

    if (FUNDEF_STATUS (arg_node) == ST_spmdfun) {
        FUNDEF_ICM (arg_node)
          = CreateIcmMT_SPMD_FUN_DEC (FUNDEF_NAME (arg_node),
                                      FUNDEF_NAME (FUNDEF_LIFTEDFROM (arg_node)), icm_tab,
                                      tab_size);
    } else {
        FUNDEF_ICM (arg_node)
          = CreateIcmND_FUN_DEC (FUNDEF_NAME (arg_node), icm_tab, tab_size);
    }

    /*
     * From now on FUNDEF_RETURN(fundef) points to N_icm instead of function's
     * return-statement.
     */

    FREE (icm_tab);
    FREE (type_tab);

    /*
     * traverse next function if any
     */
    if (NULL != FUNDEF_NEXT (arg_node)) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    /*
     * remove pragma
     */
    if (FUNDEF_PRAGMA (arg_node) != NULL) {
        FUNDEF_PRAGMA (arg_node) = FreeNode (FUNDEF_PRAGMA (arg_node));
    }

    /*
     * Remove fold-function from list of functions.
     * However, it cannot be freed since it is still needed by several ICM
     * implementations. Therefore, all fold-functions are stored in a special
     * fundef-chain fixed to the N_modul node. So, they still exist, but
     * won't be printed.
     */

    if (FUNDEF_STATUS (arg_node) == ST_foldfun) {
        node *tmp;

        tmp = FUNDEF_NEXT (arg_node);

        FUNDEF_NEXT (arg_node) = MODUL_FOLDFUNS (INFO_COMP_MODUL (arg_info));
        MODUL_FOLDFUNS (INFO_COMP_MODUL (arg_info)) = arg_node;

        arg_node = tmp;
    }

    /* pop arg_info */
    INFO_COMP_FUNDEF (arg_info) = old_fundef;
    INFO_COMP_ACTUALATTRIB (arg_info) = old_actualattrib;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPBlock( node *arg_node, node *arg_info)
 *
 * description:
 *   Stacks INFO_COMP_LASTASSIGN and sets it to the current node while
 *   traversal.
 *
 ******************************************************************************/

node *
COMPBlock (node *arg_node, node *arg_info)
{
    node *old_lastassign, *assign;
    char *fun_name, *cs_tag;

    DBUG_ENTER ("COMPBlock");

    /* stacking of old info! (nested blocks) */
    old_lastassign = INFO_COMP_LASTASSIGN (arg_info);

    if (BLOCK_CACHESIM (arg_node) != NULL) {
        fun_name = FUNDEF_NAME (INFO_COMP_FUNDEF (arg_info));
        cs_tag
          = (char *)Malloc (strlen (BLOCK_CACHESIM (arg_node)) + strlen (fun_name) + 14);
        if (BLOCK_CACHESIM (arg_node)[0] == '\0') {
            sprintf (cs_tag, "\"%s(...)\"", fun_name);
        } else {
            sprintf (cs_tag, "\"%s in %s(...)\"", BLOCK_CACHESIM (arg_node), fun_name);
        }

        FREE (BLOCK_CACHESIM (arg_node));

        DBUG_ASSERT ((BLOCK_INSTR (arg_node) != NULL),
                     "first instruction of block is NULL (should be a N_empty node)");
        assign = BLOCK_INSTR (arg_node);

        BLOCK_INSTR (arg_node)
          = MakeAssign (MakeIcm1 ("CS_START", MakeId1 (cs_tag)), BLOCK_INSTR (arg_node));

        while ((ASSIGN_NEXT (assign) != NULL)
               && (NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (assign))) != N_return)) {
            assign = ASSIGN_NEXT (assign);
        }

        ASSIGN_NEXT (assign)
          = MakeAssign (MakeIcm1 ("CS_STOP", MakeId1 (cs_tag)), ASSIGN_NEXT (assign));
    }

    INFO_COMP_LASTASSIGN (arg_info) = arg_node;

    if (FUNDEF_BODY (INFO_COMP_FUNDEF (arg_info)) == arg_node) {
        /* this block is the body of a function definition */

        DBUG_ASSERT ((INFO_COMP_VARDECS (arg_info) == NULL),
                     "INFO_COMP_VARDECS is not initialized!");

        INFO_COMP_VARDECS (arg_info) = BLOCK_VARDEC (arg_node);
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
        /*
         * update vardecs
         * (insert new vardecs generated during compilation of the body)
         */
        BLOCK_VARDEC (arg_node) = INFO_COMP_VARDECS (arg_info);

        /*
         * we must compile the vardecs last, because we need the uncompiled vardecs
         *  during traversal of the block.
         *
         * vardecs are not compiled for "foldfuns" because these vardecs are later
         * on inserted into the vardec chains of the function each foldfun belongs
         * to, i.e., they are compiled when this function is compiled.
         */
        if ((BLOCK_VARDEC (arg_node) != NULL)
            && (NODE_TYPE (BLOCK_VARDEC (arg_node)) == N_vardec)
            /*
             * Note: NODE_TYPE( BLOCK_VARDEC( arg_node)) might be N_icm !!
             */
            && (FUNDEF_STATUS (INFO_COMP_FUNDEF (arg_info)) != ST_foldfun)) {
            BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
        }
    } else {
        /* this block is *not* the body of a function definition */

        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);

        DBUG_ASSERT ((BLOCK_VARDEC (arg_node) == NULL), "local vardecs found!");
    }

    /* restoring old info! (nested blocks) */
    INFO_COMP_LASTASSIGN (arg_info) = old_lastassign;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPAssign( node *arg_node, node *arg_info)
 *
 * description:
 *   Compiles a N_assign node.
 *
 * remarks:
 *   - INFO_COMP_LASTASSIGN(arg_info) contains a pointer to the last assigment
 *     (predecessor of 'arg_node').
 *     During traversal of ASSIGN_INSTR(arg_node) via this pointer some ICMs
 *     are inserted before *and* after 'arg_node'!
 *     Because this is a very ugly behaviour, newer routines (new with-loop/
 *     SPMD) *return* an assign-chain. In this case 'COMPAssign' inserts this
 *     assignments into the syntax-tree!
 *     With this mechanismus it is not neccessary to manipulate remote parts
 *     of the tree via 'arg_info'.
 *
 ******************************************************************************/

node *
COMPAssign (node *arg_node, node *arg_info)
{
    node *old_next_assign;
    node *old_last_assign;
    node *instr;

    DBUG_ENTER ("COMPAssign");

    old_last_assign = INFO_COMP_LASTASSIGN (arg_info);
    old_next_assign = ASSIGN_NEXT (arg_node);

    instr = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (NODE_TYPE (instr) == N_assign) {
        /*
         * an assignment-chain was returned.
         *  => insert it at the current position into the tree.
         */
        ASSIGN_INSTR (arg_node) = ASSIGN_INSTR (instr);
        ASSIGN_INSTR (instr) = NULL;
        instr = FreeNode (instr);
        ASSIGN_NEXT (arg_node) = AppendAssign (instr, ASSIGN_NEXT (arg_node));
    } else {
        ASSIGN_INSTR (arg_node) = instr;
    }

    if (old_next_assign != NULL) {
        INFO_COMP_LASTASSIGN (arg_info) = arg_node;
        /*
         * Now, we skip all those assigns that were inserted during the
         * traversal of 'ASSIGN_INSTR(arg_node)' !!
         */
        while (old_next_assign != ASSIGN_NEXT (INFO_COMP_LASTASSIGN (arg_info))) {
            INFO_COMP_LASTASSIGN (arg_info)
              = ASSIGN_NEXT (INFO_COMP_LASTASSIGN (arg_info));
        }

        old_next_assign = Trav (old_next_assign, arg_info);
    }

    /*
     * although the new assigns are already inserted correctly into
     * the chain of assignments, we have to return the correct pointer,
     * since the normal insertion mechanism probably is used !!!
     */
    arg_node = BLOCK_INSTR_OR_ASSIGN_NEXT (old_last_assign);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPLet(node *arg_node, node *arg_info)
 *
 * description:
 *   Compiles a N_let node.
 *
 * remarks:
 *   - In LET_EXPR() many routines use INFO_COMP_LASTLET to recycle the let
 *     node as an ICM, if necessary.
 *     Because this is a very ugly behaviour, newer routines (new with-loop/
 *     SPMD) *return* an assign-chain. In this case 'COMPLet' frees the old
 *     'arg_node' and returns the assign-chain to 'COMPAssign', where this
 *     assignments are inserted into the syntax-tree! (see 'COMPAssign').
 *     With this mechanismus it is not neccessary to manipulate remote parts
 *     of the tree via 'arg_info'.
 *
 ******************************************************************************/

node *
COMPLet (node *arg_node, node *arg_info)
{
    node *expr;

    DBUG_ENTER ("COMPLet");

    INFO_COMP_LASTLET (arg_info) = arg_node;
    INFO_COMP_LASTIDS (arg_info) = LET_IDS (arg_node);

    if (LET_IDS (arg_node) != NULL) {
        DBUG_PRINT ("COMP", ("LHS of let-assignment: %s", LET_NAME (arg_node)));
    }

    expr = Trav (LET_EXPR (arg_node), arg_info);

    if ((NODE_TYPE (expr) == N_assign) && (NODE_TYPE (arg_node) == N_let)) {

#ifdef TAGGED_ARRAYS
        /*
         * CAUTION: Some old routines recycles the let-node as a N_block node!!!
         *          (e.g. COMPprf when inserting 'ND_DECL_AKS'-ICMs ...)
         *          In these cases we must not free 'arg_node'!!!
         */

#else  /* TAGGED_ARRAYS */

        /*
         * CAUTION: Some old routines recycles the let-node as a N_block node!!!
         *          (e.g. COMPprf when inserting 'ND_KS_DECL_ARRAY'-ICMs ...)
         *          In these cases we must not free 'arg_node'!!!
         */
#endif /* TAGGED_ARRAYS */

        LET_EXPR (arg_node) = NULL;
        arg_node = FreeTree (arg_node);
        arg_node = expr;
    } else {
        LET_EXPR (arg_node) = expr;
    }

    INFO_COMP_LASTLET (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPVardec(node *arg_node, node *arg_info)
 *
 * description:
 *   transforms N_vardec to N_icm node if it is the declaration of an array
 *
 * remarks:
 *   If it is a declaration of an array, a N_assign node will be inserted to
 *   get the new N_icm node into the chain of N_vardecs.
 *
 *   Before traversing the next vardec node, we must test, whether this node
 *   is compiled already or not (NODE_TYPE != N_vardec).
 *   'COMPNwith2()' possibly have inserted compiled vardecs of the dummy
 *   fold-fun!!!!
 *
 ******************************************************************************/

node *
COMPVardec (node *arg_node, node *arg_info)
{
    node *assign, *id_node, *n_dim, *id_type, *icm_arg;
#ifdef TAGGED_ARRAYS
    node *shape_elems;
    char *macvalues[] = {"ND_DECL_AKS", "ND_DECL_SCL"};
    char *mymac;

#else /* TAGGED_ARRAYS */

#endif /* TAGGED_ARRAYS */
    int i;

    types *full_type;

    DBUG_ENTER ("COMPVardec");

    GET_BASIC_TYPE (full_type, VARDEC_TYPE (arg_node), 0);

#ifdef TAGGED_ARRAYS
    if (TYPES_DIM (full_type) >= SCALAR) {
        /*
         * variable is an array with known shape or a scalar.
         */

        id_type = MakeId1 (type_string[TYPES_BASETYPE (full_type)]);
        /* declared type */
        id_node = MakeId1 (VARDEC_NAME (arg_node)); /* name of variable */
        n_dim = MakeNum (TYPES_DIM (full_type));

        /* now create N_icm */
        /*
         * First, we build-up an N_exprs-list containing the shape-vector.
         */
        shape_elems = NULL; /* Handle scalar case */
        for (i = TYPES_DIM (full_type) - 1; i >= 0; i--) {
            shape_elems = MakeExprs (MakeNum (TYPES_SHAPE (full_type, i)), shape_elems);
        }
        /*
         * Now, we can build up the ND_DECL_AKS-icm using shape_elems
         * as last arguments...
         */
        if (TYPES_DIM (full_type) == SCALAR)
            mymac = macvalues[1];
        else
            mymac = macvalues[0];
        assign = MakeAssign (MakeIcm4 (mymac, id_type, id_node, n_dim, shape_elems),
                             VARDEC_NEXT (arg_node));

#else  /* TAGGED_ARRAYS */
    if (TYPES_DIM (full_type) > SCALAR) {
        /*
         * full_type is an array with known shape.
         */

        id_type = MakeId1 (type_string[TYPES_BASETYPE (full_type)]);
        /* declared type */
        id_node = MakeId1 (VARDEC_NAME (arg_node)); /* name of variable */
        n_dim = MakeNum (TYPES_DIM (full_type));

        /* now create N_icm */
        assign = MakeAssign (MakeIcm0 ("ND_KS_DECL_ARRAY"), VARDEC_NEXT (arg_node));

        ICM_ARGS (ASSIGN_INSTR (assign)) = MakeExprs (id_type, NULL);
        icm_arg = ICM_ARGS (ASSIGN_INSTR (assign));
        MAKE_NEXT_ICM_ARG (icm_arg, id_node);
        MAKE_NEXT_ICM_ARG (icm_arg, n_dim);

        for (i = 0; i < TYPES_DIM (full_type); i++) {
            /* the shape information will be converted & added */
            MAKE_NEXT_ICM_ARG (icm_arg, MakeNum (full_type->shpseg->shp[i]));
        }
#endif /* TAGGED_ARRAYS */

        /* now free some nodes */
        if (T_user == TYPES_BASETYPE (VARDEC_TYPE (arg_node))) {
            FREE_TYPE (full_type);
        }

        FREE_VARDEC (arg_node);
        arg_node = assign; /* set arg_node, because this node will be returned */

        if ((NULL != ASSIGN_NEXT (arg_node))
            && (NODE_TYPE (ASSIGN_NEXT (arg_node)) == N_vardec)) {
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), NULL);
            /* dkr: Trav(...) with (arg_info == NULL) !?!? */
        }
    } else if (TYPES_DIM (full_type) == UNKNOWN_SHAPE) {
        /*
         *  full_type is an array with unknown shape and dimension.
         */

        id_type = MakeId1 (type_string[TYPES_BASETYPE (full_type)]);
        /* declared type */
        id_node = MakeId1 (VARDEC_NAME (arg_node)); /* name of variable */

        /* now create N_icm */
        assign = MakeAssign (MakeIcm0 ("ND_DECL_ARRAY"), VARDEC_NEXT (arg_node));

        ICM_ARGS (ASSIGN_INSTR (assign)) = MakeExprs (id_type, NULL);
        icm_arg = ICM_ARGS (ASSIGN_INSTR (assign));
        MAKE_NEXT_ICM_ARG (icm_arg, id_node);

        /* now free some nodes */
        if (T_user == TYPES_BASETYPE (VARDEC_TYPE (arg_node))) {
            FREE_TYPE (full_type);
        }

        FREE_VARDEC (arg_node);
        arg_node = assign; /* set arg_node, because this node will be returned */

        if ((NULL != ASSIGN_NEXT (arg_node))
            && (NODE_TYPE (ASSIGN_NEXT (arg_node)) == N_vardec)) {
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), NULL);
            /* dkr: Trav(...) with (arg_info == NULL) !?!? */
        }
    } else if (TYPES_DIM (full_type) < KNOWN_DIM_OFFSET) {
        /*
         *  full_type is an array with unknown shape and known dimension.
         */

        id_type = MakeId1 (type_string[TYPES_BASETYPE (full_type)]);
        /* declared type */
        id_node = MakeId1 (VARDEC_NAME (arg_node)); /* name of variable */
        n_dim = MakeNum (KNOWN_DIM_OFFSET - TYPES_DIM (full_type));

        /* now create N_icm */
        assign = MakeAssign (MakeIcm0 ("ND_KD_DECL_ARRAY"), VARDEC_NEXT (arg_node));

        ICM_ARGS (ASSIGN_INSTR (assign)) = MakeExprs (id_type, NULL);
        icm_arg = ICM_ARGS (ASSIGN_INSTR (assign));
        MAKE_NEXT_ICM_ARG (icm_arg, id_node);
        MAKE_NEXT_ICM_ARG (icm_arg, n_dim);

        /* now free some nodes */
        if (T_user == TYPES_BASETYPE (VARDEC_TYPE (arg_node))) {
            FREE_TYPE (full_type);
        }

        FREE_VARDEC (arg_node);
        arg_node = assign; /* set arg_node, because this node will be returned */

        if ((NULL != ASSIGN_NEXT (arg_node))
            && (NODE_TYPE (ASSIGN_NEXT (arg_node)) == N_vardec)) {
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), NULL);
            /* dkr: Trav(...) with (arg_info == NULL) !?!? */
        }
    } else if (IsNonUniqueHidden (VARDEC_TYPE (arg_node))) {
        CREATE_2_ARY_ICM (assign, "ND_DECL_RC", MakeId1 ("void*"),
                          MakeId1 (VARDEC_NAME (arg_node)));

        if ((VARDEC_NEXT (arg_node) != NULL)
            && (NODE_TYPE (VARDEC_NEXT (arg_node))) == N_vardec) {
            ASSIGN_NEXT (assign) = Trav (VARDEC_NEXT (arg_node), NULL);
            /* dkr: Trav(...) with (arg_info == NULL) !?!? */
        }
        FREE_VARDEC (arg_node);
        arg_node = assign;
    } else if (TYPES_DIM (VARDEC_TYPE (arg_node)) < 0) {
        /* current vardec-node has unknown shape and will be removed */
        node *tmp;
        tmp = arg_node;
        if ((VARDEC_NEXT (arg_node) != NULL)
            && (NODE_TYPE (VARDEC_NEXT (arg_node))) == N_vardec) {
            arg_node = Trav (VARDEC_NEXT (arg_node), NULL);
            /* dkr: Trav(...) with (arg_info == NULL) !?!? */
        } else {
            arg_node = NULL;
        }
        FREE_VARDEC (tmp);
    } else if ((VARDEC_NEXT (arg_node) != NULL)
               && (NODE_TYPE (VARDEC_NEXT (arg_node))) == N_vardec) {
        /* traverse next N_vardec node if any */
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), NULL);
        /* dkr: Trav(...) with (arg_info == NULL) !?!? */
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPPrfModarray
 *  arguments     : 1) N_prf node
 *                  2)
 *  description   : transforms N_prf node F_modarray to N_icm nodes
 *  remarks       : INFO_COMP_LASTIDS(arg_info) contains name of assigned variable
 *                  INFO_COMP_LASTASSIGN(arg_info) contains pointer to node before
 *                   last assign_node (to get last or next assign node use
 *                                     macros LAST_ASSIGN or NEXT_ASSIGN)
 *                  INFO_COMP_LASTASSIGN(arg_info) is used to insert new assign_nodes
 *                   in front of or after last assign node
 *                  INFO_COMP_LASTLET(arg_info) contains pointer to last N_let
 */

node *
COMPPrfModarray (node *arg_node, node *arg_info)
{
    node *res, *length, *res_ref, *type_id_node, *dim_res, *line, *first_assign,
      *next_assign, *icm_arg, *old_arg_node, *last_assign;
    node *arg1 = PRF_ARG1 (arg_node);
    node *arg2 = PRF_ARG2 (arg_node);
    node *arg3 = PRF_ARG3 (arg_node);
    int n, dim;
    simpletype s_type;
    char *icm_name;

    DBUG_ENTER ("COMPPrfModarray");

    res = MakeId3 (INFO_COMP_LASTIDS (arg_info));

    /* compute basic_type of result */
    GET_BASIC_SIMPLETYPE (s_type,
                          VARDEC_TYPE (IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
    type_id_node = MakeId1 (type_string[s_type]);

    /* store dimension of result */
    GET_DIM (dim, VARDEC_TYPE (IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
    dim_res = MakeNum (dim);

    /* store refcount of res as N_num */
    res_ref = MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

    /* store line of prf function */
    line = MakeNum (NODE_LINE (arg_node));

    if (NODE_TYPE (arg2) == N_array) {
        /* index is constant! */
        if (NODE_TYPE (arg3) == N_array) {
            /* value is constant! */
            DBUG_ASSERT (0, "sorry compilation of ND_PRF_MODARRAY_AxCxC not yet done");
        } else {
            COUNT_ELEMS (n, ARRAY_AELEMS (arg2));
            length = MakeNum (n);

            if ((N_id == NODE_TYPE (arg3))
                && (1 == IsArray (VARDEC_TYPE (ID_VARDEC (arg3))))) {
                if (1 == arg1->refcnt) {
                    icm_name = "ND_PRF_MODARRAY_AxCxA_CHECK_REUSE";
                } else {
                    icm_name = "ND_PRF_MODARRAY_AxCxA";
                }

                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), icm_name, line,
                               type_id_node);
                MAKE_NEXT_ICM_ARG (icm_arg, dim_res);
                MAKE_NEXT_ICM_ARG (icm_arg, res);
                MAKE_NEXT_ICM_ARG (icm_arg, arg1);
                MAKE_NEXT_ICM_ARG (icm_arg, arg3);
                MAKE_NEXT_ICM_ARG (icm_arg, length);
                icm_arg->node[1] = ARRAY_AELEMS (arg2);
                SET_VARS_FOR_MORE_ICMS;
                DEC_OR_FREE_RC_ND (arg3, MakeNum (1));
            } else {
                if (1 == arg1->refcnt) {
                    icm_name = "ND_PRF_MODARRAY_AxCxS_CHECK_REUSE";
                } else {
                    icm_name = "ND_PRF_MODARRAY_AxCxS";
                }

                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), icm_name, line,
                               type_id_node);
                MAKE_NEXT_ICM_ARG (icm_arg, dim_res);
                MAKE_NEXT_ICM_ARG (icm_arg, res);
                MAKE_NEXT_ICM_ARG (icm_arg, arg1);
                MAKE_NEXT_ICM_ARG (icm_arg, arg3);
                MAKE_NEXT_ICM_ARG (icm_arg, length);
                icm_arg->node[1] = ARRAY_AELEMS (arg2);
                SET_VARS_FOR_MORE_ICMS;
            }
            INC_RC_ND (res, res_ref);
            DEC_OR_FREE_RC_ND (arg1, MakeNum (1));
            INSERT_ASSIGN;
        }
    } else {
        /* index is a variable ! */
        DBUG_ASSERT ((NODE_TYPE (arg2) == N_id),
                     "wrong 2nd arg of modarray (neither N_array nor N_id!");

        if (NODE_TYPE (arg3) == N_array) { /* value is constant! */
            DBUG_ASSERT (0, "sorry compilation of ND_PRF_MODARRAY_AxVxC not yet done");
        } else {
            DBUG_ASSERT (((TYPES_DIM (ID_TYPE (arg2)) == 1)
                          && (TYPES_BASETYPE (ID_TYPE (arg2)) == T_int)),
                         "indexing var of wrong type as 2nd arg of modarray!");

            length = MakeNum (TYPES_SHAPE (ID_TYPE (arg2), 0));

            if ((N_id == NODE_TYPE (arg3)) && (1 == IsArray (ID_TYPE (arg3)))) {
                if (1 == arg1->refcnt) {
                    icm_name = "ND_PRF_MODARRAY_AxVxA_CHECK_REUSE";
                } else {
                    icm_name = "ND_PRF_MODARRAY_AxVxA";
                }

                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), icm_name, line,
                               type_id_node);
                MAKE_NEXT_ICM_ARG (icm_arg, dim_res);
                MAKE_NEXT_ICM_ARG (icm_arg, res);
                MAKE_NEXT_ICM_ARG (icm_arg, arg1);
                MAKE_NEXT_ICM_ARG (icm_arg, arg3);
                MAKE_NEXT_ICM_ARG (icm_arg, length);
                MAKE_NEXT_ICM_ARG (icm_arg, arg2);
                SET_VARS_FOR_MORE_ICMS;
                DEC_OR_FREE_RC_ND (arg3, MakeNum (1));
            } else {
                if (1 == arg1->refcnt) {
                    icm_name = "ND_PRF_MODARRAY_AxVxS_CHECK_REUSE";
                } else {
                    icm_name = "ND_PRF_MODARRAY_AxVxS";
                }

                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), icm_name, line,
                               type_id_node);
                MAKE_NEXT_ICM_ARG (icm_arg, dim_res);
                MAKE_NEXT_ICM_ARG (icm_arg, res);
                MAKE_NEXT_ICM_ARG (icm_arg, arg1);
                MAKE_NEXT_ICM_ARG (icm_arg, arg3);
                MAKE_NEXT_ICM_ARG (icm_arg, length);
                MAKE_NEXT_ICM_ARG (icm_arg, arg2);
                SET_VARS_FOR_MORE_ICMS;
            }
            INC_RC_ND (res, res_ref);
            DEC_OR_FREE_RC_ND (arg1, MakeNum (1));
            INSERT_ASSIGN;
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPIdxModarray
 *  arguments     : 1) N_prf node
 *                  2)
 *  description   : transforms N_prf node F_idx_modarray to N_icm nodes
 *  remarks       : INFO_COMP_LASTIDS(arg_info) contains name of assigned variable
 *                  INFO_COMP_LASTASSIGN(arg_info) contains pointer to node before
 *                   last assign_node (to get last or next assign node use
 *                                     macros LAST_ASSIGN or NEXT_ASSIGN )
 *                  INFO_COMP_LASTASSIGN(arg_info) is used to insert new assign_nodes
 *                   in front of or after last assign node
 *                  INFO_COMP_LASTLET(arg_info) contains pointer to last N_let
 */

node *
COMPIdxModarray (node *arg_node, node *arg_info)
{
    node *res, *res_ref, *type_id_node, *line, *first_assign, *next_assign, *icm_arg,
      *old_arg_node, *last_assign;
    node *arg1 = PRF_ARG1 (arg_node);
    node *arg2 = PRF_ARG2 (arg_node);
    node *arg3 = PRF_ARG3 (arg_node);

    simpletype s_type;

    DBUG_ENTER ("COMPIdxModarray");

    res = MakeId3 (INFO_COMP_LASTIDS (arg_info));

    /* compute basic_type of result */
    GET_BASIC_SIMPLETYPE (s_type,
                          VARDEC_TYPE (IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
    type_id_node = MakeId1 (type_string[s_type]);

    /* store refcount of res as N_num */
    res_ref = MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

    /* store line of prf function */
    line = MakeNum (NODE_LINE (arg_node));

    if (NODE_TYPE (arg3) == N_array) { /* value is constant! */
        DBUG_ASSERT (0, "sorry compilation of ND_IDX_MODARRAY_AxVxC not yet done");
    } else {

        if ((N_id == NODE_TYPE (arg3)) && (1 == IsArray (ID_TYPE (arg3)))) {
            char *icm_name;

            if (1 == arg1->refcnt) {
                icm_name = "ND_IDX_MODARRAY_AxVxA_CHECK_REUSE";
            } else {
                icm_name = "ND_IDX_MODARRAY_AxVxA";
            }

            BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), icm_name, line, type_id_node);
            MAKE_NEXT_ICM_ARG (icm_arg, res);
            MAKE_NEXT_ICM_ARG (icm_arg, arg1);
            MAKE_NEXT_ICM_ARG (icm_arg, arg2);
            MAKE_NEXT_ICM_ARG (icm_arg, arg3);
            SET_VARS_FOR_MORE_ICMS;
            DEC_OR_FREE_RC_ND (arg3, MakeNum (1));
        } else {
            char *icm_name;

            if (1 == arg1->refcnt)
                icm_name = "ND_IDX_MODARRAY_AxVxS_CHECK_REUSE";
            else
                icm_name = "ND_IDX_MODARRAY_AxVxS";

            BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), icm_name, line, type_id_node);
            MAKE_NEXT_ICM_ARG (icm_arg, res);
            MAKE_NEXT_ICM_ARG (icm_arg, arg1);
            MAKE_NEXT_ICM_ARG (icm_arg, arg2);
            MAKE_NEXT_ICM_ARG (icm_arg, arg3);
            SET_VARS_FOR_MORE_ICMS;
        }
        INC_RC_ND (res, res_ref);
        DEC_OR_FREE_RC_ND (arg1, MakeNum (1));
        INSERT_ASSIGN;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPConvert
 *  arguments     : 1) N_prf nodei (F_toi, F_tod, F_tof, F_toi_A, F_tof_A,
 *                                  F_tod_A)
 *                  2) NULL
 *  description   :
 *  remarks       : INFO_COMP_LASTIDS(arg_info) contains name of assigned variable
 */

node *
COMPConvert (node *arg_node, node *arg_info)
{
    int convert = 0;

    DBUG_ENTER ("COMPConvert");

    switch (PRF_PRF (arg_node)) {
    case F_toi:
    case F_tod:
    case F_tof: {
        node *dummy = arg_node;

        /* return argument of ftoi */
        arg_node = PRF_ARG1 (arg_node);
        FREE (dummy->node[0]); /* free N_exprs node */
        FREE (dummy);          /* free N_prf node */
        break;
    }
    case F_tof_A:
        convert = 1;
        /* here is NO break missing!! */
    case F_tod_A:
        convert = 2;
        /* here is NO break missing!! */
    case F_toi_A: {
        int length;
        node *res_rc, *n_length, *first_assign, *next_assign, *type_id_node,
          *old_arg_node, *last_assign, *res, *icm_arg, *arg1;
        simpletype s_type;

        arg1 = PRF_ARG1 (arg_node);
        res = MakeId3 (INFO_COMP_LASTIDS (arg_info));
        /* compute basic type */
        GET_BASIC_SIMPLETYPE (s_type,
                              VARDEC_TYPE (IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
        type_id_node = MakeId1 (type_string[s_type]);
        res_rc = MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));
        BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY", type_id_node, res);
        MAKE_NEXT_ICM_ARG (icm_arg, res_rc);
        SET_VARS_FOR_MORE_ICMS;

        if (N_id == NODE_TYPE (arg1)) {
            switch (convert) {
            case 0:
                CREATE_2_ARY_ICM (next_assign, "ND_2I_A", arg1, res);
                break;
            case 1:
                CREATE_2_ARY_ICM (next_assign, "ND_2F_A", arg1, res);
                break;
            case 2:
                CREATE_2_ARY_ICM (next_assign, "ND_2D_A", arg1, res);
                break;
            default:
                DBUG_ASSERT (0, "wrong tag (convert)");
                break;
            }
            APPEND_ASSIGNS (first_assign, next_assign);

            DEC_OR_FREE_RC_ND (arg1, MakeNum (1));
            INSERT_ASSIGN;
        } else {
            DBUG_ASSERT (N_array == NODE_TYPE (arg1), "wrong node != N_array");
            DBUG_ASSERT (NULL != ARRAY_TYPE (arg1), " info.types is NULL");
            COUNT_ELEMS (length, ARRAY_AELEMS (arg1));
            if (1 < TYPES_DIM (ARRAY_TYPE (arg1))) {
                node *dummy;
                /*
                 *  it is an array of arrays, so we have to use
                 *  ND_CREATE_CONST_ARRAY_A
                 */
                DBUG_ASSERT (N_id == NODE_TYPE (arg1->node[0]->node[0]),
                             "wrong node != N_id");
                GET_LENGTH (length, VARDEC_TYPE (ID_VARDEC (arg1->node[0]->node[0])));
                n_length = MakeNum (length);

                CREATE_3_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_A", res, n_length,
                                  MakeNum (length));
                icm_arg->node[1] = ARRAY_AELEMS (arg1);
                APPEND_ASSIGNS (first_assign, next_assign);

                /* now decrement refcount of the arrays */
                dummy = arg1->node[0];
                while (NULL != dummy) {
                    DBUG_ASSERT (N_id == NODE_TYPE (dummy->node[0]),
                                 "wrong nodetype != N_id");
                    DEC_OR_FREE_RC_ND (dummy->node[0], MakeNum (1));
                    dummy = dummy->node[1];
                }
            } else {
                /* it is an array out of scalar values */
                CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_S", res,
                                  MakeNum (length));
                icm_arg->node[1] = arg1->node[0];
                APPEND_ASSIGNS (first_assign, next_assign);
            }
            INSERT_ASSIGN;
        }
        break;
    }
    default:
        /*   DBUG_ASSERT(0,"wrong prf"); */
        break;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPPrf
 *  arguments     : 1) N_prf node
 *                  2) NULL
 *  description   : transforms N_prf node to N_icm nodes if prf works on
 *                  arrays
 *  remarks       : INFO_COMP_LASTIDS(arg_info) contains name of assigned variable
 *                  INFO_COMP_LASTASSIGN(arg_info) contains pointer to node before
 *                   last assign_node (to get last or next assign node use
 *                                     macros LAST_ASSIGN or NEXT_ASSIGN )
 *                  INFO_COMP_LASTASSIGN(arg_info) is used to insert new assign_nodes
 *                   in front of or after last assign node
 *                  INFO_COMP_LASTLET(arg_info) contains pointer to last N_let
 */

/*
 * This function is partly (!) adjusted to the new returning convention of
 * COMPAssign and COMPLet !
 * To allow for both techniques to co-exist for a while, we use a flag
 * "assign_chain_res" for switching between the two conventions.
 * If (assign_chain_res == 1) holds, we return icms which should hold
 * an assignment chain that will replace the actual assignment (!= arg_node!)
 * Note here, that this variant should NOT free/ re-use arg_node!
 * For (assign_chain_res == 0), it is assumed, that all modifications happen
 * here! This includes the insertion of further assign nodes as well as
 * freeing not further used parts of the AST!
 * In the long term, we should try to convert all parts to (assign_chain_res == 1)!!!
 */

node *
COMPPrf (node *arg_node, node *arg_info)
{
    int assign_chain_res = 0;
    node *icms;
    node *array, *scalar, *tmp, *res, *res_ref, *icm_arg, *prf_id_node, *type_id_node,
      *arg1, *arg2, *arg3, *first_assign = NULL, *next_assign = NULL, *last_assign = NULL,
                           *length_node, *tmp_array1, *tmp_array2, *dim_node, *tmp_rc,
                           *exprs;
    node *old_arg_node;
    simpletype res_stype = LET_BASETYPE (INFO_COMP_LASTLET (arg_info));
    int dim, is_SxA = 0, n_elems = 0, is_drop = 0, array_is_const = 0;
    simpletype s_type;

    DBUG_ENTER ("COMPPrf");

    DBUG_PRINT ("COMP",
                ("%s line: %d", mdb_prf[PRF_PRF (arg_node)], NODE_LINE (arg_node)));

    /*
     * NOTE:  F_neq should be the last "function enumerator" that hasn't
     *        arrays as arguments.
     */
    if (PRF_PRF (arg_node) > F_neq) {
        ids *let_ids = INFO_COMP_LASTIDS (arg_info);
        node *new_name, *new_assign, *old_name;
        int insert_assign = 0;

        exprs = PRF_ARGS (arg_node);
        /* test whether an identifier occurs on the right and left side of a
         * let. In this case rename the one on the right side ,assign old and new
         * variable and add vardec for the new variable.
         * (e.g: A=A+1 => __A=A; A=__A+1; )
         */
        while (NULL != exprs) {
            if (N_id == NODE_TYPE (EXPRS_EXPR (exprs))) {
                if (0 == strcmp (IDS_NAME (let_ids), ID_NAME (EXPRS_EXPR (exprs)))) {
                    if (0 == insert_assign) {
                        new_name = MakeId1 (RenameVar (IDS_NAME (let_ids), 0));
                        old_name = MakeId3 (let_ids);
                        CREATE_2_ARY_ICM (new_assign, "ND_KS_ASSIGN_ARRAY", old_name,
                                          new_name);
                        ASSIGN_NEXT (new_assign) = CURR_ASSIGN (arg_info);
                        INSERT_BEFORE (arg_info, new_assign);

                        /* set info_node to right node (update info_node )*/
                        INFO_COMP_LASTASSIGN (arg_info) = new_assign;

                        insert_assign = 1;

                        /* now insert vardec if necessary */
                        INFO_COMP_VARDECS (arg_info)
                          = AddVardec (INFO_COMP_VARDECS (arg_info),
                                       VARDEC_TYPE (IDS_VARDEC (let_ids)),
                                       ID_NAME (new_name), INFO_COMP_FUNDEF (arg_info));
                    }

                    /* now rename N_id */
                    FREE (ID_NAME (EXPRS_EXPR (exprs)));
                    ID_NAME (EXPRS_EXPR (exprs)) = RenameVar (IDS_NAME (let_ids), 0);
                }
            }

            exprs = EXPRS_NEXT (exprs);
        }

        /*
         * Select primitive funcction:
         */
        switch (PRF_PRF (arg_node)) {
        case F_modarray:
            arg_node = COMPPrfModarray (arg_node, arg_info);
            break;
        case F_idx_modarray:
            arg_node = COMPIdxModarray (arg_node, arg_info);
            break;
        case F_add_SxA:
        case F_div_SxA:
        case F_sub_SxA:
        case F_mul_SxA:
            is_SxA = 1;
            /* here is NO break missing */
        case F_add_AxS:
        case F_div_AxS:
        case F_sub_AxS:
        case F_mul_AxS: {
            simpletype array_stype;

            /* store arguments and result (as N_id)  */
            if (0 == is_SxA) {
                array = arg_node->node[0]->node[0];
                GET_BASIC_SIMPLETYPE_OF_NODE (array_stype, array);
                scalar = arg_node->node[0]->node[1]->node[0];
            } else {
                array = arg_node->node[0]->node[1]->node[0];
                GET_BASIC_SIMPLETYPE_OF_NODE (array_stype, array);
                scalar = arg_node->node[0]->node[0];
            }
            res = MakeId3 (INFO_COMP_LASTIDS (arg_info));

            /* store prf as N_id */
            prf_id_node = MakeId1 (prf_string[arg_node->info.prf]);

            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, VARDEC_TYPE (
                                            IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
            type_id_node = MakeId1 (type_string[s_type]);

            /* store refcount of res as N_num */
            res_ref = MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

            if (N_id == NODE_TYPE (array)) {
                last_assign = NEXT_ASSIGN (arg_info);

                CHECK_REUSE__ALLOC_ARRAY_ND (res, res_stype, array, array_stype);
            } else {
                /* array is constant, so make a block , declare a temporary
                 * variable __TMP and create a constant array
                 */

                array_is_const = 1;
                old_arg_node = arg_node;

                /* count number of elements */
                tmp = ARRAY_AELEMS (array);
                while (NULL != tmp) {
                    n_elems++;
                    tmp = EXPRS_NEXT (tmp);
                }

                tmp_array1 = MakeId1 ("__TMP");

                /* reuse previous N_let */
                NODE_TYPE (INFO_COMP_LASTLET (arg_info)) = N_block;
#ifdef TAGGED_ARRAYS
                CREATE_4_ARY_ICM (first_assign, "ND_DECL_AKS", type_id_node, tmp_array1,
                                  MakeNum (1), MakeNum (n_elems));
#else  /* TAGGED_ARRAYS */
                CREATE_4_ARY_ICM (first_assign, "ND_KS_DECL_ARRAY", type_id_node,
                                  tmp_array1, MakeNum (1), MakeNum (n_elems));
#endif /* TAGGED_ARRAYS */
                arg_node = first_assign;

                /* create const array */
                CREATE_CONST_ARRAY (array, tmp_array1, type_id_node, res_ref);

                /* reuse temporary array __TMP */
                CREATE_2_ARY_ICM (next_assign, "ND_KS_ASSIGN_ARRAY", tmp_array1, res);
                APPEND_ASSIGNS (first_assign, next_assign);

                array = tmp_array1; /* set array to __TMP */
            }

            if (0 == is_SxA) {
                /* create ND_BINOP_AxS_A */
                CREATE_4_ARY_ICM (next_assign, "ND_BINOP_AxS_A", prf_id_node, array,
                                  scalar, res);
            } else {
                /* create ND_BINOP_SxA_A */
                CREATE_4_ARY_ICM (next_assign, "ND_BINOP_SxA_A", prf_id_node, scalar,
                                  array, res);
            }
            APPEND_ASSIGNS (first_assign, next_assign);

            if (0 == array_is_const) {
                if (0 < res_ref->info.cint) {
                    /* create ND_INC_RC */
                    CREATE_2_ARY_ICM (next_assign, "ND_INC_RC", res, res_ref);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    /* create ND_DEC_RC */
                    DEC_OR_FREE_RC_ND (array, MakeNum (1));
                } else {
                    /* create ND_DEC_RC_FREE_ARRAY */
                    DEC_OR_FREE_RC_ND (array, MakeNum (1));
                }
            }

            if (0 == array_is_const) {
                INSERT_ASSIGN;
            }
            FREE (old_arg_node);
            break;
        }
        case F_add_AxA:
        case F_sub_AxA:
        case F_mul_AxA:
        case F_div_AxA: {
            simpletype arg1_stype, arg2_stype;

            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            res = MakeId3 (INFO_COMP_LASTIDS (arg_info));

            /* store prf as N_id */
            prf_id_node = MakeId1 (prf_string[arg_node->info.prf]);

            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, VARDEC_TYPE (
                                            IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
            type_id_node = MakeId1 (type_string[s_type]);

            /* compute basic_type of arg1 and arg2 */
            GET_BASIC_SIMPLETYPE_OF_NODE (arg1_stype, arg1);
            GET_BASIC_SIMPLETYPE_OF_NODE (arg2_stype, arg2);

            /* store refcount of res as N_num */
            res_ref = MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

            if ((N_id == NODE_TYPE (arg1)) && (N_id == NODE_TYPE (arg2))) {
                last_assign = NEXT_ASSIGN (arg_info);
                if ((1 == arg1->refcnt) && (1 == arg2->refcnt)) {
                    node *num;

                    BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_CHECK_REUSE_ARRAY",
                                   arg1, res);
                    SET_VARS_FOR_MORE_ICMS;

                    CREATE_2_ARY_ICM (next_assign, "ND_CHECK_REUSE_ARRAY", arg2, res);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    num = MakeNum (0);
                    CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, res,
                                      num);
                    APPEND_ASSIGNS (first_assign, next_assign);
                } else if (1 == arg1->refcnt) {
                    CHECK_REUSE__ALLOC_ARRAY_ND (res, res_stype, arg1, arg1_stype);
                } else if (1 == arg2->refcnt) {
                    CHECK_REUSE__ALLOC_ARRAY_ND (res, res_stype, arg2, arg2_stype);
                } else {
                    node *num;

                    num = MakeNum (0);
                    TRI_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY",
                                   type_id_node, res, num);
                    first_assign = CURR_ASSIGN (arg_info);
                    old_arg_node = arg_node;
                    last_assign = NEXT_ASSIGN (arg_info);
                    arg_node = LET_EXPR (INFO_COMP_LASTLET (arg_info));

                    DBUG_ASSERT ((((-1 == arg1->refcnt) && (-1 == arg2->refcnt))
                                  || (!(optimize & OPT_RCO))),
                                 "Refcnt of BINOP_A_A arg neither -1 nor 1 !");
                }
            } else {
                /* reuse previous N_let */
                NODE_TYPE (INFO_COMP_LASTLET (arg_info)) = N_block;
                old_arg_node = arg_node;
                if ((N_array == NODE_TYPE (arg1)) && (N_array == NODE_TYPE (arg2))) {
                    array_is_const = 3;
                    DECL_ARRAY (first_assign, arg1->node[0], "__TMP1", tmp_array1);
                    arg_node = first_assign; /* set new arg_node */
                    DECL_ARRAY (next_assign, arg2->node[0], "__TMP2", tmp_array2);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    CREATE_CONST_ARRAY (arg1, tmp_array1, type_id_node, res_ref);
                    CREATE_CONST_ARRAY (arg2, tmp_array2, type_id_node, MakeNum (0));
                    CREATE_2_ARY_ICM (next_assign, "ND_KS_ASSIGN_ARRAY", tmp_array1, res);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    /* set arg1 and arg2 for later use as parameters of BIN_OP */
                    arg1 = tmp_array1;
                    arg2 = tmp_array2;
                } else if (N_array == NODE_TYPE (arg1)) {
                    array_is_const = 1;
                    DECL_ARRAY (first_assign, arg1->node[0], "__TMP1", tmp_array1);
                    arg_node = first_assign;
                    CREATE_CONST_ARRAY (arg1, tmp_array1, type_id_node, res_ref);
                    CREATE_2_ARY_ICM (next_assign, "ND_KS_ASSIGN_ARRAY", tmp_array1, res);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    /* set arg1 for later use as parameters of BIN_OP */
                    arg1 = tmp_array1;
                } else {
                    array_is_const = 2;
                    DECL_ARRAY (first_assign, arg2->node[0], "__TMP2", tmp_array2);
                    arg_node = first_assign;
                    CREATE_CONST_ARRAY (arg2, tmp_array2, type_id_node, res_ref);
                    CREATE_2_ARY_ICM (next_assign, "ND_KS_ASSIGN_ARRAY", tmp_array2, res);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    /* set arg2 for later use as parameters of BIN_OP */
                    arg2 = tmp_array2;
                }
            }
            CREATE_4_ARY_ICM (next_assign, "ND_BINOP_AxA_A", prf_id_node, arg1, arg2,
                              res);
            APPEND_ASSIGNS (first_assign, next_assign);

            switch (array_is_const) {
            case 0:
                CREATE_2_ARY_ICM (next_assign, "ND_INC_RC", res, res_ref);
                APPEND_ASSIGNS (first_assign, next_assign);
                DEC_OR_FREE_RC_ND (arg2, MakeNum (1));
                DEC_OR_FREE_RC_ND (arg1, MakeNum (1));
                break;
            case 1:
                CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE_ARRAY", arg2, MakeNum (1));
                APPEND_ASSIGNS (first_assign, next_assign);
                break;
            case 2:
                CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE_ARRAY", arg1, MakeNum (1));
                APPEND_ASSIGNS (first_assign, next_assign);
                break;
            case 3:
                CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE_ARRAY", arg2, MakeNum (1));
                APPEND_ASSIGNS (first_assign, next_assign);
                break;
            default:
                DBUG_ASSERT (0, "array_is_const is out of range");
                break;
            }

            if (0 == array_is_const) {
                INSERT_ASSIGN;
            }
            FREE (old_arg_node);

            break;
        }
        case F_drop:
            is_drop = 1;
            /*
             * here is NO break missing
             */
        case F_take: {
            node *num;
            /*
             * store arguments and result (res contains refcount and pointer to
             * vardec ( don't free INFO_COMP_LASTIDS(arg_info) !!! )
             *
             * if first argument of prf is a scalar (N_um), it will be compiled
             * like an vector (array) with one element
             *
            arg1=arg_node->node[0]->node[0];
            arg2=arg_node->node[0]->node[1]->node[0];
             */
            arg1 = EXPRS_EXPR (PRF_ARGS (arg_node));
            arg2 = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node)));
            DBUG_ASSERT (((NODE_TYPE (arg1) == N_array) || (NODE_TYPE (arg1) == N_num)
                          || ((NODE_TYPE (arg1) == N_id) && (ID_ISCONST (arg1))
                              && (VARDEC_BASETYPE (ID_VARDEC (arg1)) == T_int))),
                         "first argument of take/drop isn't an array or scalar jhs");

            res = MakeId3 (INFO_COMP_LASTIDS (arg_info));

            /*
             * compute basic_type of result
             */
            GET_BASIC_SIMPLETYPE (s_type, VARDEC_TYPE (
                                            IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
            type_id_node = MakeId1 (type_string[s_type]);
            /*
             * store refcount of res as N_num
             */
            res_ref = MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

            num = MakeNum (0);
            if (NODE_TYPE (arg2) == N_id) {
                GET_DIM (dim, VARDEC_TYPE (ID_VARDEC (arg2)));
                /*
                 *  store dimension of argument-array
                 */
                dim_node = MakeNum (dim);
                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY",
                               type_id_node, res);
                MAKE_NEXT_ICM_ARG (icm_arg, num);
                SET_VARS_FOR_MORE_ICMS;
            } else {
                DBUG_ASSERT ((NODE_TYPE (arg2) == N_array), "wrong nodetype");
                /*
                 *  store dimension of argument-array
                 */
                dim_node = MakeNum (1);
                CREATE_TMP_CONST_ARRAY (arg2, res_ref);
                CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, res, num);
                APPEND_ASSIGNS (first_assign, next_assign);
            }

            if (NODE_TYPE (arg1) == N_array) {
                n_elems = ARRAY_VECLEN (arg1);
            } else {
                if (NODE_TYPE (arg1) == N_id) {
                    n_elems = ID_VECLEN (arg1);
                } else /* if (NODE_TYPE(arg1) == N_num) */ {
                    n_elems = 1;
                }
            }
            if (is_drop == 1) {
                CREATE_4_ARY_ICM (next_assign, "ND_KD_DROP_CxA_A", dim_node, arg2, res,
                                  MakeNum (n_elems));
            } else {
                CREATE_4_ARY_ICM (next_assign, "ND_KD_TAKE_CxA_A", dim_node, arg2, res,
                                  MakeNum (n_elems));
            }
            if (NODE_TYPE (arg1) == N_num) {
                MAKE_NEXT_ICM_ARG (icm_arg, arg1);
            } else if (NODE_TYPE (arg1) == N_array) {
                icm_arg = MakeExprs (ARRAY_AELEMS (arg1), NULL);
            } else
#if 0
           if (NODE_TYPE(arg1) == N_id)
#endif
            {
                int i;
                for (i = 0; i < n_elems; i++) {
                    num = MakeNum (((int *)ID_CONSTVEC (arg1))[i]);
                    MAKE_NEXT_ICM_ARG (icm_arg, num);
                }
            }

            APPEND_ASSIGNS (first_assign, next_assign);

            if (array_is_const == 0) {
                DEC_OR_FREE_RC_ND (arg2, MakeNum (1));
                INC_RC_ND (res, res_ref);
                INSERT_ASSIGN;
            }
            FREE (old_arg_node);
            break;
        }
        case F_reshape: {
            /*
             * this function complies to the NEW returning convention of
             * COMPAssign/ COMPLet, i.e., we return an assignment chain
             * of ICMs and we DO NOT free any parts of arg_node!
             * For compliance with older parts (other cases...), we
             * finally set the flag "assign_chain_res" to 1 which causes
             * the function to return "icms" rather than "arg_node"!
             */
            assign_chain_res = 1;

            arg1 = PRF_ARG1 (arg_node);
            arg2 = PRF_ARG2 (arg_node);

            DBUG_ASSERT ((NODE_TYPE (arg2) == N_id),
                         "N_id as 2nd arg of F_reshape expected!");
            if (0 == strcmp (ID_NAME (arg2), IDS_NAME (INFO_COMP_LASTIDS (arg_info)))) {
                /*
                 * We are dealing with an assignment of the kind:
                 *   a = reshape( b, a);
                 * which we compile into:
                 *   NOOP() which will be eliminated by ???
                 */
                icms = MakeAssignIcm0 ("NOOP");
            } else {
                res = MakeId3 (DupOneIds (INFO_COMP_LASTIDS (arg_info), NULL));
                icms = MakeAssignIcm2 ("ND_KS_ASSIGN_ARRAY", DupTree (arg2), res);
                ASSIGN_NEXT (icms) = MakeAdjustRcICM (ID_IDS (res), ID_REFCNT (res) - 1);
            }

            DBUG_ASSERT ((NODE_TYPE (arg1) == N_id),
                         "N_id as 1st arg of F_reshape expected!");
            icms = MakeDecRcICMs (ID_IDS (arg1), icms);
            break;
        }
        case F_psi: {
            /*
             * store arguments and result (res contains refcount and pointer to
             * vardec ( don't free INFO_COMP_LASTIDS(arg_info) !!! )
             */
            node *line, *arg1_length;

            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            res = MakeId3 (INFO_COMP_LASTIDS (arg_info));
            line = MakeNum (NODE_LINE (arg_node));

            last_assign = NEXT_ASSIGN (arg_info);

            /* compute basic type */
            GET_BASIC_SIMPLETYPE (s_type, VARDEC_TYPE (
                                            IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
            type_id_node = MakeId1 (type_string[s_type]);

            /* compute length of arg1 */
            if (N_id == NODE_TYPE (arg1)) {
                GET_LENGTH (n_elems, VARDEC_TYPE (ID_VARDEC (arg1)));
                arg1_length = MakeNum (n_elems);
            } else {
                n_elems = 0;
                tmp = arg1->node[0];
                do {
                    n_elems++;
                    tmp = tmp->node[1];
                } while (NULL != tmp);
                arg1_length = MakeNum (n_elems);
            }

            if (0 == IsArray (VARDEC_TYPE (ID_VARDEC (res)))) {
                if (N_id == NODE_TYPE (arg2)) {
                    if (N_id == NODE_TYPE (arg1)) {
                        BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_KD_PSI_VxA_S",
                                       line, arg2);
                        MAKE_NEXT_ICM_ARG (icm_arg, res);
                        MAKE_NEXT_ICM_ARG (icm_arg, arg1_length);
                        MAKE_NEXT_ICM_ARG (icm_arg, arg1);
                    } else {
                        BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_KD_PSI_CxA_S",
                                       line, arg2);
                        MAKE_NEXT_ICM_ARG (icm_arg, res);
                        MAKE_NEXT_ICM_ARG (icm_arg, arg1_length);
                        icm_arg->node[1] = arg1->node[0];
                        /*
                         * FREE(arg1);
                         *
                         * arg1 cannot be freed here since it is used 6 lines
                         * farther.
                         */
                    }
                    first_assign = CURR_ASSIGN (arg_info);
                    old_arg_node = arg_node;
                    arg_node = LET_EXPR (INFO_COMP_LASTLET (arg_info));
                    DEC_OR_FREE_RC_ND (arg2, MakeNum (1));
                    if (N_id == NODE_TYPE (arg1)) {
                        DEC_OR_FREE_RC_ND (arg1, MakeNum (1));
                    }

                    INSERT_ASSIGN;

                    FREE (old_arg_node);
                } else {
                    /* arg2 is a constant array */
                    DBUG_ASSERT ((N_array == NODE_TYPE (arg2)), "nodetype != N_array");
                    tmp_rc = MakeNum (0);
                    CREATE_TMP_CONST_ARRAY (arg2, tmp_rc);
                    if (N_id == NODE_TYPE (arg1)) {
                        CREATE_5_ARY_ICM (next_assign, "ND_KD_PSI_VxA_S", line, arg2, res,
                                          arg1_length, arg1);
                    } else {
                        CREATE_4_ARY_ICM (next_assign, "ND_KD_PSI_CxA_S", line, arg2, res,
                                          arg1_length);
                        icm_arg->node[1] = arg1->node[0];
                        FREE (arg1);
                    }
                    APPEND_ASSIGNS (first_assign, next_assign);
                }
            } else {
                node *num;

                DBUG_ASSERT ((N_id == NODE_TYPE (arg2)), "arg2 != N_id");
                GET_DIM (dim, VARDEC_TYPE (ID_VARDEC (arg2)));
                dim_node = MakeNum (dim);
                /* store refcount of res as N_num */
                res_ref = MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

                num = MakeNum (0);
                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY",
                               type_id_node, res);
                MAKE_NEXT_ICM_ARG (icm_arg, num);
                SET_VARS_FOR_MORE_ICMS;

                if (N_id == NODE_TYPE (arg1)) {
                    CREATE_6_ARY_ICM (next_assign, "ND_KD_PSI_VxA_A", line, dim_node,
                                      arg2, res, arg1_length, arg1);
                } else {
                    CREATE_5_ARY_ICM (next_assign, "ND_KD_PSI_CxA_A", line, dim_node,
                                      arg2, res, arg1_length);
                    icm_arg->node[1] = arg1->node[0];
                    FREE (arg1);
                }
                APPEND_ASSIGNS (first_assign, next_assign);

                DEC_OR_FREE_RC_ND (arg2, MakeNum (1));
                INC_RC_ND (res, res_ref);
                INSERT_ASSIGN;
                FREE (old_arg_node);
            }
            break;
        }
        case F_idx_psi: {
            node *arg2_ref;
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            DBUG_ASSERT (N_id == NODE_TYPE (arg2), "wrong second arg of idx_psi");

            arg2_ref = MakeNum (1);
            res = MakeId3 (INFO_COMP_LASTIDS (arg_info));
            if (1 == IsArray (VARDEC_TYPE (IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))))) {
                res_ref = MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));
                GET_BASIC_SIMPLETYPE (s_type, VARDEC_TYPE (IDS_VARDEC (
                                                INFO_COMP_LASTIDS (arg_info))));
                type_id_node = MakeId1 (type_string[s_type]);
                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY",
                               type_id_node, res);
                SET_VARS_FOR_MORE_ICMS;
                MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
                CREATE_3_ARY_ICM (next_assign, "ND_IDX_PSI_A", arg1, arg2, res);
                APPEND_ASSIGNS (first_assign, next_assign);
            } else {
                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_IDX_PSI_S", arg1, arg2);
                MAKE_NEXT_ICM_ARG (icm_arg, res);
                SET_VARS_FOR_MORE_ICMS;
            }
            DEC_OR_FREE_RC_ND (arg2, arg2_ref);
            INSERT_ASSIGN;
            break;
        }
        case F_dim: {
            arg1 = arg_node->node[0]->node[0];
            NODE_TYPE (arg_node) = N_num;
            if (N_array == NODE_TYPE (arg1)) {
                arg_node->info.cint = 1;
            } else {
                node *id_node;

                GET_DIM (arg_node->info.cint, VARDEC_TYPE (ID_VARDEC (arg1)));
                first_assign = CURR_ASSIGN (arg_info);
                last_assign = NEXT_ASSIGN (arg_info);
                id_node = MakeId1 (ID_NAME (arg1));
                ID_REFCNT (id_node) = ID_REFCNT (arg1);
                DEC_OR_FREE_RC_ND (id_node, MakeNum (1));
                INSERT_ASSIGN;
            }
            FreeTree (arg_node->node[0]);
            break;
        }
        case F_shape: {
            int dim;
            arg1 = arg_node->node[0]->node[0];
            /* store type of new array */
            type_id_node = MakeId1 ("int");
            /* store name of new array */
            res = MakeId3 (INFO_COMP_LASTIDS (arg_info));
            /* store refcount of res as N_num */
            res_ref = MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));
            BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY", type_id_node,
                           res);
            MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
            SET_VARS_FOR_MORE_ICMS;
            if (N_id == NODE_TYPE (arg1)) {
                GET_DIM (dim, VARDEC_TYPE (ID_VARDEC (arg1)));
                tmp_array1 = ShapeToArray (ID_VARDEC (arg1));
            } else {
                DBUG_ASSERT ((N_array == NODE_TYPE (arg1)), "wrong nodetype");
                COUNT_ELEMS (n_elems, arg1->node[0]);
                tmp_array1 = MakeNode (N_exprs);
                tmp_array1->node[0] = MakeNum (n_elems);
                dim = 1;
            }
            length_node = MakeNum (dim); /* store length of shape_vector */
            CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_S", res, length_node);
            icm_arg->node[1] = tmp_array1; /* append shape_vector */
            APPEND_ASSIGNS (first_assign, next_assign);
            if (N_id == NODE_TYPE (arg1)) {
                node *id_node;
                id_node = MakeId1 (ID_NAME (arg1));
                ID_REFCNT (id_node) = ID_REFCNT (arg1);

                DEC_OR_FREE_RC_ND (id_node, MakeNum (1));
            }
            FreeTree (old_arg_node);
            INSERT_ASSIGN;
            break;
        }
        case F_cat: {
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            arg3 = arg_node->node[0]->node[1]->node[1]->node[0];
            res = MakeId3 (INFO_COMP_LASTIDS (arg_info));
            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, VARDEC_TYPE (
                                            IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
            type_id_node = MakeId1 (type_string[s_type]);
            /* store refcount of res as N_num */
            res_ref = MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

            if ((N_id == NODE_TYPE (arg2)) && (N_id == NODE_TYPE (arg3))) {
                GET_DIM (dim, VARDEC_TYPE (ID_VARDEC (arg2)));
                dim_node = MakeNum (dim);
                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY",
                               type_id_node, res);
                MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
                SET_VARS_FOR_MORE_ICMS;
                CREATE_5_ARY_ICM (next_assign, "ND_KD_CAT_SxAxA_A", dim_node, arg2, arg3,
                                  res, arg1);
                APPEND_ASSIGNS (first_assign, next_assign);

                DEC_OR_FREE_RC_ND (arg2, MakeNum (1));
                DEC_OR_FREE_RC_ND (arg3, MakeNum (1));
                INSERT_ASSIGN;

            } else {
                /* reuse previous N_let */
                NODE_TYPE (INFO_COMP_LASTLET (arg_info)) = N_block;
                old_arg_node = arg_node;
                tmp_rc = MakeNum (0);
                if ((N_array == NODE_TYPE (arg2)) && (N_array == NODE_TYPE (arg3))) {
                    array_is_const = 3;
                    GET_DIM (dim, ARRAY_TYPE (arg2));
                    DECL_ARRAY (first_assign, arg2->node[0], "__TMP1", tmp_array1);
                    arg_node = first_assign; /* set new arg_node */
                    DECL_ARRAY (next_assign, arg3->node[0], "__TMP2", tmp_array2);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    CREATE_CONST_ARRAY (arg2, tmp_array1, type_id_node, tmp_rc);
                    CREATE_CONST_ARRAY (arg3, tmp_array2, type_id_node, tmp_rc);
                    arg2 = tmp_array1;
                    arg3 = tmp_array2;
                } else if (N_array == NODE_TYPE (arg2)) {
                    array_is_const = 1;
                    GET_DIM (dim, ARRAY_TYPE (arg2));
                    DECL_ARRAY (first_assign, arg2->node[0], "__TMP1", tmp_array1);
                    arg_node = first_assign;
                    CREATE_CONST_ARRAY (arg2, tmp_array1, type_id_node, tmp_rc);
                    /* set arg2 for later use as parameters of ND_KD_CAT*/
                    arg2 = tmp_array1;
                } else {
                    array_is_const = 2;
                    GET_DIM (dim, ARRAY_TYPE (arg3));
                    DECL_ARRAY (first_assign, arg3->node[0], "__TMP2", tmp_array2);
                    arg_node = first_assign;
                    CREATE_CONST_ARRAY (arg3, tmp_array2, type_id_node, tmp_rc);
                    /* set arg3 for later use as parameters of ND_KD_CAT*/
                    arg3 = tmp_array2;
                }
                CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, res,
                                  res_ref);
                APPEND_ASSIGNS (first_assign, next_assign);
                dim_node = MakeNum (dim);
                CREATE_5_ARY_ICM (next_assign, "ND_KD_CAT_SxAxA_A", dim_node, arg2, arg3,
                                  res, arg1);
                APPEND_ASSIGNS (first_assign, next_assign);
                switch (array_is_const) {
                case 1:
                    DEC_OR_FREE_RC_ND (arg3, MakeNum (1));
                    DEC_RC_FREE_ND (arg2, MakeNum (1));
                    break;
                case 2:
                    DEC_OR_FREE_RC_ND (arg3, MakeNum (1));
                    DEC_RC_FREE_ND (arg3, MakeNum (1));
                    break;
                case 3:
                    DEC_RC_FREE_ND (arg2, MakeNum (1));
                    DEC_RC_FREE_ND (arg3, MakeNum (1));
                    break;
                default:
                    DBUG_ASSERT (0, "array_is_const is out of range");
                    break;
                }
                FREE (old_arg_node);
            }
            break;
        }
        case F_rotate:
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            arg3 = arg_node->node[0]->node[1]->node[1]->node[0];
            res = MakeId3 (INFO_COMP_LASTIDS (arg_info));
            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, VARDEC_TYPE (
                                            IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
            type_id_node = MakeId1 (type_string[s_type]);
            /* store refcount of res as N_num */
            res_ref = MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

            if (N_id == NODE_TYPE (arg3)) {
                GET_DIM (dim,
                         VARDEC_TYPE (ID_VARDEC (arg3))); /* dim will be used later */
                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY",
                               type_id_node, res);
                MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
                SET_VARS_FOR_MORE_ICMS;
            } else {
                array_is_const = 1;
                /* reuse previous N_let */
                NODE_TYPE (INFO_COMP_LASTLET (arg_info)) = N_block;
                old_arg_node = arg_node;
                GET_DIM (dim, ARRAY_TYPE (arg3)); /* dim will be used later */
                DECL_ARRAY (first_assign, arg3->node[0], "__TMP1", tmp_array1);
                arg_node = first_assign;
                CREATE_CONST_ARRAY (arg3, tmp_array1, type_id_node, MakeNum (1));
                arg3 = tmp_array1;
                CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, res,
                                  res_ref);
                APPEND_ASSIGNS (first_assign, next_assign);
            }
            /* store dimension of arg3 */
            dim_node = MakeNum (dim);
            DBUG_ASSERT ((N_num == NODE_TYPE (arg1)), "wrong 1.arg of 'rotate'");
            CREATE_5_ARY_ICM (next_assign, "ND_KD_ROT_CxSxA_A", arg1, arg2, dim_node,
                              arg3, res);
            APPEND_ASSIGNS (first_assign, next_assign);
            if (0 == array_is_const) {

                DEC_OR_FREE_RC_ND (arg3, MakeNum (1));
                INSERT_ASSIGN;
            } else {
                DEC_RC_FREE_ND (arg3, MakeNum (1));
                FREE (old_arg_node);
            }
            break;
        case F_toi_A:
        case F_tod_A:
        case F_tof_A: {
            arg_node = COMPConvert (arg_node, arg_info);
            break;
        }
        default:
#if 0
         DBUG_ASSERT(0,"wrong prf");
#endif
            break;
        }
    } else { /* Here, we have (PRF_PRF(arg_node) <= F_neq) */
        switch (PRF_PRF (arg_node)) {
        case F_toi:
        case F_tof:
        case F_tod:
            arg_node = COMPConvert (arg_node, arg_info);
            break;

        case F_min:
            NODE_TYPE (arg_node) = N_icm;
            ICM_NAME (arg_node) = "ND_MIN";
            ICM_INDENT (arg_node) = 0;
            break;

        case F_max:
            NODE_TYPE (arg_node) = N_icm;
            ICM_NAME (arg_node) = "ND_MAX";
            ICM_INDENT (arg_node) = 0;
            break;

        case F_abs:
            NODE_TYPE (arg_node) = N_icm;
            ICM_NAME (arg_node) = "ND_ABS";
            ICM_INDENT (arg_node) = 0;
            break;

        default:
            break;
        }
    }

    if (assign_chain_res == 0) {
        DBUG_RETURN (arg_node);
    } else {
        DBUG_RETURN (icms);
    }
}

/*
 *
 *  functionname  : COMPArray
 *  arguments     : 1) N_arrray node
 *                  2) info node
 *  description   :
 *  remarks       : INFO_COMP_LASTIDS(arg_info) contains name of assigned variable
 *                  INFO_COMP_LASTASSIGN(arg_info) contains pointer to node before
 *                    last assign_node
 *                  INFO_COMP_LASTLET(arg_info) contains pointer to previous N_let
 *
 */

node *
COMPArray (node *arg_node, node *arg_info)
{
    int n_elems = 0;
    simpletype s_type;

#ifndef TAGGED_ARRAYS

    node *first_assign, *next_assign, *exprs, *res, *type_id_node, *old_arg_node,
      *icm_arg, *res_ref, *last_assign;
    int icm_created = 0;
#endif

#ifdef TAGGED_ARRAYS

    int elems_are_non_hidden_scalars;
    node *exprs, *res, *type_id_node, *res_ref, *last_assign;
    node *icm1, *icm2, *elems, *name_id_node;
    types *elem_type;

    DBUG_ENTER ("COMPArray");

    /* store next assign */
    last_assign = NEXT_ASSIGN (arg_info);

    /* compute basic_type of result */
    GET_BASIC_SIMPLETYPE (s_type,
                          VARDEC_TYPE (IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
    name_id_node = MakeId1 (VARDEC_NAME (IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
    type_id_node = MakeId1 (type_string[s_type]);

    /* store refcount of res as N_num */
    res_ref = MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

    /* create ND_ALLOC_ARRAY */
    icm1 = MakeIcm3 ("ND_ALLOC_ARRAY", type_id_node, name_id_node, res_ref);

    /* Now, we start constructing the CONST_ARRAY icm! */
    exprs = ARRAY_AELEMS (arg_node);
    elems = DupTree (exprs);

    while (NULL != exprs) {
        n_elems++;
        exprs = EXPRS_NEXT (exprs);
    }

    /*
     * First, we want to find out about the array elems:
     * [ 1, 3, 2] => elems_are_non_hidden_scalars
     * [ a, 3, 2] => elems_are_non_hidden_scalars
     * [ a, b, c] => !elems_are_non_hidden_scalars iff simpletype( a) == T_hidden
     * [ a, b, c] =>  ERROR                        iff dim( a) != SCALAR
     */
    if (NODE_TYPE (EXPRS_EXPR (elems)) == N_id) {
        elem_type = ID_TYPE (EXPRS_EXPR (elems));
        if (TYPES_BASETYPE (elem_type) == T_hidden) {
            elems_are_non_hidden_scalars = 0;
        } else if (TYPES_DIM (elem_type) != SCALAR) {
            DBUG_ASSERT (0, "no nested arrays expected here!");
        } else {
            elems_are_non_hidden_scalars = 1;
        }
    } else {
        elems_are_non_hidden_scalars = 1;
    }

    if (elems_are_non_hidden_scalars) {
        icm2
          = MakeIcm3 ("ND_CREATE_CONST_ARRAY_S", name_id_node, MakeNum (n_elems), elems);
    } else {
        icm2
          = MakeIcm3 ("ND_CREATE_CONST_ARRAY_H", name_id_node, MakeNum (n_elems), elems);
    }

    res = MakeAssign (icm1, MakeAssign (icm2, NULL));

    DBUG_RETURN (res);
}

#else  /* TAGGED_ARRAYS */

    DBUG_ENTER ("COMPArray");

    /* store next assign */
    last_assign = NEXT_ASSIGN (arg_info);

    /* compute basic_type of result */
    GET_BASIC_SIMPLETYPE (s_type,
                          VARDEC_TYPE (IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
    type_id_node = MakeId1 (type_string[s_type]);

    /* store result as N_id */
    res = MakeId3 (INFO_COMP_LASTIDS (arg_info));

    /* store refcount of res as N_num */
    res_ref = MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

    /* create ND_ALLOC_ARRAY */
    BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY", type_id_node, res);
    MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
    first_assign = CURR_ASSIGN (arg_info);
    old_arg_node = arg_node;

    /* do we believe old_arg_node is an N_array?? */
    DBUG_ASSERT ((NODE_TYPE (old_arg_node) == N_array), ("old_argnode is no N_array"));

    arg_node = LET_EXPR (INFO_COMP_LASTLET (arg_info));

    /* create ND_CREATE_CONST_ARRAY */

    /*
     *  Count number of array elements and pack it into an N_num.
     */
    exprs = ARRAY_AELEMS (old_arg_node);
    n_elems = 0;
    while (exprs != NULL) {
        n_elems++;
        exprs = EXPRS_NEXT (exprs);
    }

    if (ARRAY_AELEMS (old_arg_node) != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (ARRAY_AELEMS (old_arg_node))) == N_id) {
            if (IsArray (
                  VARDEC_TYPE (ID_VARDEC (EXPRS_EXPR (ARRAY_AELEMS (old_arg_node)))))
                == 1) {

                /*  #### */

                node *length;
                int len;
                GET_LENGTH (len,
                            VARDEC_TYPE (ID_VARDEC (old_arg_node->node[0]->node[0])));
                length = MakeNum (len);
                CREATE_3_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_A", res, length,
                                  MakeNum (n_elems));
                icm_created = 1;
            } else {
                if (IsNonUniqueHidden (ID_TYPE (old_arg_node->node[0]->node[0]))) {
                    CREATE_3_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_H", res,
                                      MakeId1 (
                                        GenericFun (0,
                                                    ID_TYPE (
                                                      old_arg_node->node[0]->node[0]))),
                                      MakeNum (n_elems));
                    icm_created = 1;
                }
            }
        }
    }
    if (0 == icm_created) {
        if (ARRAY_STRING (old_arg_node) != NULL) {

            CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_C", res,
                              MakeStr (ARRAY_STRING (old_arg_node)));
        } else {
            CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_S", res,
                              MakeNum (n_elems));
            /* now append the elements of the array to the last N_icm */
            icm_arg->node[1] = old_arg_node->node[0];
        }
    } else {
        /* now append the elements of the array to the last N_icm */
        EXPRS_NEXT (icm_arg) = old_arg_node->node[0];
    }

    APPEND_ASSIGNS (first_assign, next_assign);

    INSERT_ASSIGN;

    DBUG_RETURN (arg_node);
}
#endif /* TAGGED_ARRAYS */

/*
 *
 *  functionname  : COMPId
 *  arguments     : 1) N_id node
 *                  2) N_info with VARDECS of fun, LASTASSIGN, LASTLET
 *                     and LASTIDS!
 *  description   :
 *  remarks       : INFO_COMP_LASTIDS(arg_info) contains name of assigned variable
 *                  INFO_COMP_LASTASSIGN(arg_info) contains pointer to to node before
 *                   last assign_node
 *                  INFO_COMP_LASTLET(arg_info) contains pointer to previous N_let
 *
 */

node *
COMPId (node *arg_node, node *arg_info)
{
    node *first_assign, *next_assign, *last_assign, *old_arg_node, *icm_arg, *res,
      *icm_node;

    DBUG_ENTER ("COMPId");

    if (MUST_REFCOUNT (ID, arg_node)) {
        if (NULL != arg_info) {
            res = MakeId3 (INFO_COMP_LASTIDS (arg_info));

            if (ID_MAKEUNIQUE (arg_node)) {
                if (ID_REFCNT (arg_node) == 1) {
                    if (IsArray (VARDEC_TYPE (ID_VARDEC (arg_node)))) {
                        TRI_ICM_REUSE (INFO_COMP_LASTLET (arg_info),
                                       "ND_KS_MAKE_UNIQUE_ARRAY", arg_node, res,
                                       MakeNum (BasetypeSize (
                                         VARDEC_TYPE (ID_VARDEC (arg_node)))));

                        SET_VARS_FOR_MORE_ICMS;
                        SET_RC_ND (res,
                                   MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info))));
                        INSERT_ASSIGN;
                    } else {
                        TRI_ICM_REUSE (INFO_COMP_LASTLET (arg_info),
                                       "ND_NO_RC_MAKE_UNIQUE_HIDDEN", arg_node, res,
                                       MakeId1 (GenericFun (0, ID_TYPE (arg_node))));
                    }
                } else {
                    if (IsArray (VARDEC_TYPE (ID_VARDEC (arg_node)))) {
                        TRI_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_KS_COPY_ARRAY",
                                       arg_node, res,
                                       MakeNum (BasetypeSize (
                                         VARDEC_TYPE (ID_VARDEC (arg_node)))));

                        SET_VARS_FOR_MORE_ICMS;

                        CREATE_1_ARY_ICM (next_assign, "ND_ALLOC_RC",
                                          MakeId1 (ID_NAME (res)));
                        APPEND_ASSIGNS (first_assign, next_assign);

                        SET_RC_ND (res,
                                   MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info))));
                        DEC_RC_ND (MakeId1 (ID_NAME (old_arg_node)), MakeNum (1));

                        INSERT_ASSIGN;
                    } else {
                        TRI_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_COPY_HIDDEN",
                                       arg_node, res,
                                       MakeId1 (GenericFun (0, ID_TYPE (arg_node))));

                        SET_VARS_FOR_MORE_ICMS;

                        DEC_RC_ND (MakeId1 (ID_NAME (old_arg_node)), MakeNum (1));

                        INSERT_ASSIGN;
                    }
                }
            } else {
                if (IsArray (VARDEC_TYPE (ID_VARDEC (arg_node)))) {
                    BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_KS_ASSIGN_ARRAY",
                                   arg_node, res);
                } else {
                    BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ASSIGN_HIDDEN",
                                   arg_node, res);
                }

                SET_VARS_FOR_MORE_ICMS;

                if (0 == IDS_REFCNT (INFO_COMP_LASTIDS (arg_info))) {
                    DEC_RC_FREE_ND (res, MakeNum (1));
                } else if (1 < IDS_REFCNT (INFO_COMP_LASTIDS (arg_info))) {
                    INC_RC_ND (res,
                               MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)) - 1));
                }

                INSERT_ASSIGN;
            }
        } else {
            DBUG_ASSERT ((arg_info != NULL), "corrupted arg-info");

            icm_node = MakeIcm ("ND_KS_RET_ARRAY", MakeExprs (arg_node, NULL), NULL);
            arg_node = icm_node;
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPAp
 *  arguments     : 1) N_Ap node
 *                  2) info node
 *  description   : - creates N_icm for function application
 *                  - insert N_icm to decrement the refcount of functions
 *                    arguments
 *  remarks       : INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *                  INFO_COMP_LASTASSIGN(arg_info) contains pointer to node before
 *                    last assign_node.
 *                  INFO_COMP_LASTLET(arg_info) contains pointer to previous N_let.
 *                  INFO_COMP_VARDECS(arg_info) contains pointer to vardecs.
 *
 */

node *
COMPAp (node *arg_node, node *arg_info)
{
    node *tmp, *next, *exprs, *icm_arg, *save_icm_arg, *id_node, *tag_node, *next_assign,
      *first_assign, *add_assigns_before, *fundef_args, *add_assigns_after, *last_assign,
      **icm_tab, *icm_tab_entry;
    ids *ids;
    int i, cnt_param, tab_size, ids_for_dots = 0;
    types *fundef_rettypes;
    char *tag;

    DBUG_ENTER ("COMPAp");

    ids = LET_IDS (INFO_COMP_LASTLET (arg_info));
    fundef_rettypes = FUNDEF_TYPES (AP_FUNDEF (arg_node));

    add_assigns_before = MakeNode (N_assign);
    add_assigns_after = MakeNode (N_assign);
    /*
     * will be used to store N_icms
     * for incrementation and decrementation
     * of refcounts
     */

    /*
     * Now, insert N_icms for incrementation of refcounts of function results
     * if any
     */
    first_assign = add_assigns_after;

    cnt_param = 0;

    tab_size = CountFunctionParams (AP_FUNDEF (arg_node)) + 2;
    icm_tab = (node **)Malloc (sizeof (node *) * tab_size);

    for (i = 0; i < tab_size; i++) {
        icm_tab[i] = NULL;
    }

    DBUG_PRINT ("COMP", ("COMPiling application of function %s",
                         ItemName (AP_FUNDEF (arg_node))));

    while (ids != NULL) {
        /*
         * First, we check if this variable is used as function argument
         * as well.
         */

        DBUG_PRINT ("COMP", ("Handling return value bound to %s", IDS_NAME (ids)));

        tmp = AP_ARGS (arg_node);

        while (tmp != NULL) {
            if ((NODE_TYPE (EXPRS_EXPR (tmp)) == N_id)
                && (0 == strcmp (ID_NAME (EXPRS_EXPR (tmp)), IDS_NAME (ids)))) {
                break;
            }

            tmp = EXPRS_NEXT (tmp);
        }

        if (MUST_REFCOUNT (IDS, ids)) {
            if (FUN_DOES_REFCOUNT (AP_FUNDEF (arg_node), cnt_param)) {
                id_node = MakeId3 (ids);
                tag_node = MakeId1 ("out_rc");

                if (IDS_REFCNT (ids) > 1) {
                    /*
                     * create N_icm to increment refcount of function result.
                     * It will be stored in refs_node->node[1]->.. and will be
                     * inserted later.
                     */
                    INC_RC_ND (id_node, MakeNum (ids->refcnt - 1));
                } else if (IDS_REFCNT (ids) == 0) {
                    if (IsNonUniqueHidden (IDS_TYPE (ids))) {
                        CREATE_2_ARY_ICM (next_assign, "ND_FREE_HIDDEN",
                                          MakeId1 (IDS_NAME (ids)),
                                          MakeId1 (GenericFun (1, IDS_TYPE (ids))));
                    } else {
                        CREATE_1_ARY_ICM (next_assign, "ND_FREE_ARRAY",
                                          MakeId1 (IDS_NAME (ids)));
                    }
                    APPEND_ASSIGNS (first_assign, next_assign);
                }
            } else {
                if ((tmp == NULL) || (IsUnique (VARDEC_TYPE (IDS_VARDEC (ids))))) {
                    id_node = MakeId3 (ids);
                } else {
                    id_node = MakeId1 (TmpVar ());

#if 0
          INFO_COMP_VARDECS( arg_info)
            = MakeVardec( StringCopy( ID_NAME( id_node)),
                          DuplicateTypes( VARDEC_TYPE( IDS_VARDEC( ids)), 1),
                          INFO_COMP_VARDECS( arg_info));
#else
                    INFO_COMP_VARDECS (arg_info)
                      = AddVardec (INFO_COMP_VARDECS (arg_info),
                                   VARDEC_TYPE (IDS_VARDEC (ids)), ID_NAME (id_node),
                                   INFO_COMP_FUNDEF (arg_info));
#endif

                    if (IsArray (VARDEC_TYPE (IDS_VARDEC (ids)))) {
                        CREATE_2_ARY_ICM (next_assign, "ND_KS_NO_RC_ASSIGN_ARRAY",
                                          MakeId1 (ID_NAME (id_node)),
                                          MakeId1 (IDS_NAME (ids)));
                    } else {
                        CREATE_2_ARY_ICM (next_assign, "ND_NO_RC_ASSIGN_HIDDEN",
                                          MakeId1 (ID_NAME (id_node)),
                                          MakeId1 (IDS_NAME (ids)));
                    }

                    APPEND_ASSIGNS (first_assign, next_assign);
                }

                if (IDS_REFCNT (ids) > 0) {
                    CREATE_1_ARY_ICM (next_assign, "ND_ALLOC_RC",
                                      MakeId1 (IDS_NAME (ids)));
                    APPEND_ASSIGNS (first_assign, next_assign);

                    CREATE_2_ARY_ICM (next_assign, "ND_SET_RC", MakeId1 (IDS_NAME (ids)),
                                      MakeNum (IDS_REFCNT (ids)));
                } else if (IsNonUniqueHidden (IDS_TYPE (ids))) {
                    CREATE_2_ARY_ICM (next_assign, "ND_NO_RC_FREE_HIDDEN",
                                      MakeId1 (IDS_NAME (ids)),
                                      MakeId1 (GenericFun (1, IDS_TYPE (ids))));
                } else {
                    CREATE_1_ARY_ICM (next_assign, "ND_NO_RC_FREE_ARRAY",
                                      MakeId1 (IDS_NAME (ids)));
                }
                APPEND_ASSIGNS (first_assign, next_assign);

                tag_node = MakeId1 ("out");
            }
        } else {
            id_node = MakeId3 (ids);
            tag_node = MakeId1 ("out");
        }

        if (ids_for_dots) {
            MAKE_NEXT_ICM_ARG (icm_arg, tag_node);
            MAKE_NEXT_ICM_ARG (icm_arg, id_node);

            if (IDS_NEXT (ids) == NULL) {
                InsertApDotsParam (icm_tab, icm_tab_entry);
            }
        } else if (TYPES_BASETYPE (fundef_rettypes) == T_dots) {
            ids_for_dots = 1;
            icm_arg = MakeExprs (tag_node, NULL);
            icm_tab_entry = icm_arg;
            MAKE_NEXT_ICM_ARG (icm_arg, id_node);

            if (IDS_NEXT (ids) == NULL) {
                InsertApDotsParam (icm_tab, icm_tab_entry);
            }
            cnt_param++;
        } else {
            icm_arg = MakeExprs (tag_node, NULL);
            icm_tab_entry = icm_arg;

            MAKE_NEXT_ICM_ARG (icm_arg, id_node);

            InsertApReturnParam (icm_tab, icm_tab_entry, VARDEC_TYPE (IDS_VARDEC (ids)),
                                 (FUNDEF_PRAGMA (AP_FUNDEF (arg_node)) == NULL)
                                   ? NULL
                                   : FUNDEF_LINKSIGN (AP_FUNDEF (arg_node)),
                                 cnt_param);

            fundef_rettypes = TYPES_NEXT (fundef_rettypes);
            cnt_param++;
        }

        ids = IDS_NEXT (ids);
    }

    /*
     *  Now, do the arguments of the function
     */

    exprs = AP_ARGS (arg_node);
    fundef_args = FUNDEF_ARGS (AP_FUNDEF (arg_node));
    ids_for_dots = 0;

    if (cnt_param == 0) {
        cnt_param = 1;
    }

    while (NULL != exprs) {
        DBUG_PRINT ("COMP", ("Handling argument #%d", cnt_param));

        next = EXPRS_NEXT (exprs);

        if ((N_id == NODE_TYPE (EXPRS_EXPR (exprs)))
            && (MUST_REFCOUNT (ID, EXPRS_EXPR (exprs)))) {
            if (FUN_DOES_REFCOUNT (AP_FUNDEF (arg_node), cnt_param)) {
                tag = ID_ATTRIB (EXPRS_EXPR (exprs)) == ST_inout ? "inout_rc" : "in_rc";
            } else {
                if (ID_ATTRIB (EXPRS_EXPR (exprs)) == ST_inout) {
                    if (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_Cfun) {
                        if (IsBoxed (ARG_TYPE (fundef_args))) {
                            tag = "upd_bx";
                        } else {
                            tag = "upd";
                        }
                    } else {
                        tag = "inout";
                    }
                } else {
                    tag = "in";
                }

                if (ID_REFCNT (EXPRS_EXPR (exprs)) > 1) {
                    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC",
                                      MakeId1 (ID_NAME (EXPRS_EXPR (exprs))),
                                      MakeNum (1));
                } else if (IsNonUniqueHidden (ID_TYPE (EXPRS_EXPR (exprs)))) {
                    CREATE_3_ARY_ICM (next_assign, "ND_DEC_RC_FREE_HIDDEN",
                                      MakeId1 (ID_NAME (EXPRS_EXPR (exprs))), MakeNum (1),
                                      MakeId1 (
                                        GenericFun (1, ID_TYPE (EXPRS_EXPR (exprs)))));
                } else {
                    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE_ARRAY",
                                      MakeId1 (ID_NAME (EXPRS_EXPR (exprs))),
                                      MakeNum (1));
                }

                ASSIGN_NEXT (next_assign) = ASSIGN_NEXT (add_assigns_after);
                ASSIGN_NEXT (add_assigns_after) = next_assign;

                if (first_assign == add_assigns_after) {
                    first_assign = next_assign;
                }

                /*
                 *  These refcount-ICMs must be inserted at the beginning
                 *  of the new ICMs chain.
                 */
            }
        } else {
            if ((NODE_TYPE (EXPRS_EXPR (exprs)) == N_id)
                && (ID_ATTRIB (EXPRS_EXPR (exprs)) == ST_inout)) {
                if (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_Cfun) {
                    if (IsBoxed (ARG_TYPE (fundef_args))) {
                        tag = "upd_bx";
                    } else {
                        tag = "upd";
                    }
                } else {
                    tag = "inout";
                }
            } else {
                tag = "in";
            }
        }

        if (ids_for_dots) {
            icm_arg = save_icm_arg;

            MAKE_NEXT_ICM_ARG (icm_arg, MakeId1 (tag));
            EXPRS_NEXT (icm_arg) = exprs;
            EXPRS_NEXT (exprs) = NULL;
            icm_arg = exprs;

            save_icm_arg = icm_arg;
            /*
             *  The value of icm_arg has to be saved here because a
             *  very intelligent programmer used this variable implicitly(!!)
             *  in macros like CREATE_3_ARY_ICM.
             */

            if (next == NULL) {
                InsertApDotsParam (icm_tab, icm_tab_entry);
            }
        } else if (ARG_BASETYPE (fundef_args) == T_dots) {
            ids_for_dots = 1;

            icm_arg = MakeExprs (MakeId1 (tag), NULL);
            icm_tab_entry = icm_arg;
            EXPRS_NEXT (icm_arg) = exprs;
            EXPRS_NEXT (exprs) = NULL;
            icm_arg = exprs;

            save_icm_arg = icm_arg;

            if (next == NULL) {
                InsertApDotsParam (icm_tab, icm_tab_entry);
            }
        } else {
            icm_tab_entry = MakeExprs (MakeId1 (tag), NULL);
            EXPRS_NEXT (icm_tab_entry) = exprs;
            EXPRS_NEXT (exprs) = NULL;

            InsertApArgParam (icm_tab, icm_tab_entry, ARG_TYPE (fundef_args),
                              ID_REFCNT (EXPRS_EXPR (exprs)), add_assigns_before,
                              FUNDEF_PRAGMA (AP_FUNDEF (arg_node)) == NULL
                                ? NULL
                                : FUNDEF_LINKSIGN (AP_FUNDEF (arg_node)),
                              cnt_param);

            fundef_args = ARG_NEXT (fundef_args);
        }

        exprs = next;
        cnt_param++;
    }

    INFO_COMP_LASTLET (arg_info)
      = CreateApIcm (INFO_COMP_LASTLET (arg_info), FUNDEF_NAME (AP_FUNDEF (arg_node)),
                     icm_tab, tab_size);

    AdjustAddedAssigns (add_assigns_before, add_assigns_after);

    /*
     *  First, new assign-nodes are inserted behind the current one.
     */

    last_assign = CURR_ASSIGN (arg_info);

    if (NULL != ASSIGN_NEXT (add_assigns_after)) {
        ASSIGN_NEXT (first_assign) = NEXT_ASSIGN (arg_info);
        ASSIGN_NEXT (last_assign) = ASSIGN_NEXT (add_assigns_after);
    }

    /*
     *  Second, new assign-nodes are inserted before the current one.
     */

    if (ASSIGN_NEXT (add_assigns_before) != NULL) {
        INSERT_BEFORE (arg_info, ASSIGN_NEXT (add_assigns_before));

        ASSIGN_NEXT (add_assigns_before)
          = AppendAssign (ASSIGN_NEXT (add_assigns_before), last_assign);
    }

    FREE (add_assigns_after);
    FREE (add_assigns_before);
    FREE (icm_tab);

    DBUG_RETURN (ICM_ARGS (INFO_COMP_LASTLET (arg_info)));
}

/******************************************************************************
 *
 * function:
 *   node *COMPNormalFunReturn(node *arg_node, node *arg_info)
 *
 * description:
 *   generates ICMs for N_return-node found in body of a non-SPMD-function.
 *
 ******************************************************************************/

node *
COMPNormalFunReturn (node *arg_node, node *arg_info)
{
    node *next, *exprs, *last, *ret;
    int cnt_param;

    DBUG_ENTER ("COMPNormalFunReturn");

    ret = RenameReturn (arg_node, arg_info);
    exprs = RETURN_EXPRS (arg_node);
    last = arg_node;
    /*
     * The new N_exprs chain will be stored in arg_node->node[1]
     * temporarily due to INSERT_ID_NODE and initial setting of 'last'.
     */

    /*
     * First, the real return values are traversed.
     */

    DBUG_PRINT ("COMP", ("Starting evaluation of return parameters"));

    DBUG_ASSERT ((arg_node->node[1] == NULL), "node[1] already used");

    cnt_param = 0;

    while (NULL != exprs) {
        DBUG_ASSERT ((N_id == NODE_TYPE (EXPRS_EXPR (exprs))),
                     " wrong node type (!= N_id)");

        DBUG_PRINT ("COMP", ("Current return id: %s", ID_NAME (EXPRS_EXPR (exprs))));

        next = EXPRS_NEXT (exprs);

        if (MUST_REFCOUNT (ID, EXPRS_EXPR (exprs))) {
            INSERT_ID_NODE (exprs, last, "out_rc");
        } else {
            INSERT_ID_NODE (exprs, last, "out");
        }

        /*
         *  In COMPReturn, we don't have to distinguish between functions
         *  that do the refcounting on their own and those that do not
         *  because we're definitely inside a SAC function and these
         *  always do their own refcounting.
         */

        last = exprs;
        exprs = next;
        cnt_param++;
    }

    DBUG_PRINT ("COMP", ("Handled original return values finished"));

    exprs = RETURN_REFERENCE (arg_node);

    /*
     * Second, the counterparts of reference parameters are traversed
     * and added to the chain.
     */

    while (NULL != exprs) {
        DBUG_ASSERT ((N_id == NODE_TYPE (EXPRS_EXPR (exprs))), " wrong node (!= N_id)");

        DBUG_PRINT ("COMP", ("Current return id: %s", ID_NAME (EXPRS_EXPR (exprs))));

        next = EXPRS_NEXT (exprs);

        if (MUST_REFCOUNT (ID, EXPRS_EXPR (exprs))) {
            INSERT_ID_NODE (exprs, last, "inout_rc");
        } else {
            INSERT_ID_NODE (exprs, last, "inout");
        }

        last = exprs;
        exprs = next;
    }

    DBUG_PRINT ("COMP", ("Handling counterparts of reference parameters finished"));

    NODE_TYPE (arg_node) = N_icm;
    ICM_INDENT (arg_node) = 0;

    if (arg_node->node[1] == NULL) {
        ICM_NAME (arg_node) = "NOOP";
    } else {
        ICM_NAME (arg_node) = "ND_FUN_RET";

        exprs = MakeNode (N_exprs);
        EXPRS_EXPR (exprs) = MakeNum (0);
        EXPRS_NEXT (exprs) = arg_node->node[1];
        /* put number of ret-values in front */
        RETURN_EXPRS (arg_node) = exprs;
        arg_node->node[1] = NULL; /* was only used temporarily */

        exprs = MakeExprs (NULL, NULL);
        EXPRS_EXPR (exprs) = MakeId1 ("");
        EXPRS_NEXT (exprs) = RETURN_EXPRS (arg_node);
        RETURN_EXPRS (arg_node) = exprs;
    }

    arg_node = ret; /* set new return_value of current function
                     * (N_let node if at least one variable in the "return"
                     * statement has been renamed, or  N_return otherwise)
                     */

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPSpmdFunReturn(node *arg_node, node *arg_info)
 *
 * description:
 *   generates ICMs for N_return-node found in body of a SPMD-function.
 *
 ******************************************************************************/

node *
COMPSpmdFunReturn (node *arg_node, node *arg_info)
{
    node *exprs, *args, *last_arg, *tag;
    int cnt_params;

    DBUG_ENTER ("COMPSpmdFunReturn");

    exprs = RETURN_EXPRS (arg_node);
    args = NULL;
    cnt_params = 0;

    while (exprs != NULL) {
        DBUG_ASSERT ((N_id == NODE_TYPE (EXPRS_EXPR (exprs))), "wrong node type found");

        if (ID_REFCNT (EXPRS_EXPR (exprs)) >= 0) {
            tag = MakeExprs (MakeId ("out_rc", NULL, ST_regular), NULL);
        } else {
            tag = MakeExprs (MakeId ("out", NULL, ST_regular), NULL);
        }

        if (args == NULL) {
            args = tag;
        } else {
            EXPRS_NEXT (last_arg) = tag;
        }
        EXPRS_NEXT (tag) = last_arg = exprs;

        exprs = EXPRS_NEXT (exprs);
        cnt_params++;
    }

    args = MakeExprs (MakeNum (cnt_params), args);
    args = MakeExprs (MakeNum (barrier_id), args);

    arg_node = MakeIcm1 ("MT_SPMD_FUN_RET", args);
    FUNDEF_RETURN (INFO_COMP_FUNDEF (arg_info)) = arg_node;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPReturn(node *arg_node, node *arg_info)
 *
 * description:
 *   generates N_icms for N_return of a function (ND or MT).
 *
 ******************************************************************************/
node *
COMPReturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("COMPReturn");

    if (FUNDEF_STATUS (INFO_COMP_FUNDEF (arg_info)) == ST_spmdfun) {
        arg_node = COMPSpmdFunReturn (arg_node, arg_info);
    } else {
        arg_node = COMPNormalFunReturn (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPArg
 *  arguments     : 1) N_arg node
 *                  2) info node
 *  description   :
 *  remarks       : INFO_COMP_FIRSTASSIGN(arg_info) contains pointer to first
 *                  assignment of block.
 *                  INFO_COMP_FUNDEF(arg_info) contains pointer to fundef.
 *                  INFO_COMP_CNTPARAM(arg_info) contains parameter number
 *
 */

node *
COMPArg (node *arg_node, node *arg_info)
{
    node *icm_arg, *new_assign, *icm_tab_entry, *type_id_node;
    char *id_name;
    types *fulltype;
    char *tag;

    DBUG_ENTER ("COMPArg");

    GET_BASIC_TYPE (fulltype, ARG_TYPE (arg_node), 042);

    id_name = (NULL != ARG_NAME (arg_node)) ? ARG_NAME (arg_node) : "";
    /* store name of formal parameter */

    type_id_node = MakeId1 (MakeTypeString (fulltype));

    if ((MUST_REFCOUNT (ARG, arg_node))
        && (FUN_DOES_REFCOUNT (INFO_COMP_FUNDEF (arg_info),
                               INFO_COMP_CNTPARAM (arg_info)))) {

        if ((ARG_ATTRIB (arg_node) == ST_inout)
            || (ARG_ATTRIB (arg_node) == ST_spmd_inout)) {
            tag = "inout_rc";
        } else {
            tag = "in_rc";
        }

        /*
         * put "ND_INC_RC" ICMs at beginning of function block
         * if the function is *not* a SPMD-function
         */

        if (FUNDEF_STATUS (INFO_COMP_FUNDEF (arg_info)) != ST_spmdfun) {
            if (1 < ARG_REFCNT (arg_node)) {
                CREATE_2_ARY_ICM (new_assign, "ND_INC_RC", MakeId1 (id_name),
                                  MakeNum (ARG_REFCNT (arg_node) - 1));

                ASSIGN_NEXT (new_assign) = INFO_COMP_FIRSTASSIGN (arg_info);
                INFO_COMP_FIRSTASSIGN (arg_info) = new_assign;
            } else {
                if (0 == ARG_REFCNT (arg_node)) {
                    if (IsNonUniqueHidden (ARG_TYPE (arg_node))) {
                        CREATE_3_ARY_ICM (new_assign, "ND_DEC_RC_FREE_HIDDEN",
                                          MakeId1 (id_name), MakeNum (1),
                                          MakeId1 (GenericFun (1, ARG_TYPE (arg_node))));
                    } else {
                        CREATE_2_ARY_ICM (new_assign, "ND_DEC_RC_FREE_ARRAY",
                                          MakeId1 (id_name), MakeNum (1));
                    }
                    ASSIGN_NEXT (new_assign) = INFO_COMP_FIRSTASSIGN (arg_info);
                    INFO_COMP_FIRSTASSIGN (arg_info) = new_assign;
                }
            }
        }
    } else {
        if ((FUNDEF_PRAGMA (INFO_COMP_FUNDEF (arg_info)) != NULL)
            && (FUNDEF_REFCOUNTING (INFO_COMP_FUNDEF (arg_info)) != NULL)
            && (FUNDEF_REFCOUNTING (
                  INFO_COMP_FUNDEF (arg_info))[INFO_COMP_CNTPARAM (arg_info)]
                == 1)) {
            WARN (NODE_LINE (arg_node), ("Pragma 'refcounting` illegal"));
            CONT_WARN (("Function wants to do refcounting on non-refcounted "
                        "parameter no. %d",
                        INFO_COMP_CNTPARAM (arg_info)));
        }

        if (ARG_ATTRIB (arg_node) == ST_inout) {
            if (FUNDEF_STATUS (INFO_COMP_FUNDEF (arg_info)) == ST_Cfun) {
                if (IsBoxed (ARG_TYPE (arg_node))) {
                    tag = "upd_bx";
                } else {
                    tag = "upd";
                }
            } else {
                tag = "inout";
            }
        } else {
            tag = "in";
        }
    }

    icm_arg = MakeExprs (MakeId1 (tag), NULL);
    icm_tab_entry = icm_arg;
    MAKE_NEXT_ICM_ARG (icm_arg, type_id_node);
    MAKE_NEXT_ICM_ARG (icm_arg, MakeId1 (id_name));

    /*
     * store args in 'icm_tab'
     */

    if (ARG_BASETYPE (arg_node) == T_dots) {
        InsertDefDotsParam (INFO_COMP_ICMTAB (arg_info), icm_tab_entry);
    } else {
        InsertDefArgParam (INFO_COMP_ICMTAB (arg_info), icm_tab_entry,
                           INFO_COMP_TYPETAB (arg_info), ARG_TYPE (arg_node),
                           NULL == FUNDEF_PRAGMA (INFO_COMP_FUNDEF (arg_info))
                             ? NULL
                             : FUNDEF_LINKSIGN (INFO_COMP_FUNDEF (arg_info)),
                           INFO_COMP_CNTPARAM (arg_info), NODE_LINE (arg_node));
    }

    INFO_COMP_CNTPARAM (arg_info)++;

    if (NULL != ARG_NEXT (arg_node)) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    /*
     * Additional icms for the function body are generated regardless of the
     * existence of such a block, but COMPFundef only inserts them if a
     * block already exists.
     */

    /*
     * put "ND_KS_DECL_ARRAY_ARG" ICMs at beginning of function block
     */

    if (TYPES_DIM (fulltype) > 0) {
        node *dim_node, *shape;
        int i;

        /* store dim and shape */
        dim_node = MakeNum (TYPES_DIM (fulltype));
        CREATE_2_ARY_ICM (new_assign, "ND_KS_DECL_ARRAY_ARG", MakeId1 (id_name),
                          dim_node);

        for (i = 0; i < TYPES_DIM (fulltype); i++) {
            shape = MakeNum (TYPES_SHAPE (fulltype, i));
            MAKE_NEXT_ICM_ARG (icm_arg, shape);
        }

        /*
         * now put node at beginning of function block
         */

        ASSIGN_NEXT (new_assign) = INFO_COMP_FIRSTASSIGN (arg_info);
        INFO_COMP_FIRSTASSIGN (arg_info) = new_assign;
    }

    /*
     * put "ND_DECL_INOUT_PARAM" or "ND_DECL_INOUT_PARAM_RC" ICMs respectively
     * at beginning of function block
     */

    if (ARG_ATTRIB (arg_node) == ST_inout) {
        if (MUST_REFCOUNT (ARG, arg_node)) {
            CREATE_2_ARY_ICM (new_assign, "ND_DECL_INOUT_PARAM_RC", type_id_node,
                              MakeId1 (id_name));
        } else {
            CREATE_2_ARY_ICM (new_assign, "ND_DECL_INOUT_PARAM", type_id_node,
                              MakeId1 (id_name));
        }

        /*
         * now put node at beginning of function block
         */

        ASSIGN_NEXT (new_assign) = INFO_COMP_FIRSTASSIGN (arg_info);
        INFO_COMP_FIRSTASSIGN (arg_info) = new_assign;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPLoop(node *arg_node, node *arg_info)
 *
 * description:
 *   I brushed the whole function and named the variables ingenious, but I
 *   still do not unterstand the whole exercise. But I think the following:
 *     Compiling a loop is easy, but here are refcount-corrections done also.
 *   Special ids-chains are inferred in refcounting and now used to produce
 *   corecctions in the body and after the loop. Thats all I think ... (jhs)
 *
 * attention:
 *   This function is used to compile N_do *and* N_while nodes, meaning it
 *   compiles do- *and* while-loops!
 *
 * remark (only text of the original comment):
 *   INFO_COMP_LASTASSIGN(arg_info) contains pointer to last assign_node.
 *
 ******************************************************************************/
node *
COMPLoop (node *arg_node, node *arg_info)
{
    node *first_assign;
    node *next_assign;
    node *dummy_assign;
    node *icm_arg; /* necessary due usage of macros CREATE_x_ARY_ICM */
    node *loop_assign;
    node *label;
    node *tmp;
    int found; /* bool */
    ids *usevar;
    ids *defvar;

    DBUG_ENTER ("COMPLoop");

    /* first compile termination condition and body of loop */
    loop_assign = CURR_ASSIGN (arg_info);
    L_DO_OR_WHILE_COND (arg_node, (Trav (DO_OR_WHILE_COND (arg_node), arg_info)));
    L_DO_OR_WHILE_BODY (arg_node, (Trav (DO_OR_WHILE_BODY (arg_node), arg_info)));

    /* now add some DEC_RC at begining of and after the loop */

    /*
     *  Build icm's and insert them *before* the first instruction in the loop.
     *  The following parts are done:
     *    (1) Build DEC_RC-icms
     *    (2) Build ND-label-icm (do-loop only)
     *    (3) Build INC_RC-icms
     *    (4) Insert all icms at beginning of inner instructions.
     */
    dummy_assign = MakeAssign (NULL, NULL);
    first_assign = dummy_assign;
    /*
     *  Before (1) - Build DEC_RC-icms.
     *
     *  All variables defined but not used in the loop where counted during
     *  refcounting with 1 (for while-loops) resp. with 0 (for do-loops).
     *
     *  The variables inspected are defined in the loop (used after it) but not
     *  used in the loop itself. In context of a while-loop this means the
     *  variable has been set before the loop, otherwise, any usage after the
     *  loop isn't inferable if the loop is not executed. The do-loop hos no
     *  such problem.
     *
     *  Such an defined but not used variable will be "overwritten" in another
     *  following pass of the loop body, therefore the variables refcounters have
     *  to be decremented before each pass.i Thats is what the icms here are
     *  produced for. Because the first-pass of a do-loop does not involve any
     *  predefined variables, it is necessary to jump around this part in such a
     *  pass (therefore this obscure goto-label-construct.
     */
    if (DO_OR_WHILE_DEFVARS (arg_node) != NULL) {
        defvar = DO_OR_WHILE_DEFVARS (arg_node);
        while (defvar != NULL) {
            /*
             *  looking if defvar is in DEFVARS \ USEVARS,
             *  meaning (! found) in usevars.
             */
            found = FALSE;
            usevar = DO_OR_WHILE_USEVARS (arg_node);
            while ((usevar != NULL) && (!found)) {
                found = found || (strcmp (IDS_NAME (usevar), IDS_NAME (defvar)) == 0);
                usevar = IDS_NEXT (usevar);
            }
            if (!found) {
                /*
                 *  we don`t know the refcount of defvar in the current context,
                 *  so we use ND_DEC_RC_FREE_ARRAY.
                 */
                if (IsNonUniqueHidden (IDS_TYPE (defvar))) {
                    CREATE_3_ARY_ICM (next_assign, "ND_DEC_RC_FREE_HIDDEN",
                                      MakeId2 (DupOneIds (defvar, NULL)), MakeNum (1),
                                      MakeId1 (GenericFun (1, IDS_TYPE (defvar))));
                } else {
                    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE_ARRAY",
                                      MakeId2 (DupOneIds (defvar, NULL)), MakeNum (1));
                }

                APPEND_ASSIGNS (first_assign, next_assign);
            }

            defvar = IDS_NEXT (defvar);
        }
    }
    /*
     *  Before (2) - Build ND-label-icm (do-loop-only).
     *
     *  Needed to aviod the above DEC_RC's in the first pass of a do_loop.
     *  See explanations above.
     */
    if (N_do == NODE_TYPE (arg_node)) {
        label_nr++;
        label = MakeId1 (GenName (label_nr, LABEL_NAME));
        CREATE_1_ARY_ICM (next_assign, "ND_LABEL", label);
        APPEND_ASSIGNS (first_assign, next_assign);
    }
    /*
     *  Before (3) - Build INC_RC-icms.
     *
     *  The variable used in the loop were refcounted once for the loop (plus
     *  their occurences in the rest of the programm if not also defined in the loop).
     *  If the loop is not executed this counter is reduced (icm's builded somewhere
     *  else). But if the loop is executed the refcounters have to be adjusted to
     *  the number of usages in the loop plus possible usage in another pass of the
     *  loop. The extra counter for a next pass is also reduced after the loop.
     */
    usevar = DO_OR_WHILE_USEVARS (arg_node);
    while (usevar != NULL) {
        if (IDS_REFCNT (usevar) > 1) {
            CREATE_2_ARY_ICM (next_assign, "ND_INC_RC",
                              MakeId2 (DupOneIds (usevar, NULL)),
                              MakeNum (IDS_REFCNT (usevar) - 1));
            APPEND_ASSIGNS (first_assign, next_assign);
        }
        usevar = IDS_NEXT (usevar);
    }
    /*
     *  Before (4) - Insert icms at beginning of loop instructions.
     */
    if (ASSIGN_NEXT (dummy_assign) != NULL) {
        ASSIGN_NEXT (first_assign) = DO_OR_WHILE_INSTR (arg_node);
        L_DO_OR_WHILE_INSTR (arg_node, ASSIGN_NEXT (dummy_assign));
        ASSIGN_NEXT (dummy_assign) = NULL;
    }

    /*
     *  Build icm's and insert them *after* the loop-assignment.
     *  The following parts are done:
     *    (1) Build DEC_RC-icms
     *    (2) Build INC_RC-icms
     *    (3) Insert all icms after the loop-assignment
     *    (4) Insert GOTO (do-loop only)
     */
    /* dummy_assign is reused from above */
    first_assign = dummy_assign;

    /*
     *  After (1) - Build DEC_RC-icms.
     */
    if (DO_OR_WHILE_USEVARS (arg_node) != NULL) {
        usevar = DO_OR_WHILE_USEVARS (arg_node);
        while (usevar != NULL) {
            /*
             *  looking if usevar is in USEVARS \ DEFVARS,
             *  meaning (! found) in usevars
             */
            found = FALSE;
            defvar = DO_OR_WHILE_DEFVARS (arg_node);
            while ((defvar != NULL) && (!found)) {
                found = found || (strcmp (IDS_NAME (usevar), IDS_NAME (defvar)) == 0);
                defvar = IDS_NEXT (defvar);
            }

            if (!found) {
#if 0
        if (IDS_REFCNT( usevar) > 1) {  
            /*
             * sbs: Initially, a ND_DEC_RC icm was built here!
             *      (which actually forced the introduction of the conditional here)
             *      Unfortunately, the refcount of usevar in some situations seems
             *      to be >1 although this dynamically turns out to be the last ref!
             *      Therefore, I changed ND_DEC_RC into ND_DEC_RC_FREE_ARRAY!
             *      The example, where things went wrong is APLtomcatv; in that
             *      program x and y of the main tomcatv loop would not be set
             *      free at last!!!
             */
          CREATE_2_ARY_ICM( next_assign, "ND_DEC_RC_FREE_ARRAY",
                            MakeId2(DupOneIds(usevar, NULL)), 
                            MakeNum( 1));
          APPEND_ASSIGNS( first_assign, next_assign);
        }
        else if (IDS_REFCNT( usevar) > 0) {
          CREATE_2_ARY_ICM( next_assign, "ND_DEC_RC_FREE_ARRAY",
                            MakeId2(DupOneIds(usevar, NULL)), 
                            MakeNum( 1));
          APPEND_ASSIGNS( first_assign, next_assign);
        }
#else
                if (IDS_REFCNT (usevar) > 0) {
                    if (IsNonUniqueHidden (IDS_TYPE (usevar))) {
                        CREATE_3_ARY_ICM (next_assign, "ND_DEC_RC_FREE_HIDDEN",
                                          MakeId2 (DupOneIds (usevar, NULL)), MakeNum (1),
                                          MakeId1 (GenericFun (1, IDS_TYPE (usevar))));
                    } else {
                        CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE_ARRAY",
                                          MakeId2 (DupOneIds (usevar, NULL)),
                                          MakeNum (1));
                    }
                    APPEND_ASSIGNS (first_assign, next_assign);
                }
#endif
            }
            usevar = IDS_NEXT (usevar);
        }
    }

    /*
     *  After (2) - Build INC_RC-icms.
     *              They increase RC of arrays that are defined
     *              in the loop and are used after it.
     */
    defvar = DO_OR_WHILE_DEFVARS (arg_node);
    while (defvar != NULL) {
        if (IDS_REFCNT (defvar) > 1) {
            CREATE_2_ARY_ICM (next_assign, "ND_INC_RC",
                              MakeId2 (DupOneIds (defvar, NULL)),
                              MakeNum (IDS_REFCNT (defvar) - 1));
            APPEND_ASSIGNS (first_assign, next_assign);
        }
        defvar = IDS_NEXT (defvar);
    }

    /*
     *  After (3) - Insert icms after end of loop-assigment
     *              (before next instruction).
     */
    if (ASSIGN_NEXT (dummy_assign) != NULL) {
        if (ASSIGN_NEXT (loop_assign) != NULL) {
            ASSIGN_NEXT (first_assign) = ASSIGN_NEXT (loop_assign);
        }
        ASSIGN_NEXT (loop_assign) = ASSIGN_NEXT (dummy_assign);
    }
    FREE (dummy_assign);

    /*
     *  After (4) - Insert GOTO before do-loop (do-loop only).
     */
    if (NODE_TYPE (arg_node) == N_do) {
        /* put N_icm 'ND_GOTO', in front of N_do node */
        CREATE_1_ARY_ICM (first_assign, "ND_GOTO", label);
        if (NULL != ASSIGN_NEXT (loop_assign)) {
            /* next assign after do-loop */
            ASSIGN_NEXT (first_assign) = ASSIGN_NEXT (loop_assign);
        }

        /* only temporary used (N_do): */
        tmp = ASSIGN_INSTR (loop_assign);
        /* N_icm (ND_GOTO) node: */
        ASSIGN_INSTR (loop_assign) = ASSIGN_INSTR (first_assign);
        ASSIGN_NEXT (loop_assign) = first_assign;
        arg_node = ASSIGN_INSTR (first_assign);
        /* put N_do node: */
        ASSIGN_INSTR (first_assign) = tmp;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPCond(node *arg_node, node *arg_info)
 *
 * description:
 *   Compiling a conditional, that should be easy, except for the fact
 *   that refcount corrections are needed in both branches.
 *
 ******************************************************************************/

node *
COMPCond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("COMPCond");

    /* compile condition, then and else part */
    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);
    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

    /*
     *  Insert N_icms to correct refcounts of then and else part.
     *  The corrections are inserted before the actual instructions
     *  of both branches. The corretions are refcount-decrements, which
     *  parameters (variable and count) are taken from THENVARS and ELSEVARS
     *  (ids-chains), the refcounter-values of these ids tell how many
     *  refcounts shall be consumed.                                    (jhs)
     */

    if (COND_THENVARS (arg_node) != NULL) {
        BLOCK_INSTR (COND_THEN (arg_node))
          = MakeDecRcICMs (COND_THENVARS (arg_node), BLOCK_INSTR (COND_THEN (arg_node)));
    }

    if (COND_ELSEVARS (arg_node) != NULL) {
        BLOCK_INSTR (COND_ELSE (arg_node))
          = MakeDecRcICMs (COND_ELSEVARS (arg_node), BLOCK_INSTR (COND_ELSE (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPTypedef
 *  arguments     : 1) arg node
 *                  2) info node
 *  description   : transforms N_typedef to N_icm if it is a definition of an
 *                  array
 *  remarks       :
 *
 */

node *
COMPTypedef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("COMPTypedef");

    if (0 != TYPEDEF_DIM (arg_node)) {
        /*
         * Here, a new type is defined as a composite type
         * (for the time being an array type!)
         * Therefore, we have to translate the typedef-node
         * into an ICM-node "ND_TYPEDEF_ARRAY" ....
         */
        node *type1, *type2, *icm_node;

        type1 = MakeId1 (type_string[TYPEDEF_BASETYPE (arg_node)]);
        type2 = MakeId1 (TYPEDEF_NAME (arg_node));

        icm_node = MakeIcm2 ("ND_TYPEDEF_ARRAY", type1, type2);

        /*
         * Now, we free the old typedef node and implicitly set arg_node to the
         * TYPEDEF_NEXT(arg_node)!
         */
        arg_node = FreeNode (arg_node);

        if (arg_node != NULL) {
            ICM_NEXT (icm_node) = Trav (arg_node, arg_info);
        }
        arg_node = icm_node; /* for returning the new node 8-) */
    } else {
        /*
         * arg_node defines a type-synonym of a simple type and
         * therefore has not to be translated into an ICM!!
         * Hence, we simply traverse the next typedef!
         */
        if (TYPEDEF_NEXT (arg_node) != NULL) {
            TYPEDEF_NEXT (arg_node) = Trav (TYPEDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPObjdef
 *  arguments     : 1) N_objdef node
 *                  2) arg_info unused
 *  description   : The N_objdef node is replaced if the object's type
 *                  is an array.
 *  remarks       :
 *
 */

node *
COMPObjdef (node *arg_node, node *arg_info)
{
    node *icm, *icm_arg, *type_id_node, *id_node;
    int i;
    types *full_type;

    DBUG_ENTER ("COMPObjdef");

    if (IsArray (OBJDEF_TYPE (arg_node))) {
        icm = MakeIcm (NULL, NULL, NULL);

        GET_BASIC_TYPE (full_type, OBJDEF_TYPE (arg_node), 042);

        type_id_node = MakeId1 (type_string[TYPES_BASETYPE (full_type)]);
        icm_arg = MakeExprs (type_id_node, NULL);
        ICM_ARGS (icm) = icm_arg;

        id_node = MakeId1 (OBJDEF_NAME (arg_node));
        MAKE_NEXT_ICM_ARG (icm_arg, id_node);

        MAKE_NEXT_ICM_ARG (icm_arg, MakeNum (TYPES_DIM (full_type)));

        if ((OBJDEF_STATUS (arg_node) == ST_imported_mod)
            || (OBJDEF_STATUS (arg_node) == ST_imported_class)) {
            ICM_NAME (icm) = "ND_KD_DECL_EXTERN_ARRAY";
        } else {
            ICM_NAME (icm) = "ND_KS_DECL_GLOBAL_ARRAY";

            for (i = 0; i < TYPES_DIM (full_type); i++) {
                MAKE_NEXT_ICM_ARG (icm_arg, MakeNum (TYPES_SHAPE (full_type, i)));
            }
        }

        OBJDEF_ICM (arg_node) = icm;
    } else {
        OBJDEF_ICM (arg_node) = NULL;
    }

    OBJDEF_EXPR (arg_node) = NULL;
    /*
     *  The initialization expression is not freed because it may be used
     *  in the main function.
     */

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *BuildParamsByDFM( DFMmask_t *mask,
 *                           char *tag, int *num_args, node *icm_args)
 *
 * description:
 *   Builds triplet-chain (tag, type, name) from dfm-mask mask,
 *   tag will we used as base for the tags (used raw or _rc is added),
 *   num_args will be incremented for each triplet added (maybe NULL),
 *   at the end of this chain icm_args will be concatenated.
 *
 ******************************************************************************/

node *
BuildParamsByDFM (DFMmask_t *mask, char *tag, int *num_args, node *icm_args)
{
    node *vardec;
    char *rc_tag, *this_tag;

    DBUG_ENTER ("BuildParamsByDFM");

    rc_tag = StringConcat (tag, "_rc");

    vardec = DFMGetMaskEntryDeclSet (mask);
    while (vardec != NULL) {

        DBUG_PRINT ("JHS", ("%s", NODE_TEXT (vardec)));
        DBUG_PRINT ("JHS", ("%s", VARDEC_OR_ARG_NAME (vardec)));
        /*
            if (VARDEC_OR_ARG_REFCNT( vardec) >= 0) {
              this_tag = rc_tag;
            } else {
              this_tag = tag;
            }
            icm_args = MakeExprs( MakeId1( this_tag),
                       MakeExprs( MakeId1( MakeTypeString(VARDEC_OR_ARG_TYPE( vardec))),
                       MakeExprs( MakeId1( VARDEC_OR_ARG_NAME( vardec)),
                                  icm_args)));
            if (num_args != NULL) {
              *num_args = *num_args + 1;
            }

            if (num_args != NULL) {
              DBUG_PRINT("SPMD", ("bpbdfm num_args:%i %s",
                                  *num_args, VARDEC_OR_ARG_NAME( vardec )));
            } else {
              DBUG_PRINT("SPMD", ("bpbdfm num_args:- %s",
                                  VARDEC_OR_ARG_NAME( vardec )));
            }
        */

        vardec = DFMGetMaskEntryDeclSet (NULL);
    }

    free (rc_tag);

    DBUG_RETURN (icm_args);
}

/******************************************************************************
 *
 * function:
 *   node *COMPSpmd( node *arg_node, node *arg_info)
 *
 * description:
 *   compiles a N_spmd node.
 *
 ******************************************************************************/

node *
COMPSpmd (node *arg_node, node *arg_info)
{
    node *fundef, *icm_args, *assigns;
    int num_args;

    DBUG_ENTER ("COMPSpmd");

    /*
     * Compile original code within spmd-block in order to produce sequential
     * code version.
     */
    SPMD_ICM_SEQUENTIAL (arg_node) = Trav (SPMD_REGION (arg_node), arg_info);

    /*
     * build ICM for SPMD-region
     */

    SPMD_ICM_BEGIN (arg_node)
      = MakeIcm1 ("MT_SPMD_BEGIN", MakeId1 (FUNDEF_NAME (SPMD_FUNDEF (arg_node))));
    SPMD_ICM_ALTSEQ (arg_node)
      = MakeIcm1 ("MT_SPMD_ALTSEQ", MakeId1 (FUNDEF_NAME (SPMD_FUNDEF (arg_node))));
    SPMD_ICM_END (arg_node)
      = MakeIcm1 ("MT_SPMD_END", MakeId1 (FUNDEF_NAME (SPMD_FUNDEF (arg_node))));

    /*
     * Now, build up the ICMs of the parallel block.
     */

    fundef = INFO_COMP_FUNDEF (arg_info);

    assigns = NULL;

    assigns = MakeAssign (MakeIcm1 ("MT_SPMD_EXECUTE",
                                    MakeId1 (FUNDEF_NAME (SPMD_FUNDEF (arg_node)))),
                          assigns);

    /*
     * Now, build up the arguments for MT_SPMD_SETUP ICM.
     */

    num_args = 0;
    icm_args = BLOCK_SPMD_SETUP_ARGS (FUNDEF_BODY (SPMD_FUNDEF (arg_node)));
    while (icm_args != NULL) {
        num_args++;
        icm_args = ICM_NEXT (icm_args);
    }
    num_args = num_args / 3;

    DBUG_PRINT ("COMPi", ("catched num_args %i", num_args));

    /*  icm_args = BLOCK_SPMD_SETUP_ARGS(FUNDEF_BODY(SPMD_FUNDEF(arg_node))); */
    num_args = 0;
    icm_args = NULL;
    icm_args = BuildParamsByDFM (SPMD_IN (arg_node), "in", &num_args, icm_args);
    icm_args = BuildParamsByDFM (SPMD_OUT (arg_node), "out", &num_args, icm_args);
    icm_args = BuildParamsByDFM (SPMD_SHARED (arg_node), "shared", &num_args, icm_args);

    icm_args = MakeExprs (MakeId1 (FUNDEF_NAME (SPMD_FUNDEF (arg_node))),
                          MakeExprs (MakeNum (num_args), icm_args));

    assigns = MakeAssign (MakeIcm1 ("MT_SPMD_SETUP", icm_args), assigns);

    assigns = AppendAssign (BLOCK_SPMD_PROLOG_ICMS (FUNDEF_BODY (SPMD_FUNDEF (arg_node))),
                            assigns);

    SPMD_ICM_PARALLEL (arg_node) = MakeBlock (assigns, NULL);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPSync( node *arg_node, node *arg_info)
 *
 * description:
 *   compiles a N_sync node:
 *
 *     < malloc-ICMs >                   // if (FIRST == 0) only
 *     MT_CONTINUE( ...)                 // if (FIRST == 0) only
 *     MT_START_SYNCBLOCK( ...)
 *     < with-loop code without malloc/free-ICMs >
 *     MT_SYNC...( ...)
 *     < free-ICMs >
 *
 ******************************************************************************/

node *
COMPSync (node *arg_node, node *arg_info)
{
    node *icm_args, *icm_args3, *vardec, *with, *block, *instr, *assign, *last_assign,
      *prolog_icms, *epilog_icms, *new_icm, *setup_args, *assigns = NULL;
    ids *with_ids;
    types *type;
    simpletype s_type;
    char *tag, *icm_name, *var_name, *fold_type;
    int num_args, count_nesting;

    int prolog; /* used as boolean */
    int epilog; /* used as boolean */

    node *backup;
    node *let;
    node *last_sync;

    int num_fold_args;
    node *fold_args;

    int num_sync_args;
    node *sync_args;

    int num_barrier_args;
    node *barrier_args;

    DBUG_ENTER ("COMPSync");

    /*
     * build arguments of ICM 'MT_START_SYNCBLOCK'
     */
    icm_args3 = NULL;
    num_args = 0;
    vardec = DFMGetMaskEntryDeclSet (SYNC_IN (arg_node));
    while (vardec != NULL) {
        if (VARDEC_OR_ARG_REFCNT (vardec) >= 0) {
            tag = "in_rc";
        } else {
            tag = "in";
        }
        icm_args3
          = AppendExprs (icm_args3,
                         MakeExprs (MakeId1 (tag),
                                    MakeExprs (MakeId1 (MakeTypeString (
                                                 VARDEC_OR_ARG_TYPE (vardec))),
                                               MakeExprs (MakeId1 (
                                                            VARDEC_OR_ARG_NAME (vardec)),
                                                          NULL))));
        num_args++;

        vardec = DFMGetMaskEntryDeclSet (NULL);
    }
    icm_args3 = MakeExprs (MakeNum (num_args), icm_args3);

    DBUG_PRINT ("COMPi", ("--- Enter sync ---"));

    DBUG_PRINT ("COMPi", ("Enter sync-args"));

    /* inter-thread sync parameters */
    sync_args = NULL;
    num_sync_args = 0;
    vardec = DFMGetMaskEntryDeclSet (SYNC_INOUT (arg_node));
    while (vardec != NULL) {

        sync_args = AppendExprs (sync_args,
                                 MakeExprs (MakeId1 (VARDEC_OR_ARG_NAME (vardec)), NULL));

        DBUG_PRINT ("COMPi", ("%s", VARDEC_OR_ARG_NAME (vardec)));

        num_sync_args++;
        vardec = DFMGetMaskEntryDeclSet (NULL);
    }
    sync_args = MakeExprs (MakeNum (num_sync_args), sync_args);

    DBUG_PRINT ("COMPi", ("Enter fold-args"));

    last_sync = INFO_COMP_LAST_SYNC (arg_info);
    fold_args = NULL;
    num_fold_args = 0;
    if (last_sync != NULL) {
        DBUG_PRINT ("COMPi", ("last-sync found"));
        /*
         *  Traverse assignments
         */
        assign = BLOCK_INSTR (SYNC_REGION (last_sync));

        while (assign != NULL) {
            DBUG_PRINT ("COMPi", ("assign found"));

            DBUG_ASSERT ((NODE_TYPE (assign) == N_assign), ("wrong node type"));

            let = ASSIGN_INSTR (assign);
            DBUG_ASSERT ((NODE_TYPE (let) == N_let), ("wrong node type"));

            with = LET_EXPR (let);
            /* #### comments missing */
            if (NODE_TYPE (with) == N_Nwith2) {
                DBUG_ASSERT ((NODE_TYPE (with) == N_Nwith2), ("wrong node type"));

                with_ids = LET_IDS (let);

                if ((NWITH2_TYPE (with) == WO_foldprf)
                    || (NWITH2_TYPE (with) == WO_foldfun)) {
                    num_fold_args++;

                    type = IDS_TYPE (with_ids);
                    if (TYPES_DIM (type) > 0) {
                        fold_type = "array";
                    } else {
                        GET_BASIC_SIMPLETYPE (s_type, type);
                        fold_type = type_string[s_type];
                    }

                    /*
                     * <fold_type>, <accu_var>
                     */
                    fold_args
                      = MakeExprs (MakeId1 (fold_type),
                                   MakeExprs (MakeId1 (IDS_NAME (with_ids)), fold_args));

                    DBUG_PRINT ("COMPi", ("last's folds %s is %s", IDS_NAME (with_ids),
                                          fold_type));
                }
            }
            assign = ASSIGN_NEXT (assign);
        } /* while (assign != NULL) */
    }

    DBUG_PRINT ("COMPi", ("--- end ---"));

    /*
     * build arguments of ICMs
     */

    block = SYNC_REGION (arg_node);
    assign = BLOCK_INSTR (block);

    /*
     *  assign browses over the assignments of the sync_region
     */

    num_barrier_args = 0;
    barrier_args = NULL;
    while (assign != NULL) {
        let = ASSIGN_INSTR (assign);
        /*
         *  One does not know if this really is a with-loop!
         */
        with = LET_EXPR (let);
        with_ids = LET_IDS (let);

        /*
         *  Is this assignment a fold-with-loop?
         *  If so, one needs some arguments for the barrier from it.
         */
        if ((NODE_TYPE (with) == N_Nwith2)
            && ((NWITH2_TYPE (with) == WO_foldprf)
                || (NWITH2_TYPE (with) == WO_foldfun))) {
            /*
             *  Increase the number of fold-with-loops.
             */
            num_barrier_args++;

            /*
             * create fold_type-tag
             */
            type = IDS_TYPE (with_ids);
            if (TYPES_DIM (type) > 0) {
                if (MUST_REFCOUNT (IDS, with_ids)) {
                    fold_type = "array_rc";
                } else {
                    fold_type = "array";
                }
            } else {
                GET_BASIC_SIMPLETYPE (s_type, type);
                fold_type = type_string[s_type];
            }

            /*
             * <fold_type>, <accu_var>
             */
            icm_args = MakeExprs (MakeId1 (fold_type),
                                  MakeExprs (MakeId1 (IDS_NAME (with_ids)), NULL));

            barrier_args = AppendExprs (barrier_args, icm_args);

            DBUG_PRINT ("COMP", ("%s", IDS_NAME (with_ids)));

            /*
             * <tmp_var>, <fold_op>
             */
            DBUG_ASSERT ((NWITH2_FUNDEF (with) != NULL), "no fundef found");
            barrier_args
              = AppendExprs (barrier_args,
                             MakeExprs (MakeId1 (
                                          ID_NAME (NCODE_CEXPR (NWITH2_CODE (with)))),
                                        MakeExprs (MakeId1 (
                                                     FUNDEF_NAME (NWITH2_FUNDEF (with))),
                                                   NULL)));
        }

        assign = ASSIGN_NEXT (assign);
    }

    /*
     * finish arguments of ICM 'MT_CONTINUE'
     */
    fold_args = MakeExprs (MakeNum (num_fold_args), fold_args);

    /*
     *  compile the sync-region
     *  backup is needed, so one could use the block while compiling the next
     *  N_sync. One needs to copy that here, because further compiling
     *  is destructive.
     */
    backup = DupTree (BLOCK_INSTR (SYNC_REGION (arg_node)));
    SYNC_REGION (arg_node) = Trav (SYNC_REGION (arg_node), arg_info);

    /*
     *  now we extract all ICMs for memory-management concerning the
     *  IN/INOUT/OUT-vars of the current sync-region.
     *  (they must be moved into the sync-barrier!)
     */

    prolog_icms = NULL;
    epilog_icms = NULL;
    assign = BLOCK_INSTR (SYNC_REGION (arg_node));
    last_assign = NULL;
    /*
     * prolog == TRUE: current assignment is in front of WL_..._BEGIN-ICM (part of prolog)
     * epilog == TRUE: current assignment is behind WL_..._END-ICM (part of epilog)
     */
    prolog = TRUE;
    epilog = FALSE;
    count_nesting = 0; /* # of inner WLs */
    while (assign != NULL) {

        DBUG_ASSERT ((NODE_TYPE (assign) == N_assign), "no assign found");
        instr = ASSIGN_INSTR (assign);
        DBUG_ASSERT ((instr != NULL), "no instr found");

        var_name = NULL;
        if (NODE_TYPE (instr) == N_icm) {

            /* var_name = NULL; */
            if ((strcmp (ICM_NAME (instr), "WL_FOLD_BEGIN") == 0)
                || (strcmp (ICM_NAME (instr), "WL_NONFOLD_BEGIN") == 0)) {
                /*
                 *  begin of with-loop code found
                 *  -> skip with-loop code
                 *  -> stack nested with-loops
                 */
                count_nesting++;
                DBUG_PRINT ("COMP", ("ICM: %s is ++", ICM_NAME (instr)));
            } else if ((strcmp (ICM_NAME (instr), "WL_FOLD_END") == 0)
                       || (strcmp (ICM_NAME (instr), "WL_NONFOLD_END") == 0)) {
                /*
                 *  end of with-loop code found?
                 *  -> end of one (possibly nested) with loop
                 */
                DBUG_ASSERT ((count_nesting > 0),
                             "WL_..._BEGIN/END-ICMs non-balanced (too much ENDs)");
                count_nesting--;
                DBUG_PRINT ("COMP", ("ICM: %s is --", ICM_NAME (instr)));
            } else if (count_nesting == 0) {
                /*
                 *  not within any (possibly nested) with-loop
                 *  is this an 'prolog-icm' or 'epilog-icm' for memory management?
                 *
                 *  prolog == TRUE: current icm (assignment) is part of prolog
                 *  epilog == TRUE: current icm (assignment) is part of epilog
                 */

                if ((strcmp (ICM_NAME (instr), "ND_ALLOC_ARRAY") == 0)
                    || (strcmp (ICM_NAME (instr), "ND_CHECK_REUSE_ARRAY") == 0)
                    || (strcmp (ICM_NAME (instr), "ND_CHECK_REUSE_HIDDEN") == 0)) {
                    var_name = ID_NAME (EXPRS_EXPR (EXPRS_NEXT (ICM_ARGS (instr))));
                    prolog = TRUE;
                    epilog = FALSE;
                    DBUG_PRINT ("COMP", ("ICM: %s is prolog", ICM_NAME (instr)));
                } else if (strcmp (ICM_NAME (instr), "ND_INC_RC") == 0) {
                    var_name = ID_NAME (EXPRS_EXPR (ICM_ARGS (instr)));
                    prolog = TRUE;
                    epilog = FALSE;
                    DBUG_PRINT ("COMP", ("ICM: %s is prolog", ICM_NAME (instr)));
                } else if ((strcmp (ICM_NAME (instr), "ND_DEC_RC_FREE_ARRAY") == 0)
                           || (strcmp (ICM_NAME (instr), "ND_DEC_RC") == 0)) {
                    var_name = ID_NAME (EXPRS_EXPR (ICM_ARGS (instr)));
                    prolog = FALSE;
                    epilog = TRUE;
                    DBUG_PRINT ("COMP", ("ICM: %s is epilog", ICM_NAME (instr)));
                } else {
                    prolog = FALSE;
                    epilog = FALSE;
                    DBUG_PRINT ("COMP", ("ICM: %s is nothing", ICM_NAME (instr)));
                }
            } else {
                var_name = NULL;
                DBUG_PRINT ("COMP", ("ICM: %s is ignored!!!!", ICM_NAME (instr)));
            }

            if (var_name != NULL) {
                /*
                 *  the assignment (icm) is moved before/behind the the sync-block
                 *  if it is necessary to do it before the sync-block.
                 *  That copes icm's working on in(out)-variables with memory to be
                 *  reserved before of gen- or modarray with-loops, but *not* fold-results
                 *  (out-variables). Also outrep-varibale-icm's are not touched.
                 */
                if (DFMTestMaskEntry (SYNC_IN (arg_node), var_name, NULL)
                    || DFMTestMaskEntry (SYNC_INOUT (arg_node), var_name, NULL)) {
                    new_icm = DupNode (assign);

                    if (last_assign == NULL) {
                        assign = BLOCK_INSTR (SYNC_REGION (arg_node)) = FreeNode (assign);
                    } else {
                        assign = ASSIGN_NEXT (last_assign) = FreeNode (assign);
                    }

                    DBUG_ASSERT ((prolog || epilog), ("icm has to be prolog or epilog"));
                    DBUG_ASSERT ((!(prolog && epilog)),
                                 ("icm cannot be in prolog and epilog at the same time"));
                    if (prolog) {
                        prolog_icms = AppendAssign (prolog_icms, new_icm);
                    }
                    if (epilog) {
                        epilog_icms = AppendAssign (epilog_icms, new_icm);
                    }
                } else {
                    /*
                     *  one wants to test (var_name == NULL) later to decide,
                     *  whether the current, assignment was removed or not!
                     */
                    var_name = NULL;
                }
            }
        }

        /*
         * if current assignment was left untouched, goto next one
         */
        if (var_name == NULL) {
            last_assign = assign;
            assign = ASSIGN_NEXT (assign);
        }
    } /* while (assign != NULL) */

    /*
     * if this sync-region is *not* the first one of the current SPMD-region
     *  -> insert extracted prolog-ICMs (malloc).
     *  -> insert ICM (MT_CONTINUE),
     */
    if (!SYNC_FIRST (arg_node)) {
        assigns = AppendAssign (assigns, prolog_icms);
        assigns = AppendAssignIcm (assigns, "MT_MASTER_SEND_FOLDRESULTS", fold_args);
        assigns = AppendAssignIcm (assigns, "MT_MASTER_SEND_SYNCARGS", sync_args);
        assigns = AppendAssignIcm (assigns, "MT_START_WORKERS", NULL);
        assigns = AppendAssignIcm (assigns, "MT_MASTER_END", MakeExprsNum (barrier_id));
        assigns = AppendAssignIcm (assigns, "MT_WORKER_BEGIN", MakeExprsNum (barrier_id));
        assigns = AppendAssignIcm (assigns, "MT_WORKER_WAIT", NULL);
        assigns = AppendAssignIcm (assigns, "MT_MASTER_RECEIVE_FOLDRESULTS",
                                   DupTree (fold_args));
        assigns
          = AppendAssignIcm (assigns, "MT_MASTER_RECEIVE_SYNCARGS", DupTree (sync_args));
        assigns = AppendAssignIcm (assigns, "MT_WORKER_END", MakeExprsNum (barrier_id));
        assigns = AppendAssignIcm (assigns, "MT_RESTART", MakeExprsNum (barrier_id));

    } else {
        /*
         * this sync-region is the first one of the current SPMD-region.
         *  -> remove already built arguments for ICMs SEND and RECEIVE
         *  -> store prolog ICMs for subsequent compilation of corresponding
         *     spmd-block.
         */
        if (sync_args != NULL) {
            sync_args = FreeTree (sync_args);
        }
        if (fold_args != NULL) {
            fold_args = FreeTree (fold_args);
        }

        num_args = 0;
        setup_args = NULL;
        setup_args
          = BuildParamsByDFM (SYNC_INOUT (arg_node), "in", &num_args, setup_args);

        DBUG_PRINT ("COMPi", ("num_args %i", num_args));

        BLOCK_SPMD_PROLOG_ICMS (FUNDEF_BODY (INFO_COMP_FUNDEF (arg_info))) = prolog_icms;
        BLOCK_SPMD_SETUP_ARGS (FUNDEF_BODY (INFO_COMP_FUNDEF (arg_info))) = setup_args;
    }
    barrier_id++;

    /*
     * insert ICM 'MT_START_SYNCBLOCK',
     * ICM 'MT_SCHEDULER_BEGIN', and contents of modified sync-region-block
     */
    assigns = AppendAssign (
      assigns,
      MakeAssign (MakeIcm2 ("MT_START_SYNCBLOCK", MakeNum (barrier_id), icm_args3),
                  /*                        MakeAssign(
                     SCHCompileSchedulingBegin(SYNC_SCHEDULING(arg_node), arg_node), */
                  BLOCK_INSTR (SYNC_REGION (arg_node)) /*  )*/));

    /*
     *  see comment on setting backup!
     *  the modified code is now attached to another tree-part.
     */
    BLOCK_INSTR (SYNC_REGION (arg_node)) = backup;

    /*
     * insert ICM 'MT_SCHEDULER_END'
     */

    /*  assigns = AppendAssign(assigns,
                             MakeAssign(
       SCHCompileSchedulingEnd(SYNC_SCHEDULING(arg_node), arg_node), NULL));  */
    /*
     * insert ICM 'MT_SYNC_...'
     *
     * MT_SYNC_FOLD needs number of folds, MT_SYNC_NONFOLD and MT_SYNC_ONEFOLD
     * do not need this number, so it is only added at the MT_SYNC_FOLD arguments.
     */

    if (DFMTestMask (SYNC_INOUT (arg_node)) > 0) {
        if (DFMTestMask (SYNC_OUT (arg_node)) > 0) {
            if (DFMTestMask (SYNC_OUT (arg_node)) > 1) {
                /* possible, but not implemented: icm_name = "MT_SYNC_FOLD_NONFOLD"; */
                barrier_args = MakeExprs (MakeNum (num_barrier_args), barrier_args);
                barrier_args = MakeExprs (MakeNum (barrier_id), barrier_args);
                icm_name = "MT_SYNC_FOLD";

                DBUG_PRINT ("COMPi", ("MT_SYNC_FOLD (instead of MT_SYNC_FOLD_NONFOLD)"));
            } else {
                /* DFMTestMask( SYNC_OUT( arg_node)) == 1  */
                /* possible, but not implemented: icm_name = "MT_SYNC_ONEFOLD_NONFOLD"; */
                barrier_args = MakeExprs (MakeNum (num_barrier_args), barrier_args);
                barrier_args = MakeExprs (MakeNum (barrier_id), barrier_args);
                icm_name = "MT_SYNC_FOLD";

                DBUG_PRINT ("COMPi",
                            ("MT_SYNC_FOLD (instead of MT_SYNC_ONEFOLD_NONFOLD)"));
            }
        } else {
            barrier_args = MakeExprs (MakeNum (barrier_id), barrier_args);
            icm_name = "MT_SYNC_NONFOLD";

            DBUG_PRINT ("COMPi", (" MT_SYNC_NONFOLD"));
        }
    } else {
        DBUG_ASSERT ((DFMTestMask (SYNC_OUT (arg_node)) > 0), "no target found");

        DBUG_PRINT ("COMPi",
                    ("DFMTestMask( OUT ): %i", DFMTestMask (SYNC_OUT (arg_node))));

        if (DFMTestMask (SYNC_OUT (arg_node)) > 1) {
            barrier_args = MakeExprs (MakeNum (num_barrier_args), barrier_args);
            barrier_args = MakeExprs (MakeNum (barrier_id), barrier_args);
            icm_name = "MT_SYNC_FOLD";

            DBUG_PRINT ("COMPi", (" MT_SYNC_FOLD"));
        } else {
            /* DFMTestMask( SYNC_OUT( arg_node)) == 1 */
            barrier_args = MakeExprs (MakeNum (num_barrier_args), barrier_args);
            barrier_args = MakeExprs (MakeNum (barrier_id), barrier_args);
            icm_name = "MT_SYNC_FOLD";

            DBUG_PRINT ("COMPi", (" MT_SYNC_ONEFOLD"));
        }
    }

    DBUG_PRINT ("COMPi", ("using syncronisation: %s barrier: %i", icm_name, barrier_id));

    assigns = AppendAssignIcm (assigns, icm_name, barrier_args);

    /*
     * insert extracted epilog-ICMs (free).
     */

    /*
     *  Begins Block structure of value exchanges blocks or function return
     *  This has to be done, before epilogs ar inserted, which belong to the
     *  master-part of this blocks.
     */
    if (!SYNC_LAST (arg_node)) {
        assigns = AppendAssignIcm (assigns, "MT_MASTER_BEGIN", MakeExprsNum (barrier_id));
    } else {
        assigns
          = AppendAssignIcm (assigns, "MT_MASTER_BEGIN", /* ####jhs use other name? */
                             MakeExprsNum (barrier_id));
    }

    assigns = AppendAssign (assigns, epilog_icms);

    /*
     *  Exchanging LAST_SYNC, an free the last one.
     *  Will be needed for next N_sync, if no next N_sync exists COMPFundef
     *  will take care of this tree.
     */
    if (INFO_COMP_LAST_SYNC (arg_info) != NULL) {
        INFO_COMP_LAST_SYNC (arg_info) = FreeTree (INFO_COMP_LAST_SYNC (arg_info));
    }
    INFO_COMP_LAST_SYNC (arg_info) = arg_node;

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *COMPNwith2( node *arg_node, node *arg_info)
 *
 * description:
 *   Compilation of a N_with2 node.
 *   If this is a fold-with-loop, we append the vardecs of the pseudo fold-fun
 *    to the vardec-chain of the current function.
 *
 * remarks:
 *   - 'wl_ids' points always to LET_IDS of the current with-loop.
 *   - 'wl_node' points always to the N_Nwith2-node.
 *
 ******************************************************************************/

node *
COMPNwith2 (node *arg_node, node *arg_info)
{
    node *rc_icms_wl_ids = NULL, *assigns = NULL;
    node *fundef, *vardec, *icm_args, *neutral, *info, *dummy_assign, *tmp, *new,
      *old_wl_node, *fold_vardecs;
    ids *old_wl_ids;
    char *icm_name1, *icm_name2, *profile_name;

    DBUG_ENTER ("COMPNwith2");

    /*
     * we must store the with-loop ids *before* compiling the codes
     *  because INFO_COMP_LASTIDS is possibly updated afterwards !!!
     */
    old_wl_ids = wl_ids; /* stack 'wl_ids' */
    wl_ids = INFO_COMP_LASTIDS (arg_info);
    old_wl_node = wl_node; /* stack 'wl_node' */
    wl_node = arg_node;

    /*
     * When update-in-place is active:
     *   Insert ICMs for memory management (ND_CHECK_REUSE_ARRAY, ND_ALLOC_ARRAY),
     *    if we have a genarray- or modarray-op.
     *   (If we have a fold, this is done automaticly when compiling the
     *    init-assignment 'wl_ids = neutral' !!!)
     *
     * If we have a fold-op, insert the vardecs of the pseudo fold-fun
     *  into the vardec-chain of the current function.
     * Afterwards an update of the DFM-base is done.
     * (This must be done here, because 'COMPSync()' needs the corrected masks)
     */
    if ((NWITH2_TYPE (arg_node) == WO_genarray)
        || (NWITH2_TYPE (arg_node) == WO_modarray)) {

        if (optimize & OPT_UIP) {
            /*
             * find all arrays, that possibly can be reused.
             */
            arg_node = GetReuseArrays (arg_node, INFO_COMP_FUNDEF (arg_info), wl_ids);

            /*
             * 'ND_CHECK_REUSE_ARRAY'
             */
            vardec = DFMGetMaskEntryDeclSet (NWITH2_REUSE (arg_node));
            while (vardec != NULL) {
                assigns = MakeAssign (MakeIcm2 ("ND_CHECK_REUSE_ARRAY",
                                                MakeId1 (VARDEC_OR_ARG_NAME (vardec)),
                                                MakeId1 (IDS_NAME (wl_ids))),
                                      assigns);

                vardec = DFMGetMaskEntryDeclSet (NULL);
            }
            NWITH2_REUSE (arg_node) = DFMRemoveMask (NWITH2_REUSE (arg_node));
        }

        /*
         * 'ND_ALLOC_ARRAY'
         */
        assigns = AppendAssign (assigns, MakeAllocArrayICMs_reuse (wl_ids, NULL));
    } else {
        /* fold-with-loop */

        fundef = INFO_COMP_FUNDEF (arg_info);

        fold_vardecs = GetFoldVardecs (NWITH2_FUNDEF (arg_node));
        if (fold_vardecs != NULL) {

            /*
             * insert vardecs of pseudo fold-fun
             */
            FUNDEF_VARDEC (fundef) = AppendVardecs (FUNDEF_VARDEC (fundef), fold_vardecs);

            /*
             * update DFM-base
             */
            FUNDEF_DFM_BASE (fundef)
              = DFMUpdateMaskBase (FUNDEF_DFM_BASE (fundef), FUNDEF_ARGS (fundef),
                                   FUNDEF_VARDEC (fundef));
        }
    }

    /*
     * insert ICMs to allocate memory for index-vector
     */
    if (IDS_REFCNT (NWITHID_VEC (NWITH2_WITHID (arg_node))) > 0) {
        assigns
          = AppendAssign (assigns,
                          MakeAllocArrayICMs (NWITHID_VEC (NWITH2_WITHID (arg_node)),
                                              NULL));
    }

    /*
     * compile all code blocks
     */
    NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);

    /*
     * build arguments for  'WL_..._BEGIN'-ICM and 'WL_..._END'-ICM
     */
    icm_args
      = MakeExprs (MakeId2 (DupOneIds (wl_ids, NULL)),
                   MakeExprs (MakeId2 (
                                DupOneIds (NWITHID_VEC (NWITH2_WITHID (wl_node)), NULL)),
                              MakeExprs (MakeNum (NWITH2_DIMS (arg_node)), NULL)));

    switch (NWITHOP_TYPE (NWITH2_WITHOP (arg_node))) {
    case WO_genarray:
        icm_name1 = "WL_NONFOLD_BEGIN";
        icm_name2 = "WL_NONFOLD_END";
        profile_name = "genarray";
        break;
    case WO_modarray:
        icm_name1 = "WL_NONFOLD_BEGIN";
        icm_name2 = "WL_NONFOLD_END";
        profile_name = "modarray";
        break;

    case WO_foldfun:
        /* here is no break missing! */
    case WO_foldprf:
        /****************************************
         * compile 'wl_ids = neutral' !!!
         */
        neutral = NWITHOP_NEUTRAL (NWITH2_WITHOP (arg_node));
        info = MakeInfo ();
        /*
         * some older routines (e.g. COMPArray) uses 'INFO_COMP_LASTASSIGN'
         *  to bypass the Trav-mechanismus and insert some ICMs *directly* into the
         *  syntax-tree.
         * To fool this mechanismus, we must create two dummy assignments in
         *  'INFO_COMP_LASTASSIGN' (a last and a current assignment).
         * When returning from 'Trav' we will find the supplemented ICMs behind these
         *  nodes.
         */
        dummy_assign = MakeAssign (NULL, MakeAssign (NULL, NULL));
        INFO_COMP_LASTASSIGN (info) = dummy_assign;
        neutral = Trav (MakeLet (DupTree (neutral), DupOneIds (wl_ids, NULL)), info);
        FREE (info);
        dummy_assign = FreeNode (FreeNode (dummy_assign));
        /*
         * concate compiled assignments
         */
        if (neutral != NULL) {
            neutral = MakeAssign (neutral, NULL);
        }
        neutral = AppendAssign (neutral, dummy_assign);

        /*
         * All RC-ICMs on 'wl_ids' must be moved *behind* the WL-code!!
         * Therefore we collect them in 'rc_icms_wl_ids' to insert them later.
         */
        tmp = neutral;
        while (ASSIGN_NEXT (tmp) != NULL) {
            if ((NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (tmp))) == N_icm)
                && ((strcmp (ICM_NAME (ASSIGN_INSTR (ASSIGN_NEXT (tmp))), "ND_DEC_RC")
                     == 0)
                    || (strcmp (ICM_NAME (ASSIGN_INSTR (ASSIGN_NEXT (tmp))), "ND_INC_RC")
                        == 0)
                    || (strcmp (ICM_NAME (ASSIGN_INSTR (ASSIGN_NEXT (tmp))),
                                "ND_DEC_RC_FREE_ARRAY")
                        == 0))) {
                new = ASSIGN_NEXT (tmp);
                ASSIGN_NEXT (tmp) = ASSIGN_NEXT (ASSIGN_NEXT (tmp));
                ASSIGN_NEXT (new) = NULL;
                rc_icms_wl_ids = AppendAssign (rc_icms_wl_ids, new);
            } else {
                tmp = ASSIGN_NEXT (tmp);
            }
        }

        /*
         * insert compiled assignments
         */
        assigns = AppendAssign (assigns, neutral);

        icm_name1 = "WL_FOLD_BEGIN";
        icm_name2 = "WL_FOLD_END";
        profile_name = "fold";
        break;

    default:
        DBUG_ASSERT ((0), "wrong withop type found");
    }

    /*
     * create 'PF_BEGIN_WITH' (for profiling) and 'WL_..._BEGIN'-ICM
     */
    assigns
      = AppendAssign (assigns,
                      MakeAssign (MakeIcm1 ("PF_BEGIN_WITH", MakeId1 (profile_name)),
                                  MakeAssignIcm1 (icm_name1, icm_args)));

    /*
     * compile the with-segments
     *  -> we get an assignment-chain
     */
    assigns = AppendAssign (assigns, Trav (NWITH2_SEGS (arg_node), arg_info));

    /*
     * create PF_END_WITH (for profiling) and  'WL_END'-ICM
     */
    assigns
      = AppendAssign (assigns, MakeAssign (MakeIcm1 (icm_name2, icm_args),
                                           MakeAssignIcm1 ("PF_END_WITH",
                                                           MakeId1 (profile_name))));

    /*
     * insert RC-ICMs from 'wl_ids = neutral'
     */
    assigns = AppendAssign (assigns, rc_icms_wl_ids);

    /*
     * insert ICMs for memory management ('DEC_RC_FREE').
     */
    assigns = AppendAssign (assigns, MakeDecRcICMs (NWITH2_DEC_RC_IDS (arg_node), NULL));

    /*
     * insert 'DEC_RC_FREE'-ICM for index-vector.
     */
    if (IDS_REFCNT (NWITHID_VEC (NWITH2_WITHID (arg_node))) > 0) {
        assigns = AppendAssign (assigns, MakeDecRcICMs (NWITH2_VEC (arg_node), NULL));
    }

    /*
     * with-loop representation is useless now!
     */
    arg_node = FreeTree (arg_node);

    /*
     * pop 'wl_ids', 'wl_node'.
     */
    wl_ids = old_wl_ids;
    wl_node = old_wl_node;

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *COMPNcode( node *arg_node, node *arg_info)
 *
 * description:
 *   compiles a Ncode node.
 *
 ******************************************************************************/

node *
COMPNcode (node *arg_node, node *arg_info)
{
    node *icm_assigns;

    DBUG_ENTER ("COMPNcode");

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    /*
     * build a 'ND_INC_RC'-ICM for each ids in 'NCODE_INC_RC_IDS( arg_node)'.
     */
    icm_assigns = MakeIncRcICMs (NCODE_INC_RC_IDS (arg_node), NULL);

    if (icm_assigns != NULL) {
        /*
         * insert these ICMs as first statement into the code-block
         */

        DBUG_ASSERT ((NCODE_CBLOCK (arg_node) != NULL),
                     "no code block found in N_Ncode node");
        DBUG_ASSERT ((NCODE_CBLOCK_INSTR (arg_node) != NULL),
                     "first instruction of block is NULL (should be a N_empty node)");

        if (NODE_TYPE (NCODE_CBLOCK_INSTR (arg_node)) == N_empty) {
            /*
             * remove a N_empty node
             */
            NCODE_CBLOCK_INSTR (arg_node) = FreeTree (NCODE_CBLOCK_INSTR (arg_node));
        }
        NCODE_CBLOCK_INSTR (arg_node)
          = AppendAssign (icm_assigns, NCODE_CBLOCK_INSTR (arg_node));
    }

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPWLseg(node *arg_node, node *arg_info)
 *
 * description:
 *   compilation of an N_WLseg-node:
 *     returns an N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *
 * remark:
 *   - 'wl_seg' points to the current with-loop segment.
 *
 ******************************************************************************/

node *
COMPWLseg (node *arg_node, node *arg_info)
{
    node *assigns, *old_wl_seg;

    DBUG_ENTER ("COMPWLseg");

    /*
     * stack old 'wl_seg'.
     * store pointer to current segment in 'wl_seg'.
     */
    old_wl_seg = wl_seg;
    wl_seg = arg_node;

    /*
     * compile the contents of the segment
     *  => we get an assignment-chain
     */
    assigns = Trav (WLSEG_CONTENTS (arg_node), arg_info);

    /*
     * insert ICM "WL_INIT_OFFSET"
     */
    if ((NWITHOP_TYPE (NWITH2_WITHOP (wl_node)) == WO_genarray)
        || (NWITHOP_TYPE (NWITH2_WITHOP (wl_node)) == WO_modarray)) {
        assigns
          = MakeAssign (MakeIcm4 ("WL_INIT_OFFSET",
                                  MakeNum (
                                    TYPES_DIM (VARDEC_OR_ARG_TYPE (IDS_VARDEC (wl_ids)))),
                                  MakeId1 (IDS_NAME (wl_ids)),
                                  MakeId1 (
                                    IDS_NAME (NWITHID_VEC (NWITH2_WITHID (wl_node)))),
                                  MakeNum (NWITH2_DIMS (wl_node))),
                        assigns);
    }

    /*
     * Insert scheduling specific ICMs.
     */
    assigns
      = AppendAssign (assigns,
                      MakeAssign (SCHCompileSchedulingEnd (WLSEG_SCHEDULING (arg_node),
                                                           arg_node),
                                  NULL));
    assigns
      = MakeAssign (SCHCompileSchedulingBegin (WLSEG_SCHEDULING (arg_node), arg_node),
                    assigns);

    /*
     * append compilat (assignment-chain) of next segment to 'assigns'
     */
    if (WLSEG_NEXT (arg_node) != NULL) {
        assigns = AppendAssign (assigns, Trav (WLSEG_NEXT (arg_node), arg_info));
    }

    /*
     * pop 'wl_seg'.
     */
    wl_seg = old_wl_seg;

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *COMPWLblock(node *arg_node, node *arg_info)
 *
 * description:
 *   compilation of an N_WLblock-node:
 *     returns an N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *
 * remarks:
 *   - 'wl_ids' points always to LET_IDS of the current with-loop.
 *   - 'wl_node' points always to the N_Nwith2-node.
 *   - 'wl_seg' points to the current with-loop segment.
 *
 ******************************************************************************/

node *
COMPWLblock (node *arg_node, node *arg_info)
{
    node *icm_args;
    ids *ids_vector, *ids_scalar;
    char *icm_name;
    node *assigns = NULL;

    DBUG_ENTER ("COMPWLblock");

    /* compile contents */
    if (WLBLOCK_CONTENTS (arg_node) != NULL) {
        DBUG_ASSERT ((WLBLOCK_NEXTDIM (arg_node) == NULL),
                     "contents and nextdim used simultaneous");
        assigns = Trav (WLBLOCK_CONTENTS (arg_node), arg_info);
    }

    /* compile nextdim */
    if (WLBLOCK_NEXTDIM (arg_node) != NULL) {
        DBUG_ASSERT ((WLBLOCK_CONTENTS (arg_node) == NULL),
                     "contents and nextdim used simultaneous");
        assigns = Trav (WLBLOCK_NEXTDIM (arg_node), arg_info);
    }

    /* build argument list of ICMs */
    ids_vector = NWITHID_VEC (NWITH2_WITHID (wl_node));
    ids_scalar
      = GetIndexIds (NWITHID_IDS (NWITH2_WITHID (wl_node)), WLBLOCK_DIM (arg_node));
    icm_args
      = MakeExprs (MakeNum (WLBLOCK_DIM (arg_node)),
                   MakeExprs (MakeId2 (DupOneIds (ids_vector, NULL)),
                              MakeExprs (MakeId2 (DupOneIds (ids_scalar, NULL)),
                                         MakeExprs (MakeNum (WLBLOCK_BOUND1 (arg_node)),
                                                    MakeExprs (MakeNum (WLBLOCK_BOUND2 (
                                                                 arg_node)),
                                                               MakeExprs (MakeNum (
                                                                            WLBLOCK_STEP (
                                                                              arg_node)),
                                                                          NULL))))));

    /*
     * insert ICMs for current node
     */

    DBUG_ASSERT ((assigns != NULL), "contents and nextdim are empty");

    if (WLBLOCK_LEVEL (arg_node) == 0) {
        if (NWITH2_MT (wl_node)) {
            icm_name = "WL_MT_BLOCK_LOOP0_BEGIN";
        } else {
            icm_name = "WL_BLOCK_LOOP0_BEGIN";
        }
    } else {
        if (NWITH2_MT (wl_node)) {
            icm_name = "WL_MT_BLOCK_LOOP_BEGIN";
        } else {
            icm_name = "WL_BLOCK_LOOP_BEGIN";
        }
    }

    assigns = MakeAssign (MakeIcm1 (icm_name, icm_args), assigns);

    if ((WLBLOCK_LEVEL (arg_node) == 0) && (NWITH2_MT (wl_node))
        && (SCHAdjustmentRequired (WLBLOCK_DIM (arg_node), wl_seg))) {
        assigns
          = MakeAssign (MakeIcm6 ("MT_ADJUST_SCHEDULER", MakeNum (WLBLOCK_DIM (arg_node)),
                                  MakeNum (WLSEG_DIMS (wl_seg)),
                                  MakeNum (WLBLOCK_BOUND1 (arg_node)),
                                  MakeNum (WLBLOCK_BOUND2 (arg_node)),
                                  MakeNum (
                                    MAX (WLSEG_SV (wl_seg)[WLBLOCK_DIM (arg_node)],
                                         WLSEG_UBV (wl_seg)[WLBLOCK_DIM (arg_node)])),
                                  MakeId1 (IDS_NAME (wl_ids))),
                        assigns);
    }

    if (NWITH2_MT (wl_node)) {
        icm_name = "WL_MT_BLOCK_LOOP_END";
    } else {
        icm_name = "WL_BLOCK_LOOP_END";
    }

    assigns = AppendAssign (assigns,
                            MakeAssign (MakeIcm1 (icm_name, DupTree (icm_args)), NULL));

    /* compile successor */
    if (WLBLOCK_NEXT (arg_node) != NULL) {
        assigns = AppendAssign (assigns, Trav (WLBLOCK_NEXT (arg_node), arg_info));
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *COMPWLublock(node *arg_node, node *arg_info)
 *
 * description:
 *   compilation of an N_WLublock-node:
 *     returns an N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *
 * remarks:
 *   - 'wl_ids' points always to LET_IDS of the current with-loop.
 *   - 'wl_node' points always to the N_Nwith2-node.
 *   - 'wl_seg' points to the current with-loop segment.
 *
 ******************************************************************************/

node *
COMPWLublock (node *arg_node, node *arg_info)
{
    node *icm_args;
    ids *ids_vector, *ids_scalar;
    char *icm_name;
    node *assigns = NULL;

    DBUG_ENTER ("COMPWLublock");

    /* compile contents */
    if (WLUBLOCK_CONTENTS (arg_node) != NULL) {
        DBUG_ASSERT ((WLUBLOCK_NEXTDIM (arg_node) == NULL),
                     "contents and nextdim used simultaneous");
        assigns = Trav (WLUBLOCK_CONTENTS (arg_node), arg_info);
    }

    /* compile nextdim */
    if (WLUBLOCK_NEXTDIM (arg_node) != NULL) {
        DBUG_ASSERT ((WLUBLOCK_CONTENTS (arg_node) == NULL),
                     "contents and nextdim used simultaneous");
        assigns = Trav (WLUBLOCK_NEXTDIM (arg_node), arg_info);
    }

    /* build argument list of ICMs */
    ids_vector = NWITHID_VEC (NWITH2_WITHID (wl_node));
    ids_scalar
      = GetIndexIds (NWITHID_IDS (NWITH2_WITHID (wl_node)), WLUBLOCK_DIM (arg_node));
    icm_args
      = MakeExprs (MakeNum (WLUBLOCK_DIM (arg_node)),
                   MakeExprs (MakeId2 (DupOneIds (ids_vector, NULL)),
                              MakeExprs (MakeId2 (DupOneIds (ids_scalar, NULL)),
                                         MakeExprs (MakeNum (WLUBLOCK_BOUND1 (arg_node)),
                                                    MakeExprs (MakeNum (WLUBLOCK_BOUND2 (
                                                                 arg_node)),
                                                               MakeExprs (MakeNum (
                                                                            WLUBLOCK_STEP (
                                                                              arg_node)),
                                                                          NULL))))));

    /*
     * insert ICMs for current node
     */
    DBUG_ASSERT ((assigns != NULL), "contents and nextdim are empty");

    if (WLUBLOCK_LEVEL (arg_node) == 0) {
        if (NWITH2_MT (wl_node)) {
            icm_name = "WL_MT_UBLOCK_LOOP0_BEGIN";
        } else {
            icm_name = "WL_UBLOCK_LOOP0_BEGIN";
        }
    } else {
        if (NWITH2_MT (wl_node)) {
            icm_name = "WL_MT_UBLOCK_LOOP_BEGIN";
        } else {
            icm_name = "WL_UBLOCK_LOOP_BEGIN";
        }
    }

    assigns = MakeAssign (MakeIcm1 (icm_name, icm_args), assigns);

    if ((WLUBLOCK_LEVEL (arg_node) == 0) && (NWITH2_MT (wl_node))
        && (SCHAdjustmentRequired (WLUBLOCK_DIM (arg_node), wl_seg))) {
        assigns
          = MakeAssign (MakeIcm6 ("MT_ADJUST_SCHEDULER",
                                  MakeNum (WLUBLOCK_DIM (arg_node)),
                                  MakeNum (WLSEG_DIMS (wl_seg)),
                                  MakeNum (WLUBLOCK_BOUND1 (arg_node)),
                                  MakeNum (WLUBLOCK_BOUND2 (arg_node)),
                                  MakeNum (
                                    MAX (WLSEG_SV (wl_seg)[WLUBLOCK_DIM (arg_node)],
                                         WLSEG_UBV (wl_seg)[WLUBLOCK_DIM (arg_node)])),
                                  MakeId1 (IDS_NAME (wl_ids))),
                        assigns);
    }

    if (NWITH2_MT (wl_node)) {
        icm_name = "WL_MT_UBLOCK_LOOP_END";
    } else {
        icm_name = "WL_UBLOCK_LOOP_END";
    }

    assigns = AppendAssign (assigns,
                            MakeAssign (MakeIcm1 (icm_name, DupTree (icm_args)), NULL));

    /* compile successor */
    if (WLUBLOCK_NEXT (arg_node) != NULL) {
        assigns = AppendAssign (assigns, Trav (WLUBLOCK_NEXT (arg_node), arg_info));
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *COMPWLstride(node *arg_node, node *arg_info)
 *
 * description:
 *   compilation of an N_WLstride-node:
 *     returns an N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *
 * remarks:
 *   - 'wl_ids' points always to LET_IDS of the current with-loop.
 *   - 'wl_node' points always to the N_Nwith2-node.
 *   - 'wl_seg' points to the current with-loop segment.
 *
 ******************************************************************************/

node *
COMPWLstride (node *arg_node, node *arg_info)
{
    node *icm_args;
    ids *ids_vector, *ids_scalar;
    char *icm_name_begin, *icm_name_end;
    node *assigns, *new_assigns;
    int cnt_unroll, i;

    DBUG_ENTER ("COMPWLstride");

    /*
     * compile contents
     */
    assigns = Trav (WLSTRIDE_CONTENTS (arg_node), arg_info);

    /*
     * build argument list of ICMs
     */
    ids_vector = NWITHID_VEC (NWITH2_WITHID (wl_node));
    ids_scalar
      = GetIndexIds (NWITHID_IDS (NWITH2_WITHID (wl_node)), WLSTRIDE_DIM (arg_node));
    icm_args
      = MakeExprs (MakeNum (WLSTRIDE_DIM (arg_node)),
                   MakeExprs (MakeId2 (DupOneIds (ids_vector, NULL)),
                              MakeExprs (MakeId2 (DupOneIds (ids_scalar, NULL)),
                                         MakeExprs (MakeNum (WLSTRIDE_BOUND1 (arg_node)),
                                                    MakeExprs (MakeNum (WLSTRIDE_BOUND2 (
                                                                 arg_node)),
                                                               MakeExprs (MakeNum (
                                                                            WLSTRIDE_STEP (
                                                                              arg_node)),
                                                                          NULL))))));

    /*
     * insert ICMs for current node
     */

    if (WLSTRIDE_UNROLLING (arg_node) == 1) {
        /*
         * unrolling
         */

        if (NWITH2_MT (wl_node)) {
            icm_name_begin = "WL_MT_STRIDE_UNROLL_BEGIN";
            icm_name_end = "WL_MT_STRIDE_UNROLL_END";
        } else {
            icm_name_begin = "WL_STRIDE_UNROLL_BEGIN";
            icm_name_end = "WL_STRIDE_UNROLL_END";
        }

        new_assigns = NULL;
        DBUG_ASSERT ((((WLSTRIDE_BOUND2 (arg_node) - WLSTRIDE_BOUND1 (arg_node))
                       % WLSTRIDE_STEP (arg_node))
                      == 0),
                     "'step' is not a divisor of 'bound2 - bound1'");
        cnt_unroll = (WLSTRIDE_BOUND2 (arg_node) - WLSTRIDE_BOUND1 (arg_node))
                     / WLSTRIDE_STEP (arg_node);
        for (i = 1; i < cnt_unroll; i++) {
            new_assigns = AppendAssign (new_assigns, DupTree (assigns));
        }
        assigns = AppendAssign (new_assigns, assigns);

    } else {
        /*
         * no unrolling
         */

        if (NWITH2_MT (wl_node)) {
            if (WLSTRIDE_LEVEL (arg_node) == 0) {
                icm_name_begin = "WL_MT_STRIDE_LOOP0_BEGIN";
            } else {
                icm_name_begin = "WL_MT_STRIDE_LOOP_BEGIN";
            }
            icm_name_end = "WL_MT_STRIDE_LOOP_END";
        } else {
            if (WLSTRIDE_LEVEL (arg_node) == 0) {
                icm_name_begin = "WL_STRIDE_LOOP0_BEGIN";
            } else {
                icm_name_begin = "WL_STRIDE_LOOP_BEGIN";
            }
            icm_name_end = "WL_STRIDE_LOOP_END";
        }
    }

    assigns = MakeAssign (MakeIcm1 (icm_name_begin, icm_args), assigns);

    if ((WLSTRIDE_LEVEL (arg_node) == 0) && (NWITH2_MT (wl_node))
        && (SCHAdjustmentRequired (WLSTRIDE_DIM (arg_node), wl_seg))) {
        assigns
          = MakeAssign (MakeIcm6 ("MT_ADJUST_SCHEDULER",
                                  MakeNum (WLSTRIDE_DIM (arg_node)),
                                  MakeNum (WLSEG_DIMS (wl_seg)),
                                  MakeNum (WLSTRIDE_BOUND1 (arg_node)),
                                  MakeNum (WLSTRIDE_BOUND2 (arg_node)),
                                  MakeNum (
                                    MAX (WLSEG_SV (wl_seg)[WLSTRIDE_DIM (arg_node)],
                                         WLSEG_UBV (wl_seg)[WLSTRIDE_DIM (arg_node)])),
                                  MakeId1 (IDS_NAME (wl_ids))),
                        assigns);
    }

    assigns
      = AppendAssign (assigns,
                      MakeAssign (MakeIcm1 (icm_name_end, DupTree (icm_args)), NULL));

    /* compile successor */
    if (WLSTRIDE_NEXT (arg_node) != NULL) {
        assigns = AppendAssign (assigns, Trav (WLSTRIDE_NEXT (arg_node), arg_info));
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   int GetDim_WL_ADJUST_OFFSET( node *grid, node *wl, node *seg)
 *
 * Description:
 *   'wl' is a pointer to the current N_Nwith2-node,
 *   'seg' is a pointer to the current N_Nseg-node.
 *   'multi_segs' indicates whether there are multiple segments or not.
 *
 *   This function infers in which dimension the WL_ADJUST_OFFSET-icm is needed.
 *   A return value -1 means that this ICM is not needed at all.
 *
 *   'multi_segs' == 0:
 *     insert ICM, if blocking is activ and the next dim is the last one
 *     (otherwise the ICM redundant) and we do not have a fold with-loop
 *     (in this case we do not use any offset!).
 *
 *   'multi_segs' == 1:
 *     insert ICM, if next dim is the last dim and we do not have a fold
 *     with-loop.
 *
 ******************************************************************************/

int
GetDim_WL_ADJUST_OFFSET (node *grid, node *wl, node *seg)
{
    int first_block_dim, d;
    int icm_dim = -1;

    DBUG_ENTER ("InsertIcm_WL_ADJUST_OFFSET");

    if ((NWITH2_TYPE (wl) != WO_foldprf) && (NWITH2_TYPE (wl) != WO_foldfun)) {

        /*
         * infer first blocking dimension
         */
        d = 0;
        while ((d < WLSEG_DIMS (seg)) && ((WLSEG_BV (seg, 0))[d] == 1)) {
            d++;
        }
        if (d == WLSEG_DIMS (seg)) {
            /*
             * no blocking => inspect 'ubv'
             */
            d = 0;
            while ((d < WLSEG_DIMS (seg)) && ((WLSEG_UBV (seg))[d] == 1)) {
                d++;
            }
        }
        first_block_dim = d;

        /*
         * check whether 'WL_ADJUST_OFFSET' is needed or not
         */
        if (MULTIPLE_SEGS == 0) {

            /*
             * is the next dim the last one and blocking activ?
             */
            if ((WLGRID_DIM (grid) + 2 == WLSEG_DIMS (seg))
                && (first_block_dim < WLSEG_DIMS (seg))) {
                icm_dim = first_block_dim;
            }
        } else {
            /* MULTIPLE_SEGS == 1 */

            /*
             * is the next dim the last one?
             */
            if (WLGRID_DIM (grid) + 2 == WLSEG_DIMS (seg)) {
                icm_dim = first_block_dim;
            }
        }
    }

    DBUG_RETURN (icm_dim);
}

/******************************************************************************
 *
 * function:
 *   node *COMPWLgrid( node *arg_node, node *arg_info)
 *
 * description:
 *   compilation of an N_WLgrid-node:
 *     returns an N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *
 * remarks:
 *   - 'wl_ids' points always to LET_IDS of the current with-loop.
 *   - 'wl_node' points always to the N_with-node.
 *   - 'wl_seg' points to the current with-loop segment.
 *
 ******************************************************************************/

node *
COMPWLgrid (node *arg_node, node *arg_info)
{
    node *assigns = NULL, *dec_rc_cexpr = NULL;
    node *icm_args, *icm_args2, *cexpr, *new_assigns;
    ids *ids_vector, *ids_scalar, *withid_ids;
    char *icm_name, *icm_name_begin, *icm_name_end;
    int num_args, cnt_unroll, adjust_dim, i;

    DBUG_ENTER ("COMPWLgrid");

    /*
     * build argument list for 'WL_ASSIGN_...'/'WL_FOLD'/'WL_ADJUST_OFFSET'-ICM
     */
    icm_args2 = NULL;
    num_args = 0;
    withid_ids = NWITHID_IDS (NWITH2_WITHID (wl_node));
    while (withid_ids != NULL) {
        icm_args2
          = AppendExprs (icm_args2,
                         MakeExprs (MakeId2 (DupOneIds (withid_ids, NULL)), NULL));
        num_args++;
        withid_ids = IDS_NEXT (withid_ids);
    }
    icm_args2
      = MakeExprs (MakeNum (TYPES_DIM (IDS_TYPE (wl_ids))),
                   MakeExprs (MakeId2 (DupOneIds (wl_ids, NULL)),
                              MakeExprs (MakeId2 (DupOneIds (NWITHID_VEC (
                                                               NWITH2_WITHID (wl_node)),
                                                             NULL)),
                                         MakeExprs (MakeNum (num_args), icm_args2))));

    if (WLGRID_NEXTDIM (arg_node) != NULL) {

        adjust_dim = GetDim_WL_ADJUST_OFFSET (arg_node, wl_node, wl_seg);
        if (adjust_dim >= 0) {
            assigns = MakeAssignIcm3 ("WL_ADJUST_OFFSET", MakeNum (WLGRID_DIM (arg_node)),
                                      MakeNum (adjust_dim), icm_args2);
        } else {
            icm_args2 = FreeTree (icm_args2);
        }

        /*
         * compile nextdim
         */
        DBUG_ASSERT ((WLGRID_CODE (arg_node) == NULL),
                     "code and nextdim used simultaneous");
        assigns = AppendAssign (assigns, Trav (WLGRID_NEXTDIM (arg_node), arg_info));

    } else {

        if (WLGRID_CODE (arg_node) != NULL) {

            /*
             * insert compiled code.
             */
            cexpr = NCODE_CEXPR (WLGRID_CODE (arg_node));
            DBUG_ASSERT ((cexpr != NULL), "no code expr found");

            DBUG_ASSERT ((NCODE_CBLOCK (WLGRID_CODE (arg_node)) != NULL),
                         "no code block found in N_Ncode node");
            DBUG_ASSERT ((NCODE_CBLOCK_INSTR (WLGRID_CODE (arg_node)) != NULL),
                         "first instruction of block is NULL (should be a N_empty node)");

            if (NODE_TYPE (NCODE_CBLOCK_INSTR (WLGRID_CODE (arg_node))) != N_empty) {
                assigns = DupTree (NCODE_CBLOCK_INSTR (WLGRID_CODE (arg_node)));
            }

            /*
             * choose right ICM
             */
            switch (NWITH2_TYPE (wl_node)) {
            case WO_genarray:
                /* here is no break missing! */
            case WO_modarray:
                DBUG_ASSERT ((NODE_TYPE (cexpr) == N_id), "code expr is not a id");
                icm_args2 = MakeExprs (MakeNum (TYPES_DIM (ID_TYPE (cexpr))),
                                       MakeExprs (DupNode (cexpr), icm_args2));
                icm_name = "WL_ASSIGN";
                /*
                 * we must decrement the RC of 'cexpr' (consumed argument)
                 */
                if (ID_REFCNT (cexpr) >= 0) {
                    dec_rc_cexpr = MakeAssignIcm2 ("ND_DEC_RC_FREE_ARRAY",
                                                   DupNode (cexpr), MakeNum (1));
                }
                break;

            case WO_foldfun:
                /* here is no break missing! */
            case WO_foldprf:
                /*
                 * here we do not need any ICM-args
                 * (we do not even need an ICM...)
                 */
                icm_args2 = FreeTree (icm_args2);

                /*
                 * insert code of the pseudo fold-fun
                 */
                assigns = AppendAssign (assigns,
                                        GetUnadjustedFoldCode (NWITH2_FUNDEF (wl_node)));

                icm_name = NULL;
                break;

            default:
                DBUG_ASSERT ((0), "wrong withop type found");
            }

        } else {
            /*
             * no code found.
             *  => init/copy/noop
             */

            assigns = NULL;

            /*
             * choose right ICM
             */
            switch (NWITH2_TYPE (wl_node)) {
            case WO_genarray:
                icm_name = "WL_ASSIGN_INIT";
                break;

            case WO_modarray:
                icm_args2 = MakeExprs (DupNode (NWITHOP_ARRAY (NWITH2_WITHOP (wl_node))),
                                       icm_args2);
                icm_name = "WL_ASSIGN_COPY";
                break;

            case WO_foldfun:
                /* here is no break missing! */
            case WO_foldprf:
                icm_name = "WL_FOLD_NOOP";
                break;

            default:
                DBUG_ASSERT ((0), "wrong withop type found");
            }
        }

        if (icm_name != NULL) {
            assigns = AppendAssign (assigns, MakeAssign (MakeIcm1 (icm_name, icm_args2),
                                                         dec_rc_cexpr));
        }
    }

    /* build argument list for 'WL_GRID_...'-ICMs */
    ids_vector = NWITHID_VEC (NWITH2_WITHID (wl_node));
    ids_scalar
      = GetIndexIds (NWITHID_IDS (NWITH2_WITHID (wl_node)), WLGRID_DIM (arg_node));
    icm_args
      = MakeExprs (MakeNum (WLGRID_DIM (arg_node)),
                   MakeExprs (MakeId2 (DupOneIds (ids_vector, NULL)),
                              MakeExprs (MakeId2 (DupOneIds (ids_scalar, NULL)),
                                         MakeExprs (MakeNum (WLGRID_BOUND1 (arg_node)),
                                                    MakeExprs (MakeNum (WLGRID_BOUND2 (
                                                                 arg_node)),
                                                               NULL)))));

    /*
     * insert ICMs for current node
     */

    /*
     * if the index-vector is needed somewhere in the code-blocks,
     *  and this is not a dummy grid (init, copy, noop),
     *  we must add the ICM 'WL_GRID_SET_IDX'.
     */
    if ((IDS_REFCNT (NWITHID_VEC (NWITH2_WITHID (wl_node))) > 0)
        && ((WLGRID_CODE (arg_node) != NULL) || (WLGRID_NEXTDIM (arg_node)))) {
        assigns = MakeAssign (MakeIcm1 ("WL_GRID_SET_IDX", icm_args), assigns);
    }

    if ((WLGRID_UNROLLING (arg_node) == 1)
        || (WLGRID_BOUND2 (arg_node) - WLGRID_BOUND1 (arg_node) == 1)) {
        /*
         * unrolling (or (width == 1))
         */

        if (NWITH2_MT (wl_node)) {
            icm_name_begin = "WL_MT_GRID_UNROLL_BEGIN";
            icm_name_end = "WL_MT_GRID_UNROLL_END";
        } else {
            icm_name_begin = "WL_GRID_UNROLL_BEGIN";
            icm_name_end = "WL_GRID_UNROLL_END";
        }

        new_assigns = NULL;
        cnt_unroll = WLGRID_BOUND2 (arg_node) - WLGRID_BOUND1 (arg_node);
        for (i = 1; i < cnt_unroll; i++) {
            new_assigns = AppendAssign (new_assigns, DupTree (assigns));
        }
        assigns = AppendAssign (new_assigns, assigns);

    } else {
        /*
         * no unrolling
         */

        if (NWITH2_MT (wl_node)) {
            icm_name_begin = "WL_MT_GRID_LOOP_BEGIN";
            icm_name_end = "WL_MT_GRID_LOOP_END";
        } else {
            icm_name_begin = "WL_GRID_LOOP_BEGIN";
            icm_name_end = "WL_GRID_LOOP_END";
        }
    }

    assigns = MakeAssign (MakeIcm1 (icm_name_begin, icm_args), assigns);

    assigns = AppendAssign (assigns, MakeAssignIcm1 (icm_name_end, DupTree (icm_args)));

    /* compile successor */
    if (WLGRID_NEXT (arg_node) != NULL) {
        assigns = AppendAssign (assigns, Trav (WLGRID_NEXT (arg_node), arg_info));
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *COMPWLsegVar(node *arg_node, node *arg_info)
 *
 * description:
 *   compilation of an N_WLsegVar-node:
 *     returns an N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *
 * remark:
 *   - 'wl_seg' points to the current with-loop segment.
 *
 ******************************************************************************/

node *
COMPWLsegVar (node *arg_node, node *arg_info)
{
    node *assigns, *old_wl_seg;

    DBUG_ENTER ("COMPWLsegVar");

    /*
     * stack old 'wl_seg'.
     * store pointer to current segment in 'wl_seg'.
     */
    old_wl_seg = wl_seg;
    wl_seg = arg_node;

    /*
     * compile the contents of the segment
     *  => we get an assignment-chain
     */
    assigns = Trav (WLSEGVAR_CONTENTS (arg_node), arg_info);

    /*
     * insert ICM "WL_INIT_OFFSET"
     */
    if ((NWITHOP_TYPE (NWITH2_WITHOP (wl_node)) == WO_genarray)
        || (NWITHOP_TYPE (NWITH2_WITHOP (wl_node)) == WO_modarray)) {
        assigns
          = MakeAssign (MakeIcm4 ("WL_INIT_OFFSET",
                                  MakeNum (
                                    TYPES_DIM (VARDEC_OR_ARG_TYPE (IDS_VARDEC (wl_ids)))),
                                  MakeId1 (IDS_NAME (wl_ids)),
                                  MakeId1 (
                                    IDS_NAME (NWITHID_VEC (NWITH2_WITHID (wl_node)))),
                                  MakeNum (NWITH2_DIMS (wl_node))),
                        assigns);
    }

    /*
     * Insert scheduling specific ICMs.
     */
    assigns
      = AppendAssign (assigns,
                      MakeAssign (SCHCompileSchedulingEnd (WLSEGVAR_SCHEDULING (arg_node),
                                                           arg_node),
                                  NULL));
    assigns
      = MakeAssign (SCHCompileSchedulingBegin (WLSEGVAR_SCHEDULING (arg_node), arg_node),
                    assigns);

    /*
     * append compilat (assignment-chain) of next segment to 'assigns'
     */
    if (WLSEGVAR_NEXT (arg_node) != NULL) {
        assigns = AppendAssign (assigns, Trav (WLSEGVAR_NEXT (arg_node), arg_info));
    }

    /*
     * pop 'wl_seg'.
     */
    wl_seg = old_wl_seg;

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *COMPWLstriVar(node *arg_node, node *arg_info)
 *
 * description:
 *   compilation of an N_WLstriVar-node:
 *     returns an N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *
 ******************************************************************************/

node *
COMPWLstriVar (node *arg_node, node *arg_info)
{
    node *icm_args;
    ids *ids_vector, *ids_scalar;
    char *icm_name_begin, *icm_name_end;
    node *assigns;

    DBUG_ENTER ("COMPWLstriVar");

    /* compile contents */
    assigns = Trav (WLSTRIVAR_CONTENTS (arg_node), arg_info);

    /* build argument list of ICMs */
    ids_vector = NWITHID_VEC (NWITH2_WITHID (wl_node));
    ids_scalar
      = GetIndexIds (NWITHID_IDS (NWITH2_WITHID (wl_node)), WLSTRIVAR_DIM (arg_node));
    icm_args = MakeExprs (
      MakeNum (WLSTRIVAR_DIM (arg_node)),
      MakeExprs (
        MakeId2 (DupOneIds (ids_vector, NULL)),
        MakeExprs (MakeId2 (DupOneIds (ids_scalar, NULL)),
                   MakeExprs (IdOrNumToIndex (WLSTRIVAR_BOUND1 (arg_node),
                                              WLSTRIVAR_DIM (arg_node)),
                              MakeExprs (IdOrNumToIndex (WLSTRIVAR_BOUND2 (arg_node),
                                                         WLSTRIVAR_DIM (arg_node)),
                                         MakeExprs (IdOrNumToIndex (WLSTRIVAR_STEP (
                                                                      arg_node),
                                                                    WLSTRIVAR_DIM (
                                                                      arg_node)),
                                                    NULL))))));

    /*
     * insert ICMs for current node
     */

    if (NWITH2_MT (wl_node)) {
        icm_name_begin = "WL_MT_STRIDE_LOOP0_BEGIN";
        icm_name_end = "WL_MT_STRIDE_LOOP_END";
    } else {
        icm_name_begin = "WL_STRIDE_LOOP0_BEGIN";
        icm_name_end = "WL_STRIDE_LOOP_END";
    }

    assigns = MakeAssign (MakeIcm1 (icm_name_begin, icm_args), assigns);

    assigns
      = AppendAssign (assigns,
                      MakeAssign (MakeIcm1 (icm_name_end, DupTree (icm_args)), NULL));

    /* compile successor */
    if (WLSTRIVAR_NEXT (arg_node) != NULL) {
        assigns = AppendAssign (assigns, Trav (WLSTRIVAR_NEXT (arg_node), arg_info));
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *COMPWLgridVar(node *arg_node, node *arg_info)
 *
 * description:
 *   compilation of an N_WLgridVar-node:
 *     returns an N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *
 ******************************************************************************/

node *
COMPWLgridVar (node *arg_node, node *arg_info)
{
    node *assigns = NULL, *dec_rc_cexpr = NULL;
    node *icm_args, *icm_args2, *cexpr;
    ids *ids_vector, *ids_scalar, *withid_ids;
    char *icm_name, *icm_name_begin, *icm_name_end;
    int num_args;

    DBUG_ENTER ("COMPWLgridVar");

    if (WLGRIDVAR_NEXTDIM (arg_node) != NULL) {

        /*
         * compile nextdim
         */

        DBUG_ASSERT ((WLGRIDVAR_CODE (arg_node) == NULL),
                     "code and nextdim used simultaneous");
        assigns = Trav (WLGRIDVAR_NEXTDIM (arg_node), arg_info);

    } else {

        /*
         * build argument list for 'WL_ASSIGN_...'/'WL_FOLD'-ICM
         */
        icm_args2 = NULL;
        num_args = 0;
        withid_ids = NWITHID_IDS (NWITH2_WITHID (wl_node));
        while (withid_ids != NULL) {
            icm_args2
              = AppendExprs (icm_args2,
                             MakeExprs (MakeId2 (DupOneIds (withid_ids, NULL)), NULL));
            num_args++;
            withid_ids = IDS_NEXT (withid_ids);
        }
        icm_args2
          = MakeExprs (MakeNum (TYPES_DIM (IDS_TYPE (wl_ids))),
                       MakeExprs (MakeId2 (DupOneIds (wl_ids, NULL)),
                                  MakeExprs (MakeId2 (
                                               DupOneIds (NWITHID_VEC (
                                                            NWITH2_WITHID (wl_node)),
                                                          NULL)),
                                             MakeExprs (MakeNum (num_args), icm_args2))));

        if (WLGRIDVAR_CODE (arg_node) != NULL) {

            /*
             * insert compiled code.
             */

            cexpr = NCODE_CEXPR (WLGRIDVAR_CODE (arg_node));
            DBUG_ASSERT ((cexpr != NULL), "no code expr found");

            DBUG_ASSERT ((NCODE_CBLOCK (WLGRIDVAR_CODE (arg_node)) != NULL),
                         "no code block found in N_Ncode node");
            DBUG_ASSERT ((NCODE_CBLOCK_INSTR (WLGRIDVAR_CODE (arg_node)) != NULL),
                         "first instruction of block is NULL (should be a N_empty node)");

            if (NODE_TYPE (NCODE_CBLOCK_INSTR (WLGRIDVAR_CODE (arg_node))) != N_empty) {
                assigns = DupTree (NCODE_CBLOCK_INSTR (WLGRIDVAR_CODE (arg_node)));
            }

            /*
             * choose right ICM.
             */
            switch (NWITH2_TYPE (wl_node)) {
            case WO_genarray:
                /* here is no break missing! */
            case WO_modarray:
                DBUG_ASSERT ((NODE_TYPE (cexpr) == N_id), "code expr is not a id");
                icm_args2 = MakeExprs (MakeNum (TYPES_DIM (ID_TYPE (cexpr))),
                                       MakeExprs (DupNode (cexpr), icm_args2));
                icm_name = "WL_ASSIGN";
                /*
                 * we must decrement the RC of 'cexpr' (consumed argument)
                 */
                if (ID_REFCNT (cexpr) >= 0) {
                    dec_rc_cexpr = MakeAssignIcm2 ("ND_DEC_RC_FREE_ARRAY",
                                                   DupNode (cexpr), MakeNum (1));
                }
                break;

            case WO_foldfun:
                /* here is no break missing! */
            case WO_foldprf:
                /*
                 * here we do not need any ICM-args
                 * (we do not even need an ICM...)
                 */
                icm_args2 = FreeTree (icm_args2);

                /*
                 * insert code of the pseudo fold-fun
                 */
                assigns = AppendAssign (assigns,
                                        GetUnadjustedFoldCode (NWITH2_FUNDEF (wl_node)));

                icm_name = NULL;
                break;

            default:
                DBUG_ASSERT ((0), "wrong withop type found");
            }

        } else {
            /*
             * no code found.
             *  => init/copy/noop
             */

            assigns = NULL;

            /*
             * choose right ICM.
             */
            switch (NWITH2_TYPE (wl_node)) {
            case WO_genarray:
                icm_name = "WL_ASSIGN_INIT";
                break;

            case WO_modarray:
                icm_args2 = MakeExprs (DupNode (NWITHOP_ARRAY (NWITH2_WITHOP (wl_node))),
                                       icm_args2);
                icm_name = "WL_ASSIGN_COPY";
                break;

            case WO_foldfun:
                /* here is no break missing! */
            case WO_foldprf:
                icm_name = "WL_FOLD_NOOP";
                break;

            default:
                DBUG_ASSERT ((0), "wrong withop type found");
            }
        }

        if (icm_name != NULL) {
            assigns = AppendAssign (assigns, MakeAssign (MakeIcm1 (icm_name, icm_args2),
                                                         dec_rc_cexpr));
        }
    }

    /* build argument list for 'WL_GRID_...'-ICMs */
    ids_vector = NWITHID_VEC (NWITH2_WITHID (wl_node));
    ids_scalar
      = GetIndexIds (NWITHID_IDS (NWITH2_WITHID (wl_node)), WLGRIDVAR_DIM (arg_node));
    icm_args = MakeExprs (
      MakeNum (WLGRIDVAR_DIM (arg_node)),
      MakeExprs (MakeId2 (DupOneIds (ids_vector, NULL)),
                 MakeExprs (MakeId2 (DupOneIds (ids_scalar, NULL)),
                            MakeExprs (IdOrNumToIndex (WLGRIDVAR_BOUND1 (arg_node),
                                                       WLGRIDVAR_DIM (arg_node)),
                                       MakeExprs (IdOrNumToIndex (WLGRIDVAR_BOUND2 (
                                                                    arg_node),
                                                                  WLGRIDVAR_DIM (
                                                                    arg_node)),
                                                  NULL)))));

    /*
     * insert ICMs for current node
     */

    /*
     * if the index-vector is needed somewhere in the code-blocks,
     *  and this is not a dummy grid (init, copy, noop),
     *  we must add the ICM 'WL_GRID_SET_IDX'.
     */
    if ((IDS_REFCNT (NWITHID_VEC (NWITH2_WITHID (wl_node))) > 0)
        && ((WLGRIDVAR_CODE (arg_node) != NULL) || (WLGRIDVAR_NEXTDIM (arg_node)))) {
        assigns = MakeAssign (MakeIcm1 ("WL_GRID_SET_IDX", icm_args), assigns);
    }

    if (NWITH2_MT (wl_node)) {
        icm_name_begin = "WL_MT_GRIDVAR_LOOP_BEGIN";
        icm_name_end = "WL_MT_GRIDVAR_LOOP_END";
    } else {
        icm_name_begin = "WL_GRIDVAR_LOOP_BEGIN";
        icm_name_end = "WL_GRIDVAR_LOOP_END";
    }

    assigns = MakeAssign (MakeIcm1 (icm_name_begin, icm_args), assigns);

    assigns = AppendAssign (assigns, MakeAssignIcm1 (icm_name_end, DupTree (icm_args)));

    /* compile successor */
    if (WLGRIDVAR_NEXT (arg_node) != NULL) {
        assigns = AppendAssign (assigns, Trav (WLGRIDVAR_NEXT (arg_node), arg_info));
    }

    DBUG_RETURN (assigns);
}

/* move to tree_compound  !!! #### */

node *
MakeAssigns1 (node *part1)
{
    return (MakeAssign (part1, NULL));
}

node *
MakeAssigns2 (node *part1, node *part2)
{
    return (MakeAssign (part1, MakeAssigns1 (part2)));
}

node *
MakeAssigns3 (node *part1, node *part2, node *part3)
{
    return (MakeAssign (part1, MakeAssigns2 (part2, part3)));
}

node *
MakeAssigns4 (node *part1, node *part2, node *part3, node *part4)
{
    return (MakeAssign (part1, MakeAssigns3 (part2, part3, part4)));
}

node *
MakeAssigns5 (node *part1, node *part2, node *part3, node *part4, node *part5)
{
    return (MakeAssign (part1, MakeAssigns4 (part2, part3, part4, part5)));
}

node *
MakeAssigns6 (node *part1, node *part2, node *part3, node *part4, node *part5,
              node *part6)
{
    return (MakeAssign (part1, MakeAssigns5 (part2, part3, part4, part5, part6)));
}

node *
MakeAssigns7 (node *part1, node *part2, node *part3, node *part4, node *part5,
              node *part6, node *part7)
{
    return (MakeAssign (part1, MakeAssigns6 (part2, part3, part4, part5, part6, part7)));
}

node *
MakeAssigns8 (node *part1, node *part2, node *part3, node *part4, node *part5,
              node *part6, node *part7, node *part8)
{
    return (
      MakeAssign (part1, MakeAssigns7 (part2, part3, part4, part5, part6, part7, part8)));
}

/*
 *  jhs ####
 */
#define DO_NOT_COMPILE_MTN_xxx

/******************************************************************************
 *
 * function:
 *   node *COMPMt( node *arg_node, node *arg_info)
 *
 * description:
 *   compiles an N_mt-node
 *
 ******************************************************************************/
node *
COMPMt (node *arg_node, node *arg_info)
{
    node *result;
    node *allocate;
    node *broadcast;
    node *activate;
    node *code;
    node *barrier;

    DBUG_ENTER ("COMPMt");

#ifdef DO_NOT_COMPILE_MTN
    DBUG_ASSERT (0, ("COMPMt not implemented yet, cannot compile this"));
#endif

    /*
     *  Part 1 - Allocate
     */
    allocate = MakeIcm0 ("ALLOCATE"); /* #### */

    /*
     *  Part 2 - Broadcast
     */
    broadcast
      = MakeIcm5 ("MT2_MASTER_BROADCAST", MakeId1 ("ALLOC"),
                  MakeNum (MT_IDENTIFIER (arg_node)),
                  MakeNum (DFMTestMask (MT_ALLOC (arg_node))
                           + DFMTestMask (MT_USEMASK (arg_node))),
                  BuildParamsByDFM (MT_ALLOC (arg_node), "alloc", NULL, NULL),
                  BuildParamsByDFM (MT_USEMASK (arg_node), "usemask", NULL, NULL));

    /*
     *  Part 3 - Activate
     */
    activate = MakeIcm2 ("MT2_ACTIVATE", MakeId1 ("SYNC"),
                         MakeId1 (FUNDEF_NAME (MT_FUNDEF (arg_node))));

    /*
     *  Part 4 - Code
     *  Compile region and move compilation to result.
     */
    MT_REGION (arg_node) = Trav (MT_REGION (arg_node), arg_info);
    code = BLOCK_INSTR (MT_REGION (arg_node));
    BLOCK_INSTR (MT_REGION (arg_node)) = NULL;

    /*
     *  Part 5 - Barrier
     */
    barrier = MakeIcm1 ("MT2_MASTER_BARRIER", MakeNum (MT_IDENTIFIER (arg_node)));

    /*
     *  Finalization ... build result and free old tree
     */
    result = MakeAssigns5 (allocate, broadcast, activate, code, barrier);

    arg_node = FreeTree (arg_node);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *COMPMt( node *arg_node, node *arg_info)
 *
 * description:
 *   compiles an N_mt-node
 *
 ******************************************************************************/
node *
COMPSt (node *arg_node, node *arg_info)
{
    node *result;
    node *fundef;
    node *barrier, *code, *allocate, *broadcast, *activate, *suspend, *receive;

    DBUG_ENTER ("COMPSt");

#ifdef DO_NOT_COMPILE_MTN
    DBUG_ASSERT (0, ("COMPSt not implemented yet, cannot compile this"));
#endif

    fundef = INFO_COMP_FUNDEF (arg_info);
    DBUG_PRINT ("COMPjhs", ("compiling %s attrib: %s status: %s", FUNDEF_NAME (fundef),
                            mdb_statustype[FUNDEF_ATTRIB (fundef)],
                            mdb_statustype[FUNDEF_STATUS (fundef)]));

    if (FUNDEF_ATTRIB (fundef) == ST_call_mt_master) {
        barrier = MakeIcm0 ("MTN_MASTER_BARRIER");
        code = MakeIcm0 ("CODE");
        allocate = MakeIcm0 ("ALLOCATE");
        broadcast = MakeIcm0 ("MTN_MASTER_BROADCAST");
        activate = MakeIcm0 ("MTN_ACTIVATE");

        result = MakeAssigns5 (barrier, code, allocate, broadcast, activate);
    } else if (FUNDEF_ATTRIB (fundef) == ST_call_mt_worker) {
        barrier = MakeIcm0 ("MTN_WORKER_BARRIER");
        suspend = MakeIcm0 ("MTN_SUSPEND");
        receive = MakeIcm0 ("MTN_MASTER_RECEIVE");

        result = MakeAssigns3 (barrier, suspend, receive);
    } else {
        DBUG_ASSERT (0, "can not handle such a function");
    }

    arg_node = FreeTree (arg_node);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *COMPMTsignal( node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
COMPMTsignal (node *arg_node, node *arg_info)
{
    node *assigns;

    DBUG_ENTER ("COMPMTsignal");

#ifdef DO_NOT_COMPILE_MTN
    DBUG_ASSERT (0, ("COMPMTsignal not implemented yet, cannot compile this"));
#endif

    assigns
      = MakeAssigns1 (MakeIcm1 ("MT2_SIGNAL", BuildParamsByDFM (MTSIGNAL_IDSET (arg_node),
                                                                "ids", NULL, NULL)));

    arg_node = FreeTree (arg_node);

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *COMPMTalloc( node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
COMPMTalloc (node *arg_node, node *arg_info)
{
    node *fundef;
    node *assigns;
    node *if_;
    node *alloc;
    node *broadcast;
    node *activate;
    node *else_;
    node *suspend;
    node *receive;
    node *end_;
    char *broadcast_icm;
    char *receive_icm;

    DBUG_ENTER ("COMPMTalloc");

#ifdef DO_NOT_COMPILE_MTN
    DBUG_ASSERT (0, ("COMPMTalloc not implemented yet, cannot compile this"));
#endif

    fundef = INFO_COMP_FUNDEF (arg_info);
    if ((FUNDEF_ATTRIB (fundef) == ST_call_mt_master)
        || (FUNDEF_ATTRIB (fundef) == ST_call_st)) {
        broadcast_icm = "MT2_MASTER_BROADCAST";
        receive_icm = "MT2_MASTER_RECEIVE";
    } else if ((FUNDEF_ATTRIB (fundef) == ST_call_mt_worker)
               || (FUNDEF_ATTRIB (fundef) == ST_call_mtlift)) {
        broadcast_icm = "MT2_WORKER_BROADCAST";
        receive_icm = "MT2_WORKER_RECEIVE";
    } else {
        DBUG_PRINT ("JHS", ("%s", mdb_statustype[FUNDEF_ATTRIB (fundef)]));
        DBUG_ASSERT (0, "can not handle such a function");
    }

    if_ = MakeIcm0 ("MT2_IF_I_AM_FIRST");
    alloc = MakeIcm0 ("ALLOC");

    /*
      DFMPrintMaskDetailed (stdout, MTALLOC_IDSET( arg_node));
    */
    printf ("\n");

    broadcast
      = MakeIcm2 (broadcast_icm, MakeId1 ("ALLOC"),
                  BuildParamsByDFM (MTALLOC_IDSET (arg_node), "alloc", NULL, NULL));
    activate = MakeIcm2 ("MT2_ACTIVATE", MakeId1 ("ALLOC"), MakeId1 ("NULL"));
    else_ = MakeIcm0 ("MT2_ELSE_IF_I_AM_NOT_FIRST");
    suspend = MakeIcm1 ("MT2_SUSPEND", MakeId1 ("ALLOC"));
    receive = MakeIcm2 (receive_icm, MakeId1 ("ALLOC"),
                        BuildParamsByDFM (MTALLOC_IDSET (arg_node), "alloc", NULL, NULL));
    end_ = MakeIcm0 ("MT2_END_I_AM_FIRST");

    assigns
      = MakeAssigns8 (if_, alloc, broadcast, activate, else_, suspend, receive, end_);

    arg_node = FreeTree (arg_node);

    DBUG_RETURN (assigns);
}

node *
COMPMTsync (node *arg_node, node *arg_info)
{
    node *result;

    DBUG_ENTER ("COMPMTsync");

#ifdef DO_NOT_COMPILE_MTN
    DBUG_ASSERT (0, ("COMPMTsync not implemented yet, cannot compile this"));
#endif

    result = MakeAssigns1 (MakeIcm0 ("CANNOT_COMPILE_N_SYNC"));

    arg_node = FreeTree (arg_node);

    DBUG_RETURN (result);
}
