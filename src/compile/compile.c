/*
 *
 * $Log$
 * Revision 2.95  2000/10/27 00:08:43  dkr
 * some code brushing done
 * COMPAp revisited (not finished yet, some known bugs still left)
 *
 * Revision 2.94  2000/10/24 14:48:32  dkr
 * some brushing done
 *
 * Revision 2.93  2000/10/24 11:09:47  dkr
 * some functions renamed
 * compilation of prfs streamlined, corrected and simplified
 * superfluous macro definitions removed
 *
 * Revision 2.92  2000/10/17 18:01:07  dkr
 * COMPAp and COMPPrf streamlined and corrected
 *
 * Revision 2.91  2000/10/09 19:20:31  dkr
 * GetUnadjustedFoldCode() renamed into GetFoldCode()
 *
 * Revision 2.90  2000/09/27 16:48:31  dkr
 * a = fun( a) is flattened now during flattening phase and not during
 * compilation phase :-)
 *
 * Revision 2.89  2000/09/25 15:19:03  dkr
 * Compilation of hidden types streamlined and simplified.
 * MakeTypeString() modified:
 *   Hidden types are no longer compiled into 'void*' but into the name of
 *   the typedef (we can rely on the corresponding typedef here!)
 * Function GetBasicType() created.
 *
 * Revision 2.88  2000/09/20 18:20:41  dkr
 * ID_MAKEUNIQUE renamed into ID_CLSCONV
 * Compilation of  v = from_class( w)  corrected (COMPId())
 *
 * Revision 2.87  2000/09/15 15:43:14  dkr
 * COMPId() revisited
 *
 * Revision 2.86  2000/09/12 16:47:09  dkr
 * COMPArg simplified (MakeAdjustRcICMs() used)
 *
 * Revision 2.85  2000/08/24 15:51:18  dkr
 * macro MUST_REFCOUNT renamed into IS_REFCOUNTED to prevent mix-up with macro
 * of same name in refcount.[ch]
 *
 * Revision 2.84  2000/08/24 10:12:38  dkr
 * fixed a bug in COMPAp():
 * icm ND_FREE_... replaced by ND_DEC_RC_FREE...
 *
 * Revision 2.83  2000/08/18 23:07:48  dkr
 * COMPNormalFunReturn() recoded in order to resolve name clashes of
 * out-parameters correctly.
 * (The renaming is done via the ND_FUN_RET() ICM now)
 *
 * Revision 2.82  2000/08/17 10:15:07  dkr
 * comment about IdOrNumToIndex corrected
 *
 * Revision 2.81  2000/07/31 10:45:52  cg
 * Eventually, the son ICM_NEXT is removed from the N_icm node.
 * The creation function MakeIcm is adjusted accordingly.
 *
 * Revision 2.80  2000/07/28 17:17:10  cg
 * Compilation of N_vardec nodes modified: ICM now is temporary
 * attribute of N_vardec node which is no longer removed during
 * code generation.
 *
 * Revision 2.79  2000/07/28 12:30:48  cg
 * N_typedef nodes are no longer replaced by N_icm node during
 * code generation. Instead an N_icm node is added to the N_typedef
 * node as additional temporary attribute when necessary.
 *
 * Revision 2.78  2000/07/28 11:41:21  cg
 * Added code to handle dummy parts of iteration spaces due to
 * array padding efficiently, i.e. to perform no action at all.
 *
 * Revision 2.77  2000/07/24 16:41:18  dkr
 * superfluous 'line' parameter of ICMs for array-prfs removed
 *
 * Revision 2.76  2000/07/21 11:26:03  jhs
 * Added CreateIcmMT2_FUN_DEC.
 * Building RECEIVE/BARRIER combination around functions lifted
 * by mt2; rearrange their args as vardec.
 * Using MT2_FUN_DEC, MT2_FUN_RET for mt2 connected functions.
 *
 * Revision 2.75  2000/07/17 14:34:56  jhs
 * all allocs of mt2 are now done by MakeAllocs.
 *
 * Revision 2.74  2000/07/14 14:43:11  nmw
 * free changed to FREE, when used with StringConcat()
 *
 * Revision 2.73  2000/07/13 13:33:46  jhs
 * Modified creation of allocs for new mt.
 *
 * Revision 2.72  2000/07/13 11:59:07  jhs
 * Splited ICM_INDENT into ICM_INDENT_BEFORE and ICM_INDENT_AFTER.
 *
 * Revision 2.71  2000/07/12 17:17:44  jhs
 * Fixed params.
 *
 * Revision 2.70  2000/07/12 15:15:15  dkr
 * function DuplicateTypes renamed into DupTypes
 *
 * Revision 2.69  2000/07/11 15:45:21  jhs
 * Added homework ...
 * containing BuildParamsByDFMfold, compilations of N_mt, N_st,
 * N_MTalloc, N_MTsignal, N_MTsync, ...
 *
 * Revision 2.68  2000/07/11 11:29:16  dkr
 * minor changes done
 *
 * Revision 2.67  2000/07/10 14:23:26  cg
 * The return type of a function that will become the actual return
 * type of the corresponding compiled C function is tagged as ST_crettype.
 *
 * Revision 2.66  2000/07/06 17:34:04  dkr
 * some obsolete TAGGED_ARRAY stuff removed
 *
 * Revision 2.65  2000/07/05 12:24:12  dkr
 * unused variable hidden now
 *
 * Revision 2.64  2000/07/04 14:55:57  jhs
 * Added mtn-stuff.
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
 * hidden objects are no longer handled by FREE_ARRAY, DEC_RC_FREE_ARRAY ICMs
 * 8-))
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
 * Extremly brushed COMPLoop
 * (now I unterstand at least half of it, before I had not
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
 *
 */

#include <stdlib.h>
#include <string.h>

#include "tree.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "prf.h"

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
#include "typecheck.h" /* for some ugly old macros ... */

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

#define LABEL_NAME "SAC__Label" /* basic-name for goto label */

#define ICMPARAM_TAG(expr) ID_NAME (EXPRS_EXPR (expr))
#define ICMPARAM_ARG(expr) EXPRS_EXPR (EXPRS_NEXT (expr))

/*
 * This macro indicates whether there are multiple segments present or not.
 * It uses the global variable 'wl_seg'.
 */
#define MULTIPLE_SEGS(wl_seg)                                                            \
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
    {                                                                                    \
        node *_num;                                                                      \
        if ((1 == ID_REFCNT (old)) && (stype_new == stype_old)) {                        \
            /* create ND_CHECK_REUSE_ARRAY  */                                           \
            BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_CHECK_REUSE_ARRAY", old,    \
                           new);                                                         \
            first_assign = CURR_ASSIGN (arg_info);                                       \
            DBUG_PRINT ("COMP", ("first:" P_FORMAT, first_assign));                      \
            old_arg_node = arg_node;                                                     \
            arg_node = LET_EXPR (INFO_COMP_LASTLET (arg_info));                          \
            /* create ND_ALLOC_ARRAY */                                                  \
            _num = MakeNum (0);                                                          \
            CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", res_type, new, _num);       \
            APPEND_ASSIGNS (first_assign, next_assign);                                  \
        } else {                                                                         \
            /* create ND_ALLOC_ARRAY */                                                  \
            DBUG_ASSERT (((ID_REFCNT (old) == -1) || (!(optimize & OPT_RCO))),           \
                         "Refcnt of prf-arg neither -1 nor 1 !");                        \
            _num = MakeNum (0);                                                          \
            BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY", res_type,     \
                           new);                                                         \
            MAKE_NEXT_ICM_ARG (icm_arg, _num);                                           \
            first_assign = CURR_ASSIGN (arg_info);                                       \
            DBUG_PRINT ("COMP", ("first:" P_FORMAT, first_assign));                      \
            old_arg_node = arg_node;                                                     \
            arg_node = LET_EXPR (INFO_COMP_LASTLET (arg_info));                          \
        }                                                                                \
    }

#define DEC_OR_FREE_RC_ND(array, num_node)                                               \
    if (1 < ID_REFCNT (array)) { /* create ND_DEC_RC */                                  \
        CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC", array, num_node);                    \
        APPEND_ASSIGNS (first_assign, next_assign);                                      \
    } else {                                                                             \
        if (ID_REFCNT (array) > 0) {                                                     \
            DEC_RC_FREE_ND (array, num_node);                                            \
        }                                                                                \
    }

#define DEC_RC_FREE_ND(array, num_node) /* create ND_DEC_RC_FREE_ARRAY */                \
    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE_ARRAY", array, num_node);             \
    APPEND_ASSIGNS (first_assign, next_assign)

#define CREATE_TMP_CONST_ARRAY(array, rc)                                                \
    {                                                                                    \
        int _n_elems;                                                                    \
        array_is_const = 1;                                                              \
        old_arg_node = arg_node;                                                         \
        _n_elems = GetExprsLength (ARRAY_AELEMS (array));                                \
        tmp_array1 = MakeId_Copy ("__TMP");                                              \
        /* reuse previous N_let*/                                                        \
        NODE_TYPE (INFO_COMP_LASTLET (arg_info)) = N_block;                              \
        CREATE_4_ARY_ICM (first_assign, "ND_KS_DECL_ARRAY", res_type, tmp_array1,        \
                          MakeNum (1), MakeNum (_n_elems));                              \
        arg_node = first_assign;                                                         \
        CREATE_CONST_ARRAY (array, tmp_array1, res_type, rc)                             \
        array = tmp_array1; /* set array to __TMP */                                     \
    }

#define CREATE_CONST_ARRAY(array, name, type, rc)                                        \
    {                                                                                    \
        int _n_elems;                                                                    \
        CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type, name, rc);                \
        APPEND_ASSIGNS (first_assign, next_assign);                                      \
        _n_elems = GetExprsLength (ARRAY_AELEMS (array));                                \
        CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_S", name,                  \
                          MakeNum (_n_elems));                                           \
        EXPRS_NEXT (icm_arg) = ARRAY_AELEMS (array);                                     \
        APPEND_ASSIGNS (first_assign, next_assign);                                      \
    }

#define DECL_ARRAY(assign, node, var_str, var_str_node)                                  \
    {                                                                                    \
        int _n_elems;                                                                    \
        _n_elems = GetExprsLength (node);                                                \
        var_str_node = MakeId_Copy (var_str);                                            \
        CREATE_4_ARY_ICM (assign, "ND_KS_DECL_ARRAY", res_type, var_str_node,            \
                          MakeNum (1), MakeNum (_n_elems));                              \
    }

#define CURR_ASSIGN(arg_info)                                                            \
    ((N_block == NODE_TYPE (INFO_COMP_LASTASSIGN (arg_info)))                            \
       ? BLOCK_INSTR (INFO_COMP_LASTASSIGN (arg_info))                                   \
       : ASSIGN_NEXT (INFO_COMP_LASTASSIGN (arg_info)))

#define NEXT_ASSIGN(arg_info) ASSIGN_NEXT (CURR_ASSIGN (arg_info))

#define MAKE_NEXT_ICM_ARG(prev, new_node)                                                \
    {                                                                                    \
        node *_tmp;                                                                      \
        _tmp = MakeExprs (new_node, NULL);                                               \
        EXPRS_NEXT (prev) = _tmp;                                                        \
        prev = _tmp;                                                                     \
    }

#define APPEND_ICM_ARG(prev, new)                                                        \
    EXPRS_NEXT (prev) = new;                                                             \
    prev = new;

#define CREATE_1_ARY_ICM(assign, str, arg)                                               \
    assign = MakeAssign (MakeIcm (str, NULL), NULL);                                     \
    ICM_ARGS (ASSIGN_INSTR (assign)) = MakeExprs (arg, NULL);                            \
    icm_arg = ICM_ARGS (ASSIGN_INSTR (assign));

#define CREATE_2_ARY_ICM(assign, str, arg1, arg2)                                        \
    CREATE_1_ARY_ICM (assign, str, arg1);                                                \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2)

#define CREATE_3_ARY_ICM(assign, str, arg1, arg2, arg3)                                  \
    CREATE_1_ARY_ICM (assign, str, arg1);                                                \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2);                                                   \
    MAKE_NEXT_ICM_ARG (icm_arg, arg3)

#define CREATE_4_ARY_ICM(assign, str, arg1, arg2, arg3, arg4)                            \
    CREATE_1_ARY_ICM (assign, str, arg1);                                                \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2);                                                   \
    MAKE_NEXT_ICM_ARG (icm_arg, arg3);                                                   \
    MAKE_NEXT_ICM_ARG (icm_arg, arg4)

#define CREATE_5_ARY_ICM(assign, str, arg1, arg2, arg3, arg4, arg5)                      \
    CREATE_4_ARY_ICM (assign, str, arg1, arg2, arg3, arg4);                              \
    MAKE_NEXT_ICM_ARG (icm_arg, arg5)

#define BIN_ICM_REUSE(reuse, str, arg1, arg2)                                            \
    NODE_TYPE (reuse) = N_icm;                                                           \
    ICM_NAME (reuse) = str;                                                              \
    ICM_ARGS (reuse) = MakeExprs (arg1, NULL);                                           \
    ICM_INDENT_BEFORE (reuse) = 0;                                                       \
    ICM_INDENT_AFTER (reuse) = 0;                                                        \
    icm_arg = ICM_ARGS (reuse);                                                          \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2)

#define TRI_ICM_REUSE(reuse, str, arg1, arg2, arg3)                                      \
    NODE_TYPE (reuse) = N_icm;                                                           \
    ICM_NAME (reuse) = str;                                                              \
    ICM_ARGS (reuse) = MakeExprs (arg1, NULL);                                           \
    ICM_INDENT_BEFORE (reuse) = 0;                                                       \
    ICM_INDENT_AFTER (reuse) = 0;                                                        \
    icm_arg = ICM_ARGS (reuse);                                                          \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2);                                                   \
    MAKE_NEXT_ICM_ARG (icm_arg, arg3)

#define APPEND_ASSIGNS(first, next)                                                      \
    ASSIGN_NEXT (first) = next;                                                          \
    first = next

/******************************************************************************
 *
 * Function:
 *   node *MakeTypeNode( types *type)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
MakeTypeNode (types *type)
{
    node *ret_node;
    simpletype basetype;
    char *str, *ret;

    DBUG_ENTER ("MakeTypeNode");

    basetype = GetBasetype (type);

    if (basetype == T_user) {
        str = TYPES_NAME (type);
        DBUG_ASSERT ((str != NULL), "Name of user-defined type not found");
    } else {
        str = type_string[basetype];
    }

    if (GetDim (type) != 0) {
        ret = (char *)MALLOC (sizeof (char) * (strlen (str) + 3));
        strcpy (ret, str);
        strcat (ret, " *");
    } else {
        ret = StringCopy (str);
    }

    ret_node = MakeId (ret, NULL, ST_regular);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeBasetypeNode( types *type)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
MakeBasetypeNode (types *type)
{
    node *ret_node;
    simpletype basetype;
    char *str;

    DBUG_ENTER ("MakeBasetypeNode");

    /*
     * We do not necessaryly need the implementation type here, because the
     * compiled code contains proper typedef statements already :-)
     */
#if 1
    basetype = GetBasetype (type);
#else
    basetype = TYPES_BASETYPE (type);
#endif

    if (basetype == T_user) {
        str = TYPES_NAME (type);
        DBUG_ASSERT ((str != NULL), "Name of user-defined type not found");
    } else {
        str = type_string[basetype];
    }

    ret_node = MakeId_Copy (str);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   char *GenericFun( int which, types *type)
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
 *   node *GetFoldCode( node *fundef)
 *
 * description:
 *   Returns the foldop-code of the pseudo fold-fun 'fundef'.
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
GetFoldCode (node *fundef)
{
    node *fold_code, *tmp;

    DBUG_ENTER ("GetFoldCode");

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
 *   node *MakeIncRcIcm( char *name, types *type, int rc, int num)
 *
 * description:
 *   Builds a ND_INC_RC( name, num) icm if (rc >= 0) and (num > 0).
 *
 ******************************************************************************/

static node *
MakeIncRcIcm (char *name, types *type, int rc, int num)
{
    node *id;
    node *icm = NULL;

    DBUG_ENTER ("MakeIncRcIcm");

    if (rc >= 0) { /* is it a refcounted data? */
        DBUG_ASSERT ((num >= 0), "increment for rc must be >= 0.");

        if (num > 0) {
            id = MakeId_Copy (name);

            icm = MakeAssignIcm2 ("ND_INC_RC", id, MakeNum (num));
        }
    }

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * function:
 *   node *MakeDecRcIcm( char *name, types* type, int rc, int num)
 *
 * description:
 *   According to 'type', 'rc' and 'num', builds either
 *             a ND_DEC_RC( name, num) icm,
 *          or a ND_DEC_RC_FREE_HIDDEN( name, num, freefun) icm,
 *          or a ND_DEC_RC_FREE_ARRAY( name, num) icm,
 *          or no icm at all.
 *
 ******************************************************************************/

static node *
MakeDecRcIcm (char *name, types *type, int rc, int num)
{
    node *id;
    node *icm = NULL;

    DBUG_ENTER ("MakeDecRcIcm");

    if (rc >= 0) { /* is it a refcounted data? */
        DBUG_ASSERT ((num >= 0), "decrement for rc must be >= 0.");

        if (num > 0) {
            id = MakeId_Copy (name);

            if (rc - num > 0) { /* definitely no FREE needed? */
                icm = MakeAssignIcm2 ("ND_DEC_RC", id, MakeNum (num));
            } else {
                if (IsNonUniqueHidden (type)) {
                    icm = MakeAssignIcm3 ("ND_DEC_RC_FREE_HIDDEN", id, MakeNum (num),
                                          MakeId_Copy (GenericFun (1, type)));
                } else {
                    icm = MakeAssignIcm2 ("ND_DEC_RC_FREE_ARRAY", id, MakeNum (num));
                }
            }
        }
    }

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * function:
 *   node *MakeAdjustRcIcm( char *name, types* type, int rc, int num)
 *
 * description:
 *   According to num, either a ND_INC_RC( varname, num) icm,
 *                       or   no ICM at all,
 *                       or   a ND_DEC_RC_...( varname, -num) icm
 *   is created.
 *
 ******************************************************************************/

static node *
MakeAdjustRcIcm (char *name, types *type, int rc, int num)
{
    node *icm = NULL;

    DBUG_ENTER ("MakeAdjustRcIcm");

    if (rc >= 0) { /* is it a refcounted data? */
        if (num > 0) {
            icm = MakeIncRcIcm (name, type, rc, num);
        } else if (num < 0) {
            icm = MakeDecRcIcm (name, type, rc, -num);
        }
    }

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * function:
 *   node *Ids2AllocArrayICMs( ids *ids_chain, node *next)
 *
 * description:
 *   Builds a ND_ALLOC_ARRAY icm for each RC-ids in 'ids_chain'.
 *   The given node 'next' is appended to the created assign-chain.
 *
 *   CAUTION: Do not use this function in conjunction with a
 *            'ND_CHECK_REUSE_ARRAY' icm.
 *            Use 'MakeAllocArrayICMs_reuse()' instead!!
 *
 ******************************************************************************/

static node *
Ids2AllocArrayICMs (ids *ids_chain, node *next)
{
    node *assign;
    node *last_assign = NULL;
    node *assigns = NULL;

    DBUG_ENTER ("Ids2AllocArrayICMs");

    while (ids_chain != NULL) {
        if (IDS_REFCNT (ids_chain) >= 0) {
            assign
              = MakeAssignIcm3 ("ND_ALLOC_ARRAY", MakeBasetypeNode (IDS_TYPE (ids_chain)),
                                DupIds_Id (ids_chain), MakeNum (IDS_REFCNT (ids_chain)));

            if (assigns == NULL) {
                assigns = assign;
            } else {
                ASSIGN_NEXT (last_assign) = assign;
            }
            last_assign = assign;
        }
        ids_chain = IDS_NEXT (ids_chain);
    }

    if (last_assign != NULL) {
        ASSIGN_NEXT (last_assign) = next;
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *Ids2AllocArrayICMs_reuse( ids *ids_chain, node *next)
 *
 * description:
 *   builds a 'ND_ALLOC_ARRAY, ND_INC_RC' icm for each RC-ids in 'ids_chain':
 *     ND_ALLOC_ARRAY( <type>, ids_chain, 0);
 *     ND_INC_RC( ids_chain, IDS_RC( ids_chain));
 *   The extra 'ND_INC_RC' is needed, if there are any 'ND_CHECK_REUSE_ARRAY'
 *   ICMs above 'ND_ALLOC_ARRAY' !!!
 *
 *   The given node 'next' is appended to the created assign-chain.
 *
 ******************************************************************************/

static node *
Ids2AllocArrayICMs_reuse (ids *ids_chain, node *next)
{
    node *assign;
    node *last_assign = NULL;
    node *assigns = NULL;

    DBUG_ENTER ("Ids2AllocArrayICMs_reuse");

    while (ids_chain != NULL) {
        if (IDS_REFCNT (ids_chain) >= 0) {
            assign = MakeAssign (MakeIcm3 ("ND_ALLOC_ARRAY",
                                           MakeBasetypeNode (IDS_TYPE (ids_chain)),
                                           DupIds_Id (ids_chain), MakeNum (0)),
                                 MakeAssignIcm2 ("ND_INC_RC", DupIds_Id (ids_chain),
                                                 MakeNum (IDS_REFCNT (ids_chain))));

            if (assigns == NULL) {
                assigns = assign;
            } else {
                ASSIGN_NEXT (last_assign) = assign;
            }
            last_assign = ASSIGN_NEXT (assign);
        }
        ids_chain = IDS_NEXT (ids_chain);
    }

    if (last_assign != NULL) {
        ASSIGN_NEXT (last_assign) = next;
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *Ids2IncRcICMs( ids *ids_chain, node *next)
 *
 * description:
 *   Builds a ND_INC_RC( name, rc) icm for each ids in 'ids_chain'.
 *   The given node 'next' is appended to the created assign-chain.
 *
 ******************************************************************************/

static node *
Ids2IncRcICMs (ids *ids_chain, node *next)
{
    node *assign;
    node *last_assign = NULL;
    node *assigns = NULL;

    DBUG_ENTER ("Ids2IncRcICMs");

    while (ids_chain != NULL) {
        assign = MakeIncRcIcm (IDS_NAME (ids_chain), IDS_TYPE (ids_chain),
                               IDS_REFCNT (ids_chain), IDS_REFCNT (ids_chain));

        if (assign != NULL) {
            if (assigns == NULL) {
                assigns = assign;
            } else {
                ASSIGN_NEXT (last_assign) = assign;
            }
            last_assign = assign;
        }

        ids_chain = IDS_NEXT (ids_chain);
    }

    if (last_assign != NULL) {
        ASSIGN_NEXT (last_assign) = next;
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *Ids2DecRcICMs( ids *ids_chain, node *next)
 *
 * description:
 *   According to rc and type, builds either
 *             a ND_DEC_RC( name, 1)
 *          or a ND_DEC_RC_FREE_HIDDEN( name, 1, freefun)
 *          or a ND_DEC_RC_FREE_ARRAY( name, 1)
 *   icm for each ids in 'ids_chain'.
 *   The given node 'next' is appended to the created assign-chain.
 *
 ******************************************************************************/

static node *
Ids2DecRcICMs (ids *ids_chain, node *next)
{
    node *assign;
    node *last_assign = NULL;
    node *assigns = NULL;

    DBUG_ENTER ("Ids2DecRcICMs");

    while (ids_chain != NULL) {
        assign = MakeDecRcIcm (IDS_NAME (ids_chain), IDS_TYPE (ids_chain),
                               IDS_REFCNT (ids_chain), 1);

        if (assign != NULL) {
            if (assigns == NULL) {
                assigns = assign;
            } else {
                ASSIGN_NEXT (last_assign) = assign;
            }
            last_assign = assign;
        }

        ids_chain = IDS_NEXT (ids_chain);
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
 *    because the params of the corresponding ICMs must be N_id-nodes...
 *
 ******************************************************************************/

static node *
IdOrNumToIndex (node *id_or_num, int dim)
{
    node *index;
    char *str;

    DBUG_ENTER ("IdOrNumToIndex");

    if (NODE_TYPE (id_or_num) == N_id) {
        str = (char *)MALLOC ((strlen (ID_NAME (id_or_num)) + 25) * sizeof (char));
        sprintf (str, "SAC_ND_READ_ARRAY( %s,  %d)", ID_NAME (id_or_num), dim);
        index = MakeId_Copy (str);
    } else {
        DBUG_ASSERT ((NODE_TYPE (id_or_num) == N_num), "wrong node type found");
        index = DupNode (id_or_num);
    }

    DBUG_RETURN (index);
}

/******************************************************************************/

static void
AdjustAddedAssigns (node *before, node *after)
{
    char *new_id, *old_id;
    node *tmp, *last;

    DBUG_ENTER ("AdjustAddedAssigns");

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
 *  functionname  : MergeIcmsAp
 *  arguments     : 1) icm for out-parameter which is already situated
 *                     in the table
 *                  2) icm for in-parameter which was to be added to the
 *                     table when the mapping was detected.
 *                  3) type of parameter
 *                  4) refcount of variable
 *
 */

static node *
MergeIcmsAp (node *out_icm, node *in_icm, types *type, int rc)
{
    node *new_assign = NULL;

    DBUG_ENTER ("MergeIcmsAp");

    DBUG_ASSERT ((NODE_TYPE (ICMPARAM_ARG (out_icm)) == N_id),
                 "out-argument of application must be a N_id node");

    if (IsBoxed (type)) {
        ICMPARAM_TAG (out_icm) = "upd_bx";

        DBUG_ASSERT ((NODE_TYPE (ICMPARAM_ARG (in_icm)) == N_id),
                     "boxed in-argument of application must be a N_id node");

        if (IsArray (type)) {
            if (IsUnique (type)) {
                DBUG_PRINT ("COMP", ("Merging ICM-args: unique array %s - %s",
                                     ID_NAME (ICMPARAM_ARG (in_icm)),
                                     ID_NAME (ICMPARAM_ARG (out_icm))));

                if (strcmp (ID_NAME (ICMPARAM_ARG (in_icm)),
                            ID_NAME (ICMPARAM_ARG (out_icm)))) {
                    new_assign = MakeAssignIcm2 ("ND_KS_NO_RC_ASSIGN_ARRAY",
                                                 DupNode (ICMPARAM_ARG (in_icm)),
                                                 DupNode (ICMPARAM_ARG (out_icm)));
                }
            } else if (rc == 1) {
                DBUG_PRINT ("COMP", ("Merging ICM-args non-unique array with rc==1"
                                     " %s - %s",
                                     ID_NAME (ICMPARAM_ARG (in_icm)),
                                     ID_NAME (ICMPARAM_ARG (out_icm))));

                new_assign = MakeAssignIcm3 ("ND_KS_MAKE_UNIQUE_ARRAY",
                                             DupNode (ICMPARAM_ARG (in_icm)),
                                             DupNode (ICMPARAM_ARG (out_icm)),
                                             MakeNum (GetBasetypeSize (type)));
            } else {
                DBUG_PRINT ("COMP", ("Merging ICM-args non-unique array with rc>1"
                                     " %s - %s",
                                     ID_NAME (ICMPARAM_ARG (in_icm)),
                                     ID_NAME (ICMPARAM_ARG (out_icm))));

                new_assign
                  = MakeAssignIcm3 ("ND_KS_COPY_ARRAY", DupNode (ICMPARAM_ARG (in_icm)),
                                    DupNode (ICMPARAM_ARG (out_icm)),
                                    MakeNum (GetBasetypeSize (type)));
            }
        } else if (IsUnique (type)) {
            DBUG_PRINT ("COMP", ("Merging ICM-args unique hidden %s - %s",
                                 ID_NAME (ICMPARAM_ARG (in_icm)),
                                 ID_NAME (ICMPARAM_ARG (out_icm))));

            if (strcmp (ID_NAME (ICMPARAM_ARG (in_icm)),
                        ID_NAME (ICMPARAM_ARG (out_icm)))) {
                new_assign = MakeAssignIcm2 ("ND_NO_RC_ASSIGN_HIDDEN",
                                             DupNode (ICMPARAM_ARG (in_icm)),
                                             DupNode (ICMPARAM_ARG (out_icm)));
            }
        } else if (rc == 1) {
            DBUG_PRINT ("COMP", ("Merging ICM-args non-unique hidden %s - %s"
                                 " with rc==1",
                                 ID_NAME (ICMPARAM_ARG (in_icm)),
                                 ID_NAME (ICMPARAM_ARG (out_icm))));

            new_assign
              = MakeAssignIcm3 ("ND_MAKE_UNIQUE_HIDDEN", DupNode (ICMPARAM_ARG (in_icm)),
                                DupNode (ICMPARAM_ARG (out_icm)),
                                MakeId_Copy (GenericFun (0, type)));
        } else {
            DBUG_PRINT ("COMP", ("Merging ICM-args non-unique hidden %s - %s"
                                 " with rc>1",
                                 ID_NAME (ICMPARAM_ARG (in_icm)),
                                 ID_NAME (ICMPARAM_ARG (out_icm))));

            new_assign
              = MakeAssignIcm3 ("ND_KS_COPY_HIDDEN", DupNode (ICMPARAM_ARG (in_icm)),
                                DupNode (ICMPARAM_ARG (out_icm)),
                                MakeId_Copy (GenericFun (0, type)));
        }
    } else {
        ICMPARAM_TAG (out_icm) = "upd";

        DBUG_PRINT ("COMP",
                    ("Merging ICM-args unboxed %s", ID_NAME (ICMPARAM_ARG (out_icm))));

        if ((NODE_TYPE (ICMPARAM_ARG (in_icm)) != N_id)
            || strcmp (ID_NAME (ICMPARAM_ARG (in_icm)),
                       ID_NAME (ICMPARAM_ARG (out_icm)))) {
            new_assign = MakeAssign (MakeLet (DupNode (ICMPARAM_ARG (in_icm)),
                                              DupId_Ids (ICMPARAM_ARG (out_icm))),
                                     NULL);
        }
    }

    DBUG_PRINT ("COMP", ("Merging icm args of \"ND_FUN_AP\", new tag=\"%s\"",
                         ID_NAME (EXPRS_EXPR (out_icm))));

    DBUG_RETURN (new_assign);
}

/******************************************************************************/

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
 * Function:
 *   node *CreateApIcm( char *name, node **icm_tab, int tab_size)
 *
 * Description:
 *   Builds a ND_FUN_AP icm.
 *
 * Remark:
 *   The content of 'icm_tab' is removed!!
 *
 ******************************************************************************/

static node *
CreateApIcm (char *name, node **icm_tab, int tab_size)
{
    node *ret_node, *arg2, *tmp;
    int cnt_icm, i;

    DBUG_ENTER ("CreateApIcm");

    DBUG_PRINT ("COMP", ("Creating ICM \"ND_FUN_AP\""));

    if (icm_tab[1] == NULL) {
        arg2 = MakeId_Copy ("");
    } else {
        /* tag is not needed! */
        icm_tab[1] = FreeNode (icm_tab[1]);
        arg2 = icm_tab[1];
        icm_tab[1] = NULL;
    }

    /*
     * count the number of ICM arguments
     */
    cnt_icm = 0;
    tmp = icm_tab[0];
    while (tmp != NULL) {
        cnt_icm++;
        tmp = EXPRS_NEXT (EXPRS_NEXT (tmp));
    }
    for (i = 2; i < tab_size; i++) {
        if (icm_tab[i] != NULL) {
            cnt_icm++;
        }
    }

    ret_node = MakeAssignIcm3 ("ND_FUN_AP", MakeId_Copy (name), arg2, MakeNum (cnt_icm));

    /*
     * The ICM arguments are extracted from the table and inserted into the ICM
     */
    tmp = ICM_EXPRS3 (ASSIGN_INSTR (ret_node));
    for (i = 2; i < tab_size; i++) {
        if (icm_tab[i] != NULL) {
            EXPRS_NEXT (tmp) = icm_tab[i];
            tmp = EXPRS_NEXT (icm_tab[i]);
            icm_tab[i] = NULL;
        }
    }

    if (icm_tab[0] != NULL) {
        EXPRS_NEXT (tmp) = icm_tab[0];
        icm_tab[0] = NULL;
    }

    DBUG_RETURN (ret_node);
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
    icm_arg = MakeExprs (MakeId_Copy (name), NULL);
    ICM_ARGS (icm) = icm_arg;

    /*
     *  Second argument is returntype, void if none given.
     *  returntype is stored in icm_tab[1]
     */
    if (icm_tab[1] == NULL) {
        MAKE_NEXT_ICM_ARG (icm_arg, MakeId_Copy ("void"));
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

    icm_arg = MakeExprs (MakeId_Copy (name), NULL);
    ICM_ARGS (icm) = icm_arg;

    MAKE_NEXT_ICM_ARG (icm_arg, MakeId_Copy (from));

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

/******************************************************************************
 *
 * function:
 *   node *CreateIcmMT2_FUN_DEC( char *kindof, node *fundef,
 *                               node **icm_tab, int tab_size)
 *
 * description:
 *   creates a MT2_FUN_DEC ICM.
 *
 ******************************************************************************/

static node *
CreateIcmMT2_FUN_DEC (char *kindof, node *fundef, node **icm_tab, int tab_size)
{
    node *icm, *icm_arg;
    int i;
    int cnt_icm = 0;

    DBUG_ENTER ("CreateIcmMT2_FUN_DEC");

    DBUG_PRINT ("COMP", ("Creating ICM \"MT2_FUN_DEC\""));

    icm = MakeIcm0 ("MT2_FUN_DEC");

    icm_arg = MakeExprs (MakeId_Copy (kindof), NULL);
    ICM_ARGS (icm) = icm_arg;

    MAKE_NEXT_ICM_ARG (icm_arg, MakeId_Copy (FUNDEF_NAME (fundef)));

    MAKE_NEXT_ICM_ARG (icm_arg, MakeId_Copy (FUNDEF_NAME (FUNDEF_LIFTEDFROM (fundef))));

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

/******************************************************************************/

static void
InsertApDotsParam (node **icm_tab, node *icm_arg)
{
    DBUG_ENTER ("InsertApDotsParam");

    DBUG_PRINT ("COMP", ("Inserting ...-param in application"));

    icm_tab[0] = icm_arg;

    DBUG_VOID_RETURN;
}

/******************************************************************************/

static node *
InsertApArgParam (node **icm_tab, node *icm_arg, types *type, int rc, int *linksign,
                  int cnt_param)
{
    node *icm_node = NULL;

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
         *  create special icm table depending on pragma linksign
         */

        if (icm_tab[linksign[cnt_param] + 1] == NULL) {
            icm_tab[linksign[cnt_param] + 1] = icm_arg;
        } else {
            icm_node = MergeIcmsAp (icm_tab[linksign[cnt_param] + 1], icm_arg, type, rc);
        }
    }

    DBUG_RETURN (icm_node);
}

/******************************************************************************/

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

/******************************************************************************
 *
 * Function:
 *   void InsertDefDotsParam(node **icm_tab, node *icm_args)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
InsertDefDotsParam (node **icm_tab, node *icm_args)
{
    DBUG_ENTER ("InsertDefDotsParam");

    icm_tab[0] = icm_args;

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   statustype InsertDefReturnParam( node **icm_tab, node *icm_args,
 *                                    types **type_tab, types *type_args,
 *                                    int *linksign, int cnt_param, int line)
 *
 * description:
 *   This function creates an entry in the icm_tab for a return type of a
 *   function definition. The location of the given return type is identified
 *   either by inspection of the linksign pragma or by maintaining the original
 *   sequence. However, the first non-refcounted return type is compiled to
 *   the corresponding C function's only return type while all others are
 *   indirectly implemented by means of reference parameters.
 *
 *   The function either returns ST_crettype for the designated return type
 *   or ST_regular for all other return types that will be mapped to
 *   reference parameters.
 *
 ******************************************************************************/

static statustype
InsertDefReturnParam (node **icm_tab, node *icm_args, types **type_tab, types *type_args,
                      int *linksign, int cnt_param, int line)
{
    statustype ret = ST_regular;

    DBUG_ENTER ("InsertDefReturnParam");

    DBUG_PRINT ("COMP", ("Inserting arg #%d, tag=\"%s\"", cnt_param,
                         ID_NAME (EXPRS_EXPR (icm_args))));

    if (linksign == NULL) {
        /*
         *  create standard icm table
         */

        if ((0 == strcmp (ID_NAME (EXPRS_EXPR (icm_args)), "out"))
            && (icm_tab[1] == NULL)) {
            icm_tab[1] = icm_args;
            ret = ST_crettype;
        } else {
            icm_tab[cnt_param + 2] = icm_args;
            ret = ST_regular;
        }
    } else {
        /*
         *  create special icm table depending on pragma linksign
         */

        if (icm_tab[linksign[cnt_param] + 1] == NULL) {
            icm_tab[linksign[cnt_param] + 1] = icm_args;
            type_tab[linksign[cnt_param] + 1] = type_args;
            if (linksign[cnt_param] == 0) {
                ret = ST_crettype;
            } else {
                ret = ST_regular;
            }
        } else {
            ERROR (line, ("Pragma 'linksign` illegal"));
            CONT_ERROR (("2 return parameters mapped to same position !"));
            ABORT_ON_ERROR;
        }
    }

    DBUG_RETURN (ret);
}

/******************************************************************************/

static void
InsertDefArgParam (node **icm_tab, node *icm_args, types **type_tab, types *type_args,
                   int *linksign, int cnt_param, int line)
{
    DBUG_ENTER ("InsertDefArgParam");

    DBUG_PRINT ("COMP", ("Inserting arg #%d, tag=\"%s\"", cnt_param,
                         ID_NAME (EXPRS_EXPR (icm_args))));

    if (linksign == NULL) {
        /*
         *  create standard icm table
         */
        icm_tab[cnt_param + 2] = icm_args;
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
            icm_tab[linksign[cnt_param] + 1] = icm_args;
        } else {
            MergeIcmsFundef (icm_tab[linksign[cnt_param] + 1], icm_args,
                             type_tab[linksign[cnt_param] + 1], type_args, line);
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *ReorganizeReturnIcm( node *ret_icm)
 *
 * description:
 *   The ND_FUN_RET() ICM handles the first out-parameter of a function
 *   different from all others since this one is compiled to the original
 *   single return value of a C function.
 *   This function reorganizes the given args for ND_FUN_RET() accordingly.
 *
 ******************************************************************************/

static node *
ReorganizeReturnIcm (node *ret_icm)
{
    node *icm_args, *icm_arg, *pred_arg, *ret_node, *tmp;
    bool first_out = FALSE;
    int cnt = 0;

    DBUG_ENTER ("ReorganizeReturnIcm");

    if (ret_icm != NULL) {
        icm_args = ICM_ARGS (ret_icm);
        if (icm_args != NULL) {
            pred_arg = EXPRS_NEXT (icm_args);
            icm_arg = EXPRS_NEXT (pred_arg);

            while (icm_arg != NULL) {
                if ((!first_out)
                    && (0 == strcmp ("out", ID_NAME (EXPRS_EXPR (icm_arg))))) {
                    first_out = TRUE;
                    /*
                     * copy the name of the current ICM arg into the first arg.
                     */
                    EXPRS_EXPR (icm_args) = FreeTree (EXPRS_EXPR (icm_args));
                    ret_node = EXPRS_NEXT (icm_arg);
                    EXPRS_EXPR (icm_args) = DupNode (EXPRS_EXPR (ret_node));

                    DBUG_ASSERT ((!strcmp (ID_NAME (EXPRS_EXPR (ret_node)),
                                           ID_NAME (EXPRS_EXPR (EXPRS_NEXT (ret_node))))),
                                 "code-name and decl-name of return-argument differ!");

                    /*
                     * remove the next three ICM args
                     */
                    tmp = EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (icm_arg)));
                    EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (icm_arg))) = NULL;
                    icm_arg = FreeTree (icm_arg);
                    icm_arg = EXPRS_NEXT (pred_arg) = tmp;
                } else {
                    pred_arg = EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (pred_arg)));
                    icm_arg = EXPRS_NEXT (pred_arg);
                    cnt++;
                }
            }
            NUM_VAL (EXPRS_EXPR (EXPRS_NEXT (icm_args))) = cnt;
        }
    }

    DBUG_RETURN (ret_icm);
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

static node *
BuildParamsByDFM (DFMmask_t *mask, char *tag, int *num_args, node *icm_args)
{
    node *vardec;
    char *rc_tag, *this_tag;

    DBUG_ENTER ("BuildParamsByDFM");

    DBUG_PRINT ("JHS", ("begin %s", tag));

    rc_tag = StringConcat (tag, "_rc");

    vardec = DFMGetMaskEntryDeclSet (mask);
    while (vardec != NULL) {

        DBUG_PRINT ("JHS", ("%s", NODE_TEXT (vardec)));
        DBUG_PRINT ("JHS", ("%s", VARDEC_OR_ARG_NAME (vardec)));

        if (VARDEC_OR_ARG_REFCNT (vardec) >= 0) {
            this_tag = rc_tag;
        } else {
            this_tag = tag;
        }
        icm_args
          = MakeExprs (MakeId_Copy (this_tag),
                       MakeExprs (MakeTypeNode (VARDEC_OR_ARG_TYPE (vardec)),
                                  MakeExprs (MakeId_Copy (VARDEC_OR_ARG_NAME (vardec)),
                                             icm_args)));
        if (num_args != NULL) {
            *num_args = *num_args + 1;
        }

        if (num_args != NULL) {
            DBUG_PRINT ("SPMD", ("bpbdfm num_args:%i %s", *num_args,
                                 VARDEC_OR_ARG_NAME (vardec)));
        } else {
            DBUG_PRINT ("SPMD", ("bpbdfm num_args:- %s", VARDEC_OR_ARG_NAME (vardec)));
        }

        vardec = DFMGetMaskEntryDeclSet (NULL);
    }

    FREE (rc_tag);

    DBUG_PRINT ("JHS", ("end %s", tag));

    DBUG_RETURN (icm_args);
}

/******************************************************************************
 *
 * function:
 *   node *BuildParamsByDFMfold( DFMmask_t *mask,
 *                               char *tag, int *num_args, node *icm_args)
 *
 * description:
 *   Builds tuple-chain (tag, type, name, fun) from dfm-foldmask mask,
 *   tag will we used as base for the tags (used raw or _rc is added),
 *   num_args will be incremented for each triplet added (maybe NULL),
 *   at the end of this chain icm_args will be concatenated.
 *
 ******************************************************************************/

static node *
BuildParamsByDFMfold (DFMfoldmask_t *mask, char *tag, int *num_args, node *icm_args)
{
    node *vardec;
    char *rc_tag, *this_tag;

    DBUG_ENTER ("BuildParamsByDFMfold");

    DBUG_PRINT ("JHS", ("begin %s", tag));

    rc_tag = StringConcat (tag, "_rc");

    while (mask != NULL) {
        vardec = DFMFM_VARDEC (mask);

        DBUG_PRINT ("JHS", ("%s", NODE_TEXT (vardec)));
        DBUG_PRINT ("JHS", ("%s", VARDEC_OR_ARG_NAME (vardec)));

        if (VARDEC_OR_ARG_REFCNT (vardec) >= 0) {
            this_tag = rc_tag;
        } else {
            this_tag = tag;
        }
        icm_args
          = MakeExprs (MakeId_Copy (this_tag),
                       MakeExprs (MakeTypeNode (VARDEC_OR_ARG_TYPE (vardec)),
                                  MakeExprs (MakeId_Copy (VARDEC_OR_ARG_NAME (vardec)),
                                             icm_args)));
        if (num_args != NULL) {
            *num_args = *num_args + 1;
        }

        if (num_args != NULL) {
            DBUG_PRINT ("SPMD", ("bpbdfm num_args:%i %s", *num_args,
                                 VARDEC_OR_ARG_NAME (vardec)));
        } else {
            DBUG_PRINT ("SPMD", ("bpbdfm num_args:- %s", VARDEC_OR_ARG_NAME (vardec)));
        }

        mask = DFMFM_NEXT (mask);
    }

    FREE (rc_tag);

    DBUG_PRINT ("JHS", ("end"));

    DBUG_RETURN (icm_args);
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

static int
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
        if (MULTIPLE_SEGS (wl_seg) == 0) {
            /*
             * is the next dim the last one and blocking activ?
             */
            if ((WLGRID_DIM (grid) + 2 == WLSEG_DIMS (seg))
                && (first_block_dim < WLSEG_DIMS (seg))) {
                icm_dim = first_block_dim;
            }
        } else { /* MULTIPLE_SEGS( wl_seg) != 0 */
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
 *   Compiles an N_modul node: traverses sons.
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
 * Function:
 *   node *COMPTypedef(node *arg_node, node *arg_info)
 *
 * Description:
 *   If needed an appropriate ICM is generated and stored in TYPEDEF_ICM.
 *   The rest of the N_typdef node ist left untouched!
 *
 ******************************************************************************/

node *
COMPTypedef (node *arg_node, node *arg_info)
{
    node *icm = NULL;

    DBUG_ENTER ("COMPTypedef");

    if (IsArray (TYPEDEF_TYPE (arg_node))) {
        icm = MakeIcm2 ("ND_TYPEDEF_ARRAY", MakeBasetypeNode (TYPEDEF_TYPE (arg_node)),
                        MakeId_Copy (TYPEDEF_NAME (arg_node)));
    } else if (IsHidden (TYPEDEF_TYPE (arg_node))) {
        icm = MakeIcm1 ("ND_TYPEDEF_HIDDEN", MakeId_Copy (TYPEDEF_NAME (arg_node)));
    }

    TYPEDEF_ICM (arg_node) = icm;

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = Trav (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPObjdef( node *arg_node, node *arg_info)
 *
 * Description:
 *   If needed an appropriate ICM is generated and stored in OBJDEF_ICM.
 *   The rest of the N_objdef node ist left untouched!
 *
 ******************************************************************************/

node *
COMPObjdef (node *arg_node, node *arg_info)
{
    node *res_type, *id_node, *dim_node;
    node *icm = NULL;

    DBUG_ENTER ("COMPObjdef");

    if (IsArray (OBJDEF_TYPE (arg_node))) {
        res_type = MakeBasetypeNode (OBJDEF_TYPE (arg_node));
        id_node = MakeId_Copy (OBJDEF_NAME (arg_node));
        dim_node = MakeNum (GetDim (OBJDEF_TYPE (arg_node)));

        if ((OBJDEF_STATUS (arg_node) == ST_imported_mod)
            || (OBJDEF_STATUS (arg_node) == ST_imported_class)) {
            icm = MakeIcm3 ("ND_KD_DECL_EXTERN_ARRAY", res_type, id_node, dim_node);
        } else {
            icm = MakeIcm4 ("ND_KS_DECL_GLOBAL_ARRAY", res_type, id_node, dim_node,
                            Type2Exprs (OBJDEF_TYPE (arg_node)));
        }
    }

    OBJDEF_ICM (arg_node) = icm;

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
 *   node *COMPFundef( node *arg_node, node *arg_info)
 *
 * description:
 *   Compiles a N_fundef node.
 *
 * *** CODE NOT BRUSHED YET ***
 *
 ******************************************************************************/

node *
COMPFundef (node *arg_node, node *arg_info)
{
    node *return_node, *return_icm, *icm_args;
    node *old_fundef, *vardec, *assigns, *assign;
    node **icm_tab;
    types *rettypes;
    types **type_tab;
    statustype old_actualattrib;
    int cnt_param, tab_size, i;
    node *front;
    node *back;

    DBUG_ENTER ("COMPFundef");

    DBUG_PRINT ("COMP", ("compiling %s", FUNDEF_NAME (arg_node)));

    DBUG_PRINT ("COMPjhs", ("compiling %s attrib: %s status: %s", FUNDEF_NAME (arg_node),
                            mdb_statustype[FUNDEF_ATTRIB (arg_node)],
                            mdb_statustype[FUNDEF_STATUS (arg_node)]));

    /*
     *  building icms for mt2,
     *  vardecs are needed for this operation, but vardecs
     *  will be rebuiled into icms, while compiling the body, so
     *  we do it now.
     */
    if (FUNDEF_ATTRIB (arg_node) == ST_call_mtlift) {
        front
          = MakeIcm4 ("MT2_WORKER_RECEIVE", MakeId_Copy ("SYNC"),
                      MakeNum (FUNDEF_IDENTIFIER (arg_node)),
                      MakeNum (DFMTestMask (FUNDEF_MT2USE (arg_node))),
                      BuildParamsByDFM (FUNDEF_MT2USE (arg_node), "receive", NULL, NULL));
        back
          = MakeIcm3 ("MT2_WORKER_BARRIER", MakeNum (FUNDEF_IDENTIFIER (arg_node)),
                      MakeNum (DFMTestMask (FUNDEF_MT2DEF (arg_node))),
                      BuildParamsByDFM (FUNDEF_MT2DEF (arg_node), "barrier", NULL, NULL));

        BLOCK_INSTR (FUNDEF_BODY (arg_node))
          = MakeAssign (front, BLOCK_INSTR (FUNDEF_BODY (arg_node)));

        /*
         * insert before return
         */
        assigns = BLOCK_INSTR (FUNDEF_BODY (arg_node));
        assign = MakeAssign (back, NULL);
        while ((ASSIGN_NEXT (assigns) != NULL)
               && (NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (assigns))) != N_return)) {
            assigns = ASSIGN_NEXT (assigns);
        }
        ASSIGN_NEXT (assign) = ASSIGN_NEXT (assigns);
        ASSIGN_NEXT (assigns) = assign;

        /* convert args into vardecs .... */
        if (FUNDEF_VARDEC (arg_node) != NULL) {
            vardec = FUNDEF_VARDEC (arg_node);
            while (VARDEC_NEXT (vardec) != NULL) {
                vardec = VARDEC_NEXT (vardec);
            }

            VARDEC_NEXT (vardec) = FUNDEF_ARGS (arg_node);
            vardec = VARDEC_NEXT (vardec);
        } else {
            FUNDEF_VARDEC (arg_node) = FUNDEF_ARGS (arg_node);
            vardec = FUNDEF_VARDEC (arg_node);
        }

        while (vardec != NULL) {
            /*
             * !!!!!!!!! THIS PART NEEDS RECODING !!!!!!!!!
             *
             * dkr: OUCH, this code will possibly fail with a modified tree
             *      implementation!!!
             */
            NODE_TYPE (vardec) = N_vardec; /* VERY ugly! */
            vardec = VARDEC_NEXT (vardec);
        }

        FUNDEF_ARGS (arg_node) = NULL;
    } /* ST_call_mtlift */

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
     * compile them ordered!), this includes the destruction of such a
     * N_sync-tree.
     * After compilation of the function the last known sync is destroyed then.
     */
    INFO_COMP_LAST_SYNC (arg_info) = NULL;

    /*
     * traverse body
     */
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    /*
     *  Destruction of last known N_sync is done here, all others have been killed
     *  while traversing.
     */
    if (INFO_COMP_LAST_SYNC (arg_info) != NULL) {
        INFO_COMP_LAST_SYNC (arg_info) = FreeTree (INFO_COMP_LAST_SYNC (arg_info));
        INFO_COMP_LAST_SYNC (arg_info) = NULL;
    }

    /********** end: traverse body **********/

    tab_size = CountFunctionParams (arg_node) + 2;
    icm_tab = (node **)MALLOC (sizeof (node *) * tab_size);
    type_tab = (types **)MALLOC (sizeof (types *) * tab_size);
    for (i = 0; i < tab_size; i++) {
        icm_tab[i] = NULL;
        type_tab[i] = NULL;
    }

    /*
     * compile return types
     */
    rettypes = FUNDEF_TYPES (arg_node);
    if ((rettypes != NULL) && (TYPES_BASETYPE (rettypes) != T_void)
        && (FUNDEF_BODY (arg_node) != NULL)) {
        /*
         * FUNDEF_RETURN( arg_node) points to a return-icm.
         * 'return_node' will point to the N_exprs in *front* off the first
         *  return value.
         */
        return_icm = FUNDEF_RETURN (arg_node);

        DBUG_ASSERT (((return_icm != NULL) && (NODE_TYPE (return_icm) == N_icm)),
                     "No return-ICM found");

        return_node = ICM_EXPRS2 (return_icm);
    } else {
        return_node = NULL;
    }

    cnt_param = 0;
    while ((rettypes != NULL) && (TYPES_BASETYPE (rettypes) != T_void)
           && (TYPES_BASETYPE (rettypes) != T_dots)) {
        /*
         * third ICM arg: var name
         */
        if (NULL == FUNDEF_BODY (arg_node)) {
            /* an extern declaration */
            icm_args = MakeExprs (MakeId_Copy (""), NULL);
        } else {
            /*
             * put return_node to next N_exprs where a function return_value
             * is behind
             */
            if (!strcmp (ICM_NAME (return_icm), "ND_FUN_RET")) {
                return_node = EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (return_node)));
            } else if ((!strcmp (ICM_NAME (return_icm), "MT_SPMD_FUN_RET"))
                       || (!strcmp (ICM_NAME (return_icm), "MT2_FUN_RET"))) {
                return_node = EXPRS_NEXT (EXPRS_NEXT (return_node));
            } else {
                DBUG_ASSERT ((0), "wrong return-ICM found");
            }

            DBUG_ASSERT ((return_node != NULL), "no return-value found");
            DBUG_ASSERT ((N_id == NODE_TYPE (EXPRS_EXPR (return_node))),
                         "return-value must be a N_id node!");

            icm_args = MakeExprs (DupNode (EXPRS_EXPR (return_node)), NULL);
        }

        /*
         * second ICM arg: full type
         */
        icm_args = MakeExprs (MakeTypeNode (rettypes), icm_args);

        /*
         * first ICM arg: tag
         */
        if ((IsArray (rettypes) + IsNonUniqueHidden (rettypes))
            && (FUN_DOES_REFCOUNT (arg_node, cnt_param))) {
            icm_args = MakeExprs (MakeId_Copy ("out_rc"), icm_args);
        } else {
            icm_args = MakeExprs (MakeId_Copy ("out"), icm_args);
        }

        TYPES_STATUS (rettypes)
          = InsertDefReturnParam (icm_tab, icm_args, type_tab, rettypes,
                                  (FUNDEF_PRAGMA (arg_node) == NULL)
                                    ? NULL
                                    : FUNDEF_LINKSIGN (arg_node),
                                  cnt_param, NODE_LINE (arg_node));

        rettypes = TYPES_NEXT (rettypes);
        cnt_param++;
    } /* while */

    if ((rettypes != NULL) && (TYPES_BASETYPE (rettypes) == T_dots)) {
        icm_args = MakeExprs (MakeId_Copy (""), NULL);
        icm_args = MakeExprs (MakeTypeNode (rettypes), icm_args);
        icm_args = MakeExprs (MakeId_Copy ("in"), icm_args);

        InsertDefDotsParam (icm_tab, icm_args);
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

    /*
     *  if mtn(==mt2) is set, we check if this is a lifted mt-function inspecting
     *  FUNDEF_ATTRIB, otherwise we check FUNDEF_STATUS to see if this is
     *  an spmd-function (old mt==mto) or decide it is a "normal" function.
     */
    if (gen_mt_code == GEN_MT_NEW) {
        switch (FUNDEF_ATTRIB (arg_node)) {
        case ST_call_mtlift:
            FUNDEF_ICM (arg_node)
              = CreateIcmMT2_FUN_DEC ("CALL_MTLIFT", arg_node, NULL, 0);
            break;

        case ST_call_mt_worker:
            FUNDEF_ICM (arg_node)
              = CreateIcmMT2_FUN_DEC ("CALL_MTWORKER", arg_node, icm_tab, tab_size);
            break;

        case ST_call_mt_master:
            FUNDEF_ICM (arg_node)
              = CreateIcmMT2_FUN_DEC ("CALL_MTMASTER", arg_node, icm_tab, tab_size);
            break;

        case ST_call_rep:
            FUNDEF_ICM (arg_node)
              = CreateIcmMT2_FUN_DEC ("CALL_MTREP", arg_node, icm_tab, tab_size);
            break;

        case ST_call_st:
            /* here is no break missing */
        case ST_regular:
            /* using normal function-declaration for single threaded calls */
            FUNDEF_RETURN (arg_node) = ReorganizeReturnIcm (FUNDEF_RETURN (arg_node));
            FUNDEF_ICM (arg_node)
              = CreateIcmND_FUN_DEC (FUNDEF_NAME (arg_node), icm_tab, tab_size);
            break;

        default:
            DBUG_PRINT ("jhs", ("%s", mdb_statustype[FUNDEF_ATTRIB (arg_node)]));
            DBUG_PRINT ("jhs", ("%s", mdb_statustype[FUNDEF_STATUS (arg_node)]));
            DBUG_ASSERT (0, "unknown kind of function while in mt2");
            break;
        }
    } else if (FUNDEF_STATUS (arg_node) == ST_spmdfun) {
        FUNDEF_ICM (arg_node)
          = CreateIcmMT_SPMD_FUN_DEC (FUNDEF_NAME (arg_node),
                                      FUNDEF_NAME (FUNDEF_LIFTEDFROM (arg_node)), icm_tab,
                                      tab_size);
    } else {
        FUNDEF_RETURN (arg_node) = ReorganizeReturnIcm (FUNDEF_RETURN (arg_node));
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

    /*
     * pop arg_info
     */
    INFO_COMP_FUNDEF (arg_info) = old_fundef;
    INFO_COMP_ACTUALATTRIB (arg_info) = old_actualattrib;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPArg( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 * *** CODE NOT BRUSHED YET ***
 *
 ******************************************************************************/

node *
COMPArg (node *arg_node, node *arg_info)
{
    node *icm_arg, *new_assign, *icm_tab_entry, *res_type;
    char *id_name;
    types *fulltype;
    char *tag;
    int dim;

    DBUG_ENTER ("COMPArg");

    id_name = (NULL != ARG_NAME (arg_node)) ? ARG_NAME (arg_node) : "";
    /* store name of formal parameter */

    res_type = MakeTypeNode (ARG_TYPE (arg_node));

    if ((IS_REFCOUNTED (ARG, arg_node))
        && (FUN_DOES_REFCOUNT (INFO_COMP_FUNDEF (arg_info),
                               INFO_COMP_CNTPARAM (arg_info)))) {

        if ((ARG_ATTRIB (arg_node) == ST_inout)
            || (ARG_ATTRIB (arg_node) == ST_spmd_inout)) {
            tag = "inout_rc";
        } else {
            tag = "in_rc";
        }

        /*
         * put ICMs for RC-adjustment at beginning of function block
         * if the function is *not* a SPMD-function
         */

        if (FUNDEF_STATUS (INFO_COMP_FUNDEF (arg_info)) != ST_spmdfun) {
            new_assign
              = MakeAdjustRcIcm (ARG_NAME (arg_node), ARG_TYPE (arg_node),
                                 ARG_REFCNT (arg_node), ARG_REFCNT (arg_node) - 1);
            if (new_assign != NULL) {
                ASSIGN_NEXT (new_assign) = INFO_COMP_FIRSTASSIGN (arg_info);
                INFO_COMP_FIRSTASSIGN (arg_info) = new_assign;
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

    icm_arg = MakeExprs (MakeId_Copy (tag), NULL);
    icm_tab_entry = icm_arg;
    MAKE_NEXT_ICM_ARG (icm_arg, res_type);
    MAKE_NEXT_ICM_ARG (icm_arg, MakeId_Copy (id_name));

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

    fulltype = GetTypes (ARG_TYPE (arg_node));
    dim = GetDim (ARG_TYPE (arg_node));
    if (dim > 0) {
        new_assign = MakeAssignIcm3 ("ND_KS_DECL_ARRAY_ARG", MakeId_Copy (id_name),
                                     MakeNum (dim), Type2Exprs (ARG_TYPE (arg_node)));

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
        if (IS_REFCOUNTED (ARG, arg_node)) {
            new_assign = MakeAssignIcm2 ("ND_DECL_INOUT_PARAM_RC", res_type,
                                         MakeId_Copy (id_name));
        } else {
            new_assign
              = MakeAssignIcm2 ("ND_DECL_INOUT_PARAM", res_type, MakeId_Copy (id_name));
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
 *   node *COMPVardec( node *arg_node, node *arg_info)
 *
 * description:
 *   Compiles a N_vardec node. The generated ICM chain is stored in
 *   VARDEC_ICM. The rest of the N_vardec node is left untouched in order
 *   to have the declarations still available. (Note, that each id contains a
 *   pointer to its vardec ...!)
 *
 ******************************************************************************/

node *
COMPVardec (node *arg_node, node *arg_info)
{
    int dim;
    node *icm = NULL;

    DBUG_ENTER ("COMPVardec");

    dim = GetDim (VARDEC_TYPE (arg_node));

    if (dim > SCALAR) {
        /*
         * array with known shape.
         */
        icm = MakeIcm4 ("ND_KS_DECL_ARRAY", MakeBasetypeNode (VARDEC_TYPE (arg_node)),
                        MakeId_Copy (VARDEC_NAME (arg_node)),
                        MakeNum (GetDim (VARDEC_TYPE (arg_node))),
                        Type2Exprs (VARDEC_TYPE (arg_node)));
    } else if (dim == UNKNOWN_SHAPE) {
        /*
         * array with unknown shape and dimension.
         */
        icm = MakeIcm2 ("ND_DECL_ARRAY", MakeBasetypeNode (VARDEC_TYPE (arg_node)),
                        MakeId_Copy (VARDEC_NAME (arg_node)));
    } else if (dim < KNOWN_DIM_OFFSET) {
        /*
         * array with unknown shape and known dimension.
         */
        icm = MakeIcm3 ("ND_KD_DECL_ARRAY", MakeBasetypeNode (VARDEC_TYPE (arg_node)),
                        MakeId_Copy (VARDEC_NAME (arg_node)),
                        MakeNum (KNOWN_DIM_OFFSET - dim));
    } else if (IsNonUniqueHidden (VARDEC_TYPE (arg_node))) {
        /*
         * non-unique abstract data type (implicit type)
         */
        icm = MakeIcm2 ("ND_DECL_RC", MakeBasetypeNode (VARDEC_TYPE (arg_node)),
                        MakeId_Copy (VARDEC_NAME (arg_node)));
    }

    VARDEC_ICM (arg_node) = icm;

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

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
 * *** CODE NOT BRUSHED YET ***
 *
 *******************************************************************************/

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
          = (char *)MALLOC (strlen (BLOCK_CACHESIM (arg_node)) + strlen (fun_name) + 14);
        if (BLOCK_CACHESIM (arg_node)[0] == '\0') {
            sprintf (cs_tag, "\"%s(...)\"", fun_name);
        } else {
            sprintf (cs_tag, "\"%s in %s(...)\"", BLOCK_CACHESIM (arg_node), fun_name);
        }

        FREE (BLOCK_CACHESIM (arg_node));

        DBUG_ASSERT ((BLOCK_INSTR (arg_node) != NULL),
                     "first instruction of block is NULL"
                     " (should be a N_empty node)");
        assign = BLOCK_INSTR (arg_node);

        BLOCK_INSTR (arg_node) = MakeAssign (MakeIcm1 ("CS_START", MakeId_Copy (cs_tag)),
                                             BLOCK_INSTR (arg_node));

        while ((ASSIGN_NEXT (assign) != NULL)
               && (NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (assign))) != N_return)) {
            assign = ASSIGN_NEXT (assign);
        }

        ASSIGN_NEXT (assign)
          = MakeAssign (MakeIcm1 ("CS_STOP", MakeId_Copy (cs_tag)), ASSIGN_NEXT (assign));
    }

    INFO_COMP_LASTASSIGN (arg_info) = arg_node;

    BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
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
 * *** CODE NOT BRUSHED YET ***
 *
 ******************************************************************************/

node *
COMPAssign (node *arg_node, node *arg_info)
{
    node *old_last_assign, *old_next_assign;
    node *instr, *last;

    DBUG_ENTER ("COMPAssign");

    old_last_assign = INFO_COMP_LASTASSIGN (arg_info);
    old_next_assign = ASSIGN_NEXT (arg_node);

    instr = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (NODE_TYPE (instr) == N_assign) {
        /*
         * an assignment-chain was returned.
         *  => insert it at the current position into the tree.
         */

        /* insert head of 'instr' into AST */
        ASSIGN_INSTR (arg_node) = ASSIGN_INSTR (instr);

        /* insert tail of 'instr' into AST (last element) */
        last = instr;
        while (ASSIGN_NEXT (last) != NULL) {
            last = ASSIGN_NEXT (last);
        }
        ASSIGN_NEXT (last) = ASSIGN_NEXT (arg_node);

        /* update INFO_COMP_LASTASSIGN */
        /*
        INFO_COMP_LASTASSIGN( arg_info) = last;
        */

        /* free head of 'instr' */
        ASSIGN_INSTR (instr) = NULL;
        instr = FreeNode (instr);

        /* insert tail of 'instr' into AST (first element) */
        ASSIGN_NEXT (arg_node) = instr;
    } else {
        /* insert 'instr' into AST */
        ASSIGN_INSTR (arg_node) = instr;

        /* update INFO_COMP_LASTASSIGN */
        /*
        INFO_COMP_LASTASSIGN( arg_info) = arg_node;
        */
    }

    /*
     * compile next assignment
     */
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
     * Although the new assigns are already inserted correctly into
     * the chain of assignments, we have to return the correct pointer here !!!
     */
    arg_node = BLOCK_INSTR_OR_ASSIGN_NEXT (old_last_assign);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPNormalFunReturn(node *arg_node, node *arg_info)
 *
 * description:
 *   generates ICMs for N_return-node found in body of a non-SPMD-function.
 *
 * *** CODE NOT BRUSHED YET ***
 *
 ******************************************************************************/

node *
COMPNormalFunReturn (node *arg_node, node *arg_info)
{
    node *ret_exprs, *ret_expr, *icm_args, *last_icm_arg, *tmp;
    int ret_cnt;
    bool rename;

    DBUG_ENTER ("COMPNormalFunReturn");

    /*
     * Create dummy values for the first two ICM args.
     * These values are corrected later on by the function ReorganizeReturnIcm().
     */
    icm_args = MakeExprs (MakeId_Copy (""), MakeExprs (MakeNum (0), NULL));
    last_icm_arg = EXPRS_NEXT (icm_args);

    /*
     * First, the real return values are traversed.
     */
    ret_exprs = RETURN_EXPRS (arg_node);
    ret_expr = ret_exprs;
    ret_cnt = 0;
    while (ret_expr != NULL) {
        /*
         * Search for a return variable with the same name as 'ret_expr'
         * (we must examine the predecessors of the exprs-chain only!)
         *
         * Example:
         *   The following SAC function
         *       int[], int[] fun( int[] A)
         *       {
         *         return( A, A);
         *       }
         *   must be compiled into:
         *       SAC_ND_FUN_DEC( fun, void, 3,
         *                       out_rc, int*, A,
         *                       out_rc, int*, A_1,
         *                       in_rc,  int*, A)
         *       {
         *         ...
         *         SAC_ND_FUN_RET( , 3, out_rc, A, A, out_rc, A, A_1)
         *       }
         *   Note, that the second out-argument (A) must be renamed in the
         *   SAC_ND_FUN_DEC() ICM, in order to prevent a name conflict with the
         *   first out-argument!!
         *   The renaming from the code-name (A) into the decl-name (A_1) is
         *   done via the SAC_ND_FUN_RET() ICM.
         */
        rename = FALSE;
        tmp = ret_exprs;
        while (tmp != ret_expr) {
            DBUG_ASSERT (((NODE_TYPE (EXPRS_EXPR (ret_expr)) == N_id)
                          && (NODE_TYPE (EXPRS_EXPR (tmp)) == N_id)),
                         "return value must be of type N_id!");
            if (!strcmp (ID_NAME (EXPRS_EXPR (ret_expr)), ID_NAME (EXPRS_EXPR (tmp)))) {
                /*
                 * 'ret_expr' and 'tmp' have the same name!
                 */
                rename = TRUE;
                break;
            }
            tmp = EXPRS_NEXT (tmp);
        }

        /*
         * Append out-tag to ICM args.
         *
         * Here we do not have to distinguish between functions that do the
         * refcounting on their own and those that do not because we are definitely
         * inside a SAC function and these always do their own refcounting.
         */
        if (IS_REFCOUNTED (ID, EXPRS_EXPR (ret_expr))) {
            last_icm_arg = EXPRS_NEXT (last_icm_arg)
              = MakeExprs (MakeId_Copy ("out_rc"), NULL);
        } else {
            last_icm_arg = EXPRS_NEXT (last_icm_arg)
              = MakeExprs (MakeId_Copy ("out"), NULL);
        }

        /*
         * Append the code-name and the decl-name to ICM args
         */
        last_icm_arg = EXPRS_NEXT (last_icm_arg)
          = MakeExprs (DupNode (EXPRS_EXPR (ret_expr)), NULL);
        if (rename) {
            last_icm_arg = EXPRS_NEXT (last_icm_arg)
              = MakeExprs (MakeId_Copy (TmpVarName (ID_NAME (EXPRS_EXPR (ret_expr)))),
                           NULL);
        } else {
            last_icm_arg = EXPRS_NEXT (last_icm_arg)
              = MakeExprs (DupNode (EXPRS_EXPR (ret_expr)), NULL);
        }

        ret_expr = EXPRS_NEXT (ret_expr);
        ret_cnt++;
    } /* while */

    /*
     * Second, the counterparts of reference parameters are traversed and added
     * to the chain of ICM args.
     */
    ret_expr = RETURN_REFERENCE (arg_node);
    while (ret_expr != NULL) {
        /*
         * Append inout-tag to ICM args.
         */
        if (IS_REFCOUNTED (ID, EXPRS_EXPR (ret_expr))) {
            last_icm_arg = EXPRS_NEXT (last_icm_arg)
              = MakeExprs (MakeId_Copy ("inout_rc"), NULL);
        } else {
            last_icm_arg = EXPRS_NEXT (last_icm_arg)
              = MakeExprs (MakeId_Copy ("inout"), NULL);
        }

        /*
         * Append the code-name and the decl-name to ICM args
         */
        last_icm_arg = EXPRS_NEXT (last_icm_arg)
          = MakeExprs (DupNode (EXPRS_EXPR (ret_expr)),
                       MakeExprs (DupNode (EXPRS_EXPR (ret_expr)), NULL));

        ret_expr = EXPRS_NEXT (ret_expr);
        ret_cnt++;
    }
    /*
     * correct the ICM arg that contains the number of arguments
     */
    NUM_VAL (EXPRS_EXPR (EXPRS_NEXT (icm_args))) = ret_cnt;

    /*
     * replace N_return node by a new N_icm node
     */
    DBUG_ASSERT ((FUNDEF_RETURN (INFO_COMP_FUNDEF (arg_info)) == arg_node),
                 "FUNDEF_RETURN not found via 'arg_info'!");

    arg_node = FreeTree (arg_node);
    if (last_icm_arg == EXPRS_NEXT (icm_args)) {
        icm_args = FreeTree (icm_args);
        arg_node = MakeIcm ("NOOP", icm_args);
    } else {
        arg_node = MakeIcm ("ND_FUN_RET", icm_args);
    }

    FUNDEF_RETURN (INFO_COMP_FUNDEF (arg_info)) = arg_node;

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
 * *** CODE NOT BRUSHED YET ***
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
 *   node *COMPMT2FunReturn(node *arg_node, node *arg_info)
 *
 * description:
 *   generates ICMs for N_return-node found in body of a MT2-function.
 *
 * *** CODE NOT BRUSHED YET ***
 *
 ******************************************************************************/

node *
COMPMT2FunReturn (node *arg_node, node *arg_info)
{
    node *exprs, *args, *last_arg, *tag;
    int cnt_params;

    DBUG_ENTER ("COMPMT2FunReturn");

    exprs = RETURN_EXPRS (arg_node);
    args = NULL;
    cnt_params = 0;
#if 1
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
#endif
    args = MakeExprs (MakeNum (barrier_id), args);

    arg_node = MakeIcm1 ("MT2_FUN_RET", args);
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
 * *** CODE NOT BRUSHED YET ***
 *
 ******************************************************************************/

node *
COMPReturn (node *arg_node, node *arg_info)
{
    node *fundef;

    DBUG_ENTER ("COMPReturn");

    fundef = INFO_COMP_FUNDEF (arg_info);

    if (gen_mt_code == GEN_MT_NEW) {
        if ((FUNDEF_ATTRIB (fundef) == ST_call_mtlift)
            || (FUNDEF_ATTRIB (fundef) == ST_call_mt_worker)
            || (FUNDEF_ATTRIB (fundef) == ST_call_mt_master)
            || (FUNDEF_ATTRIB (fundef) == ST_call_rep)) {
            arg_node = COMPMT2FunReturn (arg_node, arg_info);
        } else if ((FUNDEF_ATTRIB (fundef) == ST_call_st)
                   || (FUNDEF_ATTRIB (fundef) == ST_regular)) {
            arg_node = COMPNormalFunReturn (arg_node, arg_info);
        } else {
            DBUG_PRINT ("jhs", ("%s", mdb_statustype[FUNDEF_ATTRIB (fundef)]));
            DBUG_PRINT ("jhs", ("%s", mdb_statustype[FUNDEF_STATUS (fundef)]));
            DBUG_ASSERT (0, "unknown kind of function while in mt2");
        }
    } else if (FUNDEF_STATUS (fundef) == ST_spmdfun) {
        arg_node = COMPSpmdFunReturn (arg_node, arg_info);
    } else {
        arg_node = COMPNormalFunReturn (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPLet( node *arg_node, node *arg_info)
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
 * *** CODE NOT BRUSHED YET ***
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

        /*
         * CAUTION: Some old routines recycles the let-node as a N_block node!!!
         *          (e.g. COMPPrf when inserting 'ND_KS_DECL_ARRAY'-ICMs ...)
         *          In these cases we must not free 'arg_node'!!!
         */

        LET_EXPR (arg_node) = NULL;
        arg_node = FreeTree (arg_node);
        arg_node = expr;
    } else {
        LET_EXPR (arg_node) = expr;
    }

    INFO_COMP_LASTLET (arg_info) = NULL;
    INFO_COMP_LASTIDS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPAp( node *arg_node, node *arg_info)
 *
 * Description:
 *   Creates an ICM for function application and insert ICMs to decrement the
 *   RC of function arguments.
 *
 * Remarks:
 *   The flattening phase assures that no one of the arguments occurs on the LHS
 *   of the application (a = fun(a) is impossible).
 *
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned var.
 *   INFO_COMP_LASTASSIGN(arg_info) contains pointer to node before last assign.
 *   INFO_COMP_LASTLET(arg_info) contains pointer to previous let.
 *
 ******************************************************************************/

node *
COMPAp (node *arg_node, node *arg_info)
{
    node *icm_node, *last_node, *ret_node, *merge_node;
    node **icm_tab;
    node *last_entry, *ret_entry;
    ids *let_ids;
    types *fundef_types;
    node *fundef_args, *args, *arg;
    char *tag;
    bool ids_for_dots;
    int tab_size, cnt_param, arg_idx, i;

    DBUG_ENTER ("COMPAp");

    DBUG_PRINT ("COMP", ("COMPiling application of function %s",
                         ItemName (AP_FUNDEF (arg_node))));

    let_ids = LET_IDS (INFO_COMP_LASTLET (arg_info));
    fundef_types = FUNDEF_TYPES (AP_FUNDEF (arg_node));

    /*
     * create dummy node to append ICMs to
     */
    ret_node = last_node = MakeAssign (NULL, NULL);

    /*
     * init ICM tabular
     */
    tab_size = CountFunctionParams (AP_FUNDEF (arg_node)) + 2;
    icm_tab = (node **)MALLOC (sizeof (node *) * tab_size);
    for (i = 0; i < tab_size; i++) {
        icm_tab[i] = NULL;
    }

    /*
     * traverse LHS of let-clause
     */
    cnt_param = 0;
    ids_for_dots = FALSE;
    while (let_ids != NULL) {
        DBUG_PRINT ("COMP", ("Handling return value bound to %s", IDS_NAME (let_ids)));

#ifndef DBUG_OFF
        /* assure that no one of the array-arguments occurs on LHS */
        {
            node *args, *arg_id;
            args = AP_ARGS (arg_node);
#if 1
            arg_idx = 0; /*
                          * dkr:
                          * not correct yet!!!!!
                          * should be the number of LHS vars???
                          */
#endif
            while (args != NULL) {
                arg_id = EXPRS_EXPR (args);
                if ((NODE_TYPE (arg_id) == N_id) && IS_REFCOUNTED (ID, arg_id)
                    && (!FUN_DOES_REFCOUNT (AP_FUNDEF (arg_node), arg_idx))) {
                    /*
                     * dkr:
                     * Here, the interpretation of the refcouting-pragma is not correct
                     * yet!!!!!
                     */
                    if (!strcmp (IDS_NAME (let_ids), ID_NAME (arg_id))) {
                        DBUG_ASSERT ((0),
                                     "application of a user-defined function without "
                                     "own refcouting: "
                                     "refcounted argument occurs also on LHS!");
                    }
                }
                args = EXPRS_NEXT (args);
                arg_idx++;
            }
        }
#endif

        /* choose the right tag for the argument and generate ICMs for RC */
        if (IS_REFCOUNTED (IDS, let_ids)) {
            if (FUN_DOES_REFCOUNT (AP_FUNDEF (arg_node), cnt_param)) {
                tag = "out_rc";

                icm_node
                  = MakeAdjustRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                     IDS_REFCNT (let_ids), IDS_REFCNT (let_ids) - 1);
                if (icm_node != NULL) {
                    last_node = ASSIGN_NEXT (last_node) = icm_node;
                }
            } else {
                /* function does no refcounting */
                tag = "out";

                if (IDS_REFCNT (let_ids) > 0) {
                    icm_node = MakeAssignIcm1 ("ND_ALLOC_RC", DupIds_Id (let_ids));
                    last_node = ASSIGN_NEXT (last_node) = icm_node;

                    icm_node = MakeAssignIcm2 ("ND_SET_RC", DupIds_Id (let_ids),
                                               MakeNum (IDS_REFCNT (let_ids)));
                    last_node = ASSIGN_NEXT (last_node) = icm_node;
                } else if (IsNonUniqueHidden (IDS_TYPE (let_ids))) {
                    icm_node
                      = MakeAssignIcm2 ("ND_NO_RC_FREE_HIDDEN", DupIds_Id (let_ids),
                                        MakeId_Copy (GenericFun (1, IDS_TYPE (let_ids))));
                    last_node = ASSIGN_NEXT (last_node) = icm_node;
                } else {
                    icm_node
                      = MakeAssignIcm1 ("ND_NO_RC_FREE_ARRAY", DupIds_Id (let_ids));
                }
            }
        } else {
            /* let_ids is not refcounted */
            tag = "out";
        }

        if (!ids_for_dots) {
            last_entry = ret_entry = MakeExprs (MakeId_Copy (tag), NULL);

            last_entry = EXPRS_NEXT (last_entry) = MakeExprs (DupIds_Id (let_ids), NULL);

            if (TYPES_BASETYPE (fundef_types) == T_dots) {
                ids_for_dots = TRUE;
            } else {
                InsertApReturnParam (icm_tab, ret_entry, IDS_TYPE (let_ids),
                                     (FUNDEF_PRAGMA (AP_FUNDEF (arg_node)) == NULL)
                                       ? NULL
                                       : FUNDEF_LINKSIGN (AP_FUNDEF (arg_node)),
                                     cnt_param);

                fundef_types = TYPES_NEXT (fundef_types);
            }

            cnt_param++;
        } else {
            /* ids_for_dots == TRUE */

            last_entry = EXPRS_NEXT (last_entry) = MakeExprs (MakeId_Copy (tag), NULL);

            last_entry = EXPRS_NEXT (last_entry) = MakeExprs (DupIds_Id (let_ids), NULL);
        }

        let_ids = IDS_NEXT (let_ids);

        if (ids_for_dots && (let_ids == NULL)) {
            InsertApDotsParam (icm_tab, ret_entry);
        }
    } /* while( let_ids) */

    /*
     * traverse arguments of application
     */
    args = AP_ARGS (arg_node);
    fundef_args = FUNDEF_ARGS (AP_FUNDEF (arg_node));
    ids_for_dots = FALSE;
    merge_node = NULL;
    if (cnt_param == 0) {
        cnt_param = 1;
    }
    while (args != NULL) {
        DBUG_PRINT ("COMP", ("Handling argument #%d", cnt_param));

        arg = EXPRS_EXPR (args);

        /* choose the right tag for the argument and generate ICMs for RC */
        if (NODE_TYPE (arg) == N_id) {
            if (IS_REFCOUNTED (ID, arg)
                && FUN_DOES_REFCOUNT (AP_FUNDEF (arg_node), cnt_param)) {
                if (ID_ATTRIB (arg) == ST_inout) {
                    tag = "inout_rc";
                } else {
                    tag = "in_rc";
                }
            } else {
                /* argument is not refcounted or function does no refcounting */
                if (ID_ATTRIB (arg) == ST_inout) {
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

                icm_node
                  = MakeAdjustRcIcm (ID_NAME (arg), ID_TYPE (arg), ID_REFCNT (arg), -1);
                if (icm_node != NULL) {
                    last_node = ASSIGN_NEXT (last_node) = icm_node;
                }
            }
        } else {
            /* argument is not a N_id node */
            tag = "in";
        }

        if (!ids_for_dots) {
            last_entry = ret_entry = MakeExprs (MakeId_Copy (tag), NULL);

            last_entry = EXPRS_NEXT (last_entry) = DupNode (args);

            if (ARG_BASETYPE (fundef_args) == T_dots) {
                ids_for_dots = TRUE;
            } else {
                merge_node
                  = InsertApArgParam (icm_tab, ret_entry, ARG_TYPE (fundef_args),
                                      (NODE_TYPE (arg) == N_id) ? ID_REFCNT (arg) : -1,
                                      FUNDEF_PRAGMA (AP_FUNDEF (arg_node)) == NULL
                                        ? NULL
                                        : FUNDEF_LINKSIGN (AP_FUNDEF (arg_node)),
                                      cnt_param);

                fundef_args = ARG_NEXT (fundef_args);
            }
        } else {
            last_entry = EXPRS_NEXT (last_entry) = MakeExprs (MakeId_Copy (tag), NULL);

            last_entry = EXPRS_NEXT (last_entry) = DupNode (args);
        }

        args = EXPRS_NEXT (args);
        cnt_param++;

        if (ids_for_dots && (args == NULL)) {
            InsertApDotsParam (icm_tab, ret_entry);
        }
    }

    AdjustAddedAssigns (merge_node, ret_node);

    /* create ND_FUN_AP ICM */
    icm_node = CreateApIcm (FUNDEF_NAME (AP_FUNDEF (arg_node)), icm_tab, tab_size);
    /* insert ND_FUN_AP ICM at head of assignment chain */
    ASSIGN_NEXT (icm_node) = ASSIGN_NEXT (ret_node);
    ASSIGN_NEXT (ret_node) = icm_node;

    if (merge_node != NULL) {
        /* insert 'merge_node' at head of assignment chain */
        ASSIGN_NEXT (merge_node) = ASSIGN_NEXT (ret_node);
        ASSIGN_NEXT (ret_node) = merge_node;
    }

    /* free empty ICM tabular */
    FREE (icm_tab);
    /* free dummy node (head of chain) */
    ret_node = FreeNode (ret_node);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfDim( node *arg_node, node *arg_info)
 *
 * Description:
 *   Transforms N_prf node of type F_dim into ICM chain.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfDim (node *arg_node, node *arg_info)
{
    node *arg;

    DBUG_ENTER ("COMPPrfDim");

    arg = PRF_ARG1 (arg_node);
    DBUG_ASSERT ((NODE_TYPE (arg) == N_id), "N_id as arg of F_dim expected!");

    arg_node = MakeAssign (/* let_ids = dim( arg) */
                           MakeLet (MakeNum (GetDim (ID_TYPE (arg))),
                                    DupOneIds (INFO_COMP_LASTIDS (arg_info))),
                           Ids2DecRcICMs (ID_IDS (arg), NULL));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfShape( node *arg_node, node *arg_info)
 *
 * Description:
 *   Transforms N_prf node of type F_shape into ICM chain.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfShape (node *arg_node, node *arg_info)
{
    node *icm_node, *last_node;
    node *arg;
    ids *let_ids;

    DBUG_ENTER ("COMPPrfShape");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg) == N_id), "N_id as 1st arg of F_shape expected!");

    DBUG_ASSERT ((strcmp (IDS_NAME (let_ids), ID_NAME (arg))),
                 "a = shape( a) not allowed!");

    arg_node = MakeAssignIcm3 ("ND_ALLOC_ARRAY", /* result array */
                               MakeId_Copy ("int"), DupIds_Id (let_ids),
                               MakeNum (IDS_REFCNT (let_ids)));
    last_node = arg_node;

    icm_node
      = MakeAssignIcm3 ("ND_CREATE_CONST_ARRAY_S", DupIds_Id (let_ids),
                        MakeNum (GetDim (ID_TYPE (arg))), Type2Exprs (ID_TYPE (arg)));
    last_node = ASSIGN_NEXT (last_node) = icm_node;

    icm_node = MakeAdjustRcIcm (ID_NAME (arg), ID_TYPE (arg), ID_REFCNT (arg), -1);
    if (icm_node != NULL) {
        last_node = ASSIGN_NEXT (last_node) = icm_node;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfReshape( node *arg_node, node *arg_info)
 *
 * Description:
 *   Transforms N_prf node of type F_reshape into ICM chain.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfReshape (node *arg_node, node *arg_info)
{
    node *arg1, *arg2;
    ids *let_ids;

    DBUG_ENTER ("COMPPrfReshape");

    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    let_ids = INFO_COMP_LASTIDS (arg_info);

    DBUG_ASSERT ((NODE_TYPE (arg1) == N_id), "N_id as 1st arg of F_reshape expected!");
    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id), "N_id as 2nd arg of F_reshape expected!");

    DBUG_ASSERT ((strcmp (IDS_NAME (let_ids), ID_NAME (arg1))),
                 "a = reshape( a, .) not allowed!");

    /*
     * compile assignment
     * (v = reshape( shp, w)    =>   v = w)
     */
    arg_node = Trav (arg2, arg_info);

    arg_node = Ids2DecRcICMs (ID_IDS (arg1), arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfIdxPsi( node *arg_node, node *arg_info)
 *
 * Description:
 *   Transforms N_prf node of type F_idx_psi into ICM chain.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfIdxPsi (node *arg_node, node *arg_info)
{
    node *icm_node, *last_node;
    node *arg1, *arg2;
    ids *let_ids;

    DBUG_ENTER ("COMPPrfIdxPsi");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT (((NODE_TYPE (arg1) == N_id) || (NODE_TYPE (arg1) == N_num)),
                 "N_id or N_num as 1st arg of F_idx_psi expected!");
    DBUG_ASSERT (((NODE_TYPE (arg1) == N_num) || (!IS_REFCOUNTED (ID, arg1))),
                 "Non-refcounted object as 1st arg of F_idx_psi expected!");
    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id), "N_id as 2nd arg of F_idx_psi expected!");

    if (IsArray (IDS_TYPE (let_ids))) {
        /*
         * This is possible only in case of an instrinsic array-psi:
         *   psi( iv, A): int[] -> type[] -> type[]
         * But this is an non-purely intrinsic. Normally an array-psi
         * is transformed into a with-loop containing scalar-psi's only.
         */
        arg_node
          = MakeAssignIcm3 ("ND_ALLOC_ARRAY", MakeBasetypeNode (IDS_TYPE (let_ids)),
                            DupIds_Id (let_ids), MakeNum (IDS_REFCNT (let_ids)));
        last_node = arg_node;

        icm_node = MakeAssignIcm3 ("ND_IDX_PSI_A", DupNode (arg1), DupNode (arg2),
                                   DupIds_Id (let_ids));
        last_node = ASSIGN_NEXT (last_node) = icm_node;
    } else {
        arg_node = MakeAssignIcm3 ("ND_IDX_PSI_S", DupNode (arg1), DupNode (arg2),
                                   DupIds_Id (let_ids));
        last_node = arg_node;
    }

    icm_node = MakeAdjustRcIcm (ID_NAME (arg2), ID_TYPE (arg2), ID_REFCNT (arg2), -1);
    if (icm_node != NULL) {
        last_node = ASSIGN_NEXT (last_node) = icm_node;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfIdxModarray( node *arg_node, node *arg_info)
 *
 * Description:
 *   Transforms N_prf node of type F_idx_modarray into ICM chain.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfIdxModarray (node *arg_node, node *arg_info)
{
    node *icm_node, *last_node;
    node *arg1, *arg2, *arg3;
    char *icm_name;
    ids *let_ids;

    DBUG_ENTER ("COMPPrfIdxModarray");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    arg3 = PRF_ARG3 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg1) == N_id),
                 "N_id as 1st arg of F_idx_modarray expected!");
    /*
     * Because of IVE, at the 2nd argument position of F_idx_modarray might occur
     * an arithmetical expression (see function IdxArray)!!!
     */
    DBUG_ASSERT (((NODE_TYPE (arg2) == N_id) || (NODE_TYPE (arg2) == N_num)
                  || (NODE_TYPE (arg2) == N_prf)),
                 "N_id or N_num as 2nd arg of F_idx_modarray expected!");
    DBUG_ASSERT (((NODE_TYPE (arg2) != N_id) || (!IS_REFCOUNTED (ID, arg2))),
                 "Non-refcounted object as 2nd arg of F_idx_modarray expected!");
    DBUG_ASSERT ((NODE_TYPE (arg3) != N_array),
                 "No N_array as 3rd arg of F_idx_modarray expected!");

    DBUG_ASSERT ((strcmp (IDS_NAME (let_ids), ID_NAME (arg1))),
                 "a = idx_modarray( a, ., .) not allowed!");
    DBUG_ASSERT (((NODE_TYPE (arg3) != N_id)
                  || strcmp (IDS_NAME (let_ids), ID_NAME (arg3))),
                 "a = idx_modarray( ., ., a) not allowed!");

    DBUG_ASSERT ((ID_REFCNT (arg1) != 0), "Reference with (rc == 0) found!");

    if ((NODE_TYPE (arg3) == N_id) && IsArray (ID_TYPE (arg3))) {
        if (ID_REFCNT (arg1) == 1) {
            icm_name = "ND_IDX_MODARRAY_AxVxA_CHECK_REUSE";
        } else {
            icm_name = "ND_IDX_MODARRAY_AxVxA";
        }
    } else {
        if (ID_REFCNT (arg1) == 1) {
            icm_name = "ND_IDX_MODARRAY_AxVxS_CHECK_REUSE";
        } else {
            icm_name = "ND_IDX_MODARRAY_AxVxS";
        }
    }

    arg_node = MakeAssignIcm5 (icm_name, MakeBasetypeNode (IDS_TYPE (let_ids)),
                               DupIds_Id (let_ids), DupNode (arg1), DupNode (arg2),
                               DupNode (arg3));
    last_node = arg_node;

    icm_node = MakeAdjustRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                IDS_REFCNT (let_ids), IDS_REFCNT (let_ids));
    if (icm_node != NULL) {
        last_node = ASSIGN_NEXT (last_node) = icm_node;
    }

    icm_node = MakeAdjustRcIcm (ID_NAME (arg1), ID_TYPE (arg1), ID_REFCNT (arg1), -1);
    if (icm_node != NULL) {
        last_node = ASSIGN_NEXT (last_node) = icm_node;
    }

    if (NODE_TYPE (arg3) == N_id) {
        icm_node = MakeAdjustRcIcm (ID_NAME (arg3), ID_TYPE (arg3), ID_REFCNT (arg3), -1);
        if (icm_node != NULL) {
            last_node = ASSIGN_NEXT (last_node) = icm_node;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfPsi( node *arg_node, node *arg_info)
 *
 * Description:
 *   Transforms N_prf node of type F_psi into ICM chain.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfPsi (node *arg_node, node *arg_info)
{
    node *icm_node, *last_node;
    node *arg1, *arg2;
    ids *let_ids;

    DBUG_ENTER ("COMPPrfPsi");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT (((NODE_TYPE (arg1) == N_id) || (NODE_TYPE (arg1) == N_array)),
                 "N_id or N_array as 1st arg of F_psi expected!");
    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id), "N_id as 2nd arg of F_psi expected!");

    DBUG_ASSERT (((NODE_TYPE (arg1) != N_id)
                  || strcmp (IDS_NAME (let_ids), ID_NAME (arg1))),
                 "a = psi( a, .) not allowed!");
    DBUG_ASSERT ((strcmp (IDS_NAME (let_ids), ID_NAME (arg2))),
                 "a = psi( ., a) not allowed!");

    if (!IsArray (IDS_TYPE (let_ids))) {
        /* 'let_ids' is a scalar */

        if (NODE_TYPE (arg1) == N_id) {
            arg_node
              = MakeAssignIcm4 ("ND_KD_PSI_VxA_S", DupNode (arg2), DupIds_Id (let_ids),
                                MakeNum (GetTypesLength (ID_TYPE (arg1))),
                                DupNode (arg1));
            last_node = arg_node;
        } else {
            /* 'arg1' is a N_array node */
            arg_node
              = MakeAssignIcm4 ("ND_KD_PSI_CxA_S", DupNode (arg2), DupIds_Id (let_ids),
                                MakeNum (GetExprsLength (ARRAY_AELEMS (arg1))),
                                DupTree (ARRAY_AELEMS (arg1)));
            last_node = arg_node;
        }
    } else {
        /* 'let_ids' is an array */

        arg_node
          = MakeAssignIcm3 ("ND_ALLOC_ARRAY", MakeBasetypeNode (IDS_TYPE (let_ids)),
                            DupIds_Id (let_ids), MakeNum (IDS_REFCNT (let_ids)));
        last_node = arg_node;

        if (NODE_TYPE (arg1) == N_id) {
            icm_node
              = MakeAssignIcm5 ("ND_KD_PSI_VxA_A", MakeNum (GetDim (ID_TYPE (arg2))),
                                DupNode (arg2), DupIds_Id (let_ids),
                                MakeNum (GetTypesLength (ID_TYPE (arg1))),
                                DupNode (arg1));
            last_node = ASSIGN_NEXT (last_node) = icm_node;
        } else {
            /* 'arg1' is a N_array node */
            icm_node
              = MakeAssignIcm5 ("ND_KD_PSI_CxA_A", MakeNum (GetDim (ID_TYPE (arg2))),
                                DupNode (arg2), DupIds_Id (let_ids),
                                MakeNum (GetExprsLength (ARRAY_AELEMS (arg1))),
                                DupTree (ARRAY_AELEMS (arg1)));
            last_node = ASSIGN_NEXT (last_node) = icm_node;
        }
    }

    if (NODE_TYPE (arg1) == N_id) {
        icm_node = MakeAdjustRcIcm (ID_NAME (arg1), ID_TYPE (arg1), ID_REFCNT (arg1), -1);
        if (icm_node != NULL) {
            last_node = ASSIGN_NEXT (last_node) = icm_node;
        }
    }

    icm_node = MakeAdjustRcIcm (ID_NAME (arg2), ID_TYPE (arg2), ID_REFCNT (arg2), -1);
    if (icm_node != NULL) {
        last_node = ASSIGN_NEXT (last_node) = icm_node;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfModarray( node *arg_node, node *arg_info)
 *
 * Description:
 *   Transforms N_prf node of type F_modarray into ICM chain.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfModarray (node *arg_node, node *arg_info)
{
    node *icm_node, *last_node;
    node *arg1, *arg2, *arg3;
    ids *let_ids;
    node *res_dim, *res_type;
    char *icm_name;

    DBUG_ENTER ("COMPPrfModarray");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    arg3 = PRF_ARG3 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg1) == N_id), "N_id as 1st arg of F_modarray expected!");
    DBUG_ASSERT (((NODE_TYPE (arg2) == N_id) || (NODE_TYPE (arg2) == N_array)),
                 "N_id or N_array as 2nd arg of F_modarray expected!");
    DBUG_ASSERT ((NODE_TYPE (arg3) != N_array),
                 "No N_array as 3rd arg of F_modarray expected!");

    DBUG_ASSERT ((strcmp (IDS_NAME (let_ids), ID_NAME (arg1))),
                 "a = modarray( a, ., .) not allowed!");
    DBUG_ASSERT (((NODE_TYPE (arg2) != N_id)
                  || strcmp (IDS_NAME (let_ids), ID_NAME (arg2))),
                 "a = modarray( ., a, .) not allowed!");
    DBUG_ASSERT (((NODE_TYPE (arg3) != N_id)
                  || strcmp (IDS_NAME (let_ids), ID_NAME (arg3))),
                 "a = modarray( ., ., a) not allowed!");

    DBUG_ASSERT ((ID_REFCNT (arg1) != 0), "Reference with (rc == 0) found!");

    /* basic type of result */
    res_type = MakeBasetypeNode (IDS_TYPE (let_ids));
    /* dimension of result */
    res_dim = MakeNum (GetDim (IDS_TYPE (let_ids)));

    if (NODE_TYPE (arg2) == N_array) {
        /* index is constant! */

        if ((N_id == NODE_TYPE (arg3)) && IsArray (ID_TYPE (arg3))) {
            if (ID_REFCNT (arg1) == 1) {
                icm_name = "ND_PRF_MODARRAY_AxCxA_CHECK_REUSE";
            } else {
                icm_name = "ND_PRF_MODARRAY_AxCxA";
            }
        } else {
            /* 'arg3' is a scalar */

            if (ID_REFCNT (arg1) == 1) {
                icm_name = "ND_PRF_MODARRAY_AxCxS_CHECK_REUSE";
            } else {
                icm_name = "ND_PRF_MODARRAY_AxCxS";
            }
        }

        arg_node = MakeAssignIcm7 (icm_name, res_type, res_dim, DupIds_Id (let_ids),
                                   DupNode (arg1), DupNode (arg3),
                                   MakeNum (GetExprsLength (ARRAY_AELEMS (arg2))),
                                   DupTree (ARRAY_AELEMS (arg2)));
        last_node = arg_node;
    } else {
        /* 'arg2' is a N_id node */
        DBUG_ASSERT (((GetDim (ID_TYPE (arg2)) == 1)
                      && (GetBasetype (ID_TYPE (arg2)) == T_int)),
                     "2nd arg of F_modarray is a illegal indexing var!");

        if ((N_id == NODE_TYPE (arg3)) && IsArray (ID_TYPE (arg3))) {
            if (ID_REFCNT (arg1) == 1) {
                icm_name = "ND_PRF_MODARRAY_AxVxA_CHECK_REUSE";
            } else {
                icm_name = "ND_PRF_MODARRAY_AxVxA";
            }
        } else {
            /* 'arg3' is a scalar */

            if (ID_REFCNT (arg1) == 1) {
                icm_name = "ND_PRF_MODARRAY_AxVxS_CHECK_REUSE";
            } else {
                icm_name = "ND_PRF_MODARRAY_AxVxS";
            }
        }

        arg_node
          = MakeAssignIcm7 (icm_name, res_type, res_dim, DupIds_Id (let_ids),
                            DupNode (arg1), DupNode (arg3),
                            MakeNum (TYPES_SHAPE (ID_TYPE (arg2), 0)), DupNode (arg2));
        last_node = arg_node;
    }

    icm_node = MakeAdjustRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                IDS_REFCNT (let_ids), IDS_REFCNT (let_ids));
    if (icm_node != NULL) {
        last_node = ASSIGN_NEXT (last_node) = icm_node;
    }

    icm_node = MakeAdjustRcIcm (ID_NAME (arg1), ID_TYPE (arg1), ID_REFCNT (arg1), -1);
    if (icm_node != NULL) {
        last_node = ASSIGN_NEXT (last_node) = icm_node;
    }

    if (NODE_TYPE (arg2) == N_id) {
        icm_node = MakeAdjustRcIcm (ID_NAME (arg2), ID_TYPE (arg2), ID_REFCNT (arg2), -1);
        if (icm_node != NULL) {
            last_node = ASSIGN_NEXT (last_node) = icm_node;
        }
    }

    if (NODE_TYPE (arg3) == N_id) {
        icm_node = MakeAdjustRcIcm (ID_NAME (arg3), ID_TYPE (arg3), ID_REFCNT (arg3), -1);
        if (icm_node != NULL) {
            last_node = ASSIGN_NEXT (last_node) = icm_node;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfIcm1( char *icm_name, node *arg_node, node *arg_info)
 *
 * Description:
 *   Transforms a N_prf node into an unary ICM.
 *
 ******************************************************************************/

static node *
COMPPrfIcm1 (char *icm_name, node *arg_node, node *arg_info)
{
    node *tmp;

    DBUG_ENTER ("COMPPrfIcm1");

    tmp = arg_node;
    arg_node = MakeIcm1 (icm_name, DupNode (PRF_ARG1 (arg_node)));
    tmp = FreeNode (tmp);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfIcm2( char *icm_name, node *arg_node, node *arg_info)
 *
 * Description:
 *   Transforms a N_prf node into a binary ICM.
 *
 ******************************************************************************/

static node *
COMPPrfIcm2 (char *icm_name, node *arg_node, node *arg_info)
{
    node *tmp;

    DBUG_ENTER ("COMPPrfIcm2");

    tmp = arg_node;
    arg_node
      = MakeIcm2 (icm_name, DupNode (PRF_ARG1 (arg_node)), DupNode (PRF_ARG2 (arg_node)));
    tmp = FreeNode (tmp);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfConvertScalar( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles N_prf node of type F_toi, F_tod, F_tof:
 *   We can simply remove the conversion function :-)
 *   Therefore the return value is the first argument of the given N_prf node.
 *
 ******************************************************************************/

static node *
COMPPrfConvertScalar (node *arg_node, node *arg_info)
{
    node *tmp;

    DBUG_ENTER ("COMPPrfConvertScalar");

    /* return argument, free the remaining parts */
    tmp = arg_node;
    arg_node = DupNode (PRF_ARG1 (arg_node));
    tmp = FreeNode (tmp);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfConvertArr( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles N_prf node of type F_toi_A, F_tof_A, F_tod_A.
 *   The return value is a N_assign chain of ICMs.
 *
 * Remark:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 * *** CODE NOT BRUSHED YET ***
 *
 ******************************************************************************/

static node *
COMPPrfConvertArr (node *arg_node, node *arg_info)
{
    int length;
    node *res_rc, *n_length, *first_assign, *next_assign, *res_type, *old_arg_node,
      *last_assign, *res, *icm_arg, *arg1;
    ids *let_ids;

    int convert = 0;

    DBUG_ENTER ("COMPPrfConvertArr");

    switch (PRF_PRF (arg_node)) {
    case F_tof_A:
        convert = 1;
        /* here is NO break missing!! */
    case F_tod_A:
        convert = 2;
        /* here is NO break missing!! */
    case F_toi_A:
        let_ids = INFO_COMP_LASTIDS (arg_info);
        arg1 = PRF_ARG1 (arg_node);
        res = DupIds_Id (let_ids);
        /* compute basic type */
        res_type = MakeBasetypeNode (IDS_TYPE (let_ids));
        res_rc = MakeNum (IDS_REFCNT (let_ids));
        BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY", res_type, res);
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
            length = GetExprsLength (ARRAY_AELEMS (arg1));
            if (1 < TYPES_DIM (ARRAY_TYPE (arg1))) {
                node *dummy;
                /*
                 *  it is an array of arrays, so we have to use
                 *  ND_CREATE_CONST_ARRAY_A
                 */
                DBUG_ASSERT ((N_id == NODE_TYPE (arg1->node[0]->node[0])),
                             "wrong node != N_id");
                length = GetTypesLength (ID_TYPE (arg1->node[0]->node[0]));
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

    default:
        DBUG_ASSERT ((0), "Illegal conversion function (prf) found!");
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrf( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compilation of a N_prf node.
 *   The return value is a N_assign chain of ICMs, a single N_icm node, or
 *   the unchanged N_prf node.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 * *** CODE NOT FULLY BRUSHED YET ***
 *
 ******************************************************************************/

node *
COMPPrf (node *arg_node, node *arg_info)
{
    node *ret_node = NULL;

    DBUG_ENTER ("COMPPrf");

    DBUG_PRINT ("COMP",
                ("%s line: %d", mdb_prf[PRF_PRF (arg_node)], NODE_LINE (arg_node)));

    if (SCALAR_ARGS (PRF_PRF (arg_node))) {
        switch (PRF_PRF (arg_node)) {
        case F_toi:
        case F_tof:
        case F_tod:
            ret_node = COMPPrfConvertScalar (arg_node, arg_info);
            break;

        case F_abs:
            ret_node = COMPPrfIcm1 ("ND_ABS", arg_node, arg_info);
            break;

        case F_min:
            ret_node = COMPPrfIcm2 ("ND_MIN", arg_node, arg_info);
            break;

        case F_max:
            ret_node = COMPPrfIcm2 ("ND_MAX", arg_node, arg_info);
            break;

        case F_not:
        case F_add:
        case F_sub:
        case F_mul:
        case F_div:
        case F_mod:
        case F_and:
        case F_or:
        case F_le:
        case F_lt:
        case F_eq:
        case F_neq:
        case F_ge:
        case F_gt:
            ret_node = arg_node;
            break;

        default:
            DBUG_ASSERT ((0), "illegal prf found!");
            break;
        }
    } else if (ARRAY_ARGS_INTRINSIC (PRF_PRF (arg_node))) {
        switch (PRF_PRF (arg_node)) {
        case F_genarray:
            DBUG_ASSERT ((0), "F_genarray not yet implemented!");
            break;

        case F_dim:
            ret_node = COMPPrfDim (arg_node, arg_info);
            break;

        case F_shape:
            ret_node = COMPPrfShape (arg_node, arg_info);
            break;

        case F_reshape:
            ret_node = COMPPrfReshape (arg_node, arg_info);
            break;

        case F_idx_psi:
            ret_node = COMPPrfIdxPsi (arg_node, arg_info);
            break;

        case F_idx_modarray:
            ret_node = COMPPrfIdxModarray (arg_node, arg_info);
            break;

        case F_psi:
            ret_node = COMPPrfPsi (arg_node, arg_info);
            break;

        case F_modarray:
            ret_node = COMPPrfModarray (arg_node, arg_info);
            break;

        default:
            DBUG_ASSERT ((0), "illegal prf found!");
            break;
        }
    } else { /* Here, we have ARRAY_ARGS_NON_INTRINSIC( PRF_PRF( arg_node)) */
        /*
         *
         * THE FOLLOWING CODE IS MORE OR LESS REDUNDANT AND *REALLY* UGLY!!!
         *
         */
        node *array, *scalar, *res, *res_ref, *icm_arg;
        node *prf_id_node, *res_type, *arg1, *arg2, *arg3;
        node *dim_node;
        node *old_arg_node;
        node *tmp_array1, *tmp_array2;
        int dim;

        node *first_assign = NULL;
        node *next_assign = NULL;
        node *last_assign = NULL;
        ids *let_ids = INFO_COMP_LASTIDS (arg_info);
        simpletype res_stype = LET_BASETYPE (INFO_COMP_LASTLET (arg_info));
        int is_drop = 0;
        int array_is_const = 0;
        int is_SxA = 0;

        DBUG_ASSERT ((ARRAY_ARGS_NON_INTRINSIC (PRF_PRF (arg_node))),
                     "illegal prf found!");

#ifndef DBUG_OFF
        /*
         * Assure that no one of the array-arguments occurs on the LHS of the
         * application.
         */
        if ((PRF_PRF (arg_node) != F_dim) && (PRF_PRF (arg_node) != F_idx_psi)) {
            node *args, *arg_id;
            args = PRF_ARGS (arg_node);
            while (args != NULL) {
                arg_id = EXPRS_EXPR (args);
                if ((NODE_TYPE (arg_id) == N_id) && IS_REFCOUNTED (ID, arg_id)) {
                    if (!strcmp (IDS_NAME (let_ids), ID_NAME (arg_id))) {
                        DBUG_ASSERT ((0), "refcounted argument of application occurs "
                                          "also on LHS!");
                    }
                }
                args = EXPRS_NEXT (args);
            }
        }
#endif

        /*
         * Select primitive funcction:
         */
        switch (PRF_PRF (arg_node)) {
        case F_drop:
            is_drop = 1;
            /* here is NO break missing */
        case F_take: {
            node *num;
            int n_elems;

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

            res = DupIds_Id (let_ids);

            /* basic type of result */
            res_type = MakeId_Copy (type_string[GetBasetype (IDS_TYPE (let_ids))]);
            /*
             * store refcount of res as N_num
             */
            res_ref = MakeNum (IDS_REFCNT (let_ids));

            num = MakeNum (0);
            if (NODE_TYPE (arg2) == N_id) {
                dim = GetDim (ID_TYPE (arg2));
                /*
                 *  store dimension of argument-array
                 */
                dim_node = MakeNum (dim);
                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY", res_type,
                               res);
                MAKE_NEXT_ICM_ARG (icm_arg, num);
                SET_VARS_FOR_MORE_ICMS;
            } else {
                DBUG_ASSERT ((NODE_TYPE (arg2) == N_array), "wrong nodetype");
                /*
                 *  store dimension of argument-array
                 */
                dim_node = MakeNum (1);
                CREATE_TMP_CONST_ARRAY (arg2, res_ref)
                CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", res_type, res, num);
                APPEND_ASSIGNS (first_assign, next_assign);
            }

            if (NODE_TYPE (arg1) == N_array) {
                n_elems = ARRAY_VECLEN (arg1);
            } else {
                if (NODE_TYPE (arg1) == N_id) {
                    n_elems = ID_VECLEN (arg1);
                } else {
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
            } else {
                int i;
                for (i = 0; i < n_elems; i++) {
                    num = MakeNum (((int *)ID_CONSTVEC (arg1))[i]);
                    MAKE_NEXT_ICM_ARG (icm_arg, num);
                }
            }

            APPEND_ASSIGNS (first_assign, next_assign);

            if (array_is_const == 0) {
                if (NODE_TYPE (arg2) == N_id) {
                    DEC_OR_FREE_RC_ND (arg2, MakeNum (1));
                }
                CREATE_2_ARY_ICM (next_assign, "ND_INC_RC", res, res_ref);
                APPEND_ASSIGNS (first_assign, next_assign);
                INSERT_ASSIGN;
            }
            FREE (old_arg_node);
            break;
        }

        case F_cat:
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            arg3 = arg_node->node[0]->node[1]->node[1]->node[0];
            res = DupIds_Id (let_ids);
            /* compute basic_type of result */
            res_type = MakeId_Copy (type_string[GetBasetype (IDS_TYPE (let_ids))]);
            /* store refcount of res as N_num */
            res_ref = MakeNum (IDS_REFCNT (let_ids));

            if ((N_id == NODE_TYPE (arg2)) && (N_id == NODE_TYPE (arg3))) {
                dim = GetDim (ID_TYPE (arg2));
                dim_node = MakeNum (dim);
                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY", res_type,
                               res);
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
                if ((N_array == NODE_TYPE (arg2)) && (N_array == NODE_TYPE (arg3))) {
                    array_is_const = 3;
                    dim = GetDim (ARRAY_TYPE (arg2));
                    DECL_ARRAY (first_assign, arg2->node[0], "__TMP1", tmp_array1);
                    arg_node = first_assign; /* set new arg_node */
                    DECL_ARRAY (next_assign, arg3->node[0], "__TMP2", tmp_array2);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    CREATE_CONST_ARRAY (arg2, tmp_array1, res_type, MakeNum (0));
                    CREATE_CONST_ARRAY (arg3, tmp_array2, res_type, MakeNum (0));
                    arg2 = tmp_array1;
                    arg3 = tmp_array2;
                } else if (N_array == NODE_TYPE (arg2)) {
                    array_is_const = 1;
                    dim = GetDim (ARRAY_TYPE (arg2));
                    DECL_ARRAY (first_assign, arg2->node[0], "__TMP1", tmp_array1);
                    arg_node = first_assign;
                    CREATE_CONST_ARRAY (arg2, tmp_array1, res_type, MakeNum (0));
                    /* set arg2 for later use as parameters of ND_KD_CAT*/
                    arg2 = tmp_array1;
                } else {
                    array_is_const = 2;
                    dim = GetDim (ARRAY_TYPE (arg3));
                    DECL_ARRAY (first_assign, arg3->node[0], "__TMP2", tmp_array2);
                    arg_node = first_assign;
                    CREATE_CONST_ARRAY (arg3, tmp_array2, res_type, MakeNum (0));
                    /* set arg3 for later use as parameters of ND_KD_CAT*/
                    arg3 = tmp_array2;
                }
                CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", res_type, res, res_ref);
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

        case F_rotate:
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            arg3 = arg_node->node[0]->node[1]->node[1]->node[0];
            res = DupIds_Id (let_ids);
            /* compute basic_type of result */
            res_type = MakeId_Copy (type_string[GetBasetype (IDS_TYPE (let_ids))]);
            /* store refcount of res as N_num */
            res_ref = MakeNum (IDS_REFCNT (let_ids));

            if (N_id == NODE_TYPE (arg3)) {
                dim = GetDim (ID_TYPE (arg3)); /* dim will be used later */
                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY", res_type,
                               res);
                MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
                SET_VARS_FOR_MORE_ICMS;
            } else {
                array_is_const = 1;
                /* reuse previous N_let */
                NODE_TYPE (INFO_COMP_LASTLET (arg_info)) = N_block;
                old_arg_node = arg_node;
                dim = GetDim (ARRAY_TYPE (arg3)); /* dim will be used later */
                DECL_ARRAY (first_assign, arg3->node[0], "__TMP1", tmp_array1);
                arg_node = first_assign;
                CREATE_CONST_ARRAY (arg3, tmp_array1, res_type, MakeNum (1));
                arg3 = tmp_array1;
                CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", res_type, res, res_ref);
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
        case F_tof_A:
        case F_tod_A:
            arg_node = COMPPrfConvertArr (arg_node, arg_info);
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
            int n_elems;

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
            res = DupIds_Id (INFO_COMP_LASTIDS (arg_info));

            /* store prf as N_id */
            prf_id_node = MakeId_Copy (prf_string[arg_node->info.prf]);

            /* compute basic_type of result */
            res_type = MakeId_Copy (
              type_string[GetBasetype (IDS_TYPE (INFO_COMP_LASTIDS (arg_info)))]);

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
                n_elems = GetExprsLength (ARRAY_AELEMS (array));

                tmp_array1 = MakeId_Copy ("__TMP");

                /* reuse previous N_let */
                NODE_TYPE (INFO_COMP_LASTLET (arg_info)) = N_block;
                CREATE_4_ARY_ICM (first_assign, "ND_KS_DECL_ARRAY", res_type, tmp_array1,
                                  MakeNum (1), MakeNum (n_elems));
                arg_node = first_assign;

                /* create const array */
                CREATE_CONST_ARRAY (array, tmp_array1, res_type, res_ref);

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
            res = DupIds_Id (INFO_COMP_LASTIDS (arg_info));

            /* store prf as N_id */
            prf_id_node = MakeId_Copy (prf_string[arg_node->info.prf]);

            /* compute basic_type of result */
            res_type = MakeId_Copy (
              type_string[GetBasetype (IDS_TYPE (INFO_COMP_LASTIDS (arg_info)))]);

            /* compute basic_type of arg1 and arg2 */
            GET_BASIC_SIMPLETYPE_OF_NODE (arg1_stype, arg1);
            GET_BASIC_SIMPLETYPE_OF_NODE (arg2_stype, arg2);

            /* store refcount of res as N_num */
            res_ref = MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

            if ((N_id == NODE_TYPE (arg1)) && (N_id == NODE_TYPE (arg2))) {
                last_assign = NEXT_ASSIGN (arg_info);
                if ((1 == ID_REFCNT (arg1)) && (1 == ID_REFCNT (arg2))) {
                    node *num;

                    BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_CHECK_REUSE_ARRAY",
                                   arg1, res);
                    SET_VARS_FOR_MORE_ICMS;

                    CREATE_2_ARY_ICM (next_assign, "ND_CHECK_REUSE_ARRAY", arg2, res);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    num = MakeNum (0);
                    CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", res_type, res, num);
                    APPEND_ASSIGNS (first_assign, next_assign);
                } else if (1 == ID_REFCNT (arg1)) {
                    CHECK_REUSE__ALLOC_ARRAY_ND (res, res_stype, arg1, arg1_stype);
                } else if (1 == ID_REFCNT (arg2)) {
                    CHECK_REUSE__ALLOC_ARRAY_ND (res, res_stype, arg2, arg2_stype);
                } else {
                    node *num;

                    num = MakeNum (0);
                    TRI_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY",
                                   res_type, res, num);
                    first_assign = CURR_ASSIGN (arg_info);
                    old_arg_node = arg_node;
                    last_assign = NEXT_ASSIGN (arg_info);
                    arg_node = LET_EXPR (INFO_COMP_LASTLET (arg_info));

                    DBUG_ASSERT ((((-1 == ID_REFCNT (arg1)) && (-1 == ID_REFCNT (arg2)))
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
                    CREATE_CONST_ARRAY (arg1, tmp_array1, res_type, res_ref);
                    CREATE_CONST_ARRAY (arg2, tmp_array2, res_type, MakeNum (0));
                    CREATE_2_ARY_ICM (next_assign, "ND_KS_ASSIGN_ARRAY", tmp_array1, res);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    /* set arg1 and arg2 for later use as parameters of BIN_OP */
                    arg1 = tmp_array1;
                    arg2 = tmp_array2;
                } else if (N_array == NODE_TYPE (arg1)) {
                    array_is_const = 1;
                    DECL_ARRAY (first_assign, arg1->node[0], "__TMP1", tmp_array1);
                    arg_node = first_assign;
                    CREATE_CONST_ARRAY (arg1, tmp_array1, res_type, res_ref);
                    CREATE_2_ARY_ICM (next_assign, "ND_KS_ASSIGN_ARRAY", tmp_array1, res);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    /* set arg1 for later use as parameters of BIN_OP */
                    arg1 = tmp_array1;
                } else {
                    array_is_const = 2;
                    DECL_ARRAY (first_assign, arg2->node[0], "__TMP2", tmp_array2);
                    arg_node = first_assign;
                    CREATE_CONST_ARRAY (arg2, tmp_array2, res_type, res_ref);
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

        default:
            DBUG_ASSERT (0, "illegal prf found!");
            break;
        }

        ret_node = arg_node;
    }

    DBUG_ASSERT ((ret_node != NULL), "no return value found!");

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPIdLet( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles let expression with id on RHS.
 *   The return value is a (possibly empty) N_assign chain of ICMs.
 *
 ******************************************************************************/

static node *
COMPIdLet (node *arg_node, node *arg_info)
{
    ids *let_ids;
    node *icm_node;
    node *ret_node = NULL;

    DBUG_ENTER ("COMPIdLet");

    let_ids = LET_IDS (INFO_COMP_LASTLET (arg_info));

    /*
     * 'arg_node' and 'res' are both non-unique or both unique
     */
    if (!strcmp (ID_NAME (arg_node), IDS_NAME (let_ids))) {
        /*
         * We are dealing with an assignment of the kind:
         *   a = a;
         * which we compile into:
         *   NOOP()
         */
        ret_node = MakeAssignIcm0 ("NOOP");
    } else {
        if (IsArray (ID_TYPE (arg_node))) {
            ret_node = MakeAssignIcm2 ("ND_KS_ASSIGN_ARRAY", DupNode (arg_node),
                                       DupIds_Id (let_ids));
        } else if (IsNonUniqueHidden (ID_TYPE (arg_node))) {
            ret_node = MakeAssignIcm2 ("ND_ASSIGN_HIDDEN", DupNode (arg_node),
                                       DupIds_Id (let_ids));
        }
    }

    icm_node = MakeAdjustRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                IDS_REFCNT (let_ids), IDS_REFCNT (let_ids) - 1);
    if (icm_node != NULL) {
        DBUG_ASSERT ((ret_node != NULL), "ICMs missing!");
        ASSIGN_NEXT (ret_node) = icm_node;
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPIdFromClass( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles let expression with a N_id node representing an application of
 *   the from_class() conversion function on RHS.
 *   The return value is a (possibly empty) N_assign chain of ICMs.
 *
 ******************************************************************************/

static node *
COMPIdFromClass (node *arg_node, node *arg_info)
{
    ids *let_ids;

    node *icm_node, *last_node;
    node *ret_node = NULL;

    DBUG_ENTER ("COMPIdFromClass");

    let_ids = LET_IDS (INFO_COMP_LASTLET (arg_info));

    /*
     * 'arg_node' is unique and 'let_ids' is non-unique
     */
    if (IsArray (ID_TYPE (arg_node))) {
        ret_node = MakeAssignIcm2 ("ND_KS_ASSIGN_ARRAY", DupNode (arg_node),
                                   DupIds_Id (let_ids));
        last_node = ret_node;

        icm_node = MakeAdjustRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                    IDS_REFCNT (let_ids), IDS_REFCNT (let_ids) - 1);
        if (icm_node != NULL) {
            last_node = ASSIGN_NEXT (last_node) = icm_node;
        }
    } else if (IsHidden (ID_TYPE (arg_node))) {
        ret_node = MakeAssignIcm2 ("ND_NO_RC_ASSIGN_HIDDEN", DupNode (arg_node),
                                   DupIds_Id (let_ids));
        last_node = ret_node;

        icm_node = MakeAssignIcm1 ("ND_ALLOC_RC", DupIds_Id (let_ids));
        last_node = ASSIGN_NEXT (last_node) = icm_node;

        icm_node = MakeAssignIcm2 ("ND_SET_RC", DupIds_Id (let_ids),
                                   MakeNum (IDS_REFCNT (let_ids)));
        last_node = ASSIGN_NEXT (last_node) = icm_node;
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPIdToClass( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles let expression with a N_id node representing an application of
 *   the to_class() conversion function on RHS.
 *   The return value is a (possibly empty) N_assign chain of ICMs.
 *
 ******************************************************************************/

static node *
COMPIdToClass (node *arg_node, node *arg_info)
{
    ids *let_ids;
    node *icm_node, *last_node;
    node *ret_node = NULL;

    DBUG_ENTER ("COMPIdToClass");

    let_ids = LET_IDS (INFO_COMP_LASTLET (arg_info));

    /*
     * 'arg_node' is non-unique and 'let_ids' is unique
     */
    if (IS_REFCOUNTED (ID, arg_node)) {
        DBUG_ASSERT ((ID_REFCNT (arg_node) != 0), "Reference with (rc == 0) found!");

        if (ID_REFCNT (arg_node) == 1) {
            if (IsArray (ID_TYPE (arg_node))) {
                ret_node
                  = MakeAssignIcm3 ("ND_KS_MAKE_UNIQUE_ARRAY", DupNode (arg_node),
                                    DupIds_Id (let_ids),
                                    MakeNum (GetBasetypeSize (ID_TYPE (arg_node))));
                last_node = ret_node;

                icm_node = MakeAssignIcm2 ("ND_SET_RC", DupIds_Id (let_ids),
                                           MakeNum (IDS_REFCNT (let_ids)));
                last_node = ASSIGN_NEXT (last_node) = icm_node;
            } else {
                ret_node
                  = MakeAssignIcm3 ("ND_NO_RC_MAKE_UNIQUE_HIDDEN", DupNode (arg_node),
                                    DupIds_Id (let_ids),
                                    MakeId_Copy (GenericFun (0, ID_TYPE (arg_node))));
                last_node = ret_node;
            }
        } else {
            if (IsArray (ID_TYPE (arg_node))) {
                ret_node
                  = MakeAssignIcm3 ("ND_KS_COPY_ARRAY", DupNode (arg_node),
                                    DupIds_Id (let_ids),
                                    MakeNum (GetBasetypeSize (ID_TYPE (arg_node))));
                last_node = ret_node;

                icm_node = MakeAssignIcm1 ("ND_ALLOC_RC", DupIds_Id (let_ids));
                last_node = ASSIGN_NEXT (last_node) = icm_node;

                icm_node = MakeAssignIcm2 ("ND_SET_RC", DupIds_Id (let_ids),
                                           MakeNum (IDS_REFCNT (let_ids)));
                last_node = ASSIGN_NEXT (last_node) = icm_node;

                icm_node = MakeAssignIcm2 ("ND_DEC_RC", DupNode (arg_node), MakeNum (1));
                last_node = ASSIGN_NEXT (last_node) = icm_node;
            } else {
                ret_node
                  = MakeAssignIcm3 ("ND_COPY_HIDDEN", DupNode (arg_node),
                                    DupIds_Id (let_ids),
                                    MakeId_Copy (GenericFun (0, ID_TYPE (arg_node))));
                last_node = ret_node;

                icm_node = MakeAssignIcm2 ("ND_DEC_RC", DupNode (arg_node), MakeNum (1));
                last_node = ASSIGN_NEXT (last_node) = icm_node;
            }
        }
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPId( node *arg_node, node *arg_info)
 *
 * Remarks:
 *   Compiles let expression with a N_id node (which possibly represents an
 *   application of a class conversion function!) on RHS.
 *   The return value is a N_assign chain of ICMs or the unchanged N_id node.
 *
 ******************************************************************************/

node *
COMPId (node *arg_node, node *arg_info)
{
    node *ret_node;

    DBUG_ENTER ("COMPId");

    switch (ID_CLSCONV (arg_node)) {
    case NO_CLSCONV:
        ret_node = COMPIdLet (arg_node, arg_info);
        break;

    case FROM_CLASS:
        ret_node = COMPIdFromClass (arg_node, arg_info);
        break;

    case TO_CLASS:
        ret_node = COMPIdToClass (arg_node, arg_info);
        break;

    default:
        DBUG_ASSERT (0, "unknown type of class conversion function found");
        break;
    }

    /*
     * If (ret_node == NULL) is hold, the expression remains unchanged.
     * Otherwise return the ICM chain stored in 'ret_node'.
     */
    if (ret_node != NULL) {
        arg_node = ret_node;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPArray(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 * *** CODE NOT BRUSHED YET ***
 *
 ******************************************************************************/

node *
COMPArray (node *arg_node, node *arg_info)
{
    node *first_assign, *next_assign, *res, *res_type, *old_arg_node, *icm_arg, *res_ref,
      *last_assign;
    int n_elems;
    int icm_created = 0;

    DBUG_ENTER ("COMPArray");

    /* store next assign */
    last_assign = NEXT_ASSIGN (arg_info);

    /* compute basic_type of result */
    res_type = MakeBasetypeNode (IDS_TYPE (INFO_COMP_LASTIDS (arg_info)));

    /* store result as N_id */
    res = DupIds_Id (INFO_COMP_LASTIDS (arg_info));

    /* store refcount of res as N_num */
    res_ref = MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

    /* create ND_ALLOC_ARRAY */
    BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY", res_type, res);
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
    n_elems = GetExprsLength (ARRAY_AELEMS (old_arg_node));

    if (ARRAY_AELEMS (old_arg_node) != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (ARRAY_AELEMS (old_arg_node))) == N_id) {
            if (IsArray (ID_TYPE (EXPRS_EXPR (ARRAY_AELEMS (old_arg_node))))) {

                /*  #### */

                node *length;
                int len;
                len = GetTypesLength (ID_TYPE (old_arg_node->node[0]->node[0]));
                length = MakeNum (len);
                CREATE_3_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_A", res, length,
                                  MakeNum (n_elems));
                icm_created = 1;
            } else {
                if (IsNonUniqueHidden (ID_TYPE (old_arg_node->node[0]->node[0]))) {
                    CREATE_3_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_H", res,
                                      MakeId_Copy (
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
 * *** CODE NOT BRUSHED YET ***
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

    DBUG_ASSERT (((NODE_TYPE (DO_OR_WHILE_COND (arg_node)) == N_id)
                  || (NODE_TYPE (DO_OR_WHILE_COND (arg_node)) == N_bool)),
                 "loop condition is neither a N_id nor a N_bool node!");

    /*
     * DO_OR_WHILE_COND(arg_node) must not be traversed!
     */

    /* first compile termination condition and body of loop */
    loop_assign = CURR_ASSIGN (arg_info);
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
                                      DupIds_Id (defvar), MakeNum (1),
                                      MakeId_Copy (GenericFun (1, IDS_TYPE (defvar))));
                } else {
                    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE_ARRAY",
                                      DupIds_Id (defvar), MakeNum (1));
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
        label = MakeId_Copy (TmpVarName (LABEL_NAME));
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
            CREATE_2_ARY_ICM (next_assign, "ND_INC_RC", DupIds_Id (usevar),
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
                            DupIds_Id( usevar),
                            MakeNum( 1));
          APPEND_ASSIGNS( first_assign, next_assign);
        }
        else if (IDS_REFCNT( usevar) > 0) {
          CREATE_2_ARY_ICM( next_assign, "ND_DEC_RC_FREE_ARRAY",
                            DupIds_Id( usevar),
                            MakeNum( 1));
          APPEND_ASSIGNS( first_assign, next_assign);
        }
#else
                if (IDS_REFCNT (usevar) > 0) {
                    if (IsNonUniqueHidden (IDS_TYPE (usevar))) {
                        CREATE_3_ARY_ICM (next_assign, "ND_DEC_RC_FREE_HIDDEN",
                                          DupIds_Id (usevar), MakeNum (1),
                                          MakeId_Copy (
                                            GenericFun (1, IDS_TYPE (usevar))));
                    } else {
                        CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE_ARRAY",
                                          DupIds_Id (usevar), MakeNum (1));
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
            CREATE_2_ARY_ICM (next_assign, "ND_INC_RC", DupIds_Id (defvar),
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
 * *** CODE NOT BRUSHED YET ***
 *
 ******************************************************************************/

node *
COMPCond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("COMPCond");

    DBUG_ASSERT (((NODE_TYPE (COND_COND (arg_node)) == N_id)
                  || (NODE_TYPE (COND_COND (arg_node)) == N_bool)),
                 "if-clause condition is neither a N_id nor a N_bool node!");

    /*
     * COND_COND(arg_node) must not be traversed!
     */

    /* compile then and else part */
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
          = Ids2DecRcICMs (COND_THENVARS (arg_node), BLOCK_INSTR (COND_THEN (arg_node)));
    }

    if (COND_ELSEVARS (arg_node) != NULL) {
        BLOCK_INSTR (COND_ELSE (arg_node))
          = Ids2DecRcICMs (COND_ELSEVARS (arg_node), BLOCK_INSTR (COND_ELSE (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/**************************
 *
 *  with-loop
 *
 */

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
 * *** CODE NOT BRUSHED YET ***
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
                                                MakeId_Copy (VARDEC_OR_ARG_NAME (vardec)),
                                                DupIds_Id (wl_ids)),
                                      assigns);

                vardec = DFMGetMaskEntryDeclSet (NULL);
            }
            NWITH2_REUSE (arg_node) = DFMRemoveMask (NWITH2_REUSE (arg_node));
        }

        /*
         * 'ND_ALLOC_ARRAY'
         */
        assigns = AppendAssign (assigns, Ids2AllocArrayICMs_reuse (wl_ids, NULL));
    } else {
        /* fold-with-loop */

        fundef = INFO_COMP_FUNDEF (arg_info);

        fold_vardecs = GetFoldVardecs (NWITH2_FUNDEF (arg_node));
        if (fold_vardecs != NULL) {
            /*
             * insert vardecs of pseudo fold-fun
             */
            FUNDEF_VARDEC (fundef) = AppendVardec (FUNDEF_VARDEC (fundef), fold_vardecs);

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
    if (IDS_REFCNT (NWITH2_VEC (arg_node)) > 0) {
        assigns
          = AppendAssign (assigns, Ids2AllocArrayICMs (NWITH2_VEC (arg_node), NULL));
    }

    /*
     * compile all code blocks
     */
    NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);

    /*
     * build arguments for  'WL_..._BEGIN'-ICM and 'WL_..._END'-ICM
     */
    icm_args = MakeExprs (DupIds_Id (wl_ids),
                          MakeExprs (DupIds_Id (NWITH2_VEC (wl_node)),
                                     MakeExprs (MakeNum (NWITH2_DIMS (arg_node)), NULL)));

    switch (NWITH2_TYPE (arg_node)) {
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
        neutral = NWITH2_NEUTRAL (arg_node);
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
        neutral = Trav (MakeLet (DupTree (neutral), DupOneIds (wl_ids)), info);
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
                      MakeAssign (MakeIcm1 ("PF_BEGIN_WITH", MakeId_Copy (profile_name)),
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
                                                           MakeId_Copy (profile_name))));

    /*
     * insert RC-ICMs from 'wl_ids = neutral'
     */
    assigns = AppendAssign (assigns, rc_icms_wl_ids);

    /*
     * insert ICMs for memory management ('DEC_RC_FREE').
     */
    assigns = AppendAssign (assigns, Ids2DecRcICMs (NWITH2_DEC_RC_IDS (arg_node), NULL));

    /*
     * insert 'DEC_RC_FREE'-ICM for index-vector.
     */
    if (IDS_REFCNT (NWITH2_VEC (arg_node)) > 0) {
        assigns = AppendAssign (assigns, Ids2DecRcICMs (NWITH2_VEC (arg_node), NULL));
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
 * *** CODE NOT BRUSHED YET ***
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
    icm_assigns = Ids2IncRcICMs (NCODE_INC_RC_IDS (arg_node), NULL);

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
 * *** CODE NOT BRUSHED YET ***
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
    if ((NWITH2_TYPE (wl_node) == WO_genarray)
        || (NWITH2_TYPE (wl_node) == WO_modarray)) {
        assigns
          = MakeAssign (MakeIcm4 ("WL_INIT_OFFSET", MakeNum (GetDim (IDS_TYPE (wl_ids))),
                                  DupIds_Id (wl_ids), DupIds_Id (NWITH2_VEC (wl_node)),
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
 * *** CODE NOT BRUSHED YET ***
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
    ids_vector = NWITH2_VEC (wl_node);
    ids_scalar
      = GetIndexIds (NWITHID_IDS (NWITH2_WITHID (wl_node)), WLBLOCK_DIM (arg_node));
    icm_args
      = MakeExprs (MakeNum (WLBLOCK_DIM (arg_node)),
                   MakeExprs (DupIds_Id (ids_vector),
                              MakeExprs (DupIds_Id (ids_scalar),
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
                                  DupIds_Id (wl_ids)),
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
 * *** CODE NOT BRUSHED YET ***
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
    ids_vector = NWITH2_VEC (wl_node);
    ids_scalar
      = GetIndexIds (NWITHID_IDS (NWITH2_WITHID (wl_node)), WLUBLOCK_DIM (arg_node));
    icm_args
      = MakeExprs (MakeNum (WLUBLOCK_DIM (arg_node)),
                   MakeExprs (DupIds_Id (ids_vector),
                              MakeExprs (DupIds_Id (ids_scalar),
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
                                  DupIds_Id (wl_ids)),
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
 * *** CODE NOT BRUSHED YET ***
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
    ids_vector = NWITH2_VEC (wl_node);
    ids_scalar
      = GetIndexIds (NWITHID_IDS (NWITH2_WITHID (wl_node)), WLSTRIDE_DIM (arg_node));
    icm_args
      = MakeExprs (MakeNum (WLSTRIDE_DIM (arg_node)),
                   MakeExprs (DupIds_Id (ids_vector),
                              MakeExprs (DupIds_Id (ids_scalar),
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
                                  DupIds_Id (wl_ids)),
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
 * *** CODE NOT BRUSHED YET ***
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
        icm_args2 = ExprsConcat (icm_args2, MakeExprs (DupIds_Id (withid_ids), NULL));
        num_args++;
        withid_ids = IDS_NEXT (withid_ids);
    }
    icm_args2
      = MakeExprs (MakeNum (GetDim (IDS_TYPE (wl_ids))),
                   MakeExprs (DupIds_Id (wl_ids),
                              MakeExprs (DupIds_Id (NWITH2_VEC (wl_node)),
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

        if ((WLGRID_CODE (arg_node) != NULL)
            && (!NCODE_APT_DUMMY_CODE (WLGRID_CODE (arg_node)))) {

            /*
             * insert compiled code.
             */
            cexpr = WLGRID_CEXPR (arg_node);
            DBUG_ASSERT ((cexpr != NULL), "no code expr found");

            DBUG_ASSERT ((WLGRID_CBLOCK (arg_node) != NULL),
                         "no code block found in N_Ncode node");
            DBUG_ASSERT ((WLGRID_CBLOCK_INSTR (arg_node) != NULL),
                         "first instruction of block is NULL (should be a N_empty node)");

            if (NODE_TYPE (WLGRID_CBLOCK_INSTR (arg_node)) != N_empty) {
                assigns = DupTree (WLGRID_CBLOCK_INSTR (arg_node));
            }

            /*
             * choose right ICM
             */
            switch (NWITH2_TYPE (wl_node)) {
            case WO_genarray:
                /* here is no break missing! */
            case WO_modarray:
                DBUG_ASSERT ((NODE_TYPE (cexpr) == N_id), "code expr is not a id");
                icm_args2 = MakeExprs (MakeNum (GetDim (ID_TYPE (cexpr))),
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
                assigns = AppendAssign (assigns, GetFoldCode (NWITH2_FUNDEF (wl_node)));

                icm_name = NULL;
                break;

            default:
                DBUG_ASSERT ((0), "wrong withop type found");
            }

        } else {
            /*
             * no code found or array padding dummy code.
             *  => init/copy/noop
             */

            assigns = NULL;

            /*
             * choose right ICM
             */
            if (WLGRID_CODE (arg_node) == NULL) {

                switch (NWITH2_TYPE (wl_node)) {
                case WO_genarray:
                    icm_name = "WL_ASSIGN_INIT";
                    break;

                case WO_modarray:
                    icm_args2 = MakeExprs (DupNode (NWITH2_ARRAY (wl_node)), icm_args2);
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
            } else {
                /* dummy code inserted by array padding */
                icm_name = "WL_ASSIGN_NOOP";
            }
        }

        if (icm_name != NULL) {
            assigns = AppendAssign (assigns, MakeAssign (MakeIcm1 (icm_name, icm_args2),
                                                         dec_rc_cexpr));
        }
    }

    /* build argument list for 'WL_GRID_...'-ICMs */
    ids_vector = NWITH2_VEC (wl_node);
    ids_scalar
      = GetIndexIds (NWITHID_IDS (NWITH2_WITHID (wl_node)), WLGRID_DIM (arg_node));
    icm_args
      = MakeExprs (MakeNum (WLGRID_DIM (arg_node)),
                   MakeExprs (DupIds_Id (ids_vector),
                              MakeExprs (DupIds_Id (ids_scalar),
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
    if ((IDS_REFCNT (NWITH2_VEC (wl_node)) > 0)
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
 * *** CODE NOT BRUSHED YET ***
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
    if ((NWITH2_TYPE (wl_node) == WO_genarray)
        || (NWITH2_TYPE (wl_node) == WO_modarray)) {
        assigns
          = MakeAssign (MakeIcm4 ("WL_INIT_OFFSET", MakeNum (GetDim (IDS_TYPE (wl_ids))),
                                  DupIds_Id (wl_ids), DupIds_Id (NWITH2_VEC (wl_node)),
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
 * *** CODE NOT BRUSHED YET ***
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
    ids_vector = NWITH2_VEC (wl_node);
    ids_scalar
      = GetIndexIds (NWITHID_IDS (NWITH2_WITHID (wl_node)), WLSTRIVAR_DIM (arg_node));
    icm_args = MakeExprs (
      MakeNum (WLSTRIVAR_DIM (arg_node)),
      MakeExprs (
        DupIds_Id (ids_vector),
        MakeExprs (DupIds_Id (ids_scalar),
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
 * *** CODE NOT BRUSHED YET ***
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
            icm_args2 = ExprsConcat (icm_args2, MakeExprs (DupIds_Id (withid_ids), NULL));
            num_args++;
            withid_ids = IDS_NEXT (withid_ids);
        }
        icm_args2
          = MakeExprs (MakeNum (GetDim (IDS_TYPE (wl_ids))),
                       MakeExprs (DupIds_Id (wl_ids),
                                  MakeExprs (DupIds_Id (NWITH2_VEC (wl_node)),
                                             MakeExprs (MakeNum (num_args), icm_args2))));

        if (WLGRIDVAR_CODE (arg_node) != NULL) {

            /*
             * insert compiled code.
             */
            cexpr = WLGRIDVAR_CEXPR (arg_node);
            DBUG_ASSERT ((cexpr != NULL), "no code expr found");

            DBUG_ASSERT ((WLGRIDVAR_CBLOCK (arg_node) != NULL),
                         "no code block found in N_Ncode node");
            DBUG_ASSERT ((WLGRIDVAR_CBLOCK_INSTR (arg_node) != NULL),
                         "first instruction of block is NULL (should be a N_empty node)");

            if (NODE_TYPE (WLGRIDVAR_CBLOCK_INSTR (arg_node)) != N_empty) {
                assigns = DupTree (WLGRIDVAR_CBLOCK_INSTR (arg_node));
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
                assigns = AppendAssign (assigns, GetFoldCode (NWITH2_FUNDEF (wl_node)));

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
                icm_args2 = MakeExprs (DupNode (NWITH2_ARRAY (wl_node)), icm_args2);
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
    ids_vector = NWITH2_VEC (wl_node);
    ids_scalar
      = GetIndexIds (NWITHID_IDS (NWITH2_WITHID (wl_node)), WLGRIDVAR_DIM (arg_node));
    icm_args = MakeExprs (
      MakeNum (WLGRIDVAR_DIM (arg_node)),
      MakeExprs (DupIds_Id (ids_vector),
                 MakeExprs (DupIds_Id (ids_scalar),
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
    if ((IDS_REFCNT (NWITH2_VEC (wl_node)) > 0)
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

/**************************
 *
 *  MT
 *
 */

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
      = MakeIcm1 ("MT_SPMD_BEGIN", MakeId_Copy (FUNDEF_NAME (SPMD_FUNDEF (arg_node))));
    SPMD_ICM_ALTSEQ (arg_node)
      = MakeIcm1 ("MT_SPMD_ALTSEQ", MakeId_Copy (FUNDEF_NAME (SPMD_FUNDEF (arg_node))));
    SPMD_ICM_END (arg_node)
      = MakeIcm1 ("MT_SPMD_END", MakeId_Copy (FUNDEF_NAME (SPMD_FUNDEF (arg_node))));

    /*
     * Now, build up the ICMs of the parallel block.
     */

    fundef = INFO_COMP_FUNDEF (arg_info);

    assigns = NULL;

    assigns = MakeAssign (MakeIcm1 ("MT_SPMD_EXECUTE",
                                    MakeId_Copy (FUNDEF_NAME (SPMD_FUNDEF (arg_node)))),
                          assigns);

    /*
     * Now, build up the arguments for MT_SPMD_SETUP ICM.
     */

    num_args = 0;
    icm_args = BLOCK_SPMD_SETUP_ARGS (FUNDEF_BODY (SPMD_FUNDEF (arg_node)));
    while (icm_args != NULL) {
        num_args++;
        icm_args = EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (icm_args)));
        /*
         * Icm args are organized in triples, i.e. three ICM args belong
         * to a single original parameter.
         */
    }

    DBUG_PRINT ("COMPi", ("catched num_args %i", num_args));

    /*  icm_args = BLOCK_SPMD_SETUP_ARGS(FUNDEF_BODY(SPMD_FUNDEF(arg_node))); */
    num_args = 0;
    icm_args = NULL;
    icm_args = BuildParamsByDFM (SPMD_IN (arg_node), "in", &num_args, icm_args);
    icm_args = BuildParamsByDFM (SPMD_OUT (arg_node), "out", &num_args, icm_args);
    icm_args = BuildParamsByDFM (SPMD_SHARED (arg_node), "shared", &num_args, icm_args);

    icm_args = MakeExprs (MakeId_Copy (FUNDEF_NAME (SPMD_FUNDEF (arg_node))),
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
 *     < MALLOC-ICMs >                   // if (FIRST == 0) only
 *     MT_CONTINUE( ...)                 // if (FIRST == 0) only
 *     MT_START_SYNCBLOCK( ...)
 *     < with-loop code without MALLOC/free-ICMs >
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
          = ExprsConcat (icm_args3,
                         MakeExprs (MakeId_Copy (tag),
                                    MakeExprs (MakeTypeNode (VARDEC_OR_ARG_TYPE (vardec)),
                                               MakeExprs (MakeId_Copy (
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

        sync_args
          = ExprsConcat (sync_args,
                         MakeExprs (MakeId_Copy (VARDEC_OR_ARG_NAME (vardec)), NULL));

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
                        fold_type = type_string[GetBasetype (type)];
                    }

                    /*
                     * <fold_type>, <accu_var>
                     */
                    fold_args = MakeExprs (MakeId_Copy (fold_type),
                                           MakeExprs (DupIds_Id (with_ids), fold_args));

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
                if (IS_REFCOUNTED (IDS, with_ids)) {
                    fold_type = "array_rc";
                } else {
                    fold_type = "array";
                }
            } else {
                fold_type = type_string[GetBasetype (type)];
            }

            /*
             * <fold_type>, <accu_var>
             */
            icm_args = MakeExprs (MakeId_Copy (fold_type),
                                  MakeExprs (DupIds_Id (with_ids), NULL));

            barrier_args = ExprsConcat (barrier_args, icm_args);

            DBUG_PRINT ("COMP", ("%s", IDS_NAME (with_ids)));

            /*
             * <tmp_var>, <fold_op>
             */
            DBUG_ASSERT ((NWITH2_FUNDEF (with) != NULL), "no fundef found");
            barrier_args = ExprsConcat (barrier_args,
                                        MakeExprs (DupNode (NWITH2_CEXPR (with)),
                                                   MakeExprs (MakeId_Copy (FUNDEF_NAME (
                                                                NWITH2_FUNDEF (with))),
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
                    var_name = ID_NAME (ICM_ARG2 (instr));
                    prolog = TRUE;
                    epilog = FALSE;
                    DBUG_PRINT ("COMP", ("ICM: %s is prolog", ICM_NAME (instr)));
                } else if (strcmp (ICM_NAME (instr), "ND_INC_RC") == 0) {
                    var_name = ID_NAME (ICM_ARG1 (instr));
                    prolog = TRUE;
                    epilog = FALSE;
                    DBUG_PRINT ("COMP", ("ICM: %s is prolog", ICM_NAME (instr)));
                } else if ((strcmp (ICM_NAME (instr), "ND_DEC_RC_FREE_ARRAY") == 0)
                           || (strcmp (ICM_NAME (instr), "ND_DEC_RC") == 0)) {
                    var_name = ID_NAME (ICM_ARG1 (instr));
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
     *  -> insert extracted prolog-ICMs (MALLOC).
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

/*****************************************************************************
 **
 ** CONSTRUCTION AREA:
 **
 **/

/* move to tree_compound  !!! #### */

node *
MakeAssign_ (node *instr, node *next)
{
    node *result;

    if (NODE_TYPE (instr) == N_assign) {
        result = AppendAssign (instr, next);
    } else {
        result = MakeAssign (instr, next);
    }

    return (result);
}

node *
MakeAssigns1 (node *part1)
{
    return (MakeAssign_ (part1, NULL));
}

node *
MakeAssigns2 (node *part1, node *part2)
{
    return (MakeAssign_ (part1, MakeAssigns1 (part2)));
}

node *
MakeAssigns3 (node *part1, node *part2, node *part3)
{
    return (MakeAssign_ (part1, MakeAssigns2 (part2, part3)));
}

node *
MakeAssigns4 (node *part1, node *part2, node *part3, node *part4)
{
    return (MakeAssign_ (part1, MakeAssigns3 (part2, part3, part4)));
}

node *
MakeAssigns5 (node *part1, node *part2, node *part3, node *part4, node *part5)
{
    return (MakeAssign_ (part1, MakeAssigns4 (part2, part3, part4, part5)));
}

node *
MakeAssigns6 (node *part1, node *part2, node *part3, node *part4, node *part5,
              node *part6)
{
    return (MakeAssign_ (part1, MakeAssigns5 (part2, part3, part4, part5, part6)));
}

node *
MakeAssigns7 (node *part1, node *part2, node *part3, node *part4, node *part5,
              node *part6, node *part7)
{
    return (MakeAssign_ (part1, MakeAssigns6 (part2, part3, part4, part5, part6, part7)));
}

node *
MakeAssigns8 (node *part1, node *part2, node *part3, node *part4, node *part5,
              node *part6, node *part7, node *part8)
{
    return (MakeAssign_ (part1,
                         MakeAssigns7 (part2, part3, part4, part5, part6, part7, part8)));
}

node *
MakeAssigns9 (node *part1, node *part2, node *part3, node *part4, node *part5,
              node *part6, node *part7, node *part8, node *part9)
{
    return (MakeAssign_ (part1, MakeAssigns8 (part2, part3, part4, part5, part6, part7,
                                              part8, part9)));
}

/* #### */
node *
MakeAllocs (DFMmask_t mask)
{
    char *name;
    node *result;
    node *icm;

    DBUG_ENTER ("MakeAllocs");

    if (DFMTestMask (mask) != 0) {

        result = NULL;

        name = DFMGetMaskEntryNameSet (mask);
        while (name != NULL) {
            icm = MakeIcm1 ("ND_ALLOC_ARRAY", MakeId_Copy (name));
            result = MakeAssign (icm, result);
            name = DFMGetMaskEntryNameSet (NULL);
        }

        result = MakeAssign (MakeIcm1 ("COMMENT", MakeId_Copy ("ALLOCS")), result);

    } else {
        result = MakeAssigns1 (MakeIcm1 ("COMMENT", MakeId_Copy ("NO ALLOCS")));
    }

    DBUG_RETURN (result);
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
    /*
      allocate  = MakeIcm2( "MT2_ALLOCATE",
                            MakeNum( DFMTestMask( MT_ALLOC( arg_node))),
                            BuildParamsByDFM( MT_ALLOC( arg_node), "alloc", NULL, NULL));

    */
    allocate = MakeAllocs (MT_ALLOC (arg_node));

    /*
     *  Part 2 - Broadcast
     */
    broadcast
      = MakeIcm5 ("MT2_MASTER_BROADCAST", MakeId_Copy ("SYNC"),
                  MakeNum (MT_IDENTIFIER (arg_node)),
                  MakeNum (DFMTestMask (MT_ALLOC (arg_node))
                           + DFMTestMask (MT_USEMASK (arg_node))),
                  BuildParamsByDFM (MT_ALLOC (arg_node), "alloc", NULL, NULL),
                  BuildParamsByDFM (MT_USEMASK (arg_node), "usemask", NULL, NULL));

    /*
     *  Part 3 - Activate
     */
    activate = MakeIcm2 ("MT2_ACTIVATE", MakeId_Copy ("SYNC"),
                         MakeId_Copy (FUNDEF_NAME (MT_FUNDEF (arg_node))));

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
 *   node *COMPSt( node *arg_node, node *arg_info)
 *
 * description:
 *   compiles an N_st-node
 *
 ******************************************************************************/

node *
COMPSt (node *arg_node, node *arg_info)
{
    node *result, *fundef, *barrier, *code, *allocate;
    node *broadcast, *activate, *suspend, *receive;
    DFMmask_t dfm_flat; /* flattening needed */
    DFMmask_t bset;     /* broadcast-set */

    DBUG_ENTER ("COMPSt");

#ifdef DO_NOT_COMPILE_MTN
    DBUG_ASSERT (0, ("COMPSt not implemented yet, cannot compile this"));
#endif

    dfm_flat = DFMGenMaskAnd (ST_SYNC (arg_node), ST_NEEDLATER_MT (arg_node));
    bset = DFMGenMaskOr (ST_ALLOC (arg_node), dfm_flat);
    dfm_flat = DFMRemoveMask (dfm_flat);

    fundef = INFO_COMP_FUNDEF (arg_info);

    DBUG_PRINT ("COMPjhs", ("compiling %s attrib: %s status: %s", FUNDEF_NAME (fundef),
                            mdb_statustype[FUNDEF_ATTRIB (fundef)],
                            mdb_statustype[FUNDEF_STATUS (fundef)]));

    if (FUNDEF_ATTRIB (fundef) == ST_call_mt_master) {
        barrier = MakeIcm1 ("MT2_MASTER_BARRIER", MakeNum (ST_IDENTIFIER (arg_node)));
        code = MakeIcm0 ("CODE");

        allocate = MakeAllocs (ST_ALLOC (arg_node));
        /*
            allocate  = MakeIcm2( "MT2_ALLOCATE",
                                  MakeNum( DFMTestMask( ST_ALLOC( arg_node))),
                                  BuildParamsByDFM( ST_ALLOC( arg_node), "alloc", NULL,
           NULL));
        */
        broadcast
          = MakeIcm4 ("MT2_MASTER_BROADCAST", MakeId_Copy ("SYNC"),
                      MakeNum (MT_IDENTIFIER (arg_node)), MakeNum (DFMTestMask (bset)),
                      BuildParamsByDFM (bset, "bset", NULL, NULL));
        activate = MakeIcm2 ("MT2_ACTIVATE", MakeId_Copy ("SYNC"), MakeId_Copy ("NULL"));

        result = MakeAssigns5 (barrier, code, allocate, broadcast, activate);
    } else if (FUNDEF_ATTRIB (fundef) == ST_call_mt_worker) {
        barrier = MakeIcm1 ("MT2_WORKER_BARRIER", MakeNum (ST_IDENTIFIER (arg_node)));
        suspend = MakeIcm1 ("MT2_SUSPEND", MakeId_Copy ("SYNC"));
        receive
          = MakeIcm4 ("MT2_MASTER_RECEIVE", MakeId_Copy ("SYNC"),
                      MakeNum (MT_IDENTIFIER (arg_node)), MakeNum (DFMTestMask (bset)),
                      BuildParamsByDFM (bset, "bset", NULL, NULL));

        result = MakeAssigns3 (barrier, suspend, receive);
    } else {
        DBUG_ASSERT (0, "can not handle such a function");
    }

    bset = DFMRemoveMask (bset);

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

    assigns = MakeAssigns1 (MakeIcm0 ("MT2_SIGNAL_DATA"
#if 0
                ,
                MakeId_Copy( "SYNC"),
                MakeNum( DFMTestMask( MTSIGNAL_IDSET( arg_node))),
                BuildParamsByDFM( MTSIGNAL_IDSET( arg_node),
                                  "ids",
                                  NULL,
                                  NULL)
#endif
                                      ));

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
    alloc = MakeAllocs (MTALLOC_IDSET (arg_node));
    broadcast
      = MakeIcm3 (broadcast_icm, MakeId_Copy ("ALLOC"),
                  MakeNum (DFMTestMask (MTALLOC_IDSET (arg_node))),
                  BuildParamsByDFM (MTALLOC_IDSET (arg_node), "alloc", NULL, NULL));
    activate = MakeIcm2 ("MT2_ACTIVATE", MakeId_Copy ("ALLOC"), MakeId_Copy ("NULL"));
    else_ = MakeIcm0 ("MT2_ELSE_IF_I_AM_NOT_FIRST");
    suspend = MakeIcm1 ("MT2_SUSPEND", MakeId_Copy ("ALLOC"));
    receive = MakeIcm3 (receive_icm, MakeId_Copy ("ALLOC"),
                        MakeNum (DFMTestMask (MTALLOC_IDSET (arg_node))),
                        BuildParamsByDFM (MTALLOC_IDSET (arg_node), "alloc", NULL, NULL));
    end_ = MakeIcm0 ("MT2_END_I_AM_FIRST");

    assigns
      = MakeAssigns8 (if_, alloc, broadcast, activate, else_, suspend, receive, end_);

    arg_node = FreeTree (arg_node);

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPMTsync( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
COMPMTsync (node *arg_node, node *arg_info)
{
    node *result;
    node *fundef;
    node *if_;
    node *wait;
    node *broadcast;
    node *activate;
    node *else_;
    node *end_;
    node *args;
    node *suspend;
    node *receive;
    node *alloc;
    int nums;
    char *broadcast_icm;
    char *receive_icm;

    DBUG_ENTER ("COMPMTsync");

#ifdef DO_NOT_COMPILE_MTN
    DBUG_ASSERT (0, ("COMPMTsync not implemented yet, cannot compile this"));
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

    /* if */
    if_ = MakeIcm0 ("MT2_IF_I_AM_FIRST");
    /* alloc */
    alloc = MakeAllocs (MTSYNC_ALLOC (arg_node));
    /* wait */
    nums = 0;
    args = BuildParamsByDFMfold (MTSYNC_FOLD (arg_node), "sync", &nums, NULL);
    wait = MakeIcm2 ("MT2_WAIT_DATA", MakeNum (nums), args);
    /* broadcast */
    nums = 0;
    args = BuildParamsByDFMfold (MTSYNC_FOLD (arg_node), "sync", &nums,
                                 BuildParamsByDFM (MTSYNC_ALLOC (arg_node), "sync", &nums,
                                                   NULL));
    broadcast = MakeIcm3 (broadcast_icm, MakeId_Copy ("SYNC"), MakeNum (nums), args);
    /* activate */
    activate = MakeIcm2 ("MT2_ACTIVATE", MakeId_Copy ("SYNC"), MakeId_Copy ("NULL"));
    /* else */
    else_ = MakeIcm0 ("MT2_ELSE_IF_I_AM_NOT_FIRST");
    /* suspend */
    suspend = MakeIcm1 ("MT2_SUSPEND", MakeId_Copy ("SYNC"));
    /* receive */
    nums = 0;
    args = BuildParamsByDFMfold (MTSYNC_FOLD (arg_node), "sync", &nums,
                                 BuildParamsByDFM (MTSYNC_ALLOC (arg_node), "sync", &nums,
                                                   NULL));
    receive = MakeIcm3 (receive_icm, MakeId_Copy ("SYNC"), MakeNum (nums), args);
    /* end */
    end_ = MakeIcm0 ("MT2_END_I_AM_FIRST");

    result = MakeAssigns9 (if_, alloc, wait, broadcast, activate, else_, suspend, receive,
                           end_);

    arg_node = FreeTree (arg_node);

    DBUG_RETURN (result);
}
