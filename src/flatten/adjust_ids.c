/*
 *
 * $Log$
 * Revision 1.1  2000/02/17 16:15:25  cg
 * Initial revision
 *
 *
 *
 */

/*****************************************************************************
 *
 * file:   adjust_ids.c
 *
 * prefix: AI
 *
 * description:
 *
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
 *   Its capabilities are used by Fun2Lac.
 *
 *****************************************************************************/

#include "dbug.h"

#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "internal_lib.h"
#include "DupTree.h"

/******************************************************************************
 *
 * function:
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

static node *
FindOrMakeVardec (char *var_name, node *fundef, node *vardec_or_arg)
{
    node *vardec;

    DBUG_ENTER ("*FindOrMakeVardec");

    vardec = BLOCK_VARDEC (FUNDEF_BODY (fundef));

    while (vardec != NULL) {
        if (0 == strcmp (var_name, VARDEC_NAME (vardec))) {
            break;
        }
        vardec = VARDEC_NEXT (vardec);
    }

    if (vardec == NULL) {
        if (NODE_TYPE (vardec_or_arg) == N_vardec) {
            vardec = DupNode (vardec_or_arg);
            VARDEC_NEXT (vardec) = BLOCK_VARDEC (FUNDEF_BODY (fundef));
        } else {
            vardec = MakeVardec (StringCopy (ARG_NAME (vardec_or_arg)),
                                 DupTypes (ARG_TYPE (vardec_or_arg)),
                                 BLOCK_VARDEC (FUNDEF_BODY (fundef)));
            VARDEC_STATUS (vardec) = ARG_STATUS (vardec_or_arg);
            if ((ARG_ATTRIB (vardec_or_arg) == ST_unique)
                || (ARG_ATTRIB (vardec_or_arg) == ST_reference)
                || (ARG_ATTRIB (vardec_or_arg) == ST_readonly_reference)) {
                VARDEC_ATTRIB (vardec) = ST_unique;
            }
        }
        BLOCK_VARDEC (FUNDEF_BODY (fundef)) = vardec;
    }

    DBUG_RETURN (vardec);
}

/******************************************************************************
 *
 * function:
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

ids *
AIids (ids *arg_ids, node *arg_info)
{
    node *new_let;

    DBUG_ENTER ("AIids");

    if (0 != strcmp (IDS_NAME (arg_ids), IDS_NAME (INFO_AI_IDS (arg_info)))) {
        new_let = MakeLet (MakeId (StringCopy (IDS_NAME (INFO_AI_IDS (arg_info))), NULL,
                                   ST_regular),
                           arg_ids);
        ID_VARDEC (LET_EXPR (new_let))
          = FindOrMakeVardec (IDS_NAME (INFO_AI_IDS (arg_info)),
                              INFO_AI_FUNDEF (arg_info),
                              IDS_VARDEC (INFO_AI_IDS (arg_info)));
        INFO_AI_POSTASSIGN (arg_info)
          = MakeAssign (new_let, INFO_AI_POSTASSIGN (arg_info));
        arg_ids
          = MakeIds (StringCopy (IDS_NAME (INFO_AI_IDS (arg_info))), NULL, ST_regular);
        IDS_VARDEC (arg_ids) = ID_VARDEC (LET_EXPR (new_let));
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
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

node *
AIassign (node *arg_node, node *arg_info)
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
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

node *
AIexprs (node *arg_node, node *arg_info)
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
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

node *
AIreturn (node *arg_node, node *arg_info)
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
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

node *
AIap (node *arg_node, node *arg_info)
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
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

node *
AIid (node *arg_node, node *arg_info)
{
    node *new_let;
    char *desired_name;

    DBUG_ENTER ("AIid");

    DBUG_ASSERT ((ID_VARDEC (arg_node) != NULL),
                 "Missing back reference for identifier.");

    FREE (ID_NAME (arg_node));
    ID_NAME (arg_node) = StringCopy (VARDEC_OR_ARG_NAME (ID_VARDEC (arg_node)));

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
            new_let = MakeLet (arg_node,
                               MakeIds (StringCopy (IDS_NAME (INFO_AI_IDS (arg_info))),
                                        NULL, ST_regular));

            IDS_VARDEC (LET_IDS (new_let))
              = FindOrMakeVardec (IDS_NAME (INFO_AI_IDS (arg_info)),
                                  INFO_AI_FUNDEF (arg_info),
                                  IDS_VARDEC (INFO_AI_IDS (arg_info)));
            INFO_AI_PREASSIGN (arg_info)
              = MakeAssign (new_let, INFO_AI_PREASSIGN (arg_info));
            arg_node
              = MakeId (StringCopy (IDS_NAME (INFO_AI_IDS (arg_info))), NULL, ST_regular);
            ID_VARDEC (arg_node) = IDS_VARDEC (LET_IDS (new_let));
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

            IDS_VARDEC (LET_IDS (new_let))
              = FindOrMakeVardec (desired_name, INFO_AI_FUNDEF (arg_info),
                                  ID_VARDEC (EXPRS_EXPR (INFO_AI_ARGS (arg_info))));

            INFO_AI_PREASSIGN (arg_info)
              = MakeAssign (new_let, INFO_AI_PREASSIGN (arg_info));
            arg_node = MakeId (StringCopy (desired_name), NULL, ST_regular);
            ID_VARDEC (arg_node) = IDS_VARDEC (LET_IDS (new_let));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

node *
AIlet (node *arg_node, node *arg_info)
{
    ids *var;

    DBUG_ENTER ("AIlet");

    var = LET_IDS (arg_node);

    while (var != NULL) {
        DBUG_ASSERT ((IDS_VARDEC (var) != NULL),
                     "Missing back reference for identifier.");

        FREE (IDS_NAME (var));
        IDS_NAME (var) = StringCopy (VARDEC_OR_ARG_NAME (IDS_VARDEC (var)));

        var = IDS_NEXT (var);
    }

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
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

node *
AIvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("AIvardec");

    FREE (VARDEC_NAME (arg_node));

    VARDEC_NAME (arg_node) = TmpVar ();

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

node *
AIarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("AIarg");

    DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (INFO_AI_ARGS (arg_info))) == N_id),
                 "Illegal actual parameter in AIarg()");

    FREE (ARG_NAME (arg_node));

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
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

node *
AIblock (node *arg_node, node *arg_info)
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
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

node *
AIfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("AIfundef");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

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
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

node *
AdjustIdentifiers (node *fundef, node *let)
{
    node *info_node;
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

    info_node = MakeInfo ();

    INFO_AI_IDS_CHAIN (info_node) = LET_IDS (let);
    INFO_AI_ARGS_CHAIN (info_node) = AP_ARGS (LET_EXPR (let));

    fundef = Trav (fundef, info_node);

    FreeNode (info_node);

    act_tab = old_tab;

    DBUG_RETURN (fundef);
}
