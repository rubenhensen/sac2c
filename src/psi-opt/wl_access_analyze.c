/*
 *
 * $Log$
 * Revision 3.7  2001/11/19 15:30:53  sbs
 * fixed a bug in WLAAncode where wrong info concerning WLARRAY and
 * INDEXVAR were inserted whenever nested WLs were used...
 *
 * Revision 3.6  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.5  2001/05/17 13:41:26  nmw
 * MALLOC/FREE replaced by Malloc/Free, using result of Free()
 *
 * Revision 3.4  2001/02/06 01:18:58  dkr
 * include of print.h removed
 *
 * Revision 3.3  2000/12/13 14:51:39  sbs
 * changed initialization of feature_t vars from NULL into 0 !
 *
 * Revision 3.2  2000/12/06 19:22:16  cg
 * Removed compiler warnings in production mode.
 *
 * Revision 3.1  2000/11/20 18:01:57  sacbase
 * new release made
 *
 * Revision 2.20  2000/10/26 13:47:52  dkr
 * MakeShpseg used now :-)
 *
 * Revision 2.19  2000/08/03 19:29:39  bs
 * Bug fixed in WLAAprf, case F_sub_AxS
 *
 * Revision 2.18  2000/07/07 15:30:59  bs
 *  DBUG_PRINT in WLAAid modified.
 *
 * Revision 2.17  2000/07/06 08:23:45  mab
 * syntax error in line 1559 fixed
 *
 * Revision 2.16  2000/07/05 15:28:18  bs
 * DBUG_PRINTs added.
 *
 * Revision 2.15  2000/07/04 17:50:58  bs
 * WLAAfundef modified (WLAA and TSI lifted on module level).
 *
 * Revision 2.14  2000/05/26 14:24:54  sbs
 * restriction on N_fundef as sole entry point alleviated
 * Now, N_module is allowed as well.
 *
 * Revision 2.13  2000/04/25 17:17:18  bs
 * Bug fixed in WLAAprf.
 *
 * Revision 2.12  2000/01/31 19:29:50  bs
 * Function added: AddIntVec2Shpseg
 * Function modified: WLAAprf: the cases F_add_AxA and F_sub_AxA
 * Function modified: SearchAccess has got no break anymore.
 *
 * Revision 2.11  2000/01/26 17:26:20  dkr
 * type of traverse-function-table changed.
 *
 * Revision 2.10  1999/11/24 14:53:58  bs
 * Another wlaa feature added: FEATURE_UNKNOWN for an noninferable offset.
 *
 * Revision 2.9  1999/08/30 18:31:15  bs
 * Bugs fixed.
 *
 * Revision 2.8  1999/08/30 13:29:49  bs
 * WLAAncode modified: access macros added, access macros renamed.
 *
 * Revision 2.7  1999/08/04 14:35:11  bs
 * NCODE_WLAA_xxx macros added.
 *
 * Revision 2.6  1999/07/28 13:16:17  bs
 * No more DupNode's.
 * WLAAcond modified.
 * Bug fixed in WLAAprf.
 *
 * Revision 2.5  1999/07/21 14:49:12  bs
 * Some DupNode()'s inserted.
 * WLAAcond modified.
 *
 * Revision 2.4  1999/07/08 16:13:14  bs
 * ASSERTIONS changed to WARNINGS
 *
 * Revision 2.3  1999/05/19 14:25:14  bs
 * Bug fixed (in WLAAnwith and WLAAncode).
 *
 * Revision 2.2  1999/05/18 13:08:55  dkr
 * print.h included (for WLAAprintAccesses() )
 *
 * Revision 2.1  1999/05/12 14:02:51  bs
 * attribute names for handling constant vectors at N_id, N_array, and N_info
 * nodes adjusted to each other...
 *
 * Revision 1.4  1999/05/10 16:19:09  bs
 * Bug fixed in WLAAprf
 *
 * Revision 1.3  1999/05/10 12:20:38  bs
 * Comments added.
 *
 * Revision 1.2  1999/05/10 10:20:30  bs
 * Comments added.
 *
 * Revision 1.1  1999/05/07 15:31:06  bs
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   wl_access_analyze.h
 *
 * prefix: WLAA
 *
 * description:
 *
 *   This compiler module analyzes array accesses within withloops.
 *   It is used by the tilesize inference (and maybe later by WL fusion).
 *
 *   The following access macros are defined for the info-node:
 *
 *   INFO_WLAA_ACCESS(n)      ((access_t*)(n->info2))
 *   INFO_WLAA_COUNT(n)                   (n->counter)
 *   INFO_WLAA_FEATURE(n)     ((feature_t)(n->lineno))
 *   INFO_WLAA_WOTYPE(n)     ((WithOpType)(n->varno))
 *   INFO_WLAA_LASTLETIDS(n)              (n->info.ids)
 *   INFO_WLAA_BELOWAP(n)                 (n->flag)
 *   INFO_WLAA_WLLEVEL(n)                 (n->refcnt)
 *   INFO_WLAA_INDEXVAR(n)                (n->node[0])
 *   INFO_WLAA_ACCESSVEC(n)     ((shpseg*)(n->node[1]))     not in use
 *   INFO_WLAA_TMPACCESS(n)   ((access_t*)(n->node[2]))     not in use
 *   INFO_WLAA_WLARRAY(n)                 (n->node[3])
 *
 *****************************************************************************/

#include <stdlib.h>
#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"
#include "Error.h"
#include "wl_access_analyze.h"

/******************************************************************************
 *
 * function:
 *   node *WLAccessAnalyze(node *arg_node)
 *
 * description:
 *
 *   This function initiates the WL access analyze scheme, i.e.
 *   act_tab is set to WLAA_tab and the traversal mechanism is started.
 *   Just as the other optimization schemes, WL access analyzing is performed
 *   on single function definitions rather than on the entire syntax tree.
 *
 *****************************************************************************/

node *
WLAccessAnalyze (node *arg_node)
{
    funtab *tmp_tab;
    node *arg_info;

    DBUG_ENTER ("WLAccessAnalyze");

    DBUG_PRINT ("WLAA", ("WLAccessAnalyze"));

    DBUG_ASSERT (((NODE_TYPE (arg_node) == N_modul)
                  || (NODE_TYPE (arg_node) == N_fundef)),
                 "WLAccessAnalyze not initiated on N_modul or N_fundef level");

    tmp_tab = act_tab;
    act_tab = wlaa_tab;
    arg_info = MakeInfo ();
    INFO_WLAA_WLLEVEL (arg_info) = 0;

    arg_node = Trav (arg_node, arg_info);

    arg_info = FreeInfo (arg_info, NULL);
    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   nums   *NumVect2NumsList(int coeff, node *exprs)
 *
 * description:
 *   This function converts an N_exprs list from an N_array node into a list
 *   of type nums, i.e. the elements of a constant int vector are transformed
 *   into a format suitable to build a shape segment. During the format
 *   conversion each element is multiplied by <coeff>.
 *
 * remark:
 *   Here, a recursive implementation is used although recrsion is not without
 *   problems concerning the stack size and the performance. However, a
 *   recursive implementation where the order of the list is untouched is much
 *   easier than an iterative one and this function is only used for index
 *   vectors, i.e. the lists are quite short.
 *
 *****************************************************************************/

static nums *
NumVect2NumsList (int coeff, node *exprs)
{
    nums *tmp;

    DBUG_ENTER ("NumVect2NumsList");

    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs),
                 "Illegal node type in call to function NumVect2NumsList()");

    if (exprs == NULL) {
        tmp = NULL;
    } else {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (exprs)) == N_num),
                     "Illegal expression in call to function NumVect2NumsList()");

        tmp = MakeNums (coeff * NUM_VAL (EXPRS_EXPR (exprs)),
                        NumVect2NumsList (coeff, EXPRS_NEXT (exprs)));
    }

    DBUG_RETURN (tmp);
}

/******************************************************************************
 *
 * function:
 *   shpseg *Shpseg2Shpseg(int coeff, shpseg *old_shpseg)
 *
 * description:
 *   This function creates a new shape vector. The new shape vector
 *   is made of 'old_shpseg' multiplied by <coeff>.
 *
 * remark:
 *   Here, a recursive implementation is used although recrsion is not without
 *   problems concerning the stack size and the performance. However, a
 *   recursive implementation where the order of the list is untouched is much
 *   easier than an iterative one and this function is only used for index
 *   vectors, i.e. the lists are quite short.
 *
 ******************************************************************************/

static shpseg *
Shpseg2Shpseg (int coeff, shpseg *old_shpseg)
{
    int i;
    shpseg *new_shpseg;

    DBUG_ENTER ("Shpseg2Shpseg");

    if (old_shpseg == NULL) {
        new_shpseg = NULL;
    } else {
        new_shpseg = MakeShpseg (NULL);
        for (i = 0; i < SHP_SEG_SIZE; i++) {
            SHPSEG_SHAPE (new_shpseg, i) = coeff * SHPSEG_SHAPE (old_shpseg, i);
        }
        SHPSEG_NEXT (new_shpseg) = Shpseg2Shpseg (coeff, SHPSEG_NEXT (old_shpseg));
    }

    DBUG_RETURN (new_shpseg);
}

/******************************************************************************
 *
 * function:
 *   shpseg *IntVec2Shpseg(int coeff, int length, int *intvec, shpseg *next)
 *
 * description:
 *   This functions convert a vector of constant integers into a shape vector.
 *   During the format conversion each element is multiplied by <coeff>.
 *
 *****************************************************************************/

static shpseg *
IntVec2Shpseg (int coeff, int length, int *intvec, shpseg *next)
{
    int i;
    shpseg *result;

    DBUG_ENTER ("IntVec2Shpseg");

    result = MakeShpseg (NULL);

    if (length == 0) {
        for (i = 0; i < SHP_SEG_SIZE; i++) {
            SHPSEG_SHAPE (result, i) = 0;
        }
    } else {
        for (i = 0; i < length; i++) {
            SHPSEG_SHAPE (result, i) = coeff * intvec[i];
        }
        for (i = length; i < SHP_SEG_SIZE; i++) {
            SHPSEG_SHAPE (result, i) = 0;
        }
    }
    SHPSEG_NEXT (result) = next;

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   shpseg *AddIntVec2Shpseg(int coeff, int length, int *intvec, shpseg *next)
 *
 * description:
 *   This functions convert a vector of constant integers into a shape vector.
 *   During the format conversion each element is multiplied by <coeff>.
 *   The result will be added to the shape vector <next> if not NULL.
 *
 *****************************************************************************/

static shpseg *
AddIntVec2Shpseg (int coeff, int length, int *intvec, shpseg *next)
{
    int i;
    shpseg *result;

    DBUG_ENTER ("IntVec2Shpseg");

    if (next == NULL) {
        result = MakeShpseg (NULL);
        for (i = 0; i < SHP_SEG_SIZE; i++) {
            SHPSEG_SHAPE (result, i) = 0;
        }
    } else {
        result = next;
    }

    for (i = 0; i < length; i++) {
        SHPSEG_SHAPE (result, i) += (coeff * intvec[i]);
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   bool IsIndexVect(types *type)
 *
 * description:
 *
 *   This function classifies certain types as "index vectors", i.e. small
 *   integer vectors on which primitive arithmetic operations shall not harm
 *   tile size inference.
 *
 *****************************************************************************/

static bool
IsIndexVect (types *type)
{
    bool res = FALSE;

    DBUG_ENTER ("IsIndexVect");

    if ((TYPES_BASETYPE (type) == T_int) && (TYPES_DIM (type) == 1)
        && (TYPES_SHAPE (type, 0) < SHP_SEG_SIZE)) {
        res = TRUE;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   int AccessInAccesslist(access_t* access, access_t* access_list)
 *
 * description:
 *
 *   AccessInAccesslist( a, al ) = 1  if al contains a,
 *                                 0  otherwise.
 *
 ******************************************************************************/

static int
AccessInAccesslist (access_t *access, access_t *access_list)
{
    int isin = FALSE;
    int i;

    while ((access_list != NULL) && (isin == FALSE)) {

        DBUG_ASSERT ((access != NULL), ("No access to compare with !"));

        if ((ACCESS_CLASS (access) == ACCESS_CLASS (access_list))
            && (ACCESS_IV (access) == ACCESS_IV (access_list))
            && (ACCESS_ARRAY (access) == ACCESS_ARRAY (access_list))
            && (ACCESS_DIR (access) == ACCESS_DIR (access_list))) {
            i = 0;
            isin = TRUE;
            while ((i < SHP_SEG_SIZE) && isin) {
                if (SHPSEG_SHAPE (ACCESS_OFFSET (access), i)
                    != SHPSEG_SHAPE (ACCESS_OFFSET (access_list), i)) {
                    isin = FALSE;
                }
                i++;
            }
        }
        access_list = ACCESS_NEXT (access_list);
    }

    return (isin);
}

/******************************************************************************
 *
 * function:
 *   access_t* KillDoubleAccesses(access_t* access_list)
 *
 * description:
 *
 *   This function results the access_list containing not the same
 *   entrie twice.
 *
 *
 *****************************************************************************/
#if 0
/*
 *  not needed for the time being
 */
static access_t* KillDoubleAccesses(access_t* access_list) 
{
  access_t* result;
  
  if (AccessInAccesslist(access_list, ACCESS_NEXT( access_list))) {
    result = KillDoubleAccesses(ACCESS_NEXT( access_list));
    FreeOneAccess( access_list);
  }
  else {
    result = access_list;
    ACCESS_NEXT( result) = KillDoubleAccesses( ACCESS_NEXT( access_list));
  }
  return(result);
}
#endif

/******************************************************************************
 *
 * function:
 *   access_t* CatAccesslists(access_t* a_list_1, access_t* a_list_2)
 *
 * description:
 *
 *   This function results the concatenation of a_list_1 and a_list_2.
 *
 *
 *****************************************************************************/

static access_t *
CatAccesslists (access_t *a_list_1, access_t *a_list_2)
{
    access_t *a_list_res;

    if (a_list_1 == NULL) {
        a_list_1 = a_list_2;
        a_list_res = a_list_2;
    } else {
        a_list_res = a_list_1;
        while (ACCESS_NEXT (a_list_1) != NULL) {
            a_list_1 = ACCESS_NEXT (a_list_1);
        }
        ACCESS_NEXT (a_list_1) = a_list_2;
    }

    return (a_list_res);
}

/******************************************************************************
 *
 * function:
 *   access_t *SearchAccess(node *arg_info)
 *
 * description:
 *
 *   This function traverses all accesses stored in the given arg_info node.
 *   It returns the first one whose index vector is exactly the same variable
 *   that is instantiated in the last let-expression. Additionally, the access
 *   class still must be ACL_unknown.
 *
 ******************************************************************************/

static access_t *
SearchAccess (access_t *access, node *arg_info)
{
    int found = 0;

    DBUG_ENTER ("SearchAccess");

    while ((access != NULL) && (found == 0)) {
        if ((ACCESS_IV (access) == IDS_VARDEC (INFO_WLAA_LASTLETIDS (arg_info)))
            && (ACCESS_CLASS (access) == ACL_unknown)) {
            found = 1;
        } else {
            access = ACCESS_NEXT (access);
        }
    }

    DBUG_RETURN (access);
}

/******************************************************************************
 *
 * function:
 *   node *WLAAfundef(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_fundef node.
 *
 *   The traversal is limited to the function body, arguments and remaining
 *   functions are not traversed.
 *
 *****************************************************************************/

node *
WLAAfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLAAfundef");

    DBUG_PRINT ("WLAA", ("WLAAfundef"));

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        /*
         * Nodetype of FUNDEF_BODY(arg_node) is N_block.
         */
    }
#if 1
    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }
#endif

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLAAblock(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_block node.
 *
 *   The traversal is limited to the assignments chain, variable declarations
 *   are not traversed.
 *
 *
 *****************************************************************************/

node *
WLAAblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLAAblock");

    DBUG_PRINT ("WLAA", ("WLAAblock"));

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
        /*
         * Nodetype of BLOCK_INSTR(arg_node) is N_assign or N_empty.
         */
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLAAassign(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_assign node.
 *
 *   This function just realizes a post-order traversal of the code.
 *
 *****************************************************************************/

node *
WLAAassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLAAassign");

    DBUG_PRINT ("WLAA", ("WLAAassign"));

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "N_assign node without instruction.");

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLAAnwith(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_Nwith node.
 *
 *   Here, the arg_info node is created and initialized. Everywhere beyond
 *   an N_Nwith node, arg_info is not NULL. See general remarks about usage
 *   of arg_info in this compiler module.
 *
 *   In the case of a nested with-loop, the old arg_info is stored and the
 *   whole game is restarted. Afterwards, the outer with-loops features bit
 *   mask is set accordingly.
 *
 *****************************************************************************/

node *
WLAAnwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLAAnwith");

    DBUG_PRINT ("WLAA", ("WLAAnwith"));

    DBUG_ASSERT ((arg_info != NULL), "WLAAnwith called with empty info node!");
    INFO_WLAA_INDEXVAR (arg_info) = IDS_VARDEC (NWITH_VEC (arg_node));
    INFO_WLAA_WOTYPE (arg_info) = NWITH_TYPE (arg_node);
    INFO_WLAA_WLLEVEL (arg_info) = INFO_WLAA_WLLEVEL (arg_info) + 1;

    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    INFO_WLAA_WLLEVEL (arg_info) = INFO_WLAA_WLLEVEL (arg_info) - 1;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLAAncode(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_Ncode node.
 *
 *   The code block of the first operator is traversed, the information
 *   gathered is stored in this node, and the traversal continues with the
 *   following operator.
 *
 *****************************************************************************/

node *
WLAAncode (node *arg_node, node *arg_info)
{
    node *old_arg_info;

    DBUG_ENTER ("WLAAncode");

    DBUG_PRINT ("WLAA", ("WLAAncode"));

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    /*
     * Store old arg_info for the case of nested with-loops.
     */
    old_arg_info = arg_info;
    arg_info = MakeInfo ();

    INFO_WLAA_ACCESS (arg_info) = NULL;
    INFO_WLAA_COUNT (arg_info) = 0;
    INFO_WLAA_INDEXVAR (arg_info) = INFO_WLAA_INDEXVAR (old_arg_info);
    INFO_WLAA_FEATURE (arg_info) = FEATURE_NONE;
    INFO_WLAA_WOTYPE (arg_info) = INFO_WLAA_WOTYPE (old_arg_info);
    INFO_WLAA_BELOWAP (arg_info) = 0;
    INFO_WLAA_WLLEVEL (arg_info) = INFO_WLAA_WLLEVEL (old_arg_info);
    INFO_WLAA_WLARRAY (arg_info) = INFO_WLAA_WLARRAY (old_arg_info);

    if (NCODE_CBLOCK (arg_node) != NULL) {
        if ((INFO_WLAA_WOTYPE (arg_info) == WO_genarray)
            || (INFO_WLAA_WOTYPE (arg_info) == WO_modarray)) {
            INFO_WLAA_ACCESS (arg_info)
              = MakeAccess (INFO_WLAA_WLARRAY (arg_info), INFO_WLAA_INDEXVAR (arg_info),
                            ACL_offset, NULL, ADIR_write, INFO_WLAA_ACCESS (arg_info));
            INFO_WLAA_COUNT (arg_info)++;
            ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
              = IntVec2Shpseg (1, 0, NULL, ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info)));
        }

        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    NCODE_WLAA_INFO (arg_node) = MakeInfo ();
    NCODE_WLAA_ACCESS (arg_node) = INFO_WLAA_ACCESS (arg_info);
    NCODE_WLAA_FEATURE (arg_node) = INFO_WLAA_FEATURE (arg_info);
    NCODE_WLAA_ACCESSCNT (arg_node) = INFO_WLAA_COUNT (arg_info);
    FreeInfo (arg_info, NULL);
    arg_info = old_arg_info;

    /*
     * WLARRAY and INDEXVAR have to be taken from old_arg_info, since
     * they are set from OUTSIDE rather than being inferred during
     * the traversal of the code block!
     */

    NCODE_WLAA_WLARRAY (arg_node) = INFO_WLAA_WLARRAY (arg_info);
    NCODE_WLAA_INDEXVAR (arg_node) = INFO_WLAA_INDEXVAR (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLAAwhile(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_while node.
 *
 *   If a with-loop operator contains a sequential loop (for/do/while), then
 *   tile size inference has lost. The only exception is the case where the
 *   loop does not contain any array operations. The function detects exactly
 *   this and sets the features bit mask accordingly.
 *
 *****************************************************************************/

node *
WLAAwhile (node *arg_node, node *arg_info)
{
    access_t *old_access = NULL;
    feature_t old_feature = 0;

    DBUG_ENTER ("WLAAwhile");

    DBUG_PRINT ("WLAA", ("WLAAwhile"));

    if (INFO_WLAA_WLLEVEL (arg_info) > 0) {
        old_access = INFO_WLAA_ACCESS (arg_info);
        old_feature = INFO_WLAA_FEATURE (arg_info);
        INFO_WLAA_ACCESS (arg_info) = NULL;
        INFO_WLAA_FEATURE (arg_info) = FEATURE_NONE;
    }

    WHILE_BODY (arg_node) = Trav (WHILE_BODY (arg_node), arg_info);

    if (INFO_WLAA_WLLEVEL (arg_info) > 0) {
        if ((INFO_WLAA_ACCESS (arg_info) == NULL)
            && (INFO_WLAA_FEATURE (arg_info) == FEATURE_NONE)) {
            /*
             * Nothing harmful happened within the loop.
             */
            INFO_WLAA_ACCESS (arg_info) = old_access;
            INFO_WLAA_FEATURE (arg_info) = old_feature;
        } else {
            /*
             * Something harmful happened within the loop.
             */
            FreeAllAccess (INFO_WLAA_ACCESS (arg_info));
            INFO_WLAA_ACCESS (arg_info) = old_access;
            INFO_WLAA_FEATURE (arg_info)
              = INFO_WLAA_FEATURE (arg_info) | FEATURE_LOOP | old_feature;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLAAdo(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_do node.
 *
 *   This function is equivalent to WLAAwhile for while loops. The kind of loop
 *   makes no difference for the purpose of tile size inference.
 *
 *****************************************************************************/

node *
WLAAdo (node *arg_node, node *arg_info)
{
    access_t *old_access = NULL;
    feature_t old_feature = 0;

    DBUG_ENTER ("WLAAdo");

    DBUG_PRINT ("WLAA", ("WLAAdo"));

    if (INFO_WLAA_WLLEVEL (arg_info) > 0) {
        old_access = INFO_WLAA_ACCESS (arg_info);
        old_feature = INFO_WLAA_FEATURE (arg_info);
        INFO_WLAA_ACCESS (arg_info) = NULL;
        INFO_WLAA_FEATURE (arg_info) = FEATURE_NONE;
    }

    DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);

    if (INFO_WLAA_WLLEVEL (arg_info) > 0) {
        if ((INFO_WLAA_ACCESS (arg_info) == NULL)
            && (INFO_WLAA_FEATURE (arg_info) == FEATURE_NONE)) {
            /*
             * Nothing harmful happened within the loop.
             */
            INFO_WLAA_ACCESS (arg_info) = old_access;
            INFO_WLAA_FEATURE (arg_info) = old_feature;
        } else {
            /*
             * Anything harmful happened within the loop.
             */
            FreeAllAccess (INFO_WLAA_ACCESS (arg_info));
            INFO_WLAA_ACCESS (arg_info) = old_access;
            INFO_WLAA_FEATURE (arg_info)
              = INFO_WLAA_FEATURE (arg_info) | FEATURE_LOOP | old_feature;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLAAcond(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 *
 *
 *****************************************************************************/

node *
WLAAcond (node *arg_node, node *arg_info)
{
    node *then_arg_info, *else_arg_info;
    int equal_flag = TRUE;
    access_t *access;

    DBUG_ENTER ("WLAAcond");

    DBUG_PRINT ("WLAA", ("WLAAcond"));

    if (INFO_WLAA_WLLEVEL (arg_info) > 0) {
        then_arg_info = MakeInfo ();
        INFO_WLAA_INDEXVAR (then_arg_info) = INFO_WLAA_INDEXVAR (arg_info);
        INFO_WLAA_WOTYPE (then_arg_info) = INFO_WLAA_WOTYPE (arg_info);
        INFO_WLAA_WLLEVEL (then_arg_info) = INFO_WLAA_WLLEVEL (arg_info);
        INFO_WLAA_WLARRAY (then_arg_info) = INFO_WLAA_WLARRAY (arg_info);
        INFO_WLAA_LASTLETIDS (then_arg_info) = INFO_WLAA_LASTLETIDS (arg_info);
        INFO_WLAA_BELOWAP (then_arg_info) = INFO_WLAA_BELOWAP (arg_info);
        INFO_WLAA_ACCESS (then_arg_info) = NULL;
        INFO_WLAA_COUNT (then_arg_info) = 0;
        INFO_WLAA_FEATURE (then_arg_info) = FEATURE_NONE;

        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), then_arg_info);

        else_arg_info = MakeInfo ();
        INFO_WLAA_INDEXVAR (else_arg_info) = INFO_WLAA_INDEXVAR (arg_info);
        INFO_WLAA_WOTYPE (else_arg_info) = INFO_WLAA_WOTYPE (arg_info);
        INFO_WLAA_WLLEVEL (else_arg_info) = INFO_WLAA_WLLEVEL (arg_info);
        INFO_WLAA_WLARRAY (else_arg_info) = INFO_WLAA_WLARRAY (arg_info);
        INFO_WLAA_LASTLETIDS (else_arg_info) = INFO_WLAA_LASTLETIDS (arg_info);
        INFO_WLAA_BELOWAP (else_arg_info) = INFO_WLAA_BELOWAP (arg_info);
        INFO_WLAA_ACCESS (else_arg_info) = NULL;
        INFO_WLAA_COUNT (else_arg_info) = 0;
        INFO_WLAA_FEATURE (else_arg_info) = FEATURE_NONE;

        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), else_arg_info);

        if (INFO_WLAA_FEATURE (then_arg_info) != INFO_WLAA_FEATURE (else_arg_info)) {
            equal_flag = FALSE;
        } else if (INFO_WLAA_COUNT (then_arg_info) != INFO_WLAA_COUNT (else_arg_info))
            equal_flag = FALSE;
        else {
            access = INFO_WLAA_ACCESS (then_arg_info);
            while ((access != NULL) && (equal_flag == TRUE)) {
                equal_flag
                  = AccessInAccesslist (access, INFO_WLAA_ACCESS (else_arg_info));
                access = ACCESS_NEXT (access);
            }
        }

        if (equal_flag) {
            /*
             *  In COND_THEN and COND_ELSE are the same accesses.
             */
            INFO_WLAA_ACCESS (arg_info)
              = CatAccesslists (INFO_WLAA_ACCESS (then_arg_info),
                                INFO_WLAA_ACCESS (arg_info));
            INFO_WLAA_COUNT (arg_info) += INFO_WLAA_COUNT (then_arg_info);
        } else {
            INFO_WLAA_FEATURE (arg_info) |= FEATURE_COND;
            FreeAllAccess (INFO_WLAA_ACCESS (then_arg_info));
        }
        FreeAllAccess (INFO_WLAA_ACCESS (else_arg_info));

        INFO_WLAA_FEATURE (arg_info) = INFO_WLAA_FEATURE (arg_info)
                                       | INFO_WLAA_FEATURE (then_arg_info)
                                       | INFO_WLAA_FEATURE (else_arg_info);

        else_arg_info = FreeTree (else_arg_info); /* Attention !!! */
        then_arg_info = FreeTree (then_arg_info); /* Attention !!! */
    }

    else {
        /*
         *  Here we aren't within a withloop:
         */
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLAAlet(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_let node.
 *
 *   This function mainly sets the LASTLETIDS entry of the arg_info node
 *   correctly and traverses the left hand side of the let.
 *   Afterwards, it is checked whether any of the variables set by this let,
 *   has been used as an index vector in a sel operation and the respective
 *   access is still classified ACL_unknown. In this case, the access is
 *   re-classified as ACL_irregular. Any "regular" access would have already
 *   been handled during the traversal of the left hand side.
 *
 *****************************************************************************/

node *
WLAAlet (node *arg_node, node *arg_info)
{
    ids *var;
    access_t *access;

    DBUG_ENTER ("WLAAlet");

    DBUG_PRINT ("WLAA", ("WLAAlet"));

    INFO_WLAA_LASTLETIDS (arg_info) = LET_IDS (arg_node);
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_Nwith)
        INFO_WLAA_WLARRAY (arg_info) = IDS_VARDEC (LET_IDS (arg_node));

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    if (INFO_WLAA_WLLEVEL (arg_info) > 0) {
        /*
         * Here, we are within a with-loop.
         */
        INFO_WLAA_LASTLETIDS (arg_info) = NULL;

        var = LET_IDS (arg_node);

        while (var != NULL) {
            access = INFO_WLAA_ACCESS (arg_info);
            while (access != NULL) {
                if ((IDS_VARDEC (var) == ACCESS_IV (access))
                    && (ACCESS_CLASS (access) == ACL_unknown)) {

                    DBUG_ASSERT (((NODE_TYPE (ACCESS_IV (access)) == N_arg)
                                  || (NODE_TYPE (ACCESS_IV (access)) == N_vardec)),
                                 "Not a valid index-vector!");
#if 0          
          ACCESS_CLASS(access) = ACL_irregular;
#endif
                    INFO_WLAA_FEATURE (arg_info) |= FEATURE_UNKNOWN;
                }
                access = ACCESS_NEXT (access);
            }
            var = IDS_NEXT (var);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLAAprf(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_prf node.
 *
 *   This function does most of the code analysis. See specific functions
 *   for more information.
 *
 *****************************************************************************/

node *
WLAAprf (node *arg_node, node *arg_info)
{
    int i, argnum;
    /*
    int      access_flag = CURRENT_A;
    */
    int *offset;
    access_t *access;
    node *arg_node_arg1, *arg_node_arg2, *prf_arg;

    DBUG_ENTER ("WLAAprf");

    DBUG_PRINT ("WLAA", ("WLAAprf"));

    if (INFO_WLAA_WLLEVEL (arg_info) > 0) {
        /*
         * Here, we are within a with-loop.
         */
        argnum = 0;
        prf_arg = PRF_ARGS (arg_node);
        /*
         *   counting the arguments:
         */
        while (prf_arg != NULL) {
            argnum++;
            prf_arg = EXPRS_NEXT (prf_arg);
        }
        if (argnum == 2) {
            arg_node_arg1 = PRF_ARG1 (arg_node);
            arg_node_arg2 = PRF_ARG2 (arg_node);

            switch (PRF_PRF (arg_node)) {
            case F_sel:
                DBUG_PRINT ("WLAA_INFO", ("primitive function F_sel"));

                if (INFO_WLAA_INDEXDIM (arg_info) != INFO_WLAA_ARRAYDIM (arg_info)) {
                    /*
                     *  Result of sel must not be a skalar !
                     */
                    DBUG_PRINT ("WLAA_INFO",
                                ("primitive function sel with array return value"));
                    INFO_WLAA_FEATURE (arg_info)
                      = INFO_WLAA_FEATURE (arg_info) | FEATURE_ASEL;
                }
                if (IDS_DIM (INFO_WLAA_LASTLETIDS (arg_info)) == SCALAR) {

                    DBUG_ASSERT (((NODE_TYPE (arg_node_arg1) == N_id)
                                  || (NODE_TYPE (arg_node_arg1) == N_array)),
                                 "1st arg of sel is neither an array nor a variable");
                    DBUG_ASSERT ((NODE_TYPE (arg_node_arg2) == N_id),
                                 "2nd arg of sel is not variable");

                    if (NODE_TYPE (arg_node_arg1) == N_id) {
                        INFO_WLAA_ACCESS (arg_info)
                          = MakeAccess (ID_VARDEC (arg_node_arg2),
                                        ID_VARDEC (arg_node_arg1), ACL_unknown, NULL,
                                        ADIR_read, INFO_WLAA_ACCESS (arg_info));
                        if (ACCESS_IV (INFO_WLAA_ACCESS (arg_info))
                            == INFO_WLAA_INDEXVAR (arg_info)) {
                            /*
                             *  The array is accessed by the index vector of the
                             *  surrounding with-loop.
                             */
                            ACCESS_CLASS (INFO_WLAA_ACCESS (arg_info)) = ACL_offset;

                            DBUG_ASSERT ((ID_DIM (arg_node_arg2) > 0),
                                         "Unknown dimension for 2nd arg of sel");

                            ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
                              = IntVec2Shpseg (1, 0, NULL,
                                               ACCESS_OFFSET (
                                                 INFO_WLAA_ACCESS (arg_info)));
                        } else if (ID_CONSTVEC (arg_node_arg1) != NULL) {
                            ACCESS_CLASS (INFO_WLAA_ACCESS (arg_info)) = ACL_const;

                            DBUG_ASSERT ((ID_VECLEN (arg_node_arg1) > SCALAR),
                                         "propagated constant vector is no array");

                            ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
                              = IntVec2Shpseg (1, ID_VECLEN (arg_node_arg1),
                                               ((int *)ID_CONSTVEC (arg_node_arg1)),
                                               ACCESS_OFFSET (
                                                 INFO_WLAA_ACCESS (arg_info)));
                        } else {
                            /*
                             * The first arg of sel is a variable. The offset of the
                             * access have to be infered later.
                             */
                        }
                    } else {
                        INFO_WLAA_ACCESS (arg_info)
                          = MakeAccess (ID_VARDEC (arg_node_arg2), NULL, ACL_const, NULL,
                                        ADIR_read, INFO_WLAA_ACCESS (arg_info));
                        ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
                          = IntVec2Shpseg (1, ARRAY_VECLEN (arg_node_arg1),
                                           ((int *)ARRAY_CONSTVEC (arg_node_arg1)),
                                           ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info)));
                    }
                    INFO_WLAA_COUNT (arg_info)++;

                } else {
                    DBUG_PRINT ("WLAA_INFO",
                                ("primitive function sel with array return value"));
                    INFO_WLAA_FEATURE (arg_info)
                      = INFO_WLAA_FEATURE (arg_info) | FEATURE_ASEL;
                }
                break;

            case F_take:
                DBUG_PRINT ("WLAA_INFO", ("primitive function F_take"));
                INFO_WLAA_FEATURE (arg_info)
                  = INFO_WLAA_FEATURE (arg_info) | FEATURE_TAKE;
                break;

            case F_drop:
                DBUG_PRINT ("WLAA_INFO", ("primitive function F_drop"));
                INFO_WLAA_FEATURE (arg_info)
                  = INFO_WLAA_FEATURE (arg_info) | FEATURE_DROP;
                break;

            case F_cat:
                DBUG_PRINT ("WLAA_INFO", ("primitive function F_cat"));
                INFO_WLAA_FEATURE (arg_info) = INFO_WLAA_FEATURE (arg_info) | FEATURE_CAT;
                break;

            case F_rotate:
                DBUG_PRINT ("WLAA_INFO", ("primitive function F_rotate"));
                INFO_WLAA_FEATURE (arg_info) = INFO_WLAA_FEATURE (arg_info) | FEATURE_ROT;
                break;

            case F_modarray:
                DBUG_PRINT ("WLAA_INFO", ("primitive function F_modarray"));
                INFO_WLAA_FEATURE (arg_info)
                  = INFO_WLAA_FEATURE (arg_info) | FEATURE_MODA;
                break;

            case F_add_SxA:
                DBUG_PRINT ("WLAA_INFO", ("primitive function F_add_SxA"));
                access = SearchAccess (INFO_WLAA_ACCESS (arg_info), arg_info);
                if (access != NULL) {
                    if (NODE_TYPE (arg_node_arg1) == N_num) {
                        if (ID_VARDEC (arg_node_arg2) == INFO_WLAA_INDEXVAR (arg_info)) {
                            ACCESS_CLASS (access) = ACL_offset;
                            offset = Malloc (VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access))
                                             * sizeof (int));
                            for (i = 0; i < VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access));
                                 i++) {
                                offset[i] = NUM_VAL (arg_node_arg1);
                            }
                            ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
                              = IntVec2Shpseg (1,
                                               VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access)),
                                               offset,
                                               ACCESS_OFFSET (
                                                 INFO_WLAA_ACCESS (arg_info)));
                            offset = Free (offset);
                            ACCESS_IV (INFO_WLAA_ACCESS (arg_info))
                              = INFO_WLAA_INDEXVAR (arg_info);
                        } else if (ID_CONSTVEC (arg_node_arg2) != NULL) {
                            ACCESS_CLASS (access) = ACL_const;
                            ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
                              = IntVec2Shpseg (1, ID_VECLEN (arg_node_arg2),
                                               ((int *)ID_CONSTVEC (arg_node_arg2)),
                                               ACCESS_OFFSET (
                                                 INFO_WLAA_ACCESS (arg_info)));
                        } else { /* arg2 is neither the indexvar nor a constant */
                            ACCESS_CLASS (access) = ACL_irregular;
                        }
                    } else { /* NODE_TYPE(arg_node_arg1) == N_id */
                        ACCESS_CLASS (access) = ACL_irregular;
                    }
                } else { /* access == NULL */
                    if (!IsIndexVect (IDS_TYPE (INFO_WLAA_LASTLETIDS (arg_info)))) {
                        DBUG_PRINT ("WLAA_INFO",
                                    ("primitive arithmetic operation on arrays "
                                     "(not index vector access)"));
                        INFO_WLAA_FEATURE (arg_info)
                          = INFO_WLAA_FEATURE (arg_info) | FEATURE_AARI;
                    }
                }
                break;

            case F_add_AxS:
                DBUG_PRINT ("WLAA_INFO", ("primitive function F_add_AxS"));
                access = SearchAccess (INFO_WLAA_ACCESS (arg_info), arg_info);
                if (access != NULL) {
                    if (NODE_TYPE (arg_node_arg2) == N_num) {
                        if (ID_VARDEC (arg_node_arg1) == INFO_WLAA_INDEXVAR (arg_info)) {
                            ACCESS_CLASS (access) = ACL_offset;
                            offset = Malloc (VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access))
                                             * sizeof (int));
                            for (i = 0; i < VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access));
                                 i++) {
                                offset[i] = NUM_VAL (arg_node_arg2);
                            }
                            ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
                              = IntVec2Shpseg (1,
                                               VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access)),
                                               offset,
                                               ACCESS_OFFSET (
                                                 INFO_WLAA_ACCESS (arg_info)));
                            ACCESS_IV (INFO_WLAA_ACCESS (arg_info))
                              = INFO_WLAA_INDEXVAR (arg_info);
                            offset = Free (offset);
                        } else if (ID_CONSTVEC (arg_node_arg1) != NULL) {
                            ACCESS_CLASS (access) = ACL_const;
                            ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
                              = IntVec2Shpseg (1, ID_VECLEN (arg_node_arg1),
                                               ((int *)ID_CONSTVEC (arg_node_arg1)),
                                               ACCESS_OFFSET (
                                                 INFO_WLAA_ACCESS (arg_info)));
                        } else { /* arg1 is neither the indexvar nor a constant */
                            ACCESS_CLASS (access) = ACL_irregular;
                        }
                    } else { /* NODE_TYPE(arg_node_arg2) == N_id */
                        ACCESS_CLASS (access) = ACL_irregular;
                    }
                } else { /* access == NULL */
                    if (!IsIndexVect (IDS_TYPE (INFO_WLAA_LASTLETIDS (arg_info)))) {
                        DBUG_PRINT ("WLAA_INFO",
                                    ("primitive arithmetic operation on arrays "
                                     "(not index vector access)"));
                        INFO_WLAA_FEATURE (arg_info)
                          = INFO_WLAA_FEATURE (arg_info) | FEATURE_AARI;
                    }
                }
                break;

            case F_add_AxA:
                DBUG_PRINT ("WLAA_INFO", ("primitive function F_add_AxA"));

                if ((NODE_TYPE (arg_node_arg1) != N_id)
                    || (NODE_TYPE (arg_node_arg2) != N_id)) {
                    SYSWARN (("WLAA only works correctly with constant folding,"
                              " N_id exspected !"));
                } else {
                    access = SearchAccess (INFO_WLAA_ACCESS (arg_info), arg_info);

                    if (access == NULL) {
                        if (!IsIndexVect (IDS_TYPE (INFO_WLAA_LASTLETIDS (arg_info)))) {
                            DBUG_PRINT ("WLAA_INFO",
                                        ("primitive arithmetic operation on arrays "
                                         "(not index vector access)"));
                            INFO_WLAA_FEATURE (arg_info)
                              = INFO_WLAA_FEATURE (arg_info) | FEATURE_AARI;
                        }
                    } else { /* access != NULL */
                        while (access != NULL) {
                            if (ID_VARDEC (arg_node_arg1)
                                == INFO_WLAA_INDEXVAR (arg_info)) {
                                ACCESS_IV (access) = INFO_WLAA_INDEXVAR (arg_info);
                                if (ID_CONSTVEC (arg_node_arg2)
                                    != NULL) { /* arg2 is constant */
                                    ACCESS_CLASS (access) = ACL_offset;
                                    ACCESS_OFFSET (access)
                                      = AddIntVec2Shpseg (1, ID_VECLEN (arg_node_arg2),
                                                          ((int *)ID_CONSTVEC (
                                                            arg_node_arg2)),
                                                          ACCESS_OFFSET (access));
                                    /* offset = offset + arg2 */
                                } else { /* arg2 is not constant */
                                    ACCESS_CLASS (access) = ACL_irregular;
                                }
                            } else { /* arg1 is not the wl-indexvector */
                                if (ID_VARDEC (arg_node_arg2)
                                    == INFO_WLAA_INDEXVAR (arg_info)) {
                                    ACCESS_IV (access) = INFO_WLAA_INDEXVAR (arg_info);
                                    if (ID_CONSTVEC (arg_node_arg1) != NULL) {
                                        /* arg1 is constant */
                                        ACCESS_CLASS (access) = ACL_offset;
                                        ACCESS_OFFSET (access)
                                          = AddIntVec2Shpseg (1,
                                                              ID_VECLEN (arg_node_arg1),
                                                              ((int *)ID_CONSTVEC (
                                                                arg_node_arg1)),
                                                              ACCESS_OFFSET (access));
                                        /* offset = offset + arg1 */
                                    } else { /* arg1 is not constant */
                                        ACCESS_CLASS (access) = ACL_irregular;
                                    }
                                } else { /* None of the arguments is the index vector! */
#if 0                
                  if ((ID_VECLEN(arg_node_arg1) < 0)
                      && (ID_VECLEN(arg_node_arg2) < 0)) {
                    
                    SYSWARN(("WLAA only works correctly with constant"
                             " folding, variable exspected !"));
                  }
#endif
                                    if (ID_CONSTVEC (arg_node_arg1) != NULL) {
                                        /* arg1 is constant */
                                        ACCESS_OFFSET (access)
                                          = AddIntVec2Shpseg (1,
                                                              ID_VECLEN (arg_node_arg1),
                                                              ((int *)ID_CONSTVEC (
                                                                arg_node_arg1)),
                                                              ACCESS_OFFSET (access));
                                        /* offset = offset + arg1 */
                                        if (ID_CONSTVEC (arg_node_arg2) != NULL) {
                                            /* arg2 is constant */
                                            ACCESS_CLASS (access) = ACL_const;
                                            ACCESS_OFFSET (access)
                                              = AddIntVec2Shpseg (1,
                                                                  ID_VECLEN (
                                                                    arg_node_arg2),
                                                                  ((int *)ID_CONSTVEC (
                                                                    arg_node_arg2)),
                                                                  ACCESS_OFFSET (access));
                                            /* offset = offset + arg2 */
                                        } else {
                                            ACCESS_IV (access)
                                              = ID_VARDEC (arg_node_arg2);
                                        }
                                    } else { /* arg1 is not constant */
                                        if (ID_CONSTVEC (arg_node_arg2) != NULL) {
                                            /* arg2 is constant */
                                            ACCESS_IV (access)
                                              = ID_VARDEC (arg_node_arg1);
                                            ACCESS_OFFSET (access)
                                              = AddIntVec2Shpseg (1,
                                                                  ID_VECLEN (
                                                                    arg_node_arg2),
                                                                  ((int *)ID_CONSTVEC (
                                                                    arg_node_arg2)),
                                                                  ACCESS_OFFSET (access));
                                            /* offset = offset + arg2 */
                                        } else {
                                            ACCESS_CLASS (access) = ACL_irregular;
                                        }
                                    }
                                }
                            }
                            access = SearchAccess (ACCESS_NEXT (access), arg_info);
                        }
                    }
                }

                break;

            case F_sub_AxS:
                DBUG_PRINT ("WLAA_INFO", ("primitive function F_sub_AxS"));
                access = SearchAccess (INFO_WLAA_ACCESS (arg_info), arg_info);
                /* not used yet:
                if (access == NULL) {
                  access = SearchAccess(INFO_WLAA_TMPACCESS(arg_info), arg_info);
                  access_flag = TEMP_A;
                } */
                if (access != NULL) {
                    if (ID_VARDEC (arg_node_arg1) == INFO_WLAA_INDEXVAR (arg_info)) {
                        if (NODE_TYPE (arg_node_arg2) == N_num) {
                            ACCESS_CLASS (access) = ACL_offset;
                            offset = Malloc (VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access))
                                             * sizeof (int));
                            for (i = 0; i < VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access));
                                 i++) {
                                offset[i] = NUM_VAL (arg_node_arg2);
                            }
                            ACCESS_OFFSET (access)
                              = IntVec2Shpseg (-1, ID_VECLEN (arg_node_arg2), offset,
                                               ACCESS_OFFSET (access));
                            ACCESS_IV (access) = INFO_WLAA_INDEXVAR (arg_info);
                        } else {
                            ACCESS_CLASS (access) = ACL_irregular;
                        }
                    } else if (!IsIndexVect (
                                 IDS_TYPE (INFO_WLAA_LASTLETIDS (arg_info)))) {
                        DBUG_PRINT ("WLAA_INFO",
                                    ("primitive arithmetic operation on arrays "
                                     "(not index vector access)"));
                        INFO_WLAA_FEATURE (arg_info)
                          = INFO_WLAA_FEATURE (arg_info) | FEATURE_AARI;
                    }
                } else { /* access == NULL */
                    if (!IsIndexVect (IDS_TYPE (INFO_WLAA_LASTLETIDS (arg_info)))) {
                        DBUG_PRINT ("WLAA_INFO",
                                    ("primitive arithmetic operation on arrays "
                                     "(not index vector access)"));
                        INFO_WLAA_FEATURE (arg_info)
                          = INFO_WLAA_FEATURE (arg_info) | FEATURE_AARI;
                    }
                }
                break;

            case F_sub_AxA:
                DBUG_PRINT ("WLAA_INFO", ("primitive function F_sub_AxA"));

                if ((NODE_TYPE (arg_node_arg1) != N_id)
                    || (NODE_TYPE (arg_node_arg2) != N_id)) {
                    SYSWARN (("WLAA only works correctly with constant folding,"
                              " N_id exspected !"));
                } else {
                    access = SearchAccess (INFO_WLAA_ACCESS (arg_info), arg_info);

                    if (access == NULL) {
                        if (!IsIndexVect (IDS_TYPE (INFO_WLAA_LASTLETIDS (arg_info)))) {
                            DBUG_PRINT ("WLAA_INFO",
                                        ("primitive arithmetic operation on "
                                         "arrays (not index vector access)"));
                            INFO_WLAA_FEATURE (arg_info)
                              = INFO_WLAA_FEATURE (arg_info) | FEATURE_AARI;
                        }
                    } else { /* access != NULL */
                        while (access != NULL) {
                            if (ID_VARDEC (arg_node_arg1)
                                == INFO_WLAA_INDEXVAR (arg_info)) {
                                /*
                                 *  arg1 is the wl-indexvector
                                 */
                                ACCESS_IV (access) = INFO_WLAA_INDEXVAR (arg_info);
                                if (ID_CONSTVEC (arg_node_arg2) != NULL) {
                                    /* arg2 is constant */
                                    ACCESS_CLASS (access) = ACL_offset;
                                    ACCESS_OFFSET (access)
                                      = AddIntVec2Shpseg (-1, ID_VECLEN (arg_node_arg2),
                                                          ((int *)ID_CONSTVEC (
                                                            arg_node_arg2)),
                                                          ACCESS_OFFSET (access));
                                    /* offset = offset - arg2 */
                                } else { /* arg2 is not constant */
                                    ACCESS_CLASS (access) = ACL_irregular;
                                }
                            } else { /* arg1 is not the wl-indexvector */
                                if (ID_VARDEC (arg_node_arg2)
                                    == INFO_WLAA_INDEXVAR (arg_info)) {
                                    /*
                                     *  arg2 is the wl-indexvector
                                     */
                                    ACCESS_CLASS (access) = ACL_irregular;
                                } else { /* None of the arguments is the index vector! */
#if 0                
                  if ((ID_VECLEN(arg_node_arg1) < 0)
                      && (ID_VECLEN(arg_node_arg2) < 0)) {
                    
                    SYSWARN(("WLAA only works correctly with constant folding,"
                             " variable exspected !"));
                  }
#endif
                                    if (ID_CONSTVEC (arg_node_arg1) != NULL) {
                                        /* arg1 is constant */
                                        ACCESS_OFFSET (access)
                                          = AddIntVec2Shpseg (1,
                                                              ID_VECLEN (arg_node_arg1),
                                                              ((int *)ID_CONSTVEC (
                                                                arg_node_arg1)),
                                                              ACCESS_OFFSET (access));
                                        /* offset = offset + arg1 */
                                        if (ID_CONSTVEC (arg_node_arg2) != NULL) {
                                            /* arg2 is constant */
                                            ACCESS_CLASS (access) = ACL_const;
                                            ACCESS_OFFSET (access)
                                              = AddIntVec2Shpseg (-1,
                                                                  ID_VECLEN (
                                                                    arg_node_arg2),
                                                                  ((int *)ID_CONSTVEC (
                                                                    arg_node_arg2)),
                                                                  ACCESS_OFFSET (access));
                                            /* offset = offset - arg2 */
                                        } else {
                                            ACCESS_CLASS (access) = ACL_irregular;
                                        }
                                    } else { /* arg1 is not constant */
                                        if (ID_CONSTVEC (arg_node_arg2) != NULL) {
                                            /* arg2 is constant */
                                            ACCESS_IV (access)
                                              = ID_VARDEC (arg_node_arg1);
                                            ACCESS_OFFSET (access)
                                              = AddIntVec2Shpseg (-1,
                                                                  ID_VECLEN (
                                                                    arg_node_arg2),
                                                                  ((int *)ID_CONSTVEC (
                                                                    arg_node_arg2)),
                                                                  ACCESS_OFFSET (access));
                                            /* offset = offset - arg2 */
                                        } else {
                                            ACCESS_CLASS (access) = ACL_irregular;
                                        }
                                    }
                                }
                            }
                            access = SearchAccess (ACCESS_NEXT (access), arg_info);
                        }
                    }
                }
                break;

            case F_sub_SxA:
            case F_mul_SxA:
            case F_mul_AxS:
            case F_mul_AxA:
            case F_div_SxA:
            case F_div_AxS:
            case F_div_AxA:
                DBUG_PRINT ("WLAA_INFO", ("primitive function not inferable or unknown"));
                if (!IsIndexVect (IDS_TYPE (INFO_WLAA_LASTLETIDS (arg_info)))) {
                    INFO_WLAA_FEATURE (arg_info)
                      = INFO_WLAA_FEATURE (arg_info) | FEATURE_AARI;
                }
                break;

            case F_idx_sel:
            case F_idx_modarray:
                /*
                 * These functions are only introduced by index vector elimination,
                 * however tile size inference must always be applied before index
                 * vector elimination.
                 */
                DBUG_PRINT ("WLAA_INFO",
                            ("primitive function F_idx_sel | F_idx_modarray"));
                DBUG_ASSERT (1, "primitive function idx_sel or idx_modarray found "
                                "during tile size selection");
                break;

            default:
                DBUG_PRINT ("WLAA_INFO",
                            ("primitive function which do not deal with arrays"));
                /*
                 * Do nothing !
                 *
                 * All other primitive functions do not deal with arrays.
                 */
                break;
            }
        } else {
            DBUG_PRINT ("WLAA_INFO", ("none-relevant primitive function"));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLAAap(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_ap node.
 *
 *   This function detects function applications within with-loop operators
 *   and sets the features bit mask accordingly. However, functions which do
 *   not deal with arrays are no problem for tile size inference. We could
 *   investigate this by an inter-functional analysis. But for now, we only
 *   check whether one of the arguments or one of the return values is of an
 *   array type.
 *
 *****************************************************************************/

node *
WLAAap (node *arg_node, node *arg_info)
{
    ids *ret_var;

    DBUG_ENTER ("WLAAap");

    DBUG_PRINT ("WLAA", ("WLAAap"));

    if (INFO_WLAA_WLLEVEL (arg_info) > 0) {
        /*
         * Here, we are beyond a with-loop.
         */
        ret_var = INFO_WLAA_LASTLETIDS (arg_info);

        while (ret_var != NULL) {
            if (IDS_DIM (ret_var) != SCALAR) {
                INFO_WLAA_FEATURE (arg_info) = INFO_WLAA_FEATURE (arg_info) | FEATURE_AP;
                break;
            }
            ret_var = IDS_NEXT (ret_var);
        }

        if (!(INFO_WLAA_FEATURE (arg_info) & FEATURE_AP)) {
            /*
             * FEATURE_AP has not been set yet.
             */
            if (AP_ARGS (arg_node) != NULL) {
                INFO_WLAA_BELOWAP (arg_info) = 1;
                AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
                INFO_WLAA_BELOWAP (arg_info) = 0;
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLAAid(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_id node.
 *
 *   This function applies only beyond an N_ap node within a with-loop
 *   operator. If the function argument is not of scalar type the AP bit
 *   of the features bit mask is set.
 *
 *
 *****************************************************************************/

node *
WLAAid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLAAid");

    DBUG_PRINT ("WLAA", ("WLAAid"));

    if ((INFO_WLAA_WLLEVEL (arg_info) > 0) && (INFO_WLAA_BELOWAP (arg_info))) {
        if (ID_DIM (arg_node) != SCALAR) {
            INFO_WLAA_FEATURE (arg_info) = INFO_WLAA_FEATURE (arg_info) | FEATURE_AP;
        }
    }

    DBUG_RETURN (arg_node);
}
