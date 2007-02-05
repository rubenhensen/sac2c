/*
 *
 * $Id$
 *
 */

#include "object_init.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "dbug.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "new_types.h"
#include "shape.h"
#include "namespaces.h"
#include "DupTree.h"

/*
 * helper functions
 */

/** <!-- ****************************************************************** -->
 * @brief builds a type conversion fun converting from type from to type to.
 *
 * @param name    name of function to be built
 * @param ns      ns of function to be built
 * @param from    type to convert from
 * @param to      type to convert to
 * @param prf     primitive function to stick in
 *
 * @return N_fundef node representing the type conversion
 ******************************************************************************/
static node *
BuildTypeConversion (const char *name, const namespace_t *ns, ntype *from, ntype *to,
                     prf prf)
{
    node *result;
    node *avisarg;
    node *assign;
    node *block;

    DBUG_ENTER ("BuildTypeConversion");

    avisarg = TBmakeAvis (STRcpy ("from"), TYcopyType (from));
    AVIS_DECLTYPE (avisarg) = TYcopyType (AVIS_TYPE (avisarg));

    /*
     * return( res);
     */
    assign = TBmakeAssign (TBmakeReturn (
                             TBmakeExprs (TBmakeSpid (NULL, STRcpy ("result")), NULL)),
                           NULL);
    /*
     * res = prf( (:restype) arg);
     */
    assign = TBmakeAssign (TBmakeLet (TBmakeSpids (STRcpy ("result"), NULL),
                                      TBmakeCast (TYcopyType (to), TBmakeId (avisarg))),
                           assign);

    /*
     * create the fundef body block
     */
    block = TBmakeBlock (assign, NULL);

    /*
     * create the fundef node
     */
    result = TBmakeFundef (STRcpy (name), NSdupNamespace (ns),
                           TBmakeRet (TYcopyType (to), NULL), TBmakeArg (avisarg, NULL),
                           block, NULL);

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *CreateTypeConversions( node *typedefs, node *funs)
 *
 * @brief creates type conversion funs for all local class typedefs in chain
 *
 * @param typedefs typedef chain
 * @param funs fundef chain
 *
 * @return fundef chain with new funs appended in front
 ******************************************************************************/
static node *
CreateTypeConversions (node *typedefs, node *funs)
{
    DBUG_ENTER ("CreateTypeConversions");

    if (TYPEDEF_NEXT (typedefs) != NULL) {
        funs = CreateTypeConversions (TYPEDEF_NEXT (typedefs), funs);
    }

    if (TYPEDEF_ISUNIQUE (typedefs) && TYPEDEF_ISLOCAL (typedefs)) {
        node *to_fun, *from_fun;
        char *to_name, *from_name;
        ntype *tdef_type;

        to_name = STRcat ("to_", TYPEDEF_NAME (typedefs));
        from_name = STRcat ("from_", TYPEDEF_NAME (typedefs));

        tdef_type = TYmakeAKS (TYmakeSymbType (STRcpy (TYPEDEF_NAME (typedefs)),
                                               NSdupNamespace (TYPEDEF_NS (typedefs))),
                               SHmakeShape (0));

        to_fun = BuildTypeConversion (to_name, TYPEDEF_NS (typedefs),
                                      TYPEDEF_NTYPE (typedefs), tdef_type, F_to_unq);

        from_fun = BuildTypeConversion (from_name, TYPEDEF_NS (typedefs), tdef_type,
                                        TYPEDEF_NTYPE (typedefs), F_from_unq);

        FUNDEF_NEXT (to_fun) = funs;
        FUNDEF_NEXT (from_fun) = to_fun;
        funs = from_fun;

        tdef_type = TYfreeType (tdef_type);
        to_name = MEMfree (to_name);
        from_name = MEMfree (from_name);
    }

    DBUG_RETURN (funs);
}

/** <!-- ****************************************************************** -->
 * @fn node *BuildInitFun( char *name, namespace_t *ns,
 *                         ntype *objtype, node *expr)
 *
 * @brief builds an init function. Consumes ALL its arguments!
 *
 * @param name     name of function to build
 * @param ns       namespace -"-
 * @param objtype  returntype -"-
 * @param expr     expression -"-
 *
 * @return N_fundef node
 ******************************************************************************/
static node *
BuildInitFun (char *name, namespace_t *ns, ntype *objtype, node *expr)
{
    node *result;
    node *assign;
    node *argavis;

    DBUG_ENTER ("BuildInitFun");

    argavis = TBmakeAvis (STRcpy ("_OI_object"), objtype);
    AVIS_DECLTYPE (argavis) = TYcopyType (AVIS_TYPE (argavis));

    /*
     * return( );
     */
    assign = TBmakeAssign (TBmakeReturn (NULL), NULL);

    /*
     * arg = <expr>
     *
     * we have to use a SPIDS here, as this code is added prior to
     * insert vardec!
     */
    assign = TBmakeAssign (TBmakeLet (TBmakeSpids (STRcpy ("_OI_object"), NULL), expr),
                           assign);

    /*
     * void <ns>::<name> (<objtype> &arg)
     */
    result = TBmakeFundef (name, ns, NULL, TBmakeArg (argavis, NULL),
                           TBmakeBlock (assign, NULL), NULL);
    ARG_ISREFERENCE (FUNDEF_ARGS (result)) = TRUE;

    DBUG_RETURN (result);
}

static node *
CreateInitFuns (node *objdefs, node *funs)
{
    node *initfun;

    DBUG_ENTER ("CreateInitFuns");

    if (OBJDEF_NEXT (objdefs) != NULL) {
        funs = CreateInitFuns (OBJDEF_NEXT (objdefs), funs);
    }

    if (OBJDEF_ISLOCAL (objdefs) && !OBJDEF_ISEXTERN (objdefs)
        && !OBJDEF_ISALIAS (objdefs)) {
        initfun
          = BuildInitFun (STRcat ("init_", OBJDEF_NAME (objdefs)), NSgetInitNamespace (),
                          TYcopyType (OBJDEF_TYPE (objdefs)), OBJDEF_EXPR (objdefs));

        OBJDEF_EXPR (objdefs) = NULL;
        OBJDEF_INITFUN (objdefs) = initfun;

        FUNDEF_NEXT (initfun) = funs;
        funs = initfun;
    }

    DBUG_RETURN (funs);
}

/** <!-- ****************************************************************** -->
 * @fn node *OIdoObjectInit(node *syntax_tree)
 *
 * @brief starter function of object initialisation.
 *
 * During object initialisation, the special conversion funs to_XXX and
 * from_XXX are built and inserted into the AST. Firthermore, the init
 * expression of every local objdef is lifted into a special init function
 * to allow for arbitraty expressions in object init expressions.
 *
 * @param syntax_tree the entire syntaxtree, thus the N_module node.
 *
 * @return the transformed tree
 ******************************************************************************/
node *
OIdoObjectInit (node *syntax_tree)
{
    DBUG_ENTER ("OIdoObjectInit");

    if (MODULE_TYPES (syntax_tree) != NULL) {
        MODULE_FUNS (syntax_tree)
          = CreateTypeConversions (MODULE_TYPES (syntax_tree), MODULE_FUNS (syntax_tree));
    }

    if (MODULE_OBJS (syntax_tree) != NULL) {
        MODULE_FUNS (syntax_tree)
          = CreateInitFuns (MODULE_OBJS (syntax_tree), MODULE_FUNS (syntax_tree));
    }

    DBUG_RETURN (syntax_tree);
}
