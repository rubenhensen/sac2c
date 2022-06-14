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

#define DBUG_PREFIX "DES"
#include "debug.h"

#include "memory.h"
#include "str.h"
#include "hidestructs.h"

/**
 *
 * @file destruct.c
 *
 * Remove all traces of struct usage.
 *
 */

/**
 * INFO structure
 * - cleanup: Boolean indicating cleanup mode state.
 * - inlet: Boolean: the traversal is now in a child of a N_let.
 * - module: Pointer to the N_module.
 * - makeset: See struct INFO_makeset.
 * - args2exprs: Flag for DESarg to store a list of N_ids as N_exprs instead of
 *   doing expansion.
 * - retexprs: Pointer to said N_exprs from DESarg.
 * - nonrecursive: Prevent DESarg from doing recursive expansion.
 */
struct INFO {
    int cleanup;
    int inlet;
    int incondfun;
    int nonrecursive;
    int args2exprs;
    node *argexprs;
    node *module;
};

/**
 * INFO macros
 */
#define INFO_CLEANUP(n) ((n)->cleanup)
#define INFO_INLET(n) ((n)->inlet)
#define INFO_INCONDFUN(n) ((n)->incondfun)
#define INFO_NONRECURSIVE(n) ((n)->nonrecursive)
#define INFO_ARGS2EXPRS(n) ((n)->args2exprs)
#define INFO_ARGEXPRS(n) ((n)->argexprs)
#define INFO_MODULE(n) ((n)->module)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_CLEANUP (result) = 0;
    INFO_INLET (result) = 0;
    INFO_INCONDFUN (result) = 0;
    INFO_NONRECURSIVE (result) = 0;
    INFO_ARGS2EXPRS (result) = 0;
    INFO_ARGEXPRS (result) = NULL;
    INFO_MODULE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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

    DBUG_ASSERT (arg != NULL, "Trying to explode NULL struct");
    /* TODO: This should use the existing traversal mechanism. */
    if (selem == NULL) {
        return ARG_NEXT (arg);
    }
    newarg = DUPdoDupNode (arg);
    ARG_NTYPE (newarg) = TYfreeType (ARG_NTYPE (newarg));
    ARG_NTYPE (newarg) = TYcopyType (TYPEDEF_NTYPE (STRUCTELEM_TYPEDEF (selem)));
    old_name = ARG_NAME (newarg);
    /* Old name: my_s_arg.
     * New names: _my_s_arg_e1, _my_s_arg_e2, ...
     */
    ARG_NAME (newarg) = STRcatn (4, "_", old_name, "_", STRUCTELEM_NAME (selem));
    old_name = MEMfree (old_name);
    DBUG_PRINT ("Created new N_arg: %s", ARG_NAME (newarg));
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

    DBUG_ASSERT (exprs != NULL, "Trying to explode NULL struct");
    id = EXPRS_EXPR (exprs);
    DBUG_ASSERT (NODE_TYPE (id) == N_id, "Exploding non-N_id node as struct.");
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
    ID_NAME (newid) = STRcatn (4, "_", old_name, "_", STRUCTELEM_NAME (selem));
    old_name = MEMfree (old_name);
    EXPRS_EXPR (newexprs) = newid;
    DBUG_PRINT ("Created new N_id: %s", ID_NAME (newid));
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

    DBUG_ASSERT (ret != NULL, "Trying to explode NULL struct");
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
 * @fn node *ExplodeIds( node *ids, node *selem)
 *
 *   @brief  Return given element list as a N_ids list (like ExplodeRet).
 *
 ******************************************************************************/
static node *
ExplodeIds (node *ids, node *selem)
{
    node *newids;
    char *old_name;

    DBUG_ASSERT (ids != NULL, "Trying to explode NULL struct");
    /* TODO: This should use the existing traversal mechanism. */
    if (selem == NULL) {
        return IDS_NEXT (ids);
    }
    newids = DUPdoDupNode (ids);
    /* DUPdoDupNode does not copy the N_avis because it is an attribute. Create a
     * new one for this fresh N_ids. */
    IDS_AVIS (newids) = DUPdoDupNode (IDS_AVIS (ids));
    IDS_NTYPE (newids) = TYfreeType (IDS_NTYPE (newids));
    IDS_NTYPE (newids) = TYcopyType (TYPEDEF_NTYPE (STRUCTELEM_TYPEDEF (selem)));
    old_name = IDS_NAME (newids);
    /* Old name: my_s_id.
     * New names: _my_s_id_e1, _my_s_id_e2, ...
     */
    IDS_NAME (newids) = STRcatn (4, "_", old_name, "_", STRUCTELEM_NAME (selem));
    old_name = MEMfree (old_name);
    /* Recursion for the rest of the struct elements. */
    IDS_NEXT (newids) = ExplodeIds (ids, STRUCTELEM_NEXT (selem));

    return newids;
}

/** <!--********************************************************************-->
 *
 * @fn node *IDstruct2elem( node *id, node *selem)
 *
 *   @brief  Substitute given struct var by a var for given struct element.
 *
 ******************************************************************************/
static node *
IDstruct2elem (node *id, node *selem)
{
    node *newid;
    char *old_name;

    DBUG_ASSERT (NODE_TYPE (id) == N_id && NODE_TYPE (selem) == N_structelem,
                 "Illegal arguments to IDstruct2elem");
    DBUG_ASSERT (TYPEDEF_STRUCTDEF (
                   UTgetTdef (TYgetUserType (TYgetScalar (ID_NTYPE (id)))))
                   != NULL,
                 "Non-struct var to IDstruct2elem");
    newid = DUPdoDupNode (id);
    ID_AVIS (newid) = DUPdoDupNode (ID_AVIS (newid));
    ID_NTYPE (newid) = TYfreeType (ID_NTYPE (newid));
    ID_NTYPE (newid) = TYcopyType (TYPEDEF_NTYPE (STRUCTELEM_TYPEDEF (selem)));
    old_name = ID_NAME (newid);
    ID_NAME (newid) = STRcatn (4, "_", old_name, "_", STRUCTELEM_NAME (selem));
    old_name = MEMfree (old_name);
    id = FREEdoFreeNode (id);

    return (newid);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateFCAssignChain( node *assign, node *selem)
 *
 *   @brief  Create a new N_assign chain of N_funconds with the [N_id]s that
 *           reference struct vars exploded to the elements of given struct.
 *
 ******************************************************************************/
static node *
CreateFCAssignChain (node *assign, node *selem)
{
    node *let;
    node *newass;
    node *newids;
    node *newfc;
    char *old_name;

    let = ASSIGN_STMT (assign);
    DBUG_ASSERT (NODE_TYPE (let) == N_let && NODE_TYPE (LET_EXPR (let)) == N_funcond,
                 "CreateFCAssignChain called with illegal first argument.");
    if (selem == NULL) {
        return ASSIGN_NEXT (assign);
    }

    newass = DUPdoDupNode (assign);
    /* Change the left-hand side. */
    newids = LET_IDS (ASSIGN_STMT (newass));
    /* DUPdoDupNode does not copy the N_avis because it is an attribute. Create a
     * new one for this fresh N_ids. */
    IDS_AVIS (newids) = DUPdoDupNode (IDS_AVIS (newids));
    IDS_NTYPE (newids) = TYfreeType (IDS_NTYPE (newids));
    IDS_NTYPE (newids) = TYcopyType (TYPEDEF_NTYPE (STRUCTELEM_TYPEDEF (selem)));
    old_name = IDS_NAME (newids);
    IDS_NAME (newids) = STRcatn (4, "_", old_name, "_", STRUCTELEM_NAME (selem));
    old_name = MEMfree (old_name);
    /* Change the right-hand side. */
    newfc = LET_EXPR (ASSIGN_STMT (newass));
    FUNCOND_THEN (newfc) = IDstruct2elem (FUNCOND_THEN (newfc), selem);
    FUNCOND_ELSE (newfc) = IDstruct2elem (FUNCOND_ELSE (newfc), selem);
    DBUG_PRINT ("Created new funcond for %s", IDS_NAME (newids));

    ASSIGN_NEXT (newass) = CreateFCAssignChain (assign, STRUCTELEM_NEXT (selem));

    return newass;
}

/** <!--********************************************************************-->
 *
 * @fn node *Fundecl2Fundef( node *fundec, node *body, node *module)
 *
 *   @brief  Give body to this function declaration (make it a fundef).
 *
 *   Also moves the node from the module's fundecl stack to the fundef stack and
 *   marks it as `inline'.
 *
 ******************************************************************************/
static node *
Fundecl2Fundef (node *fundec, node *body, node *module)
{
    FUNDEF_BODY (fundec) = body;
    FUNDEF_ISEXTERN (fundec) = FALSE;
    FUNDEF_ISINLINE (fundec) = TRUE;
    /* The copy constructors used to be sticky but now that their use has been
     * determined the unused ones can safely be removed.
     */
    FUNDEF_ISSTICKY (fundec) = FALSE;
    /* Now, since this changed from a function declaration to a definition, it
     * must be moved to the correct place in the N_module.
     */
    FUNDEF_NEXT (fundec) = MODULE_FUNS (module);
    MODULE_FUNS (module) = fundec;
    return fundec;
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeConstructor( node *constructor, info *arg_info)
 *
 *   @brief  Gives this constructor a body that immediately returns all args.
 *
 * Returns the new constructor.
 *
 ******************************************************************************/
static node *
MakeConstructor (node *constructor, info *arg_info)
{
    node *body;

    DBUG_PRINT ("Giving constructor %s a body...", FUNDEF_NAME (constructor));
    /* Copy the arg list to an exprs list. */
    DBUG_ASSERT ((INFO_ARGS2EXPRS (arg_info) == FALSE
                  && INFO_ARGEXPRS (arg_info) == NULL),
                 "Garbage traversal data encountered.");
    INFO_ARGS2EXPRS (arg_info) = TRUE;
    FUNDEF_ARGS (constructor) = TRAVopt (FUNDEF_ARGS (constructor), arg_info);
    INFO_ARGS2EXPRS (arg_info) = FALSE;
    /* Create a function body (N_block) with one instruction (N_assign): the
     * return statement.
     */
    body
      = TBmakeBlock (TBmakeAssign (TBmakeReturn (INFO_ARGEXPRS (arg_info)), NULL), NULL);
    INFO_ARGEXPRS (arg_info) = NULL;
    return Fundecl2Fundef (constructor, body, INFO_MODULE (arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeSetter( node *setter, info *arg_info)
 *
 *   @brief  Gives this setter an appropriate body.
 *
 * Returns the new setter.
 *
 ******************************************************************************/
static node *
MakeSetter (node *setter, info *arg_info)
{
    node *body;
    /* node * structdef; */
    node *e;

    DBUG_PRINT ("Giving setter %s a body...", FUNDEF_NAME (setter));
#if 0
  /* Get the struct definition of the entire struct this getter is for. */
  structdef = TYPEDEF_STRUCTDEF( UTgetTdef( TYgetUserType( TYgetScalar(
            ARG_NTYPE( ARG_NEXT( FUNDEF_ARGS( setter)))))));
#endif
    /* The first argument is the new value (`e'), the second is the struct
     * (`s').
     */
    e = FUNDEF_ARGS (setter);
    /* TODO: INFO_MAKESET_NEWAVIS( arg_info) = ARG_AVIS( e); */
    INFO_ARGS2EXPRS (arg_info) = TRUE;
    ARG_NEXT (e) = TRAVdo (ARG_NEXT (e), arg_info);
    INFO_ARGS2EXPRS (arg_info) = FALSE;
    body
      = TBmakeBlock (TBmakeAssign (TBmakeReturn (INFO_ARGEXPRS (arg_info)), NULL), NULL);
    INFO_ARGEXPRS (arg_info) = NULL;
    return Fundecl2Fundef (setter, body, INFO_MODULE (arg_info));
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

    DBUG_ENTER ();

    DBUG_PRINT ("Starting struct removal.");

    info = MakeInfo ();

    TRAVpush (TR_des);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("Done removing all structs.");

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
    DBUG_ENTER ();

    INFO_MODULE (arg_info) = arg_node;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_CLEANUP (arg_info) = TRUE;
    MODULE_STRUCTS (arg_node) = TRAVopt (MODULE_STRUCTS (arg_node), arg_info);
    MODULE_TYPES (arg_node) = TRAVopt (MODULE_TYPES (arg_node), arg_info);
    INFO_CLEANUP (arg_info) = FALSE;

    INFO_MODULE (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DEStypedef( node *arg_node, info *arg_info)
 *
 *   @brief  At the end of this phase, delete the artificial typedefs.
 *
 *
 ******************************************************************************/
node *
DEStypedef (node *arg_node, info *arg_info)
{
    node *next;

    DBUG_ENTER ();

    if (INFO_CLEANUP (arg_info)) {
        if (TYPEDEF_STRUCTDEF (arg_node) != NULL) {
            DBUG_PRINT ("Cleaning up typedef %s.", TYPEDEF_NAME (arg_node));
            next = FREEdoFreeNode (arg_node);
            arg_node = TRAVopt (next, arg_info);
        }
    } else {
        /* Not in cleanup but in normal (first) traversal. */
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DESstructdef( node *arg_node, info *arg_info)
 *
 *   @brief  At the end of this phase, delete the struct definitions.
 *
 *
 ******************************************************************************/
node *
DESstructdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_CLEANUP (arg_info)) {
        DBUG_PRINT ("Cleaning up structdefs.");
        /* This operation frees the entire list right away. */
        arg_node = FREEdoFreeTree (arg_node);
        DBUG_ASSERT (arg_node == NULL, "Structdefs not properly freed.");
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DESfundef( node *arg_node, info *arg_info)
 *
 *   @brief  If this function is a struct creator, getter or setter (created by
 *   the HideStruct phase) give it body.
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
    node *avis;
    node *body;
    node *selem;
    node *next_fundecl;
    node *next_fundef;
    char *ret_name;

    DBUG_ENTER ();

    /* No matter what kind of function this is, it will want at least a
     * non-recursive expansion of the argument list.
     */
    INFO_NONRECURSIVE (arg_info) = TRUE;
    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
    INFO_NONRECURSIVE (arg_info) = FALSE;

    /* Getter. */
    if (FUNDEF_STRUCTGETTER (arg_node) != NULL && !FUNDEF_ISWRAPPERFUN (arg_node)) {
        if (FUNDEF_BODY (arg_node) != NULL) {
            /* This is a re-traversal of a ex-fundec getter, which is now a fundef.
             * The sons of this node have already been traversed, so they can be
             * skipped.
             */
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
            DBUG_RETURN (arg_node);
        }
        selem = FUNDEF_STRUCTGETTER (arg_node);
        /* TODO: The argument's name is known: `s'. */
        ret_name = STRcatn (4, "_", ARG_NAME (FUNDEF_ARGS (arg_node)), "_",
                            STRUCTELEM_NAME (selem));
        avis = TBmakeAvis (ret_name, TYcopyType (STRUCTELEM_TYPE (selem)));
        body
          = TBmakeBlock (TBmakeAssign (TBmakeReturn (TBmakeExprs (TBmakeId (avis), NULL)),
                                       NULL),
                         NULL);
        /* Save a pointer to the next function declaration. */
        next_fundecl = FUNDEF_NEXT (arg_node);
        arg_node = Fundecl2Fundef (arg_node, body, INFO_MODULE (arg_info));
        DBUG_PRINT ("Getter %s now has body", FUNDEF_NAME (arg_node));
        /* Pull the current arg_node (ex-fundec, now getter fundef with body) out of
         * the stack it is in (= fundec stack) and continue traversal on the next
         * node (which is the next fundec or NULL). First, though, the traversal
         * must continue on the child-nodes of this fundef. TRAVcont also operates
         * on the NEXT attribute, which is undesirable. (note: fundef != fundecl)
         */
        next_fundef = FUNDEF_NEXT (arg_node);
        FUNDEF_NEXT (arg_node) = NULL;
        arg_node = TRAVcont (arg_node, arg_info);
        FUNDEF_NEXT (arg_node) = next_fundef;
        DBUG_RETURN (TRAVopt (next_fundecl, arg_info));
    }

    /* Setter. */
    else if (FUNDEF_STRUCTSETTER (arg_node) != NULL) {
        next_fundecl = FUNDEF_NEXT (arg_node);
        arg_node = MakeSetter (arg_node, arg_info);
        next_fundef = FUNDEF_NEXT (arg_node);
        FUNDEF_NEXT (arg_node) = NULL;
        arg_node = TRAVcont (arg_node, arg_info);
        FUNDEF_NEXT (arg_node) = next_fundef;
        DBUG_RETURN (TRAVopt (next_fundecl, arg_info));
    }

    /* Lifted conditional. */
    if (FUNDEF_ISCONDFUN (arg_node)) {
        INFO_INCONDFUN (arg_info) = TRUE;
    }

    /* Handle all sons (including the arguments list, return list, the body and
     * even the next fundef) before this one. This ensures the args are properly
     * exploded before creating the body. */
    arg_node = TRAVcont (arg_node, arg_info);

    /* Reset lifted conditional state. */
    INFO_INCONDFUN (arg_info) = FALSE;

    /* Constructor. */
    if (FUNDEF_ISSTRUCTCONSTR (arg_node) && !FUNDEF_ISWRAPPERFUN (arg_node)) {
        DBUG_ASSERT (FUNDEF_BODY (arg_node) == NULL, "Constructor already has a body.");
        DBUG_ASSERT (FUNDEF_ISEXTERN (arg_node), "Non-extern constructor.");
        next_fundecl = FUNDEF_NEXT (arg_node);
        arg_node = MakeConstructor (arg_node, arg_info);
        DBUG_PRINT ("Constructor %s now has body", FUNDEF_NAME (arg_node));
        /* Traversal has already done the next node (this part is bottom-up), so no
         * need to repeat traversal on the next node. */
        arg_node = next_fundecl;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DESarg( node *arg_node, info *arg_info)
 *
 *   @brief  If this argument value is a struct, expand it to its elements.
 *
 *   Do note that this means an empty struct will expand to zero arguments.  If
 *   this occurs, the next node in the N_arg list is returned by this function.
 *   If there is no other next argument, NULL is returned.
 *
 *   Does not operate recursively if the INFO_NONRECURSIVE flag is set.
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

    DBUG_ENTER ();

    /* Special task of DESarg: copying arg list to exprs list. */
    if (INFO_ARGS2EXPRS (arg_info) == TRUE) {
        arg_node = TRAVcont (arg_node, arg_info);
        INFO_ARGEXPRS (arg_info)
          = TBmakeExprs (TBmakeId (DUPdoDupNode (ARG_AVIS (arg_node))),
                         INFO_ARGEXPRS (arg_info));
        DBUG_RETURN (arg_node);
    }

    sd = NULL;
    type = ARG_NTYPE (arg_node);
    if (TUisArrayOfUser (type)) {
        orig_td = UTgetTdef (TYgetUserType (TYgetScalar (type)));
        DBUG_ASSERT (orig_td != NULL, "No typedef found for this user type");
        sd = TYPEDEF_STRUCTDEF (orig_td);
        if (sd != NULL) {
            /* TODO: Why can't I put this funcall directly in DBUG_PRINT()? - hly */
            typestr = TYtype2String (type, FALSE, 0);
            DBUG_PRINT ("Exploding arg struct %s (`%s')", STRUCTDEF_NAME (sd), typestr);
            old_arg = arg_node;
            arg_node = ExplodeArg (old_arg, STRUCTDEF_STRUCTELEM (sd));
            /* TODO: Applications of this function can have an N_id node as an
             * argument. These nodes have N_avis attributes, obviously, but as a Link,
             * not as a Node attribute. So they link to this node's N_avis. Therefore,
             * the memory can not be freed here. How do I fix this?
             */
            /* old_arg = FREEdoFreeNode( old_arg); */
            DBUG_PRINT ("Done exploding arg struct %s", STRUCTDEF_NAME (sd));
        }
    }

    if (arg_node != NULL) {
        if (sd != NULL && !INFO_NONRECURSIVE (arg_info)) {
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

    DBUG_ENTER ();

    sd = NULL;
    type = RET_TYPE (arg_node);
    if (TUisArrayOfUser (type)) {
        orig_td = UTgetTdef (TYgetUserType (TYgetScalar (type)));
        DBUG_ASSERT (orig_td != NULL, "No typedef found for this user type");
        sd = TYPEDEF_STRUCTDEF (orig_td);
        if (sd != NULL) {
            /* TODO: Why can't I put this funcall directly in DBUG_PRINT()? - hly */
            typestr = TYtype2String (type, FALSE, 0);
            DBUG_PRINT ("Exploding ret struct %s (`%s')", STRUCTDEF_NAME (sd), typestr);
            old_ret = arg_node;
            arg_node = ExplodeRet (old_ret, STRUCTDEF_STRUCTELEM (sd));
            old_ret = FREEdoFreeNode (old_ret);
            DBUG_PRINT ("Done exploding ret struct %s", STRUCTDEF_NAME (sd));
        }
    }

    if (sd != NULL && arg_node != NULL) {
        /* If this was a struct type return value originally, that node has been
         * removed and the traversal needs to continue on this arg_node.
         */
        arg_node = TRAVdo (arg_node, arg_info);
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *desexprs( node *arg_node, info *arg_info)
 *
 *   @brief  if an expression list contains a n_id that is a struct, explode it.
 *
 ******************************************************************************/
node *
DESassign (node *arg_node, info *arg_info)
{
    ntype *type;
    node *instr;
    node *expr;
    node *newassign;
    node *orig_td;
    node *sd;
    node *ids;

    DBUG_ENTER ();
    DBUG_PRINT ("Entering DESassign.");

    /* Only deal with possible N_funcond if it's in a lifted conditional function.
     */
    if (!INFO_INCONDFUN (arg_info)) {
        arg_node = TRAVcont (arg_node, arg_info);
        DBUG_PRINT ("Shortcutting DESassign.");
        DBUG_RETURN (arg_node);
    }

    DBUG_ASSERT (arg_node != NULL, "Empty N_assign in N_funcond.");
    newassign = NULL;
    instr = ASSIGN_STMT (arg_node);

    if (instr == NULL) {
        DBUG_PRINT ("N_assign with empty instruction.");
    } else if (NODE_TYPE (instr) == N_let && LET_IDS (instr) != NULL) {
        /* In some cases, N_let nodes of void functions can still occur here. This
         * means that there is no IDS node being assigned to, so those situations
         * can be skipped.
         */
        expr = LET_EXPR (instr);
        ids = LET_IDS (instr);
        type = IDS_NTYPE (ids);
        if (NODE_TYPE (expr) == N_funcond && TUisArrayOfUser (type)) {
            orig_td = UTgetTdef (TYgetUserType (TYgetScalar (type)));
            DBUG_ASSERT (orig_td != NULL, "No typedef found for this user type");
            sd = TYPEDEF_STRUCTDEF (orig_td);
            if (sd != NULL) {
                DBUG_PRINT ("CondFun for a struct var: %s", IDS_NAME (LET_IDS (instr)));
                /* Expand this assignment to a struct var into multiple assignments for
                 * each of the elements. */
                newassign = CreateFCAssignChain (arg_node, STRUCTDEF_STRUCTELEM (sd));
            }
        }
    }

    if (newassign != NULL) {
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = TRAVdo (newassign, arg_info);
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_PRINT ("Leaving DESassign.");
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *desexprs( node *arg_node, info *arg_info)
 *
 *   @brief  if an expression list contains a n_id that is a struct, explode it.
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

    DBUG_ENTER ();

    sd = NULL;
    expr = EXPRS_EXPR (arg_node);
    if (NODE_TYPE (expr) != N_id) {
        arg_node = TRAVcont (arg_node, arg_info);
        DBUG_RETURN (arg_node);
    }

    type = ID_NTYPE (expr);
    if (TUisArrayOfUser (type)) {
        orig_td = UTgetTdef (TYgetUserType (TYgetScalar (type)));
        DBUG_ASSERT (orig_td != NULL, "No typedef found for this user type");
        sd = TYPEDEF_STRUCTDEF (orig_td);
        if (sd != NULL) {
            typestr = TYtype2String (type, FALSE, 0);
            DBUG_PRINT ("Exploding N_id %s (type %s).", ID_NAME (expr), typestr);
            old_exprs = arg_node;
            arg_node = ExplodeExprs (old_exprs, STRUCTDEF_STRUCTELEM (sd));
            DBUG_PRINT ("Done exploding N_id of type %s.", typestr);
            /* TODO: Is freeing old_exprs safe, here? */
        }
    }

    if (sd != NULL && arg_node != NULL) {
        arg_node = TRAVdo (arg_node, arg_info);
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DESlet( node *arg_node, info *arg_info)
 *
 *   @brief  If a value is assigned to a struct type, it must be exploded.
 *
 *   SAC does not allow multiple bindings at once (a, b = c, d) but it does
 *   allow multiple return values (a, b = f()). If the right-hand side is a
 *   struct (a = b) it will create an illegal construct (_a_e1, _a_e2 = b). To
 *   fix that, the right-hand side can be wrapped in a function that immediately
 *   returns its sole argument (the 'copy constructor') (_a_e1, _a_e2 = s(b)),
 *   which will then, by DESarg, by expanded to seperate arguments (_a_e1, _a_e2
 *   = s(_b_e1, _b_e2)).
 *
 *   This function does two things:
 *   - Make sure all struct-N_ids on the left side are expanded.
 *   - Wrap right-hand side struct N_id in a copy constructor.
 *
 ******************************************************************************/
node *
DESlet (node *arg_node, info *arg_info)
{
    ntype *type;
    node *ap;
    node *id;
    node *sd;
    node *orig_td;

    DBUG_ENTER ();

    sd = NULL;
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_id) {
        id = LET_EXPR (arg_node);
        type = ID_NTYPE (id);
        if (TUisArrayOfUser (type)) {
            orig_td = UTgetTdef (TYgetUserType (TYgetScalar (type)));
            DBUG_ASSERT (orig_td != NULL, "No typedef found for this user type");
            sd = TYPEDEF_STRUCTDEF (orig_td);
            if (sd != NULL) {
                /* Replace right-hand side by a function call. */
                ap = TBmakeAp (STRUCTDEF_COPYCONSTRUCTOR (sd), TBmakeExprs (id, NULL));
                LET_EXPR (arg_node) = ap;
            }
        }
    }

    DBUG_ASSERT (!INFO_INLET (arg_info), "Nested N_let with only N_id");
    INFO_INLET (arg_info) = TRUE;
    /* Explode the left-hand side identifier if it's a struct. */
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
    INFO_INLET (arg_info) = FALSE;
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DESids( node *arg_node, info *arg_info)
 *
 *   @brief  If this N_ids occurs in the left hand of a N_let and is a struct
 *   type, explode it.
 *
 *   TODO: What if it is a struct type, but not in a N_let?
 *
 ******************************************************************************/
node *
DESids (node *arg_node, info *arg_info)
{
    ntype *type;
    node *orig_td;
    node *sd;

    DBUG_ENTER ();

    type = IDS_NTYPE (arg_node);
    sd = NULL;
    if (TUisArrayOfUser (type)) {
        orig_td = UTgetTdef (TYgetUserType (TYgetScalar (type)));
        DBUG_ASSERT (orig_td != NULL, "No typedef found for this user type");
        sd = TYPEDEF_STRUCTDEF (orig_td);
        if (sd != NULL && INFO_INLET (arg_info)) {
            DBUG_PRINT ("Exploding N_let's ids %s", STRUCTDEF_NAME (sd));
            arg_node = ExplodeIds (arg_node, STRUCTDEF_STRUCTELEM (sd));
            DBUG_PRINT ("Done exploding N_let's ids %s.", STRUCTDEF_NAME (sd));
            /* TODO: Check if a memfree of the old ids is safe here. */
        }
    }

    if (arg_node != NULL && sd != NULL && INFO_INLET (arg_info)) {
        arg_node = TRAVdo (arg_node, arg_info);
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DESvardec( node *arg_node, info *arg_info)
 *
 *   @brief  If this is a declaration of a record-type variable it can safely be
 *   removed, because the rest of this phase makes sure that those variables are
 *   never used anywhere.
 *
 ******************************************************************************/
node *
DESvardec (node *arg_node, info *arg_info)
{
    ntype *type;
    node *orig_td;
    node *sd;

    DBUG_ENTER ();

    /* Bottom-up. */
    arg_node = TRAVcont (arg_node, arg_info);

    type = VARDEC_NTYPE (arg_node);
    sd = NULL;
    if (TUisArrayOfUser (type)) {
        orig_td = UTgetTdef (TYgetUserType (TYgetScalar (type)));
        DBUG_ASSERT (orig_td != NULL, "No typedef found for this user type");
        sd = TYPEDEF_STRUCTDEF (orig_td);
        if (sd != NULL) {
            DBUG_PRINT ("Removing struct vardec %s", VARDEC_NAME (arg_node));
            // oldvd = arg_node;
            arg_node = VARDEC_NEXT (arg_node);
            /* TODO: Check if it is safe to free this node (hint: it's not). */
            /* FREEdoFreeNode( oldvd); */
        }
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
