/**
 * @file resolvesymboltypes.c
 * @brief this phase resolves all symbol-types to the
 *        corresponding user-defined-types or
 *        generates an error message otherwise
 * @author Stephan Herhut
 * @date 2005-05-31
 */

#include "resolvesymboltypes.h"
#include "traverse.h"
#include "new_types.h"
#include "user_types.h"
#include "tree_basic.h"
#include "dbug.h"
#include "ctinfo.h"

static ntype *
RSTntype (ntype *arg_type, info *arg_info)
{
    DBUG_ENTER ("RSTntype");

    if (TYisArray (arg_type)) {
        ntype *scalar = TYgetScalar (arg_type);
        scalar = RSTntype (scalar, arg_info);
        arg_type = TYsetScalar (arg_type, scalar);
    } else if (TYisProd (arg_type)) {
        int max = TYgetProductSize (arg_type);
        int cnt;
        ntype *member;

        for (cnt = 0; cnt < max; cnt++) {
            member = TYgetProductMember (arg_type, cnt);
            member = RSTntype (member, arg_info);
            arg_type = TYsetProductMember (arg_type, cnt, member);
        }
    } else if (TYisFun (arg_type)) {
        /*
         * there is no mechanism to traverse the contents
         * of a fun type, so we leave it alone here
         */
    } else if (TYisSymb (arg_type)) {
        usertype udt;

        udt = UTfindUserType (TYgetName (arg_type), TYgetMod (arg_type));

        if (udt == UT_NOT_DEFINED) {
            CTIerror ("unknown user defined type `%s:%s'.", TYgetMod (arg_type),
                      TYgetName (arg_type));
        } else {
            DBUG_PRINT ("RST", ("resolved symbol type %s:%s.", TYgetMod (arg_type),
                                TYgetName (arg_type)));

            arg_type = TYfreeType (arg_type);
            arg_type = TYmakeUserType (udt);
        }
    }

    DBUG_RETURN (arg_type);
}

node *
RSTmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSTmodule");

    /*
     * gather udt database
     */
    if (MODULE_TYPES (arg_node) != NULL) {
        MODULE_TYPES (arg_node) = TRAVdo (MODULE_TYPES (arg_node), arg_info);
    }

    /*
     * resolve symbol types
     */
    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSTtypedef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSTtypedef");

    /*
     * nothing to do here yet, as up to now the udt database
     * is built during typechecking
     */

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSTfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSTfundef");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_RETS (arg_node) != NULL) {
        FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
    }

    if (FUNDEF_WRAPPERTYPE (arg_node) != NULL) {
        FUNDEF_WRAPPERTYPE (arg_node)
          = RSTntype (FUNDEF_WRAPPERTYPE (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSTarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSTarg");

    if (ARG_AVIS (arg_node) != NULL) {
        ARG_AVIS (arg_node) = TRAVdo (ARG_AVIS (arg_node), arg_info);
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSTret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSTret");

    if (RET_TYPE (arg_node) != NULL) {
        RET_TYPE (arg_node) = RSTntype (RET_TYPE (arg_node), arg_info);
    }

    if (RET_NEXT (arg_node) != NULL) {
        RET_NEXT (arg_node) = TRAVdo (RET_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSTavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSTavis");

    if (AVIS_TYPE (arg_node) != NULL) {
        AVIS_TYPE (arg_node) = RSTntype (AVIS_TYPE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSTdoResolveSymbolTypes (node *syntax_tree)
{
    DBUG_ENTER ("RSTdoResolveSymbolTypes");

    TRAVpush (TR_rst);

    syntax_tree = TRAVdo (syntax_tree, NULL);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}
