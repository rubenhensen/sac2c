/*
 *
 * $Log$
 * Revision 1.6  2004/07/19 12:38:47  ktr
 * INFO structure is now initialized correctly.
 *
 * Revision 1.5  2004/07/17 10:26:04  ktr
 * EMAL now uses an INFO structure.
 *
 * Revision 1.4  2004/07/16 12:52:05  ktr
 * support for F_accu added.
 *
 * Revision 1.3  2004/07/16 12:07:14  ktr
 * EMAL now traverses into N_ap and N_funcond, too.
 *
 * Revision 1.2  2004/07/15 13:39:23  ktr
 * renamed EMALAllocateFill into EMAllocateFill
 *
 * Revision 1.1  2004/07/14 15:26:36  ktr
 * Initial revision
 *
 *
 */

/**
 * @defgroup emm Explicit Memory Management
 *
 * This group includes all the files needed by explicit memory management
 *
 * @{
 */

/**
 *
 * @file alloc.c
 *
 * SAC -> SAC-MemVal.
 *
 * much more text needed
 *
 */
#define NEW_INFO

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "internal_lib.h"
#include "DupTree.h"
#include "ssa.h"
#include "new_types.h"
#include "alloc.h"
#include "print.h"

/*
 * The following switch controls wheter AKS-Information should be used
 * when possible
 */
#define USEAKS

/** <!--******************************************************************-->
 *
 *  Structure used for ALLOCLIST.
 *
 ***************************************************************************/
typedef struct ALLOCLIST_STRUCT {
    node *avis;
    node *dim;
    node *shape;
    struct ALLOCLIST_STRUCT *next;
} alloclist_struct;

/**
 * INFO structure
 */
struct INFO {
    alloclist_struct *alloclist;
    node *fundef;
    node *withops;
    node *indexvector;
    bool mustfill;
};

/**
 * INFO macros
 */
#define INFO_EMAL_ALLOCLIST(n) (n->alloclist)
#define INFO_EMAL_FUNDEF(n) (n->fundef)
#define INFO_EMAL_WITHOPS(n) (n->withops)
#define INFO_EMAL_INDEXVECTOR(n) (n->indexvector)
#define INFO_EMAL_MUSTFILL(n) (n->mustfill)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_EMAL_ALLOCLIST (result) = NULL;
    INFO_EMAL_FUNDEF (result) = NULL;
    INFO_EMAL_WITHOPS (result) = NULL;
    INFO_EMAL_INDEXVECTOR (result) = NULL;
    INFO_EMAL_MUSTFILL (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

#define NWITHOP_MEMAVIS(n) (n->node[3])

/**
 *
 *  ALLOCLIST FUNCTIONS
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn MakeALS
 *
 *  @brief creates an ALLOCLIST_STRUCT for allocation
 *
 *  @param ALLOCLIST
 *  @param avis
 *  @param dim
 *  @param shape
 *
 *  @return ALLOCLIST_STRUCT
 *
 ***************************************************************************/
static alloclist_struct *
MakeALS (alloclist_struct *als, node *avis, node *dim, node *shape)
{

    DBUG_ENTER ("MakeALS");

    if (als == NULL) {
        als = Malloc (sizeof (alloclist_struct));

        als->avis = avis;
        als->dim = dim;
        als->shape = shape;
        als->next = NULL;
    } else {
        als->next = MakeALS (als->next, avis, dim, shape);
    }

    DBUG_RETURN (als);
}

/** <!--******************************************************************-->
 *
 * @fn FreeALS
 *
 *  @brief frees all elements of the given ALLOCLIST
 *
 *  @param als ALLOCLIST
 *
 *  @return NULL
 *
 ***************************************************************************/
static alloclist_struct *
FreeALS (alloclist_struct *als)
{
    DBUG_ENTER ("FreeALS");

    if (als != NULL) {
        if (als->dim != NULL) {
            als->dim = FreeTree (als->dim);
        }

        if (als->shape != NULL) {
            als->shape = FreeTree (als->shape);
        }

        if (als->next != NULL) {
            als->next = FreeALS (als->next);
        }

        als = Free (als);
    }

    DBUG_RETURN (als);
}

/** <!--******************************************************************-->
 *
 * @fn MakeAllocAssignment
 *
 *  @brief converts an ALLOCLIST_STRUCT into an allocation assignment.
 *
 *  @param als
 *  @param next_node
 *
 *  @return An allocation assignment for the given als
 *
 ***************************************************************************/
static node *
MakeAllocAssignment (alloclist_struct *als, node *next_node)
{
    node *alloc;
    ids *ids;

    DBUG_ENTER ("MakeAllocAssignment");

    ids = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (als->avis))), NULL,
                   ST_regular);
    IDS_AVIS (ids) = als->avis;
    IDS_VARDEC (ids) = AVIS_VARDECORARG (als->avis);

    DBUG_ASSERT (als->dim != NULL, "alloc requires a dim expression!");
    DBUG_ASSERT (als->shape != NULL, "alloc requires a shape expression!");

#ifdef USEAKS
    if (TYIsAKS (AVIS_TYPE (als->avis))) {
        als->dim = FreeTree (als->dim);
        als->dim = MakeNum (TYGetDim (AVIS_TYPE (als->avis)));
        als->shape = FreeTree (als->shape);
        als->shape = SHShape2Array (TYGetShape (AVIS_TYPE (als->avis)));
    }
#endif
    alloc = MakePrf2 (F_alloc, als->dim, als->shape);

    als->dim = NULL;
    als->shape = NULL;

    alloc = MakeAssign (MakeLet (alloc, ids), next_node);
    AVIS_SSAASSIGN (IDS_AVIS (ids)) = alloc;

    DBUG_RETURN (alloc);
}

/**
 * @}
 */

/**
 *
 *  TRAVERSAL FUNCTIONS
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn EMALap
 *
 *  @brief removes all elements from ALLOCLIST.
 *
 *  Function applications return MemVals so we don't need to
 *  allocatate memory which must be filled.
 *
 *  @param arg_node the function application
 *  @param arg_info containing ALLOCLIST
 *
 *  @return arg_node (unmodified) and modified ALLOCLIST
 *
 ***************************************************************************/
node *
EMALap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMALap");

    INFO_EMAL_ALLOCLIST (arg_info) = FreeALS (INFO_EMAL_ALLOCLIST (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALarray
 *
 *  @brief updates ALLOCLIST with shape information about the given array.
 *
 *  shape information can be obtained from syntactic shape and element shape.
 *
 *  Ex. [[ a, b], [ c, d]]
 *  dim = 2 + dim(a)
 *  shape = [2,2]++shape(a)  <---->   shape( genarray( [2,2], a))
 *
 *  @param arg_node an N_array node
 *  @param arg_info containing ALLOCLIST for this assignment
 *
 *  @return unmodified N_array node. ALLOCLIST has been updated, though.
 *
 ***************************************************************************/
node *
EMALarray (node *arg_node, info *arg_info)
{
    alloclist_struct *als;

    DBUG_ENTER ("EMALarray");

    als = INFO_EMAL_ALLOCLIST (arg_info);

    if (ARRAY_AELEMS (arg_node) != NULL) {
        /*
         * [ a, ... ]
         * alloc( outer_dim + dim(a), shape( genarray( outer_shape, a)))
         */
        als->dim
          = MakePrf2 (F_add_SxS, MakeNum (SHGetDim (ARRAY_SHAPE (arg_node))),
                      MakePrf (F_dim, DupNode (EXPRS_EXPR (ARRAY_AELEMS (arg_node)))));

        als->shape
          = MakePrf (F_shape,
                     MakePrf2 (F_genarray, SHShape2Array (ARRAY_SHAPE (arg_node)),
                               DupNode (EXPRS_EXPR (ARRAY_AELEMS (arg_node)))));
    } else {
        /*
         * []: empty array
         * alloc_or_reuse( 1, outer_dim, outer_shape)
         */
        als->dim = MakeNum (SHGetDim (ARRAY_SHAPE (arg_node)));

        als->shape = SHShape2Array (ARRAY_SHAPE (arg_node));
    }

    /*
     * Signal EMALlet to wrap this RHS in a fill-operation
     */
    INFO_EMAL_MUSTFILL (arg_info) = TRUE;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALassign
 *
 *  @brief performs a bottom-up traversal and inserts alloc-statements.
 *
 *  Assignment get prepended with all the alloc-statements specified in
 *  ALLOCLIST.
 *
 *  @param arg_node N_assign node
 *  @param arg_info containing ALLOCLIST and COPYFILLLIST
 *
 *  @return assignmentchain with allocs and fills.
 *
 ***************************************************************************/
node *
EMALassign (node *arg_node, info *arg_info)
{
    alloclist_struct *als;

    DBUG_ENTER ("EMALassign");

    /*
     * Bottom-up travseral
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    /*
     * Traverse RHS of assignment
     */
    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

    /*
     * Allocate missing variables
     */
    als = INFO_EMAL_ALLOCLIST (arg_info);
    while (als != NULL) {
        arg_node = MakeAllocAssignment (als, arg_node);
        als = als->next;
    }
    INFO_EMAL_ALLOCLIST (arg_info) = FreeALS (INFO_EMAL_ALLOCLIST (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALcode
 *
 *  @brief appends codeblocks with necessary suballoc/fill combinations
 *
 *  @param arg_node with-code
 *  @param arg_info
 *
 *  @return with-code appended with suballoc/fill combinations
 *
 ***************************************************************************/
node *
EMALcode (node *arg_node, info *arg_info)
{
    ids *lhs, *arg1, *arg2;
    alloclist_struct *als;
    node *withops, *indexvector, *cexprs, *assign, *avis, *cexavis;

    DBUG_ENTER ("EMALcode");

    /*
     * Rescue ALLOCLIST, WITHOPS and INDEXVECTOR
     */
    als = INFO_EMAL_ALLOCLIST (arg_info);
    INFO_EMAL_ALLOCLIST (arg_info) = NULL;
    withops = INFO_EMAL_WITHOPS (arg_info);
    INFO_EMAL_WITHOPS (arg_info) = NULL;
    indexvector = INFO_EMAL_INDEXVECTOR (arg_info);
    INFO_EMAL_INDEXVECTOR (arg_info) = NULL;

    /*
     * Traverse block
     */
    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    /*
     * Restore ALLOCLIST, WITHOPS and INDEXVECTOR
     */
    INFO_EMAL_ALLOCLIST (arg_info) = als;
    INFO_EMAL_WITHOPS (arg_info) = withops;
    INFO_EMAL_INDEXVECTOR (arg_info) = indexvector;

    /*
     * Shape information for each result array must be set and
     * suballoc/fill(copy,...) must be inserted
     */
    als = INFO_EMAL_ALLOCLIST (arg_info);
    withops = INFO_EMAL_WITHOPS (arg_info);
    cexprs = NCODE_CEXPRS (arg_node);
    assign = NULL;

    while (withops != NULL) {
        DBUG_ASSERT (als != NULL, "ALLOCLIST must have an element for each WITHOP");
        DBUG_ASSERT (cexprs != NULL, "With-Loop must have as many results as ALLOCLIST");

        cexavis = ID_AVIS (EXPRS_EXPR (cexprs));

        /*
         * Set shape of genarray-wls if not already done and possible
         */
        if (NWITHOP_TYPE (withops) == WO_genarray) {
            if (als->dim == NULL) {
                if (TYIsAKS (AVIS_TYPE (cexavis))) {
                    als->dim
                      = MakePrf2 (F_add_SxS,
                                  MakePrf2 (F_sel, MakeNum (0),
                                            MakePrf (F_shape,
                                                     DupNode (NWITHOP_SHAPE (withops)))),
                                  MakeNum (TYGetDim (AVIS_TYPE (cexavis))));
                }
            }
            if (als->shape == NULL) {
                if (TYIsAKS (AVIS_TYPE (cexavis))) {
                    als->shape
                      = MakePrf2 (F_cat_VxV, DupNode (NWITHOP_SHAPE (withops)),
                                  SHShape2Array (TYGetShape (AVIS_TYPE (cexavis))));
                }
            }
        }

        /*
         * Insert suballoc/fill combinations for genarray-modarray operators
         */
        if ((NWITHOP_TYPE (withops) == WO_genarray)
            || (NWITHOP_TYPE (withops) == WO_modarray)) {

            /*
             * Create a new memory variable
             */
            FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info))
              = MakeVardec (TmpVar (),
                            DupOneTypes (VARDEC_TYPE (AVIS_VARDECORARG (cexavis))),
                            FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info)));

            avis = MakeAvis (FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info)));
            AVIS_TYPE (avis) = TYCopyType (AVIS_TYPE (cexavis));
            VARDEC_AVIS (FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info))) = avis;

            /*
             * Create fill operation
             */
            lhs = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (cexavis))), NULL,
                           ST_artificial);
            IDS_AVIS (lhs) = cexavis;
            IDS_VARDEC (lhs) = AVIS_VARDECORARG (cexavis);

            arg1 = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (cexavis))), NULL,
                            ST_regular);
            IDS_AVIS (arg1) = cexavis;
            IDS_VARDEC (arg1) = AVIS_VARDECORARG (cexavis);

            arg2 = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (avis))), NULL,
                            ST_regular);
            IDS_AVIS (arg2) = avis;
            IDS_VARDEC (arg2) = AVIS_VARDECORARG (avis);

            assign
              = MakeAssign (MakeLet (MakePrf2 (F_fill,
                                               MakePrf1 (F_copy, MakeIdFromIds (arg1)),
                                               MakeIdFromIds (arg2)),
                                     lhs),
                            assign);

            /*
             * Create suballoc assignment
             */
            lhs = DupOneIds (arg2);

            arg1 = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (als->avis))), NULL,
                            ST_regular);
            IDS_AVIS (arg1) = als->avis;
            IDS_VARDEC (arg1) = AVIS_VARDECORARG (als->avis);

            assign = MakeAssign (MakeLet (MakePrf2 (F_suballoc, MakeIdFromIds (arg1),
                                                    DupTree (
                                                      INFO_EMAL_INDEXVECTOR (arg_info))),
                                          lhs),
                                 assign);
        }

        als = als->next;
        withops = NWITHOP_NEXT (withops);
        cexprs = EXPRS_NEXT (cexprs);
    }

    BLOCK_INSTR (NCODE_CBLOCK (arg_node))
      = AppendAssign (BLOCK_INSTR (NCODE_CBLOCK (arg_node)), assign);

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALconst
 *
 *  @brief updates ALLOCLIST with shape information about the given constant
 *
 *  @param constant
 *  @param arg_info
 *
 *  @return constant
 *
 ***************************************************************************/
node *
EMALconst (node *arg_node, info *arg_info)
{
    alloclist_struct *als;

    DBUG_ENTER ("EMALconst");

    als = INFO_EMAL_ALLOCLIST (arg_info);

    als->dim = MakeNum (0);
    als->shape = CreateZeroVector (0, T_int);

    /*
     * Signal EMALlet to wrap this RHS in a fill operation
     */
    INFO_EMAL_MUSTFILL (arg_info) = TRUE;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALfuncond
 *
 *  @brief removes all elements from ALLOCLIST.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node (unmodified) and modified ALLOCLIST
 *
 ***************************************************************************/
node *
EMALfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMALfuncond");

    INFO_EMAL_ALLOCLIST (arg_info) = FreeALS (INFO_EMAL_ALLOCLIST (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALfundef
 *
 *  @brief traverses a fundef node by traversing the functions body.
 *         After that, SSA form is restored.
 *
 *  @param fundef
 *  @param arg_info
 *
 *  @return fundef
 *
 ***************************************************************************/
node *
EMALfundef (node *fundef, info *arg_info)
{
    DBUG_ENTER ("EMALfundef");

    INFO_EMAL_FUNDEF (arg_info) = fundef;

    /*
     * Traverse fundef body
     */
    if (FUNDEF_BODY (fundef) != NULL) {
        FUNDEF_BODY (fundef) = Trav (FUNDEF_BODY (fundef), arg_info);

        /*
         * Restore SSA form
         */
        fundef = RestoreSSAOneFundef (fundef);
    }

    /*
     * Traverse other fundefs
     */
    if (FUNDEF_NEXT (fundef) != NULL) {
        FUNDEF_NEXT (fundef) = Trav (FUNDEF_NEXT (fundef), arg_info);
    }

    return (fundef);
}

/** <!--******************************************************************-->
 *
 * @fn EMALicm
 *
 *  @brief allocates memory for the first argument of IVE-ICMs
 *
 *  @param arg_node
 *  @param arg_info containing ALLOCLIST
 *
 *  @return arg_node (unmodified) and modified ALLOCLIST
 *
 ***************************************************************************/
node *
EMALicm (node *arg_node, info *arg_info)
{
    char *name;

    DBUG_ENTER ("EMALicm");

    name = ICM_NAME (arg_node);
    if ((strstr (name, "USE_GENVAR_OFFSET") != NULL)
        || (strstr (name, "VECT2OFFSET") != NULL)
        || (strstr (name, "IDX2OFFSET") != NULL)) {

        INFO_EMAL_ALLOCLIST (arg_info)
          = MakeALS (INFO_EMAL_ALLOCLIST (arg_info),
                     IDS_AVIS (ID_IDS (ICM_ARG1 (arg_node))), MakeNum (0),
                     CreateZeroVector (0, T_int));
    } else {
        DBUG_ASSERT ((0), "Unknown ICM found during EMAL");
    }
    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALid
 *
 *  @brief removes all elements from ALLOCLIST.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node (unmodified) and modified ALLOCLIST
 *
 ***************************************************************************/
node *
EMALid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMALid");

    INFO_EMAL_ALLOCLIST (arg_info) = FreeALS (INFO_EMAL_ALLOCLIST (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALlet
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
EMALlet (node *arg_node, info *arg_info)
{
    ids *ids;

    DBUG_ENTER ("EMALlet");

    /*
     * Put all LHS identifiers into ALLOCLIST
     */
    ids = LET_IDS (arg_node);

    while (ids != NULL) {
        INFO_EMAL_ALLOCLIST (arg_info)
          = MakeALS (INFO_EMAL_ALLOCLIST (arg_info), IDS_AVIS (ids), NULL, NULL);
        ids = IDS_NEXT (ids);
    }

    /*
     * Traverse RHS
     */
    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

        /*
         * Wrap RHS in Fill-operation if necessary
         */
        if (INFO_EMAL_MUSTFILL (arg_info)) {
            ids = LET_IDS (arg_node);

            LET_EXPR (arg_node)
              = MakePrf (F_fill, MakeExprs (LET_EXPR (arg_node), Ids2Exprs (ids)));

            /*
             * By setting IDS_STATUS of LHS identifiers to ST_artificial,
             * SSATransform will mark these with SSAUNDOFLAG causing
             * UndoSSATransform to rename these into their original identifiers:
             *
             * Before UndoSSATransform:
             * a' = fill( ..., a);
             *
             * After UndoSSATransform:
             * a  = fill( ..., a);
             */
            while (ids != NULL) {
                IDS_STATUS (ids) = ST_artificial;
                ids = IDS_NEXT (ids);
            }

            INFO_EMAL_MUSTFILL (arg_info) = FALSE;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALprf
 *
 *  @brief updates ALLOCLIST with shape information about the given prf
 *
 *  @param arg_node prf
 *  @param arg_info
 *
 *  @return prf
 *
 ***************************************************************************/
node *
EMALprf (node *arg_node, info *arg_info)
{
    alloclist_struct *als;

    DBUG_ENTER ("EMALprf");

    als = INFO_EMAL_ALLOCLIST (arg_info);

    /*
     * Signal EMALlet to wrap this prf in a fill-operation
     */
    INFO_EMAL_MUSTFILL (arg_info) = TRUE;

    switch (PRF_PRF (arg_node)) {
    case F_dim:
        /*
         * dim( A );
         * alloc( 0, [] );
         */
        als->dim = MakeNum (0);
        als->shape = CreateZeroVector (0, T_int);
        break;

    case F_shape:
        /*
         * shape( A );
         * alloc(1, shape( shape( A )))
         */
        als->dim = MakeNum (1);
        als->shape = MakePrf (F_shape, DupTree (arg_node));
        break;

    case F_reshape:
        /*
         * reshape( sh, A );
         * alloc( shape( sh )[0], sh );
         */
        als->dim = MakePrf2 (F_sel, MakeNum (0),
                             MakePrf (F_shape, DupNode (PRF_ARG1 (arg_node))));

        als->shape = DupNode (PRF_ARG1 (arg_node));
        break;

    case F_drop_SxV:
    case F_take_SxV:
    case F_cat_VxV:
        /*
         * drop( dv, V );
         * alloc( 1, shape( drop( dv, V )));
         * take( tv, V );
         * alloc( 1, shape( take( tv, V )));
         * cat( V1, V2 );
         * alloc( 1, shape( cat( V1, V2 )));
         */
        als->dim = MakeNum (1);
        als->shape = MakePrf (F_shape, DupTree (arg_node));
        break;

    case F_sel:
        /*
         * sel( iv, A );
         * alloc( dim(A) - shape(iv)[0], shape( sel( iv, A)))
         */
        als->dim = MakePrf2 (F_sub_SxS, MakePrf (F_dim, DupNode (PRF_ARG2 (arg_node))),
                             MakePrf2 (F_sel, MakeNum (0),
                                       MakePrf (F_shape, DupNode (PRF_ARG1 (arg_node)))));

        als->shape = MakePrf (F_shape, DupNode (arg_node));
        break;

    case F_idx_sel:
        /*
         * idx_sel can only occur when the result shape is known!!!
         *
         * a = idx_sel( idx, A);
         * alloc( dim( a), shape( idx_sel( idx, A)));
         */
        als->dim = MakeNum (TYGetDim (AVIS_TYPE (als->avis)));
        als->shape = MakePrf (F_shape, DupNode (arg_node));
        break;

    case F_modarray:
    case F_idx_modarray:
        /*
         * idx_modarray( A, idx, val);
         * alloc( dim( A ), shape ( A ));
         */
        als->dim = MakePrf (F_dim, DupNode (PRF_ARG1 (arg_node)));
        als->shape = MakePrf (F_shape, DupNode (PRF_ARG1 (arg_node)));
        break;

    case F_add_SxS:
    case F_sub_SxS:
    case F_mul_SxS:
    case F_div_SxS:
    case F_toi_S:
    case F_tof_S:
    case F_tod_S:
        /*
         * single scalar operations
         * alloc( 0, [])
         */
        als->dim = MakeNum (0);
        als->shape = CreateZeroVector (0, T_int);
        break;

    case F_mod:
    case F_min:
    case F_max:
    case F_neg:
    case F_not:
    case F_abs:
    case F_and:
    case F_or:
    case F_le:
    case F_lt:
    case F_eq:
    case F_neq:
    case F_ge:
    case F_gt:
    case F_toi_A:
    case F_tof_A:
    case F_tod_A:
    case F_add_AxS:
    case F_sub_AxS:
    case F_mul_AxS:
    case F_div_AxS:
    case F_add_AxA:
    case F_mul_AxA:
    case F_sub_AxA:
    case F_div_AxA:
    case F_add_SxA:
    case F_sub_SxA:
    case F_mul_SxA:
    case F_div_SxA:
        if ((CountExprs (PRF_ARGS (arg_node)) < 2)
            || (NODE_TYPE (PRF_ARG2 (arg_node)) != N_id)
            || (GetShapeDim (ID_TYPE (PRF_ARG2 (arg_node))) == SCALAR)) {
            als->dim = MakePrf (F_dim, DupNode (PRF_ARG1 (arg_node)));
            als->shape = MakePrf (F_shape, DupNode (PRF_ARG1 (arg_node)));
        } else {
            als->dim = MakePrf (F_dim, DupNode (PRF_ARG2 (arg_node)));
            als->shape = MakePrf (F_shape, DupNode (PRF_ARG2 (arg_node)));
        }
        break;

    case F_accu:
        /*
         * a,... = accu( iv, n, ...)
         * accu requires a special treatment as none of its return
         * value must be allocated
         */
        INFO_EMAL_ALLOCLIST (arg_info) = FreeALS (INFO_EMAL_ALLOCLIST (arg_info));

        INFO_EMAL_MUSTFILL (arg_info) = FALSE;
        break;

    case F_alloc:
    case F_suballoc:
    case F_fill:
    case F_alloc_or_reuse:
    case F_inc_rc:
    case F_dec_rc:
    case F_free:
    case F_copy:
        DBUG_ASSERT ((0), "invalid prf found!");
        break;

    case F_take:
    case F_drop:
    case F_cat:
    case F_rotate:
    case F_genarray:
        DBUG_ASSERT ((0), "Non-instrinsic primitive functions not implemented!"
                          " Use array.lib instead!");
        break;

        /*
         *  otherwise
         */

    case F_type_error:
    case F_to_unq:
    case F_from_unq:
        INFO_EMAL_MUSTFILL (arg_info) = FALSE;
        break;

    default:
        DBUG_ASSERT ((0), "unknown prf found!");
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALwith
 *
 *  @brief
 *
 *  @param arg_node with-loop
 *  @param arg_info
 *
 *  @return with-loop
 *
 ***************************************************************************/
node *
EMALwith (node *arg_node, info *arg_info)
{
    ids *i;

    DBUG_ENTER ("EMALwith");

    /*
     * ALLOCLIST is needed to traverse NCODEs and will be rescued there
     */

    /*
     * In order to build a proper suballoc/fill combinations in each Code-Block
     * it is necessary to know which result variables refer to
     * genarray/modarray - withops
     * Furthermore, a code template for the index vector is needed
     */
    INFO_EMAL_WITHOPS (arg_info) = NWITH2_WITHOP (arg_node);
    INFO_EMAL_INDEXVECTOR (arg_info) = MakeIdFromIds (DupOneIds (NWITH_VEC (arg_node)));

    /*
     * Traverse codes
     */
    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    /*
     * Free INDEXVECTOR code template
     */
    INFO_EMAL_INDEXVECTOR (arg_info) = FreeTree (INFO_EMAL_INDEXVECTOR (arg_info));

    /*
     * Rebuild ALLOCLIST by traversing WITHOPS
     */
    if (NWITH_WITHOP (arg_node) != NULL) {
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    }

    /*
     * Allocate memory for the index vector
     */
    if (NWITH_VEC (arg_node) != NULL) {
        INFO_EMAL_ALLOCLIST (arg_info)
          = MakeALS (INFO_EMAL_ALLOCLIST (arg_info), IDS_AVIS (NWITH_VEC (arg_node)),
                     MakeNum (1), MakePrf (F_shape, DupNode (NWITH_BOUND1 (arg_node))));
    }

    /*
     * Allocate memory for the index variables
     */
    i = NWITH_IDS (arg_node);
    while (i != NULL) {
        INFO_EMAL_ALLOCLIST (arg_info)
          = MakeALS (INFO_EMAL_ALLOCLIST (arg_info), IDS_AVIS (i), MakeNum (0),
                     CreateZeroVector (0, T_int));

        i = IDS_NEXT (i);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALwith2
 *
 *  @brief
 *
 *  @param arg_node with-loop2
 *  @param arg_info
 *
 *  @return with-loop2
 *
 ***************************************************************************/
node *
EMALwith2 (node *arg_node, info *arg_info)
{
    ids *i;

    DBUG_ENTER ("EMALwith2");

    /*
     * ALLOCLIST is needed to traverse NCODEs and will be rescued there
     */

    /*
     * In order to build a proper suballoc/fill combinations in each Code-Block
     * it is necessary to know which result variables refer to
     * genarray/modarray - withops
     * Furthermore, a code template for the index vector is needed
     */
    INFO_EMAL_WITHOPS (arg_info) = NWITH2_WITHOP (arg_node);
    INFO_EMAL_INDEXVECTOR (arg_info) = MakeIdFromIds (DupOneIds (NWITH2_VEC (arg_node)));

    /*
     * Traverse codes
     */
    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    /*
     * Free INDEXVECTOR code template
     */
    INFO_EMAL_INDEXVECTOR (arg_info) = FreeTree (INFO_EMAL_INDEXVECTOR (arg_info));

    /*
     * Rebuild ALLOCLIST by traversing WITHOPS
     */
    if (NWITH2_WITHOP (arg_node) != NULL) {
        NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
    }

    if (NWITH2_VEC (arg_node) != NULL) {
        INFO_EMAL_ALLOCLIST (arg_info)
          = MakeALS (INFO_EMAL_ALLOCLIST (arg_info), IDS_AVIS (NWITH2_VEC (arg_node)),
                     MakeNum (1),
                     SHShape2Array (TYGetShape (
                       AVIS_TYPE (IDS_AVIS (NWITHID_VEC (NWITH2_WITHID (arg_node)))))));
    }

    /*
     * Allocate memory for the index variables
     */
    i = NWITH2_IDS (arg_node);
    while (i != NULL) {
        INFO_EMAL_ALLOCLIST (arg_info)
          = MakeALS (INFO_EMAL_ALLOCLIST (arg_info), IDS_AVIS (i), MakeNum (0),
                     CreateZeroVector (0, T_int));

        i = IDS_NEXT (i);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALwithop
 *
 *  @brief
 *
 *  @param arg_node with-loop
 *  @param arg_info
 *
 *  @return with-loop
 *
 ***************************************************************************/
node *
EMALwithop (node *arg_node, info *arg_info)
{
    alloclist_struct *als;

    DBUG_ENTER ("EMALwithop");

    DBUG_ASSERT (INFO_EMAL_ALLOCLIST (arg_info) != NULL,
                 "ALLOCLIST must contain an entry for each WITHOP!");

    als = INFO_EMAL_ALLOCLIST (arg_info);
    INFO_EMAL_ALLOCLIST (arg_info) = als->next;
    als->next = NULL;

    if (NWITHOP_NEXT (arg_node) != NULL) {
        NWITHOP_NEXT (arg_node) = Trav (NWITHOP_NEXT (arg_node), arg_info);
    }

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_foldprf:
    case WO_foldfun:
        /*
         * fold-wl:
         * fold wls allocate memory on their own
         */
        als = FreeALS (als);
        break;

    case WO_genarray:
        /*
         * If shape information has not yet been gathered it must be
         * inferred using the default element
         */
        if (als->dim == NULL) {
            DBUG_ASSERT (NWITHOP_DEFAULT (arg_node) != NULL, "Default element required!");
            als->dim = MakePrf2 (F_add_SxS,
                                 MakePrf2 (F_sel, MakeNum (0),
                                           MakePrf (F_shape,
                                                    DupNode (NWITHOP_SHAPE (arg_node)))),
                                 MakePrf (F_dim, DupNode (NWITHOP_DEFAULT (arg_node))));
        }

        if (als->shape == NULL) {
            DBUG_ASSERT (NWITHOP_DEFAULT (arg_node) != NULL, "Default element required!");
            als->shape
              = MakePrf (F_shape,
                         MakePrf2 (F_genarray, DupNode (NWITHOP_SHAPE (arg_node)),
                                   DupNode (NWITHOP_DEFAULT (arg_node))));
        }

        /*
         * Annotate which memory is to be used
         */
        NWITHOP_MEMAVIS (arg_node) = als->avis;

        /*
         * genarray-wl:
         * Allocation must remain in ALLOCLIST
         */
        als->next = INFO_EMAL_ALLOCLIST (arg_info);
        INFO_EMAL_ALLOCLIST (arg_info) = als;
        break;

    case WO_modarray:
        /*
         * modarray-wl:
         * dim, shape are the same as in the modified array
         */
        als->dim = MakePrf (F_dim, DupNode (NWITHOP_ARRAY (arg_node)));
        als->shape = MakePrf (F_shape, DupNode (NWITHOP_ARRAY (arg_node)));

        /*
         * Annotate which memory is to be used
         */
        NWITHOP_MEMAVIS (arg_node) = als->avis;

        /*
         * modarray-wl:
         * Allocation must remain in ALLOCLIST
         */
        als->next = INFO_EMAL_ALLOCLIST (arg_info);
        INFO_EMAL_ALLOCLIST (arg_info) = als;
        break;

    case WO_unknown:
        DBUG_ASSERT ((0), "Unknown Withloop-Type found");
    }

    DBUG_RETURN (arg_node);
}
/**
 * @}
 */

/** <!--******************************************************************-->
 *
 * @fn EMAllocateFill
 *
 *  @brief Starting function of transformation SAC -> SAC-MemVal.
 *
 *  @param syntax_tree
 *
 *  @return modified syntax tree containing explicit alloc and fill prfs
 *
 ***************************************************************************/
node *
EMAllocateFill (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("EMALAllocateFill");

    info = MakeInfo ();

    act_tab = emalloc_tab;
    syntax_tree = Trav (syntax_tree, info);

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/**
 * @}
 */
