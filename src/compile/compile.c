/*
 *
 * $Log$
 * Revision 3.55  2001/05/14 10:21:20  cg
 * Added facilities to collect maximum number of schedulers in single
 * SPMD functions.
 *
 * Revision 3.54  2001/05/11 14:41:35  cg
 * New ICMs for scheduler initialization are now inserted
 * when setting up environment for multithreaded execution.
 *
 * Revision 3.53  2001/05/11 14:36:56  cg
 * Scheduler ICMs now always get an additional generic argument
 * which specifies the scheduler ID for selecting an appropriate
 * set of local data structures where required by the scheduling
 * technique.
 *
 * Revision 3.52  2001/05/09 15:13:00  cg
 * All scheduling ICMs get an additional first parameter,
 * i.e. the segment ID. This is required to identify the appropriate
 * set of scheduler internal variables.
 *
 * Revision 3.51  2001/05/08 13:28:04  dkr
 * new RC macros used
 *
 * Revision 3.50  2001/05/07 16:45:06  dkr
 * COMPIdLet: DBUG_ASSERT message modified
 *
 * Revision 3.49  2001/04/27 17:33:00  nmw
 * refcounting for special fundef (non recursive) AP icms added
 *
 * Revision 3.48  2001/04/26 21:07:54  dkr
 * fixed a bug in COMPPrfIdxPsi: DBUG_ASSERT too regide
 *
 * Revision 3.47  2001/04/26 17:10:08  dkr
 * COMPCast reactivated
 *
 * Revision 3.46  2001/04/26 12:28:25  dkr
 * GetExprsLength() renamed into CountExprs()
 *
 * Revision 3.45  2001/04/26 11:56:25  nmw
 * CreateIcmND_FUN_AP() sets ICM_FUNDEF attribute
 *
 * Revision 3.44  2001/04/25 13:55:58  dkr
 * COMPFundef: ST_zombiefun added
 *
 * Revision 3.43  2001/04/24 09:39:50  dkr
 * CHECK_NULL renamed into STR_OR_EMPTY
 *
 * Revision 3.42  2001/04/06 17:17:15  dkr
 * CheckAp() removed by preprocessor if DBUG_OFF is set
 *
 * Revision 3.41  2001/04/06 16:58:29  dkr
 * CreateFundefIcm: no warning about uninitialized variable anymore
 *
 * Revision 3.40  2001/04/05 14:52:36  dkr
 * COMPAp() brushed
 *
 * Revision 3.39  2001/04/05 01:45:41  dkr
 * bug in COMPAp fixed:
 * mapping of a return value and an argument to the same argument position
 * works correctly now
 *
 * Revision 3.38  2001/04/04 23:08:41  dkr
 * - COMPFundef brushed
 * - special fold-funs are always compiled first now :-)
 *
 * Revision 3.37  2001/04/04 00:09:24  dkr
 * fixed a bug in COMPSync: fold type tag is generated correctly now even
 * for user defined types.
 *
 * Revision 3.36  2001/04/03 22:43:19  dkr
 * GetFoldCode: DBUG_ASSERT condition corrected
 *
 * Revision 3.35  2001/04/03 22:30:21  dkr
 * signature for MT_ADJUST_SCHEDULER modified
 *
 * Revision 3.34  2001/04/03 09:31:20  dkr
 * bug in InsertIcm_WL_SET_OFFSET() fixed
 *
 * Revision 3.33  2001/04/02 11:43:56  dkr
 * include of wl_bounds.h added
 *
 * Revision 3.32  2001/03/29 14:48:16  dkr
 * SYNC_SCHEDULING removed
 *
 * Revision 3.31  2001/03/29 01:36:00  dkr
 * WLSEGVAR_IDX_MIN, WLSEGVAR_IDX_MAX are now node-vectors
 *
 * Revision 3.30  2001/03/27 10:53:02  dkr
 * minor changes done
 *
 * Revision 3.29  2001/03/22 19:15:58  dkr
 * include of tree.h elimiated
 *
 * Revision 3.28  2001/03/15 21:25:35  dkr
 * signature of NodeOr..._MakeIndex() modified
 *
 * Revision 3.27  2001/03/15 11:59:42  dkr
 * ST_inout replaced by ST_reference
 *
 * Revision 3.26  2001/03/14 16:25:16  dkr
 * signature (parameter types) of icm MT_ADJUST_SCHEDULER modified
 *
 * Revision 3.25  2001/03/14 15:55:04  dkr
 * ups, typo in InsertIcm_MT_ADJUST_SCHEDULER corrected ...
 *
 * Revision 3.24  2001/03/14 15:38:08  ben
 * actual parameters for MT_ADJUST_SCHEDULER icm corrected
 *
 * Revision 3.23  2001/03/05 16:42:11  dkr
 * no macros NWITH???_IS_FOLD used
 *
 * Revision 3.22  2001/03/02 16:11:27  dkr
 * some comments and DBUG_ASSERTs added
 *
 * Revision 3.21  2001/02/22 12:51:19  nmw
 * check for empty assignment chain in blocks after removing
 * artificial variables in precompile
 *
 * Revision 3.20  2001/02/09 13:34:09  dkr
 * COMPIcm added in order to RC the arguments of ICMs correctly
 *
 * Revision 3.19  2001/02/06 01:48:45  dkr
 * with-loop: NOOP nodes added
 *
 * Revision 3.18  2001/02/02 09:42:14  dkr
 * COMPWLgrid, COMPWLgridVar, COMPWLstride, COMPWLstrideVar, COMPWLseg,
 * COMPWLsegVar removed.
 *
 * Revision 3.17  2001/01/30 16:41:25  dkr
 * rcs header compressed
 *
 * Revision 3.16  2001/01/30 12:22:28  dkr
 * signature of ICMs WL_NOOP, WL_NOOP__OFFSET modified
 *
 * Revision 3.15  2001/01/29 18:34:58  dkr
 * some superfluous attributes of N_WLsegVar removed
 *
 * Revision 3.14  2001/01/24 23:46:37  dkr
 * WLGRIDX_FITTED used
 * ICMs SAC_WL_GRIDVAR_... renamed into SAC_WL_GRID_FIT_...
 *
 * Revision 3.13  2001/01/22 15:55:47  dkr
 * bug in COMPWLgridx fixed
 *
 * Revision 3.12  2001/01/22 13:46:24  dkr
 * signature of MT_ADJUST_SCHEDULER modified
 *
 * Revision 3.11  2001/01/19 11:58:12  dkr
 * some with-loop ICMs renamed
 *
 * Revision 3.9  2001/01/10 14:28:04  dkr
 * fixed a bug with multiple segments
 *
 * Revision 3.8  2001/01/09 19:59:18  dkr
 * code for WL-nodes brushed
 * support for naive compilation extended (not finished yet)
 *
 * Revision 3.7  2001/01/08 21:59:03  dkr
 * support for naive compilation of with-loops added (not finished yet)
 *
 * Revision 3.6  2000/12/29 14:24:41  cg
 * When compiling for multithreaded execution, any function gets one additional
 * parameter to hold the thread ID, which is needed for heap management.
 * Thread-specific data is only used by malloc().
 *
 * Revision 3.5  2000/12/12 15:32:28  dkr
 * functions Make...RcIcms() are not static anymore
 *
 * Revision 3.4  2000/12/06 19:24:45  dkr
 * minor changes done
 *
 * Revision 3.3  2000/11/29 13:57:15  dkr
 * no '... might be used uninitialized' warnings anymore
 *
 * Revision 3.2  2000/11/27 21:07:35  cg
 * Added generation of new ICM ND_ALLOC_ARRAY_PLACE.
 *
 * Revision 3.1  2000/11/20 18:01:07  sacbase
 * new release made
 *
 * [ eliminated ]
 *
 */

#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "prf.h"

#include "dbug.h"
#include "my_debug.h"
#include "Error.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "convert.h"
#include "print.h"
#include "DataFlowMask.h"
#include "ReuseWithArrays.h"
#include "typecheck.h"
#include "scheduling.h"
#include "wl_bounds.h"
#include "refcount.h"
#include "compile.h"

/*
 * arg_info
 * ========
 *
 * ICM/type tables:
 *
 *   Let m be the number of return values and n the number of arguments.
 *
 *   INFO_COMP_TABSIZE  : table size (n+m+2)
 *   INFO_COMP_ICMTAB   : ICM table
 *   INFO_COMP_TYPETAB  : type table
 *
 *   INFO_COMP_CNTPARAM : current argument index ( == (table index) - 2)
 *                          0 <= INFO_COMP_CNTPARAM < m    for return values
 *                          m <= INFO_COMP_CNTPARAM < m+n  for arguments
 *
 *   INFO_COMP_MERGE    : [written by ???, read by COMPAp()]
 *     If a return value and an argument are mapped to the same argument
 *     position, possibly a merge assignment is needed. It can be found here.
 *
 *   Table layout:
 *     [0]                 : T_dots (...) argument
 *     [1]                 : return value
 *     [i+2], 0 <= i < m   : out arguments
 *     [j+2], m <= j < m+n : in arguments
 *
 * Other:
 *
 *   INFO_COMP_MODUL       : pointer to current modul
 *   INFO_COMP_FUNDEF      : pointer to current fundef
 *
 *   INFO_COMP_FIRSTASSIGN : pointer to first assignment of current fundef
 *   INFO_COMP_LASTIDS     : pointer to IDS of current let
 *   INFO_COMP_LASTSYNC    : pointer to ... ???
 *
 *   INFO_COMP_FOLDFUNS    : [flag]
 *     In order to guarantee that the special fold-funs are compiled *before*
 *     the associated with-loops, the fundef chain is traversed twice.
 *     During the first traversal (FOLDFUNS == TRUE) only special fold-funs
 *     are traversed. During the second traversal (FOLDFUNS == FALSE) only
 *     the other functions are traversed.
 */

/******************************************************************************
 *
 * global variable:  int barrier_id
 *
 * Description:
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

static ids *wlids = NULL;
static node *wlnode = NULL;
static node *wlseg = NULL;
static node *wlstride = NULL;

/* postfix for goto labels */
#define LABEL_POSTFIX "SAC__label"

/*
 * This macro indicates whether there are multiple segments present or not.
 */
#define MULTIPLE_SEGS(seg) ((seg != NULL) && (WLSEGX_NEXT (seg) != NULL))

/******************************************************************************
 *
 * Function:
 *   char *GetBasetypeStr( types *type)
 *
 * Description:
 *   Returns the basetype string of the given type, i.e. "TYPES_NAME" if type
 *   represents a user-defined type and "TYPES_BASETYPE" otherwise.
 *
 ******************************************************************************/

static char *
GetBasetypeStr (types *type)
{
    simpletype basetype;
    char *str;

    DBUG_ENTER ("GetBasetypeStr");

    basetype = GetBasetype (type);

    if (basetype == T_user) {
        str = TYPES_NAME (type);
        DBUG_ASSERT ((str != NULL), "Name of user-defined type not found");
    } else {
        str = type_string[basetype];
    }

    DBUG_RETURN (str);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeTypeNode( types *type)
 *
 * Description:
 *   Creates a new N_id node containing the implementation type string of
 *   the given type, i.e. "basetype" if type represents a scalar and
 *   "basetype*" if type represents an array.
 *
 ******************************************************************************/

static node *
MakeTypeNode (types *type)
{
    node *ret_node;
    char *str, *ret;

    DBUG_ENTER ("MakeTypeNode");

    str = GetBasetypeStr (type);

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
 *   Creates a new N_id node containing the basetype string of the given type.
 *
 ******************************************************************************/

static node *
MakeBasetypeNode (types *type)
{
    node *ret_node;
    char *str;

    DBUG_ENTER ("MakeBasetypeNode");

    str = GetBasetypeStr (type);

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

char *
GenericFun (int which, types *type)
{
    node *tdef;
    char *ret = NULL;

    DBUG_ENTER ("GenericFun");

    DBUG_PRINT ("COMP", ("Looking for generic fun %d (0==copy/1==free)"));

    if (TYPES_BASETYPE (type) == T_user) {
        tdef = TYPES_TDEF (type);
        DBUG_ASSERT ((tdef != NULL), "Failed attempt to look up typedef");

        DBUG_ASSERT ((TYPEDEF_BASETYPE (tdef) != T_user),
                     "unresolved nested user-defined type found");

        if (TYPEDEF_BASETYPE (tdef) == T_hidden) {
            switch (which) {
            case 0:
                ret = TYPEDEF_COPYFUN (tdef);
                break;
            case 1:
                ret = TYPEDEF_FREEFUN (tdef);
                break;
            default:
                DBUG_ASSERT ((0), "Unknown kind if generic function requested");
                break;
            }
        }
    }

    DBUG_PRINT ("COMP", ("Found generic fun %s", ret));

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   node *GetFoldCode( node *fundef)
 *
 * Description:
 *   Returns the foldop-code of the special fold-fun 'fundef'.
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
    node *fold_code;
    node *tmp;

    DBUG_ENTER ("GetFoldCode");

    DBUG_ASSERT ((fundef != NULL), "no special fold-fun found!");
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "corrupted special fold-fun found!");

    /*
     * get code of the special fold-fun
     */
    fold_code = DupTree (FUNDEF_INSTR (fundef));

    /*
     * remove declaration-ICMs ('ND_KS_DECL_ARRAY_ARG') from code.
     */
    while ((NODE_TYPE (ASSIGN_INSTR (fold_code)) == N_icm)
           && (!strcmp (ICM_NAME (ASSIGN_INSTR (fold_code)), "ND_KS_DECL_ARRAY_ARG"))) {
        fold_code = FreeNode (fold_code);
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
                  && (!strcmp (ICM_NAME (ASSIGN_INSTR (ASSIGN_NEXT (tmp))),
                               "ND_FUN_RET"))),
                 "no ND_FUN_RET icm found in fold code!");
    ASSIGN_NEXT (tmp) = FreeNode (ASSIGN_NEXT (tmp));

    DBUG_RETURN (fold_code);
}

/******************************************************************************
 *
 * Function:
 *   node *GetFoldVardecs( node *fundef)
 *
 * Description:
 *   returns the vardecs of the special fold-fun 'fundef'.
 *
 ******************************************************************************/

node *
GetFoldVardecs (node *fundef)
{
    node *fold_vardecs;

    DBUG_ENTER ("GetFoldVardecs");

    DBUG_ASSERT ((fundef != NULL), "no special fold-fun found!");
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "corrupted special fold-fun found!");

    DBUG_ASSERT ((FUNDEF_BODY (fundef) != NULL),
                 "special fold-fun with empty body found!");

    /*
     * get vardecs of the special fold-fun
     */
    fold_vardecs = DupTree (FUNDEF_VARDEC (fundef));

    DBUG_RETURN (fold_vardecs);
}

/******************************************************************************
 *
 * Function:
 *   ids *GetIndexIds( ids *index_ids, int dim)
 *
 * Description:
 *   returns the index-ids for dimension 'dim' found in 'index_ids'.
 *   'index_ids' is a vector of index-ids (e.g. NWITHID_IDS(...)) containing
 *   at least 'dim' elements.
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
 * Function:
 *   node *MakeIncRcIcm( char *name, types *type, int rc, int num)
 *
 * Description:
 *   Builds a ND_INC_RC( name, num) icm if (rc >= 0) and (num > 0).
 *
 ******************************************************************************/

node *
MakeIncRcIcm (char *name, types *type, int rc, int num)
{
    node *icm = NULL;

    DBUG_ENTER ("MakeIncRcIcm");

    DBUG_ASSERT ((RC_IS_LEGAL (rc)), "illegal RC value found!");

    if (rc >= 0) { /* is it a refcounted data? */
        DBUG_ASSERT ((num >= 0), "increment for rc must be >= 0.");

        if (num > 0) {
            icm = MakeAssignIcm2 ("ND_INC_RC", MakeId_Copy (name), MakeNum (num));
        }
    }

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeDecRcIcm( char *name, types* type, int rc, int num)
 *
 * Description:
 *   According to 'type', 'rc' and 'num', builds either
 *             a ND_DEC_RC( name, num) icm,
 *          or a ND_DEC_RC_FREE_HIDDEN( name, num, freefun) icm,
 *          or a ND_DEC_RC_FREE_ARRAY( name, num) icm,
 *          or no icm at all.
 *
 ******************************************************************************/

node *
MakeDecRcIcm (char *name, types *type, int rc, int num)
{
    node *icm = NULL;

    DBUG_ENTER ("MakeDecRcIcm");

    DBUG_ASSERT ((RC_IS_LEGAL (rc)), "illegal RC value found!");

    if (rc >= 0) { /* is it a refcounted data? */
        DBUG_ASSERT ((num >= 0), "decrement for rc must be >= 0.");

        if (num > 0) {
            if (rc - num > 0) { /* definitely no FREE needed? */
                icm = MakeAssignIcm2 ("ND_DEC_RC", MakeId_Copy (name), MakeNum (num));
            } else {
                if (IsNonUniqueHidden (type)) {
                    icm = MakeAssignIcm3 ("ND_DEC_RC_FREE_HIDDEN", MakeId_Copy (name),
                                          MakeNum (num),
                                          MakeId_Copy (GenericFun (1, type)));
                } else {
                    icm = MakeAssignIcm2 ("ND_DEC_RC_FREE_ARRAY", MakeId_Copy (name),
                                          MakeNum (num));
                }
            }
        }
    }

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeAdjustRcIcm( char *name, types* type, int rc, int num)
 *
 * Description:
 *   According to num, either a ND_INC_RC( varname, num) icm,
 *                       or   no ICM at all,
 *                       or   a ND_DEC_RC_...( varname, -num) icm
 *   is created.
 *
 ******************************************************************************/

node *
MakeAdjustRcIcm (char *name, types *type, int rc, int num)
{
    node *icm = NULL;

    DBUG_ENTER ("MakeAdjustRcIcm");

    DBUG_ASSERT ((RC_IS_LEGAL (rc)), "illegal RC value found!");

    if (num > 0) {
        icm = MakeIncRcIcm (name, type, rc, num);
    } else if (num < 0) {
        icm = MakeDecRcIcm (name, type, rc, -num);
    }

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * Function:
 *   node *Ids2AllocArrayICMs( ids *ids_chain, node *next)
 *
 * Description:
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
        DBUG_ASSERT ((RC_IS_LEGAL (IDS_REFCNT (ids_chain))), "illegal RC value found!");

        if (RC_IS_ACTIVE (IDS_REFCNT (ids_chain))) {
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
 * Function:
 *   node *Ids2AllocArrayICMs_reuse( ids *ids_chain, node *next)
 *
 * Description:
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
Ids2AllocArrayICMs_reuse (ids *ids_chain, node *next, node *pragma)
{
    node *alloc_icm;
    node *assign;
    node *last_assign = NULL;
    node *assigns = NULL;

    DBUG_ENTER ("Ids2AllocArrayICMs_reuse");

    while (ids_chain != NULL) {
        DBUG_ASSERT ((RC_IS_LEGAL (IDS_REFCNT (ids_chain))), "illegal RC value found!");

        if (RC_IS_ACTIVE (IDS_REFCNT (ids_chain))) {
            if (pragma == NULL) {
                alloc_icm
                  = MakeIcm3 ("ND_ALLOC_ARRAY", MakeBasetypeNode (IDS_TYPE (ids_chain)),
                              DupIds_Id (ids_chain), MakeNum (0));
            } else {
                alloc_icm = MakeIcm6 ("ND_ALLOC_ARRAY_PLACE",
                                      MakeBasetypeNode (IDS_TYPE (ids_chain)),
                                      DupIds_Id (ids_chain), MakeNum (0),
                                      DupNode (AP_ARG1 (PRAGMA_APL (pragma))),
                                      DupNode (AP_ARG2 (PRAGMA_APL (pragma))),
                                      DupNode (AP_ARG3 (PRAGMA_APL (pragma))));
            }

            assign
              = MakeAssign (alloc_icm, MakeAssignIcm2 ("ND_INC_RC", DupIds_Id (ids_chain),
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
 * Function:
 *   node *Ids2IncRcICMs( ids *ids_chain, node *next)
 *
 * Description:
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
 * Function:
 *   node *Ids2DecRcICMs( ids *ids_chain, node *next)
 *
 * Description:
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
 * Function:
 *   node *AdjustAddedAssigns( node *after_ass, node *before_ass)
 *
 * Description:
 *   Removes icms from 'after' that are obsolete because of the merging
 *   assignments/icms found in 'before':
 *
 *      r = fun( a);         ->          // 'before' assignments
 *                         (comp)        ND_FUN_AP( fun, r, a)
 *                                       // 'after' assignments
 *
 *   Example:
 *
 *      before:  ND_KS_MAKE_UNIQUE_ARRAY( a, b, 4)  // b = a;
 *
 *      after:   SAC_ND_ALLOC_RC( b)                   <-- obsolete
 *               SAC_ND_SET_RC( b, 1)
 *               SAC_ND_DEC_RC_FREE_ARRAY( a, 1)       <-- obsolete
 *
 ******************************************************************************/

static node *
AdjustAddedAssigns (node *after_ass, node *before_ass)
{
    node *after, *before;

    DBUG_ENTER ("AdjustAddedAssigns");

    /* create dummy head */
    after_ass = MakeAssign (NULL, after_ass);

    while (before_ass != NULL) {
        before = ASSIGN_INSTR (before_ass);

        if (NODE_TYPE (before) == N_icm) {
            if ((!strcmp (ICM_NAME (before), "ND_KS_MAKE_UNIQUE_ARRAY"))
                || (!strcmp (ICM_NAME (before), "ND_MAKE_UNIQUE_HIDDEN"))) {
                node *curr_after_ass = ASSIGN_NEXT (after_ass);
                node *last_after_ass = after_ass;
                char *old_name = ID_NAME (ICM_ARG1 (before));
                char *new_name = ID_NAME (ICM_ARG2 (before));

                while (curr_after_ass != NULL) {
                    after = ASSIGN_INSTR (curr_after_ass);

                    if (NODE_TYPE (after) == N_icm) {
                        char *after_name = ICM_NAME (after);

                        if (((!strcmp (after_name, "ND_DEC_RC_FREE_HIDDEN"))
                             && (!strcmp (ID_NAME (ICM_ARG1 (after)), old_name)))
                            || ((!strcmp (after_name, "ND_DEC_RC_FREE_ARRAY"))
                                && (!strcmp (ID_NAME (ICM_ARG1 (after)), old_name)))
                            || ((!strcmp (after_name, "ND_ALLOC_RC"))
                                && (!strcmp (ID_NAME (ICM_ARG1 (after)), new_name)))) {
                            ASSIGN_NEXT (last_after_ass)
                              = FreeNode (ASSIGN_NEXT (last_after_ass));
                        } else {
                            if ((!strcmp (after_name, "ND_NO_RC_FREE_ARRAY"))
                                && (!strcmp (ID_NAME (ICM_ARG1 (after)), new_name))) {
                                /*
                                 * icm names are static!
                                 *
                                 * FREE( ICM_NAME( before));
                                 */
                                ICM_NAME (before) = "ND_KS_NO_RC_MAKE_UNIQUE_ARRAY";
                            } else if ((!strcmp (after_name, "ND_NO_RC_FREE_HIDDEN"))
                                       && (!strcmp (ID_NAME (ICM_ARG1 (after)),
                                                    new_name))) {
                                /*
                                 * icm names are static!
                                 *
                                 * FREE( ICM_NAME( before));
                                 */
                                ICM_NAME (before) = "ND_NO_RC_MAKE_UNIQUE_HIDDEN";
                            } else if (((!strcmp (after_name, "ND_NO_RC_ASSIGN_HIDDEN"))
                                        || (!strcmp (after_name,
                                                     "ND_KS_NO_RC_ASSIGN_ARRAY")))
                                       && (!strcmp (ID_NAME (ICM_ARG1 (after)), new_name))
                                       && (!strcmp (ID_NAME (ICM_ARG2 (after)),
                                                    old_name))) {
                                FREE (ID_NAME (ICM_ARG2 (before)));
                                ID_NAME (ICM_ARG2 (before)) = StringCopy (old_name);
                            }
                            last_after_ass = curr_after_ass;
                        }
                    } else {
                        last_after_ass = curr_after_ass;
                    }

                    curr_after_ass = ASSIGN_NEXT (last_after_ass);
                }
            }
        }

        before_ass = ASSIGN_NEXT (before_ass);
    }

    /* remove dummy head */
    after_ass = FreeNode (after_ass);

    DBUG_RETURN (after_ass);
}

/******************************************************************************
 *
 * Function:
 *   node *MergeIcmsAp( node *out_icm, node *in_icm,
 *                      types *type, int rc,
 *                      int line)
 *
 * Description:
 *   'out_icm': icm for out-parameter which is already situated in the table.
 *   'in_icm' : icm for in-parameter which was to be added to the table when
 *              the mapping was detected.
 *   'type'   : type of parameter
 *   'rc'     : refcount of in-parameter
 *   Returns a N_assign node containing ICM code needed to merge the given
 *   parameters.
 *
 * Example:
 *   signature of external C-function:  cfun( type *a);
 *     (note, that function 'cfun' does no refcounting on 'a' !!!)
 *   functional representation in SAC:  type cfun( type a);
 *   function appl. within SAC code:    b = cfun( a);
 *   code generated for this appl.:     b = a;  cfun( b);
 *                                      ^^^^^^
 *   The ICM code for the additional assignment marked with ^^^'s is returned
 *   by this function.
 *
 ******************************************************************************/

static node *
MergeIcmsAp (node *out_icm, node *in_icm, types *type, int rc, int line)
{
    node *out_id, *in_id;
    node *new_assign = NULL;

    DBUG_ENTER ("MergeIcmsAp");

    DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (out_icm)) == N_id), "no out-tag found!");
    DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (in_icm)) == N_id), "no in-tag found!");

    if ((strcmp ("out", ID_NAME (EXPRS_EXPR (out_icm))))
        || (strcmp ("in", ID_NAME (EXPRS_EXPR (in_icm))))) {
        ERROR (line, ("Pragma 'linksign' illegal"));
        CONT_ERROR (("Mappings allowed exclusively between one parameter"
                     " and one return value on which both the function"
                     " does no refcounting !"));
        ABORT_ON_ERROR;
    }

    out_id = EXPRS_EXPR (EXPRS_NEXT (out_icm));
    in_id = EXPRS_EXPR (EXPRS_NEXT (in_icm));

    DBUG_ASSERT ((NODE_TYPE (out_id) == N_id),
                 "out-argument of application must be a N_id node");

    if (IsBoxed (type)) {
        FreeTree (EXPRS_EXPR (out_icm));
        EXPRS_EXPR (out_icm) = MakeId_Copy ("upd_bx");

        DBUG_ASSERT ((NODE_TYPE (in_id) == N_id),
                     "boxed in-argument of application must be a N_id node");

        if (IsArray (type)) {
            if (IsUnique (type)) {
                DBUG_PRINT ("COMP", ("Merging ICM-args: unique array %s - %s",
                                     ID_NAME (in_id), ID_NAME (out_id)));

                if (strcmp (ID_NAME (in_id), ID_NAME (out_id))) {
                    new_assign = MakeAssignIcm2 ("ND_KS_NO_RC_ASSIGN_ARRAY",
                                                 DupNode (in_id), DupNode (out_id));
                }
            } else if (rc == 1) {
                DBUG_PRINT ("COMP", ("Merging ICM-args non-unique array with rc==1"
                                     " %s - %s",
                                     ID_NAME (in_id), ID_NAME (out_id)));

                new_assign
                  = MakeAssignIcm3 ("ND_KS_MAKE_UNIQUE_ARRAY", DupNode (in_id),
                                    DupNode (out_id), MakeNum (GetBasetypeSize (type)));
            } else {
                DBUG_PRINT ("COMP", ("Merging ICM-args non-unique array with rc>1"
                                     " %s - %s",
                                     ID_NAME (in_id), ID_NAME (out_id)));

                new_assign
                  = MakeAssignIcm3 ("ND_KS_COPY_ARRAY", DupNode (in_id), DupNode (out_id),
                                    MakeNum (GetBasetypeSize (type)));
            }
        } else if (IsUnique (type)) {
            DBUG_PRINT ("COMP", ("Merging ICM-args unique hidden %s - %s",
                                 ID_NAME (in_id), ID_NAME (out_id)));

            if (strcmp (ID_NAME (in_id), ID_NAME (out_id))) {
                new_assign = MakeAssignIcm2 ("ND_NO_RC_ASSIGN_HIDDEN", DupNode (in_id),
                                             DupNode (out_id));
            }
        } else if (rc == 1) {
            DBUG_PRINT ("COMP", ("Merging ICM-args non-unique hidden %s - %s"
                                 " with rc==1",
                                 ID_NAME (in_id), ID_NAME (out_id)));

            new_assign
              = MakeAssignIcm3 ("ND_MAKE_UNIQUE_HIDDEN", DupNode (in_id),
                                DupNode (out_id), MakeId_Copy (GenericFun (0, type)));
        } else {
            DBUG_PRINT ("COMP", ("Merging ICM-args non-unique hidden %s - %s"
                                 " with rc>1",
                                 ID_NAME (in_id), ID_NAME (out_id)));

            new_assign
              = MakeAssignIcm3 ("ND_KS_COPY_HIDDEN", DupNode (in_id), DupNode (out_id),
                                MakeId_Copy (GenericFun (0, type)));
        }
    } else {
        FreeTree (EXPRS_EXPR (out_icm));
        EXPRS_EXPR (out_icm) = MakeId_Copy ("upd");

        DBUG_PRINT ("COMP", ("Merging ICM-args unboxed %s", ID_NAME (out_id)));

        if ((NODE_TYPE (in_id) != N_id) || strcmp (ID_NAME (in_id), ID_NAME (out_id))) {
            new_assign = MakeAssign (MakeLet (DupNode (in_id), DupId_Ids (out_id)), NULL);
        }
    }

    DBUG_PRINT ("COMP", ("Merging icm args of ND_FUN_AP, new tag=\"%s\"",
                         ID_NAME (EXPRS_EXPR (out_icm))));

    DBUG_RETURN (new_assign);
}

/******************************************************************************
 *
 * Function:
 *   void MergeIcmsFundef( node *out_icm, node *in_icm,
 *                         types *out_type, types *in_type, int line)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
MergeIcmsFundef (node *out_icm, node *in_icm, types *out_type, types *in_type, int line)
{
    DBUG_ENTER ("MergeIcmsFundef");

    if ((strcmp ("out", ID_NAME (EXPRS_EXPR (out_icm))))
        || (strcmp ("in", ID_NAME (EXPRS_EXPR (in_icm))))) {
        ERROR (line, ("Pragma 'linksign' illegal"));
        CONT_ERROR (("Mappings allowed exclusively between one parameter"
                     " and one return value on which both the function"
                     " does no refcounting !"));
        ABORT_ON_ERROR;
    }

    if (CMP_equal == CmpTypes (out_type, in_type)) {
        if (IsBoxed (out_type)) {
            EXPRS_EXPR (out_icm) = FreeTree (EXPRS_EXPR (out_icm));
            EXPRS_EXPR (out_icm) = MakeId_Copy ("upd_bx");
        } else {
            EXPRS_EXPR (out_icm) = FreeTree (EXPRS_EXPR (out_icm));
            EXPRS_EXPR (out_icm) = MakeId_Copy ("upd");
        }

        DBUG_PRINT ("COMP", ("Merging icm args of ND_FUN_DEC, new tag=\"%s\"",
                             ID_NAME (EXPRS_EXPR (out_icm))));
    } else {
        ERROR (line, ("Pragma 'linksign' illegal"));
        CONT_ERROR (("Mappings allowed exclusively between parameters"
                     " with identical types !"));
        ABORT_ON_ERROR;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 ******************************************************************************
 **
 **  Functions for icm/type table
 **
 **/

/******************************************************************************
 *
 * Function:
 *   node *GenerateIcmTypeTables( node *arg_info, node *fundef,
 *                                bool gen_icm_tab, bool gen_type_tab)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
GenerateIcmTypeTables (node *arg_info, node *fundef, bool gen_icm_tab, bool gen_type_tab)
{
    node **icm_tab;
    types **type_tab;
    int size;
    int i;

    DBUG_ENTER ("GenerateIcmTypeTables");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "no fundef found!");

    size = CountFunctionParams (fundef) + 2;
    INFO_COMP_TABSIZE (arg_info) = size;

    if (gen_icm_tab) {
        icm_tab = (node **)MALLOC (size * sizeof (node *));
        for (i = 0; i < size; i++) {
            icm_tab[i] = NULL;
        }
        INFO_COMP_ICMTAB (arg_info) = icm_tab;
    } else {
        INFO_COMP_ICMTAB (arg_info) = NULL;
    }

    if (gen_type_tab) {
        type_tab = (types **)MALLOC (size * sizeof (types *));
        for (i = 0; i < size; i++) {
            type_tab[i] = NULL;
        }
        INFO_COMP_TYPETAB (arg_info) = type_tab;
    } else {
        INFO_COMP_TYPETAB (arg_info) = NULL;
    }

    INFO_COMP_CNTPARAM (arg_info) = 0;

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * Function:
 *   node *RemoveIcmTypeTables( node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
RemoveIcmTypeTables (node *arg_info)
{
    DBUG_ENTER ("RemoveIcmTypeTables");

    INFO_COMP_TABSIZE (arg_info) = 0;
    FREE ((INFO_COMP_ICMTAB (arg_info)));
    FREE ((INFO_COMP_TYPETAB (arg_info)));

    INFO_COMP_CNTPARAM (arg_info) = 0;

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * Function:
 *   node *AddThreadIdVardec( node *block)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
AddThreadIdVardec (node *block)
{
    DBUG_ENTER ("AddThreadIdVardec");

    DBUG_ASSERT ((NODE_TYPE (block) == N_block), "no block found!");

    if ((gen_mt_code == GEN_MT_OLD) && (optimize & OPT_PHM)) {
        BLOCK_VARDEC (block) = MakeVardec (StringCopy ("SAC_MT_mythread"),
                                           MakeTypes1 (T_int), BLOCK_VARDEC (block));

        VARDEC_ICM (BLOCK_VARDEC (block))
          = MakeIcm (StringCopy ("MT_DECL_MYTHREAD"), NULL);
    }

    DBUG_RETURN (block);
}

/******************************************************************************
 *
 * Function:
 *   node *AddThreadIdIcmND_FUN_AP( node *icm_assign)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
AddThreadIdIcmND_FUN_AP (node *icm_assign)
{
    node *icm;
    node *args;

    DBUG_ENTER ("AddThreadIdIcmND_FUN_AP");

    DBUG_ASSERT ((NODE_TYPE (icm_assign) == N_assign), "no assign found!");

    icm = ASSIGN_INSTR (icm_assign);

    DBUG_ASSERT (((NODE_TYPE (icm) == N_icm) && (!strcmp (ICM_NAME (icm), "ND_FUN_AP"))),
                 "no ND_FUN_AP icm found!");

    if ((gen_mt_code == GEN_MT_OLD) && (optimize & OPT_PHM)) {
        args = ICM_EXPRS3 (icm);

        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (args)) == N_num),
                     "wrong argument in ND_FUN_AP icm found!");

        EXPRS_NEXT (args)
          = MakeExprs (MakeId_Copy ("in"),
                       MakeExprs (MakeId_Copy ("SAC_MT_mythread"), EXPRS_NEXT (args)));

        (NUM_VAL (EXPRS_EXPR (args)))++;
    }

    DBUG_RETURN (icm_assign);
}

/******************************************************************************
 *
 * Function:
 *   node *AddThreadIdIcmND_FUN_DEC( node *icm)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
AddThreadIdIcmND_FUN_DEC (node *icm)
{
    node *args;

    DBUG_ENTER ("AddThreadIdIcmND_FUN_DEC");

    DBUG_ASSERT (((NODE_TYPE (icm) == N_icm) && (!strcmp (ICM_NAME (icm), "ND_FUN_DEC"))),
                 "no ND_FUN_DEC icm found!");

    if ((gen_mt_code == GEN_MT_OLD) && (optimize & OPT_PHM)) {
        args = ICM_EXPRS3 (icm);

        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (args)) == N_num),
                     "wrong argument in ND_FUN_DEC icm found!");

        EXPRS_NEXT (args)
          = MakeExprs (MakeId_Copy ("in"),
                       MakeExprs (MakeId_Copy ("unsigned int"),
                                  MakeExprs (MakeId_Copy ("SAC_MT_mythread"),
                                             EXPRS_NEXT (args))));

        (NUM_VAL (EXPRS_EXPR (args)))++;
    }

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * Function:
 *   node *CreateIcmND_FUN_AP( node *fundef, node **icm_tab, int tab_size)
 *
 * Description:
 *   Builds a ND_FUN_AP icm.
 *
 * Remark:
 *   The content of 'icm_tab' is removed!!
 *
 ******************************************************************************/

static node *
CreateIcmND_FUN_AP (node *fundef, node **icm_tab, int tab_size)
{
    node *ret_node, *icm_arg2, *tmp;
    int cnt_icm, i;

    DBUG_ENTER ("CreateIcmND_FUN_AP");

    DBUG_PRINT ("COMP", ("Creating icm ND_FUN_AP"));

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "no fundef node found!");

    if (icm_tab[1] == NULL) {
        icm_arg2 = MakeId_Copy ("");
    } else {
        /* tag is not needed! */
        icm_tab[1] = FreeNode (icm_tab[1]);
        icm_arg2 = icm_tab[1];
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

    ret_node = MakeAssignIcm3 ("ND_FUN_AP", MakeId_Copy (FUNDEF_NAME (fundef)), icm_arg2,
                               MakeNum (cnt_icm));

    /* insert pointer to fundef and increment FUNDEF_USED counter*/
    ICM_FUNDEF (ASSIGN_INSTR (ret_node)) = fundef;

    /*
     * The ICM arguments are extracted from the table and inserted into the ICM
     */
    tmp = ICM_EXPRS3 (ASSIGN_INSTR (ret_node));
    for (i = 2; i < tab_size; i++) {
        if (icm_tab[i] != NULL) {
            EXPRS_NEXT (tmp) = icm_tab[i];
            tmp = EXPRS_NEXT (icm_tab[i]);
            DBUG_ASSERT ((EXPRS_NEXT (tmp) == NULL),
                         "Superfluous entry in ICM table found");
            icm_tab[i] = NULL;
        }
    }

    if (icm_tab[0] != NULL) {
        EXPRS_NEXT (tmp) = icm_tab[0];
        icm_tab[0] = NULL;
    }

    /*
     * add the thread id
     */
    if (FUNDEF_BODY (fundef) != NULL) {
        ret_node = AddThreadIdIcmND_FUN_AP (ret_node);
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CreateIcmND_FUN_DEC( node *fundef, node **icm_tab, int tab_size)
 *
 * Description:
 *   Creates a ND_FUN_DEC ICM, which has the following format:
 *     ND_FUN_DEC( name, rettype, narg, [TAG, type, arg]*),
 *
 * Remark:
 *   The content of 'icm_tab' is removed!!
 *
 ******************************************************************************/

static node *
CreateIcmND_FUN_DEC (node *fundef, node **icm_tab, int tab_size)
{
    node *ret_node, *icm_arg2, *tmp;
    int cnt_icm, i;

    DBUG_ENTER ("CreateIcmND_FUN_DEC");

    DBUG_PRINT ("COMP", ("Creating icm ND_FUN_DEC"));

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "no fundef node found!");

    /*
     * Second argument is returntype, void if none given.
     */
    if (icm_tab[1] == NULL) {
        icm_arg2 = MakeId_Copy ("void");
    } else {
        /* tag (1st) and name (3rd) not needed! */
        icm_tab[1] = FreeNode (icm_tab[1]);
        icm_arg2 = icm_tab[1];
        EXPRS_NEXT (icm_arg2) = FreeTree (EXPRS_NEXT (icm_arg2));
        icm_tab[1] = NULL;
    }

    /*
     * count number of arguments
     * including optional dots, but excluding return value
     */
    cnt_icm = 0;
    for (i = 0; i < tab_size; i++) {
        /*
         *  ignore actual returnvalue and not given arguments
         */
        if ((i != 1) && (icm_tab[i] != NULL)) {
            cnt_icm++;
        }
    }

    ret_node = MakeIcm3 ("ND_FUN_DEC", MakeId_Copy (FUNDEF_NAME (fundef)), icm_arg2,
                         MakeNum (cnt_icm));

    /*
     * The ICM arguments are extracted from the table and inserted into the ICM
     */
    tmp = ICM_EXPRS3 (ret_node);
    for (i = 2; i < tab_size; i++) {
        if (icm_tab[i] != NULL) {
            EXPRS_NEXT (tmp) = icm_tab[i];
            tmp = EXPRS_NEXT (EXPRS_NEXT (icm_tab[i]));
            DBUG_ASSERT ((EXPRS_NEXT (tmp) == NULL),
                         "Superfluous entry in ICM table found!");
            icm_tab[i] = NULL;
        }
    }

    /*
     * if there is an optional dots we add this one as the last arg.
     */
    if (icm_tab[0] != NULL) {
        EXPRS_NEXT (tmp) = icm_tab[0];
        icm_tab[0] = NULL;
    }

    /*
     * add the thread id
     */
    if (FUNDEF_BODY (fundef) != NULL) {
        if (!strcmp (FUNDEF_NAME (fundef), "main")) {
            FUNDEF_BODY (fundef) = AddThreadIdVardec (FUNDEF_BODY (fundef));
        } else {
            ret_node = AddThreadIdIcmND_FUN_DEC (ret_node);
        }
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CreateIcmMT_SPMD_FUN_DEC( node *fundef, node **icm_tab, int tab_size)
 *
 * Description:
 *   creates a MT_SPMD_FUN_DEC ICM.
 *
 * Remark:
 *   The content of 'icm_tab' is removed!!
 *
 ******************************************************************************/

static node *
CreateIcmMT_SPMD_FUN_DEC (node *fundef, node **icm_tab, int tab_size)
{
    node *icm, *tmp;
    int cnt_icm, i;

    DBUG_ENTER ("CreateIcmMT_SPMD_FUN_DEC");

    DBUG_PRINT ("COMP", ("Creating icm MT_SPMD_FUN_DEC"));

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "no fundef node found!");

    cnt_icm = 0;
    for (i = 1; i < tab_size; i++) {
        if (icm_tab[i] != NULL) {
            cnt_icm++;
        }
    }

    icm = MakeIcm3 ("MT_SPMD_FUN_DEC", MakeId_Copy (FUNDEF_NAME (fundef)),
                    MakeId_Copy (FUNDEF_NAME (FUNDEF_LIFTEDFROM (fundef))),
                    MakeNum (cnt_icm));

    tmp = ICM_EXPRS3 (icm);
    for (i = 1; i < tab_size; i++) {
        if (icm_tab[i] != NULL) {
            EXPRS_NEXT (tmp) = icm_tab[i];
            tmp = EXPRS_NEXT (EXPRS_NEXT (icm_tab[i]));
            icm_tab[i] = NULL;
        }
    }

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * Function:
 *   node *CreateIcmMT2_FUN_DEC( char *kindof,
 *                               node *fundef, node **icm_tab, int tab_size)
 *
 * Description:
 *   creates a MT2_FUN_DEC ICM.
 *
 ******************************************************************************/

static node *
CreateIcmMT2_FUN_DEC (char *kindof, node *fundef, node **icm_tab, int tab_size)
{
    node *icm, *tmp;
    int cnt_icm, i;

    DBUG_ENTER ("CreateIcmMT2_FUN_DEC");

    DBUG_PRINT ("COMP", ("Creating icm MT2_FUN_DEC"));

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "no fundef node found!");

    cnt_icm = 0;
    for (i = 1; i < tab_size; i++) {
        if (icm_tab[i] != NULL) {
            cnt_icm++;
        }
    }

    icm
      = MakeIcm4 ("MT2_FUN_DEC", MakeId_Copy (kindof), MakeId_Copy (FUNDEF_NAME (fundef)),
                  MakeId_Copy (FUNDEF_NAME (FUNDEF_LIFTEDFROM (fundef))),
                  MakeNum (cnt_icm));

    tmp = ICM_EXPRS4 (icm);
    for (i = 1; i < tab_size; i++) {
        if (icm_tab[i] != NULL) {
            EXPRS_NEXT (tmp) = icm_tab[i];
            tmp = EXPRS_NEXT (EXPRS_NEXT (icm_tab[i]));
        }
    }

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * Function:
 *   node *CreateFundefIcm( node *fundef, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
CreateFundefIcm (node *fundef, node *arg_info)
{
    node *icm;
    node **icm_tab = INFO_COMP_ICMTAB (arg_info);
    int tab_size = INFO_COMP_TABSIZE (arg_info);

    DBUG_ENTER ("CreateFundefIcm");

    /*
     * if mtn(==mt2) is set, we check if this is a lifted mt-function inspecting
     * FUNDEF_ATTRIB, otherwise we check FUNDEF_STATUS to see if this is
     * a spmd-function (old mt==mto) or decide it is a "normal" function.
     */
    if (gen_mt_code == GEN_MT_NEW) {
        switch (FUNDEF_ATTRIB (fundef)) {
        case ST_call_mtlift:
            icm = CreateIcmMT2_FUN_DEC ("CALL_MTLIFT", fundef, NULL, 0);
            break;

        case ST_call_mt_worker:
            icm = CreateIcmMT2_FUN_DEC ("CALL_MTWORKER", fundef, icm_tab, tab_size);
            break;

        case ST_call_mt_master:
            icm = CreateIcmMT2_FUN_DEC ("CALL_MTMASTER", fundef, icm_tab, tab_size);
            break;

        case ST_call_rep:
            icm = CreateIcmMT2_FUN_DEC ("CALL_MTREP", fundef, icm_tab, tab_size);
            break;

        case ST_call_st:
            /* using normal function-declaration for single threaded calls */
            icm = CreateIcmND_FUN_DEC (fundef, icm_tab, tab_size);
            break;

        default:
            DBUG_PRINT ("jhs", ("%s", mdb_statustype[FUNDEF_ATTRIB (fundef)]));
            DBUG_PRINT ("jhs", ("%s", mdb_statustype[FUNDEF_STATUS (fundef)]));
            DBUG_ASSERT (0, "unknown kind of function while in mt2");
            icm = NULL;
            break;
        }
    } else if (FUNDEF_STATUS (fundef) == ST_spmdfun) {
        icm = CreateIcmMT_SPMD_FUN_DEC (fundef, icm_tab, tab_size);
    } else {
        icm = CreateIcmND_FUN_DEC (fundef, icm_tab, tab_size);
    }

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * Function:
 *   void InsertApDotsParam( node **icm_tab, node *icm_arg)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
InsertApDotsParam (node **icm_tab, node *icm_arg)
{
    DBUG_ENTER ("InsertApDotsParam");

    DBUG_ASSERT ((icm_tab[0] == NULL), "ap: more than one T_dots parameter found!");

    icm_tab[0] = icm_arg;

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   node *InsertApArgParam( node **icm_tab, node *icm_arg,
 *                           types *type, int rc,
 *                           int *linksign, int cnt_param,
 *                           int line)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
InsertApArgParam (node **icm_tab, node *icm_arg, types *type, int rc, int *linksign,
                  int cnt_param, int line)
{
    node *icm_node = NULL;

    DBUG_ENTER ("InsertApArgParam");

    DBUG_PRINT ("COMP", ("Inserting arg #%d, tag=\"%s\"", cnt_param,
                         ID_NAME (EXPRS_EXPR (icm_arg))));

    if (linksign == NULL) {
        /*
         * create standard icm table
         */
        icm_tab[cnt_param + 2] = icm_arg;
    } else {
        /*
         * create special icm table depending on pragma linksign
         */
        if (icm_tab[linksign[cnt_param] + 1] == NULL) {
            icm_tab[linksign[cnt_param] + 1] = icm_arg;
        } else {
            icm_node
              = MergeIcmsAp (icm_tab[linksign[cnt_param] + 1], icm_arg, type, rc, line);
        }
    }

    DBUG_RETURN (icm_node);
}

/******************************************************************************
 *
 * Function:
 *   void InsertApReturnParam( node **icm_tab, node *icm_arg,
 *                             int *linksign, int cnt_param)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
InsertApReturnParam (node **icm_tab, node *icm_arg, int *linksign, int cnt_param)
{
    DBUG_ENTER ("InsertApReturnParam");

    DBUG_PRINT ("COMP", ("Inserting arg #%d, tag=\"%s\"", cnt_param,
                         ID_NAME (EXPRS_EXPR (icm_arg))));

    if (linksign == NULL) {
        /*
         * create standard icm table
         */
        if ((!strcmp (ID_NAME (EXPRS_EXPR (icm_arg)), "out")) && (icm_tab[1] == NULL)) {
            icm_tab[1] = icm_arg;
        } else {
            icm_tab[cnt_param + 2] = icm_arg;
        }
    } else {
        /*
         * create special icm table depending on pragma liksign
         */
        icm_tab[linksign[cnt_param] + 1] = icm_arg;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void InsertFundefDotsParam( node **icm_tab, node *icm_args)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
InsertFundefDotsParam (node **icm_tab, node *icm_args)
{
    DBUG_ENTER ("InsertFundefDotsParam");

    DBUG_ASSERT ((icm_tab[0] == NULL), "fundef: more than one T_dots parameter found!");

    icm_tab[0] = icm_args;

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void InsertFundefArgParam( node **icm_tab, node *icm_args,
 *                              types **type_tab, types *type_args,
 *                              int *linksign, int cnt_param,
 *                              int line)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
InsertFundefArgParam (node **icm_tab, node *icm_args, types **type_tab, types *type_args,
                      int *linksign, int cnt_param, int line)
{
    DBUG_ENTER ("InsertFundefArgParam");

    DBUG_PRINT ("COMP", ("Inserting arg #%d, tag=\"%s\"", cnt_param,
                         ID_NAME (EXPRS_EXPR (icm_args))));

    if (linksign == NULL) {
        /*
         * create standard icm table
         */
        icm_tab[cnt_param + 2] = icm_args;
    } else {
        /*
         * create special icm table depending on pragma liksign
         */
        if (linksign[cnt_param] == 0) {
            ERROR (line, ("Pragma 'linksign' illegal"));
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
 * Function:
 *   statustype InsertFundefReturnParam( node **icm_tab, node *icm_args,
 *                                       types **type_tab, types *type_args,
 *                                       int *linksign, int cnt_param,
 *                                       int line)
 *
 * Description:
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
InsertFundefReturnParam (node **icm_tab, node *icm_args, types **type_tab,
                         types *type_args, int *linksign, int cnt_param, int line)
{
    statustype ret = ST_regular;

    DBUG_ENTER ("InsertFundefReturnParam");

    DBUG_PRINT ("COMP", ("Inserting arg #%d, tag=\"%s\"", cnt_param,
                         ID_NAME (EXPRS_EXPR (icm_args))));

    if (linksign == NULL) {
        /*
         * create standard icm table
         */

        if ((!strcmp (ID_NAME (EXPRS_EXPR (icm_args)), "out")) && (icm_tab[1] == NULL)) {
            icm_tab[1] = icm_args;
            ret = ST_crettype;
        } else {
            icm_tab[cnt_param + 2] = icm_args;
            ret = ST_regular;
        }
    } else {
        /*
         * create special icm table depending on pragma linksign
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
            CONT_ERROR (("two return parameters mapped to same position!"));
            ABORT_ON_ERROR;
        }
    }

    DBUG_RETURN (ret);
}

/**
 **
 **  Functions for icm/type table
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *
 * Function:
 *   node *ReorganizeReturnIcm( node *ret_icm)
 *
 * Description:
 *   The ND_FUN_RET icm handles the first out-parameter of a function
 *   different from all others since this one is compiled to the original
 *   single return value of a C function.
 *   This function reorganizes the given args for ND_FUN_RET accordingly.
 *
 ******************************************************************************/

static node *
ReorganizeReturnIcm (node *ret_icm)
{
    node *icm_args, *icm_arg, *pred_arg, *ret_node, *tmp;
    bool first_out = FALSE;
    int cnt = 0;

    DBUG_ENTER ("ReorganizeReturnIcm");

    DBUG_ASSERT (((ret_icm == NULL) || (NODE_TYPE (ret_icm) == N_icm)),
                 "illegal return icm found!");

    if ((ret_icm != NULL) && (!strcmp (ICM_NAME (ret_icm), "ND_FUN_RET"))) {
        icm_args = ICM_ARGS (ret_icm);
        if (icm_args != NULL) {
            pred_arg = EXPRS_NEXT (icm_args);
            icm_arg = EXPRS_NEXT (pred_arg);

            while (icm_arg != NULL) {
                if ((!first_out) && (!strcmp ("out", ID_NAME (EXPRS_EXPR (icm_arg))))) {
                    first_out = TRUE;
                    /*
                     * copy the name of the current ICM arg into the first arg.
                     */
                    EXPRS_EXPR (icm_args) = FreeTree (EXPRS_EXPR (icm_args));
                    ret_node = EXPRS_NEXT (icm_arg);
                    EXPRS_EXPR (icm_args) = DupNode (EXPRS_EXPR (ret_node));

                    DBUG_ASSERT ((!strcmp (ID_NAME (EXPRS_EXPR (ret_node)),
                                           ID_NAME (EXPRS_EXPR2 (ret_node)))),
                                 "code-name and decl-name of return-argument differ!");

                    /*
                     * remove the next three ICM args
                     */
                    tmp = EXPRS_EXPRS4 (icm_arg);
                    EXPRS_EXPRS4 (icm_arg) = NULL;
                    icm_arg = FreeTree (icm_arg);
                    icm_arg = EXPRS_NEXT (pred_arg) = tmp;
                } else {
                    pred_arg = EXPRS_EXPRS4 (pred_arg);
                    icm_arg = EXPRS_NEXT (pred_arg);
                    cnt++;
                }
            }
            NUM_VAL (EXPRS_EXPR2 (icm_args)) = cnt;
        }
    }

    DBUG_RETURN (ret_icm);
}

#ifndef DBUG_OFF

/******************************************************************************
 *
 * Function:
 *   bool CheckAp( ids *let_ids, node *ap, node *arg_info)
 *
 * Description:
 *   Checks whether no one of the refcounted arguments occurs on LHS of the
 *   given application.
 *
 ******************************************************************************/

static bool
CheckAp (ids *let_ids, node *ap, node *arg_info)
{
    node *args, *arg_id;
    int arg_idx;
    bool ok = TRUE;

    DBUG_ENTER ("CheckAp");

    DBUG_ASSERT ((NODE_TYPE (ap) == N_ap), "no N_ap node found!");

    while (let_ids != NULL) {
        args = AP_ARGS (ap);
        arg_idx = INFO_COMP_CNTPARAM (arg_info);
        while (args != NULL) {
            arg_id = EXPRS_EXPR (args);
            if ((NODE_TYPE (arg_id) == N_id) && (RC_IS_ACTIVE (ID_REFCNT (arg_id)))
                && (!FUNDEF_DOES_REFCOUNT (AP_FUNDEF (ap), arg_idx))) {
                if (!strcmp (IDS_NAME (let_ids), ID_NAME (arg_id))) {
                    ok = FALSE;
                }
            }
            args = EXPRS_NEXT (args);
            arg_idx++;
        }
        let_ids = IDS_NEXT (let_ids);
    }

    DBUG_RETURN (ok);
}

#endif

/******************************************************************************
 *
 * Function:
 *   node *BuildParamsByDFM( DFMmask_t *mask,
 *                           char *tag, int *num_args, node *icm_args)
 *
 * Description:
 *   Builds triplet-chain (tag, type, name) from dfm-mask mask,
 *   tag will we used as base for the tags (used raw or _rc is added),
 *   num_args will be incremented for each triplet added (maybe NULL),
 *   at the end of this chain icm_args will be concatenated.
 *
 *
 * ### CODE NOT BRUSHED YET ###
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

        if (RC_IS_ACTIVE (VARDEC_OR_ARG_REFCNT (vardec))) {
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
 * Function:
 *   node *BuildParamsByDFMfold( DFMmask_t *mask,
 *                               char *tag, int *num_args, node *icm_args)
 *
 * Description:
 *   Builds tuple-chain (tag, type, name, fun) from dfm-foldmask mask,
 *   tag will we used as base for the tags (used raw or _rc is added),
 *   num_args will be incremented for each triplet added (maybe NULL),
 *   at the end of this chain icm_args will be concatenated.
 *
 *
 * ### CODE NOT BRUSHED YET ###
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

        if (RC_IS_ACTIVE (VARDEC_OR_ARG_REFCNT (vardec))) {
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
 *   node *DoSomeReallyUglyTransformations_MT2( node *fundef)
 *
 * Description:
 *
 *
 * ### CODE NOT BRUSHED YET ###
 *
 ******************************************************************************/

node *
DoSomeReallyUglyTransformations_MT2 (node *fundef)
{
    node *front, *back;
    node *vardec;
    node *assigns, *assign;

    DBUG_ENTER ("*DoSomeReallyUglyTransformations");

    /*
     * dkr:
     * The following code is unbelievable UGLY!!!!
     * But I do not want to rewrite the hole shit now :-/
     */

    front = MakeIcm4 ("MT2_WORKER_RECEIVE", MakeId_Copy ("SYNC"),
                      MakeNum (FUNDEF_IDENTIFIER (fundef)),
                      MakeNum (DFMTestMask (FUNDEF_MT2USE (fundef))),
                      BuildParamsByDFM (FUNDEF_MT2USE (fundef), "receive", NULL, NULL));
    back = MakeIcm3 ("MT2_WORKER_BARRIER", MakeNum (FUNDEF_IDENTIFIER (fundef)),
                     MakeNum (DFMTestMask (FUNDEF_MT2DEF (fundef))),
                     BuildParamsByDFM (FUNDEF_MT2DEF (fundef), "barrier", NULL, NULL));

    BLOCK_INSTR (FUNDEF_BODY (fundef))
      = MakeAssign (front, BLOCK_INSTR (FUNDEF_BODY (fundef)));

    /*
     * insert before return
     */
    assigns = BLOCK_INSTR (FUNDEF_BODY (fundef));
    assign = MakeAssign (back, NULL);
    while ((ASSIGN_NEXT (assigns) != NULL)
           && (NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (assigns))) != N_return)) {
        assigns = ASSIGN_NEXT (assigns);
    }
    ASSIGN_NEXT (assign) = ASSIGN_NEXT (assigns);
    ASSIGN_NEXT (assigns) = assign;

    /* convert args into vardecs .... */
    if (FUNDEF_VARDEC (fundef) != NULL) {
        /*
         * dkr: Here, *no* conversion is done!!
         *      How can this work correctly???
         */
        vardec = FUNDEF_VARDEC (fundef);
        while (VARDEC_NEXT (vardec) != NULL) {
            vardec = VARDEC_NEXT (vardec);
        }

        VARDEC_NEXT (vardec) = FUNDEF_ARGS (fundef);
        vardec = VARDEC_NEXT (vardec);
    } else {
        FUNDEF_VARDEC (fundef) = FUNDEF_ARGS (fundef);
        vardec = FUNDEF_VARDEC (fundef);
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

    FUNDEF_ARGS (fundef) = NULL;

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * Function:
 *   node *Compile( node *arg_node)
 *
 * Description:
 *   Starts compilation.
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

    info = FreeTree (info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPModul( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles a N_modul node: traverses sons.
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
        /*
         * compile all special fold-funs only
         */
        INFO_COMP_FOLDFUNS (arg_info) = TRUE;
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);

        /*
         * compile all other functions
         */
        INFO_COMP_FOLDFUNS (arg_info) = FALSE;
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
 *   node *COMPTypedef( node *arg_node, node *arg_info)
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
        dim_node = MakeNum (DIM_NO_OFFSET (GetDim (OBJDEF_TYPE (arg_node))));

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
 * Function:
 *   types *COMPFundefTypes( types *arg_types, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static types *
COMPFundefTypes (types *arg_types, node *arg_info)
{
    node *fundef;
    node *ret_icm, *ret_exprs;
    node *icm_args;
    types *ret_type;

    DBUG_ENTER ("COMPFundefTypes");

    fundef = INFO_COMP_FUNDEF (arg_info);
    ret_icm = FUNDEF_RETURN (fundef);

    /*
     * compile return types
     */
    if ((arg_types != NULL) && (TYPES_BASETYPE (arg_types) != T_void)
        && (FUNDEF_BODY (fundef) != NULL)) {
        /*
         * 'ret_exprs' will point to the N_exprs in *front* of the first
         *  return value.
         */

        DBUG_ASSERT (((FUNDEF_RETURN (fundef) != NULL)
                      && (NODE_TYPE (FUNDEF_RETURN (fundef)) == N_icm)),
                     "No return-ICM found");

        ret_exprs = ICM_EXPRS2 (FUNDEF_RETURN (fundef));
    } else {
        ret_exprs = NULL;
    }

    ret_type = arg_types;
    while ((ret_type != NULL) && (TYPES_BASETYPE (ret_type) != T_void)
           && (TYPES_BASETYPE (ret_type) != T_dots)) {
        /*
         * third ICM arg: var name
         */
        if (FUNDEF_BODY (fundef) == NULL) {
            /* an extern declaration */
            icm_args = MakeExprs (MakeId_Copy (""), NULL);
        } else {
            /*
             * set 'ret_exprs' in front of the next function return value
             */
            if (!strcmp (ICM_NAME (ret_icm), "ND_FUN_RET")) {
                ret_exprs = EXPRS_EXPRS4 (ret_exprs);
            } else if (!strcmp (ICM_NAME (ret_icm), "MT_SPMD_FUN_RET")) {
                ret_exprs = EXPRS_EXPRS3 (ret_exprs);
            } else if (!strcmp (ICM_NAME (ret_icm), "MT2_FUN_RET")) {
                ret_exprs = EXPRS_EXPRS3 (ret_exprs);
            } else {
                DBUG_ASSERT ((0), "illegal return-ICM found!");
            }

            DBUG_ASSERT ((ret_exprs != NULL), "no return value found");
            DBUG_ASSERT ((N_id == NODE_TYPE (EXPRS_EXPR (ret_exprs))),
                         "return value must be a N_id node!");

            icm_args = MakeExprs (DupNode (EXPRS_EXPR (ret_exprs)), NULL);
        }

        /*
         * second ICM arg: full type
         */
        icm_args = MakeExprs (MakeTypeNode (ret_type), icm_args);

        /*
         * first ICM arg: tag
         */
        if ((MUST_REFCOUNT (ret_type))
            && (FUNDEF_DOES_REFCOUNT (fundef, INFO_COMP_CNTPARAM (arg_info)))) {
            icm_args = MakeExprs (MakeId_Copy ("out_rc"), icm_args);
        } else {
            icm_args = MakeExprs (MakeId_Copy ("out"), icm_args);
        }

        TYPES_STATUS (ret_type)
          = InsertFundefReturnParam (INFO_COMP_ICMTAB (arg_info), icm_args,
                                     INFO_COMP_TYPETAB (arg_info), ret_type,
                                     (FUNDEF_PRAGMA (fundef) == NULL)
                                       ? NULL
                                       : FUNDEF_LINKSIGN (fundef),
                                     INFO_COMP_CNTPARAM (arg_info), NODE_LINE (fundef));

        ret_type = TYPES_NEXT (ret_type);
        (INFO_COMP_CNTPARAM (arg_info))++;
    } /* while */

    if ((ret_type != NULL) && (TYPES_BASETYPE (ret_type) == T_dots)) {
        icm_args = MakeExprs (MakeId_Copy ("in"),
                              MakeExprs (MakeTypeNode (ret_type),
                                         MakeExprs (MakeId_Copy (""), NULL)));

        InsertFundefDotsParam (INFO_COMP_ICMTAB (arg_info), icm_args);
    }

    DBUG_RETURN (arg_types);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPFundef( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
COMPFundef (node *arg_node, node *arg_info)
{
    node *old_fundef;

    DBUG_ENTER ("COMPFundef");

    DBUG_PRINT ("COMP", ("compiling %s", FUNDEF_NAME (arg_node)));

    /*
     * traverse special fold-funs only if INFO_COMP_FOLDFUNS is true,
     * traverse other functions only if INFO_COMP_FOLDFUNS is false.
     */
    if ((FUNDEF_STATUS (arg_node) != ST_zombiefun)
        && (((INFO_COMP_FOLDFUNS (arg_info)) && (FUNDEF_STATUS (arg_node) == ST_foldfun))
            || ((!INFO_COMP_FOLDFUNS (arg_info))
                && (FUNDEF_STATUS (arg_node) != ST_foldfun)))) {

        /*
         * push 'arg_info'
         */
        old_fundef = INFO_COMP_FUNDEF (arg_info);
        INFO_COMP_FUNDEF (arg_info) = arg_node;

        /*
         * building icms for mt2
         */
        if (FUNDEF_ATTRIB (arg_node) == ST_call_mtlift) {
            arg_node = DoSomeReallyUglyTransformations_MT2 (arg_node);
        }

        /********** begin: traverse body **********/

        /*
         * During compilation of a N_sync, the prior N_sync (if exists) is needed.
         * INFO_COMP_LASTSYNC provides these information, it is initialized here with
         * NULL and will be updated by each compilation of a N_sync (one needs to
         * compile them ordered!), this includes the destruction of such a
         * N_sync-tree.
         * After compilation of the function the last known sync is destroyed then.
         */
        INFO_COMP_LASTSYNC (arg_info) = NULL;

        /*
         * Each scheduler within a single SPMD function must be associated with a
         * unique segment ID. This is realized by means of the following counter.
         */
        INFO_COMP_SCHEDULERID (arg_info) = 0;

        /*
         * For each scheduler a specific initialization ICM is created during the
         * traversal of an SPMD function. They are chained by means of N_assign nodes
         * and will later be inserted into the code which sets up the environment
         * for multithreaded execution.
         */
        INFO_COMP_SCHEDULERINIT (arg_info) = NULL;

        /*
         * traverse body
         */
        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        }

        /*
         * Store collected scheduler information.
         */
        BLOCK_SCHEDULER_INIT (FUNDEF_BODY (arg_node))
          = INFO_COMP_SCHEDULERINIT (arg_info);

        if (INFO_COMP_SCHEDULERID (arg_info) > max_schedulers) {
            max_schedulers = INFO_COMP_SCHEDULERID (arg_info);
        }

        /*
         * Destruction of last known N_sync is done here, all others have been killed
         * while traversing.
         */
        if (INFO_COMP_LASTSYNC (arg_info) != NULL) {
            INFO_COMP_LASTSYNC (arg_info) = FreeTree (INFO_COMP_LASTSYNC (arg_info));
        }

        /********** end: traverse body **********/

        arg_info = GenerateIcmTypeTables (arg_info, arg_node, TRUE, TRUE);

        /*
         * traverse return types/values
         */
        FUNDEF_TYPES (arg_node) = COMPFundefTypes (FUNDEF_TYPES (arg_node), arg_info);

        /*
         * traverse arguments
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            if (FUNDEF_BODY (arg_node) != NULL) {
                /* first assignment of body */
                INFO_COMP_FIRSTASSIGN (arg_info) = BLOCK_INSTR (FUNDEF_BODY (arg_node));
            }

            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
            }

            if (FUNDEF_BODY (arg_node) != NULL) {
                /* new first assignment of body */
                BLOCK_INSTR (FUNDEF_BODY (arg_node)) = INFO_COMP_FIRSTASSIGN (arg_info);
                INFO_COMP_FIRSTASSIGN (arg_info) = NULL;
            }
        }

        FUNDEF_RETURN (arg_node) = ReorganizeReturnIcm (FUNDEF_RETURN (arg_node));

        FUNDEF_ICM (arg_node) = CreateFundefIcm (arg_node, arg_info);

        arg_info = RemoveIcmTypeTables (arg_info);

        /*
         * traverse next fundef
         */
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        }

        /*
         * all functions traversed -> we can remove the pragmas
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
         * pop 'arg_info'
         */
        INFO_COMP_FUNDEF (arg_info) = old_fundef;
    } else {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPArg( node *arg_node, node *arg_info)
 *
 * Description:
 *   Uses INFO_COMP_FIRSTASSIGN(arg_info) to insert some ICMs at the beginning
 *   of the current function block.
 *   Creates arguments for the ND_FUN_DEC ICM and inserts them into the
 *   table.
 *
 ******************************************************************************/

node *
COMPArg (node *arg_node, node *arg_info)
{
    node *fundef, *icm_node;
    node *icm_entry;
    char *id_name, *tag;
    int param_idx;
    int dim;

    DBUG_ENTER ("COMPArg");

    fundef = INFO_COMP_FUNDEF (arg_info);
    param_idx = INFO_COMP_CNTPARAM (arg_info);

    /* store name of formal parameter */
    id_name = STR_OR_EMPTY ((ARG_NAME (arg_node)));

    if ((RC_IS_ACTIVE (ARG_REFCNT (arg_node)))
        && (FUNDEF_DOES_REFCOUNT (fundef, param_idx))) {
        if (ARG_ATTRIB (arg_node) == ST_reference) {
            tag = "inout_rc";
        } else {
            tag = "in_rc";
        }

        /*
         * put ICMs for RC-adjustment at beginning of function block
         */
        if (FUNDEF_STATUS (INFO_COMP_FUNDEF (arg_info)) != ST_spmdfun) {
            icm_node = MakeAdjustRcIcm (ARG_NAME (arg_node), ARG_TYPE (arg_node),
                                        ARG_REFCNT (arg_node), ARG_REFCNT (arg_node) - 1);
            if (icm_node != NULL) {
                ASSIGN_NEXT (icm_node) = INFO_COMP_FIRSTASSIGN (arg_info);
                INFO_COMP_FIRSTASSIGN (arg_info) = icm_node;
            }
        }
    } else {
        if (FUNDEF_WANTS_REFCOUNT (fundef, param_idx)) {
            WARN (NODE_LINE (arg_node), ("Pragma 'refcounting' illegal"));
            CONT_WARN (("Function wants to do refcounting on non-refcounted "
                        "parameter no. %d",
                        INFO_COMP_CNTPARAM (arg_info)));
        }

        if (ARG_ATTRIB (arg_node) == ST_reference) {
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

    icm_entry = MakeExprs (MakeId_Copy (tag),
                           MakeExprs (MakeTypeNode (ARG_TYPE (arg_node)),
                                      MakeExprs (MakeId_Copy (id_name), NULL)));

    /*
     * store args in 'icm_tab'
     */
    if (ARG_BASETYPE (arg_node) == T_dots) {
        InsertFundefDotsParam (INFO_COMP_ICMTAB (arg_info), icm_entry);
    } else {
        InsertFundefArgParam (INFO_COMP_ICMTAB (arg_info), icm_entry,
                              INFO_COMP_TYPETAB (arg_info), ARG_TYPE (arg_node),
                              (FUNDEF_PRAGMA (fundef) == NULL) ? NULL
                                                               : FUNDEF_LINKSIGN (fundef),
                              param_idx, NODE_LINE (arg_node));
    }

    INFO_COMP_CNTPARAM (arg_info)++;

    if (ARG_NEXT (arg_node) != NULL) {
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
    dim = GetDim (ARG_TYPE (arg_node));
    if (dim > 0) {
        icm_node = MakeAssignIcm3 ("ND_KS_DECL_ARRAY_ARG", MakeId_Copy (id_name),
                                   MakeNum (dim), Type2Exprs (ARG_TYPE (arg_node)));
        ASSIGN_NEXT (icm_node) = INFO_COMP_FIRSTASSIGN (arg_info);
        INFO_COMP_FIRSTASSIGN (arg_info) = icm_node;
    }

    /*
     * put "ND_DECL_INOUT_PARAM" or "ND_DECL_INOUT_PARAM_RC" ICMs respectively
     * at beginning of function block
     */
    if (ARG_ATTRIB (arg_node) == ST_reference) {
        if (RC_IS_ACTIVE (ARG_REFCNT (arg_node))) {
            icm_node = MakeAssignIcm2 ("ND_DECL_INOUT_PARAM_RC",
                                       MakeTypeNode (ARG_TYPE (arg_node)),
                                       MakeId_Copy (id_name));
        } else {
            icm_node
              = MakeAssignIcm2 ("ND_DECL_INOUT_PARAM", MakeTypeNode (ARG_TYPE (arg_node)),
                                MakeId_Copy (id_name));
        }
        ASSIGN_NEXT (icm_node) = INFO_COMP_FIRSTASSIGN (arg_info);
        INFO_COMP_FIRSTASSIGN (arg_info) = icm_node;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPVardec( node *arg_node, node *arg_info)
 *
 * Description:
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
 * Function:
 *   node *COMPBlock( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles a N_block node.
 *
 *******************************************************************************/

node *
COMPBlock (node *arg_node, node *arg_info)
{
    node *assign;
    char *fun_name, *cs_tag;

    DBUG_ENTER ("COMPBlock");

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
                     "first instruction of block is NULL (should be a N_empty node)");
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

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPAssign( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles a N_assign node.
 *   Note, that the traversal of ASSIGN_INSTR(arg_node) may return a N_assign
 *   chain instead of an expression.
 *
 ******************************************************************************/

node *
COMPAssign (node *arg_node, node *arg_info)
{
    node *instr, *last, *next;

    DBUG_ENTER ("COMPAssign");

    INFO_COMP_ASSIGN (arg_info) = arg_node;
    instr = Trav (ASSIGN_INSTR (arg_node), arg_info);
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
        instr = FreeNode (instr);

        /* insert tail of 'instr' into AST (first element) */
        ASSIGN_NEXT (arg_node) = instr;
    } else {
        /* insert 'instr' into AST */
        ASSIGN_INSTR (arg_node) = instr;
    }

    if (next != NULL) {
        next = Trav (next, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPNormalFunReturn( node *arg_node, node *arg_info)
 *
 * Description:
 *   Generates ICMs for N_return-node found in body of a non-SPMD-function.
 *
 *   Here, *all* out-parameters are handled as reference parameters. The
 *   original single C return value is build by ReorganizeReturnIcm() later on.
 *   This is done, because COMPFundefTypes() relies on the un-reorganized
 *   return icm ...
 *
 ******************************************************************************/

static node *
COMPNormalFunReturn (node *arg_node, node *arg_info)
{
    node *ret_exprs, *ret_expr, *icm_args, *last_arg, *tmp;
    int ret_cnt;
    bool rename;

    DBUG_ENTER ("COMPNormalFunReturn");

    /*
     * Create dummy values for the first two ICM args.
     * These values are corrected later on by the function ReorganizeReturnIcm().
     */
    icm_args = MakeExprs (MakeId_Copy (""), MakeExprs (MakeNum (0), NULL));
    last_arg = EXPRS_NEXT (icm_args);

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
        if (RC_IS_ACTIVE (ID_REFCNT (EXPRS_EXPR (ret_expr)))) {
            last_arg = EXPRS_NEXT (last_arg) = MakeExprs (MakeId_Copy ("out_rc"), NULL);
        } else {
            last_arg = EXPRS_NEXT (last_arg) = MakeExprs (MakeId_Copy ("out"), NULL);
        }

        /*
         * Append the code-name and the decl-name to ICM args
         */
        last_arg = EXPRS_NEXT (last_arg)
          = MakeExprs (DupNode (EXPRS_EXPR (ret_expr)), NULL);
        if (rename) {
            last_arg = EXPRS_NEXT (last_arg)
              = MakeExprs (MakeId_Copy (TmpVarName (ID_NAME (EXPRS_EXPR (ret_expr)))),
                           NULL);
        } else {
            last_arg = EXPRS_NEXT (last_arg)
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
        if (RC_IS_ACTIVE (ID_REFCNT (EXPRS_EXPR (ret_expr)))) {
            last_arg = EXPRS_NEXT (last_arg) = MakeExprs (MakeId_Copy ("inout_rc"), NULL);
        } else {
            last_arg = EXPRS_NEXT (last_arg) = MakeExprs (MakeId_Copy ("inout"), NULL);
        }

        /*
         * Append the code-name and the decl-name to ICM args
         */
        last_arg = EXPRS_NEXT (last_arg)
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
    if (last_arg == EXPRS_NEXT (icm_args)) {
        icm_args = FreeTree (icm_args);
        arg_node = MakeIcm ("NOOP", NULL);
    } else {
        arg_node = MakeIcm ("ND_FUN_RET", icm_args);
    }

    FUNDEF_RETURN (INFO_COMP_FUNDEF (arg_info)) = arg_node;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPSpmdFunReturn( node *arg_node, node *arg_info)
 *
 * Description:
 *   generates ICMs for N_return-node found in body of a SPMD-function.
 *
 ******************************************************************************/

static node *
COMPSpmdFunReturn (node *arg_node, node *arg_info)
{
    node *exprs, *args, *last_arg, *tag;
    int cnt_params;

    DBUG_ENTER ("COMPSpmdFunReturn");

    exprs = RETURN_EXPRS (arg_node);
    args = NULL;
    last_arg = NULL;
    cnt_params = 0;

    while (exprs != NULL) {
        DBUG_ASSERT ((N_id == NODE_TYPE (EXPRS_EXPR (exprs))), "wrong node type found");

        if (RC_IS_ACTIVE (ID_REFCNT (EXPRS_EXPR (exprs)))) {
            tag = MakeExprs (MakeId_Copy ("out_rc"), NULL);
        } else {
            tag = MakeExprs (MakeId_Copy ("out"), NULL);
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
 * Function:
 *   node *COMPMT2FunReturn(node *arg_node, node *arg_info)
 *
 * Description:
 *   generates ICMs for N_return-node found in body of a MT2-function.
 *
 *
 * ### CODE NOT BRUSHED YET ###
 *
 ******************************************************************************/

static node *
COMPMT2FunReturn (node *arg_node, node *arg_info)
{
    node *exprs, *args, *last_arg, *tag;
    int cnt_params;

    DBUG_ENTER ("COMPMT2FunReturn");

    exprs = RETURN_EXPRS (arg_node);
    args = NULL;
    last_arg = NULL;
    cnt_params = 0;

#if 1
    while (exprs != NULL) {
        DBUG_ASSERT ((N_id == NODE_TYPE (EXPRS_EXPR (exprs))), "wrong node type found");

        if (RC_IS_ACTIVE (EXPRS_EXPR (exprs))) {
            tag = MakeExprs (MakeId_Copy ("out_rc"), NULL);
        } else {
            tag = MakeExprs (MakeId_Copy ("out"), NULL);
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
 * Function:
 *   node *COMPReturn(node *arg_node, node *arg_info)
 *
 * Description:
 *   generates N_icms for N_return of a function (ND or MT).
 *
 ******************************************************************************/

node *
COMPReturn (node *arg_node, node *arg_info)
{
    node *fundef;

    DBUG_ENTER ("COMPReturn");

    fundef = INFO_COMP_FUNDEF (arg_info);

    if (gen_mt_code == GEN_MT_NEW) {
        switch (FUNDEF_ATTRIB (fundef)) {
        case ST_call_mtlift:
            /* here is no break missing */
        case ST_call_mt_worker:
            /* here is no break missing */
        case ST_call_mt_master:
            /* here is no break missing */
        case ST_call_rep:
            arg_node = COMPMT2FunReturn (arg_node, arg_info);
            break;

        case ST_call_st:
            arg_node = COMPNormalFunReturn (arg_node, arg_info);
            break;

        default:
            DBUG_PRINT ("jhs", ("%s", mdb_statustype[FUNDEF_ATTRIB (fundef)]));
            DBUG_PRINT ("jhs", ("%s", mdb_statustype[FUNDEF_STATUS (fundef)]));
            DBUG_ASSERT (0, "unknown kind of function while in mt2");
            break;
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
 * Function:
 *   node *COMPLet( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles a N_let node.
 *   The return value is a RHS expression or a N_assign chain of ICMs.
 *   In the latter case the old 'arg_node' is removed.
 *
 ******************************************************************************/

node *
COMPLet (node *arg_node, node *arg_info)
{
    node *expr;
    node *ret_node = NULL;

    DBUG_ENTER ("COMPLet");

    INFO_COMP_LASTIDS (arg_info) = LET_IDS (arg_node);

    if (LET_IDS (arg_node) != NULL) {
        DBUG_PRINT ("COMP", ("LHS of let-assignment: %s", LET_NAME (arg_node)));
    }

    expr = Trav (LET_EXPR (arg_node), arg_info);

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
        arg_node = FreeTree (arg_node);
    } else {
        /*
         * 'expr' is a RHS expression
         */
        LET_EXPR (arg_node) = expr;
        ret_node = arg_node;
    }

    INFO_COMP_LASTIDS (arg_info) = NULL;

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   ids *COMPApIds( ids *let_ids, node *fundef, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
COMPApIds (ids *let_ids, node *fundef, node *arg_info)
{
    node *ret_node, *last_node;
    types *fundef_types;
    bool ids_for_dots;
    node *ret_entry, *last_entry;
    node *icm_node;
    char *tag;

    DBUG_ENTER ("COMPApIds");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "no fundef found!");

    /* create dummy node to append ICMs to */
    ret_node = last_node = MakeAssign (NULL, NULL);

    /*
     * traverse LHS of let-clause
     */
    fundef_types = FUNDEF_TYPES (fundef);
    ids_for_dots = FALSE;
    ret_entry = last_entry = NULL;
    while (let_ids != NULL) {
        DBUG_PRINT ("COMP", ("Handling return value bound to %s", IDS_NAME (let_ids)));

        /* choose the right tag for the argument and generate ICMs for RC */
        if (RC_IS_ACTIVE (IDS_REFCNT (let_ids))) {
            if (FUNDEF_DOES_REFCOUNT (fundef, INFO_COMP_CNTPARAM (arg_info))) {
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

                if (RC_IS_VITAL (IDS_REFCNT (let_ids))) {
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
                InsertApReturnParam (INFO_COMP_ICMTAB (arg_info), ret_entry,
                                     (FUNDEF_PRAGMA (fundef) == NULL)
                                       ? NULL
                                       : FUNDEF_LINKSIGN (fundef),
                                     INFO_COMP_CNTPARAM (arg_info));

                fundef_types = TYPES_NEXT (fundef_types);
            }

            (INFO_COMP_CNTPARAM (arg_info))++;
        } else {
            /* ids_for_dots == TRUE */

            last_entry = EXPRS_NEXT (last_entry) = MakeExprs (MakeId_Copy (tag), NULL);

            last_entry = EXPRS_NEXT (last_entry) = MakeExprs (DupIds_Id (let_ids), NULL);
        }

        let_ids = IDS_NEXT (let_ids);

        if (ids_for_dots && (let_ids == NULL)) {
            InsertApDotsParam (INFO_COMP_ICMTAB (arg_info), ret_entry);
        }
    }

    /* free dummy node (head of chain) */
    ret_node = FreeNode (ret_node);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPApArgs( node *args, node *fundef, int line, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
COMPApArgs (node *args, node *fundef, int line, node *arg_info)
{
    node *ret_node, *last_node;
    node *fundef_args;
    node *arg;
    bool ids_for_dots;
    node *ret_entry, *last_entry;
    node *icm_node;
    char *tag;

    DBUG_ENTER ("COMPApArgs");

    /* create dummy node to append ICMs to */
    ret_node = last_node = MakeAssign (NULL, NULL);

    /*
     * traverse arguments of application
     */
    fundef_args = FUNDEF_ARGS (fundef);
    ids_for_dots = FALSE;
    ret_entry = last_entry = NULL;
    INFO_COMP_MERGE (arg_info) = NULL;

    while (args != NULL) {
        DBUG_PRINT ("COMP", ("Handling argument #%d", INFO_COMP_CNTPARAM (arg_info)));

        arg = EXPRS_EXPR (args);

        /* choose the right tag for the argument and generate ICMs for RC */
        if (NODE_TYPE (arg) == N_id) {
            if (RC_IS_ACTIVE (ID_REFCNT (arg))
                && FUNDEF_DOES_REFCOUNT (fundef, INFO_COMP_CNTPARAM (arg_info))) {
                if (ID_ATTRIB (arg) == ST_reference) {
                    tag = "inout_rc";
                } else {
                    tag = "in_rc";
                }
            } else {
                /* argument is not refcounted or function does no refcounting */
                if (ID_ATTRIB (arg) == ST_reference) {
                    if (FUNDEF_STATUS (fundef) == ST_Cfun) {
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
                node *merge_node;

                merge_node
                  = InsertApArgParam (INFO_COMP_ICMTAB (arg_info), ret_entry,
                                      ARG_TYPE (fundef_args),
                                      (NODE_TYPE (arg) == N_id) ? ID_REFCNT (arg) : (-1),
                                      FUNDEF_PRAGMA (fundef) == NULL
                                        ? NULL
                                        : FUNDEF_LINKSIGN (fundef),
                                      INFO_COMP_CNTPARAM (arg_info), line);

                if (merge_node != NULL) {
                    DBUG_ASSERT ((INFO_COMP_MERGE (arg_info) == NULL),
                                 "multiple merge nodes found!");
                    INFO_COMP_MERGE (arg_info) = merge_node;
                }

                fundef_args = ARG_NEXT (fundef_args);
            }
        } else {
            last_entry = EXPRS_NEXT (last_entry) = MakeExprs (MakeId_Copy (tag), NULL);

            last_entry = EXPRS_NEXT (last_entry) = DupNode (args);
        }

        args = EXPRS_NEXT (args);
        (INFO_COMP_CNTPARAM (arg_info))++;

        if (ids_for_dots && (args == NULL)) {
            InsertApDotsParam (INFO_COMP_ICMTAB (arg_info), ret_entry);
        }
    }

    /* free dummy node (head of chain) */
    ret_node = FreeNode (ret_node);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPAp( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles N_ap node.
 *   Creates an ICM for function application and insert ICMs to decrement the
 *   RC of function arguments. (The return value is a N_assign chain of ICMs.)
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   The flattening phase assures that no one of the arguments occurs on the LHS
 *   of the application (a = fun(a) is impossible).
 *
 *   INFO_COMP_LASTIDS( arg_info) contains pointer to previous let-ids.
 *
 ******************************************************************************/

node *
COMPAp (node *arg_node, node *arg_info)
{
    node *ret_node;
    node *fundef;
    node *assigns1, *assigns2;
    node *new_icm;

    DBUG_ENTER ("COMPAp");

    fundef = AP_FUNDEF (arg_node);

    DBUG_PRINT ("COMP", ("COMPiling application of function %s", ItemName (fundef)));

    arg_info = GenerateIcmTypeTables (arg_info, fundef, TRUE, FALSE);

    /*
     * traverse ids on LHS of let-clause
     */
    assigns1 = COMPApIds (INFO_COMP_LASTIDS (arg_info), fundef, arg_info);

    DBUG_ASSERT ((CheckAp (INFO_COMP_LASTIDS (arg_info), arg_node, arg_info)),
                 "application of a user-defined function without own refcounting:"
                 " refcounted argument occurs also on LHS!");

    /*
     * traverse arguments of application
     */
    assigns2 = COMPApArgs (AP_ARGS (arg_node), fundef, NODE_LINE (arg_node), arg_info);

    ret_node = AppendAssign (assigns1, assigns2);

    ret_node = AdjustAddedAssigns (ret_node, INFO_COMP_MERGE (arg_info));

    /* create new icm */

    new_icm = CreateIcmND_FUN_AP (fundef, INFO_COMP_ICMTAB (arg_info),
                                  INFO_COMP_TABSIZE (arg_info));

    /*
     * increment FUNDEF_USED counter for external call to special loop fundefs
     * or some cond special fundef
     */
    if (FUNDEF_USED (fundef) != USED_INACTIVE) {
        if (((FUNDEF_IS_LOOPFUN (fundef))
             && (INFO_COMP_ASSIGN (arg_info) != FUNDEF_INT_ASSIGN (fundef)))
            || (FUNDEF_IS_CONDFUN (fundef))) {
            (FUNDEF_USED (fundef))++;
            DBUG_PRINT ("COMP", ("incrementing FUNDEF_USED: new value = %d",
                                 FUNDEF_USED (fundef)));
        }
    }

    /* insert ND_FUN_AP icm at head of assignment chain */
    ret_node = AppendAssign (new_icm, ret_node);

    /* insert 'merge_node' at head of assignment chain */
    ret_node = AppendAssign (INFO_COMP_MERGE (arg_info), ret_node);

    arg_info = RemoveIcmTypeTables (arg_info);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfDim( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles N_prf node of type F_dim.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfDim (node *arg_node, node *arg_info)
{
    node *ret_node;
    node *arg;

    DBUG_ENTER ("COMPPrfDim");

    arg = PRF_ARG1 (arg_node);
    DBUG_ASSERT ((NODE_TYPE (arg) == N_id), "N_id as arg of F_dim expected!");

    ret_node = MakeAssign (/* let_ids = dim( arg) */
                           MakeLet (MakeNum (DIM_NO_OFFSET (GetDim (ID_TYPE (arg)))),
                                    DupOneIds (INFO_COMP_LASTIDS (arg_info))),
                           NULL);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfShape( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles N_prf node of type F_shape.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfShape (node *arg_node, node *arg_info)
{
    node *ret_node;
    node *icm_node, *last_node;
    node *arg;
    ids *let_ids;

    DBUG_ENTER ("COMPPrfShape");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg) == N_id), "N_id as arg of F_shape expected!");

    DBUG_ASSERT ((strcmp (IDS_NAME (let_ids), ID_NAME (arg))),
                 "a = shape( a) not allowed!");

    ret_node = MakeAssignIcm3 ("ND_ALLOC_ARRAY", /* result array */
                               MakeId_Copy ("int"), DupIds_Id (let_ids),
                               MakeNum (IDS_REFCNT (let_ids)));
    last_node = ret_node;

    icm_node
      = MakeAssignIcm3 ("ND_CREATE_CONST_ARRAY_S", DupIds_Id (let_ids),
                        MakeNum (GetDim (ID_TYPE (arg))), Type2Exprs (ID_TYPE (arg)));
    last_node = ASSIGN_NEXT (last_node) = icm_node;

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfReshape( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles N_prf node of type F_reshape.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfReshape (node *arg_node, node *arg_info)
{
    node *ret_node;
    node *arg1, *arg2;
    ids *let_ids;

    DBUG_ENTER ("COMPPrfReshape");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg1) == N_id), "N_id as 1st arg of F_reshape expected!");
    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id), "N_id as 2nd arg of F_reshape expected!");

    DBUG_ASSERT ((strcmp (IDS_NAME (let_ids), ID_NAME (arg1))),
                 "a = reshape( a, .) not allowed!");

    /*
     * compile assignment
     * (v = reshape( shp, w)    =>   v = w)
     */
    ret_node = Trav (arg2, arg_info);

    ret_node = Ids2DecRcICMs (ID_IDS (arg1), ret_node);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfArith_SxA( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles N_prf node of type F_add_SxA, F_sub_SxA, F_mul_SxA, F_div_SxA.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfArith_SxA (node *arg_node, node *arg_info)
{
    node *ret_node, *icm_node, *last_node;
    node *arg1, *arg2;
    ids *let_ids;

    DBUG_ENTER ("COMPPrfArith_SxA");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg1) != N_array),
                 "No N_array as 1st arg of F_add/sub/mult/div_SxA expected!");
    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id),
                 "N_id as 2nd arg of F_add/sub/mult/div_SxA expected!");

    ret_node = MakeAssignIcm3 ("ND_ALLOC_ARRAY", MakeBasetypeNode (IDS_TYPE (let_ids)),
                               DupIds_Id (let_ids), MakeNum (0));
    last_node = ret_node;

    if (ID_REFCNT (arg2) == 1) {
        /*
         * put ND_CHECK_REUSE_ARRAY ICM in front of the ICM chain
         */
        icm_node
          = MakeAssignIcm2 ("ND_CHECK_REUSE_ARRAY", DupNode (arg2), DupIds_Id (let_ids));
        ASSIGN_NEXT (icm_node) = ret_node;
        ret_node = icm_node;
    }

    icm_node
      = MakeAssignIcm4 ("ND_BINOP_SxA_A", MakeId_Copy (prf_string[PRF_PRF (arg_node)]),
                        DupNode (arg1), DupNode (arg2), DupIds_Id (let_ids));
    last_node = ASSIGN_NEXT (last_node) = icm_node;

    icm_node = MakeAdjustRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                IDS_REFCNT (let_ids), IDS_REFCNT (let_ids));
    if (icm_node != NULL) {
        last_node = ASSIGN_NEXT (last_node) = icm_node;
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfArith_AxS( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles N_prf node of type F_add_AxS, F_sub_AxS, F_mul_AxS, F_div_AxS.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfArith_AxS (node *arg_node, node *arg_info)
{
    node *ret_node, *icm_node, *last_node;
    node *arg1, *arg2;
    ids *let_ids;

    DBUG_ENTER ("COMPPrfArith_AxS");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg1) == N_id),
                 "N_id as 1st arg of F_add/sub/mult/div_AxS expected!");
    DBUG_ASSERT ((NODE_TYPE (arg2) != N_array),
                 "No N_array as 2nd arg of F_add/sub/mult/div_AxS expected!");

    ret_node = MakeAssignIcm3 ("ND_ALLOC_ARRAY", MakeBasetypeNode (IDS_TYPE (let_ids)),
                               DupIds_Id (let_ids), MakeNum (0));
    last_node = ret_node;

    if (ID_REFCNT (arg1) == 1) {
        /*
         * put ND_CHECK_REUSE_ARRAY ICM in front of the ICM chain
         */
        icm_node
          = MakeAssignIcm2 ("ND_CHECK_REUSE_ARRAY", DupNode (arg1), DupIds_Id (let_ids));
        ASSIGN_NEXT (icm_node) = ret_node;
        ret_node = icm_node;
    }

    icm_node
      = MakeAssignIcm4 ("ND_BINOP_AxS_A", MakeId_Copy (prf_string[PRF_PRF (arg_node)]),
                        DupNode (arg1), DupNode (arg2), DupIds_Id (let_ids));
    last_node = ASSIGN_NEXT (last_node) = icm_node;

    icm_node = MakeAdjustRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                IDS_REFCNT (let_ids), IDS_REFCNT (let_ids));
    if (icm_node != NULL) {
        last_node = ASSIGN_NEXT (last_node) = icm_node;
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfArith_AxA( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles N_prf node of type F_add_AxA, F_sub_AxA, F_mul_AxA, F_div_AxA.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfArith_AxA (node *arg_node, node *arg_info)
{
    node *ret_node, *icm_node, *last_node;
    node *arg1, *arg2;
    ids *let_ids;

    DBUG_ENTER ("COMPPrfArith_AxA");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg1) == N_id),
                 "N_id as 1st arg of F_add/sub/mult/div_AxA expected.");
    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id),
                 "N_id as 2st arg of F_add/sub/mult/div_AxA expected.");

    ret_node = MakeAssignIcm3 ("ND_ALLOC_ARRAY", MakeBasetypeNode (IDS_TYPE (let_ids)),
                               DupIds_Id (let_ids), MakeNum (0));
    last_node = ret_node;

    if (ID_REFCNT (arg2) == 1) {
        /*
         * put ND_CHECK_REUSE_ARRAY ICM in front of the ICM chain
         */
        icm_node
          = MakeAssignIcm2 ("ND_CHECK_REUSE_ARRAY", DupNode (arg2), DupIds_Id (let_ids));
        ASSIGN_NEXT (icm_node) = ret_node;
        ret_node = icm_node;
    }

    if (ID_REFCNT (arg1) == 1) {
        /*
         * put ND_CHECK_REUSE_ARRAY ICM in front of the ICM chain
         */
        icm_node
          = MakeAssignIcm2 ("ND_CHECK_REUSE_ARRAY", DupNode (arg1), DupIds_Id (let_ids));
        ASSIGN_NEXT (icm_node) = ret_node;
        ret_node = icm_node;
    }

    icm_node
      = MakeAssignIcm4 ("ND_BINOP_AxA_A", MakeId_Copy (prf_string[PRF_PRF (arg_node)]),
                        DupNode (arg1), DupNode (arg2), DupIds_Id (let_ids));
    last_node = ASSIGN_NEXT (last_node) = icm_node;

    icm_node = MakeAdjustRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                IDS_REFCNT (let_ids), IDS_REFCNT (let_ids));
    if (icm_node != NULL) {
        last_node = ASSIGN_NEXT (last_node) = icm_node;
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfIdxPsi( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles N_prf node of type F_idx_psi.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfIdxPsi (node *arg_node, node *arg_info)
{
    node *ret_node, *icm_node, *last_node;
    node *arg1, *arg2;
    ids *let_ids;

    DBUG_ENTER ("COMPPrfIdxPsi");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    if ((NODE_TYPE (arg1) != N_prf) || (NODE_TYPE (PRF_ARG1 (arg1)) != N_num)) {
        /*
         * CAUTION: AE, IVE generate unflattened code!
         * The 1st argument of idx_psi() may be a N_prf node with N_num arguments
         */
        DBUG_ASSERT (((NODE_TYPE (arg1) == N_id) || (NODE_TYPE (arg1) == N_num)),
                     "N_id or N_num as 1st arg of F_idx_psi expected!");
    }
    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id), "N_id as 2nd arg of F_idx_psi expected!");

    if (IsArray (IDS_TYPE (let_ids))) {
        /*
         * This is possible only in case of an instrinsic array-psi:
         *   psi( iv, A): int[] -> type[] -> type[]
         * But this is a non-purely intrinsic. Normally an array-psi
         * is transformed into a with-loop containing scalar-psi's only.
         */
        ret_node
          = MakeAssignIcm3 ("ND_ALLOC_ARRAY", MakeBasetypeNode (IDS_TYPE (let_ids)),
                            DupIds_Id (let_ids), MakeNum (IDS_REFCNT (let_ids)));
        last_node = ret_node;

        icm_node = MakeAssignIcm3 ("ND_IDX_PSI_A", DupNode (arg1), DupNode (arg2),
                                   DupIds_Id (let_ids));
        last_node = ASSIGN_NEXT (last_node) = icm_node;
    } else {
        ret_node = MakeAssignIcm3 ("ND_IDX_PSI_S", DupNode (arg1), DupNode (arg2),
                                   DupIds_Id (let_ids));
        last_node = ret_node;
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfIdxModarray( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles N_prf node of type F_idx_modarray.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfIdxModarray (node *arg_node, node *arg_info)
{
    node *ret_node, *icm_node, *last_node;
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
    DBUG_ASSERT ((NODE_TYPE (arg3) != N_array),
                 "No N_array as 3rd arg of F_idx_modarray expected!");

    DBUG_ASSERT ((strcmp (IDS_NAME (let_ids), ID_NAME (arg1))),
                 "a = idx_modarray( a, ., .) not allowed!");
    DBUG_ASSERT (((NODE_TYPE (arg3) != N_id)
                  || strcmp (IDS_NAME (let_ids), ID_NAME (arg3))),
                 "a = idx_modarray( ., ., a) not allowed!");

    DBUG_ASSERT ((!RC_IS_ZERO (ID_REFCNT (arg1))), "Reference with (rc == 0) found!");

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

    ret_node = MakeAssignIcm5 (icm_name, MakeBasetypeNode (IDS_TYPE (let_ids)),
                               DupIds_Id (let_ids), DupNode (arg1), DupNode (arg2),
                               DupNode (arg3));
    last_node = ret_node;

    icm_node = MakeAdjustRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                IDS_REFCNT (let_ids), IDS_REFCNT (let_ids));
    if (icm_node != NULL) {
        last_node = ASSIGN_NEXT (last_node) = icm_node;
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfPsi( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles N_prf node of type F_psi.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfPsi (node *arg_node, node *arg_info)
{
    node *ret_node, *icm_node, *last_node;
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
            ret_node
              = MakeAssignIcm4 ("ND_KD_PSI_VxA_S", DupNode (arg2), DupIds_Id (let_ids),
                                MakeNum (GetTypesLength (ID_TYPE (arg1))),
                                DupNode (arg1));
            last_node = ret_node;
        } else {
            /* 'arg1' is a N_array node */
            ret_node
              = MakeAssignIcm4 ("ND_KD_PSI_CxA_S", DupNode (arg2), DupIds_Id (let_ids),
                                MakeNum (CountExprs (ARRAY_AELEMS (arg1))),
                                DupTree (ARRAY_AELEMS (arg1)));
            last_node = ret_node;
        }
    } else {
        /* 'let_ids' is an array */

        ret_node
          = MakeAssignIcm3 ("ND_ALLOC_ARRAY", MakeBasetypeNode (IDS_TYPE (let_ids)),
                            DupIds_Id (let_ids), MakeNum (IDS_REFCNT (let_ids)));
        last_node = ret_node;

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
                                MakeNum (CountExprs (ARRAY_AELEMS (arg1))),
                                DupTree (ARRAY_AELEMS (arg1)));
            last_node = ASSIGN_NEXT (last_node) = icm_node;
        }
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfModarray( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles N_prf node of type F_modarray.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfModarray (node *arg_node, node *arg_info)
{
    node *ret_node, *icm_node, *last_node;
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

    DBUG_ASSERT ((!RC_IS_ZERO (ID_REFCNT (arg1))), "Reference with (rc == 0) found!");

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

        ret_node = MakeAssignIcm7 (icm_name, res_type, res_dim, DupIds_Id (let_ids),
                                   DupNode (arg1), DupNode (arg3),
                                   MakeNum (CountExprs (ARRAY_AELEMS (arg2))),
                                   DupTree (ARRAY_AELEMS (arg2)));
        last_node = ret_node;
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

        ret_node
          = MakeAssignIcm7 (icm_name, res_type, res_dim, DupIds_Id (let_ids),
                            DupNode (arg1), DupNode (arg3),
                            MakeNum (TYPES_SHAPE (ID_TYPE (arg2), 0)), DupNode (arg2));
        last_node = ret_node;
    }

    icm_node = MakeAdjustRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                IDS_REFCNT (let_ids), IDS_REFCNT (let_ids));
    if (icm_node != NULL) {
        last_node = ASSIGN_NEXT (last_node) = icm_node;
    }

    DBUG_RETURN (ret_node);
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
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remark:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfConvertArr (node *arg_node, node *arg_info)
{
    node *ret_node, *icm_node, *last_node;
    node *arg;
    ids *let_ids;
    char *icm_name;

    DBUG_ENTER ("COMPPrfConvertArr");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg) == N_id), "N_id as arg of F_toi/tof/tod_A aspected");

    ret_node = MakeAssignIcm3 ("ND_ALLOC_ARRAY", MakeBasetypeNode (IDS_TYPE (let_ids)),
                               DupIds_Id (let_ids), MakeNum (IDS_REFCNT (let_ids)));
    last_node = ret_node;

    switch (PRF_PRF (arg_node)) {
    case F_toi_A:
        icm_name = "ND_2I_A";
        break;
    case F_tof_A:
        icm_name = "ND_2F_A";
        break;
    case F_tod_A:
        icm_name = "ND_2D_A";
        break;
    default:
        icm_name = NULL;
        DBUG_ASSERT ((0), "Illegal array conversion function found!");
        break;
    }
    icm_node = MakeAssignIcm2 (icm_name, DupNode (arg), DupIds_Id (let_ids));
    last_node = ASSIGN_NEXT (last_node) = icm_node;

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfTakeDrop( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles N_prf node of type F_take, F_drop.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remark:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfTakeDrop (node *arg_node, node *arg_info)
{
    node *ret_node, *icm_node, *last_node;
    node *arg1, *arg2;
    ids *let_ids;
    char *icm_name;
    int n_elems;

    DBUG_ENTER ("COMPPrfTakeDrop");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT (((NODE_TYPE (arg1) == N_id) && ID_ISCONST (arg1)
                  && (VARDEC_BASETYPE (ID_VARDEC (arg1)) == T_int)),
                 "Constant N_id as 1st arg of F_take/F_drop expected.");
    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id),
                 "N_id as 2st arg of F_take/F_drop expected.");

    DBUG_ASSERT ((strcmp (IDS_NAME (let_ids), ID_NAME (arg2))),
                 "a = take/drop( ., a) not allowed!");

    switch (PRF_PRF (arg_node)) {
    case F_take:
        icm_name = "ND_KD_TAKE_CxA_A";
        break;
    case F_drop:
        icm_name = "ND_KD_DROP_CxA_A";
        break;
    default:
        icm_name = NULL;
        DBUG_ASSERT ((0), "Illegal take/drop function found!");
        break;
    }

    ret_node = MakeAssignIcm3 ("ND_ALLOC_ARRAY", MakeBasetypeNode (IDS_TYPE (let_ids)),
                               DupIds_Id (let_ids), MakeNum (IDS_REFCNT (let_ids)));
    last_node = ret_node;

    n_elems = ID_VECLEN (arg1);
    icm_node = MakeAssignIcm5 (icm_name, MakeNum (GetDim (ID_TYPE (arg2))),
                               DupNode (arg2), DupIds_Id (let_ids), MakeNum (n_elems),
                               IntVec2Array (n_elems, ID_CONSTVEC (arg1)));
    last_node = ASSIGN_NEXT (last_node) = icm_node;

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfCat( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles N_prf node of type F_cat.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfCat (node *arg_node, node *arg_info)
{
    node *ret_node, *icm_node, *last_node;
    node *arg1, *arg2, *arg3;
    ids *let_ids;

    DBUG_ENTER ("COMPPrfCat");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    arg3 = PRF_ARG3 (arg_node);

    DBUG_ASSERT (((NODE_TYPE (arg1) == N_num) || (NODE_TYPE (arg1) == N_id)),
                 "N_num or N_id as 1st arg of F_cat expected!");
    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id), "N_id as 2st arg of F_cat expected.");
    DBUG_ASSERT ((NODE_TYPE (arg3) == N_id), "N_id as 3st arg of F_cat expected.");

    DBUG_ASSERT ((strcmp (IDS_NAME (let_ids), ID_NAME (arg2))),
                 "a = cat( ., a, .) not allowed!");
    DBUG_ASSERT ((strcmp (IDS_NAME (let_ids), ID_NAME (arg3))),
                 "a = cat( ., ., a) not allowed!");

    ret_node = MakeAssignIcm3 ("ND_ALLOC_ARRAY", MakeBasetypeNode (IDS_TYPE (let_ids)),
                               DupIds_Id (let_ids), MakeNum (IDS_REFCNT (let_ids)));
    last_node = ret_node;

    icm_node = MakeAssignIcm5 ("ND_KD_CAT_SxAxA_A", MakeNum (GetDim (ID_TYPE (arg2))),
                               DupNode (arg2), DupNode (arg3), DupIds_Id (let_ids),
                               DupNode (arg1));
    last_node = ASSIGN_NEXT (last_node) = icm_node;

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfRotate( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles N_prf node of type F_rotate.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfRotate (node *arg_node, node *arg_info)
{
    node *ret_node, *icm_node, *last_node;
    node *arg1, *arg2, *arg3;
    ids *let_ids;

    DBUG_ENTER ("COMPPrfRotate");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    arg3 = PRF_ARG3 (arg_node);

    DBUG_ASSERT (((NODE_TYPE (arg1) == N_num) || (NODE_TYPE (arg1) == N_id)),
                 "N_num or N_id as 1st arg of F_rotate expected.");
    DBUG_ASSERT (((NODE_TYPE (arg2) == N_num) || (NODE_TYPE (arg2) == N_id)),
                 "N_num or N_id as 2st arg of F_rotate expected.");
    DBUG_ASSERT ((NODE_TYPE (arg3) == N_id), "N_id as 3st arg of F_rotate expected.");

    DBUG_ASSERT ((strcmp (IDS_NAME (let_ids), ID_NAME (arg3))),
                 "a = rotate( ., ., a) not allowed!");

    ret_node = MakeAssignIcm3 ("ND_ALLOC_ARRAY", MakeBasetypeNode (IDS_TYPE (let_ids)),
                               DupIds_Id (let_ids), MakeNum (IDS_REFCNT (let_ids)));
    last_node = ret_node;

    icm_node = MakeAssignIcm5 ("ND_KD_ROT_CxSxA_A", DupNode (arg1), DupNode (arg2),
                               MakeNum (GetDim (ID_TYPE (arg3))), DupNode (arg3),
                               DupIds_Id (let_ids));
    last_node = ASSIGN_NEXT (last_node) = icm_node;

    DBUG_RETURN (ret_node);
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
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 ******************************************************************************/

node *
COMPPrf (node *arg_node, node *arg_info)
{
    node *ret_node = NULL;
    bool dec_rc_args = TRUE;

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
            /*
             * CAUTION: no DEC_RC on the args of F_reshape!!
             */
            dec_rc_args = FALSE;
            break;

        case F_add_SxA:
        case F_sub_SxA:
        case F_mul_SxA:
        case F_div_SxA:
            ret_node = COMPPrfArith_SxA (arg_node, arg_info);
            break;

        case F_add_AxS:
        case F_sub_AxS:
        case F_mul_AxS:
        case F_div_AxS:
            ret_node = COMPPrfArith_AxS (arg_node, arg_info);
            break;

        case F_add_AxA:
        case F_sub_AxA:
        case F_mul_AxA:
        case F_div_AxA:
            ret_node = COMPPrfArith_AxA (arg_node, arg_info);
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
        switch (PRF_PRF (arg_node)) {
        case F_toi_A:
        case F_tof_A:
        case F_tod_A:
            ret_node = COMPPrfConvertArr (arg_node, arg_info);
            break;

        case F_drop:
        case F_take:
            ret_node = COMPPrfTakeDrop (arg_node, arg_info);
            break;

        case F_cat:
            ret_node = COMPPrfCat (arg_node, arg_info);
            break;

        case F_rotate:
            ret_node = COMPPrfRotate (arg_node, arg_info);
            break;

        default:
            DBUG_ASSERT ((0), "illegal prf found!");
            break;
        }
    }

    DBUG_ASSERT ((ret_node != NULL), "no return value found!");

    if (dec_rc_args) {
        node *last_node, *icm_node;
        node *args, *arg;

        /*
         * append a DEC_RC for each arg
         */
        last_node = ret_node;
        while (ASSIGN_NEXT (last_node) != NULL) {
            last_node = ASSIGN_NEXT (last_node);
        }

        args = PRF_ARGS (arg_node);
        while (args != NULL) {
            arg = EXPRS_EXPR (args);
            if (NODE_TYPE (arg) == N_id) {
                icm_node
                  = MakeAdjustRcIcm (ID_NAME (arg), ID_TYPE (arg), ID_REFCNT (arg), -1);
                if (icm_node != NULL) {
                    last_node = ASSIGN_NEXT (last_node) = icm_node;
                }
            }
            args = EXPRS_NEXT (args);
        }
    }

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
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 ******************************************************************************/

static node *
COMPIdLet (node *arg_node, node *arg_info)
{
    ids *let_ids;
    node *icm_node;
    node *ret_node = NULL;

    DBUG_ENTER ("COMPIdLet");

    let_ids = INFO_COMP_LASTIDS (arg_info);

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
        DBUG_ASSERT ((ret_node != NULL), "ICMs missing, RCs inconsistent?");
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
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 ******************************************************************************/

static node *
COMPIdFromClass (node *arg_node, node *arg_info)
{
    ids *let_ids;
    node *icm_node, *last_node;
    node *ret_node = NULL;

    DBUG_ENTER ("COMPIdFromClass");

    let_ids = INFO_COMP_LASTIDS (arg_info);

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
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 ******************************************************************************/

static node *
COMPIdToClass (node *arg_node, node *arg_info)
{
    ids *let_ids;
    node *icm_node, *last_node;
    types *rhs_type;
    node *ret_node = NULL;

    DBUG_ENTER ("COMPIdToClass");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    rhs_type = ID_TYPE (arg_node);

    /*
     * 'arg_node' is non-unique and 'let_ids' is unique
     */
    if (RC_IS_ACTIVE (ID_REFCNT (arg_node))) {
        DBUG_ASSERT ((ID_REFCNT (arg_node) != 0), "Reference with (rc == 0) found!");

        if (ID_REFCNT (arg_node) == 1) {
            if (IsArray (rhs_type)) {
                ret_node = MakeAssignIcm3 ("ND_KS_MAKE_UNIQUE_ARRAY", DupNode (arg_node),
                                           DupIds_Id (let_ids),
                                           MakeNum (GetBasetypeSize (rhs_type)));
                last_node = ret_node;

                icm_node = MakeAssignIcm2 ("ND_SET_RC", DupIds_Id (let_ids),
                                           MakeNum (IDS_REFCNT (let_ids)));
                last_node = ASSIGN_NEXT (last_node) = icm_node;
            } else {
                ret_node = MakeAssignIcm3 ("ND_NO_RC_MAKE_UNIQUE_HIDDEN",
                                           DupNode (arg_node), DupIds_Id (let_ids),
                                           MakeId_Copy (GenericFun (0, rhs_type)));
                last_node = ret_node;
            }
        } else {
            if (IsArray (rhs_type)) {
                ret_node = MakeAssignIcm3 ("ND_KS_COPY_ARRAY", DupNode (arg_node),
                                           DupIds_Id (let_ids),
                                           MakeNum (GetBasetypeSize (rhs_type)));
                last_node = ret_node;

                icm_node = MakeAssignIcm1 ("ND_ALLOC_RC", DupIds_Id (let_ids));
                last_node = ASSIGN_NEXT (last_node) = icm_node;

                icm_node = MakeAssignIcm2 ("ND_SET_RC", DupIds_Id (let_ids),
                                           MakeNum (IDS_REFCNT (let_ids)));
                last_node = ASSIGN_NEXT (last_node) = icm_node;

                icm_node = MakeAssignIcm2 ("ND_DEC_RC", DupNode (arg_node), MakeNum (1));
                last_node = ASSIGN_NEXT (last_node) = icm_node;
            } else {
                ret_node = MakeAssignIcm3 ("ND_COPY_HIDDEN", DupNode (arg_node),
                                           DupIds_Id (let_ids),
                                           MakeId_Copy (GenericFun (0, rhs_type)));
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
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
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
        ret_node = NULL;
        DBUG_ASSERT (0, "unknown kind of class conversion function found");
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
 *   Compiles let expression with a constant array on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/

node *
COMPArray (node *arg_node, node *arg_info)
{
    node *ret_node, *icm_node, *last_node;
    node *array_elem;
    ids *let_ids;
    int num_elems;

    DBUG_ENTER ("COMPArray");

    let_ids = INFO_COMP_LASTIDS (arg_info);

    /* create ND_ALLOC_ARRAY */
    ret_node = MakeAssignIcm3 ("ND_ALLOC_ARRAY", MakeBasetypeNode (IDS_TYPE (let_ids)),
                               DupIds_Id (let_ids), MakeNum (IDS_REFCNT (let_ids)));
    last_node = ret_node;

    /*
     * Count number of array elements.
     */
    num_elems = CountExprs (ARRAY_AELEMS (arg_node));

    if (num_elems > 0) {
        array_elem = EXPRS_EXPR (ARRAY_AELEMS (arg_node));
    } else {
        array_elem = NULL;
    }

    if ((array_elem != NULL) && (NODE_TYPE (array_elem) == N_id)) {
        types *aelem_type = ID_TYPE (array_elem);

        DBUG_ASSERT ((ARRAY_STRING (arg_node) == NULL),
                     "Inconsistant string information found at N_array node!");

        if (IsArray (aelem_type)) {
            icm_node
              = MakeAssignIcm4 ("ND_CREATE_CONST_ARRAY_A", DupIds_Id (let_ids),
                                MakeNum (GetTypesLength (aelem_type)),
                                MakeNum (num_elems), DupTree (ARRAY_AELEMS (arg_node)));
        } else if (IsNonUniqueHidden (aelem_type)) {
            icm_node
              = MakeAssignIcm4 ("ND_CREATE_CONST_ARRAY_H", DupIds_Id (let_ids),
                                MakeId_Copy (GenericFun (0, aelem_type)),
                                MakeNum (num_elems), DupTree (ARRAY_AELEMS (arg_node)));
        } else {
            /* array elements are scalars */
            icm_node
              = MakeAssignIcm3 ("ND_CREATE_CONST_ARRAY_S", DupIds_Id (let_ids),
                                MakeNum (num_elems), DupTree (ARRAY_AELEMS (arg_node)));
        }
    } else {
        if (ARRAY_STRING (arg_node) != NULL) {
            /* array is a string */
            icm_node = MakeAssignIcm2 ("ND_CREATE_CONST_ARRAY_C", DupIds_Id (let_ids),
                                       MakeStr (ARRAY_STRING (arg_node)));
        } else {
            /* array elements are scalars */
            icm_node
              = MakeAssignIcm3 ("ND_CREATE_CONST_ARRAY_S", DupIds_Id (let_ids),
                                MakeNum (num_elems), DupTree (ARRAY_AELEMS (arg_node)));
        }
    }
    last_node = ASSIGN_NEXT (last_node) = icm_node;

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPLoop(node *arg_node, node *arg_info)
 *
 * Description:
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
 *
 * ### CODE NOT BRUSHED YET ###
 *
 ******************************************************************************/

node *
COMPLoop (node *arg_node, node *arg_info)
{
    node *ret_node, *first_node, *icm_node, *last_node;
    node *cond, *body;
    ids *usevar, *defvar;
    bool found;
    char *label_str = NULL;

    DBUG_ENTER ("COMPLoop");

    DBUG_ASSERT (((NODE_TYPE (DO_OR_WHILE_COND (arg_node)) == N_id)
                  || (NODE_TYPE (DO_OR_WHILE_COND (arg_node)) == N_bool)),
                 "loop condition is neither a N_id nor a N_bool node!");

    /*
     * DO_OR_WHILE_COND(arg_node) must not be traversed!
     */

    L_DO_OR_WHILE_BODY (arg_node, (Trav (DO_OR_WHILE_BODY (arg_node), arg_info)));

    /*
     * We will return a N_assign chain!!
     * Therefore we first build a new N_assign node containing the loop.
     */
    cond = DO_OR_WHILE_COND (arg_node);
    body = DO_OR_WHILE_BODY (arg_node);
    ret_node = MakeAssign ((NODE_TYPE (arg_node) == N_do) ? MakeDo (cond, body)
                                                          : MakeWhile (cond, body),
                           NULL);

    /*
     * Build icm's and insert them *before* the first instruction in the loop.
     * The following parts are done:
     *   (1) Build DEC_RC-icms
     *   (2) Build ND-label-icm (do-loop only)
     *   (3) Build INC_RC-icms
     *   (4) Insert all icms at beginning of inner instructions.
     */
    /* create a dummy node to append ICMs to */
    first_node = MakeAssign (NULL, NULL);
    last_node = first_node;
    /*
     * Before (1) - Build DEC_RC-icms.
     *
     * All variables defined but not used in the loop where counted during
     * refcounting with 1 (for while-loops) resp. with 0 (for do-loops).
     *
     * The variables inspected are defined in the loop (used after it) but not
     * used in the loop itself. In context of a while-loop this means the
     * variable has been set before the loop, otherwise, any usage after the
     * loop isn't inferable if the loop is not executed. The do-loop hos no
     * such problem.
     *
     * Such a defined but not used variable will be "overwritten" in another
     * following pass of the loop body, therefore the variables refcounters have
     * to be decremented before each pass.i Thats is what the icms here are
     * produced for. Because the first-pass of a do-loop does not involve any
     * predefined variables, it is necessary to jump around this part in such a
     * pass (therefore this obscure goto-label-construct.
     */
    defvar = DO_OR_WHILE_DEFVARS (arg_node);
    while (defvar != NULL) {
        /*
         * looking if defvar is in DEFVARS \ USEVARS,
         * meaning (! found) in usevars.
         */
        found = FALSE;
        usevar = DO_OR_WHILE_USEVARS (arg_node);
        while ((usevar != NULL) && (!found)) {
            found = found || (!strcmp (IDS_NAME (usevar), IDS_NAME (defvar)));
            usevar = IDS_NEXT (usevar);
        }
        if (!found) {
            /*
             * we don`t know the refcount of defvar in the current context,
             * so we use ND_DEC_RC_FREE_ARRAY.
             */
            icm_node = MakeAdjustRcIcm (IDS_NAME (defvar), IDS_TYPE (defvar),
                                        IDS_REFCNT (defvar), -1);
            if (icm_node != NULL) {
                last_node = ASSIGN_NEXT (last_node) = icm_node;
            }
        }

        defvar = IDS_NEXT (defvar);
    }
    /*
     * Before (2) - Build ND-label-icm (do-loop-only).
     *
     * Needed to avoid the above DEC_RC's in the first pass of a do_loop.
     * See explanations above.
     */
    if (NODE_TYPE (arg_node) == N_do) {
        label_str = TmpVarName (LABEL_POSTFIX);
        icm_node = MakeAssignIcm1 ("ND_LABEL", MakeId_Copy (label_str));
        last_node = ASSIGN_NEXT (last_node) = icm_node;
    }
    /*
     * Before (3) - Build INC_RC-icms.
     *
     * The variable used in the loop were refcounted once for the loop (plus
     * their occurences in the rest of the programm if not also defined in the loop).
     * If the loop is not executed this counter is reduced (icm's builded somewhere
     * else). But if the loop is executed the refcounters have to be adjusted to
     * the number of usages in the loop plus possible usage in another pass of the
     * loop. The extra counter for a next pass is also reduced after the loop.
     */
    usevar = DO_OR_WHILE_USEVARS (arg_node);
    while (usevar != NULL) {
        icm_node = MakeAdjustRcIcm (IDS_NAME (usevar), IDS_TYPE (usevar),
                                    IDS_REFCNT (usevar), IDS_REFCNT (usevar) - 1);
        if (icm_node != NULL) {
            last_node = ASSIGN_NEXT (last_node) = icm_node;
        }

        usevar = IDS_NEXT (usevar);
    }
    /*
     * Before (4) - Insert icms at beginning of loop instructions.
     */
    if (ASSIGN_NEXT (first_node) != NULL) {
        ASSIGN_NEXT (last_node) = BLOCK_INSTR (body);
        BLOCK_INSTR (body) = ASSIGN_NEXT (first_node);
        ASSIGN_NEXT (first_node) = NULL;
    }

    /*
     * Build icm's and insert them *after* the loop-assignment.
     * The following parts are done:
     *   (1) Build DEC_RC-icms
     *   (2) Build INC_RC-icms
     *   (3) Insert all icms after the loop-assignment
     *   (4) Insert GOTO (do-loop only)
     */
    last_node = first_node;

    /*
     * After (1) - Build DEC_RC-icms.
     */
    usevar = DO_OR_WHILE_USEVARS (arg_node);
    while (usevar != NULL) {
        /*
         * looking if usevar is in USEVARS \ DEFVARS,
         * meaning (! found) in usevars
         */
        found = FALSE;
        defvar = DO_OR_WHILE_DEFVARS (arg_node);
        while ((defvar != NULL) && (!found)) {
            found = found || (!strcmp (IDS_NAME (usevar), IDS_NAME (defvar)));
            defvar = IDS_NEXT (defvar);
        }

        if (!found) {
            icm_node = MakeAdjustRcIcm (IDS_NAME (usevar), IDS_TYPE (usevar),
                                        IDS_REFCNT (usevar), -1);
            if (icm_node != NULL) {
                last_node = ASSIGN_NEXT (last_node) = icm_node;
            }
        }

        usevar = IDS_NEXT (usevar);
    }

    /*
     * After (2) - Build INC_RC-icms.
     *             They increase RC of arrays that are defined
     *             in the loop and are used after it.
     */
    defvar = DO_OR_WHILE_DEFVARS (arg_node);
    while (defvar != NULL) {
        icm_node = MakeAdjustRcIcm (IDS_NAME (defvar), IDS_TYPE (defvar),
                                    IDS_REFCNT (defvar), IDS_REFCNT (defvar) - 1);
        if (icm_node != NULL) {
            last_node = ASSIGN_NEXT (last_node) = icm_node;
        }

        defvar = IDS_NEXT (defvar);
    }

    /*
     * After (3) - Insert icms after end of loop-assigment
     *             (before next instruction).
     */
    ASSIGN_NEXT (ret_node) = ASSIGN_NEXT (first_node);
    /* remove first dummy node */
    first_node = FreeNode (first_node);
    first_node = NULL;

    /*
     * After (4) - Insert GOTO before do-loop (do-loop only).
     */
    if (NODE_TYPE (arg_node) == N_do) {
        /* put N_icm 'ND_GOTO', in front of N_do node */
        icm_node = MakeAssignIcm1 ("ND_GOTO", MakeId_Copy (label_str));
        ASSIGN_NEXT (icm_node) = ret_node;
        ret_node = icm_node;
    }

    /*
     * The old 'arg_node' can be removed now.
     * We must set the sons to NULL here, in order to recycle them in
     * 'ret_node'!!!
     */
    L_DO_OR_WHILE_COND (arg_node, NULL);
    L_DO_OR_WHILE_BODY (arg_node, NULL);
    arg_node = FreeTree (arg_node);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPCond(node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiling a conditional, that should be easy, except for the fact
 *   that refcount corrections are needed in both branches.
 *
 *
 * ### CODE NOT BRUSHED YET ###
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

/******************************************************************************
 *
 * Function:
 *   node *COMPIcm( node *arg_node, node *arg_info)
 *
 * Description:
 *   Insert reference-counting ICMs for some ICM arguments!
 *   If new ICMs are inserted, the return value is a N_assign chain of ICMs.
 *
 ******************************************************************************/

node *
COMPIcm (node *arg_node, node *arg_info)
{
    node *args, *arg;
    node *icm_node, *last_node;
    char *name;

    DBUG_ENTER ("COMPIcm");

    name = ICM_NAME (arg_node);
    if (strstr (name, "VECT2OFFSET") != NULL) {
        /*
         * VECT2OFFSET( var, iv, ...) needs RC on all but the first argument.
         * It is expanded to    var = ... iv ...    , where 'var' is a scalar
         * variable (no reference-counted object).
         *  -> decrement the RCs of all but the first argument, if needed.
         */
        last_node = MakeAssign (arg_node, NULL);

        args = ICM_EXPRS2 (arg_node);
        while (args != NULL) {
            arg = EXPRS_EXPR (args);
            if (NODE_TYPE (arg) == N_id) {
                icm_node
                  = MakeAdjustRcIcm (ID_NAME (arg), ID_TYPE (arg), ID_REFCNT (arg), -1);
                if (icm_node != NULL) {
                    last_node = ASSIGN_NEXT (last_node) = icm_node;
                }
            }
            args = EXPRS_NEXT (args);
        }

        if (ASSIGN_NEXT (last_node) != NULL) {
            arg_node = last_node;
        } else {
            ASSIGN_INSTR (last_node) = NULL;
            last_node = FreeTree (last_node);
        }
    } else if (strstr (name, "USE_GENVAR_OFFSET") != NULL) {
        /*
         * USE_GENVAR_OFFSET( var, arr) does *not* consume its arguments!
         * It is expanded to    var = arr__off    , where 'var' is a scalar
         * and 'arr_off' an internal variable (no reference-counted objects)!
         *   -> do nothing
         */
    } else {
        DBUG_PRINT ("COMP", ("ICM not traversed: %s", ICM_NAME (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPCast( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles a N_cast node: The cast is simply removed.
 *
 ******************************************************************************/

node *
COMPCast (node *arg_node, node *arg_info)
{
    node *tmp;

    DBUG_ENTER ("COMPCast");

    tmp = arg_node;
    arg_node = Trav (CAST_EXPR (arg_node), arg_info);
    CAST_EXPR (tmp) = NULL;
    tmp = FreeTree (tmp);

    DBUG_RETURN (arg_node);
}

/**************************
 *
 *  with-loop
 *
 */

/******************************************************************************
 *
 * Function:
 *   node *BuildIcmArgs_WL_LOOP1( node *arg_node)
 *
 * Description:
 *   ICM args without 'step'.
 *
 ******************************************************************************/

static node *
BuildIcmArgs_WL_LOOP1 (node *arg_node)
{
    node *args;
    int dim;

    DBUG_ENTER ("BuildIcmArgs_WL_LOOP1");

    dim = WLNODE_DIM (arg_node);
    args = MakeExprs (
      MakeNum (dim),
      MakeExprs (
        DupIds_Id (NWITH2_VEC (wlnode)),
        MakeExprs (DupIds_Id (GetIndexIds (NWITH2_IDS (wlnode), dim)),
                   MakeExprs (NodeOrInt_MakeIndex (NODE_TYPE (arg_node),
                                                   WLNODE_GET_ADDR (arg_node, BOUND1),
                                                   dim, IDS_NAME (wlids), FALSE, FALSE),
                              MakeExprs (NodeOrInt_MakeIndex (NODE_TYPE (arg_node),
                                                              WLNODE_GET_ADDR (arg_node,
                                                                               BOUND2),
                                                              dim, IDS_NAME (wlids),
                                                              FALSE, FALSE),
                                         NULL)))));

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * Function:
 *   node *BuildIcmArgs_WL_LOOP2( node *arg_node)
 *
 * Description:
 *   ICM args with 'step'.
 *
 ******************************************************************************/

static node *
BuildIcmArgs_WL_LOOP2 (node *arg_node)
{
    node *args;

    DBUG_ENTER ("BuildIcmArgs_WL_LOOP2");

    args
      = MakeExprs (BuildIcmArgs_WL_LOOP1 (arg_node),
                   MakeExprs (NodeOrInt_MakeIndex (NODE_TYPE (arg_node),
                                                   WLBLOCKSTR_GET_ADDR (arg_node, STEP),
                                                   WLNODE_DIM (arg_node),
                                                   IDS_NAME (wlids), FALSE, FALSE),
                              NULL));

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * Function:
 *   node *BuildIcmArgs_WL_OP1( node *arg_node)
 *
 * Description:
 *   ICM args without names of loop variables.
 *
 ******************************************************************************/

static node *
BuildIcmArgs_WL_OP1 (node *arg_node)
{
    node *args;

    DBUG_ENTER ("BuildIcmArgs_WL_OP1");

    args = MakeExprs (MakeNum (DIM_NO_OFFSET (GetDim (IDS_TYPE (wlids)))),
                      MakeExprs (DupIds_Id (wlids),
                                 MakeExprs (DupIds_Id (NWITH2_VEC (wlnode)),
                                            MakeExprs (MakeNum (NWITH2_DIMS (wlnode)),
                                                       NULL))));

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * Function:
 *   node *BuildIcmArgs_WL_OP2( node *arg_node)
 *
 * Description:
 *   ICM args with names of loop variables.
 *
 ******************************************************************************/

static node *
BuildIcmArgs_WL_OP2 (node *arg_node)
{
    node *args;
    node *last_arg;
    ids *withid_ids;
    int num_args;

    DBUG_ENTER ("BuildIcmArgs_WL_OP2");

    args = BuildIcmArgs_WL_OP1 (arg_node);
    DBUG_ASSERT ((args != NULL), "no ICM args found!");
    last_arg = args;
    while (EXPRS_NEXT (last_arg) != NULL) {
        last_arg = EXPRS_NEXT (last_arg);
    }

    DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (last_arg)) == N_num), "wrong ICM arg found!");
    num_args = NUM_VAL (EXPRS_EXPR (last_arg));

    withid_ids = NWITH2_IDS (wlnode);
    while (withid_ids != NULL) {
        last_arg = EXPRS_NEXT (last_arg) = MakeExprs (DupIds_Id (withid_ids), NULL);
        num_args--;
        withid_ids = IDS_NEXT (withid_ids);
    }
    DBUG_ASSERT ((num_args == 0), "wrong number of ICM args found!");

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * Function:
 *   node *InsertIcm_MT_ADJUST_SCHEDULER( node *arg_node, node *assigns)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
InsertIcm_MT_ADJUST_SCHEDULER (node *arg_node, node *assigns)
{
    int dim;

    DBUG_ENTER ("InsertIcm_MT_ADJUST_SCHEDULER");

    DBUG_ASSERT (((NODE_TYPE (arg_node) == N_WLblock)
                  || (NODE_TYPE (arg_node) == N_WLublock)
                  || (NODE_TYPE (arg_node) == N_WLstride)
                  || (NODE_TYPE (arg_node) == N_WLstrideVar)),
                 "illegal WL-node found!");

    dim = WLNODE_DIM (arg_node);

    if ((!WLNODE_NOOP (arg_node)) && (WLNODE_LEVEL (arg_node) == 0) && NWITH2_MT (wlnode)
        && (SCHAdjustmentRequired (dim, wlseg))) {
        assigns
          = MakeAssign (MakeIcm6 ((NWITH2_OFFSET_NEEDED (wlnode))
                                    ? "MT_ADJUST_SCHEDULER__OFFSET"
                                    : "MT_ADJUST_SCHEDULER",
                                  DupIds_Id (wlids), MakeNum (WLSEGX_DIMS (wlseg)),
                                  MakeNum (dim),
                                  NodeOrInt_MakeIndex (NODE_TYPE (arg_node),
                                                       WLBLOCKSTR_GET_ADDR (arg_node,
                                                                            BOUND1),
                                                       dim, IDS_NAME (wlids), TRUE, TRUE),
                                  NodeOrInt_MakeIndex (NODE_TYPE (arg_node),
                                                       WLBLOCKSTR_GET_ADDR (arg_node,
                                                                            BOUND2),
                                                       dim, IDS_NAME (wlids), TRUE, TRUE),
                                  NodeOrInt_MakeIndex (NODE_TYPE (arg_node),
                                                       WLBLOCKSTR_GET_ADDR (arg_node,
                                                                            STEP),
                                                       dim, IDS_NAME (wlids), TRUE,
                                                       TRUE)),
                        assigns);
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *InsertIcm_WL_INIT_OFFSET( node *arg_node, node *assigns)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
InsertIcm_WL_INIT_OFFSET (node *arg_node, node *assigns)
{
    DBUG_ENTER ("InsertIcm_WL_INIT_OFFSET");

    if (NWITH2_OFFSET_NEEDED (wlnode)) {
        assigns = MakeAssign (MakeIcm1 ("WL_INIT_OFFSET", BuildIcmArgs_WL_OP1 (arg_node)),
                              assigns);
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *InsertIcm_WL_ADJUST_OFFSET( node *arg_node, node *assigns)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
InsertIcm_WL_ADJUST_OFFSET (node *arg_node, node *assigns)
{
    DBUG_ENTER ("InsertIcm_WL_ADJUST_OFFSET");

    if (NWITH2_OFFSET_NEEDED (wlnode)) {
        assigns
          = MakeAssign (MakeIcm2 ("WL_ADJUST_OFFSET", MakeNum (WLNODE_DIM (arg_node)),
                                  BuildIcmArgs_WL_OP2 (arg_node)),
                        assigns);
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *InsertIcm_WL_SET_OFFSET( node *arg_node, node *assigns)
 *
 * Description:
 *   Inserts the ICM WL_SET_OFFSET if needed.
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
InsertIcm_WL_SET_OFFSET (node *arg_node, node *assigns)
{
    int first_block_dim, first_ublock_dim, last_frac_dim;
    int dims, dim;
    int idx_min, idx_max;
    int d;
    shpseg *shape;
    int icm_dim = (-1);

    DBUG_ENTER ("InsertIcm_WL_SET_OFFSET");

    if (NWITH2_OFFSET_NEEDED (wlnode)) {
        dim = WLNODE_DIM (arg_node);
        dims = WLSEGX_DIMS (wlseg);

        if (NODE_TYPE (wlseg) == N_WLseg) {
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

            first_block_dim = MIN (first_block_dim, first_ublock_dim);
        } else {
            first_block_dim = dims;
        }

        /*
         * infer the last dimension for which the segment's domain is not the full
         * range (== -1, if the segment's domain equals the full index vector space)
         */
        shape = TYPES_SHPSEG (IDS_TYPE (wlids));
        d = dims - 1;

        while (d >= 0) {
            NodeOrInt_GetNameOrVal (NULL, &idx_min, NODE_TYPE (wlseg),
                                    WLSEGX_IDX_GET_ADDR (wlseg, IDX_MIN, d));
            NodeOrInt_GetNameOrVal (NULL, &idx_max, NODE_TYPE (wlseg),
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
         * check whether 'WL_SET_OFFSET' is needed in the current dimension or not
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
              = MakeAssign (MakeIcm3 ("WL_SET_OFFSET", MakeNum (dim), MakeNum (icm_dim),
                                      BuildIcmArgs_WL_OP2 (arg_node)),
                            assigns);
        }
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPNwith2( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compilation of a N_with2 node.
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
COMPNwith2 (node *arg_node, node *arg_info)
{
    node *fundef, *vardec, *icm_args, *fold_vardecs;
    node *let_neutral, *comp_neutral, *tmp, *new;
    node *old_wlnode;
    ids *old_wlids;
    char *icm_name_begin, *icm_name_end;
    char *profile_name;
    node *rc_icms_wlids = NULL;
    node *assigns = NULL;

    DBUG_ENTER ("COMPNwith2");

    /*
     * we must store the with-loop ids *before* compiling the codes
     *  because INFO_COMP_LASTIDS is possibly updated afterwards !!!
     */
    old_wlids = wlids; /* stack 'wlids' */
    wlids = INFO_COMP_LASTIDS (arg_info);
    old_wlnode = wlnode; /* stack 'wlnode' */
    wlnode = arg_node;

    if ((NWITH2_TYPE (arg_node) == WO_genarray)
        || (NWITH2_TYPE (arg_node) == WO_modarray)) {
        /*
         * genarray/modarray with-loop:
         *
         * Insert ICMs for memory management (ND_CHECK_REUSE_ARRAY, ND_ALLOC_ARRAY),
         * Note: In case of a fold with-loop, this is done automaticly when
         * compiling the init-assignment 'wlids = neutral' !!!
         */

        if (optimize & OPT_UIP) {
            /*
             * find all arrays, that possibly can be reused.
             */
            arg_node = GetReuseArrays (arg_node, INFO_COMP_FUNDEF (arg_info), wlids);

            /*
             * 'ND_CHECK_REUSE_ARRAY'
             */
            vardec = DFMGetMaskEntryDeclSet (NWITH2_REUSE (arg_node));
            while (vardec != NULL) {
                assigns = MakeAssign (MakeIcm2 ("ND_CHECK_REUSE_ARRAY",
                                                MakeId_Copy (VARDEC_OR_ARG_NAME (vardec)),
                                                DupIds_Id (wlids)),
                                      assigns);

                vardec = DFMGetMaskEntryDeclSet (NULL);
            }
            NWITH2_REUSE (arg_node) = DFMRemoveMask (NWITH2_REUSE (arg_node));
        }

        /*
         * 'ND_ALLOC_ARRAY'
         */
        assigns
          = AppendAssign (assigns, Ids2AllocArrayICMs_reuse (wlids, NULL,
                                                             NWITH2_PRAGMA (arg_node)));
    } else {
        /*
         * fold with-loop:
         *
         * Insert the vardecs of all special fold-funs into the vardec-chain of the
         * current function. Afterwards an update of the DFM-base is needed.
         * (This must be done here, because 'COMPSync()' needs the corrected masks)
         */

        fundef = INFO_COMP_FUNDEF (arg_info);
        fold_vardecs = GetFoldVardecs (NWITH2_FUNDEF (arg_node));
        if (fold_vardecs != NULL) {
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
    if (RC_IS_VITAL (IDS_REFCNT (NWITH2_VEC (arg_node)))) {
        assigns
          = AppendAssign (assigns, Ids2AllocArrayICMs (NWITH2_VEC (arg_node), NULL));
    }

    /*
     * compile all code blocks
     */
    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    /*
     * build arguments for  'WL_..._BEGIN'-ICM and 'WL_..._END'-ICM
     */
    icm_args = MakeExprs (DupIds_Id (wlids),
                          MakeExprs (DupIds_Id (NWITH2_VEC (wlnode)),
                                     MakeExprs (MakeNum (NWITH2_DIMS (arg_node)), NULL)));

    if (NWITH2_OFFSET_NEEDED (wlnode)) {
        icm_name_begin = "WL_BEGIN__OFFSET";
        icm_name_end = "WL_END__OFFSET";
    } else {
        icm_name_begin = "WL_BEGIN";
        icm_name_end = "WL_END";
    }

    switch (NWITH2_TYPE (arg_node)) {
    case WO_genarray:
        profile_name = "genarray";
        break;

    case WO_modarray:
        profile_name = "modarray";
        break;

    case WO_foldfun:
        /* here is no break missing! */
    case WO_foldprf:
        /****************************************
         * compile 'wlids = neutral' !!!
         */
        let_neutral = MakeLet (DupTree (NWITH2_NEUTRAL (arg_node)), DupOneIds (wlids));
        comp_neutral = Compile (let_neutral);
        if (NODE_TYPE (comp_neutral) != N_assign) {
            /*
             * compiled  'wlids = neutral'  is still a N_let node
             *  -> build a N_assign node
             */
            DBUG_ASSERT ((NODE_TYPE (let_neutral) == N_let),
                         "Compiled N_let node is neither a N_assign chain nor"
                         " a N_let node!");
            comp_neutral = MakeAssign (comp_neutral, NULL);
        }

        /*
         * All RC-ICMs on 'wlids' must be moved *behind* the WL-code!!
         * Therefore we collect them in 'rc_icms_wlids' to insert them later.
         */
        tmp = comp_neutral;
        while (ASSIGN_NEXT (tmp) != NULL) {
            if ((NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (tmp))) == N_icm)
                && ((!strcmp (ICM_NAME (ASSIGN_INSTR (ASSIGN_NEXT (tmp))), "ND_DEC_RC"))
                    || (!strcmp (ICM_NAME (ASSIGN_INSTR (ASSIGN_NEXT (tmp))),
                                 "ND_INC_RC"))
                    || (!strcmp (ICM_NAME (ASSIGN_INSTR (ASSIGN_NEXT (tmp))),
                                 "ND_DEC_RC_FREE_ARRAY")))) {
                new = ASSIGN_NEXT (tmp);
                ASSIGN_NEXT (tmp) = ASSIGN_NEXT (ASSIGN_NEXT (tmp));
                ASSIGN_NEXT (new) = NULL;
                rc_icms_wlids = AppendAssign (rc_icms_wlids, new);
            } else {
                tmp = ASSIGN_NEXT (tmp);
            }
        }

        /*
         * insert compiled assignments
         */
        assigns = AppendAssign (assigns, comp_neutral);

        profile_name = "fold";
        break;

    default:
        DBUG_ASSERT ((0), "illegal withop type found");
        icm_name_begin = icm_name_end = NULL;
        profile_name = NULL;
        break;
    }

    /*
     * create 'PF_BEGIN_WITH' (for profiling) and 'WL_..._BEGIN'-ICM
     */
    assigns
      = AppendAssign (assigns,
                      MakeAssign (MakeIcm1 ("PF_BEGIN_WITH", MakeId_Copy (profile_name)),
                                  MakeAssignIcm1 (icm_name_begin, icm_args)));

    /*
     * compile the with-segments
     *  -> we get an assignment-chain
     */
    assigns = AppendAssign (assigns, Trav (NWITH2_SEGS (arg_node), arg_info));

    /*
     * create PF_END_WITH (for profiling) and  'WL_END'-ICM
     */
    assigns
      = AppendAssign (assigns, MakeAssign (MakeIcm1 (icm_name_end, DupTree (icm_args)),
                                           MakeAssignIcm1 ("PF_END_WITH",
                                                           MakeId_Copy (profile_name))));

    /*
     * insert RC-ICMs from 'wlids = neutral'
     */
    assigns = AppendAssign (assigns, rc_icms_wlids);

    /*
     * insert ICMs for memory management ('DEC_RC_FREE')
     */
    assigns = AppendAssign (assigns, Ids2DecRcICMs (NWITH2_DEC_RC_IDS (arg_node), NULL));

    /*
     * insert 'DEC_RC_FREE'-ICM for index-vector
     */
    if (RC_IS_VITAL (IDS_REFCNT (NWITH2_VEC (arg_node)))) {
        assigns = AppendAssign (assigns, Ids2DecRcICMs (NWITH2_VEC (arg_node), NULL));
    }

    /*
     * pop 'wlids', 'wlnode'
     */
    wlids = old_wlids;
    wlnode = old_wlnode;

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPWLsegx( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compilation of a N_WLseg- or N_WLsegVar-node:
 *   Returns a N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *   (The whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *
 * remark:
 *   - 'wlseg' points to the current with-loop segment.
 *     (N_WLseg- *or* N_WLsegVar-node!)
 *
 ******************************************************************************/

node *
COMPWLsegx (node *arg_node, node *arg_info)
{
    node *assigns, *old_wlseg;

    DBUG_ENTER ("COMPWLsegx");

    /*
     * stack old 'wlseg'.
     * store pointer to current segment in 'wlseg'.
     */
    old_wlseg = wlseg;
    wlseg = arg_node;

    /*
     * compile the contents of the segment
     *  => we get an assignment-chain
     */
    assigns = Trav (WLSEGX_CONTENTS (arg_node), arg_info);

    assigns = InsertIcm_WL_INIT_OFFSET (arg_node, assigns);

    /*
     * Insert scheduling specific ICMs.
     */
    assigns
      = AppendAssign (assigns,
                      MakeAssign (SCHCompileSchedulingEnd (INFO_COMP_SCHEDULERID (
                                                             arg_info),
                                                           IDS_NAME (wlids),
                                                           WLSEGX_SCHEDULING (arg_node),
                                                           arg_node),
                                  NULL));
    assigns
      = MakeAssign (SCHCompileSchedulingBegin (INFO_COMP_SCHEDULERID (arg_info),
                                               IDS_NAME (wlids),
                                               WLSEGX_SCHEDULING (arg_node), arg_node),
                    assigns);

    /*
     * Collect initialization ICMs for schedulers.
     * This is only done for 'real' schedulers, not for the pseudo schedulers
     * used during sequential execution.
     */
    if (WLSEGX_SCHEDULING (arg_node) != NULL) {

        INFO_COMP_SCHEDULERINIT (arg_info)
          = MakeAssign (SCHCompileSchedulingInit (INFO_COMP_SCHEDULERID (arg_info),
                                                  IDS_NAME (wlids),
                                                  WLSEGX_SCHEDULING (arg_node), arg_node),
                        INFO_COMP_SCHEDULERINIT (arg_info));

        INFO_COMP_SCHEDULERID (arg_info) += 1;
    }

    /*
     * append compilat (assignment-chain) of next segment to 'assigns'
     */
    if (WLSEGX_NEXT (arg_node) != NULL) {
        assigns = AppendAssign (assigns, Trav (WLSEGX_NEXT (arg_node), arg_info));
    }

    /*
     * pop 'wlseg'.
     */
    wlseg = old_wlseg;

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPWLxblock( node *arg_node, node *arg_info)
 *
 * Description:
 *   compilation of a N_WLblock- or N_WLublock-node:
 *     returns a N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *
 * remarks:
 *   - 'wlids' points always to LET_IDS of the current with-loop.
 *   - 'wlnode' points always to the N_Nwith2-node.
 *   - 'wlseg' points to the current with-loop segment.
 *     (N_WLseg- *or* N_WLsegVar-node!)
 *
 ******************************************************************************/

node *
COMPWLxblock (node *arg_node, node *arg_info)
{
    int level, dim;
    bool is_block, mt_active, offset_needed;
    char *icm_name_begin = NULL;
    char *icm_name_end = NULL;
    node *assigns = NULL;

    DBUG_ENTER ("COMPWLxblock");

    level = WLXBLOCK_LEVEL (arg_node);
    dim = WLXBLOCK_DIM (arg_node);

    is_block = (NODE_TYPE (arg_node) == N_WLblock);
    mt_active = NWITH2_MT (wlnode);
    offset_needed = NWITH2_OFFSET_NEEDED (wlnode);

    /*******************************************
     * create ICMs for next dim / contents     *
     *******************************************/

    if (WLXBLOCK_NOOP (arg_node)) {
        assigns = InsertIcm_WL_ADJUST_OFFSET (arg_node, NULL);
    } else {
        if (WLXBLOCK_NEXTDIM (arg_node) != NULL) {
            DBUG_ASSERT ((WLXBLOCK_CONTENTS (arg_node) == NULL),
                         "CONTENTS and NEXTDIM used simultaneous!");

            assigns = Trav (WLXBLOCK_NEXTDIM (arg_node), arg_info);
        }

        if (WLXBLOCK_CONTENTS (arg_node) != NULL) {
            DBUG_ASSERT ((WLXBLOCK_NEXTDIM (arg_node) == NULL),
                         "CONTENTS and NEXTDIM used simultaneous!");

            assigns = Trav (WLXBLOCK_CONTENTS (arg_node), arg_info);
        }
    }

    /*******************************************
     * create ICMs for block loop              *
     *******************************************/

    if (WLXBLOCK_NOOP (arg_node)) {
        DBUG_ASSERT ((level == 0), "inner NOOP N_WL...-node found!");

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
        assigns = MakeAssign (MakeIcm1 (icm_name_begin, BuildIcmArgs_WL_LOOP2 (arg_node)),
                              assigns);
    }

    assigns = InsertIcm_MT_ADJUST_SCHEDULER (arg_node, assigns);

    if (icm_name_end != NULL) {
        assigns
          = AppendAssign (assigns, MakeAssignIcm1 (icm_name_end,
                                                   BuildIcmArgs_WL_LOOP2 (arg_node)));
    }

    /*******************************************
     * create ICMs for successor               *
     *******************************************/

    if (WLXBLOCK_NEXT (arg_node) != NULL) {
        assigns = AppendAssign (assigns, Trav (WLXBLOCK_NEXT (arg_node), arg_info));
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPWLstridex( node *arg_node, node *arg_info)
 *
 * Description:
 *   compilation of a N_WLstride- or N_WLstrideVar-node:
 *     returns a N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *
 * remarks:
 *   - 'wlids' points always to LET_IDS of the current with-loop.
 *   - 'wlnode' points always to the N_Nwith2-node.
 *   - 'wlseg' points to the current with-loop segment.
 *     (N_WLseg- *or* N_WLsegVar-node!)
 *   - 'wlstride' points to the current with-loop stride.
 *     (N_WLstride- *or* N_WLstrideVar-node!)
 *
 ******************************************************************************/

node *
COMPWLstridex (node *arg_node, node *arg_info)
{
    node *old_wlstride;
    node *new_assigns;
    int level, dim;
    int cnt_unroll, i;
    bool mt_active, offset_needed;
    char *icm_name_begin = NULL;
    char *icm_name_end = NULL;
    node *assigns = NULL;

    DBUG_ENTER ("COMPWLstridex");

    /*
     * stack old 'wlstride'.
     * store pointer to current segment in 'wlstride'.
     */
    old_wlstride = wlstride;
    wlstride = arg_node;

    level = WLSTRIDEX_LEVEL (arg_node);
    dim = WLSTRIDEX_DIM (arg_node);

    mt_active = NWITH2_MT (wlnode);
    offset_needed = NWITH2_OFFSET_NEEDED (wlnode);

    /*******************************************
     * create ICMs for next dim / contents     *
     *******************************************/

    if (WLSTRIDEX_NOOP (arg_node)) {
        assigns = InsertIcm_WL_ADJUST_OFFSET (arg_node, NULL);
    } else {
        if (WLSTRIDEX_CONTENTS (arg_node) != NULL) {
            assigns = Trav (WLSTRIDEX_CONTENTS (arg_node), arg_info);
        }
    }

    /*******************************************
     * create ICMs for stride loop             *
     *******************************************/

    if (WLSTRIDEX_NOOP (arg_node)) {
        DBUG_ASSERT ((level == 0), "inner NOOP N_WL...-node found!");

        if (offset_needed) {
            if (mt_active) {
                icm_name_begin = "WL_MT_STRIDE_NOOP_BEGIN";
                icm_name_end = "WL_MT_STRIDE_NOOP_END";
            } else {
                icm_name_begin = "WL_STRIDE_NOOP_BEGIN";
                icm_name_end = "WL_STRIDE_NOOP_END";
            }
        }
    } else if ((NODE_TYPE (arg_node) == N_WLstride) && (WLSTRIDE_UNROLLING (arg_node))) {
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

        new_assigns = NULL;
        DBUG_ASSERT ((((WLSTRIDE_BOUND2 (arg_node) - WLSTRIDE_BOUND1 (arg_node))
                       % WLSTRIDE_STEP (arg_node))
                      == 0),
                     "'step' is not a divisor of 'bound2 - bound1'!");
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
        assigns = MakeAssign (MakeIcm1 (icm_name_begin, BuildIcmArgs_WL_LOOP2 (arg_node)),
                              assigns);
    }

    assigns = InsertIcm_MT_ADJUST_SCHEDULER (arg_node, assigns);

    if (icm_name_end != NULL) {
        assigns
          = AppendAssign (assigns, MakeAssignIcm1 (icm_name_end,
                                                   BuildIcmArgs_WL_LOOP2 (arg_node)));
    }

    /*******************************************
     * create ICMs for successor               *
     *******************************************/

    if (WLSTRIDEX_NEXT (arg_node) != NULL) {
        assigns = AppendAssign (assigns, Trav (WLSTRIDEX_NEXT (arg_node), arg_info));
    }

    /*
     * restore old 'wlstride'.
     */
    wlstride = old_wlstride;

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPWLgridx( node *arg_node, node *arg_info)
 *
 * Description:
 *   compilation of a N_WLgrid- or N_WLgridVar-node:
 *     returns a N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *
 * remarks:
 *   - 'wlids' points always to LET_IDS of the current with-loop.
 *   - 'wlnode' points always to the N_with-node.
 *   - 'wlseg' points to the current with-loop segment.
 *     (N_WLseg- *or* N_WLsegVar-node!)
 *   - 'wlstride' points to the current with-loop stride.
 *     (N_WLstride- *or* N_WLstrideVar-node!)
 *
 ******************************************************************************/

node *
COMPWLgridx (node *arg_node, node *arg_info)
{
    node *icm_args, *cexpr;
    node *new_assigns;
    int dim;
    int cnt_unroll;
    bool mt_active, offset_needed, is_fitted;
    char *icm_name;
    char *icm_name_begin = NULL;
    char *icm_name_end = NULL;
    node *assigns = NULL;
    node *dec_rc_cexpr = NULL;

    DBUG_ENTER ("COMPWLgridx");

    dim = WLGRIDX_DIM (arg_node);

    mt_active = NWITH2_MT (wlnode);
    offset_needed = NWITH2_OFFSET_NEEDED (wlnode);
    is_fitted = WLGRID_FITTED (arg_node);

    if (WLGRIDX_NOOP (arg_node)) {
        assigns = InsertIcm_WL_ADJUST_OFFSET (arg_node, NULL);
    } else {

        /*******************************************
         * create ICMs for next dim                *
         *******************************************/

        if (WLGRIDX_NEXTDIM (arg_node) != NULL) {
            DBUG_ASSERT ((WLGRIDX_CODE (arg_node) == NULL),
                         "CODE and NEXTDIM used simultaneous!");

            assigns = InsertIcm_WL_SET_OFFSET (arg_node, NULL);

            assigns = AppendAssign (assigns, Trav (WLGRIDX_NEXTDIM (arg_node), arg_info));
        } else {

            /*******************************************
             * create ICMs for code block              *
             *******************************************/

            if (WLGRIDX_CODE (arg_node) == NULL) {
                /*
                 * no code found
                 */

                /*
                 * choose right ICM
                 */
                switch (NWITH2_TYPE (wlnode)) {
                case WO_genarray:
                    DBUG_ASSERT ((offset_needed), "wrong value for OFFSET_NEEDED found!");
                    icm_name = "WL_ASSIGN__INIT";
                    icm_args = BuildIcmArgs_WL_OP2 (arg_node);
                    break;

                case WO_modarray:
                    DBUG_ASSERT ((offset_needed), "wrong value for OFFSET_NEEDED found!");
                    icm_name = "WL_ASSIGN__COPY";
                    icm_args = MakeExprs (DupNode (NWITH2_ARRAY (wlnode)),
                                          BuildIcmArgs_WL_OP2 (arg_node));
                    break;

                case WO_foldfun:
                    /* here is no break missing! */
                case WO_foldprf:
                    DBUG_ASSERT ((0), "illegal NOOP value found!");
                    icm_name = NULL;
                    icm_args = NULL;
                    break;

                default:
                    DBUG_ASSERT ((0), "illegal withop type found!");
                    icm_name = NULL;
                    icm_args = NULL;
                    break;
                }
            } else {
                /*
                 * insert compiled code.
                 */
                cexpr = WLGRIDX_CEXPR (arg_node);
                DBUG_ASSERT ((cexpr != NULL), "no code expr found");

                DBUG_ASSERT ((WLGRIDX_CBLOCK (arg_node) != NULL),
                             "no code block found in N_Ncode node");
                DBUG_ASSERT ((WLGRIDX_CBLOCK_INSTR (arg_node) != NULL),
                             "first instruction of block is NULL"
                             " (should be a N_empty node)");

                if (NODE_TYPE (WLGRIDX_CBLOCK_INSTR (arg_node)) != N_empty) {
                    assigns = DupTree (WLGRIDX_CBLOCK_INSTR (arg_node));
                }

                /*
                 * choose right ICM
                 */
                switch (NWITH2_TYPE (wlnode)) {
                case WO_genarray:
                    /* here is no break missing! */
                case WO_modarray:
                    DBUG_ASSERT ((offset_needed), "wrong value for OFFSET_NEEDED found!");
                    DBUG_ASSERT ((NODE_TYPE (cexpr) == N_id), "code expr is not a id");

                    icm_name = "WL_ASSIGN";
                    icm_args
                      = MakeExprs (MakeNum (DIM_NO_OFFSET (GetDim (ID_TYPE (cexpr)))),
                                   MakeExprs (DupNode (cexpr),
                                              BuildIcmArgs_WL_OP2 (arg_node)));
                    /*
                     * we must decrement the RC of 'cexpr' (consumed argument)
                     */
                    dec_rc_cexpr = MakeAdjustRcIcm (ID_NAME (cexpr), ID_TYPE (cexpr),
                                                    ID_REFCNT (cexpr), -1);
                    break;

                case WO_foldfun:
                    /* here is no break missing! */
                case WO_foldprf:
                    if (offset_needed) {
                        icm_name = "WL_FOLD__OFFSET";
                    } else {
                        icm_name = "WL_FOLD";
                    }
                    icm_args = BuildIcmArgs_WL_OP2 (arg_node);

                    /*
                     * insert code of the special fold-fun
                     */
                    assigns
                      = AppendAssign (assigns, GetFoldCode (NWITH2_FUNDEF (wlnode)));
                    break;

                default:
                    DBUG_ASSERT ((0), "illegal withop type found");
                    icm_name = NULL;
                    icm_args = NULL;
                    break;
                }
            }

            assigns = AppendAssign (assigns, MakeAssign (MakeIcm1 (icm_name, icm_args),
                                                         dec_rc_cexpr));
        }
    }

    /*******************************************
     * create ICMs for grid loop               *
     *******************************************/

    /*
     * if the index-vector is needed somewhere in the code-blocks and this is
     * not a dummy grid (init, copy, noop), we must add the ICM 'WL_SET_IDXVEC'.
     */
    if ((RC_IS_VITAL (IDS_REFCNT (NWITH2_VEC (wlnode)))) && (!WLGRIDX_NOOP (arg_node))
        && ((WLGRIDX_CODE (arg_node) != NULL) || (WLGRIDX_NEXTDIM (arg_node) != NULL))) {
        assigns
          = MakeAssign (MakeIcm1 ("WL_SET_IDXVEC", BuildIcmArgs_WL_LOOP1 (arg_node)),
                        assigns);
    }

    if (WLGRIDX_NOOP (arg_node)) {
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
        if ((NODE_TYPE (arg_node) == N_WLgrid)
            && ((WLGRID_UNROLLING (arg_node))
                || ((WLGRID_BOUND2 (arg_node) - WLGRID_BOUND1 (arg_node) == 1)
                    && is_fitted))) {
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

            new_assigns = NULL;
            cnt_unroll = WLGRID_BOUND2 (arg_node) - WLGRID_BOUND1 (arg_node);
            for (; cnt_unroll > 1; cnt_unroll--) {
                new_assigns = AppendAssign (new_assigns, DupTree (assigns));
            }
            assigns = AppendAssign (new_assigns, assigns);
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
        assigns = MakeAssign (MakeIcm1 (icm_name_begin, BuildIcmArgs_WL_LOOP1 (arg_node)),
                              assigns);
    }

    if (icm_name_end != NULL) {
        assigns
          = AppendAssign (assigns, MakeAssignIcm1 (icm_name_end,
                                                   BuildIcmArgs_WL_LOOP1 (arg_node)));
    }

    /*******************************************
     * create ICMs for successor               *
     *******************************************/

    if (WLGRIDX_NEXT (arg_node) != NULL) {
        assigns = AppendAssign (assigns, Trav (WLGRIDX_NEXT (arg_node), arg_info));
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPNcode( node *arg_node, node *arg_info)
 *
 * Description:
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
            /* remove a N_empty node */
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

/**************************
 *
 *  MT
 *
 */

/******************************************************************************
 *
 * Function:
 *   node *COMPSpmd( node *arg_node, node *arg_info)
 *
 * Description:
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

    DBUG_ASSERT ((NODE_TYPE (FUNDEF_ICM (SPMD_FUNDEF (arg_node))) == N_icm),
                 "SPMD function MUST be compiled prior to compilation of "
                 "corresponding SMPD block.");

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

#if 0
  icm_args = BLOCK_SPMD_SETUP_ARGS(FUNDEF_BODY(SPMD_FUNDEF(arg_node)));
#endif
    num_args = 0;
    icm_args = NULL; /* dkr: should be FreeTree( icm_args) ?!? */
    icm_args = BuildParamsByDFM (SPMD_IN (arg_node), "in", &num_args, icm_args);
    icm_args = BuildParamsByDFM (SPMD_OUT (arg_node), "out", &num_args, icm_args);
    icm_args = BuildParamsByDFM (SPMD_SHARED (arg_node), "shared", &num_args, icm_args);

    icm_args = MakeExprs (MakeId_Copy (FUNDEF_NAME (SPMD_FUNDEF (arg_node))),
                          MakeExprs (MakeNum (num_args), icm_args));

    assigns = MakeAssign (MakeIcm1 ("MT_SPMD_SETUP", icm_args), assigns);

    assigns = AppendAssign (BLOCK_SPMD_PROLOG_ICMS (FUNDEF_BODY (SPMD_FUNDEF (arg_node))),
                            assigns);

    assigns = AppendAssign (BLOCK_SCHEDULER_INIT (FUNDEF_BODY (SPMD_FUNDEF (arg_node))),
                            assigns);

    SPMD_ICM_PARALLEL (arg_node) = MakeBlock (assigns, NULL);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   char *GetFoldTypeTag( ids *with_ids)
 *
 * Description:
 *
 *
 ******************************************************************************/

static char *
GetFoldTypeTag (ids *with_ids)
{
    char *fold_type;
    types *type;
    simpletype btype;

    DBUG_ENTER ("GetFoldTypeTag");

    type = IDS_TYPE (with_ids);
    if (TYPES_DIM (type) > 0) {
        if (RC_IS_ACTIVE (IDS_REFCNT (with_ids))) {
            fold_type = "array_rc";
        } else {
            fold_type = "array";
        }
    } else {
        btype = GetBasetype (type);
        if (btype == T_user) {
            fold_type = "hidden";
        } else {
            fold_type = type_string[btype];
        }
    }

    DBUG_RETURN (fold_type);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPSync( node *arg_node, node *arg_info)
 *
 * Description:
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
        if (RC_IS_ACTIVE (VARDEC_OR_ARG_REFCNT (vardec))) {
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

    last_sync = INFO_COMP_LASTSYNC (arg_info);
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

                if (NWITH2_IS_FOLD (with)) {
                    num_fold_args++;

                    fold_type = GetFoldTypeTag (with_ids);

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
        if ((NODE_TYPE (with) == N_Nwith2) && NWITH2_IS_FOLD (with)) {
            /*
             *  Increase the number of fold-with-loops.
             */
            num_barrier_args++;

            /*
             * create fold_type-tag
             */
            fold_type = GetFoldTypeTag (with_ids);

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
            if ((!strcmp (ICM_NAME (instr), "WL_BEGIN__OFFSET"))
                || (!strcmp (ICM_NAME (instr), "WL_BEGIN"))) {
                /*
                 *  begin of with-loop code found
                 *  -> skip with-loop code
                 *  -> stack nested with-loops
                 */
                count_nesting++;
                DBUG_PRINT ("COMP", ("ICM: %s is ++", ICM_NAME (instr)));
            } else if ((!strcmp (ICM_NAME (instr), "WL_END__OFFSET"))
                       || (!strcmp (ICM_NAME (instr), "WL_END"))) {
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
                 *  is this a 'prolog-icm' or 'epilog-icm' for memory management?
                 *
                 *  prolog == TRUE: current icm (assignment) is part of prolog
                 *  epilog == TRUE: current icm (assignment) is part of epilog
                 */

                if ((!strcmp (ICM_NAME (instr), "ND_ALLOC_ARRAY"))
                    || (!strcmp (ICM_NAME (instr), "ND_CHECK_REUSE_ARRAY"))
                    || (!strcmp (ICM_NAME (instr), "ND_CHECK_REUSE_HIDDEN"))) {
                    var_name = ID_NAME (ICM_ARG2 (instr));
                    prolog = TRUE;
                    epilog = FALSE;
                    DBUG_PRINT ("COMP", ("ICM: %s is prolog", ICM_NAME (instr)));
                } else if (!strcmp (ICM_NAME (instr), "ND_INC_RC")) {
                    var_name = ID_NAME (ICM_ARG1 (instr));
                    prolog = TRUE;
                    epilog = FALSE;
                    DBUG_PRINT ("COMP", ("ICM: %s is prolog", ICM_NAME (instr)));
                } else if ((!strcmp (ICM_NAME (instr), "ND_DEC_RC_FREE_ARRAY"))
                           || (!strcmp (ICM_NAME (instr), "ND_DEC_RC"))) {
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
     * insert ICM 'MT_START_SYNCBLOCK' and contents of modified sync-region-block
     */
    assigns
      = AppendAssign (assigns, MakeAssign (MakeIcm2 ("MT_START_SYNCBLOCK",
                                                     MakeNum (barrier_id), icm_args3),
                                           BLOCK_INSTR (SYNC_REGION (arg_node))));

    /*
     *  see comment on setting backup!
     *  the modified code is now attached to another tree-part.
     */
    BLOCK_INSTR (SYNC_REGION (arg_node)) = backup;

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
     *  Exchanging LASTSYNC, a free the last one.
     *  Will be needed for next N_sync, if no next N_sync exists COMPFundef
     *  will take care of this tree.
     */
    if (INFO_COMP_LASTSYNC (arg_info) != NULL) {
        INFO_COMP_LASTSYNC (arg_info) = FreeTree (INFO_COMP_LASTSYNC (arg_info));
    }
    INFO_COMP_LASTSYNC (arg_info) = arg_node;

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
 * Function:
 *   node *COMPMt( node *arg_node, node *arg_info)
 *
 * Description:
 *   compiles a N_mt-node
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
 * Function:
 *   node *COMPSt( node *arg_node, node *arg_info)
 *
 * Description:
 *   compiles a N_st-node
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
#if 0
    allocate  = MakeIcm2( "MT2_ALLOCATE",
                          MakeNum( DFMTestMask( ST_ALLOC( arg_node))),
                          BuildParamsByDFM( ST_ALLOC( arg_node), "alloc", NULL, NULL));
#endif
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
        result = NULL;
        DBUG_ASSERT (0, "can not handle such a function");
    }

    bset = DFMRemoveMask (bset);

    arg_node = FreeTree (arg_node);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPMTsignal( node *arg_node, node *arg_info)
 *
 * Description:
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
 * Function:
 *   node *COMPMTalloc( node *arg_node, node *arg_info)
 *
 * Description:
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
        broadcast_icm = receive_icm = NULL;
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
        broadcast_icm = receive_icm = NULL;
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
