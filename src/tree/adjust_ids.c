/*
 *
 * $Log$
 * Revision 1.7  2004/09/28 14:11:18  ktr
 * removed old refcount and generatemasks
 *
 * Revision 1.6  2004/09/22 17:40:40  ktr
 * Commented out some lines not needed in EMM
 *
 * Revision 1.5  2004/08/01 16:00:43  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.4  2001/05/17 11:37:55  dkr
 * FREE/MALLOC eliminated
 *
 * Revision 1.3  2001/05/08 13:16:22  dkr
 * new macros for RC used
 *
 * Revision 1.2  2001/03/22 19:51:01  dkr
 * include of tree.h eliminated
 *
 * Revision 1.1  2000/12/15 18:31:14  dkr
 * Initial revision
 *
 * Revision 3.1  2000/11/20 17:59:16  sacbase
 * new release made
 *
 * Revision 1.8  2000/10/31 23:20:16  dkr
 * Trav: NWITH2_CODE might be NULL
 *
 * Revision 1.7  2000/06/05 12:34:03  dkr
 * functions AIwith, AIwith2, AIcode added
 *
 * Revision 1.6  2000/05/29 14:33:34  dkr
 * new traversal function AIwithid() added
 * new abstractions RenameIds(), RenameId() added
 *
 * Revision 1.5  2000/05/25 23:02:16  dkr
 * added reference to Precompile() in header
 *
 * Revision 1.4  2000/05/08 11:37:53  cg
 * Comment added.
 *
 * Revision 1.3  2000/03/02 17:50:11  cg
 * Added support for debug printing functions right after having
 * adjusted the identifiers.
 *
 * Revision 1.2  2000/02/18 14:03:55  cg
 * Minor bugs fixed.
 *
 * Revision 1.1  2000/02/17 16:15:25  cg
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   adjust_ids.c
 *
 * prefix: AI
 *
 * description:
 *   This compiler module implements the renaming of the local identifiers
 *   of a function definition.
 *
 *   - All formal parameters are made identical to the actual parameters
 *     of the corresponding function application.
 *   - All variables in the return statement are made identical to the
 *     assigned variables of the corresponding function application.
 *   - All local variables are given fresh unique names.
 *   - If a formal parameter is assigned a new value, it is replaced by
 *     a fresh unique local identifier.
 *   - If the function is called recursively from inside itself, then the
 *     argument and return value identifers are made identical to those
 *     of the external function application.
 *
 *   This compiler module is used to prepare a function definition for
 *   more or less naive inlining.
 *
 *   Its capabilities are used by Fun2Lac() and Precompile().
 *
 *****************************************************************************/

#define NEW_INFO

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"

/*
 * INFO structure
 */
struct INFO {
    ids *ids;
    void *ids_chain;
    node *args;
    node *args_chain;
    node *fundef;
    node *preassign;
    node *postassign;
};

/*
 * INFO macros
 */
#define INFO_AI_IDS(n) (n->ids)
#define INFO_AI_IDS_CHAIN(n) (n->ids_chain)
#define INFO_AI_ARGS(n) (n->args)
#define INFO_AI_ARGS_CHAIN(n) (n->args_chain)
#define INFO_AI_FUNDEF(n) (n->fundef)
#define INFO_AI_PREASSIGN(n) (n->preassign)
#define INFO_AI_POSTASSIGN(n) (n->postassign)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_AI_IDS (result) = NULL;
    INFO_AI_IDS_CHAIN (result) = NULL;
    INFO_AI_ARGS (result) = NULL;
    INFO_AI_ARGS_CHAIN (result) = NULL;
    INFO_AI_FUNDEF (result) = NULL;
    INFO_AI_PREASSIGN (result) = NULL;
    INFO_AI_POSTASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * Function:
 *   ids *RenameIds( ids *ids_chain)
 *
 * Description:
 *
 *
 ******************************************************************************/

static ids *
RenameIds (ids *ids_chain)
{
    ids *tmp;

    DBUG_ENTER ("RenameIds");

    tmp = ids_chain;
    while (tmp != NULL) {
        DBUG_ASSERT ((IDS_VARDEC (tmp) != NULL),
                     "Missing back reference for identifier.");

        Free (IDS_NAME (tmp));
        IDS_NAME (tmp) = StringCopy (VARDEC_OR_ARG_NAME (IDS_VARDEC (tmp)));

        tmp = IDS_NEXT (tmp);
    }

    DBUG_RETURN (ids_chain);
}

/******************************************************************************
 *
 * Function:
 *   node *RenameId( node *id_node)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
RenameId (node *id_node)
{
    DBUG_ENTER ("RenameId");

    Free (ID_NAME (id_node));
    ID_NAME (id_node) = StringCopy (VARDEC_OR_ARG_NAME (ID_VARDEC (id_node)));

    DBUG_RETURN (id_node);
}

/******************************************************************************
 *
 * function:
 *   node *FindOrMakeVardec(char *var_name, node *fundef, node *vardec_or_arg)
 *
 * description:
 *
 *
 ******************************************************************************/

#if 0
static
node *FindOrMakeVardec(char *var_name, node *fundef, node *vardec_or_arg) 
{
  node *vardec;
  
  DBUG_ENTER( "FindOrMakeVardec");
  
  vardec = BLOCK_VARDEC(FUNDEF_BODY(fundef));
  
  while (vardec != NULL) {
    if (0 == strcmp(var_name, VARDEC_NAME(vardec))) {
      break;
    }
    vardec = VARDEC_NEXT(vardec);
  }
  
  if (vardec == NULL) {
    if (NODE_TYPE(vardec_or_arg) == N_vardec) {
      vardec = DupNode(vardec_or_arg);
      VARDEC_NEXT(vardec) = BLOCK_VARDEC(FUNDEF_BODY(fundef));
    }
    else {
      vardec = MakeVardec(StringCopy(ARG_NAME(vardec_or_arg)),
                          DupTypes(ARG_TYPE(vardec_or_arg)),
                          BLOCK_VARDEC(FUNDEF_BODY(fundef)));
      VARDEC_STATUS(vardec) = ARG_STATUS(vardec_or_arg);
      if ((ARG_ATTRIB(vardec_or_arg) == ST_unique) 
          || (ARG_ATTRIB(vardec_or_arg) == ST_reference) 
          || (ARG_ATTRIB(vardec_or_arg) == ST_readonly_reference)) {
        VARDEC_ATTRIB(vardec) = ST_unique;
      }
    }
    BLOCK_VARDEC(FUNDEF_BODY(fundef)) = vardec;
  }

  DBUG_RETURN(vardec);
}
#endif /*  0  */

/******************************************************************************
 *
 * function:
 *   ids *AIids(ids *arg_ids, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

ids *
AIids (ids *arg_ids, info *arg_info)
{
    node *new_let;

    DBUG_ENTER ("AIids");

    if (0 != strcmp (IDS_NAME (arg_ids), IDS_NAME (INFO_AI_IDS (arg_info)))) {
        new_let = MakeLet (MakeId (StringCopy (IDS_NAME (INFO_AI_IDS (arg_info))), NULL,
                                   ST_regular),
                           arg_ids);
#if 0
    ID_VARDEC( LET_EXPR( new_let))
      = FindOrMakeVardec( IDS_NAME(INFO_AI_IDS(arg_info)),
                          INFO_AI_FUNDEF(arg_info),
                          IDS_VARDEC(INFO_AI_IDS(arg_info)));
#else
        ID_VARDEC (LET_EXPR (new_let)) = IDS_VARDEC (INFO_AI_IDS (arg_info));
#endif
        /*
         * This back reference is a dirty trick. Formally we would have to introduce
         * a new variable declaration within the adjusted function definition. The
         * problem simply is that once we naively inline this function, there will be
         * two identical variable declarations in the calling context.
         * Therefore, we want to keep the sets of local declarations in the calling
         * function and in the called function disjoint.
         */

        if (!emm) {
            if (RC_IS_INACTIVE (IDS_REFCNT (arg_ids))) {
                ID_REFCNT (LET_EXPR (new_let)) = RC_INACTIVE;
            } else if (RC_IS_ACTIVE (IDS_REFCNT (arg_ids))) {
                ID_REFCNT (LET_EXPR (new_let)) = 1;
            } else {
                DBUG_ASSERT ((0), "illegal RC value found!");
            }
        }
        INFO_AI_POSTASSIGN (arg_info)
          = MakeAssign (new_let, INFO_AI_POSTASSIGN (arg_info));
        arg_ids
          = MakeIds (StringCopy (IDS_NAME (INFO_AI_IDS (arg_info))), NULL, ST_regular);
        IDS_VARDEC (arg_ids) = ID_VARDEC (LET_EXPR (new_let));
        if (!emm) {
            IDS_REFCNT (arg_ids) = ID_REFCNT (LET_EXPR (new_let));
        }

        IDS_NEXT (arg_ids) = IDS_NEXT (LET_IDS (new_let));
        IDS_NEXT (LET_IDS (new_let)) = NULL;
    }

    if (IDS_NEXT (arg_ids) != NULL) {
        INFO_AI_IDS (arg_info) = IDS_NEXT (INFO_AI_IDS (arg_info));
        IDS_NEXT (arg_ids) = AIids (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node *AIassign(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
AIassign (node *arg_node, info *arg_info)
{
    node *preassign, *postassign, *tmp;

    DBUG_ENTER ("AIassign");

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    preassign = INFO_AI_PREASSIGN (arg_info);
    INFO_AI_PREASSIGN (arg_info) = NULL;

    postassign = INFO_AI_POSTASSIGN (arg_info);
    INFO_AI_POSTASSIGN (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (postassign != NULL) {
        tmp = postassign;
        while (ASSIGN_NEXT (tmp) != NULL) {
            tmp = ASSIGN_NEXT (tmp);
        }
        ASSIGN_NEXT (tmp) = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = postassign;
    }

    if (preassign != NULL) {
        tmp = preassign;
        while (ASSIGN_NEXT (tmp) != NULL) {
            tmp = ASSIGN_NEXT (tmp);
        }
        ASSIGN_NEXT (tmp) = arg_node;
        arg_node = preassign;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AIexprs(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
AIexprs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AIexprs");

    EXPRS_EXPR (arg_node) = Trav (EXPRS_EXPR (arg_node), arg_info);

    if (INFO_AI_IDS (arg_info) != NULL) {
        INFO_AI_IDS (arg_info) = IDS_NEXT (INFO_AI_IDS (arg_info));
    }

    if (INFO_AI_ARGS (arg_info) != NULL) {
        INFO_AI_ARGS (arg_info) = EXPRS_NEXT (INFO_AI_ARGS (arg_info));
    }

    if (EXPRS_NEXT (arg_node) != NULL) {
        EXPRS_NEXT (arg_node) = Trav (EXPRS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AIreturn(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
AIreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AIreturn");

    INFO_AI_IDS (arg_info) = INFO_AI_IDS_CHAIN (arg_info);

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);
    }

    INFO_AI_IDS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AIap(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
AIap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AIap");

    if (AP_FUNDEF (arg_node) == INFO_AI_FUNDEF (arg_info)) {
        /* recursive function application */
        INFO_AI_ARGS (arg_info) = INFO_AI_ARGS_CHAIN (arg_info);
    }

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    if (AP_FUNDEF (arg_node) == INFO_AI_FUNDEF (arg_info)) {
        /* recursive function application */
        INFO_AI_ARGS (arg_info) = NULL;
        INFO_AI_IDS (arg_info) = INFO_AI_IDS_CHAIN (arg_info);
        /* The latter information is used in AIlet. */
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AIid( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
AIid (node *arg_node, info *arg_info)
{
    node *new_let;
    ids *new_ids;
    char *desired_name;

    DBUG_ENTER ("AIid");

    DBUG_ASSERT ((ID_VARDEC (arg_node) != NULL),
                 "Missing back reference for identifier.");

    arg_node = RenameId (arg_node);

    if (INFO_AI_IDS (arg_info) != NULL) {
        /*
         * Here, we are within a return statement.
         */
        if (0 != strcmp (ID_NAME (arg_node), IDS_NAME (INFO_AI_IDS (arg_info)))) {
            /*
             * The variable in the return statement does not match the corresponding
             * assigned variable in the function application. Therefore, the current
             * id is replaced by the matching variable and an additional assignment
             * is created.
             */
            new_ids = MakeIds_Copy (IDS_NAME (INFO_AI_IDS (arg_info)));

#if 0
      IDS_VARDEC( new_ids)
        = FindOrMakeVardec( IDS_NAME(INFO_AI_IDS(arg_info)),
                            INFO_AI_FUNDEF(arg_info),
                            IDS_VARDEC(INFO_AI_IDS(arg_info)));
#else
            IDS_VARDEC (new_ids) = IDS_VARDEC (INFO_AI_IDS (arg_info));
#endif

            if (!emm) {
                if (RC_IS_INACTIVE (ID_REFCNT (arg_node))) {
                    IDS_REFCNT (new_ids) = RC_INACTIVE;
                } else if (RC_IS_ACTIVE (ID_REFCNT (arg_node))) {
                    IDS_REFCNT (new_ids) = 1;
                } else {
                    DBUG_ASSERT ((0), "illegal RC value found!");
                }
            }

            new_let = MakeLet (arg_node, new_ids);

            INFO_AI_PREASSIGN (arg_info)
              = MakeAssign (new_let, INFO_AI_PREASSIGN (arg_info));
            arg_node = DupIds_Id (INFO_AI_IDS (arg_info));
        }
    }

    if (INFO_AI_ARGS (arg_info) != NULL) {
        /*
         * Here, we are within a recursive application of the transformed function
         * definition.
         */
        desired_name = ID_NAME (EXPRS_EXPR (INFO_AI_ARGS (arg_info)));

        if (0 != strcmp (ID_NAME (arg_node), desired_name)) {
            /*
             * The argument variable in the recursive function application does
             * not match the formal parameter. Consequently, the variable
             * is replaced by the formal parameter and an additional assignment
             * is created.
             */
            new_let
              = MakeLet (arg_node, MakeIds (StringCopy (desired_name), NULL, ST_regular));
#if 0
      IDS_VARDEC( LET_IDS( new_let))
        = FindOrMakeVardec( desired_name,
                            INFO_AI_FUNDEF(arg_info),
                            ID_VARDEC(EXPRS_EXPR(INFO_AI_ARGS(arg_info))));
#else
            IDS_VARDEC (LET_IDS (new_let))
              = ID_VARDEC (EXPRS_EXPR (INFO_AI_ARGS (arg_info)));
#endif

            if (!emm) {
                if (RC_IS_INACTIVE (ID_REFCNT (arg_node))) {
                    IDS_REFCNT (LET_IDS (new_let)) = RC_INACTIVE;
                } else if (RC_IS_ACTIVE (ID_REFCNT (arg_node))) {
                    IDS_REFCNT (LET_IDS (new_let)) = 1;
                } else {
                    DBUG_ASSERT ((0), "illegal RC value found!");
                }
            }
            INFO_AI_PREASSIGN (arg_info)
              = MakeAssign (new_let, INFO_AI_PREASSIGN (arg_info));
            arg_node = MakeId (StringCopy (desired_name), NULL, ST_regular);
            ID_VARDEC (arg_node) = IDS_VARDEC (LET_IDS (new_let));
            if (!emm) {
                ID_REFCNT (arg_node) = IDS_REFCNT (LET_IDS (new_let));
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AIlet(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
AIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AIlet");

    LET_IDS (arg_node) = RenameIds (LET_IDS (arg_node));

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    if (INFO_AI_IDS (arg_info) != NULL) {
        LET_IDS (arg_node) = AIids (LET_IDS (arg_node), arg_info);
        INFO_AI_IDS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AIvardec(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
AIvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AIvardec");

    Free (VARDEC_NAME (arg_node));
    VARDEC_NAME (arg_node) = TmpVar ();

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AIarg( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
AIarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AIarg");

    DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (INFO_AI_ARGS (arg_info))) == N_id),
                 "Illegal actual parameter in AIarg()");

    Free (ARG_NAME (arg_node));
    ARG_NAME (arg_node) = StringCopy (ID_NAME (EXPRS_EXPR (INFO_AI_ARGS (arg_info))));

    INFO_AI_ARGS (arg_info) = EXPRS_NEXT (INFO_AI_ARGS (arg_info));

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AIblock(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
AIblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AIblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AIfundef(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
AIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AIfundef");

    INFO_AI_ARGS (arg_info) = INFO_AI_ARGS_CHAIN (arg_info);

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    INFO_AI_ARGS (arg_info) = NULL;

    INFO_AI_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    INFO_AI_FUNDEF (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AIwith( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
AIwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AIwith");

    NWITH_DEC_RC_IDS (arg_node) = RenameIds (NWITH_DEC_RC_IDS (arg_node));

    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AIwith2( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
AIwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AIwith2");

    NWITH2_DEC_RC_IDS (arg_node) = RenameIds (NWITH2_DEC_RC_IDS (arg_node));

    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);

    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AIwithid( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
AIwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AIwithid");

    NWITHID_VEC (arg_node) = RenameIds (NWITHID_VEC (arg_node));
    NWITHID_IDS (arg_node) = RenameIds (NWITHID_IDS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AIcode( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
AIcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AIcode");

    NCODE_INC_RC_IDS (arg_node) = RenameIds (NCODE_INC_RC_IDS (arg_node));

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AdjustIdentifiers(node *fundef, node *let)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
AdjustIdentifiers (node *fundef, node *let)
{
    info *info;
    funtab *old_tab;

    DBUG_ENTER ("AdjustIdentifiers");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "Illegal 1st argument to AdjustIdentifiers().");
    DBUG_ASSERT ((NODE_TYPE (let) == N_let),
                 "Illegal 2nd argument to AdjustIdentifiers().");
    DBUG_ASSERT ((NODE_TYPE (LET_EXPR (let)) == N_ap),
                 "Illegal node in 2nd argument to AdjustIdentifiers().");

    old_tab = act_tab;
    act_tab = ai_tab;

    info = MakeInfo ();

    INFO_AI_IDS_CHAIN (info) = LET_IDS (let);
    INFO_AI_ARGS_CHAIN (info) = AP_ARGS (LET_EXPR (let));

    fundef = Trav (fundef, info);

    DBUG_EXECUTE ("PRINT_AI", PrintNode (fundef););

    info = FreeInfo (info);

    act_tab = old_tab;

    DBUG_RETURN (fundef);
}
