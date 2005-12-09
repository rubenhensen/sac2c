/*
 *
 * $Id$
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "dbug.h"
#include "internal_lib.h"
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
 *
 * @return N_fundef node representing the type conversion
 ******************************************************************************/
static node *
BuildTypeConversion (const char *name, const namespace_t *ns, ntype *from, ntype *to)
{
    node *result;
    node *avisarg, *avisres;
    node *assign;
    node *block;

    DBUG_ENTER ("BuildTypeConversion");

    avisarg = TBmakeAvis (ILIBstringCopy ("from"), TYcopyType (from));
    avisres = TBmakeAvis (ILIBstringCopy ("result"), TYcopyType (to));

    AVIS_DECLTYPE (avisarg) = TYcopyType (AVIS_TYPE (avisarg));

    /*
     * return( res);
     */
    assign = TBmakeAssign (TBmakeReturn (TBmakeExprs (TBmakeId (avisres), NULL)), NULL);
#if 0
  /*
   * res = type_conv( restype, arg);
   */
  assign = TBmakeAssign(
             TBmakeLet(
               TBmakeIds( avisres, NULL),
               TCmakePrf2(
                 F_type_conv,
                 TBmakeType( 
                   TYcopyType( to)),
                 TBmakeId( avisarg))), 
             assign);
#else
    /*
     * res = (:restype) arg;
     */
    assign = TBmakeAssign (TBmakeLet (TBmakeIds (avisres, NULL),
                                      TBmakeCast (TYcopyType (to), TBmakeId (avisarg))),
                           assign);
#endif

    /*
     * create the fundef body block
     */
    block = TBmakeBlock (assign, TBmakeVardec (avisres, NULL));

    /*
     * create the fundef node
     */
    result = TBmakeFundef (ILIBstringCopy (name), NSdupNamespace (ns),
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

        to_name = ILIBstringConcat ("to_", TYPEDEF_NAME (typedefs));
        from_name = ILIBstringConcat ("from_", TYPEDEF_NAME (typedefs));

        tdef_type = TYmakeAKS (TYmakeSymbType (ILIBstringCopy (TYPEDEF_NAME (typedefs)),
                                               NSdupNamespace (TYPEDEF_NS (typedefs))),
                               SHmakeShape (0));

        to_fun = BuildTypeConversion (to_name, TYPEDEF_NS (typedefs),
                                      TYPEDEF_NTYPE (typedefs), tdef_type);

        from_fun = BuildTypeConversion (from_name, TYPEDEF_NS (typedefs), tdef_type,
                                        TYPEDEF_NTYPE (typedefs));

        FUNDEF_NEXT (to_fun) = funs;
        FUNDEF_NEXT (from_fun) = to_fun;
        funs = from_fun;

        tdef_type = TYfreeType (tdef_type);
        to_name = ILIBfree (to_name);
        from_name = ILIBfree (from_name);
    }

    DBUG_RETURN (funs);
}

/** <!-- ****************************************************************** -->
 * @fn node *BuildInitFun( const char *name, namespace_t *ns,
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
BuildInitFun (const char *name, namespace_t *ns, ntype *objtype, node *expr)
{
    node *result;
    node *assign;

    DBUG_ENTER ("BuildInitFun");

    /*
     * return( expr);
     */
    assign = TBmakeAssign (TBmakeReturn (TBmakeExprs (expr, NULL)), NULL);

    /*
     * <objtype> <ns>::<name> ()
     */
    result = TBmakeFundef (name, ns, TBmakeRet (objtype, NULL), NULL,
                           TBmakeBlock (assign, NULL), NULL);

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

    if (OBJDEF_ISLOCAL (objdefs)) {
        initfun = BuildInitFun (ILIBstringConcat ("init_", OBJDEF_NAME (objdefs)),
                                NSgetInitNamespace (), TYcopyType (OBJDEF_TYPE (objdefs)),
                                OBJDEF_EXPR (objdefs));

        OBJDEF_EXPR (objdefs) = TBmakeAp (initfun, NULL);

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
