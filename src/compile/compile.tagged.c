/*
 *
 * $Log$
 * Revision 1.15  2002/06/06 18:23:22  dkr
 * some bugs fixed
 *
 * Revision 1.14  2002/06/02 21:39:02  dkr
 * some more TAGGED_ARRAYS stuff added
 *
 * Revision 1.13  2002/05/31 17:38:08  dkr
 * bug about TAGGED_ARRAYS fixed
 *
 * Revision 1.12  2002/05/31 17:26:27  dkr
 * new argtags for TAGGED_ARRAYS used
 *
 * Revision 1.11  2002/04/16 21:16:18  dkr
 * AddThreadIdVardec() no longer needed.
 * This is done by GSCPrintMain() now.
 *
 * Revision 1.10  2002/04/03 14:47:50  dkr
 * COMPAp() renamed into COMP2Ap()
 *
 * Revision 1.9  2002/04/03 14:11:09  dkr
 * code updated (compile.c)
 *
 * Revision 1.8  2002/03/07 02:22:17  dkr
 * definition for INFO_COMP_FIRSTASSIGN added
 *
 * Revision 1.7  2002/03/01 03:20:27  dkr
 * minor changes done
 *
 * Revision 1.6  2002/02/22 13:48:19  dkr
 * error in COMPMT2FunReturn() corrected
 *
 * Revision 1.5  2002/02/20 14:57:01  dkr
 * fundef DupTypes() renamed into DupAllTypes()
 *
 * Revision 1.1  2001/12/10 15:34:14  dkr
 * Initial revision
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
#include "NameTuplesUtils.h"
#include "scheduling.h"
#include "wl_bounds.h"
#include "refcount.h"
#include "compile.h"
#include "compile.tagged.h"

/*
 * arg_info
 * ========
 *
 * Other:
 *
 *   INFO_COMP_MODUL       : pointer to current modul
 *   INFO_COMP_FUNDEF      : pointer to current fundef
 *
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
 *   node *DupExprs_NT( node *exprs)
 *
 * Description:
 *   Duplicates a N_exprs chain and transforms all N_id nodes found into
 *   tagged N_id nodes.
 *
 ******************************************************************************/

node *
DupExprs_NT (node *exprs)
{
    node *expr;
    node *new_exprs = NULL;

    DBUG_ENTER ("DupExprs");

    if (exprs != NULL) {
        DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "no N_exprs node found!");

        expr = EXPRS_EXPR (exprs);
        DBUG_ASSERT ((expr != NULL), "N_exprs node contains no data!");

        new_exprs
          = MakeExprs ((NODE_TYPE (expr) == N_id) ? DupId_NT (expr) : DupNode (expr),
                       DupExprs_NT (EXPRS_NEXT (exprs)));
    }

    DBUG_RETURN (new_exprs);
}

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
 *   node *MakeBasetypeArg( types *type)
 *
 * Description:
 *   Creates a new N_id node containing the basetype string of the given type.
 *
 ******************************************************************************/

static node *
MakeBasetypeArg (types *type)
{
    node *ret_node;
    char *str;

    DBUG_ENTER ("MakeBasetypeArg");

    str = GetBasetypeStr (type);

    ret_node = MakeId_Copy (str);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeBasetypeArg_NT( types *type)
 *
 * Description:
 *   Creates a new N_id node containing the basetype string of the given type.
 *
 ******************************************************************************/

static node *
MakeBasetypeArg_NT (types *type)
{
    node *ret_node;
    char *str;

    DBUG_ENTER ("MakeBasetypeArg_NT");

    str = GetBasetypeStr (type);

    ret_node = MakeId_Copy_NT (str, type);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeTypeArgs( char *name, types *type,
 *                       bool add_type, bool add_dim, bool add_shape)
 *
 * Description:
 *   Creates a chain of N_exprs nodes containing name, type, dimensionality
 *   and shape components of the given object.
 *
 ******************************************************************************/

static node *
MakeTypeArgs (char *name, types *type, bool add_type, bool add_dim, bool add_shape)
{
    int dim;
    node *ret_node = NULL;

    DBUG_ENTER ("MakeTypeArgs");

    dim = GetShapeDim (type);

    /*
     * CAUTION:
     * It is important that (dim <= 0) is hold for AKD and AUD arrays
     * otherwise the VARINT-interpretation of the shape-args would fail
     * during icm2c!!!
     */
    if (add_shape) {
        if (dim == 0) {
            /* SCL */
            ret_node = NULL;
        } else if (dim > 0) {
            /* AKS */
            ret_node = Type2Exprs (type);
        } else if (dim < KNOWN_DIM_OFFSET) {
            /* AKD */
            ret_node = NULL;
        } else {
            /* AUD */
            ret_node = NULL;
        }
    }

    if (add_dim) {
        ret_node = MakeExprs (MakeNum (dim), ret_node);
    }

    if (add_type) {
        ret_node = MakeExprs (MakeBasetypeArg (type), ret_node);
    }

    ret_node = MakeExprs (MakeId_Copy_NT (name, type), ret_node);

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

#if 0

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

node *GetFoldCode( node *fundef)
{
  node *fold_code;
  node *tmp;

  DBUG_ENTER( "GetFoldCode");

  DBUG_ASSERT( (fundef != NULL), "no fundef found!");
  DBUG_ASSERT( (NODE_TYPE( fundef) == N_fundef), "fold-fun corrupted!");
  DBUG_ASSERT( (FUNDEF_STATUS( fundef) == ST_foldfun), "no fold-fun found!");

  /*
   * get code of the special fold-fun
   */
  fold_code = DupTree( FUNDEF_INSTR( fundef));

  /*
   * remove declaration-ICMs ('ND_DECL_ARG') from code.
   */
  while ((NODE_TYPE( ASSIGN_INSTR( fold_code)) == N_icm) &&
         (! strcmp( ICM_NAME( ASSIGN_INSTR( fold_code)), "ND_DECL_ARG"))) {
    fold_code = FreeNode( fold_code);
  }

  /*
   * remove return-ICMs ('ND_FUN_RET') from code
   * (it is the last assignment)
   */
  tmp = fold_code;
  DBUG_ASSERT( (ASSIGN_NEXT( tmp) != NULL), "corrupted fold code found!");
  while (ASSIGN_NEXT( ASSIGN_NEXT( tmp)) != NULL) {
    tmp = ASSIGN_NEXT( tmp);
  }
  DBUG_ASSERT( ((NODE_TYPE( ASSIGN_INSTR( ASSIGN_NEXT( tmp))) == N_icm) &&
                (! strcmp( ICM_NAME( ASSIGN_INSTR( ASSIGN_NEXT( tmp))),
                           "ND_FUN_RET"))),
               "no ND_FUN_RET icm found in fold code!");
  ASSIGN_NEXT( tmp) = FreeNode( ASSIGN_NEXT( tmp));

  DBUG_RETURN( fold_code);
}

#endif

/******************************************************************************
 *
 * Function:
 *   node *GetFoldVardecs( node *fundef)
 *
 * Description:
 *   returns the vardecs of the special fold-fun 'fundef'.
 *
 ******************************************************************************/

static node *
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
 *   node *MakeAllocDescIcm( char *name, types *type, int rc, node *assigns)
 *
 * Description:
 *   Builds a ND_ALLOC_DESC( name) icm if needed.
 *
 ******************************************************************************/

static node *
MakeAllocDescIcm (char *name, types *type, int rc, node *assigns)
{
    DBUG_ENTER ("MakeAllocDescIcm");

    DBUG_ASSERT ((RC_IS_LEGAL (rc)), "illegal RC value found!");

    if (RC_IS_ACTIVE (rc)) {
        assigns = MakeAssignIcm1 ("ND_ALLOC_DESC", MakeId_Copy_NT (name, type), assigns);
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeSetRcIcm( char *name, types *type, int rc, node *assigns)
 *
 * Description:
 *   Builds a ND_SET_RC( name, num) icm if needed.
 *
 ******************************************************************************/

static node *
MakeSetRcIcm (char *name, types *type, int rc, node *assigns)
{
    DBUG_ENTER ("MakeSetRcIcm");

    DBUG_ASSERT ((RC_IS_LEGAL (rc)), "illegal RC value found!");

    if (RC_IS_ACTIVE (rc)) {
        if (RC_IS_VITAL (rc)) {
            assigns = MakeAssignIcm2 ("ND_SET_RC", MakeId_Copy_NT (name, type),
                                      MakeNum (rc), assigns);
        } else {
            assigns = MakeAssignIcm2 ("ND_FREE", MakeId_Copy_NT (name, type),
                                      MakeId_Copy (GenericFun (1, type)), assigns);
        }
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeIncRcIcm( char *name, types *type, int rc, int num,
 *                       node *assigns)
 *
 * Description:
 *   Builds a ND_INC_RC( name, num) icm if needed.
 *
 ******************************************************************************/

static node *
MakeIncRcIcm (char *name, types *type, int rc, int num, node *assigns)
{
    DBUG_ENTER ("MakeIncRcIcm");

    DBUG_ASSERT ((RC_IS_LEGAL (rc)), "illegal RC value found!");

    if (RC_IS_ACTIVE (rc)) {
        DBUG_ASSERT ((num >= 0), "increment for rc must be >= 0.");

        if (num > 0) {
            assigns = MakeAssignIcm2 ("ND_INC_RC", MakeId_Copy_NT (name, type),
                                      MakeNum (num), assigns);
        }
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeDecRcIcm( char *name, types *type, int rc, int num,
 *                       node *assigns)
 *
 * Description:
 *   According to 'type', 'rc' and 'num', builds either
 *             a ND_DEC_RC( name, num) icm,
 *          or a ND_DEC_RC_FREE( name, num, freefun) icm,
 *          or no icm at all.
 *
 ******************************************************************************/

static node *
MakeDecRcIcm (char *name, types *type, int rc, int num, node *assigns)
{
    DBUG_ENTER ("MakeDecRcIcm");

    DBUG_ASSERT ((RC_IS_LEGAL (rc)), "illegal RC value found!");

    if (RC_IS_ACTIVE (rc)) {
        DBUG_ASSERT ((num >= 0), "decrement for rc must be >= 0.");

        if (num > 0) {
            if (RC_IS_VITAL (rc - num)) { /* definitely no FREE needed? */
                assigns = MakeAssignIcm2 ("ND_DEC_RC", MakeId_Copy_NT (name, type),
                                          MakeNum (num), assigns);
            } else {
                assigns = MakeAssignIcm3 ("ND_DEC_RC_FREE", MakeId_Copy_NT (name, type),
                                          MakeNum (num),
                                          MakeId_Copy (GenericFun (1, type)), assigns);
            }
        }
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeAdjustRcIcm( char *name, types *type, int rc, int num,
 *                          node *assigns)
 *
 * Description:
 *   According to num, either a ND_INC_RC( varname, num) icm,
 *                       or   no ICM at all,
 *                       or   a ND_DEC_RC_...( varname, -num) icm
 *   is created.
 *
 ******************************************************************************/

static node *
MakeAdjustRcIcm (char *name, types *type, int rc, int num, node *assigns)
{
    DBUG_ENTER ("MakeAdjustRcIcm");

    DBUG_ASSERT ((RC_IS_LEGAL (rc)), "illegal RC value found!");

    if (num > 0) {
        assigns = MakeIncRcIcm (name, type, rc, num, assigns);
    } else if (num < 0) {
        assigns = MakeDecRcIcm (name, type, rc, -num, assigns);
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *Ids2IncRcIcms( ids *ids_chain, node *assigns)
 *
 * Description:
 *   Builds a ND_INC_RC( name, rc) icm for each ids in 'ids_chain'.
 *   The given node 'assigns' is appended to the created assign-chain.
 *
 ******************************************************************************/

static node *
Ids2IncRcIcms (ids *ids_chain, node *assigns)
{
    DBUG_ENTER ("Ids2IncRcIcms");

    while (ids_chain != NULL) {
        assigns = MakeIncRcIcm (IDS_NAME (ids_chain), IDS_TYPE (ids_chain),
                                IDS_REFCNT (ids_chain), IDS_REFCNT (ids_chain), assigns);

        ids_chain = IDS_NEXT (ids_chain);
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *Ids2DecRcIcms( ids *ids_chain, node *assigns)
 *
 * Description:
 *   According to rc and type, builds either
 *             a ND_DEC_RC( name, 1)
 *          or a ND_DEC_RC_FREE( name, 1, freefun)
 *   icm for each ids in 'ids_chain'.
 *   The given node 'assigns' is appended to the created assign-chain.
 *
 ******************************************************************************/

static node *
Ids2DecRcIcms (ids *ids_chain, node *assigns)
{
    DBUG_ENTER ("Ids2DecRcIcms");

    while (ids_chain != NULL) {
        assigns = MakeDecRcIcm (IDS_NAME (ids_chain), IDS_TYPE (ids_chain),
                                IDS_REFCNT (ids_chain), 1, assigns);

        ids_chain = IDS_NEXT (ids_chain);
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeAllocIcm( char *name, types *type, int rc,
 *                       node *pragma, node *assigns)
 *
 * Description:
 *   Builds a ND_ALLOC icm.
 *   The given node 'assigns' is appended to the created assignment.
 *
 *   CAUTION: Do not use this function in conjunction with a
 *            'ND_CHECK_REUSE' icm.
 *            Use 'MakeAllocIcm_IncRc()' instead!!
 *
 ******************************************************************************/

static node *
MakeAllocIcm (char *name, types *type, int rc, node *pragma, node *assigns)
{
    node *alloc_icm;

    DBUG_ENTER ("MakeAllocIcm");

    DBUG_ASSERT ((RC_IS_LEGAL (rc)), "illegal RC value found!");

    if (RC_IS_ACTIVE (rc)) {
        if (pragma == NULL) {
            alloc_icm = MakeIcm2 ("ND_ALLOC", MakeTypeArgs (name, type, TRUE, TRUE, TRUE),
                                  MakeNum (rc));
        } else {
            alloc_icm
              = MakeIcm5 ("ND_ALLOC_PLACE", MakeTypeArgs (name, type, TRUE, TRUE, TRUE),
                          MakeNum (rc), DupNode (AP_ARG1 (PRAGMA_APL (pragma))),
                          DupNode (AP_ARG2 (PRAGMA_APL (pragma))),
                          DupNode (AP_ARG3 (PRAGMA_APL (pragma))));
        }

        assigns = MakeAssign (alloc_icm, assigns);
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeAllocIcm_IncRc( char *name, types *type, int rc,
 *                             node *pragma, node *assigns)
 *
 * Description:
 *   Builds a ND_ALLOC and a ND_INC_RC icm.
 *   The extra ND_INC_RC is needed, if there are any ND_CHECK_REUSE ICMs above
 *   ND_ALLOC !!!
 *   The given node 'assigns' is appended to the created assignment.
 *
 ******************************************************************************/

static node *
MakeAllocIcm_IncRc (char *name, types *type, int rc, node *pragma, node *assigns)
{
    node *new_assigns;

    DBUG_ENTER ("MakeAllocIcm_IncRc");

    DBUG_ASSERT ((RC_IS_LEGAL (rc)), "illegal RC value found!");

    new_assigns = MakeAllocIcm (name, type, 0, pragma, NULL);

    if (new_assigns != NULL) {
        DBUG_ASSERT ((RC_IS_VITAL (rc)), "INC_RC(rc) with (rc <= 0) found!");
        assigns = AppendAssign (new_assigns,
                                MakeAssignIcm2 ("ND_INC_RC", MakeId_Copy_NT (name, type),
                                                MakeNum (rc), assigns));
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeAllocIcm_CheckReuse( char *name, types *type, int rc,
 *                                  node *pragma,
 *                                  node *reuse_id1, node *reuse_id2,
 *                                  node *assigns)
 *
 * Description:
 *   Builds ND_CHECK_REUSE icms and a ND_ALLOC/ND_INC_RC icm.
 *   The extra ND_INC_RC is needed, if there are any ND_CHECK_REUSE ICMs above
 *   ND_ALLOC !!!
 *   The given node 'assigns' is appended to the created assignment.
 *
 ******************************************************************************/

static node *
MakeAllocIcm_CheckReuse (char *name, types *type, int rc, node *pragma, node *reuse_id1,
                         node *reuse_id2, node *assigns)
{
    bool reuse1, reuse2;

    DBUG_ENTER ("MakeAllocIcm_CheckReuse");

    DBUG_ASSERT ((RC_IS_LEGAL (rc)), "illegal RC value found!");

    DBUG_ASSERT (((reuse_id1 == NULL) || (NODE_TYPE (reuse_id1) == N_id)),
                 "1st reuse array is not a N_id node!");
    DBUG_ASSERT (((reuse_id2 == NULL) || (NODE_TYPE (reuse_id2) == N_id)),
                 "2nd reuse array is not a N_id node!");

    if (RC_IS_ACTIVE (rc)) {
        reuse1 = ((reuse_id1 != NULL) && (ID_REFCNT (reuse_id1) == 1));
        reuse2 = ((reuse_id2 != NULL) && (ID_REFCNT (reuse_id2) == 1));

        if (reuse1 || reuse2) {
            assigns = MakeAllocIcm_IncRc (name, type, rc, pragma, assigns);

            if (reuse2) {
                assigns = MakeAssignIcm2 ("ND_CHECK_REUSE", DupId_NT (reuse_id2),
                                          MakeId_Copy_NT (name, type), assigns);
            }

            if (reuse1) {
                assigns = MakeAssignIcm2 ("ND_CHECK_REUSE", DupId_NT (reuse_id1),
                                          MakeId_Copy_NT (name, type), assigns);
            }
        } else {
            assigns = MakeAllocIcm (name, type, rc, pragma, assigns);
        }
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *DFM2AllocIcm_CheckReuse( char *name, types *type, int rc,
 *                                  node *pragma,
 *                                  node *reuse_id1, node *reuse_id2,
 *                                  node *assigns)
 *
 * Description:
 *   Builds ND_CHECK_REUSE icms and a ND_ALLOC/ND_INC_RC icm.
 *   The extra ND_INC_RC is needed, if there are any ND_CHECK_REUSE ICMs above
 *   ND_ALLOC !!!
 *   The given node 'assigns' is appended to the created assignment.
 *
 ******************************************************************************/

static node *
DFM2AllocIcm_CheckReuse (char *name, types *type, int rc, node *pragma,
                         DFMmask_t *reuse_dfm, node *assigns)
{
    node *vardec;

    DBUG_ENTER ("DFM2AllocIcm_CheckReuse");

    DBUG_ASSERT ((RC_IS_LEGAL (rc)), "illegal RC value found!");

    if (RC_IS_ACTIVE (rc)) {
        if (reuse_dfm != NULL) {
            vardec = DFMGetMaskEntryDeclSet (reuse_dfm);
        } else {
            vardec = NULL;
        }

        if (vardec != NULL) {
            assigns = MakeAllocIcm_IncRc (name, type, rc, pragma, assigns);

            while (vardec != NULL) {
                assigns = MakeAssignIcm2 ("ND_CHECK_REUSE",
                                          MakeId_Copy_NT (VARDEC_OR_ARG_NAME (vardec),
                                                          VARDEC_OR_ARG_TYPE (vardec)),
                                          MakeId_Copy_NT (name, type), assigns);

                vardec = DFMGetMaskEntryDeclSet (NULL);
            }
        } else {
            assigns = MakeAllocIcm (name, type, rc, pragma, assigns);
        }
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *AddThreadIdIcm_ND_FUN_DEC( node *icm)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
AddThreadIdIcm_ND_FUN_DEC (node *icm)
{
    node *args;

    DBUG_ENTER ("AddThreadIdIcm_ND_FUN_DEC");

    DBUG_ASSERT (((NODE_TYPE (icm) == N_icm) && (!strcmp (ICM_NAME (icm), "ND_FUN_DEC"))),
                 "no ND_FUN_DEC icm found!");

    if ((gen_mt_code == GEN_MT_OLD) && (optimize & OPT_PHM)) {
        args = ICM_EXPRS3 (icm);

        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (args)) == N_num),
                     "wrong argument in ND_FUN_DEC icm found!");

#if TAGGED_ARRAYS
        EXPRS_NEXT (args)
          = MakeExprs (MakeId_Copy (mdb_argtag[ATG_in_nodesc]),
                       MakeExprs (MakeId_Copy ("unsigned int"),
                                  MakeExprs (MakeId_Copy ("SAC_MT_mythread"),
                                             EXPRS_NEXT (args))));
#endif

        (NUM_VAL (EXPRS_EXPR (args)))++;
    }

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * Function:
 *   node *AddThreadIdIcm_ND_FUN_AP( node *icm_assign)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
AddThreadIdIcm_ND_FUN_AP (node *icm_assign)
{
    node *icm;
    node *args;

    DBUG_ENTER ("AddThreadIdIcm_ND_FUN_AP");

    DBUG_ASSERT ((NODE_TYPE (icm_assign) == N_assign), "no assign found!");

    icm = ASSIGN_INSTR (icm_assign);

    DBUG_ASSERT (((NODE_TYPE (icm) == N_icm) && (!strcmp (ICM_NAME (icm), "ND_FUN_AP"))),
                 "no ND_FUN_AP icm found!");

    if ((gen_mt_code == GEN_MT_OLD) && (optimize & OPT_PHM)) {
        args = ICM_EXPRS3 (icm);

        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (args)) == N_num),
                     "wrong argument in ND_FUN_AP icm found!");

#if TAGGED_ARRAYS
        EXPRS_NEXT (args)
          = MakeExprs (MakeId_Copy (mdb_argtag[ATG_in_nodesc]),
                       MakeExprs (MakeId_Copy ("SAC_MT_mythread"), EXPRS_NEXT (args)));
#endif

        (NUM_VAL (EXPRS_EXPR (args)))++;
    }

    DBUG_RETURN (icm_assign);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeArgNode( int idx, types *type)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
MakeArgNode (int idx, types *type)
{
    node *id;
    char *name;

    DBUG_ENTER ("MakeArgNode");

    name = Malloc (20 * sizeof (char));
    sprintf (name, "SAC__arg_%d", idx);

    if (type != NULL) {
        id = MakeId_Copy_NT (name, type);
    } else {
        id = MakeId_Copy (name);
    }

    name = Free (name);

    DBUG_RETURN (id);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeIcm_ND_FUN_DEC( node *fundef)
 *
 * Description:
 *   Creates a ND_FUN_DEC ICM, which has the following format:
 *     ND_FUN_DEC( name, rettype, narg, [TAG, type, arg]*),
 *
 ******************************************************************************/

static node *
MakeIcm_ND_FUN_DEC (node *fundef)
{
    node *ret_node;
    argtab_t *argtab;
    int i;
    node *icm_args = NULL;

    DBUG_ENTER ("MakeIcm_ND_FUN_DEC");

    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no fundef node found!");

    argtab = FUNDEF_ARGTAB (fundef);
    DBUG_ASSERT ((argtab != NULL), "no argtab found!");

    /* arguments */
    for (i = argtab->size - 1; i >= 1; i--) {
        argtag_t tag;
        types *type;
        char *name;
        node *id;

        if (argtab->ptr_out[i] != NULL) {
            tag = argtab->tag[i];
            type = argtab->ptr_out[i];
            id = MakeArgNode (i, type);
        } else {
            DBUG_ASSERT ((argtab->ptr_in[i] != NULL), "argtab is uncompressed!");
            DBUG_ASSERT ((NODE_TYPE (argtab->ptr_in[i]) == N_arg),
                         "no N_arg node found in argtab");
            tag = argtab->tag[i];
            type = ARG_TYPE (argtab->ptr_in[i]);
            name = ARG_NAME (argtab->ptr_in[i]);
            if (name != NULL) {
                id = MakeId_Copy_NT (name, type);
            } else {
                id = MakeArgNode (i, type);
            }
        }

        if (TYPES_BASETYPE (type) == T_dots) {
            /*
             * for ... arguments the name should expand to an empty string
             *  -> replace 'tag' and 'id'
             */
#if TAGGED_ARRAYS
            tag = ATG_in_nodesc;
#else
            tag = ATG_notag;
#endif
            id = FreeTree (id);
            id = MakeId_Copy ("");
        }

        icm_args
          = MakeExprs (MakeId_Copy (mdb_argtag[tag]),
                       MakeExprs (MakeBasetypeArg (type), MakeExprs (id, icm_args)));
    }

    icm_args = MakeExprs (MakeNum (argtab->size - 1), icm_args);

    /* return value */
    DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab inconsistent!");
    if (argtab->ptr_out[0] == NULL) {
        icm_args = MakeExprs (MakeId_Copy (""), icm_args);
    } else {
        icm_args = MakeExprs (MakeBasetypeArg_NT (argtab->ptr_out[0]), icm_args);
    }

    ret_node = MakeIcm2 ("ND_FUN_DEC", MakeId_Copy (FUNDEF_NAME (fundef)), icm_args);

    /*
     * add the thread id
     */
    if (FUNDEF_BODY (fundef) != NULL) {
        ret_node = AddThreadIdIcm_ND_FUN_DEC (ret_node);
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeIcm_MT_SPMD_FUN_DEC( node *fundef)
 *
 * Description:
 *   creates a MT_SPMD_FUN_DEC ICM.
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

        if (argtab->ptr_out[i] != NULL) {
            id = MakeArgNode (i, NULL);
            type = argtab->ptr_out[i];
        } else {
            DBUG_ASSERT ((argtab->ptr_in[i] != NULL), "argtab is uncompressed!");
            DBUG_ASSERT ((NODE_TYPE (argtab->ptr_in[i]) == N_arg),
                         "no N_arg node found in argtab");

            name = ARG_NAME (argtab->ptr_in[i]);
            id = MakeId_Copy (STR_OR_EMPTY (name));
            type = ARG_TYPE (argtab->ptr_in[i]);
        }

        icm_args
          = MakeExprs (MakeId_Copy (mdb_argtag[argtab->tag[i]]),
                       MakeExprs (MakeBasetypeArg (type), MakeExprs (id, icm_args)));
    }
    size = argtab->size - 1;

    /* return value */
    DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab is inconsistent!");
    if (argtab->ptr_out[0] != NULL) {
        icm_args = MakeExprs (MakeId_Copy (mdb_argtag[argtab->tag[0]]),
                              MakeExprs (MakeBasetypeArg (argtab->ptr_out[0]),
                                         MakeExprs (MakeArgNode (0, NULL), icm_args)));
        size++;
    }

    icm = MakeIcm4 ("MT_SPMD_FUN_DEC", MakeId_Copy (FUNDEF_NAME (fundef)),
                    MakeId_Copy (FUNDEF_NAME (FUNDEF_LIFTEDFROM (fundef))),
                    MakeNum (size), icm_args);

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeIcm_MT2_FUN_DEC( char *kindof, node *fundef)
 *
 * Description:
 *   creates a MT2_FUN_DEC ICM.
 *
 ******************************************************************************/

static node *
MakeIcm_MT2_FUN_DEC (char *kindof, node *fundef)
{
    argtab_t *argtab;
    node *icm;
    int size;
    int i;
    node *icm_args = NULL;

    DBUG_ENTER ("MakeIcm_MT2_FUN_DEC");

    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no fundef node found!");

    argtab = FUNDEF_ARGTAB (fundef);
    DBUG_ASSERT ((argtab != NULL), "no argtab found!");

    for (i = argtab->size - 1; i >= 0; i--) {
        if (FALSE) {
            /*
             * dkr:
             * not completed yet ...
             * use 'argtab->...[i]' to build the icm args
             */
            icm_args = MakeExprs (NULL /* icm args */, icm_args);
        }
    }
    size = argtab->size /* - 1 */;

    icm
      = MakeIcm5 ("MT2_FUN_DEC", MakeId_Copy (kindof), MakeId_Copy (FUNDEF_NAME (fundef)),
                  MakeId_Copy (FUNDEF_NAME (FUNDEF_LIFTEDFROM (fundef))), MakeNum (size),
                  icm_args);

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeFundefIcm( node *fundef, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
MakeFundefIcm (node *fundef, node *arg_info)
{
    node *icm;

    DBUG_ENTER ("MakeFundefIcm");

    /*
     * if mtn(==mt2) is set, we check if this is a lifted mt-function inspecting
     * FUNDEF_ATTRIB, otherwise we check FUNDEF_STATUS to see if this is
     * a spmd-function (old mt==mto) or decide it is a "normal" function.
     */
    if (gen_mt_code == GEN_MT_NEW) {
        switch (FUNDEF_ATTRIB (fundef)) {
        case ST_call_mtlift:
            icm = MakeIcm_MT2_FUN_DEC ("CALL_MTLIFT", fundef);
            break;

        case ST_call_mt_worker:
            icm = MakeIcm_MT2_FUN_DEC ("CALL_MTWORKER", fundef);
            break;

        case ST_call_mt_master:
            icm = MakeIcm_MT2_FUN_DEC ("CALL_MTMASTER", fundef);
            break;

        case ST_call_rep:
            icm = MakeIcm_MT2_FUN_DEC ("CALL_MTREP", fundef);
            break;

        case ST_call_st:
            /* using normal function-declaration for single threaded calls */
            icm = MakeIcm_ND_FUN_DEC (fundef);
            break;

        default:
            DBUG_ASSERT (0, "unknown kind of function while in mt2");
            icm = NULL;
            break;
        }
    } else if (FUNDEF_STATUS (fundef) == ST_spmdfun) {
        icm = MakeIcm_MT_SPMD_FUN_DEC (fundef);
    } else {
        icm = MakeIcm_ND_FUN_DEC (fundef);
    }

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeIcm_ND_FUN_AP( node *ap, node *fundef, node *assigns)
 *
 * Description:
 *   Builds a N_assign node with the ND_FUN_AP icm.
 *
 ******************************************************************************/

static node *
MakeIcm_ND_FUN_AP (node *ap, node *fundef, node *assigns)
{
    node *ret_node;
    argtab_t *argtab;
    int i;
    node *icm_args = NULL;

    DBUG_ENTER ("MakeIcm_ND_FUN_AP");

    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no fundef node found!");

    DBUG_ASSERT (((ap != NULL) && (NODE_TYPE (ap) == N_ap)), "no ap node found!");

    argtab = AP_ARGTAB (ap);
    DBUG_ASSERT ((argtab != NULL), "no argtab found!");

    /* arguments */
    for (i = argtab->size - 1; i >= 1; i--) {
        node *exprs;

        if (argtab->ptr_out[i] != NULL) {
            exprs = MakeExprs (DupIds_Id_NT (argtab->ptr_out[i]), icm_args);
        } else {
            DBUG_ASSERT ((argtab->ptr_in[i] != NULL), "argtab is uncompressed!");
            DBUG_ASSERT ((NODE_TYPE (argtab->ptr_in[i]) == N_exprs),
                         "no N_exprs node found in argtab");
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (argtab->ptr_in[i])) == N_id),
                         "argument of application must be a N_id node!");

            exprs = MakeExprs (DupId_NT (EXPRS_EXPR (argtab->ptr_in[i])), NULL);
            EXPRS_NEXT (exprs) = icm_args;
        }
        icm_args = MakeExprs (MakeId_Copy (mdb_argtag[argtab->tag[i]]), exprs);
    }

    icm_args = MakeExprs (MakeNum (argtab->size - 1), icm_args);

    /* return value */
    DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab inconsistent!");
    if (argtab->ptr_out[0] == NULL) {
        icm_args = MakeExprs (MakeId_Copy (""), icm_args);
    } else {
        icm_args = MakeExprs (DupIds_Id (argtab->ptr_out[0]), icm_args);
    }

    ret_node = MakeAssignIcm2 ("ND_FUN_AP", MakeId_Copy (FUNDEF_NAME (fundef)), icm_args,
                               assigns);

    /* insert pointer to fundef */
    ICM_FUNDEF (ASSIGN_INSTR (ret_node)) = fundef;

    /*
     * add the thread id
     */
    if (FUNDEF_BODY (fundef) != NULL) {
        ret_node = AddThreadIdIcm_ND_FUN_AP (ret_node);
    }

    DBUG_RETURN (ret_node);
}

#ifndef DBUG_OFF

/******************************************************************************
 *
 * Function:
 *   bool CheckPrf( ids *let_ids, node *prf, node *arg_info)
 *
 * Description:
 *   Checks whether no one of the refcounted arguments occurs on LHS of the
 *   given application.
 *
 ******************************************************************************/

static bool
CheckPrf (ids *let_ids, node *prf, node *arg_info)
{
    node *args, *arg_id;
    ids *_ids;
    bool ok = TRUE;

    DBUG_ENTER ("CheckPrf");

    DBUG_ASSERT ((NODE_TYPE (prf) == N_prf), "no N_prf node found!");

    args = PRF_ARGS (prf);
    while (args != NULL) {
        arg_id = EXPRS_EXPR (args);
        if (NODE_TYPE (arg_id) == N_id) {
            DBUG_ASSERT ((!RC_IS_ZERO (ID_REFCNT (arg_id))),
                         "Reference with (rc == 0) found!");
            _ids = let_ids;
            while (_ids != NULL) {
                if (RC_IS_ACTIVE (ID_REFCNT (arg_id))
                    && (!strcmp (IDS_NAME (_ids), ID_NAME (arg_id)))) {
                    ok = FALSE;
                }
                _ids = IDS_NEXT (_ids);
            }
        }
        args = EXPRS_NEXT (args);
    }

    DBUG_RETURN (ok);
}

/******************************************************************************
 *
 * Function:
 *   bool CheckAp( node *ap, node *arg_info)
 *
 * Description:
 *   Checks whether no one of the externally refcounted in-arguments occurs
 *   on the LHS of the given application as well.
 *
 ******************************************************************************/

static bool
CheckAp (node *ap, node *arg_info)
{
    argtab_t *argtab;
    node *arg, *arg_id;
    ids *let_ids;
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
            if ((NODE_TYPE (arg_id) == N_id) && (RC_IS_ACTIVE (ID_REFCNT (arg_id)))) {

                for (ids_idx = 0; ids_idx < argtab->size; ids_idx++) {
                    let_ids = argtab->ptr_out[ids_idx];
                    if ((let_ids != NULL) && (ids_idx != arg_idx)
                        && (!strcmp (ID_NAME (arg_id), IDS_NAME (let_ids)))) {
#if TAGGED_ARRAYS
                        DBUG_ASSERT (((argtab->tag[arg_idx] == ATG_in)
                                      || (argtab->tag[arg_idx] == ATG_in_noshp)
                                      || (argtab->tag[arg_idx] == ATG_in_norc)
                                      || (argtab->tag[arg_idx] == ATG_in_nodesc)),
                                     "illegal tag found!");

                        if ((argtab->tag[arg_idx] == ATG_in_norc)
                            || (argtab->tag[arg_idx] == ATG_in_nodesc)) {
                            ok = FALSE;
                        }
#endif
                    }
                }
            }
        }
    }

    DBUG_RETURN (ok);
}

#endif

/******************************************************************************
 *
 * Function:
 *   node *MakeParamsByDFM( DFMmask_t *mask,
 *                          char *tag, int *num_args, node *icm_args)
 *
 * Description:
 *   Builds triplet-chain (tag, type, name) from dfm-mask mask,
 *   tag will we used as base for the tags (used raw or _rc is added),
 *   num_args will be incremented for each triplet added (maybe NULL),
 *   at the end of this chain icm_args will be concatenated.
 *
 *
 * ### CODE NOT BRUSHED YET ###
 * ### USED BY NEW MT ONLY ####
 *
 ******************************************************************************/

static node *
MakeParamsByDFM (DFMmask_t *mask, char *tag, int *num_args, node *icm_args)
{
    node *vardec;
    char *rc_tag, *this_tag;

    DBUG_ENTER ("MakeParamsByDFM");

    rc_tag = StringConcat (tag, "_rc");

    vardec = DFMGetMaskEntryDeclSet (mask);
    while (vardec != NULL) {

        if (RC_IS_ACTIVE (VARDEC_OR_ARG_REFCNT (vardec))) {
            this_tag = rc_tag;
        } else {
            this_tag = tag;
        }
        icm_args
          = MakeExprs (MakeId_Copy (this_tag),
                       MakeExprs (MakeBasetypeArg (VARDEC_OR_ARG_TYPE (vardec)),
                                  MakeExprs (MakeId_Copy (VARDEC_OR_ARG_NAME (vardec)),
                                             icm_args)));
        if (num_args != NULL) {
            *num_args = *num_args + 1;
        }

        vardec = DFMGetMaskEntryDeclSet (NULL);
    }

    rc_tag = Free (rc_tag);

    DBUG_RETURN (icm_args);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeParamsByDFMfold( DFMmask_t *mask,
 *                              char *tag, int *num_args, node *icm_args)
 *
 * Description:
 *   Builds tuple-chain (tag, type, name, fun) from dfm-foldmask mask,
 *   tag will we used as base for the tags (used raw or _rc is added),
 *   num_args will be incremented for each triplet added (maybe NULL),
 *   at the end of this chain icm_args will be concatenated.
 *
 *
 * ### CODE NOT BRUSHED YET ###
 * ### USED BY NEW MT ONLY ####
 *
 ******************************************************************************/

static node *
MakeParamsByDFMfold (DFMfoldmask_t *mask, char *tag, int *num_args, node *icm_args)
{
    node *vardec;
    char *rc_tag, *this_tag;

    DBUG_ENTER ("MakeParamsByDFMfold");

    rc_tag = StringConcat (tag, "_rc");

    while (mask != NULL) {
        vardec = DFMFM_VARDEC (mask);

        if (RC_IS_ACTIVE (VARDEC_OR_ARG_REFCNT (vardec))) {
            this_tag = rc_tag;
        } else {
            this_tag = tag;
        }
        icm_args
          = MakeExprs (MakeId_Copy (this_tag),
                       MakeExprs (MakeBasetypeArg (VARDEC_OR_ARG_TYPE (vardec)),
                                  MakeExprs (MakeId_Copy (VARDEC_OR_ARG_NAME (vardec)),
                                             icm_args)));
        if (num_args != NULL) {
            *num_args = *num_args + 1;
        }

        mask = DFMFM_NEXT (mask);
    }

    rc_tag = Free (rc_tag);

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
 * ### USED BY NEW MT ONLY ####
 *
 ******************************************************************************/

static node *
DoSomeReallyUglyTransformations_MT2 (node *fundef)
{
    node *front, *back;
    node *vardec;
    node *assigns, *assign;

    DBUG_ENTER ("DoSomeReallyUglyTransformations");

    /*
     * dkr:
     * The following code is unbelievable UGLY!!!!
     * But I do not want to rewrite the whole shit now :-/
     */

    front = MakeIcm4 ("MT2_WORKER_RECEIVE", MakeId_Copy ("SYNC"),
                      MakeNum (FUNDEF_IDENTIFIER (fundef)),
                      MakeNum (DFMTestMask (FUNDEF_MT2USE (fundef))),
                      MakeParamsByDFM (FUNDEF_MT2USE (fundef), "receive", NULL, NULL));
    back = MakeIcm3 ("MT2_WORKER_BARRIER", MakeNum (FUNDEF_IDENTIFIER (fundef)),
                     MakeNum (DFMTestMask (FUNDEF_MT2DEF (fundef))),
                     MakeParamsByDFM (FUNDEF_MT2DEF (fundef), "barrier", NULL, NULL));

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
         * !!!!!!!!! THIS PART MUST BE RECODED !!!!!!!!!
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
Compile_Tagged (node *arg_node)
{
    funtab *old_tab;
    node *info;

    DBUG_ENTER ("Compile");

    old_tab = act_tab;
    act_tab = comp2_tab;
    info = MakeInfo ();

    arg_node = Trav (arg_node, info);

    info = FreeTree (info);
    act_tab = old_tab;

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
COMP2Modul (node *arg_node, node *arg_info)
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
COMP2Typedef (node *arg_node, node *arg_info)
{
    node *icm = NULL;

    DBUG_ENTER ("COMPTypedef");

    icm = MakeIcm1 ("ND_TYPEDEF",
                    MakeTypeArgs (TYPEDEF_NAME (arg_node), TYPEDEF_TYPE (arg_node), TRUE,
                                  FALSE, FALSE));

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
COMP2Objdef (node *arg_node, node *arg_info)
{
    node *icm;

    DBUG_ENTER ("COMPObjdef");

    if ((OBJDEF_STATUS (arg_node) == ST_imported_mod)
        || (OBJDEF_STATUS (arg_node) == ST_imported_class)) {
        icm = MakeIcm1 ("ND_OBJDEF_EXTERN",
                        MakeTypeArgs (OBJDEF_NAME (arg_node), OBJDEF_TYPE (arg_node),
                                      TRUE, TRUE, FALSE));
    } else {
        icm = MakeIcm1 ("ND_OBJDEF",
                        MakeTypeArgs (OBJDEF_NAME (arg_node), OBJDEF_TYPE (arg_node),
                                      TRUE, TRUE, TRUE));
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
 *   node *COMPFundefArgs( node *fundef, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
COMPFundefArgs (node *fundef, node *arg_info)
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

    if (FUNDEF_STATUS (fundef) != ST_Cfun) {

        DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab inconsistent");
        for (i = 1; i < argtab->size; i++) {
            arg = argtab->ptr_in[i];
            if (arg != NULL) {
                DBUG_ASSERT ((NODE_TYPE (arg) == N_arg),
                             "no N_arg node found in argtab!");

                /*
                 * put ICMs for RC-adjustment at beginning of function block
                 *   BUT BEHIND THE DECLARATION ICMs!!!
                 *   -> put them at the tail of INFO_COMP_FIRSTASSIGN
                 */
                if (FUNDEF_STATUS (fundef) != ST_spmdfun) {
                    assigns
                      = AppendAssign (assigns,
                                      MakeAdjustRcIcm (ARG_NAME (arg), ARG_TYPE (arg),
                                                       ARG_REFCNT (arg),
                                                       ARG_REFCNT (arg) - 1, NULL));
                }

                /*
                 * fundef is a fold-fun?
                 *   -> generate no declarations (its code will be inlined anyways!)
                 */
                if (FUNDEF_STATUS (fundef) != ST_foldfun) {
                    /*
                     * put "ND_DECL_ARG" ICMs at beginning of function block
                     *   AND IN FRONT OF THE DECLARATION ICMs!!!
                     *   -> put ICM at the head of INFO_COMP_FIRSTASSIGN
                     */
                    assigns
                      = MakeAssignIcm1 ("ND_DECL_ARG",
                                        MakeTypeArgs (ARG_NAME (arg), ARG_TYPE (arg),
                                                      FALSE, TRUE, TRUE),
                                        assigns);

                    /*
                     * put "ND_DECL_PARAM_inout" ICM at beginning of function block
                     *   AND IN FRONT OF THE DECLARATION ICMs!!!
                     *   -> put ICM at the head of INFO_COMP_FIRSTASSIGN
                     */
                    if (argtab->tag[i] == ATG_inout) {
                        assigns = MakeAssignIcm2 ("ND_DECL_PARAM_inout",
                                                  MakeBasetypeArg (ARG_TYPE (arg)),
                                                  MakeId_Copy (ARG_NAME (arg)), assigns);
                    }
                }
            }
        }
    }

    DBUG_RETURN (assigns);
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
COMP2Fundef (node *arg_node, node *arg_info)
{
    node *old_fundef;
    node *assigns;

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
         * INFO_COMP_LASTSYNC provides these information, it is initialized here
         * with NULL and will be updated by each compilation of a N_sync (one needs
         * to compile them ordered!), this includes the destruction of such a
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
         * traversal of an SPMD function. They are chained by means of N_assign
         * nodes and will later be inserted into the code which sets up the
         * environment for multithreaded execution.
         */
        INFO_COMP_SCHEDULERINIT (arg_info) = NULL;

        if (FUNDEF_BODY (arg_node) != NULL) {
            /*
             * Traverse body
             */
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

            /*
             * Store collected scheduler information.
             */
            BLOCK_SCHEDULER_INIT (FUNDEF_BODY (arg_node))
              = INFO_COMP_SCHEDULERINIT (arg_info);

            if (INFO_COMP_SCHEDULERID (arg_info) > max_schedulers) {
                max_schedulers = INFO_COMP_SCHEDULERID (arg_info);
            }
        }

        /*
         * Destruction of last known N_sync is done here, all others have been
         * killed while traversing.
         */
        if (INFO_COMP_LASTSYNC (arg_info) != NULL) {
            INFO_COMP_LASTSYNC (arg_info) = FreeTree (INFO_COMP_LASTSYNC (arg_info));
        }

        /********** end: traverse body **********/

        /*
         * traverse arguments
         */
        if ((FUNDEF_ARGS (arg_node) != NULL) && (FUNDEF_BODY (arg_node) != NULL)) {
            assigns = COMPFundefArgs (arg_node, arg_info);

            /* new first assignment of body */
            BLOCK_INSTR (FUNDEF_BODY (arg_node))
              = AppendAssign (assigns, BLOCK_INSTR (FUNDEF_BODY (arg_node)));
        }

        FUNDEF_ICM (arg_node) = MakeFundefIcm (arg_node, arg_info);

        /*
         * traverse next fundef
         */
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
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
COMP2Vardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("COMPVardec");

    VARDEC_ICM (arg_node)
      = MakeIcm1 ("ND_DECL", MakeTypeArgs (VARDEC_NAME (arg_node), VARDEC_TYPE (arg_node),
                                           TRUE, TRUE, TRUE));

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
 ******************************************************************************/

node *
COMP2Block (node *arg_node, node *arg_info)
{
    node *assign;
    char *fun_name, *cs_tag;

    DBUG_ENTER ("COMPBlock");

    if (BLOCK_CACHESIM (arg_node) != NULL) {
        fun_name = FUNDEF_NAME (INFO_COMP_FUNDEF (arg_info));
        cs_tag
          = (char *)Malloc (strlen (BLOCK_CACHESIM (arg_node)) + strlen (fun_name) + 14);
        if (BLOCK_CACHESIM (arg_node)[0] == '\0') {
            sprintf (cs_tag, "\"%s(...)\"", fun_name);
        } else {
            sprintf (cs_tag, "\"%s in %s(...)\"", BLOCK_CACHESIM (arg_node), fun_name);
        }

        BLOCK_CACHESIM (arg_node) = Free (BLOCK_CACHESIM (arg_node));

        DBUG_ASSERT ((BLOCK_INSTR (arg_node) != NULL),
                     "first instruction of block is NULL"
                     " (should be a N_empty node)");
        assign = BLOCK_INSTR (arg_node);

        BLOCK_INSTR (arg_node)
          = MakeAssignIcm1 ("CS_START", MakeId_Copy (cs_tag), BLOCK_INSTR (arg_node));

        while ((ASSIGN_NEXT (assign) != NULL)
               && (NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (assign))) != N_return)) {
            assign = ASSIGN_NEXT (assign);
        }

        ASSIGN_NEXT (assign)
          = MakeAssignIcm1 ("CS_STOP", MakeId_Copy (cs_tag), ASSIGN_NEXT (assign));
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
COMP2Assign (node *arg_node, node *arg_info)
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
 ******************************************************************************/

static node *
COMPNormalFunReturn (node *arg_node, node *arg_info)
{
    node *fundef;
    argtab_t *argtab;
    node *ret_exprs;
    node *new_args;
    int i;
    int ret_cnt;
    node *cret_node = NULL;
    node *icm_args = NULL;
    node *last_arg = NULL;

    DBUG_ENTER ("COMPNormalFunReturn");

    fundef = INFO_COMP_FUNDEF (arg_info);
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
        cret_node = DupTree (EXPRS_EXPR (RETURN_CRET (arg_node)));
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
              = MakeExprs (MakeId_Copy (mdb_argtag[argtab->tag[i]]),
                           MakeExprs (DupId_NT (EXPRS_EXPR (ret_exprs)),
                                      MakeExprs (MakeArgNode (i, ID_TYPE (EXPRS_EXPR (
                                                                   ret_exprs))),
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
    ret_exprs = RETURN_REFERENCE (arg_node);
    while (ret_exprs != NULL) {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (ret_exprs)) == N_id),
                     "argument of return-statement must be a N_id node!");

        new_args
          = MakeExprs (MakeId_Copy (mdb_argtag[ATG_inout]),
                       MakeExprs (DupId_NT (EXPRS_EXPR (ret_exprs)),
                                  MakeExprs (DupId_NT (EXPRS_EXPR (ret_exprs)), NULL)));

        if (last_arg == NULL) {
            icm_args = new_args;
        } else {
            EXPRS_NEXT (last_arg) = new_args;
        }
        last_arg = EXPRS_EXPRS3 (new_args);

        ret_exprs = EXPRS_NEXT (ret_exprs);
        ret_cnt++;
    }

    /*
     * replace N_return node by a new N_icm node
     */

    DBUG_ASSERT ((FUNDEF_RETURN (fundef) == arg_node),
                 "FUNDEF_RETURN not found via 'arg_info'!");

    arg_node = FreeTree (arg_node);
    if (cret_node == NULL) {
        cret_node = MakeId_Copy ("");
    }
    arg_node = MakeIcm3 ("ND_FUN_RET", cret_node, MakeNum (ret_cnt), icm_args);

    FUNDEF_RETURN (fundef) = arg_node;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPSpmdFunReturn( node *arg_node, node *arg_info)
 *
 * Description:
 *   Generates ICMs for N_return-node found in body of a SPMD-function.
 *
 ******************************************************************************/

static node *
COMPSpmdFunReturn (node *arg_node, node *arg_info)
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

    fundef = INFO_COMP_FUNDEF (arg_info);
    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no fundef node found!");

    argtab = FUNDEF_ARGTAB (fundef);
    DBUG_ASSERT ((argtab != NULL), "no argtab found!");

    /* regular arguments */
    ret_exprs = RETURN_EXPRS (arg_node);
    ret_cnt = 0;
    for (i = 0; i < argtab->size; i++) {
        if (argtab->ptr_out[i] != NULL) {
            DBUG_ASSERT ((ret_exprs != NULL), "not enough return values found!");
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (ret_exprs)) == N_id),
                         "no N_id node found!");

            new_args = MakeExprs (MakeId_Copy (mdb_argtag[argtab->tag[i]]),
                                  MakeExprs (DupTree (EXPRS_EXPR (ret_exprs)),
                                             /*
                                                  MakeExprs( MakeArgNode( i, NULL),
                                              */
                                             NULL));

            if (last_arg == NULL) {
                icm_args = new_args;
            } else {
                EXPRS_NEXT (last_arg) = new_args;
            }
            last_arg = EXPRS_EXPRS2 (new_args) /* EXPRS_EXPR3( new_args) */;

            ret_exprs = EXPRS_NEXT (ret_exprs);
            ret_cnt++;
        } else {
            DBUG_ASSERT (((i == 0) || (argtab->ptr_in[i] != NULL)),
                         "argtab is uncompressed!");
        }
    }

    arg_node
      = MakeIcm3 ("MT_SPMD_FUN_RET", MakeNum (barrier_id), MakeNum (ret_cnt), icm_args);

    FUNDEF_RETURN (fundef) = arg_node;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPMT2FunReturn( node *arg_node, node *arg_info)
 *
 * Description:
 *   Generates ICMs for N_return-node found in body of a MT2-function.
 *
 ******************************************************************************/

static node *
COMPMT2FunReturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("COMPMT2FunReturn");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPReturn( node *arg_node, node *arg_info)
 *
 * Description:
 *   Generates ICMs for N_return of a function (ND or MT).
 *
 ******************************************************************************/

node *
COMP2Return (node *arg_node, node *arg_info)
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
COMP2Let (node *arg_node, node *arg_info)
{
    node *ret_node;
    node *expr;

    DBUG_ENTER ("COMPLet");

    INFO_COMP_LASTIDS (arg_info) = LET_IDS (arg_node);

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
 *   ids *COMPApIds( node *ap, node *arg_info)
 *
 * Description:
 *   Traverses ids on LHS of application.
 *
 ******************************************************************************/

static node *
COMPApIds (node *ap, node *arg_info)
{
    argtab_t *argtab;
    argtag_t tag;
    ids *let_ids;
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

            ret_node
              = MakeAssignIcm1 ("ND_REFRESH_MIRROR",
                                MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                              FALSE, TRUE, FALSE),
                                ret_node);

#ifdef TAGGED_ARRAYS
            if ((tag == ATG_out) || (tag == ATG_out_noshp)) {
                /* function does refcounting */

                ret_node = MakeAdjustRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                            IDS_REFCNT (let_ids),
                                            IDS_REFCNT (let_ids) - 1, ret_node);
            }

            if ((tag == ATG_out_norc) || (tag == ATG_out_nodesc)) {
                /* function does no refcounting */

                ret_node = MakeSetRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                         IDS_REFCNT (let_ids), ret_node);
            }

            if ((tag == ATG_out_noshp) || (tag == ATG_out_nodesc)) {
                /* function sets no shape information */

                ret_node
                  = MakeAssignIcm1 ("ND_SET_SHP",
                                    MakeTypeArgs (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                                  FALSE, TRUE, TRUE),
                                    ret_node);
            }

            if (tag == ATG_out_nodesc) {
                /* function uses no descriptor at all */

                ret_node = MakeAllocDescIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                             IDS_REFCNT (let_ids), ret_node);
            }

#endif
        }
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPApArgs( node *ap, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
COMPApArgs (node *ap, node *arg_info)
{
    argtab_t *argtab;
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

#ifdef TAGGED_ARRAYS
            if ((argtab->tag[i] == ATG_in_nodesc)
                || (argtab->tag[i] == ATG_inout_nodesc)) {
                /* function does no refcounting */

                if (NODE_TYPE (arg) == N_id) {
                    ret_node = MakeDecRcIcm (ID_NAME (arg), ID_TYPE (arg),
                                             ID_REFCNT (arg), 1, ret_node);
                }
            }
#endif
        }
    }

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
 *   INFO_COMP_LASTIDS contains pointer to previous let-ids.
 *
 ******************************************************************************/

node *
COMP2Ap (node *arg_node, node *arg_info)
{
    node *ret_node;
    ids *let_ids;
    node *fundef;
    node *assigns1, *assigns2;

    DBUG_ENTER ("COMPAp");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    fundef = AP_FUNDEF (arg_node);

    DBUG_ASSERT ((CheckAp (arg_node, arg_info)),
                 "application of a user-defined function without own refcounting:"
                 " refcounted argument occurs also on LHS!");

    /*
     * traverse ids on LHS of application
     */
    assigns1 = COMPApIds (arg_node, arg_info);

    /*
     * traverse arguments of application
     */
    assigns2 = COMPApArgs (arg_node, arg_info);

    ret_node = AppendAssign (assigns1, assigns2);

    /* insert ND_FUN_AP icm at head of assignment chain */
    ret_node = MakeIcm_ND_FUN_AP (arg_node, fundef, ret_node);

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
 *   INFO_COMP_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfDim (node *arg_node, node *arg_info)
{
    node *ret_node;
    ids *let_ids;
    node *arg;

    DBUG_ENTER ("COMPPrfDim");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg) == N_id), "N_id as arg of F_dim expected!");

    ret_node
      = MakeAssignIcm2 ("ND_PRF_DIM", DupIds_Id_NT (let_ids), DupId_NT (arg), NULL);

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
 *   INFO_COMP_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfShape (node *arg_node, node *arg_info)
{
    node *ret_node;
    node *arg;
    ids *let_ids;

    DBUG_ENTER ("COMPPrfShape");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg) == N_id), "N_id as arg of F_shape expected!");

    ret_node
      = MakeAssignIcm2 ("ND_PRF_SHAPE", DupIds_Id_NT (let_ids),
                        MakeTypeArgs (ID_NAME (arg), ID_TYPE (arg), FALSE, TRUE, FALSE),
                        NULL);

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
 *   INFO_COMP_LASTIDS contains name of assigned variable.
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
     * (v = reshape( shp, w)   =>   v = w)
     */
    ret_node = Trav (arg2, arg_info);

    ret_node = AppendAssign (ret_node, MakeDecRcIcm (ID_NAME (arg1), ID_TYPE (arg1),
                                                     ID_REFCNT (arg1), 1, NULL));

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
 *   INFO_COMP_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfArith_SxA (node *arg_node, node *arg_info)
{
    node *ret_node;
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

    ret_node
      = MakeAssignIcm4 ("ND_BINOP_SxA_A", MakeId_Copy (prf_string[PRF_PRF (arg_node)]),
                        DupNode (arg1), DupNode (arg2), DupIds_Id (let_ids), NULL);

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
 *   INFO_COMP_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfArith_AxS (node *arg_node, node *arg_info)
{
    node *ret_node;
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

    ret_node
      = MakeAssignIcm4 ("ND_BINOP_AxS_A", MakeId_Copy (prf_string[PRF_PRF (arg_node)]),
                        DupNode (arg1), DupNode (arg2), DupIds_Id (let_ids), NULL);

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
 *   INFO_COMP_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfArith_AxA (node *arg_node, node *arg_info)
{
    node *ret_node;
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

    ret_node
      = MakeAssignIcm4 ("ND_BINOP_AxA_A", MakeId_Copy (prf_string[PRF_PRF (arg_node)]),
                        DupNode (arg1), DupNode (arg2), DupIds_Id (let_ids), NULL);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfIdxSel( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles N_prf node of type F_idx_sel.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfIdxSel (node *arg_node, node *arg_info)
{
    node *ret_node;
    node *arg1, *arg2;
    ids *let_ids;

    DBUG_ENTER ("COMPPrfIdxSel");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    if ((NODE_TYPE (arg1) != N_prf) || (NODE_TYPE (PRF_ARG1 (arg1)) != N_num)) {
        /*
         * CAUTION: AE, IVE generate unflattened code!
         * The 1st argument of idx_sel() may be a N_prf node with N_num arguments
         */
        DBUG_ASSERT (((NODE_TYPE (arg1) == N_id) || (NODE_TYPE (arg1) == N_num)),
                     "N_id or N_num as 1st arg of F_idx_sel expected!");
    }
    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id), "N_id as 2nd arg of F_idx_sel expected!");

    ret_node
      = MakeAssignIcm3 (IsArray (IDS_TYPE (let_ids))
                          /*
                           * This is possible only in case of an instrinsic array-sel:
                           *   sel( iv, A): int[*] -> type[*] -> type[*]
                           * Normally an array-sel is transformed into a with-loop
                           * containing scalar-sel's only.
                           */
                          ? "ND_IDX_SEL_A"
                          : "ND_IDX_SEL_S",
                        DupNode (arg1), DupNode (arg2), DupIds_Id (let_ids), NULL);

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
 *   INFO_COMP_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfIdxModarray (node *arg_node, node *arg_info)
{
    node *ret_node;
    node *arg1, *arg2, *arg3;
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

    ret_node = MakeAssignIcm5 (((NODE_TYPE (arg3) == N_id) && IsArray (ID_TYPE (arg3)))
                                 ? "ND_IDX_MODARRAY_AxVxA"
                                 : "ND_IDX_MODARRAY_AxVxS",
                               MakeBasetypeArg (IDS_TYPE (let_ids)), DupIds_Id (let_ids),
                               DupNode (arg1), DupNode (arg2), DupNode (arg3), NULL);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPPrfSel( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles N_prf node of type F_sel.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 * Remarks:
 *   INFO_COMP_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfSel (node *arg_node, node *arg_info)
{
    node *ret_node;
    node *arg1, *arg2;
    ids *let_ids;

    DBUG_ENTER ("COMPPrfSel");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);

    DBUG_ASSERT (((NODE_TYPE (arg1) == N_id) || (NODE_TYPE (arg1) == N_array)),
                 "N_id or N_array as 1st arg of F_sel expected!");
    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id), "N_id as 2nd arg of F_sel expected!");

    if (!IsArray (IDS_TYPE (let_ids))) {
        /* 'let_ids' is a scalar */

        if (NODE_TYPE (arg1) == N_id) {
            ret_node
              = MakeAssignIcm4 ("ND_KD_SEL_VxA_S", DupNode (arg2), DupIds_Id (let_ids),
                                MakeNum (GetTypesLength (ID_TYPE (arg1))), DupNode (arg1),
                                NULL);
        } else {
            /* 'arg1' is a N_array node */
            ret_node
              = MakeAssignIcm4 ("ND_KD_SEL_CxA_S", DupNode (arg2), DupIds_Id (let_ids),
                                MakeNum (CountExprs (ARRAY_AELEMS (arg1))),
                                DupTree (ARRAY_AELEMS (arg1)), NULL);
        }
    } else {
        /* 'let_ids' is an array */

        if (NODE_TYPE (arg1) == N_id) {
            ret_node
              = MakeAssignIcm5 ("ND_KD_SEL_VxA_A", MakeNum (GetDim (ID_TYPE (arg2))),
                                DupNode (arg2), DupIds_Id (let_ids),
                                MakeNum (GetTypesLength (ID_TYPE (arg1))), DupNode (arg1),
                                NULL);
        } else {
            /* 'arg1' is a N_array node */
            ret_node
              = MakeAssignIcm5 ("ND_KD_SEL_CxA_A", MakeNum (GetDim (ID_TYPE (arg2))),
                                DupNode (arg2), DupIds_Id (let_ids),
                                MakeNum (CountExprs (ARRAY_AELEMS (arg1))),
                                DupTree (ARRAY_AELEMS (arg1)), NULL);
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
 *   INFO_COMP_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfModarray (node *arg_node, node *arg_info)
{
    node *ret_node;
    node *arg1, *arg2, *arg3;
    ids *let_ids;
    node *res_dim, *res_btype;

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

    /* basic type of result */
    res_btype = MakeBasetypeArg (IDS_TYPE (let_ids));
    /* dimension of result */
    res_dim = MakeNum (GetDim (IDS_TYPE (let_ids)));

    if (NODE_TYPE (arg2) == N_array) {
        /* index is constant! */

        ret_node
          = MakeAssignIcm7 (((N_id == NODE_TYPE (arg3)) && IsArray (ID_TYPE (arg3)))
                              ? "ND_PRF_MODARRAY_AxCxA"
                              : "ND_PRF_MODARRAY_AxCxS",
                            res_btype, res_dim, DupIds_Id (let_ids), DupNode (arg1),
                            DupNode (arg3), MakeNum (CountExprs (ARRAY_AELEMS (arg2))),
                            DupTree (ARRAY_AELEMS (arg2)), NULL);
    } else {
        /* 'arg2' is a N_id node */
        DBUG_ASSERT (((GetShapeDim (ID_TYPE (arg2)) == 1)
                      && (GetBasetype (ID_TYPE (arg2)) == T_int)),
                     "2nd arg of F_modarray is a illegal indexing var!");

        ret_node
          = MakeAssignIcm7 (((N_id == NODE_TYPE (arg3)) && IsArray (ID_TYPE (arg3)))
                              ? "ND_PRF_MODARRAY_AxVxA"
                              : "ND_PRF_MODARRAY_AxVxS",
                            res_btype, res_dim, DupIds_Id (let_ids), DupNode (arg1),
                            DupNode (arg3), MakeNum (TYPES_SHAPE (ID_TYPE (arg2), 0)),
                            DupNode (arg2), NULL);
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
 *   INFO_COMP_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfConvertArr (node *arg_node, node *arg_info)
{
    node *ret_node;
    node *arg;
    ids *let_ids;
    char *icm_name;

    DBUG_ENTER ("COMPPrfConvertArr");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    arg = PRF_ARG1 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (arg) == N_id), "N_id as arg of F_toi/tof/tod_A aspected");

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

    ret_node = MakeAssignIcm2 (icm_name, DupNode (arg), DupIds_Id (let_ids), NULL);

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
 *   INFO_COMP_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfTakeDrop (node *arg_node, node *arg_info)
{
    node *ret_node;
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

    n_elems = ID_VECLEN (arg1);

    ret_node = MakeAssignIcm5 (icm_name, MakeNum (GetDim (ID_TYPE (arg2))),
                               DupNode (arg2), DupIds_Id (let_ids), MakeNum (n_elems),
                               IntVec2Array (n_elems, ID_CONSTVEC (arg1)), NULL);

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
 *   INFO_COMP_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfCat (node *arg_node, node *arg_info)
{
    node *ret_node;
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

    ret_node = MakeAssignIcm5 ("ND_KD_CAT_SxAxA_A", MakeNum (GetDim (ID_TYPE (arg2))),
                               DupNode (arg2), DupNode (arg3), DupIds_Id (let_ids),
                               DupNode (arg1), NULL);

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
 *   INFO_COMP_LASTIDS contains name of assigned variable.
 *
 ******************************************************************************/

static node *
COMPPrfRotate (node *arg_node, node *arg_info)
{
    node *ret_node;
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

    ret_node = MakeAssignIcm5 ("ND_KD_ROT_CxSxA_A", DupNode (arg1), DupNode (arg2),
                               MakeNum (GetDim (ID_TYPE (arg3))), DupNode (arg3),
                               DupIds_Id (let_ids), NULL);

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
COMP2Prf (node *arg_node, node *arg_info)
{
    ids *let_ids;
    node *ret_node1 = NULL;
    node *ret_node2 = NULL;
    node *ret_node3 = NULL;
    node *check_reuse1 = NULL;
    node *check_reuse2 = NULL;

    DBUG_ENTER ("COMPPrf");

    let_ids = INFO_COMP_LASTIDS (arg_info);

    if (PRF_PRF (arg_node) != F_reshape) {
        /*
         * CAUTION: F_reshape is compiled differently!!!
         */

        DBUG_ASSERT ((CheckPrf (let_ids, arg_node, arg_info)),
                     "application of a primitive function:"
                     " refcounted argument occurs also on LHS!");
    }

    switch (PRF_PRF (arg_node)) {
        /*
         *  SCALAR_ARGS( PRF_PRF( arg_node))
         */

    case F_toi:
    case F_tof:
    case F_tod:
        ret_node2 = COMPPrfConvertScalar (arg_node, arg_info);
        break;

    case F_abs:
        ret_node2 = COMPPrfIcm1 ("ND_ABS", arg_node, arg_info);
        break;

    case F_min:
        ret_node2 = COMPPrfIcm2 ("ND_MIN", arg_node, arg_info);
        break;

    case F_max:
        ret_node2 = COMPPrfIcm2 ("ND_MAX", arg_node, arg_info);
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
        ret_node2 = arg_node;
        break;

        /*
         *  ARRAY_ARGS_INTRINSIC( PRF_PRF( arg_node))
         */

    case F_genarray:
        DBUG_ASSERT ((0), "F_genarray not yet implemented!");
        break;

    case F_dim:
        ret_node2 = COMPPrfDim (arg_node, arg_info);
        break;

    case F_shape:
        ret_node2 = COMPPrfShape (arg_node, arg_info);
        break;

    case F_reshape:
        ret_node2 = COMPPrfReshape (arg_node, arg_info);
        break;

    case F_add_SxA:
    case F_sub_SxA:
    case F_mul_SxA:
    case F_div_SxA:
        check_reuse1 = PRF_ARG2 (arg_node);
        ret_node2 = COMPPrfArith_SxA (arg_node, arg_info);
        break;

    case F_add_AxS:
    case F_sub_AxS:
    case F_mul_AxS:
    case F_div_AxS:
        check_reuse1 = PRF_ARG1 (arg_node);
        ret_node2 = COMPPrfArith_AxS (arg_node, arg_info);
        break;

    case F_add_AxA:
    case F_sub_AxA:
    case F_mul_AxA:
    case F_div_AxA:
        check_reuse1 = PRF_ARG1 (arg_node);
        check_reuse2 = PRF_ARG2 (arg_node);
        ret_node2 = COMPPrfArith_AxA (arg_node, arg_info);
        break;

    case F_idx_sel:
        ret_node2 = COMPPrfIdxSel (arg_node, arg_info);
        break;

    case F_idx_modarray:
        check_reuse1 = PRF_ARG1 (arg_node);
        ret_node2 = COMPPrfIdxModarray (arg_node, arg_info);
        break;

    case F_sel:
        ret_node2 = COMPPrfSel (arg_node, arg_info);
        break;

    case F_modarray:
        check_reuse1 = PRF_ARG1 (arg_node);
        ret_node2 = COMPPrfModarray (arg_node, arg_info);
        break;

        /*
         *  ARRAY_ARGS_NON_INTRINSIC( PRF_PRF( arg_node))
         */

    case F_toi_A:
    case F_tof_A:
    case F_tod_A:
        ret_node2 = COMPPrfConvertArr (arg_node, arg_info);
        break;

    case F_drop:
    case F_take:
        ret_node2 = COMPPrfTakeDrop (arg_node, arg_info);
        break;

    case F_cat:
        ret_node2 = COMPPrfCat (arg_node, arg_info);
        break;

    case F_rotate:
        ret_node2 = COMPPrfRotate (arg_node, arg_info);
        break;

    default:
        DBUG_ASSERT ((0), "illegal prf found!");
        break;
    }

    DBUG_ASSERT ((ret_node2 != NULL), "no return value found!");

    if (PRF_PRF (arg_node) != F_reshape) {
        /*
         * CAUTION: F_reshape is compiled differently!!!
         */
        node *args, *arg;

        ret_node1 = MakeAllocIcm_CheckReuse (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                             RC_INIT (IDS_REFCNT (let_ids)), NULL,
                                             check_reuse1, check_reuse2, NULL);

        ret_node3
          = MakeAdjustRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids), IDS_REFCNT (let_ids),
                             IDS_REFCNT (let_ids) - 1, NULL);

        /*
         * append a DEC_RC for each arg
         */
        args = PRF_ARGS (arg_node);
        while (args != NULL) {
            arg = EXPRS_EXPR (args);
            if (NODE_TYPE (arg) == N_id) {
                ret_node3 = MakeDecRcIcm (ID_NAME (arg), ID_TYPE (arg), ID_REFCNT (arg),
                                          1, ret_node3);
            }
            args = EXPRS_NEXT (args);
        }
    }

    /*
     * merge 'ret_node1', 'ret_node2' and 'ret_node3'
     */
    if ((ret_node1 != NULL) || (ret_node3 != NULL)) {
        DBUG_ASSERT ((NODE_TYPE (ret_node2) == N_assign), "no assignment chain found!");
        ret_node2 = AppendAssign (ret_node2, ret_node3);
        ret_node2 = AppendAssign (ret_node1, ret_node2);
    }

    DBUG_RETURN (ret_node2);
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
    node *ret_node;

    DBUG_ENTER ("COMPIdLet");

    let_ids = INFO_COMP_LASTIDS (arg_info);

    if (GetDataClassFromTypes (IDS_TYPE (let_ids))
        == GetDataClassFromTypes (ID_TYPE (arg_node))) {
        ret_node = MakeAdjustRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                    IDS_REFCNT (let_ids), IDS_REFCNT (let_ids) - 1, NULL);
    } else {
        ret_node = MakeAdjustRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                    IDS_REFCNT (let_ids), IDS_REFCNT (let_ids) - 1,
                                    MakeDecRcIcm (ID_NAME (arg_node), ID_TYPE (arg_node),
                                                  ID_REFCNT (arg_node), 1, NULL));
    }

    /*
     * 'arg_node' and 'res' are both non-unique or both unique
     */
    if (!strcmp (IDS_NAME (let_ids), ID_NAME (arg_node))) {
        /*
         * We are dealing with an assignment of the kind:
         *   a = a;
         * which we compile into:
         *   NOOP()
         */
        ret_node = MakeAssignIcm0 ("NOOP", ret_node);
    } else {
        ret_node = MakeAssignIcm2 ("ND_ASSIGN",
                                   MakeTypeArgs (ID_NAME (arg_node), ID_TYPE (arg_node),
                                                 FALSE, TRUE, FALSE),
                                   DupIds_Id_NT (let_ids), ret_node);
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPIdFromUnique( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles let expression with a N_id node representing an application of
 *   the from_class() conversion function on RHS.
 *   The return value is a (possibly empty) N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 ******************************************************************************/

static node *
COMPIdFromUnique (node *arg_node, node *arg_info)
{
    ids *let_ids;
    node *ret_node = NULL;

    DBUG_ENTER ("COMPIdFromUnique");

    let_ids = INFO_COMP_LASTIDS (arg_info);

    /*
     * 'arg_node' is unique and 'let_ids' is non-unique
     */
    if (IsArray (ID_TYPE (arg_node))) {
        ret_node
          = MakeAdjustRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids), IDS_REFCNT (let_ids),
                             IDS_REFCNT (let_ids) - 1, ret_node);
    } else if (IsHidden (ID_TYPE (arg_node))) {
        ret_node = MakeAllocDescIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                     IDS_REFCNT (let_ids),
                                     MakeSetRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                                   IDS_REFCNT (let_ids), ret_node));
    }

    if (strcmp (IDS_NAME (let_ids), ID_NAME (arg_node))) {
        ret_node = MakeAssignIcm2 ("ND_ASSIGN", DupId_NT (arg_node),
                                   DupIds_Id_NT (let_ids), ret_node);
    } else {
        ret_node = MakeAssignIcm0 ("NOOP", ret_node);
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPIdToUnique( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles let expression with a N_id node representing an application of
 *   the to_class() conversion function on RHS.
 *   The return value is a N_assign chain of ICMs.
 *   Note, that the old 'arg_node' is removed by COMPLet.
 *
 ******************************************************************************/

static node *
COMPIdToUnique (node *arg_node, node *arg_info)
{
    ids *let_ids;
    types *rhs_type;
    node *ret_node = NULL;

    DBUG_ENTER ("COMPIdToUnique");

    let_ids = INFO_COMP_LASTIDS (arg_info);
    rhs_type = ID_TYPE (arg_node);

    DBUG_ASSERT (strcmp (IDS_NAME (let_ids), ID_NAME (arg_node)),
                 ".=to_unq(.) on identical objects is not allowed!");

    /*
     * 'arg_node' is non-unique and 'let_ids' is unique
     */
    if (RC_IS_ACTIVE (ID_REFCNT (arg_node))) {
        DBUG_ASSERT ((ID_REFCNT (arg_node) != 0), "Reference with (rc == 0) found!");

        if (ID_REFCNT (arg_node) == 1) {
            if (IsArray (rhs_type)) {
                ret_node
                  = MakeAssignIcm3 ("ND_MAKE_UNIQUE", DupId_NT (arg_node),
                                    DupIds_Id_NT (let_ids),
                                    MakeNum (GetBasetypeSize (rhs_type)),
                                    MakeSetRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                                  IDS_REFCNT (let_ids), ret_node));
            } else {
                ret_node
                  = MakeAssignIcm3 ("ND_MAKE_UNIQUE", DupId_NT (arg_node),
                                    DupIds_Id_NT (let_ids),
                                    MakeId_Copy (GenericFun (0, rhs_type)), ret_node);
            }
        } else {
            if (IsArray (rhs_type)) {
                ret_node = MakeAssignIcm3 (
                  "ND_COPY", DupId_NT (arg_node), DupIds_Id_NT (let_ids),
                  MakeNum (GetBasetypeSize (rhs_type)),
                  MakeDecRcIcm (ID_NAME (arg_node), rhs_type, ID_REFCNT (arg_node), 1,
                                MakeAllocDescIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                                  IDS_REFCNT (let_ids),
                                                  MakeSetRcIcm (IDS_NAME (let_ids),
                                                                IDS_TYPE (let_ids),
                                                                IDS_REFCNT (let_ids),
                                                                ret_node))));
            } else {
                ret_node
                  = MakeAssignIcm3 ("ND_COPY", DupId_NT (arg_node),
                                    DupIds_Id_NT (let_ids),
                                    MakeId_Copy (GenericFun (0, rhs_type)),
                                    MakeDecRcIcm (ID_NAME (arg_node), rhs_type,
                                                  ID_REFCNT (arg_node), 1, ret_node));
            }
        }
    } else {
        /*
         * assignment of scalars -> no ICM
         */
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
COMP2Id (node *arg_node, node *arg_info)
{
    node *ret_node;

    DBUG_ENTER ("COMPId");

    switch (ID_UNQCONV (arg_node)) {
    case NO_UNQCONV:
        ret_node = COMPIdLet (arg_node, arg_info);
        break;

    case FROM_UNQ:
        ret_node = COMPIdFromUnique (arg_node, arg_info);
        break;

    case TO_UNQ:
        ret_node = COMPIdToUnique (arg_node, arg_info);
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
 *   node *COMPScalar( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles let expression with a constant scalar on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/

node *
COMP2Scalar (node *arg_node, node *arg_info)
{
    node *ret_node;
    ids *let_ids;

    DBUG_ENTER ("COMPScalar");

    let_ids = INFO_COMP_LASTIDS (arg_info);

    ret_node
      = MakeAllocIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                      RC_INIT (IDS_REFCNT (let_ids)), NULL,
                      MakeAssignIcm2 ("ND_ASSIGN_CONST_SCALAR", DupIds_Id_NT (let_ids),
                                      DupNode (arg_node),
                                      MakeAdjustRcIcm (IDS_NAME (let_ids),
                                                       IDS_TYPE (let_ids),
                                                       IDS_REFCNT (let_ids),
                                                       IDS_REFCNT (let_ids) - 1, NULL)));

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPArray( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compiles let expression with a constant array on the RHS.
 *   The return value is a N_assign chain of ICMs (the old 'arg_node' is
 *   removed by COMPLet) or the unchanged N_id node.
 *
 ******************************************************************************/

node *
COMP2Array (node *arg_node, node *arg_info)
{
    node *ret_node;
    node *array_elem;
    ids *let_ids;
    int num_elems;

    DBUG_ENTER ("COMPArray");

    let_ids = INFO_COMP_LASTIDS (arg_info);

    /*
     * Count number of array elements.
     */
    num_elems = CountExprs (ARRAY_AELEMS (arg_node));

    if (num_elems > 0) {
        array_elem = EXPRS_EXPR (ARRAY_AELEMS (arg_node));
    } else {
        array_elem = NULL;
    }

    ret_node = MakeAdjustRcIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                                IDS_REFCNT (let_ids), IDS_REFCNT (let_ids) - 1, NULL);

    if (ARRAY_STRING (arg_node) != NULL) {
        /* array is a string */
        ret_node = MakeAssignIcm2 ("ND_ASSIGN_CONST_STR", DupIds_Id_NT (let_ids),
                                   MakeStr (ARRAY_STRING (arg_node)), ret_node);
    } else if ((NODE_TYPE (array_elem) == N_id)
               && (IsNonUniqueHidden (ID_TYPE (array_elem)))) {
        ret_node = MakeAssignIcm4 ("ND_ASSIGN_CONST_H", DupIds_Id_NT (let_ids),
                                   MakeId_Copy (GenericFun (0, ID_TYPE (array_elem))),
                                   MakeNum (num_elems), DupTree (ARRAY_AELEMS (arg_node)),
                                   ret_node);
    } else {
        ret_node = MakeAssignIcm3 ("ND_ASSIGN_CONST_VECT", DupIds_Id_NT (let_ids),
                                   MakeNum (num_elems),
                                   DupExprs_NT (ARRAY_AELEMS (arg_node)), ret_node);
    }

    ret_node = MakeAllocIcm (IDS_NAME (let_ids), IDS_TYPE (let_ids),
                             RC_INIT (IDS_REFCNT (let_ids)), NULL, ret_node);

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
COMP2Loop (node *arg_node, node *arg_info)
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
             * so we use ND_DEC_RC_FREE.
             */
            icm_node = MakeDecRcIcm (IDS_NAME (defvar), IDS_TYPE (defvar),
                                     IDS_REFCNT (defvar), 1, NULL);
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
        icm_node = MakeAssignIcm1 ("ND_LABEL", MakeId_Copy (label_str), NULL);
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
                                    IDS_REFCNT (usevar), IDS_REFCNT (usevar) - 1, NULL);
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
            icm_node = MakeDecRcIcm (IDS_NAME (usevar), IDS_TYPE (usevar),
                                     IDS_REFCNT (usevar), 1, NULL);
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
                                    IDS_REFCNT (defvar), IDS_REFCNT (defvar) - 1, NULL);
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
        ret_node = MakeAssignIcm1 ("ND_GOTO", MakeId_Copy (label_str), ret_node);
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
COMP2Cond (node *arg_node, node *arg_info)
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
          = Ids2DecRcIcms (COND_THENVARS (arg_node), BLOCK_INSTR (COND_THEN (arg_node)));
    }

    if (COND_ELSEVARS (arg_node) != NULL) {
        BLOCK_INSTR (COND_ELSE (arg_node))
          = Ids2DecRcIcms (COND_ELSEVARS (arg_node), BLOCK_INSTR (COND_ELSE (arg_node)));
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
COMP2Icm (node *arg_node, node *arg_info)
{
    node *args, *arg;
    node *icm_node, *last_node, *new_assign;
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
        new_assign = MakeAssign (arg_node, NULL);
        last_node = new_assign;

        args = ICM_EXPRS2 (arg_node);
        while (args != NULL) {
            arg = EXPRS_EXPR (args);
            if (NODE_TYPE (arg) == N_id) {
                icm_node
                  = MakeDecRcIcm (ID_NAME (arg), ID_TYPE (arg), ID_REFCNT (arg), 1, NULL);
                if (icm_node != NULL) {
                    last_node = ASSIGN_NEXT (last_node) = icm_node;
                }
            }
            args = EXPRS_NEXT (args);
        }

        if (ASSIGN_NEXT (new_assign) != NULL) {
            arg_node = new_assign;
        } else {
            ASSIGN_INSTR (new_assign) = NULL;
            new_assign = FreeTree (new_assign);
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
COMP2Cast (node *arg_node, node *arg_info)
{
    node *tmp;

    DBUG_ENTER ("COMPCast");

    tmp = arg_node;
    arg_node = Trav (CAST_EXPR (arg_node), arg_info);
    CAST_EXPR (tmp) = NULL;

    /* do not free tmp here - LET will remove it again !
       tmp = FreeTree( tmp); */

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
 *   node *MakeIcmArgs_WL_LOOP1( node *arg_node)
 *
 * Description:
 *   ICM args without 'step'.
 *
 ******************************************************************************/

static node *
MakeIcmArgs_WL_LOOP1 (node *arg_node)
{
    node *args;
    int dim;

    DBUG_ENTER ("MakeIcmArgs_WL_LOOP1");

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
 *   node *MakeIcmArgs_WL_LOOP2( node *arg_node)
 *
 * Description:
 *   ICM args with 'step'.
 *
 ******************************************************************************/

static node *
MakeIcmArgs_WL_LOOP2 (node *arg_node)
{
    node *args;

    DBUG_ENTER ("MakeIcmArgs_WL_LOOP2");

    args
      = MakeExprs (MakeIcmArgs_WL_LOOP1 (arg_node),
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
 *   node *MakeIcmArgs_WL_OP1( node *arg_node)
 *
 * Description:
 *   ICM args without names of loop variables.
 *
 ******************************************************************************/

static node *
MakeIcmArgs_WL_OP1 (node *arg_node)
{
    node *args;

    DBUG_ENTER ("MakeIcmArgs_WL_OP1");

    args = MakeExprs (MakeNum (GetDim (IDS_TYPE (wlids))),
                      MakeExprs (DupIds_Id (wlids),
                                 MakeExprs (DupIds_Id (NWITH2_VEC (wlnode)),
                                            MakeExprs (MakeNum (NWITH2_DIMS (wlnode)),
                                                       NULL))));

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeIcmArgs_WL_OP2( node *arg_node)
 *
 * Description:
 *   ICM args with names of loop variables.
 *
 ******************************************************************************/

static node *
MakeIcmArgs_WL_OP2 (node *arg_node)
{
    node *args;
    node *last_arg;
    ids *withid_ids;
    int num_args;

    DBUG_ENTER ("MakeIcmArgs_WL_OP2");

    args = MakeIcmArgs_WL_OP1 (arg_node);
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
 *   node *MakeIcm_MT_ADJUST_SCHEDULER( node *arg_node, node *assigns)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
MakeIcm_MT_ADJUST_SCHEDULER (node *arg_node, node *assigns)
{
    int dim;

    DBUG_ENTER ("MakeIcm_MT_ADJUST_SCHEDULER");

    DBUG_ASSERT (((NODE_TYPE (arg_node) == N_WLblock)
                  || (NODE_TYPE (arg_node) == N_WLublock)
                  || (NODE_TYPE (arg_node) == N_WLstride)
                  || (NODE_TYPE (arg_node) == N_WLstrideVar)),
                 "illegal WL-node found!");

    dim = WLNODE_DIM (arg_node);

    if ((!WLNODE_NOOP (arg_node)) && (WLNODE_LEVEL (arg_node) == 0) && NWITH2_MT (wlnode)
        && (SCHAdjustmentRequired (dim, wlseg))) {
        assigns
          = MakeAssignIcm6 ((NWITH2_OFFSET_NEEDED (wlnode))
                              ? "MT_ADJUST_SCHEDULER__OFFSET"
                              : "MT_ADJUST_SCHEDULER",
                            DupIds_Id (wlids), MakeNum (WLSEGX_DIMS (wlseg)),
                            MakeNum (dim),
                            NodeOrInt_MakeIndex (NODE_TYPE (arg_node),
                                                 WLBLOCKSTR_GET_ADDR (arg_node, BOUND1),
                                                 dim, IDS_NAME (wlids), TRUE, TRUE),
                            NodeOrInt_MakeIndex (NODE_TYPE (arg_node),
                                                 WLBLOCKSTR_GET_ADDR (arg_node, BOUND2),
                                                 dim, IDS_NAME (wlids), TRUE, TRUE),
                            NodeOrInt_MakeIndex (NODE_TYPE (arg_node),
                                                 WLBLOCKSTR_GET_ADDR (arg_node, STEP),
                                                 dim, IDS_NAME (wlids), TRUE, TRUE),
                            assigns);
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeIcm_WL_INIT_OFFSET( node *arg_node, node *assigns)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
MakeIcm_WL_INIT_OFFSET (node *arg_node, node *assigns)
{
    DBUG_ENTER ("MakeIcm_WL_INIT_OFFSET");

    if (NWITH2_OFFSET_NEEDED (wlnode)) {
        assigns
          = MakeAssignIcm1 ("WL_INIT_OFFSET", MakeIcmArgs_WL_OP1 (arg_node), assigns);
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeIcm_WL_ADJUST_OFFSET( node *arg_node, node *assigns)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
MakeIcm_WL_ADJUST_OFFSET (node *arg_node, node *assigns)
{
    DBUG_ENTER ("MakeIcm_WL_ADJUST_OFFSET");

    if (NWITH2_OFFSET_NEEDED (wlnode)) {
        assigns = MakeAssignIcm2 ("WL_ADJUST_OFFSET", MakeNum (WLNODE_DIM (arg_node)),
                                  MakeIcmArgs_WL_OP2 (arg_node), assigns);
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeIcm_WL_SET_OFFSET( node *arg_node, node *assigns)
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
MakeIcm_WL_SET_OFFSET (node *arg_node, node *assigns)
{
    int first_block_dim, first_ublock_dim, last_frac_dim;
    int dims, dim;
    int idx_min, idx_max;
    int d;
    shpseg *shape;
    int icm_dim = (-1);

    DBUG_ENTER ("MakeIcm_WL_SET_OFFSET");

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
            assigns = MakeAssignIcm3 ("WL_SET_OFFSET", MakeNum (dim), MakeNum (icm_dim),
                                      MakeIcmArgs_WL_OP2 (arg_node), assigns);
        }
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPWith2( node *arg_node, node *arg_info)
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
COMP2With2 (node *arg_node, node *arg_info)
{
    node *icm_args;
    node *old_wlnode;
    ids *old_wlids;
    ids *vec_ids;
    char *icm_name_begin, *icm_name_end;
    char *profile_name;
    node *ret_node;
    node *alloc_icms = NULL;
    node *free_icms = NULL;
    node *fold_icms = NULL;
    node *fold_rc_icms = NULL;

    DBUG_ENTER ("COMPWith2");

    /*
     * we must store the with-loop ids *before* compiling the codes
     *  because INFO_COMP_LASTIDS is possibly updated afterwards !!!
     */
    old_wlids = wlids; /* stack 'wlids' */
    wlids = INFO_COMP_LASTIDS (arg_info);
    old_wlnode = wlnode; /* stack 'wlnode' */
    wlnode = arg_node;

    /*
     * fold with-loop:
     *
     * Insert the vardecs of all special fold-funs into the vardec-chain of the
     * current function. Afterwards an update of the DFM-base is needed.
     * (This must be done here, because 'COMPSync()' needs the corrected masks)
     */
    if ((NWITH2_TYPE (arg_node) == WO_foldprf)
        || (NWITH2_TYPE (arg_node) == WO_foldfun)) {
        node *fundef = INFO_COMP_FUNDEF (arg_info);
        node *fold_vardecs = GetFoldVardecs (NWITH2_FUNDEF (arg_node));

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

    /*******************************************
     * build ICMs for memory allocation        *
     *******************************************/

    /*
     * allocate memory for index-vector
     */
    vec_ids = NWITH2_VEC (arg_node);
    alloc_icms = MakeAllocIcm (IDS_NAME (vec_ids), IDS_TYPE (vec_ids),
                               IDS_REFCNT (vec_ids), NULL, alloc_icms);

    if ((NWITH2_TYPE (arg_node) == WO_genarray)
        || (NWITH2_TYPE (arg_node) == WO_modarray)) {
        /*
         * genarray/modarray with-loop:
         *
         * Insert ICMs for memory management (ND_CHECK_REUSE, ND_ALLOC),
         * Note: In case of a fold with-loop, this is done automaticly when
         * compiling the init-assignment 'wlids = neutral' !!!
         */

        if (optimize & OPT_UIP) {
            /*
             * find all arrays, that possibly can be reused.
             */
            arg_node = GetReuseArrays (arg_node, INFO_COMP_FUNDEF (arg_info), wlids);
        } else {
            DBUG_ASSERT ((NWITH2_REUSE (arg_node) == NULL),
                         "illegal value for NWITH2_REUSE found!");
        }

        alloc_icms = DFM2AllocIcm_CheckReuse (IDS_NAME (wlids), IDS_TYPE (wlids),
                                              RC_INIT (IDS_REFCNT (wlids)),
                                              NWITH2_PRAGMA (arg_node),
                                              NWITH2_REUSE (arg_node), alloc_icms);

        if (NWITH2_REUSE (arg_node) != NULL) {
            NWITH2_REUSE (arg_node) = DFMRemoveMask (NWITH2_REUSE (arg_node));
        }
    }

    /*******************************************
     * build DEC_RC_FREE ICMs                  *
     *******************************************/

    free_icms
      = Ids2DecRcIcms (NWITH2_DEC_RC_IDS (arg_node),
                       Ids2DecRcIcms (NWITH2_VEC (arg_node),
                                      MakeAdjustRcIcm (IDS_NAME (wlids), IDS_TYPE (wlids),
                                                       IDS_REFCNT (wlids),
                                                       IDS_REFCNT (wlids) - 1,
                                                       free_icms)));

    /*******************************************
     * build WL_... ICMs                       *
     *******************************************/

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
        {
            node *let_neutral
              = MakeLet (DupTree (NWITH2_NEUTRAL (arg_node)), DupOneIds (wlids));
            node *tmp, *new;

            fold_icms = MakeAssigns1 (Compile_Tagged (let_neutral));

            /*
             * All RC-ICMs on 'wlids' must be moved *behind* the WL-code!!
             * Therefore we collect them in 'fold_rc_icms' to insert them later.
             */
            tmp = fold_icms;
            while (ASSIGN_NEXT (tmp) != NULL) {
                if ((NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (tmp))) == N_icm)
                    && ((!strcmp (ICM_NAME (ASSIGN_INSTR (ASSIGN_NEXT (tmp))),
                                  "ND_DEC_RC"))
                        || (!strcmp (ICM_NAME (ASSIGN_INSTR (ASSIGN_NEXT (tmp))),
                                     "ND_INC_RC"))
                        || (!strcmp (ICM_NAME (ASSIGN_INSTR (ASSIGN_NEXT (tmp))),
                                     "ND_DEC_RC_FREE")))) {
                    new = ASSIGN_NEXT (tmp);
                    ASSIGN_NEXT (tmp) = ASSIGN_NEXT (ASSIGN_NEXT (tmp));
                    ASSIGN_NEXT (new) = fold_rc_icms;
                    fold_rc_icms = new;
                } else {
                    tmp = ASSIGN_NEXT (tmp);
                }
            }

            profile_name = "fold";
        }
        break;

    default:
        DBUG_ASSERT ((0), "illegal withop type found");
        icm_name_begin = icm_name_end = NULL;
        profile_name = NULL;
        break;
    }

    /*******************************************
     * compile all code blocks                 *
     *******************************************/

    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    /*******************************************
     * put it all together                     *
     *******************************************/

    ret_node
      = MakeAssigns7 (alloc_icms, fold_icms,
                      MakeAssignIcm1 ("PF_BEGIN_WITH", MakeId_Copy (profile_name),
                                      MakeAssignIcm1 (icm_name_begin, icm_args, NULL)),
                      Trav (NWITH2_SEGS (arg_node), arg_info),
                      MakeAssignIcm1 (icm_name_end, DupTree (icm_args),
                                      MakeAssignIcm1 ("PF_END_WITH",
                                                      MakeId_Copy (profile_name), NULL)),
                      fold_rc_icms, free_icms);

    /*
     * pop 'wlids', 'wlnode'
     */
    wlids = old_wlids;
    wlnode = old_wlnode;

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPWLsegx( node *arg_node, node *arg_info)
 *
 * Description:
 *   Compilation of a N_WLseg- or N_WLsegVar-node:
 *   Returns a N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *   (The whole with-loop-tree should be freed by COMPWith2() only!!)
 *
 * remark:
 *   - 'wlseg' points to the current with-loop segment.
 *     (N_WLseg- *or* N_WLsegVar-node!)
 *
 ******************************************************************************/

node *
COMP2WLsegx (node *arg_node, node *arg_info)
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
        next_icms = Trav (WLSEGX_NEXT (arg_node), arg_info);
    }

    /*
     * Collect initialization ICMs for schedulers.
     * This is only done for 'real' schedulers, not for the pseudo schedulers
     * used during sequential execution.
     */
    if (WLSEGX_SCHEDULING (arg_node) != NULL) {
        INFO_COMP_SCHEDULERINIT (arg_info)
          = MakeAssign (SCHCompileSchedulingWithTaskselInit (INFO_COMP_SCHEDULERID (
                                                               arg_info),
                                                             IDS_NAME (wlids),
                                                             WLSEGX_SCHEDULING (arg_node),
                                                             WLSEGX_TASKSEL (arg_node),
                                                             arg_node),
                        INFO_COMP_SCHEDULERINIT (arg_info));

        (INFO_COMP_SCHEDULERID (arg_info))++;
    }

    ret_node
      = MakeAssigns4 (SCHCompileSchedulingWithTaskselBegin (INFO_COMP_SCHEDULERID (
                                                              arg_info),
                                                            IDS_NAME (wlids),
                                                            WLSEGX_SCHEDULING (arg_node),
                                                            WLSEGX_TASKSEL (arg_node),
                                                            arg_node),
                      MakeIcm_WL_INIT_OFFSET (arg_node, Trav (WLSEGX_CONTENTS (arg_node),
                                                              arg_info)),
                      SCHCompileSchedulingWithTaskselEnd (INFO_COMP_SCHEDULERID (
                                                            arg_info),
                                                          IDS_NAME (wlids),
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

/******************************************************************************
 *
 * Function:
 *   node *COMPWLxblock( node *arg_node, node *arg_info)
 *
 * Description:
 *   compilation of a N_WLblock- or N_WLublock-node:
 *     returns a N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by COMPWith2() only!!)
 *
 * remarks:
 *   - 'wlids' points always to LET_IDS of the current with-loop.
 *   - 'wlnode' points always to the N_Nwith2-node.
 *   - 'wlseg' points to the current with-loop segment.
 *     (N_WLseg- *or* N_WLsegVar-node!)
 *
 ******************************************************************************/

node *
COMP2WLxblock (node *arg_node, node *arg_info)
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
        node_icms = MakeIcm_WL_ADJUST_OFFSET (arg_node, NULL);
    } else {
        if (WLXBLOCK_NEXTDIM (arg_node) != NULL) {
            DBUG_ASSERT ((WLXBLOCK_CONTENTS (arg_node) == NULL),
                         "CONTENTS and NEXTDIM used simultaneous!");

            node_icms = Trav (WLXBLOCK_NEXTDIM (arg_node), arg_info);
        }

        if (WLXBLOCK_CONTENTS (arg_node) != NULL) {
            DBUG_ASSERT ((WLXBLOCK_NEXTDIM (arg_node) == NULL),
                         "CONTENTS and NEXTDIM used simultaneous!");

            node_icms = Trav (WLXBLOCK_CONTENTS (arg_node), arg_info);
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
        begin_icm
          = MakeAssignIcm1 (icm_name_begin, MakeIcmArgs_WL_LOOP2 (arg_node), NULL);
    }

    if (icm_name_end != NULL) {
        end_icm = MakeAssignIcm1 (icm_name_end, MakeIcmArgs_WL_LOOP2 (arg_node), NULL);
    }

    /*******************************************
     * create ICMs for successor               *
     *******************************************/

    if (WLXBLOCK_NEXT (arg_node) != NULL) {
        next_icms = Trav (WLXBLOCK_NEXT (arg_node), arg_info);
    }

    /*******************************************
     * put it all together                     *
     *******************************************/

    ret_node = MakeAssigns5 (MakeIcm_MT_ADJUST_SCHEDULER (arg_node, NULL), begin_icm,
                             node_icms, end_icm, next_icms);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPWLstridex( node *arg_node, node *arg_info)
 *
 * Description:
 *   compilation of a N_WLstride- or N_WLstrideVar-node:
 *     returns a N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by COMPWith2() only!!)
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
COMP2WLstridex (node *arg_node, node *arg_info)
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
        node_icms = MakeIcm_WL_ADJUST_OFFSET (arg_node, NULL);
    } else {
        if (WLSTRIDEX_CONTENTS (arg_node) != NULL) {
            node_icms = Trav (WLSTRIDEX_CONTENTS (arg_node), arg_info);
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
            new_icms = AppendAssign (DupTree (node_icms), new_icms);
        }
        node_icms = AppendAssign (node_icms, new_icms);
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
          = MakeAssignIcm1 (icm_name_begin, MakeIcmArgs_WL_LOOP2 (arg_node), NULL);
    }

    if (icm_name_end != NULL) {
        end_icm = MakeAssignIcm1 (icm_name_end, MakeIcmArgs_WL_LOOP2 (arg_node), NULL);
    }

    /*******************************************
     * create ICMs for successor               *
     *******************************************/

    if (WLSTRIDEX_NEXT (arg_node) != NULL) {
        next_icms = Trav (WLSTRIDEX_NEXT (arg_node), arg_info);
    }

    /*******************************************
     * put it all together                     *
     *******************************************/

    ret_node = MakeAssigns5 (MakeIcm_MT_ADJUST_SCHEDULER (arg_node, NULL), begin_icm,
                             node_icms, end_icm, next_icms);

    /*
     * restore old 'wlstride'.
     */
    wlstride = old_wlstride;

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPWLgridx( node *arg_node, node *arg_info)
 *
 * Description:
 *   compilation of a N_WLgrid- or N_WLgridVar-node:
 *     returns a N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by COMPWith2() only!!)
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
COMP2WLgridx (node *arg_node, node *arg_info)
{
    int dim;
    bool mt_active, offset_needed, is_fitted;
    node *ret_node;
    char *icm_name_begin = NULL;
    char *icm_name_end = NULL;
    node *begin_icm = NULL;
    node *end_icm = NULL;
    node *node_icms = NULL;
    node *next_icms = NULL;

    DBUG_ENTER ("COMPWLgridx");

    dim = WLGRIDX_DIM (arg_node);

    mt_active = NWITH2_MT (wlnode);
    offset_needed = NWITH2_OFFSET_NEEDED (wlnode);
    is_fitted = WLGRID_FITTED (arg_node);

    if (WLGRIDX_NOOP (arg_node)) {
        node_icms = MakeIcm_WL_ADJUST_OFFSET (arg_node, NULL);
    } else {

        /*******************************************
         * create ICMs for next dim                *
         *******************************************/

        if (WLGRIDX_NEXTDIM (arg_node) != NULL) {
            DBUG_ASSERT ((WLGRIDX_CODE (arg_node) == NULL),
                         "CODE and NEXTDIM used simultaneous!");

            node_icms = MakeIcm_WL_SET_OFFSET (arg_node, Trav (WLGRIDX_NEXTDIM (arg_node),
                                                               arg_info));
        } else {

            /*******************************************
             * create ICMs for code block              *
             *******************************************/

            node *icm_args;
            char *icm_name;
            node *code_rc_icms = NULL;

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
                    icm_args = MakeIcmArgs_WL_OP2 (arg_node);
                    break;

                case WO_modarray:
                    DBUG_ASSERT ((offset_needed), "wrong value for OFFSET_NEEDED found!");
                    icm_name = "WL_ASSIGN__COPY";
                    icm_args = MakeExprs (DupNode (NWITH2_ARRAY (wlnode)),
                                          MakeIcmArgs_WL_OP2 (arg_node));
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

                node *cexpr = WLGRIDX_CEXPR (arg_node);

                DBUG_ASSERT ((cexpr != NULL), "no code expr found");

                DBUG_ASSERT ((WLGRIDX_CBLOCK (arg_node) != NULL),
                             "no code block found in N_Ncode node");
                DBUG_ASSERT ((WLGRIDX_CBLOCK_INSTR (arg_node) != NULL),
                             "first instruction of block is NULL"
                             " (should be a N_empty node)");

                if (NODE_TYPE (WLGRIDX_CBLOCK_INSTR (arg_node)) != N_empty) {
                    node_icms = DupTree (WLGRIDX_CBLOCK_INSTR (arg_node));
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
                    icm_args = MakeExprs (MakeNum (GetDim (ID_TYPE (cexpr))),
                                          MakeExprs (DupNode (cexpr),
                                                     MakeIcmArgs_WL_OP2 (arg_node)));
                    /*
                     * we must decrement the RC of 'cexpr' (consumed argument)
                     */
                    code_rc_icms = MakeDecRcIcm (ID_NAME (cexpr), ID_TYPE (cexpr),
                                                 ID_REFCNT (cexpr), 1, NULL);
                    break;

                case WO_foldfun:
                    /* here is no break missing! */
                case WO_foldprf:
                    if (offset_needed) {
                        icm_name = "WL_FOLD__OFFSET";
                    } else {
                        icm_name = "WL_FOLD";
                    }
                    icm_args = MakeIcmArgs_WL_OP2 (arg_node);

                    /*
                     * insert code of the special fold-fun
                     */
                    node_icms
                      = AppendAssign (node_icms, GetFoldCode (NWITH2_FUNDEF (wlnode)));
                    break;

                default:
                    DBUG_ASSERT ((0), "illegal withop type found");
                    icm_name = NULL;
                    icm_args = NULL;
                    break;
                }
            }

            node_icms = AppendAssign (node_icms,
                                      MakeAssignIcm1 (icm_name, icm_args, code_rc_icms));
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
        node_icms
          = MakeAssignIcm1 ("WL_SET_IDXVEC", MakeIcmArgs_WL_LOOP1 (arg_node), node_icms);
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
                new_icms = AppendAssign (DupTree (node_icms), new_icms);
            }
            node_icms = AppendAssign (node_icms, new_icms);
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
          = MakeAssignIcm1 (icm_name_begin, MakeIcmArgs_WL_LOOP1 (arg_node), NULL);
    }

    if (icm_name_end != NULL) {
        end_icm = MakeAssignIcm1 (icm_name_end, MakeIcmArgs_WL_LOOP1 (arg_node), NULL);
    }

    /*******************************************
     * create ICMs for successor               *
     *******************************************/

    if (WLGRIDX_NEXT (arg_node) != NULL) {
        next_icms = Trav (WLGRIDX_NEXT (arg_node), arg_info);
    }

    /*******************************************
     * put it all together                     *
     *******************************************/

    ret_node = MakeAssigns4 (begin_icm, node_icms, end_icm, next_icms);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   node *COMPWLcode( node *arg_node, node *arg_info)
 *
 * Description:
 *   compiles a N_Ncode node.
 *
 ******************************************************************************/

node *
COMP2WLcode (node *arg_node, node *arg_info)
{
    node *icm_assigns;

    DBUG_ENTER ("COMPWLcode");

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    /*
     * build a 'ND_INC_RC'-ICM for each ids in 'NCODE_INC_RC_IDS( arg_node)'.
     */
    icm_assigns = Ids2IncRcIcms (NCODE_INC_RC_IDS (arg_node), NULL);

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
 *  old MT
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
COMP2Spmd (node *arg_node, node *arg_info)
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
    assigns
      = MakeAssignIcm1 ("MT_SPMD_EXECUTE",
                        MakeId_Copy (FUNDEF_NAME (SPMD_FUNDEF (arg_node))), assigns);

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
        icm_args = EXPRS_EXPRS4 (icm_args);
        /*
         * Icm args are organized in triples, i.e. three ICM args belong
         * to a single original parameter.
         */
    }

#if 0
  icm_args = BLOCK_SPMD_SETUP_ARGS(FUNDEF_BODY(SPMD_FUNDEF(arg_node)));
#endif
    num_args = 0;
    icm_args = NULL; /* dkr: should be FreeTree( icm_args) ?!? */
    icm_args = MakeParamsByDFM (SPMD_IN (arg_node), "in", &num_args, icm_args);
    icm_args = MakeParamsByDFM (SPMD_OUT (arg_node), "out", &num_args, icm_args);
    icm_args = MakeParamsByDFM (SPMD_SHARED (arg_node), "shared", &num_args, icm_args);

    icm_args = MakeExprs (MakeId_Copy (FUNDEF_NAME (SPMD_FUNDEF (arg_node))),
                          MakeExprs (MakeNum (num_args), icm_args));

    assigns = MakeAssignIcm1 ("MT_SPMD_SETUP", icm_args, assigns);

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
 *     < malloc-ICMs >                   // if (FIRST == 0) only
 *     MT_CONTINUE( ...)                 // if (FIRST == 0) only
 *     MT_START_SYNCBLOCK( ...)
 *     < with-loop code without malloc/free-ICMs >
 *     MT_SYNC...( ...)
 *     < free-ICMs >
 *
 ******************************************************************************/

node *
COMP2Sync (node *arg_node, node *arg_info)
{
    node *icm_args, *icm_args3, *vardec, *with, *block, *instr, *assign, *last_assign,
      *prolog_icms, *epilog_icms, *new_icm, *setup_args, *assigns = NULL;
    ids *with_ids;
    argtag_t tag;
    char *icm_name, *var_name, *fold_type;
    int num_args, count_nesting;

    bool prolog;
    bool epilog;

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
        tag = ATG_in;
        icm_args3
          = AppendExprs (icm_args3,
                         MakeExprs (MakeId_Copy (mdb_argtag[tag]),
                                    MakeExprs (MakeBasetypeArg (
                                                 VARDEC_OR_ARG_TYPE (vardec)),
                                               MakeExprs (MakeId_Copy (
                                                            VARDEC_OR_ARG_NAME (vardec)),
                                                          NULL))));
        num_args++;

        vardec = DFMGetMaskEntryDeclSet (NULL);
    }
    icm_args3 = MakeExprs (MakeNum (num_args), icm_args3);

    DBUG_PRINT ("COMP_MT", ("--- Enter sync ---"));

    DBUG_PRINT ("COMP_MT", ("Enter sync-args"));

    /* inter-thread sync parameters */
    sync_args = NULL;
    num_sync_args = 0;
    vardec = DFMGetMaskEntryDeclSet (SYNC_INOUT (arg_node));
    while (vardec != NULL) {
        sync_args
          = AppendExprs (sync_args,
                         MakeExprs (MakeId_Copy (VARDEC_OR_ARG_NAME (vardec)), NULL));

        DBUG_PRINT ("COMP_MT", ("%s", VARDEC_OR_ARG_NAME (vardec)));

        num_sync_args++;
        vardec = DFMGetMaskEntryDeclSet (NULL);
    }
    sync_args = MakeExprs (MakeNum (num_sync_args), sync_args);

    DBUG_PRINT ("COMP_MT", ("Enter fold-args"));

    last_sync = INFO_COMP_LASTSYNC (arg_info);
    fold_args = NULL;
    num_fold_args = 0;
    if (last_sync != NULL) {
        DBUG_PRINT ("COMP_MT", ("last-sync found"));
        /*
         *  Traverse assignments
         */
        assign = BLOCK_INSTR (SYNC_REGION (last_sync));

        while (assign != NULL) {
            DBUG_PRINT ("COMP_MT", ("assign found"));

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

                    DBUG_PRINT ("COMP_MT", ("last's folds %s is %s", IDS_NAME (with_ids),
                                            fold_type));
                }
            }
            assign = ASSIGN_NEXT (assign);
        } /* while (assign != NULL) */
    }

    DBUG_PRINT ("COMP_MT", ("--- end ---"));

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

            barrier_args = AppendExprs (barrier_args, icm_args);

            DBUG_PRINT ("COMP", ("%s", IDS_NAME (with_ids)));

            /*
             * <tmp_var>, <fold_op>
             */
            DBUG_ASSERT ((NWITH2_FUNDEF (with) != NULL), "no fundef found");
            barrier_args = AppendExprs (barrier_args,
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

                if ((!strcmp (ICM_NAME (instr), "ND_ALLOC"))
                    || (!strcmp (ICM_NAME (instr), "ND_CHECK_REUSE"))) {
                    var_name = ID_NAME (ICM_ARG2 (instr));
                    prolog = TRUE;
                    epilog = FALSE;
                    DBUG_PRINT ("COMP", ("ICM: %s is prolog", ICM_NAME (instr)));
                } else if (!strcmp (ICM_NAME (instr), "ND_INC_RC")) {
                    var_name = ID_NAME (ICM_ARG1 (instr));
                    prolog = TRUE;
                    epilog = FALSE;
                    DBUG_PRINT ("COMP", ("ICM: %s is prolog", ICM_NAME (instr)));
                } else if ((!strcmp (ICM_NAME (instr), "ND_DEC_RC_FREE"))
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
        assigns = AppendAssign (assigns, MakeAssignIcm1 ("MT_MASTER_SEND_FOLDRESULTS",
                                                         fold_args, NULL));
        assigns = AppendAssign (assigns, MakeAssignIcm1 ("MT_MASTER_SEND_SYNCARGS",
                                                         sync_args, NULL));
        assigns = AppendAssign (assigns, MakeAssignIcm0 ("MT_START_WORKERS", NULL));
        assigns
          = AppendAssign (assigns, MakeAssignIcm1 ("MT_MASTER_END",
                                                   MakeExprsNum (barrier_id), NULL));
        assigns
          = AppendAssign (assigns, MakeAssignIcm1 ("MT_WORKER_BEGIN",
                                                   MakeExprsNum (barrier_id), NULL));
        assigns = AppendAssign (assigns, MakeAssignIcm0 ("MT_WORKER_WAIT", NULL));
        assigns = AppendAssign (assigns, MakeAssignIcm1 ("MT_MASTER_RECEIVE_FOLDRESULTS",
                                                         DupTree (fold_args), NULL));
        assigns = AppendAssign (assigns, MakeAssignIcm1 ("MT_MASTER_RECEIVE_SYNCARGS",
                                                         DupTree (sync_args), NULL));
        assigns
          = AppendAssign (assigns, MakeAssignIcm1 ("MT_WORKER_END",
                                                   MakeExprsNum (barrier_id), NULL));
        assigns
          = AppendAssign (assigns,
                          MakeAssignIcm1 ("MT_RESTART", MakeExprsNum (barrier_id), NULL));
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
        setup_args = MakeParamsByDFM (SYNC_INOUT (arg_node), "in", &num_args, setup_args);

        DBUG_PRINT ("COMP_MT", ("num_args %i", num_args));

        BLOCK_SPMD_PROLOG_ICMS (FUNDEF_BODY (INFO_COMP_FUNDEF (arg_info))) = prolog_icms;
        BLOCK_SPMD_SETUP_ARGS (FUNDEF_BODY (INFO_COMP_FUNDEF (arg_info))) = setup_args;
    }
    barrier_id++;

    /*
     * insert ICM 'MT_START_SYNCBLOCK' and contents of modified sync-region-block
     */
    assigns
      = AppendAssign (assigns,
                      MakeAssignIcm2 ("MT_START_SYNCBLOCK", MakeNum (barrier_id),
                                      icm_args3, BLOCK_INSTR (SYNC_REGION (arg_node))));

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

                DBUG_PRINT ("COMP_MT",
                            ("MT_SYNC_FOLD (instead of MT_SYNC_FOLD_NONFOLD)"));
            } else {
                /* DFMTestMask( SYNC_OUT( arg_node)) == 1  */
                /* possible, but not implemented: icm_name = "MT_SYNC_ONEFOLD_NONFOLD"; */
                barrier_args = MakeExprs (MakeNum (num_barrier_args), barrier_args);
                barrier_args = MakeExprs (MakeNum (barrier_id), barrier_args);
                icm_name = "MT_SYNC_FOLD";

                DBUG_PRINT ("COMP_MT",
                            ("MT_SYNC_FOLD (instead of MT_SYNC_ONEFOLD_NONFOLD)"));
            }
        } else {
            barrier_args = MakeExprs (MakeNum (barrier_id), barrier_args);
            icm_name = "MT_SYNC_NONFOLD";

            DBUG_PRINT ("COMP_MT", (" MT_SYNC_NONFOLD"));
        }
    } else {
        DBUG_ASSERT ((DFMTestMask (SYNC_OUT (arg_node)) > 0), "no target found");

        DBUG_PRINT ("COMP_MT",
                    ("DFMTestMask( OUT ): %i", DFMTestMask (SYNC_OUT (arg_node))));

        if (DFMTestMask (SYNC_OUT (arg_node)) > 1) {
            barrier_args = MakeExprs (MakeNum (num_barrier_args), barrier_args);
            barrier_args = MakeExprs (MakeNum (barrier_id), barrier_args);
            icm_name = "MT_SYNC_FOLD";

            DBUG_PRINT ("COMP_MT", (" MT_SYNC_FOLD"));
        } else {
            /* DFMTestMask( SYNC_OUT( arg_node)) == 1 */
            barrier_args = MakeExprs (MakeNum (num_barrier_args), barrier_args);
            barrier_args = MakeExprs (MakeNum (barrier_id), barrier_args);
            icm_name = "MT_SYNC_FOLD";

            DBUG_PRINT ("COMP_MT", (" MT_SYNC_ONEFOLD"));
        }
    }

    DBUG_PRINT ("COMP_MT",
                ("using syncronisation: %s barrier: %i", icm_name, barrier_id));

    assigns = AppendAssign (assigns, MakeAssignIcm1 (icm_name, barrier_args, NULL));

    /*
     * insert extracted epilog-ICMs (free).
     */

    /*
     *  Begins Block structure of value exchanges blocks or function return
     *  This has to be done, before epilogs ar inserted, which belong to the
     *  master-part of this blocks.
     */
    if (!SYNC_LAST (arg_node)) {
        assigns
          = AppendAssign (assigns, MakeAssignIcm1 ("MT_MASTER_BEGIN",
                                                   MakeExprsNum (barrier_id), NULL));
    } else {
        assigns = AppendAssign (assigns,
                                MakeAssignIcm1 ("MT_MASTER_BEGIN", /* use other name? */
                                                MakeExprsNum (barrier_id), NULL));
    }

    assigns = AppendAssign (assigns, epilog_icms);

    /*
     *  Exchanging LASTSYNC, a free the last one.
     *  Will be needed for next N_sync, if no next N_sync exists,
     *  COMPFundef() will take care of this tree.
     */
    if (INFO_COMP_LASTSYNC (arg_info) != NULL) {
        INFO_COMP_LASTSYNC (arg_info) = FreeTree (INFO_COMP_LASTSYNC (arg_info));
    }
    INFO_COMP_LASTSYNC (arg_info) = arg_node;

    DBUG_RETURN (assigns);
}

/*****************************************************************************
 **
 **  CONSTRUCTION AREA (new MT)
 **
 **/

static node *
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
            icm = MakeIcm1 ("ND_ALLOC", MakeId_Copy (name));
            result = MakeAssign (icm, result);
            name = DFMGetMaskEntryNameSet (NULL);
        }

        result = MakeAssignIcm1 ("COMMENT", MakeId_Copy ("ALLOCS"), result);
    } else {
        result = MakeAssignIcm1 ("COMMENT", MakeId_Copy ("NO ALLOCS"), NULL);
    }

    DBUG_RETURN (result);
}

/*****************************************************************************
 *
 *  new MT
 *
 */

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
COMP2Mt (node *arg_node, node *arg_info)
{
    node *result;
    node *allocate;
    node *broadcast;
    node *activate;
    node *code;
    node *barrier;

    DBUG_ENTER ("COMPMt");

    /*
     *  Part 1 - Allocate
     */
#if 0
  allocate  = MakeIcm2( "MT2_ALLOCATE",
                        MakeNum( DFMTestMask( MT_ALLOC( arg_node))),
                        MakeParamsByDFM( MT_ALLOC( arg_node), "alloc", NULL, NULL));

#else
    allocate = MakeAllocs (MT_ALLOC (arg_node));
#endif

    /*
     *  Part 2 - Broadcast
     */
    broadcast = MakeIcm5 ("MT2_MASTER_BROADCAST", MakeId_Copy ("SYNC"),
                          MakeNum (MT_IDENTIFIER (arg_node)),
                          MakeNum (DFMTestMask (MT_ALLOC (arg_node))
                                   + DFMTestMask (MT_USEMASK (arg_node))),
                          MakeParamsByDFM (MT_ALLOC (arg_node), "alloc", NULL, NULL),
                          MakeParamsByDFM (MT_USEMASK (arg_node), "usemask", NULL, NULL));

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
COMP2St (node *arg_node, node *arg_info)
{
    node *result, *fundef, *barrier, *code, *allocate;
    node *broadcast, *activate, *suspend, *receive;
    DFMmask_t dfm_flat; /* flattening needed */
    DFMmask_t bset;     /* broadcast-set */

    DBUG_ENTER ("COMPSt");

    dfm_flat = DFMGenMaskAnd (ST_SYNC (arg_node), ST_NEEDLATER_MT (arg_node));
    bset = DFMGenMaskOr (ST_ALLOC (arg_node), dfm_flat);
    dfm_flat = DFMRemoveMask (dfm_flat);

    fundef = INFO_COMP_FUNDEF (arg_info);

    if (FUNDEF_ATTRIB (fundef) == ST_call_mt_master) {
        barrier = MakeIcm1 ("MT2_MASTER_BARRIER", MakeNum (ST_IDENTIFIER (arg_node)));
        code = MakeIcm0 ("CODE");

#if 0
    allocate  = MakeIcm2( "MT2_ALLOCATE",
                          MakeNum( DFMTestMask( ST_ALLOC( arg_node))),
                          MakeParamsByDFM( ST_ALLOC( arg_node), "alloc", NULL, NULL));
#else
        allocate = MakeAllocs (ST_ALLOC (arg_node));
#endif
        broadcast
          = MakeIcm4 ("MT2_MASTER_BROADCAST", MakeId_Copy ("SYNC"),
                      MakeNum (MT_IDENTIFIER (arg_node)), MakeNum (DFMTestMask (bset)),
                      MakeParamsByDFM (bset, "bset", NULL, NULL));
        activate = MakeIcm2 ("MT2_ACTIVATE", MakeId_Copy ("SYNC"), MakeId_Copy ("NULL"));

        result = MakeAssigns5 (barrier, code, allocate, broadcast, activate);
    } else if (FUNDEF_ATTRIB (fundef) == ST_call_mt_worker) {
        barrier = MakeIcm1 ("MT2_WORKER_BARRIER", MakeNum (ST_IDENTIFIER (arg_node)));
        suspend = MakeIcm1 ("MT2_SUSPEND", MakeId_Copy ("SYNC"));
        receive
          = MakeIcm4 ("MT2_MASTER_RECEIVE", MakeId_Copy ("SYNC"),
                      MakeNum (MT_IDENTIFIER (arg_node)), MakeNum (DFMTestMask (bset)),
                      MakeParamsByDFM (bset, "bset", NULL, NULL));

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
COMP2MTsignal (node *arg_node, node *arg_info)
{
    node *assigns;

    DBUG_ENTER ("COMPMTsignal");

    assigns = MakeAssignIcm0 ("MT2_SIGNAL_DATA"
#if 0
                          , MakeId_Copy( "SYNC"),
                            MakeNum( DFMTestMask( MTSIGNAL_IDSET( arg_node))),
                            MakeParamsByDFM( MTSIGNAL_IDSET( arg_node),
                                             "ids",
                                             NULL, NULL)
#endif
                              ,
                              NULL);

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
COMP2MTalloc (node *arg_node, node *arg_info)
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
        DBUG_PRINT ("COMP_MT2", ("%s", mdb_statustype[FUNDEF_ATTRIB (fundef)]));
        DBUG_ASSERT (0, "can not handle such a function");
    }

    if_ = MakeIcm0 ("MT2_IF_I_AM_FIRST");
    alloc = MakeAllocs (MTALLOC_IDSET (arg_node));
    broadcast
      = MakeIcm3 (broadcast_icm, MakeId_Copy ("ALLOC"),
                  MakeNum (DFMTestMask (MTALLOC_IDSET (arg_node))),
                  MakeParamsByDFM (MTALLOC_IDSET (arg_node), "alloc", NULL, NULL));
    activate = MakeIcm2 ("MT2_ACTIVATE", MakeId_Copy ("ALLOC"), MakeId_Copy ("NULL"));
    else_ = MakeIcm0 ("MT2_ELSE_IF_I_AM_NOT_FIRST");
    suspend = MakeIcm1 ("MT2_SUSPEND", MakeId_Copy ("ALLOC"));
    receive = MakeIcm3 (receive_icm, MakeId_Copy ("ALLOC"),
                        MakeNum (DFMTestMask (MTALLOC_IDSET (arg_node))),
                        MakeParamsByDFM (MTALLOC_IDSET (arg_node), "alloc", NULL, NULL));
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
COMP2MTsync (node *arg_node, node *arg_info)
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
        DBUG_PRINT ("COMP_MT2", ("%s", mdb_statustype[FUNDEF_ATTRIB (fundef)]));
        DBUG_ASSERT (0, "can not handle such a function");
    }

    /* if */
    if_ = MakeIcm0 ("MT2_IF_I_AM_FIRST");
    /* alloc */
    alloc = MakeAllocs (MTSYNC_ALLOC (arg_node));
    /* wait */
    nums = 0;
    args = MakeParamsByDFMfold (MTSYNC_FOLD (arg_node), "sync", &nums, NULL);
    wait = MakeIcm2 ("MT2_WAIT_DATA", MakeNum (nums), args);
    /* broadcast */
    nums = 0;
    args = MakeParamsByDFMfold (MTSYNC_FOLD (arg_node), "sync", &nums,
                                MakeParamsByDFM (MTSYNC_ALLOC (arg_node), "sync", &nums,
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
    args = MakeParamsByDFMfold (MTSYNC_FOLD (arg_node), "sync", &nums,
                                MakeParamsByDFM (MTSYNC_ALLOC (arg_node), "sync", &nums,
                                                 NULL));
    receive = MakeIcm3 (receive_icm, MakeId_Copy ("SYNC"), MakeNum (nums), args);
    /* end */
    end_ = MakeIcm0 ("MT2_END_I_AM_FIRST");

    result = MakeAssigns9 (if_, alloc, wait, broadcast, activate, else_, suspend, receive,
                           end_);

    arg_node = FreeTree (arg_node);

    DBUG_RETURN (result);
}
