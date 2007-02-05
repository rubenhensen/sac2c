/* $Id$ */

/*
 * Analyzes the set of objects used by functions, then adds them as
 * reference arguments to the function applications. Also adds propagate()
 * calls to with-loops for objects added in this way.
 */

/*
 * The task of the withloop object analysis is to adjust with-loops
 * that reference objects in their body, and make sure the objects are
 * returned from the bodies to the surrounding code, as follows:
 *
 * BEFORE:
 * -------
 *
 * a = with ( iv )  {
 *       ...
 *       stdout = print ( stdout, f ( iv ) );
 *       ...
 *     } : b
 *     genarray ( shp ( b ), ... );
 *
 * AFTER:
 * ------
 *
 * a, stdout = with ( iv ) {
 *       stdout = F_prop_obj_in ( iv, stdout );
 *       ...
 *       stdout' = print ( stdout, f ( iv ) );
 *       ...
 *       stdout' = F_prop_obj_out ( iv, stdout');
 *     } : b, stdout'
 *     genarray ( shp ( b ), ... )
 *     propagate ( stdout );
 */

#include "resolve_objects.h"

#include "dbug.h"
#include "DupTree.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"
#include "free.h"

/*
 * INFO structure
 */

struct INFO {
    bool inwithloop;
    node *fundef;
    node *objects;
    node *propobj_in;
    node *propobj_out;
    node *wl;
};

/*
 * INFO macros
 */

#define INFO_INWITHLOOP(n) ((n)->inwithloop)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_OBJECTS(n) ((n)->objects)
#define INFO_PROPOBJ_IN(n) ((n)->propobj_in)
#define INFO_PROPOBJ_OUT(n) ((n)->propobj_out)
#define INFO_WL(n) ((n)->wl)

/*
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_INWITHLOOP (result) = FALSE;
    INFO_FUNDEF (result) = NULL;
    INFO_OBJECTS (result) = NULL;
    INFO_PROPOBJ_IN (result) = NULL;
    INFO_PROPOBJ_OUT (result) = NULL;
    INFO_WL (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * static helper functions
 */

/** <!-- ****************************************************************** -->
 * @brief Finds the with-loop goal expression that belongs to the given
 * propagate withop.
 *
 * @param prop
 * @param arg_info
 *
 * @return
 ******************************************************************************/
static node *
FindPropagateGoalExpr (node *prop, info *arg_info)
{
    node *wlexpr;
    node *wlop;

    DBUG_ENTER ("FindPropagateGoalExpr");

    wlexpr = CODE_CEXPRS (WITH_CODE (INFO_WL (arg_info)));
    wlop = WITH_WITHOP (INFO_WL (arg_info));

    while (wlop != NULL) {
        if (wlop == prop) {
            return wlexpr;
        }

        wlop = WITHOP_NEXT (wlop);
        wlexpr = EXPRS_NEXT (wlexpr);
    }

    DBUG_RETURN (wlexpr);
}

/** <!-- ****************************************************************** -->
 * @brief Appends the objdefs that are in the objlist set to the parameter
 * set args. It creates avis nodes for the objdefs so that they can be
 * referenced in the function body.
 *
 * @param args current function argument list
 * @param objlist list of objdefs to add
 *
 * @return new argument list
 ******************************************************************************/
static node *
AppendObjdefsToArgs (node *args, node *objlist)
{
    node *avis;

    DBUG_ENTER ("AppendObjdefsToArgs");

    if (objlist != NULL) {
        args = AppendObjdefsToArgs (args, SET_NEXT (objlist));

        avis = TBmakeAvis (ILIBtmpVarName (OBJDEF_NAME (SET_MEMBER (objlist))),
                           TYcopyType (OBJDEF_TYPE (SET_MEMBER (objlist))));
        AVIS_DECLTYPE (avis) = TYcopyType (AVIS_TYPE (avis));

        OBJDEF_ARGAVIS (SET_MEMBER (objlist)) = avis;

        args = TBmakeArg (avis, args);
        ARG_ISARTIFICIAL (args) = TRUE;
        ARG_ISREFERENCE (args) = TRUE;
        ARG_OBJDEF (args) = SET_MEMBER (objlist);
    }

    DBUG_RETURN (args);
}

/** <!-- ****************************************************************** -->
 * @brief Appends the objdefs in objlist to the given exprs list, which should
 * be the arguments of a function application.
 *
 * @param exprs function arguments
 * @param objlist objdefs to add
 *
 * @return new function arguments
 ******************************************************************************/
static node *
AppendObjdefsToArgExprs (node *exprs, node *objlist)
{
    DBUG_ENTER ("AppendObjdefsToArgExprs");

    if (objlist != NULL) {
        exprs = AppendObjdefsToArgExprs (exprs, SET_NEXT (objlist));

        DBUG_ASSERT ((OBJDEF_ARGAVIS (SET_MEMBER (objlist)) != NULL),
                     "found objdef required for fun-ap but without argarvis!");

        exprs = TBmakeExprs (TBmakeId (OBJDEF_ARGAVIS (SET_MEMBER (objlist))), exprs);
    }

    DBUG_RETURN (exprs);
}

/** <!-- ****************************************************************** -->
 * @brief Cleans up the objdefs in the list, so that the avis nodes are
 * cleared. This is so that they point to new avis nodes in the next
 * function body.
 *
 * @param list objdef list to clear
 *
 * @return cleared objdef list
 ******************************************************************************/
static node *
CleanUpObjlist (node *list)
{
    DBUG_ENTER ("CleanUpObjlist");

    if (list != NULL) {
        SET_NEXT (list) = CleanUpObjlist (SET_NEXT (list));

        OBJDEF_ARGAVIS (SET_MEMBER (list)) = NULL;
    }

    DBUG_RETURN (list);
}

/** <!-- ****************************************************************** -->
 * @brief Adds a F_prop_obj_in primitive function call to the body of a
 * with-loop. This is called for the first propagate() with-op encountered;
 * every subsequent call should use ModPropObj() instead. The call looks like:
 *
 *    object = F_prop_obj_in(iv, object);
 *
 * @param assign Start of the with-loop body assignment chain.
 * @param prop Propagate node containing object to add in call.
 * @param arg_info Info struct.
 *
 * @return New start of the with-loop body assignment chain.
 ******************************************************************************/
static node *
AddPropObj (node *assign, node *prop, info *arg_info)
{
    node *avis;
    node *prop_obj_out;
    node *wl_out;

    DBUG_ENTER ("MakeAccu");

    /**** PROPOBJ_IN ****/

    /* create avis */
    avis = TBmakeAvis (ILIBtmpVarName (ID_NAME (PROPAGATE_DEFAULT (prop))),
                       TYeliminateAKV (AVIS_TYPE (ID_AVIS (PROPAGATE_DEFAULT (prop)))));

    /* insert vardec */
    BLOCK_VARDEC (FUNDEF_BODY (INFO_FUNDEF (arg_info)))
      = TBmakeVardec (avis, BLOCK_VARDEC (FUNDEF_BODY (INFO_FUNDEF (arg_info))));

    /* create <avis> = F_prop_obj_in( <idx-varname>, <object>) */
    assign
      = TBmakeAssign (TBmakeLet (TBmakeIds (ID_AVIS (PROPAGATE_DEFAULT (prop)), NULL),
                                 TCmakePrf2 (F_prop_obj_in,
                                             DUPdupIdsId (WITH_VEC (INFO_WL (arg_info))),
                                             TBmakeId (
                                               ID_AVIS (PROPAGATE_DEFAULT (prop))))),
                      assign);

    /* set correct backref to defining assignment */
    AVIS_SSAASSIGN (avis) = assign;
    INFO_PROPOBJ_IN (arg_info) = assign;

    /**** PROPOBJ_OUT ****/

    /* find goal expression of the propagate */
    wl_out = FindPropagateGoalExpr (prop, arg_info);

    /* create avis */
    avis = TBmakeAvis (ILIBtmpVarName (ID_NAME (EXPRS_EXPR (wl_out))),
                       TYeliminateAKV (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (wl_out)))));

    /* insert vardec */
    BLOCK_VARDEC (FUNDEF_BODY (INFO_FUNDEF (arg_info)))
      = TBmakeVardec (avis, BLOCK_VARDEC (FUNDEF_BODY (INFO_FUNDEF (arg_info))));

    /* create <avis> = F_prop_obj_out( <object>) */
    prop_obj_out
      = TBmakeAssign (TBmakeLet (TBmakeIds (ID_AVIS (EXPRS_EXPR (wl_out)), NULL),
                                 TCmakePrf1 (F_prop_obj_out,
                                             TBmakeId (ID_AVIS (EXPRS_EXPR (wl_out))))),
                      NULL);
    assign = TCappendAssign (assign, prop_obj_out);

    /* set correct backref to defining assignment */
    AVIS_SSAASSIGN (avis) = prop_obj_out;
    INFO_PROPOBJ_OUT (arg_info) = prop_obj_out;

    DBUG_RETURN (assign);
}

/** <!-- ****************************************************************** -->
 * @brief Modifies the current F_prop_obj call in this with-loops body
 * to handle an additional object. The object is passed in via the propagate
 * node.
 *
 * @param prop Propagate node containing object to append to call.
 * @param arg_info Info struct.
 ******************************************************************************/
static void
ModPropObj (node *prop, info *arg_info)
{
    node *avis;
    node *args;
    node *lhs;

    DBUG_ENTER ("ModPropObj");

    /**** PROPOBJ_IN ****/

    /* create avis */
    avis = TBmakeAvis (ILIBtmpVarName (ID_NAME (PROPAGATE_DEFAULT (prop))),
                       TYeliminateAKV (AVIS_TYPE (ID_AVIS (PROPAGATE_DEFAULT (prop)))));

    /* insert vardec */
    BLOCK_VARDEC (FUNDEF_BODY (INFO_FUNDEF (arg_info)))
      = TBmakeVardec (avis, BLOCK_VARDEC (FUNDEF_BODY (INFO_FUNDEF (arg_info))));

    /* adjust the original so that
     * <avis> = F_prop_obj( <idx-varname>, ..., <new-obj> ) */
    args = PRF_ARGS (LET_EXPR (ASSIGN_INSTR (INFO_PROPOBJ_IN (arg_info))));
    lhs = LET_IDS (ASSIGN_INSTR (INFO_PROPOBJ_IN (arg_info)));

    args
      = TCappendExprs (args,
                       TBmakeExprs (TBmakeId (ID_AVIS (PROPAGATE_DEFAULT (prop))), NULL));
    lhs = TCappendIds (lhs, TBmakeIds (ID_AVIS (PROPAGATE_DEFAULT (prop)), NULL));

    /**** PROPOBJ_OUT ****/

    /* create avis */
    avis = TBmakeAvis (ILIBtmpVarName (ID_NAME (PROPAGATE_DEFAULT (prop))),
                       TYeliminateAKV (AVIS_TYPE (ID_AVIS (PROPAGATE_DEFAULT (prop)))));

    /* insert vardec */
    BLOCK_VARDEC (FUNDEF_BODY (INFO_FUNDEF (arg_info)))
      = TBmakeVardec (avis, BLOCK_VARDEC (FUNDEF_BODY (INFO_FUNDEF (arg_info))));

    /* adjust the original so that
     * <avis> = F_prop_obj_out( ..., <new-obj> ) */
    args = PRF_ARGS (LET_EXPR (ASSIGN_INSTR (INFO_PROPOBJ_OUT (arg_info))));
    lhs = LET_IDS (ASSIGN_INSTR (INFO_PROPOBJ_OUT (arg_info)));

    args
      = TCappendExprs (args,
                       TBmakeExprs (TBmakeId (ID_AVIS (PROPAGATE_DEFAULT (prop))), NULL));
    lhs = TCappendIds (lhs, TBmakeIds (ID_AVIS (PROPAGATE_DEFAULT (prop)), NULL));
    DBUG_VOID_RETURN;
}

/** <!-- ****************************************************************** -->
 * @brief Appends the objects in the objects list to the result expressions
 * of the given with-loop expression list.
 *
 * @param withexprs With-loop result expressions to append to.
 * @param objects Objects to add.
 *
 * @return New with-loop result expressions.
 ******************************************************************************/
static node *
AddObjectsToWithExprs (node *withexprs, node *objects)
{
    node *exprs;
    node *newexprs;
    node *object;
    node *avis;

    DBUG_ENTER ("AddObjectsToWithExprs");

    exprs = withexprs;
    while (exprs != NULL && EXPRS_NEXT (exprs) != NULL) {
        exprs = EXPRS_NEXT (exprs);
    }

    object = objects;
    while (object != NULL) {
        avis = ID_AVIS (EXPRS_EXPR (object));
        newexprs = TBmakeExprs (TBmakeId (avis), NULL);
        if (exprs != NULL) {
            EXPRS_NEXT (exprs) = newexprs;
            exprs = EXPRS_NEXT (exprs);
        } else {
            withexprs = newexprs;
            exprs = newexprs;
        }
        object = EXPRS_NEXT (object);
    }

    DBUG_RETURN (withexprs);
}

/** <!-- ****************************************************************** -->
 * @brief Appends a propagate() with-op for every object in objects.
 *
 * @param withops Current list of with-ops.
 * @param objects Objects to add.
 *
 * @return New list of with-ops, with propagate() calls appended.
 ******************************************************************************/
static node *
AddObjectsToWithOps (node *withops, node *objects)
{
    node *withop;
    node *object;
    node *newop;

    DBUG_ENTER ("AddObjectsToWithOps");

    /* Fast-forward to the last withop */
    withop = withops;
    while (withop != NULL && WITHOP_NEXT (withop) != NULL) {
        withop = WITHOP_NEXT (withop);
    }

    /* Append a propagate() withop for every object */
    object = objects;
    while (object != NULL) {
        /* Use the original object as default element */
        newop = TBmakePropagate (DUPdoDupTree (EXPRS_EXPR (object)));

        if (withop != NULL) {
            L_WITHOP_NEXT (withop, newop);
            withop = WITHOP_NEXT (withop);
        } else {
            withops = newop;
            withop = newop;
        }
        object = EXPRS_NEXT (object);
    }

    DBUG_RETURN (withops);
}

/** <!-- ****************************************************************** -->
 * @brief Helper function, that adds the objects in objects to the with-loops
 * result expression list, and adds propagate with-ops.
 *
 * @param withnode With-loop node.
 * @param objects Objects to add.
 *
 * @return With-loop node.
 ******************************************************************************/
static node *
AddObjectsToWithLoop (node *withnode, node *objects)
{
    node *withexprs;

    DBUG_ENTER ("AddObjectsToWithLoop");

    /* Add the objects the the withloop's exprs */
    withexprs = CODE_CEXPRS (WITH_CODE (withnode));
    CODE_CEXPRS (WITH_CODE (withnode)) = AddObjectsToWithExprs (withexprs, objects);

    /* Add a extract withop for each object */
    WITH_WITHOP (withnode) = AddObjectsToWithOps (WITH_WITHOP (withnode), objects);

    DBUG_RETURN (withnode);
}

/** <!-- ****************************************************************** -->
 * @brief Adds the objects in objects to the left-hand side of the with-loop
 * let assign node.
 *
 * @param lhs_ids Current left-hand side.
 * @param objects Objects to add.
 *
 * @return New left-hand side.
 ******************************************************************************/
static node *
AddObjectsToLHS (node *lhs_ids, node *objects)
{
    node *ids;
    node *object;
    node *avis;

    DBUG_ENTER ("AddObjectsToLHS");

    /* Append to the last exprs to match the extract() ops */
    ids = lhs_ids;
    while (ids != NULL && IDS_NEXT (ids) != NULL) {
        ids = IDS_NEXT (ids);
    }

    object = objects;
    while (object != NULL) {
        avis = ID_AVIS (EXPRS_EXPR (object));
        if (ids != NULL) {
            IDS_NEXT (ids) = TBmakeIds (avis, NULL);
            ids = IDS_NEXT (ids);
        } else {
            ids = TBmakeIds (avis, NULL);
            lhs_ids = ids;
        }
        object = EXPRS_NEXT (object);
    }

    DBUG_RETURN (lhs_ids);
}

/** <!-- ****************************************************************** -->
 * @brief Adds one N_id node to a set of N_expr nodes, but only if there
 *        is no other N_id which has the same AVIS in the set.
 *
 * @param set    The set to add to.
 * @param new_id The N_id node to add.
 *
 * @return The resulting set.
 ******************************************************************************/
static node *
AddToObjectSet (node *set, node *new_id)
{
    node *iter;

    DBUG_ENTER ("AddToObjectSet");

    iter = set;
    while (iter != NULL) {
        if (ID_AVIS (EXPRS_EXPR (iter)) == ID_AVIS (new_id)) {
            break;
        }
        iter = EXPRS_NEXT (iter);
    }

    /* At the end of the list, so it was not found. Add it. */
    if (iter == NULL) {
        set = TBmakeExprs (TBmakeId (ID_AVIS (new_id)), set);
    }

    DBUG_RETURN (set);
}

/** <!-- ****************************************************************** -->
 * @brief Merges two sets of N_expr nodes, returning the union of the two
 *        sets. Both the original sets are freed.
 *
 * @param set_a First set.
 * @param set_b Second set.
 *
 * @return Union of set_a and set_b.
 ******************************************************************************/
static node *
MergeObjectSet (node *set_a, node *set_b)
{
    node *a_iter;

    DBUG_ENTER ("MergeObjectSet");

    /*
     * For every item in set a, we add it to set b.
     * We return set b as a result.
     */
    a_iter = set_a;
    while (a_iter != NULL) {
        set_b = AddToObjectSet (set_b, EXPRS_EXPR (a_iter));
        a_iter = FREEdoFreeNode (a_iter);
    }

    DBUG_RETURN (set_b);
}

/** <!-- ****************************************************************** -->
 * @brief Creates N_id's for the N_objdef nodes in fundef_objects and adds
 * them to the N_expr nodes in set using AddToObjectSet().
 *
 * @param set Object set to add to.
 * @param fundef_objects Objdef set to add.
 *
 * @return Resulting object set.
 ******************************************************************************/
static node *
AddFundefObjectsToObjectSet (node *set, node *fundef_objects)
{
    node *avis;
    ntype *type;

    DBUG_ENTER ("AddFundefObjectsToObjectSet");

    if (fundef_objects != NULL) {
        set = AddFundefObjectsToObjectSet (set, SET_NEXT (fundef_objects));

        avis = OBJDEF_ARGAVIS (SET_MEMBER (fundef_objects));
        type = AVIS_TYPE (avis);

        DBUG_PRINT ("RSO", (">>> adding unique object to with-loop:"
                            "%s",
                            AVIS_NAME (avis)));
        set = AddToObjectSet (set, TBmakeId (avis));
    }

    DBUG_RETURN (set);
}

/*
 * traversal functions
 */

node *
RSOmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSOmodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSOfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSOfundef");

    DBUG_PRINT ("RSO", ("processing fundef %s...", CTIitemName (arg_node)));

    FUNDEF_ARGS (arg_node)
      = AppendObjdefsToArgs (FUNDEF_ARGS (arg_node), FUNDEF_OBJECTS (arg_node));

    INFO_FUNDEF (arg_info) = arg_node;
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }
    INFO_FUNDEF (arg_info) = NULL;

    FUNDEF_OBJECTS (arg_node) = CleanUpObjlist (FUNDEF_OBJECTS (arg_node));
    if (INFO_OBJECTS (arg_info) != NULL) {
        INFO_OBJECTS (arg_info) = FREEdoFreeTree (INFO_OBJECTS (arg_info));
    }

    DBUG_PRINT ("RSO", ("leaving fundef %s...", CTIitemName (arg_node)));

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSOglobobj (node *arg_node, info *arg_info)
{
    node *new_node;
    node *avis;

    DBUG_ENTER ("RSOglobobj");

    DBUG_ASSERT ((OBJDEF_ARGAVIS (GLOBOBJ_OBJDEF (arg_node)) != NULL),
                 "found a globobj with no matching arg!");

    DBUG_PRINT ("RSO", (">>> replacing global object %s by local arg %s",
                        CTIitemName (GLOBOBJ_OBJDEF (arg_node)),
                        AVIS_NAME (OBJDEF_ARGAVIS (GLOBOBJ_OBJDEF (arg_node)))));

    avis = OBJDEF_ARGAVIS (GLOBOBJ_OBJDEF (arg_node));
    new_node = TBmakeId (OBJDEF_ARGAVIS (GLOBOBJ_OBJDEF (arg_node)));

    if (INFO_INWITHLOOP (arg_info)) {
        DBUG_PRINT ("RSO", (">>> adding unique object to with-loop:"
                            "%s",
                            AVIS_NAME (avis)));
        INFO_OBJECTS (arg_info) = AddToObjectSet (INFO_OBJECTS (arg_info), new_node);
    }

    arg_node = FREEdoFreeNode (arg_node);

    DBUG_RETURN (new_node);
}

node *
RSOap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSOap");

    DBUG_PRINT ("RSO",
                (">>> updating call to function %s", CTIitemName (AP_FUNDEF (arg_node))));

    AP_ARGS (arg_node) = AppendObjdefsToArgExprs (AP_ARGS (arg_node),
                                                  FUNDEF_OBJECTS (AP_FUNDEF (arg_node)));

    if (INFO_INWITHLOOP (arg_info) == TRUE) {
        /*
         * This is an object referenced from within a with-loop. Add it
         * to the list of objects that will be attached to the with-loop.
         * But of course only if it's not in the list already.
         */
        INFO_OBJECTS (arg_info)
          = AddFundefObjectsToObjectSet (INFO_OBJECTS (arg_info),
                                         FUNDEF_OBJECTS (AP_FUNDEF (arg_node)));
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
RSOlet (node *arg_node, info *arg_info)
{
    node *saved_objs;

    DBUG_ENTER ("RSOlet");

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    if (LET_EXPR (arg_node) != NULL) {

        /*
         * If we are entering a with-loop, save the object set
         * and empty it. Afterwards, merge our existing object
         * set with the one resulting from the with-loop.
         */
        saved_objs = NULL;
        if (NODE_TYPE (LET_EXPR (arg_node)) == N_with) {
            saved_objs = INFO_OBJECTS (arg_info);
            INFO_OBJECTS (arg_info) = NULL;
        }

        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

        if (NODE_TYPE (LET_EXPR (arg_node)) == N_with) {

            /*
             * This is a let with a with-loop RHS. Add the with-loop's objects,
             * which should now be in arg_info, to the LHS expressions.
             */
            if (INFO_OBJECTS (arg_info) != NULL) {
                LET_IDS (arg_node)
                  = AddObjectsToLHS (LET_IDS (arg_node), INFO_OBJECTS (arg_info));
            }

            /* Merge with the old object set. */
            INFO_OBJECTS (arg_info)
              = MergeObjectSet (INFO_OBJECTS (arg_info), saved_objs);
        }
    }

    DBUG_RETURN (arg_node);
}

node *
RSOpropagate (node *arg_node, info *arg_info)
{
    node *block;

    DBUG_ENTER ("RSOpropagate");

    block = CODE_CBLOCK (WITH_CODE (INFO_WL (arg_info)));
    if (INFO_PROPOBJ_IN (arg_info) == NULL) {
        BLOCK_INSTR (block) = AddPropObj (BLOCK_INSTR (block), arg_node, arg_info);
    } else {
        ModPropObj (arg_node, arg_info);
    }

    if (PROPAGATE_NEXT (arg_node) != NULL) {
        PROPAGATE_NEXT (arg_node) = TRAVdo (PROPAGATE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSOwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSOwith");

    if (WITH_PART (arg_node) != NULL) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    }

    if (WITH_CODE (arg_node) != NULL) {
        /* Traverse the with-loop's code body to find object references. */
        INFO_INWITHLOOP (arg_info) = TRUE;
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_INWITHLOOP (arg_info) = FALSE;

        /* Attach the referenced objects to the with-loop. */
        arg_node = AddObjectsToWithLoop (arg_node, INFO_OBJECTS (arg_info));
    }

    /* Traverse the withops, which will add F_prop_obj's for each propagate */
    INFO_WL (arg_info) = arg_node;
    if (WITH_WITHOP (arg_node) != NULL) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    }
    INFO_WL (arg_info) = NULL;
    INFO_PROPOBJ_IN (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

node *
RSOdoResolveObjects (node *syntax_tree)
{
    info *arg_info;
    DBUG_ENTER ("RSOdoResolveObjects");

    TRAVpush (TR_rso);

    arg_info = MakeInfo ();

    syntax_tree = TRAVdo (syntax_tree, arg_info);

    arg_info = FreeInfo (arg_info);

    TRAVpop (TR_rso);

    DBUG_RETURN (syntax_tree);
}
