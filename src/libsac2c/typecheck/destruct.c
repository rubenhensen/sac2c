/**
 * $Id$
 */

#include "destruct.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "DupTree.h"
#include "free.h"
#include "traverse.h"
#include "types.h"
#include "new_types.h"
#include "user_types.h"
#include "type_utils.h"
#include "shape.h"
#include "dbug.h"
#include "memory.h"
#include "str.h"
#include "hidestructs.h"

/**
 *
 * @file destruct.c
 *
 * Remove all traces of struct usage.
 *
 * Currently only explodes argument and return value lists.
 */

/**
 * INFO structure
 * - module: Pointer to the N_module.
 */
struct INFO {
    node *module;
};

/**
 * INFO macros
 */
#define INFO_MODULE(n) ((n)->module)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_MODULE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * Private functions
 */

/** <!--********************************************************************-->
 *
 * @fn node *ExplodeArg( node *arg, node *structelem)
 *
 *   @brief  Return given element list as a N_arg list (like ExplodeRet).
 *
 ******************************************************************************/
static node *
ExplodeArg (node *arg, node *selem)
{
    node *newarg;
    char *old_name;

    DBUG_ASSERT (arg != NULL, ("Trying to explode NULL struct"));
    /* TODO: This should use the existing traversal mechanism. */
    if (selem == NULL) {
        return ARG_NEXT (arg);
    }
    newarg = DUPdoDupNode (arg);
    DBUG_PRINT ("DES", ("New arg at %p has N_avis at %p. Old arg at %p has N_avis at %p.",
                        newarg, ARG_AVIS (newarg), arg, ARG_AVIS (arg)));
    ARG_NTYPE (newarg) = TYfreeType (ARG_NTYPE (newarg));
    ARG_NTYPE (newarg) = TYcopyType (TYPEDEF_NTYPE (STRUCTELEM_TYPEDEF (selem)));
    old_name = ARG_NAME (newarg);
    /* Old name: my_s_arg.
     * New names: _my_s_arg_e1, _my_s_arg_e2, ...
     */
    ARG_NAME (newarg)
      = STRcatn (4, "_", old_name, "_", AVIS_NAME (STRUCTELEM_AVIS (selem)));
    old_name = MEMfree (old_name);
    DBUG_PRINT ("DES", ("Created new N_arg: %s", ARG_NAME (newarg)));
    /* Recursion for the rest of the struct elements. */
    ARG_NEXT (newarg) = ExplodeArg (arg, STRUCTELEM_NEXT (selem));

    return newarg;
}

/** <!--********************************************************************-->
 *
 * @fn node *ExplodeExprs( node *exprs, node *structelem)
 *
 *   @brief  Return given element list as a N_exprs list (like ExplodeRet).
 *
 *   The N_exprs's EXPR son MUST be a N_id! (it MUST even be a struct)
 *
 ******************************************************************************/
static node *
ExplodeExprs (node *exprs, node *selem)
{
    node *newexprs;
    node *id;
    node *newid;
    char *old_name;

    DBUG_ASSERT (exprs != NULL, ("Trying to explode NULL struct"));
    id = EXPRS_EXPR (exprs);
    DBUG_ASSERT (NODE_TYPE (id) == N_id, ("Exploding non-N_id node as struct."));
    /* TODO: This should use the existing traversal mechanism. */
    if (selem == NULL) {
        return EXPRS_NEXT (exprs);
    }
    newexprs = DUPdoDupNode (exprs);
    newid = EXPRS_EXPR (newexprs);
    /* Careful now: N_id has an N_avis as an *attribute*, not as a *son*. This
     * means DUPdoDupNode does *not* copy the N_avis (eventhough it copies the
     * N_exprs/N_id son of the N_exprs), but links it instead (am I right?). So, a
     * new N_avis has to be created manually. */
    ID_AVIS (newid) = DUPdoDupNode (ID_AVIS (newid));
    ID_NTYPE (newid) = TYfreeType (ID_NTYPE (newid));
    ID_NTYPE (newid) = TYcopyType (TYPEDEF_NTYPE (STRUCTELEM_TYPEDEF (selem)));
    old_name = ID_NAME (newid);
    /* Old name: my_s_id.
     * New names: _my_s_id_e1, _my_s_id_e2, ...
     */
    ID_NAME (newid)
      = STRcatn (4, "_", old_name, "_", AVIS_NAME (STRUCTELEM_AVIS (selem)));
    old_name = MEMfree (old_name);
    EXPRS_EXPR (newexprs) = newid;
    DBUG_PRINT ("DES", ("Created new N_id: %s", ID_NAME (newid)));
    /* Recursion for the rest of the struct elements. */
    EXPRS_NEXT (newexprs) = ExplodeExprs (exprs, STRUCTELEM_NEXT (selem));

    return newexprs;
}

/** <!--********************************************************************-->
 *
 * @fn node *ExplodeRet( node *ret, node *structelem)
 *
 *   @brief  Return given element list as a N_ret list.
 *
 *   The last node in the new list has the same NEXT son as the given N_ret.
 *   Aside from the TYPE attribute (that is set on a per-N_ret base), all flags
 *   and attributes of the given N_ret are copied verbatim to every new N_ret.
 *
 *   If the N_structelem list is empty, the next N_ret node in the first
 *   argument's list is returned.
 *
 ******************************************************************************/
static node *
ExplodeRet (node *ret, node *selem)
{
    node *newret;

    DBUG_ASSERT (ret != NULL, ("Trying to explode NULL struct"));
    /* TODO: This should use the existing traversal mechanism. */
    if (selem == NULL) {
        return RET_NEXT (ret);
    }
    newret = DUPdoDupNode (ret);
    RET_TYPE (newret) = TYfreeType (RET_TYPE (newret));
    RET_TYPE (newret) = TYcopyType (TYPEDEF_NTYPE (STRUCTELEM_TYPEDEF (selem)));
    /* Recursion for the rest of the struct elements. */
    RET_NEXT (newret) = ExplodeRet (ret, STRUCTELEM_NEXT (selem));

    return newret;
}

/** <!--********************************************************************-->
 *
 * @fn node *BodyForConstructor( node *constructor)
 *
 *   @brief  Gives this constructor a body that immediately returns all args.
 *
 ******************************************************************************/
static node *
BodyForConstructor (node *constructor)
{
    node *arg;
    node *exprs;
    node *id;
    node *return_node;

    /* Create a return() statement (N_return). */
    return_node = TBmakeReturn (NULL);
    /* Copy the arg list to an exprs list. */
    /* This should probably be using the existing traversal mechanism instead of
     * looping. Shame shame.
     */
    arg = FUNDEF_ARGS (constructor);
    /* Create an N_id that is the same as this argument. */
    id = TBmakeId (DUPdoDupNode (ARG_AVIS (arg)));
    /* Append that N_id to the list of N_exprs for the return value. */
    exprs = TBmakeExprs (id, NULL);
    RETURN_EXPRS (return_node) = exprs;
    /* Repeat for every successing element. */
    while (ARG_NEXT (arg) != NULL) {
        arg = ARG_NEXT (arg);
        /* Create an N_id that is the same as this argument. */
        id = TBmakeId (DUPdoDupNode (ARG_AVIS (arg)));
        /* Append that N_id to the list of N_exprs for the return value. */
        EXPRS_NEXT (exprs) = TBmakeExprs (id, NULL);
        exprs = EXPRS_NEXT (exprs);
    }
    /* Create a function body (N_block) with one instruction (N_assign): that
     * return statement.
     */
    return TBmakeBlock (TBmakeAssign (return_node, NULL), NULL);
}

/**
 * Public functions.
 */

/** <!--********************************************************************-->
 *
 * @fn node *DESdoDeStruct( node *syntax_tree)
 *
 *   @brief  Explode all argument and return value lists.
 *
 ******************************************************************************/
node *
DESdoDeStruct (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("DESdoDeStruct");

    DBUG_PRINT ("DES", ("Starting struct removal."));

    info = MakeInfo ();

    TRAVpush (TR_des);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("DES", ("Done removing all structs."));

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *DESmodule( node *arg_node, info *arg_info)
 *
 *   @brief  Store a pointer to the N_module in the arg_info struct.
 *
 ******************************************************************************/
node *
DESmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DESmodule");

    INFO_MODULE (arg_info) = arg_node;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_MODULE (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DESfundef( node *arg_node, info *arg_info)
 *
 *   @brief  If this function is a struct creator (created by the HideStruct
 *   phase) give it body.
 *
 *   In this function, I do something horrible: I sometimes transform a function
 *   declaration (IsExternal, Body = NULL) to a function definition (not
 *   IsExternal, Body is an actual instruction chain) /in-place/. Then, in order
 *   to get this on the right fundef stack on the module level, I return the
 *   value of the next N_fundef to this one, thereby pulling this fundef out of
 *   the stack it is currently in, and then I prepend it to the module's fundef
 *   stack (which I get via arg_info).
 *
 *   I would love to create a copy and add that one to the module's fundef
 *   stack, instead, but at this point all N_aps have their Fundef attribute
 *   pointing towards this node (it seems). Therefore I can not just delete it.
 *
 ******************************************************************************/
node *
DESfundef (node *arg_node, info *arg_info)
{
    node *selem;
    node *next_fundef;

    DBUG_ENTER ("DESfundef");

    /* Handle all sons (including the arguments list, return list, the body and
     * even the next fundef) before this one. This ensures the args are properly
     * exploded before creating the body. */
    arg_node = TRAVcont (arg_node, arg_info);

    /* Constructor. */
    if (FUNDEF_ISSTRUCTCONSTR (arg_node) && !FUNDEF_ISWRAPPERFUN (arg_node)) {
        /* Create a single return() statement with a copy of the arg list. */
        DBUG_ASSERT (FUNDEF_BODY (arg_node) == NULL, ("Constructor already has a body."));
        DBUG_ASSERT (FUNDEF_ISEXTERN (arg_node), ("Non-extern constructor."));
        FUNDEF_BODY (arg_node) = BodyForConstructor (arg_node);
        FUNDEF_ISEXTERN (arg_node) = FALSE;
        DBUG_PRINT ("DES", ("Constructor %s now has body", FUNDEF_NAME (arg_node)));
        /* Now, since this changed from a function declaration to a definition, it
         * must be moved to the correct place in the N_module.
         */
        next_fundef = FUNDEF_NEXT (arg_node);
        FUNDEF_NEXT (arg_node) = MODULE_FUNS (INFO_MODULE (arg_info));
        MODULE_FUNS (INFO_MODULE (arg_info)) = arg_node;
        /* Pull this node out of the current stack. */
        DBUG_RETURN (next_fundef);
    }

    /* Getter. */
    else if (FUNDEF_STRUCTGETTER (arg_node) != NULL) {
        selem = FUNDEF_STRUCTGETTER (arg_node);
        /* TODO: Give body to the struct getter. */
        DBUG_RETURN (arg_node);
    }

    /* Setter. */
    else if (FUNDEF_STRUCTSETTER (arg_node) != NULL) {
        selem = FUNDEF_STRUCTSETTER (arg_node);
        /* TODO: Give body to the struct setter. */
        DBUG_RETURN (arg_node);
    }

    else {
        DBUG_RETURN (arg_node);
    }
}

/** <!--********************************************************************-->
 *
 * @fn node *DESarg( node *arg_node, info *arg_info)
 *
 *   @brief  If this argument value is a struct, expand it to its elements.
 *
 *   Do note that this means an empty struct will expand to zero return values.
 *   If this occurs, the next node in the N_ret list is returned by this
 *   function. If there is no other next argument, NULL is returned.
 *
 ******************************************************************************/
node *
DESarg (node *arg_node, info *arg_info)
{
    ntype *type;
    node *orig_td;
    node *old_arg;
    node *sd;
    char *typestr;

    DBUG_ENTER ("DESarg");

    sd = NULL;
    type = ARG_NTYPE (arg_node);
    if (TUisArrayOfUser (type)) {
        orig_td = UTgetTdef (TYgetUserType (TYgetScalar (type)));
        DBUG_ASSERT (orig_td != NULL, ("No typedef found for this user type"));
        sd = TYPEDEF_STRUCTDEF (orig_td);
        if (sd != NULL) {
            /* TODO: Why can't I put this funcall directly in DBUG_PRINT()? - hly */
            typestr = TYtype2String (type, FALSE, 0);
            DBUG_PRINT ("DES",
                        ("Exploding arg struct %s (`%s')", STRUCTDEF_NAME (sd), typestr));
            old_arg = arg_node;
            arg_node = ExplodeArg (old_arg, STRUCTDEF_STRUCTELEM (sd));
            /* TODO: Applications of this function can have an N_id node as an
             * argument. These nodes have N_avis attributes, obviously, but as a Link,
             * not as a Node attribute. So they link to this node's N_avis. Therefore,
             * the memory can not be freed here. How do I fix this?
             */
            /* old_arg = FREEdoFreeNode( old_arg); */
            DBUG_PRINT ("DES", ("Done exploding arg struct %s", STRUCTDEF_NAME (sd)));
        }
    }

    if (arg_node != NULL) {
        if (sd != NULL) {
            /* If this was a struct type argument originally, that node has been
             * removed and the traversal needs to continue on this arg_node.
             */
            arg_node = TRAVdo (arg_node, arg_info);
        } else {
            ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DESret( node *arg_node, info *arg_info)
 *
 *   @brief  If this return value is a struct, expand it to its elements.
 *
 *   Just like DESarg, actually.
 *
 ******************************************************************************/
node *
DESret (node *arg_node, info *arg_info)
{
    ntype *type;
    node *orig_td;
    node *old_ret;
    node *sd;
    char *typestr;

    DBUG_ENTER ("DESret");

    sd = NULL;
    type = RET_TYPE (arg_node);
    if (TUisArrayOfUser (type)) {
        orig_td = UTgetTdef (TYgetUserType (TYgetScalar (type)));
        DBUG_ASSERT (orig_td != NULL, ("No typedef found for this user type"));
        sd = TYPEDEF_STRUCTDEF (orig_td);
        if (sd != NULL) {
            /* TODO: Why can't I put this funcall directly in DBUG_PRINT()? - hly */
            typestr = TYtype2String (type, FALSE, 0);
            DBUG_PRINT ("DES",
                        ("Exploding ret struct %s (`%s')", STRUCTDEF_NAME (sd), typestr));
            old_ret = arg_node;
            arg_node = ExplodeRet (old_ret, STRUCTDEF_STRUCTELEM (sd));
            old_ret = FREEdoFreeNode (old_ret);
            DBUG_PRINT ("DES", ("Done exploding ret struct %s", STRUCTDEF_NAME (sd)));
        }
    }

    if (sd != NULL && arg_node != NULL) {
        /* If this was a struct type return value originally, that node has been
         * removed and the traversal needs to continue on this arg_node.
         */
        arg_node = TRAVdo (arg_node, arg_info);
    } else {
        RET_NEXT (arg_node) = TRAVopt (RET_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DESexprs( node *arg_node, info *arg_info)
 *
 *   @brief  If an expression list contains a N_id that is a struct, explode it.
 *
 ******************************************************************************/
node *
DESexprs (node *arg_node, info *arg_info)
{
    ntype *type;
    node *expr;
    node *orig_td;
    node *sd;
    node *old_exprs;
    char *typestr;

    DBUG_ENTER ("DESexprs");

    sd = NULL;
    expr = EXPRS_EXPR (arg_node);
    if (NODE_TYPE (expr) != N_id) {
        DBUG_RETURN (arg_node);
    }

    type = ID_NTYPE (expr);
    if (TUisArrayOfUser (type)) {
        orig_td = UTgetTdef (TYgetUserType (TYgetScalar (type)));
        DBUG_ASSERT (orig_td != NULL, ("No typedef found for this user type"));
        sd = TYPEDEF_STRUCTDEF (orig_td);
        if (sd != NULL) {
            typestr = TYtype2String (type, FALSE, 0);
            DBUG_PRINT ("DES", ("Exploding N_id %s (type %s).", ID_NAME (expr), typestr));
            old_exprs = arg_node;
            arg_node = ExplodeExprs (old_exprs, STRUCTDEF_STRUCTELEM (sd));
            DBUG_PRINT ("DES", ("Done exploding N_id of type %s.", typestr));
        }
    }

    if (sd != NULL && arg_node != NULL) {
        arg_node = TRAVdo (arg_node, arg_info);
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}
