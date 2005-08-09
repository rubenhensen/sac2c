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
#include "shape.h"
#include "tree_basic.h"
#include "dbug.h"
#include "ctinfo.h"
#include "globals.h"
#include "namespaces.h"
#include "internal_lib.h"

static ntype *
RSTntype (ntype *arg_type, info *arg_info)
{
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("RSTntype");

    DBUG_EXECUTE ("RST", tmp_str = TYtype2DebugString (arg_type, FALSE, 0););
    DBUG_PRINT ("RST", ("starting to process type %s", tmp_str));
    DBUG_EXECUTE ("RST", tmp_str = ILIBfree (tmp_str););

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
        int max = TYgetProductSize (arg_type);
        int cnt;
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
            CTIerror ("unknown user defined type `%s::%s'.",
                      NSgetName (TYgetNamespace (arg_type)), TYgetName (arg_type));
        } else {
            DBUG_PRINT ("RST",
                        ("resolved symbol type %s:%s.",
                         NSgetName (TYgetNamespace (arg_type)), TYgetName (arg_type)));

            arg_type = TYfreeType (arg_type);
            arg_type = TYmakeUserType (udt);
        }
    }

    DBUG_EXECUTE ("RST", tmp_str = TYtype2DebugString (arg_type, FALSE, 0););
    DBUG_PRINT ("RST", ("resulting type is %s", tmp_str));
    DBUG_EXECUTE ("RST", tmp_str = ILIBfree (tmp_str););

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

/*******************************************************************************
 *
 * function:
 *   ntype *CheckUdtAndSetBaseType( usertype udt, int* visited)
 *
 * description:
 *  This function checks the integrity of a user defined type, and while doing
 *  so it converts Symb{} types into Udt{} types, it computes its base-type,
 *  AND stores it in the udt-repository!
 *  At the time being, the following restrictions apply:
 *  - the defining type has to be one of Symb{} Simple{}, AKS{ Symb{}},
 *    or AKS{ Simple{}}.
 *  - if the defining type contains a Symb{} type, this type and all further
 *    decendents must be defined without any recursion in type definitions!
 *
 *  The second parameter ("visited") is needed for detecting recusive
 *  definitions only. Therefore, the initial call should be made with
 *  (visited == NULL)!
 *
 *  We ASSUME, that the existence of a basetype indicates that the udt has
 *  been checked already!!!
 *  Furthermore, we ASSUME that iff basetype is not yet set, the defining
 *  type either is a simple- or a symbol-type, NOT a user-type!
 *
 ******************************************************************************/

static ntype *
CheckUdtAndSetBaseType (usertype udt, int *visited)
{
    ntype *base, *base_elem;
    usertype inner_udt;
    ntype *inner_base;
    ntype *new_base, *new_base_elem;
    int num_udt, i;

    DBUG_ENTER ("CheckUdtandSetBaseType");

    base = UTgetBaseType (udt);
    if (base == NULL) {
        base = UTgetTypedef (udt);
        if (!(TYisScalar (base) || TYisAKS (base))) {
            CTIerrorLine (global.linenum,
                          "Typedef of %s::%s is illegal; should be either"
                          " scalar type or array type of fixed shape",
                          NSgetName (UTgetNamespace (udt)), UTgetName (udt));
        } else {
            /*
             * Here, we know that we are either dealing with
             * Symb{} Simple{}, AKS{ Symb{}}, or AKS{ Simple{}}.
             * If we would be dealing with    User{} or AKS{ User{}}
             * base type would have been set already!
             */
            if (TYisSymb (base) || TYisAKSSymb (base)) {
                base_elem = (TYisSymb (base) ? base : TYgetScalar (base));
                inner_udt
                  = UTfindUserType (TYgetName (base_elem), TYgetNamespace (base_elem));
                if (inner_udt == UT_NOT_DEFINED) {
                    CTIerrorLine (global.linenum,
                                  "Typedef of %s::%s is illegal; type %s::%s unknown",
                                  NSgetName (UTgetNamespace (udt)), UTgetName (udt),
                                  NSgetName (TYgetNamespace (base_elem)),
                                  TYgetName (base_elem));
                } else {
                    /*
                     * First, we replace the defining symbol type by the appropriate
                     * user-defined-type, i.e., inner_udt!
                     */
                    new_base_elem = TYmakeUserType (inner_udt);
                    new_base
                      = (TYisSymb (base)
                           ? new_base_elem
                           : TYmakeAKS (new_base_elem, SHcopyShape (TYgetShape (base))));
                    UTsetTypedef (udt, new_base);
                    TYfreeType (base);
                    base = new_base;

                    /*
                     * If this is the initial call, we have to allocate and
                     * initialize our recursion detection mask "visited".
                     */
                    if (visited == NULL) {
                        /* This is the initial call, so visited has to be initialized! */
                        num_udt = UTgetNumberOfUserTypes ();
                        visited = (int *)ILIBmalloc (sizeof (int) * num_udt);
                        for (i = 0; i < num_udt; i++)
                            visited[i] = 0;
                    }
                    /*
                     * if we have not yet checked the inner_udt, recursively call
                     * CheckUdtAndSetBaseType!
                     */
                    if (visited[inner_udt] == 1) {
                        CTIerrorLine (global.linenum, "Type %s:%s recursively defined",
                                      NSgetName (UTgetNamespace (udt)), UTgetName (udt));
                    } else {
                        visited[udt] = 1;
                        inner_base = CheckUdtAndSetBaseType (inner_udt, visited);
                        /*
                         * Finally, we compute the resulting base-type by nesting
                         * the inner base type with the actual typedef!
                         */
                        base = TYnestTypes (base, inner_base);
                    }
                }
            } else {
                /*
                 * Here, we deal with Simple{} or AKS{ Simple{}}. In both cases
                 * base is the base type. Therefore, there will be no further
                 * recursice call. This allows us to free "visited".
                 * To be precise, we would have to free "visited in all ERROR-cases
                 * as well, but we neglect that since in case of an error the
                 * program will terminate soon anyways!
                 */
                if (visited != NULL)
                    visited = ILIBfree (visited);
            }
        }
        UTsetBaseType (udt, base);
    }

    DBUG_RETURN (base);
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
 *   see "CheckUdtAndSetBaseType") and replace the defining type by its
 *   basetype.
 *
 ******************************************************************************/
node *
RSTtypedef (node *arg_node, info *arg_info)
{
#ifndef DBUG_OFF
    char *tmp_str;
#endif
    usertype udt;
    ntype *base;

    DBUG_ENTER ("RSTtypedef");

    if (TYPEDEF_ISLOCAL (arg_node)) {
        udt = UTfindUserType (TYPEDEF_NAME (arg_node), TYPEDEF_NS (arg_node));

        if (udt != UT_NOT_DEFINED) {
            CTIerrorLine (global.linenum,
                          "Type %s multiply defined;"
                          " previous definition in line %d",
                          CTIitemName (arg_node), UTgetLine (udt));
        }

        DBUG_EXECUTE ("UDT",
                      tmp_str = TYtype2String (TYPEDEF_NTYPE (arg_node), FALSE, 0););
        DBUG_PRINT ("UDT", ("adding user type %s defined as %s", CTIitemName (arg_node),
                            tmp_str));
        DBUG_EXECUTE ("UDT", tmp_str = ILIBfree (tmp_str););

        UTaddUserType (ILIBstringCopy (TYPEDEF_NAME (arg_node)),
                       NSdupNamespace (TYPEDEF_NS (arg_node)),
                       TYcopyType (TYPEDEF_NTYPE (arg_node)), NULL, global.linenum,
                       arg_node);
    } else {
        DBUG_EXECUTE ("UDT",
                      tmp_str = TYtype2String (TYPEDEF_NTYPE (arg_node), FALSE, 0););
        DBUG_PRINT ("UDT", ("passing user type %s defined as %s", CTIitemName (arg_node),
                            tmp_str));
        DBUG_EXECUTE ("UDT", tmp_str = ILIBfree (tmp_str););
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);
    }

    udt = UTfindUserType (TYPEDEF_NAME (arg_node), TYPEDEF_NS (arg_node));

    if (TYPEDEF_ISLOCAL (arg_node)) {
        base = CheckUdtAndSetBaseType (udt, NULL);
    } else {
        base = UTgetBaseType (udt);
    }

    TYPEDEF_NTYPE (arg_node) = base;

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

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
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

    if (AVIS_DECLTYPE (arg_node) != NULL) {
        AVIS_DECLTYPE (arg_node) = RSTntype (AVIS_DECLTYPE (arg_node), arg_info);
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
RSTcast (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSTcast");

    if (CAST_NTYPE (arg_node) != NULL) {
        CAST_NTYPE (arg_node) = RSTntype (CAST_NTYPE (arg_node), arg_info);
    }

    arg_node = TRAVcont (arg_node, arg_info);

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
