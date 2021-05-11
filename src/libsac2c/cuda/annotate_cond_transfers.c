/*****************************************************************************
 *
 *
 * file:   annotate_cond_transfers.c
 *
 * prefix: ACTRAN
 *
 * brief:
 *    overall, we attempt to move _host2device_ calls up and _device2host_
 *    calls down. This phase looks at conditional functions and annotates
 *    the assignments of all prf-calls to _host2device_ or _device2host_ as
 *    follows:
 *    _host2device_ calls are annotated as ASSIGN_ISNOTALLOWEDTOBEMOVEDUP iff
 *     
 *    _device2host_ calls are annotated as ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN iff
 *
 *    Cond-Functions have the general form:
 *
 *    rt1, ..., rtn Cond( bool p, a1, ..., am)
 *    {
 *       if (p) {
 *          ti = _host2device_(ak); //ASSIGN_ISNOTALLOWEDTOBEMOVEDUP!
 *          ... ak ...
 *       } else {
 *          ej = _device2host_(a_dev); // ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN!
 *          ... ej ...
 *       }
 *       r1 = ( p ? t1 : e1);   // occurrences here are being counted too!
 *       ...
 *       rn = ( p ? tn : en);
 *       return (r1, ..., rn);
 *    }
 *
 *****************************************************************************/

#include "annotate_cond_transfers.h"

#include <stdlib.h>
#include "tree_basic.h"
#include "tree_compound.h"

#define DBUG_PREFIX "ACTRAN"
#include "debug.h"

#include "traverse.h"
#include "memory.h"
#include "NumLookUpTable.h"
#include "cuda_utils.h"
#include "new_types.h"

enum traverse_mode { trav_collect, trav_annotate };

/*
 * INFO structure
 */
struct INFO {
    bool incondfun;
    nlut_t *nlut;
    enum traverse_mode travmode;
    int funargnum;
    node *lastassign;
    node *recursive_apargs;
    bool inrecursiveapargs;
    bool infuncond;
    node *fundef;
    node *letids;
};

/*
 * INFO macros
 */

#define INFO_NLUT(n) (n->nlut)
#define INFO_TRAVMODE(n) (n->travmode)
#define INFO_FUNARGNUM(n) (n->funargnum)
#define INFO_LASTASSIGN(n) (n->lastassign)
#define INFO_INCONDFUN(n) (n->incondfun)
#define INFO_RECURSIVE_APARGS(n) (n->recursive_apargs)
#define INFO_INRECURSIVEAPARGS(n) (n->inrecursiveapargs)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LETIDS(n) (n->letids)
#define INFO_INFUNCOND(n) (n->infuncond)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_NLUT (result) = NULL;
    INFO_TRAVMODE (result) = trav_collect;
    INFO_FUNARGNUM (result) = 0;
    INFO_LASTASSIGN (result) = NULL;
    INFO_INCONDFUN (result) = FALSE;
    INFO_RECURSIVE_APARGS (result) = NULL;
    INFO_INRECURSIVEAPARGS (result) = FALSE;
    INFO_FUNDEF (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_INFUNCOND (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *AMTRANdoAnnotateMemoryTransfers( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACTRANdoAnnotateCondTransfers (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ();

    info = MakeInfo ();
    TRAVpush (TR_actran);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *AMTRANfundef( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACTRANfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

    /* We only traverse cond-fun. */
    if (FUNDEF_ISCONDFUN (arg_node)) {
        INFO_INCONDFUN (arg_info) = TRUE;
        /* First traversal, collect variable usage information */
        INFO_NLUT (arg_info)
          = NLUTgenerateNlut (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));
        INFO_TRAVMODE (arg_info) = trav_collect;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

        /* Second traversal, annotate <host2device> and <device2host> */
        INFO_TRAVMODE (arg_info) = trav_annotate;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_NLUT (arg_info) = NLUTremoveNlut (INFO_NLUT (arg_info));
        INFO_INCONDFUN (arg_info) = FALSE;
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *ACTRANassign( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACTRANassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LASTASSIGN (arg_info) = arg_node;
    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *AMTRANlet( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACTRANlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *AMTRANfuncond( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACTRANfuncond (node *arg_node, info *arg_info)
{
    node *then_id, *else_id;
    /* ntype *then_type, *else_type; */
    node *then_ssaassign, *else_ssaassign;

    DBUG_ENTER ();

    /* N_funcond can only appear after the N_cond node in cond-fun
     * The N_funcond selects value from either branch of the N_cond
     * as the return value of the cond-fun.
     */
    if (INFO_INCONDFUN (arg_info)) {
        if (INFO_TRAVMODE (arg_info) == trav_annotate) {
            then_id = FUNCOND_THEN (arg_node);
            else_id = FUNCOND_ELSE (arg_node);
            DBUG_ASSERT ((NODE_TYPE (then_id) == N_id && NODE_TYPE (else_id) == N_id),
                         "N_funcond has non N_id node in either THEN or ELSE!");
            /*
                  then_type = AVIS_TYPE( ID_AVIS( then_id));
                  else_type = AVIS_TYPE( ID_AVIS( else_id));
                  cond = !TYisAUD( then_type) &&
                         !TYisAUD( else_type) &&
                         TYeqTypes( then_type, else_type);
            */

            then_ssaassign = AVIS_SSAASSIGN (ID_AVIS (then_id));
            else_ssaassign = AVIS_SSAASSIGN (ID_AVIS (else_id));

            if (ISDEVICE2HOST (then_ssaassign) && ISDEVICE2HOST (else_ssaassign)) {
                ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (then_ssaassign) = FALSE;
                ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (else_ssaassign) = FALSE;
            } else if (ISDEVICE2HOST (then_ssaassign)) {
                if (NODE_TYPE (AVIS_DECL (ID_AVIS (else_id))) == N_arg) {
                    if (NLUTgetNum (INFO_NLUT (arg_info), ARG_AVIS (ID_DECL (else_id)))
                        != 0) {
                        ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (then_ssaassign) = TRUE;
                    }
                } else {
                    ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (then_ssaassign) = TRUE;
                }
            } else if (ISDEVICE2HOST (else_ssaassign)) {
                if (NODE_TYPE (AVIS_DECL (ID_AVIS (then_id))) == N_arg) {
                    if (NLUTgetNum (INFO_NLUT (arg_info), ARG_AVIS (ID_DECL (then_id)))
                        != 0) {
                        ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (else_ssaassign) = TRUE;
                    }
                } else {
                    ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (else_ssaassign) = TRUE;
                }
            } else {
                /* Do nothing */
            }
        } else {
            DBUG_ASSERT (INFO_TRAVMODE (arg_info) == trav_collect,
                         "illegal traversal mode in ACTRAN");
            /* Here, we traverse all N_id's of the funcond to make sure we capture
             * all possible occurences of variables.
             * This is in response to bug 626 but may adversely affect the ability
             * to memopt loops.... (sbs, 2015)
             */
            arg_node = TRAVcont (arg_node, arg_info);
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *ACTRANid( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACTRANid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_INCONDFUN (arg_info)) {
        if (INFO_TRAVMODE (arg_info) == trav_collect) {
            NLUTincNum (INFO_NLUT (arg_info), ID_AVIS (arg_node), 1);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *AMTRANprf( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACTRANprf (node *arg_node, info *arg_info)
{
    node *id;
    DBUG_ENTER ();

    if (INFO_INCONDFUN (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_host2device:
            /* Ensure that each <host2device> is initially
             * tagged as can be moved out. */
            if (INFO_TRAVMODE (arg_info) == trav_collect) {
                ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (INFO_LASTASSIGN (arg_info)) = FALSE;
            }
            /* If we are in trav_annotate traverse mode */
            if (INFO_TRAVMODE (arg_info) == trav_annotate) {
                id = PRF_ARG1 (arg_node);
                /* We only look at <host2device> whose host variable
                 * is passed in as an argument of the cond-fun*/
                if (NODE_TYPE (ID_DECL (id)) == N_arg) {

                    /* If the reference count of the host variable is not 0,
                     * we annotates the transfer to be not allowed to be moved out.
                     */
                    if (NLUTgetNum (INFO_NLUT (arg_info), ID_AVIS (id)) != 0) {
                        ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (INFO_LASTASSIGN (arg_info))
                          = TRUE;
                    }
                }
                /* If the host variable is not passed as an argument to the cond-fun,
                 * the <host2device> CANNOT be moved out of the cond-fun.
                 */
                else {
                    ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (INFO_LASTASSIGN (arg_info)) = TRUE;
                }
            }
            break;
        case F_device2host:
            /* Ensure that each <device2host> is initially
             * tagged as can be moved out */
            if (INFO_TRAVMODE (arg_info) == trav_collect) {
                ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (INFO_LASTASSIGN (arg_info)) = FALSE;
            }
            if (INFO_TRAVMODE (arg_info) == trav_annotate) {
                /* If the reference count of the host variable is not 0,
                 * we annotates the transfer to be not allowed to be moved out.
                 */
                if (NLUTgetNum (INFO_NLUT (arg_info), IDS_AVIS (INFO_LETIDS (arg_info)))
                    != 0) {
                    ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (INFO_LASTASSIGN (arg_info)) = TRUE;
                }
            }
            break;
        default:
            PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
            break;
        }
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
