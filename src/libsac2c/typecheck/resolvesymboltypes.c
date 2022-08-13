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
#include "type_utils.h"
#include "shape.h"
#include "tree_basic.h"

#define DBUG_PREFIX "RST"
#include "debug.h"

#include "ctinfo.h"
#include "globals.h"
#include "namespaces.h"
#include "str.h"
#include "memory.h"

static ntype *
RSTntype (ntype *arg_type, info *arg_info)
{
#ifndef DBUG_OFF
    char *tmp_str = NULL;
#endif

    DBUG_ENTER ();

    DBUG_EXECUTE (tmp_str = TYtype2DebugString (arg_type, FALSE, 0));
    DBUG_PRINT ("starting to process type %s", tmp_str);
    DBUG_EXECUTE (tmp_str = MEMfree (tmp_str));

    /*
     * as the TYget functions do not copy the internal type
     * prior to returning it, we have to copy it here.
     * This is neccessary, as the TYset functions free the inner
     * type prior to assigning the new one.
     */
    if (TYisArray (arg_type)) {
        ntype *scalar = TYcopyType (TYgetScalar (arg_type));
        scalar = RSTntype (scalar, arg_info);
        arg_type = TYsetScalar (arg_type, scalar);
    } else if (TYisProd (arg_type)) {
        size_t max = TYgetProductSize (arg_type);
        size_t cnt;
        ntype *member;

        for (cnt = 0; cnt < max; cnt++) {
            member = TYcopyType (TYgetProductMember (arg_type, cnt));
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

        udt = UTfindUserType (TYgetName (arg_type), TYgetNamespace (arg_type));

        if (udt == UT_NOT_DEFINED) {
            CTIerror (EMPTY_LOC, "unknown user defined type `%s::%s'.",
                      NSgetName (TYgetNamespace (arg_type)), TYgetName (arg_type));
        } else {
            DBUG_PRINT ("resolved symbol type %s:%s.",
                        NSgetName (TYgetNamespace (arg_type)), TYgetName (arg_type));

            arg_type = TYfreeType (arg_type);
            arg_type = TYmakeUserType (udt);
        }
    }

    DBUG_EXECUTE (tmp_str = TYtype2DebugString (arg_type, FALSE, 0));
    DBUG_PRINT ("resulting type is %s", tmp_str);
    DBUG_EXECUTE (tmp_str = MEMfree (tmp_str));

    DBUG_RETURN (arg_type);
}

node *
RSTmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * gather udt database
     */
    if (MODULE_TYPES (arg_node) != NULL) {
        MODULE_TYPES (arg_node) = TRAVdo (MODULE_TYPES (arg_node), arg_info);
    }

    /*
     * resolve symbol types
     */
    if (MODULE_OBJS (arg_node) != NULL) {
        MODULE_OBJS (arg_node) = TRAVdo (MODULE_OBJS (arg_node), arg_info);
    }

    if (MODULE_FUNSPECS (arg_node) != NULL) {
        MODULE_FUNSPECS (arg_node) = TRAVdo (MODULE_FUNSPECS (arg_node), arg_info);
    }

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *RSTtypedef(node *arg_node, info *arg_info)
 *
 * description:
 *   On the traversal down, we insert all user defined types. While doing so
 *   we check on duplicate definitions and issue ERROR-messages if neccessary.
 *   On the traversal up, we check on consistency (for the exact restrictions
 *   see "UTcheckUdtAndSetBaseType") and replace the defining type by its
 *   basetype.
 *
 ******************************************************************************/
node *
RSTtypedef (node *arg_node, info *arg_info)
{
#ifndef DBUG_OFF
    char *tmp_str = NULL;
#endif
    char *err_str1, *err_str2;
    usertype udt, alias;

    DBUG_ENTER ();

    if (TYPEDEF_ISLOCAL (arg_node)) {
        udt = UTfindUserType (TYPEDEF_NAME (arg_node), TYPEDEF_NS (arg_node));

        if (udt != UT_NOT_DEFINED) {
            /*
             * depending on whether the type is an alias (thus imported)
             * or locally defined, we create different error messages.
             */
            if (TYPEDEF_ISALIAS (arg_node)) {
                err_str1 = TYtype2String (TYPEDEF_NTYPE (arg_node), FALSE, 0);
            } else {
                err_str1 = STRcpy (CTIitemName (arg_node));
            }

            if (UTisAlias (udt)) {
                err_str2 = TYtype2String (UTgetTypedef (udt), FALSE, 0);
            } else {
                err_str2 = STRcpy (CTIitemName (UTgetTdef (udt)));
            }

            CTIerror (LINE_TO_LOC (global.linenum),
                      "%s %s collides with previously %s %s in line %zu.",
                      TYPEDEF_ISALIAS (arg_node) ? "Imported type"
                                                 : "Local definition of",
                      err_str1, UTisAlias (udt) ? "imported type" : "defined type",
                      err_str2, UTgetLine (udt));

            err_str1 = MEMfree (err_str1);
            err_str2 = MEMfree (err_str2);
        }

        DBUG_EXECUTE_TAG ("UDT",
                          tmp_str = TYtype2String (TYPEDEF_NTYPE (arg_node), FALSE, 0));

        if (TYPEDEF_ISALIAS (arg_node)) {
            DBUG_PRINT_TAG ("UDT", "adding user type alias %s for %s",
                            CTIitemName (arg_node), tmp_str);

            DBUG_ASSERT (TYisAKSUdt (TYPEDEF_NTYPE (arg_node)),
                         "invalid type alias found!");

            DBUG_ASSERT (TYgetDim (TYPEDEF_NTYPE (arg_node)) == 0,
                         "non scalar type as type alias found");

            alias = TYgetUserType (TYgetScalar (TYPEDEF_NTYPE (arg_node)));

            UTaddAlias (STRcpy (TYPEDEF_NAME (arg_node)),
                        NSdupNamespace (TYPEDEF_NS (arg_node)), alias,
                        NODE_LINE (arg_node), arg_node);
        } else {
            DBUG_PRINT_TAG ("UDT", "adding user type %s defined as %s",
                            CTIitemName (arg_node), tmp_str);

            UTaddUserType (STRcpy (TYPEDEF_NAME (arg_node)),
                           NSdupNamespace (TYPEDEF_NS (arg_node)),
                           TYcopyType (TYPEDEF_NTYPE (arg_node)), NULL,
                           NODE_LINE (arg_node), arg_node,
                           TYPEDEF_ISNESTED (arg_node),
                           TYPEDEF_ISEXTERNAL (arg_node));
        }

        DBUG_EXECUTE_TAG ("UDT", tmp_str = MEMfree (tmp_str));
    } else {
        DBUG_EXECUTE_TAG ("UDT",
                          tmp_str = TYtype2String (TYPEDEF_NTYPE (arg_node), FALSE, 0));
        DBUG_PRINT_TAG ("UDT", "passing user type %s defined as %s",
                        CTIitemName (arg_node), tmp_str);
        DBUG_EXECUTE_TAG ("UDT", tmp_str = MEMfree (tmp_str));
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);
    }

    udt = UTfindUserType (TYPEDEF_NAME (arg_node), TYPEDEF_NS (arg_node));

    TUcheckUdtAndSetBaseType (udt, NULL);

    /*
     * now that all types are in the UDT base, we can process the
     * declared type and replace it by a udt if it is a symbol
     * type.
     */
    TYPEDEF_NTYPE (arg_node) = RSTntype (TYPEDEF_NTYPE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
RSTobjdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    OBJDEF_TYPE (arg_node) = RSTntype (OBJDEF_TYPE (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
RSTfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

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

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSTarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

    if (AVIS_TYPE (arg_node) != NULL) {
        AVIS_TYPE (arg_node) = RSTntype (AVIS_TYPE (arg_node), arg_info);
    }

    if (AVIS_DECLTYPE (arg_node) != NULL) {
        AVIS_DECLTYPE (arg_node) = RSTntype (AVIS_DECLTYPE (arg_node), arg_info);
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
RSTarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (ARRAY_ELEMTYPE (arg_node) != NULL) {
        ARRAY_ELEMTYPE (arg_node) = RSTntype (ARRAY_ELEMTYPE (arg_node), arg_info);
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
RSTcast (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (CAST_NTYPE (arg_node) != NULL) {
        CAST_NTYPE (arg_node) = RSTntype (CAST_NTYPE (arg_node), arg_info);
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
RSTtype (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    TYPE_TYPE (arg_node) = RSTntype (TYPE_TYPE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
RSTdoResolveSymbolTypes (node *syntax_tree)
{
    DBUG_ENTER ();

    TRAVpush (TR_rst);

    syntax_tree = TRAVdo (syntax_tree, NULL);

    TRAVpop ();

    DBUG_EXECUTE (UTprintRepository (stderr));

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
