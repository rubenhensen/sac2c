/**
 *
 * MOI handles the special case of object initialisers.
 *
 * global objects need to be initialised somewhere. To do so,
 * we need to have some code that knows about the global object
 * and that performs the necessary allocations, if needed. To 
 * reuse existing code generation, in SaC, we generate a special
 * INIT function which expects the global object as a reference
 * parameter. Inside that function, we generate the actual 
 * object including potentially needed memory allocations and
 * we assign it to the global variable by "returning" it 
 * through the reference parameter.
 * While this approach ensures that the usual reference-parameter 
 * mechanism sorts almost everything, there is one little catch:
 * the "incoming" object, in contrast to the normal reference
 * parameter case, does not yet exist / is not yet allocated or
 * initialised. Therefore, we must make sure that these artificial
 * parameters are not being used. THIS is the purpose of this
 * traversal. It traverses through the object init functions and
 * blindly eliminates all assignments whose RHS refers to the
 * incoming reference parameter. The only special case are
 * occurances of alloc_or_reuse which are replaced by _alloc_.
 *
 * Example:
 *
 * class X;
 * classtype int[10]
 * objdef myX = createX();
 *
 * Before this phase, we have a function:
 *
 * X:_INIT::init_myX (int[10] & _OI_object)   which has the code of 
 *                                            createX() inlined
 *
 * Because the compiler "thinks" that this is a normal function,
 * it will try to "consume" its argument, either through DEC_RC
 * or through _alloc_or_reuse. Given that we plan to call this function
 * with an uninitialisd global variable, we have to elide the DEC_RC 
 * and we have to replace a potential alloc_or_reuse by an _alloc.
 *
 * Note that we also have a wrapper function for this thingy:
 *
 * X::init_myX (int[10] & _OI_object)     which calls the one above
 *                                        with its reference parameter.
 * Consequently, this traversal elides that call, ending in an
 * (as far as I can tell) unused function.
 *
 * Any program that wants to use this global object will call
 * X:_INIT::init_myX with a pointer to the global variable as argument!
 *
 */

#include "manage_object_initialisers.h"

#include "traverse.h"
#include "free.h"

#define DBUG_PREFIX "MOI"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "namespaces.h"
#include "ctinfo.h"
#include "print.h"

/*
 * INFO structure
 */
struct INFO {
    node *args;
    bool mdelete;
};

/*
 * INFO macros
 */
#define INFO_ARGS(n) ((n)->args)
#define INFO_DELETE(n) ((n)->mdelete)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_ARGS (result) = NULL;
    INFO_DELETE (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * helper functions
 */
static bool
ArgsContainAvis (node *args, node *avis)
{
    bool result;

    DBUG_ENTER ();

    if (args == NULL) {
        result = FALSE;
    } else {
        result = (ARG_AVIS (args) == avis) || ArgsContainAvis (ARG_NEXT (args), avis);
    }

    DBUG_RETURN (result);
}

/*
 * traversal functions
 */
node *
MOIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * we do it bottom up
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /*
     * check whether rhs contains a reference to one of
     * the args
     */
    INFO_DELETE (arg_info) = FALSE;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_DELETE (arg_info)) {
        DBUG_PRINT ("    found global object reference on RHS,"
                    " deleting the following assignment:");
        DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg_node));
        arg_node = FREEdoFreeNode (arg_node);
        INFO_DELETE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

node *
MOIid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_DELETE (arg_info)
      = INFO_DELETE (arg_info)
        || ArgsContainAvis (INFO_ARGS (arg_info), ID_AVIS (arg_node));

    DBUG_RETURN (arg_node);
}

node *
MOIprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);

    if (INFO_DELETE (arg_info) && (PRF_PRF (arg_node) == F_alloc_or_reuse)) {
        DBUG_PRINT ("    found global object reference in F_alloc_or_reuse,"
                    " modifying:");
        DBUG_EXECUTE (PRTdoPrintFile (stderr, arg_node));
        DBUG_PRINT ("    into:");
        PRF_EXPRS4 (arg_node) = FREEdoFreeTree (PRF_EXPRS4 (arg_node));
        PRF_PRF (arg_node) = F_alloc;
        DBUG_EXECUTE (PRTdoPrintFile (stderr, arg_node));
        INFO_DELETE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

node *
MOIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISOBJINITFUN (arg_node)) {
        DBUG_PRINT (">>> entering fundef %s", CTIitemName (arg_node));

        /*
         * this is an init function, so we have to remove all lhs references
         * to the arguments in the body!
         */
        if (FUNDEF_BODY (arg_node) != NULL) {
            INFO_ARGS (arg_info) = FUNDEF_ARGS (arg_node);
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            INFO_ARGS (arg_info) = NULL;
        }

        DBUG_PRINT ("<<< leaving fundef %s", CTIitemName (arg_node));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 * Traversal start function
 */
node *
MOIdoManageObjectInitialisers (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();
    TRAVpush (TR_moi);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
