/*
 *
 * $Log$
 * Revision 1.1  2004/11/25 15:14:37  ktr
 * Initial revision
 *
 */
/**
 * @defgroup opc Object Precompilation
 * @ingroup prec
 *
 *   - Artificial arguments and return values are removed.
 *   - A function with code for object initialization is created.
 *
 * @{
 */

/**
 *
 * @file objectprecompile.c
 *
 *
 *
 */

#include "objectprecompile.h"

#include <string.h>
#include "internal_lib.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "precompile.h"
#include "traverse.h"
#include "globals.h"
#include "free.h"
#include "DataflowMask.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *objinitfundef;
    node *modul;
    node *rhs;
    types *rettypes;
};

/*
 * INFO macros
 */
#define INFO_OPC_FUNDEF(n) (n->fundef)
#define INFO_OPC_OBJINITFUNDEF(n) (n->objinitfundef)
#define INFO_OPC_MODUL(n) (n->modul)
#define INFO_OPC_RETTYPES(n) (n->rettypes)
#define INFO_OPC_RHS(n) (n->rhs)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_OPC_FUNDEF (result) = NULL;
    INFO_OPC_OBJINITFUNDEF (result) = NULL;
    INFO_OPC_MODUL (result) = NULL;
    INFO_OPC_RETTYPES (result) = NULL;
    INFO_OPC_RHS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/** <!--******************************************************************-->
 *
 * @fn OPCdoObjectPreCompile
 *
 *  @brief
 *
 *  @param syntax_tree
 *
 *  @return modified syntax tree containing explicit memory management
 *          instructions
 *
 ***************************************************************************/
node *
OPCdoObjectPrecompile (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("OPCdoObjectPrecompile");

    info = MakeInfo ();

    TRAVpush (TR_emrc);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   char *OPCobjInitFunctionName( bool before_rename)
 *
 * description:
 *   Returns new allocated string with objinitfunction name
 *
 * parameters:
 *   uses global variable modulename!
 *
 ******************************************************************************/

char *
OPCobjInitFunctionName (bool before_rename)
{
    char *name = "GlobalObjInit";
    char *new_name;

    DBUG_ENTER ("OPCobjInitFunctionName");

    if (before_rename) {
        new_name = (char *)ILIBmalloc (strlen (name) + 1);

        strcpy (new_name, name);
    } else {
        new_name = PRECrenameFunName (global.modulename, name, ST_regular, NULL);
    }

    DBUG_RETURN (new_name);
}

/******************************************************************************
 *
 * function:
 *   node *CreateObjInitFundef( node *module, info *arg_info)
 *
 * description:
 *   builds up new fundef with empty block, that will contain all init calls
 *   for global objects. This functions will be called during the startup in
 *   the main function or from a separate init functions when used in
 *   a c library
 *
 * parameters:
 *   module where to add fundef
 *   arg_info, to tag new inserted fundef
 *
 * returns:
 *   modified module
 *
 ******************************************************************************/

static node *
CreateObjInitFundef (node *module, info *arg_info)
{
    node *fundef;
    node *assign;
    node *returns;

    DBUG_ENTER ("CreateObjInitFundef");

    returns = TBmakeReturn (NULL);
    assign = TBmakeAssign (returns, NULL);

    /* create void procedure without args and with empty return in body */
    fundef = TBmakeFundef (OPCobjInitFunctionName (TRUE), MODULE_NAME (module), NULL,
                           NULL, TBmakeBlock (assign, NULL), MODULE_FUNS (module));

    FUNDEF_TYPES (fundef) = TBmakeTypes1 (T_void);
    FUNDEF_RETURN (fundef) = returns;

    MODULE_FUNS (module) = fundef;

    INFO_OPC_OBJINITFUNDEF (arg_info) = fundef;

    DBUG_RETURN (module);
}

/******************************************************************************
 *
 * Function:
 *   node *InsertObjInit( node *block, node *objdef)
 *
 * Description:
 *   For this global object defined in SAC an application of its generic
 *   initialization function is inserted at the beginning of block.
 *
 ******************************************************************************/

static node *
InsertObjInit (node *block, node *objdef)
{
    node *new_ids;
    node *new_id;

    DBUG_ENTER ("InsertObjInit");

    new_ids = TBmakeIds (OBJDEF_AVIS (objdef), NULL);
    new_id = TBmakeId (OBJDEF_AVIS (objdef));

    BLOCK_INSTR (block)
      = TCmakeAssignIcm1 ("INITGLOBALOBJECT_BEGIN", new_id,
                          TBmakeAssign (TBmakeLet (new_ids, OBJDEF_EXPR (objdef)),
                                        TCmakeAssignIcm0 ("INITGLOBALOBJECT_END",
                                                          BLOCK_INSTR (block))));

    OBJDEF_EXPR (objdef) = NULL;

    DBUG_RETURN (block);
}

/****************************************************************************
 *
 *  OBJECT PRECOMPILATION TRAVERSAL FUNCTIONS
 *
 ****************************************************************************/
/******************************************************************************
 *
 * function:
 *   node *OPCmodule( node *arg_node, info *arg_info)
 *
 * description:
 *   Creates fundef for object-initialization code.
 *
 ******************************************************************************/

node *
OPCmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OPCmodule");

    arg_node = CreateObjInitFundef (arg_node, arg_info);

    if (MODULE_TYPES (arg_node) != NULL) {
        MODULE_TYPES (arg_node) = TRAVdo (MODULE_TYPES (arg_node), arg_info);
    }

    if (MODULE_OBJS (arg_node) != NULL) {
        MODULE_OBJS (arg_node) = TRAVdo (MODULE_OBJS (arg_node), arg_info);
    }

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *OPCobjdef( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
OPCobjdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OPCobjdef");

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = TRAVdo (OBJDEF_NEXT (arg_node), arg_info);
    }

    /* insert init code */
    FUNDEF_BODY (INFO_OPC_OBJINITFUNDEF (arg_info))
      = InsertObjInit (FUNDEF_BODY (INFO_OPC_OBJINITFUNDEF (arg_info)), arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *OPCfundef(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
OPCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OPCfundef");

    /*
     * unset inline flag
     */
    FUNDEF_ISINLINE (arg_node) = FALSE;

    /*
     * The function body is traversed in order to remove artificial return
     * values and parameters of function applications.
     */
    if (FUNDEF_BODY (arg_node) != NULL) {
        if (FUNDEF_DFM_BASE (arg_node) == NULL) {
            FUNDEF_DFM_BASE (arg_node)
              = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
        }

        DBUG_ASSERT (((FUNDEF_RETURN (arg_node) != NULL)
                      && (NODE_TYPE (FUNDEF_RETURN (arg_node)) == N_return)),
                     "N_fundef node has no reference to N_return node");

        /*
         * The reference checked above is actually not needed by the
         * precompiler. This is done to check consistency of the syntax
         * tree for further compilation steps.
         */
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    } else {
        DBUG_ASSERT ((FUNDEF_DFM_BASE (arg_node) == NULL),
                     "FUNDEF_DFM_BASE without body found!");
    }

    /*
     * Now, traverse the following functions.
     * All function bodies must be traversed before arguments and
     * return values of functions are modified.
     */
    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    /*
     * The function arguments are traversed, artificial arguments are removed
     * and the number of reference parameters (including global objects)
     * is counted and stored in 'cnt_artificial'
     */
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    DBUG_ASSERT ((FUNDEF_RETS (arg_node) != NULL), "no return value found!");

    /*
     * Traverse RETS in order to remove artificial return values
     */
    INFO_OPC_RETTYPES (arg_info) = FUNDEF_TYPES (arg_node);
    FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
    FUNDEF_TYPES (arg_node) = INFO_OPC_RETTYPES (arg_info);

    if (FUNDEF_RETS (arg_node) == NULL) {
        /*
         * all return types removed -> create T_void
         */
        FUNDEF_TYPES (arg_node) = TBmakeTypes1 (T_void);
        FUNDEF_RETS (arg_node) = TBmakeRet (NULL, NULL);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *OPCarg( node *arg_node, info *arg_info)
 *
 * description:
 *   An artificial argument is removed, the attribs are switched:
 *       ST_readonly_reference -> ST_regular
 *       ST_was_reference -> ST_reference
 *
 ******************************************************************************/

node *
OPCarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OPCarg");

    if (ARG_ISARTIFICIAL (arg_node)) {
        arg_node = FREEdoFreeNode (arg_node);

        if (arg_node != NULL) {
            arg_node = TRAVdo (arg_node, arg_info);
        }
    } else {
        if (ARG_ISREFERENCE (arg_node) && ARG_ISREADONLY (arg_node)) {
#if 0
      /*
       * TODO: correct?
       */
         ARG_ATTRIB( arg_node) = ST_regular;
#endif
            ARG_ISREFERENCE (arg_node) = FALSE;
            ARG_ISREADONLY (arg_node) = FALSE;
        } else {
            if (ARG_WASREFERENCE (arg_node)) {
                ARG_WASREFERENCE (arg_node) = FALSE;
                ARG_ISREFERENCE (arg_node) = TRUE;
            }
        }

        if (ARG_NEXT (arg_node) != NULL) {
            ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *OPCvardec( node *arg_node, info *arg_info)
 *
 * Description:
 *   Removes artificial variable declarations.
 *
 ******************************************************************************/

node *
OPCvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OPCvardec");

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    if (VARDEC_ISARTIFICIAL (arg_node)) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *OPCassign( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
OPCassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OPCassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_INSTR (arg_node) == NULL) {
        arg_node = FREEdoFreeNode (arg_node);
        if (arg_node != NULL) {
            arg_node = TRAVdo (arg_node, arg_info);
        }
    } else {
        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *OPClet( node *arg_node, info *arg_info)
 *
 * description:
 *   Removes all artificial identifiers on the left hand side of a let.
 *
 *****************************************************************************/
node *
OPClet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OPClet");

    if (LET_IDS (arg_node) != NULL) {
        INFO_OPC_RHS (arg_info) = LET_EXPR (arg_node);
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (LET_EXPR (arg_node) == NULL) {
        arg_node = FREEdoFreeTree (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *OPCexprs_ap( node *current, node *formal)
 *
 * Description:
 *   Removes all artificial parameters.
 *   The status of those current parameters which belong to formal reference
 *   parameters is modified to ST_reference.
 *   Global objects given as parameters to the applied function get a reference
 *   to the object definition and are renamed with the new name of the global
 *   object.
 *
 *****************************************************************************/
static node *
OPCexprs_ap (node *current, node *formal)
{
    node *expr;

    DBUG_ENTER ("OPCexprs_ap");

    if (formal != NULL) {
        EXPRS_NEXT (current) = OPCexprs_ap (EXPRS_NEXT (current), ARG_NEXT (formal));

        if (ARG_ISARTIFICIAL (formal)) {
            current = FREEdoFreeNode (current);
        } else {
            expr = EXPRS_EXPR (current);

            if (ARG_WASREFERENCE (formal)) {
                ID_ISREFERENCE (expr) = TRUE;
                ID_ISREADONLY (expr) = FALSE;
            }

            if (AVIS_WASOBJECT (ID_AVIS (expr))) {
                node *object = TBmakeGlobobject (AVIS_OBJDEF (ID_AVIS (expr)));
                expr = FREEdoFreeTree (expr);
                expr = object;
            }

            EXPRS_EXPR (current) = expr;
        }
    }

    DBUG_RETURN (current);
}

/******************************************************************************
 *
 * Function:
 *   node *OPCexprs_return( node *ret_exprs, node *ret_node)
 *
 * Description:
 *   Removes all artificial return values from the chain.
 *   A new chain is built up for all those return values which belong to
 *   original reference parameters. These are stored in RETURN_REFERENCE for
 *   later use in compile.c.
 *
 ******************************************************************************/

static node *
OPCexprs_return (node *ret_exprs, node *return_node, node *formal)
{
    node *tmp;

    DBUG_ENTER ("OPCexprs_return");

    if (formal != NULL) {
        if (EXPRS_NEXT (ret_exprs) != NULL) {
            EXPRS_NEXT (ret_exprs)
              = OPCexprs_return (EXPRS_NEXT (ret_exprs), return_node, RET_NEXT (formal));
        }

        if (RET_ISARTIFICIAL (formal)) {
            if (ARG_ISARTIFICIAL (RET_CORRESPONDINGARG (formal))) {
                /*
                 * This artificial return value belongs to a global object,
                 * so it can be removed.
                 */
                ret_exprs = FREEdoFreeNode (ret_exprs);
            } else {
                /*
                 * This artificial return value belongs to an original reference
                 * parameter, so it is stored in RETURN_REFERENCE to be compiled
                 * to an "inout" parameter.
                 */
                DBUG_ASSERT (ARG_WASREFERENCE (RET_CORRESPONDINGARG (formal)),
                             "Corresponding ARG of artificial RET is "
                             "neither reference nor artificial!");
                tmp = ret_exprs;
                ret_exprs = EXPRS_NEXT (ret_exprs);
                EXPRS_NEXT (tmp) = RETURN_REFERENCE (return_node);
                RETURN_REFERENCE (ret_node) = tmp;
            }
        }
    }

    DBUG_RETURN (ret_exprs);
}

/******************************************************************************
 *
 * Function:
 *   node *OPCap( node *arg_node, info *arg_info)
 *
 * Description:
 *   Traverses the current arguments using function OPCexprs_ap that is given
 *   a pointer to the first formal parameter of the applied function.
 *
 ******************************************************************************/

node *
OPCap (node *arg_node, info *arg_info)
{
    node *arg;
    node *ret_node;

    DBUG_ENTER ("OPCap");

    /*
     * TODO: What about this?
     */
    if (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_classfun) {
        arg = EXPRS_EXPR (AP_ARGS (arg_node));

        if (NODE_TYPE (arg) == N_id) {
            if (!strncmp (AP_NAME (arg_node), "to_", 3)) {
                DBUG_ASSERT ((!TCisUnique (ID_TYPE (arg))),
                             "Argument of to_class function is unique already!");

                ret_node = TBmakePrf (F_to_unq, AP_ARGS (arg_node));
                AP_ARGS (arg_node) = NULL;
            } else {
                /*
                 * This must be a "from" function. So, the argument is of a class
                 * type which implies that it is an identifier.
                 */
                DBUG_ASSERT (TCisUnique (ID_TYPE (arg)),
                             "Argument of from_class function not unique!");

                ret_node = TBmakePrf (F_from_unq, AP_ARGS (arg_node));
                AP_ARGS (arg_node) = NULL;
            }
        } else {
            /* argument of class conversion function is no N_id node */

            /*
             * -> argument is a scalar value
             * -> simply remove the conversion function
             */
            ret_node = arg;
            EXPRS_EXPR (AP_ARGS (arg_node)) = NULL;
        }

        arg_node = ILIBfree (arg_node);
        arg_node = ret_node;
    } else {
        if (AP_ARGS (arg_node) != NULL) {
            AP_ARGS (arg_node)
              = OPCexprs_ap (AP_ARGS (arg_node), FUNDEF_ARGS (AP_FUNDEF (arg_node)));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *OPCreturn( node *arg_node, info *arg_info)
 *
 * Description:
 *   Traverses the return values using function OPCexprs_return.
 *
 ******************************************************************************/

node *
OPCreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OPCreturn");

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = OPCexprs_return (RETURN_EXPRS (arg_node), arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *OPCid( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 *****************************************************************************/

node *
OPCid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OPCid");

  if ( AVIS_WASOBJECT( ID_AVIS( arg_node)) {
        arg_node = FREEdoFreeNode (arg_node);
  }
  else {
        DBUG_ASSERT ((ID_VARDEC (arg_node) != NULL), "no ID_VARDEC found!");

        /*
         * TODO: What about this?
         */
        if (VARDEC_OR_ARG_STATUS (ID_VARDEC (arg_node)) == ST_artificial) {
            ID_VARDEC (arg_node) = VARDEC_OR_ARG_OBJDEF (ID_VARDEC (arg_node));
        }
  }

  DBUG_RETURN( arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *OPCids( node *arg_node, info *arg_info)
 *
 * Description:
 *   This function removes those identifiers from the chain which are marked
 *   as 'artificial'.
 *   If the identifier is found on the LHS of a function application it is
 *   checked that it indeed have a counterpart of the same name on the RHS.
 *
 *****************************************************************************/

node *
OPCids (node *arg_node, info *arg_info)
{
    bool found;
    node *exprs;
    node *rhs;

    DBUG_ENTER ("OPCids");

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    if (IDS_ISARTIFICIAL (arg_node)) {
        rhs = INFO_OPC_RHS (arg_info);

        if ((rhs != NULL) && (NODE_TYPE (rhs) == N_ap)) {
            found = FALSE;
            exprs = AP_ARGS (rhs);
            while ((!found) && (exprs != NULL)) {
                if (NODE_TYPE (EXPRS_EXPR (exprs)) == N_id) {
                    found
                      = (0 == strcmp (ID_NAME (EXPRS_EXPR (exprs)), IDS_NAME (arg_node)));
                }
                exprs = EXPRS_NEXT (exprs);
            }
        } else {
            found = TRUE;
        }

        if (found) {
            arg_node = FREEdoFreeNode (arg_node);
        } else {
            DBUG_ASSERT ((0), "application with corrupted reference argument found!");
        }

        DBUG_RETURN (arg_node);
    }

    /******************************************************************************
     *
     * Function:
     *   node *OPCret( node *arg_node, info *arg_info)
     *
     * Description:
     *
     *
     *****************************************************************************/

    node *OPCret (node * arg_node, info * arg_info)
    {
        types *tmptypes;

        DBUG_ENTER ("OPCret");

        tmptypes = INFO_OPC_RETTYPES (arg_info);
        INFO_OPC_RETTYPES (arg_info) = TYPES_NEXT (tmptypes);

        if (RET_NEXT (arg_node) != NULL) {
            RET_NEXT (arg_node) = TRAVdo (RET_NEXT (arg_node), arg_info);
        }

        TYPES_NEXT (tmptypes) = INFO_OPC_RETTYPES (arg_info);

        if (RET_ISARTIFICIAL (arg_node)) {
            arg_node = FREEdoFreeNode (arg_node);
            tmptypes = FREEfreeOneTypes (tmptypes);
        }

        INFO_OPC_RETTYPES (arg_info) = tmptypes;

        DBUG_RETURN (arg_node);
    }

    /* @} */
