/*
 *
 * $Log$
 * Revision 3.8  2002/05/31 14:51:54  sbs
 * intermediate version to ensure compilable overall state.
 *
 * Revision 3.7  2002/03/12 15:15:24  sbs
 * wrapepr creation inserted.
 *
 * Revision 3.6  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.5  2001/05/17 11:34:07  sbs
 * return value of Free now used ...
 *
 * Revision 3.4  2001/05/17 09:20:42  sbs
 * MALLOC FREE aliminated
 *
 * Revision 3.3  2001/03/22 21:01:29  dkr
 * no changes done
 *
 * Revision 3.2  2001/03/22 20:39:02  dkr
 * include of tree.h eliminated
 *
 * Revision 3.1  2000/11/20 18:00:07  sacbase
 * new release made
 *
 * Revision 1.5  2000/06/23 13:58:57  dkr
 * functions for old with-loop removed (IdxGen, IdxWith)
 *
 * Revision 1.4  2000/03/15 15:59:08  dkr
 * SET_VARDEC_OR_ARG_COLCHN renamed to L_VARDEC_OR_ARG_COLCHN
 *
 * Revision 1.3  2000/01/26 17:28:10  dkr
 * type of traverse-function-table changed.
 *
 * Revision 1.1  1999/10/20 12:51:11  sbs
 * Initial revision
 *
 */

#include <stdio.h>
#include <string.h>
#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "globals.h"
#include "lac2fun.h"
#include "CheckAvis.h"
#include "SSATransform.h"
#include "insert_vardec.h"
#include "create_wrappers.h"

#include "user_types.h"
#include "new_types.h"
#include "new_typecheck.h"

/*
 * OPEN PROBLEMS:
 *
 * I) not yet solved:
 *
 * II) to be fixed here:
 *
 * III) to be fixed somewhere else:
 *
 */

/*
 */
/*
 * Thus, we finally find the following usages of the arg_info node:
 *
 *    INFO_NTC_TYPE             the inferred type of the expression traversed
 *    INFO_NTC_NUM_EXPRS_SOFAR  is used to count the number of exprs while
 *                              traversing them
 */

#define INFO_NTC_TYPE(n) ((ntype *)(n->dfmask[0]))
#define INFO_NTC_NUM_EXPRS_SOFAR(n) (n->flag)

typedef enum { NTC_not_checked, NTC_checking, NTC_checked } NTC_stat;

/******************************************************************************
 *
 * function:
 *    node *NewTypeCheck(node *arg_node)
 *
 * description:
 *    starts the new type checking traversal!
 *
 ******************************************************************************/

node *
NewTypeCheck (node *arg_node)
{
    funtab *tmp_tab;
    node *info_node;

    DBUG_ENTER ("NewTypeCheck");

    tmp_tab = act_tab;
    act_tab = ntc_tab;

    info_node = MakeInfo ();

    arg_node = Trav (arg_node, info_node);

    info_node = FreeNode (info_node);

    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 ***
 ***          local helper functions
 ***          ----------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    node *UpdateSignature( node *fundef, ntype *args)
 *
 * description:
 *    Here, we assume that all argument types are either array types or
 *    type variables with identical Min and Max!
 *    This function replaces the old type siganture (in the N_arg nodes)
 *    by the given argument types (arg_ts), and updates the function type
 *    ( FUNDEF_TYPE( fundef) ) as well. It returns the modified N_fundef
 *    node.
 *
 ******************************************************************************/

static node *
UpdateSignature (node *fundef, ntype *arg_ts)
{
    node *args;
    ntype *type;
    int i = 0;

    DBUG_ENTER ("UpdateSignature");
    DBUG_ASSERT ((CountArgs (FUNDEF_ARGS (fundef)) == TYGetProductSize (arg_ts)),
                 "UpdateSignature called with incompatible no of arguments!");
    DBUG_ASSERT ((TYIsProdOfArrayOrFixedAlpha (arg_ts)),
                 "UpdateSignature called with non-fixed args!");

    args = FUNDEF_ARGS (fundef);
    while (args) {
        type = TYGetProductMember (arg_ts, i);
        ARG_TYPE (args) = FreeOneTypes (ARG_TYPE (args));
        ARG_TYPE (args) = TYType2OldType (type);
        args = ARG_NEXT (args);
        i++;
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *    node *DispatchFunction( node *fundef, ntype *args)
 *
 * description:
 *    Here, we assume that all argument types are either array types or
 *    type variables with identical Min and Max!
 *
 ******************************************************************************/

static node *
DispatchFunction (node *fundef, ntype *args)
{
#ifndef DBUG_OFF
    char *tmp_str;
#endif
    node *res;

    DBUG_ENTER ("DispatchFunction");
    DBUG_ASSERT ((TYIsProdOfArrayOrFixedAlpha (args)),
                 "DispatchFunction called with non-fixed args!");

    if (FUNDEF_IS_LACFUN (fundef)) {
        UpdateSignature (fundef, args);
        FUNDEF_TYPE (fundef) = CreateFuntype (fundef);
        res = fundef;
    } else {
        DBUG_EXECUTE ("NTC", tmp_str = TYType2String (args, 0, 0););
        DBUG_PRINT ("NTC", ("dispatching %s for %s", FUNDEF_NAME (fundef), tmp_str));
        DBUG_EXECUTE ("NTC", Free (tmp_str););
        res = fundef;
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 ***
 ***          Here, the traversal functions start!
 ***          ------------------------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    node *NTCmodul(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCmodul (node *arg_node, node *arg_info)
{
    node *fundef;

    DBUG_ENTER ("NTCmodul");
#if 0
  /*
   * First, we gather all typedefs and setup the global table
   * which is kept in "new_types".
   */
  if ( NULL != MODUL_TYPES(arg_node))
    MODUL_TYPES(arg_node)=Trav(MODUL_TYPES(arg_node), arg_info);
  DBUG_EXECUTE( "UDT", UTPrintRepository( stderr););
  ABORT_ON_ERROR;
#endif

    /*
     * Before doing the actual type inference, we want to switch to the FUN-
     * representation. This requires seeral preparational steps:
     *
     * I) Inserting Vardecs
     * =====================
     *
     * First, we insert a vardec node for each identifier used.
     * This has to be done prior to Lac2Fun as Lac2Fun uses datafolowmasks
     * which in turn rely on the existance of Vardecs. Note here, that the
     * vardecs do not contain any type info yet!
     */
    arg_node = InsertVardec (arg_node);
    if ((break_after == PH_typecheck) && (0 == strcmp (break_specifier, "ivd"))) {
        goto DONE;
    }

    /*
     * II) Creating Wrappers
     * ======================
     *
     * Since Lac2Fun needs to know about reference parameters, it requires each
     * application of a user defined function to contain a backref to the function
     * that is actually applied. Since this -in general- cannot be statically decided,
     * the backref points to a wrapper function which contains the (intersection type
     * based) function type of the overloaded function as well as pointers to all
     * potential implementations. These structures are created by "CreateWrappers".
     */
    arg_node = CreateWrappers (arg_node);
    if ((break_after == PH_typecheck) && (0 == strcmp (break_specifier, "cwr"))) {
        goto DONE;
    }

    /*
     * Now that all ids have backrefs to vardecs and all funaps have backrefs
     * to wrapper-fundefs, Lac2Fun can finally be run.
     */
    arg_node = Lac2Fun (arg_node);
    if ((break_after == PH_typecheck) && (0 == strcmp (break_specifier, "l2f"))) {
        goto DONE;
    }

    arg_node = CheckAvis (arg_node);
    if ((break_after == PH_typecheck) && (0 == strcmp (break_specifier, "cha"))) {
        goto DONE;
    }

    arg_node = SSATransform (arg_node);
    if ((break_after == PH_typecheck) && (0 == strcmp (break_specifier, "ssa"))) {
        goto DONE;
    }

    /*
     * Now, we do the actual type inference ....
     *
     * First, we mark all functions that need to be checked as NTC_not_checked.
     * All others are marked NTC_checked:
     */

    fundef = MODUL_FUNS (arg_node);
    while (fundef != NULL) {
        if (FUNDEF_STATUS (fundef) != ST_wrapperfun) {
            FUNDEF_TCSTAT (fundef) = NTC_not_checked;
        } else {
            FUNDEF_TCSTAT (fundef) = NTC_checked;
        }
        fundef = FUNDEF_NEXT (fundef);
    }

    if (NULL != MODUL_FUNS (arg_node))
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);

DONE:
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
 *  The second parameter ("visited") is needed for detecting recusive definitions
 *  only. Therefore, the initial call should be made with (visited == NULL)!
 *
 *  We ASSUME, that the existence of a basetype indicates that the udt has
 *  been checked already!!!
 *  Furthermore, we ASSUME that iff basetype is not yet set, the defining
 *  type either is a simple- or a symbol-type, NOT a user-type!
 *
 ******************************************************************************/

ntype *
CheckUdtAndSetBaseType (usertype udt, int *visited)
{
    ntype *base, *base_elem;
    usertype inner_udt;
    ntype *inner_base;
    ntype *new_base, *new_base_elem;
    int num_udt, i;

    DBUG_ENTER ("CheckUdt");

    base = UTGetBaseType (udt);
    if (base == NULL) {
        base = UTGetTypedef (udt);
        if (!(TYIsScalar (base) || TYIsAKS (base))) {
            ERROR (linenum,
                   ("typedef of %s:%s is illegal; should be either scalar type or"
                    "array type of fixed shape",
                    UTGetMod (udt), UTGetName (udt)));
        } else {
            /*
             * Here, we know that we are either dealing with
             * Symb{} Simple{}, AKS{ Symb{}}, or AKS{ Simple{}}.
             * If we would be dealing with    User{} or AKS{ User{}}
             * base type would have been set already!
             */
            if (TYIsSymb (base) || TYIsAKSSymb (base)) {
                base_elem = (TYIsSymb (base) ? base : TYGetScalar (base));
                inner_udt = UTFindUserType (TYGetName (base_elem), TYGetMod (base_elem));
                if (inner_udt == UT_NOT_DEFINED) {
                    ERROR (linenum, ("typedef of %s:%s is illegal; type %s:%s unknown",
                                     UTGetMod (udt), UTGetName (udt),
                                     TYGetMod (base_elem), TYGetName (base_elem)));
                } else {
                    /*
                     * First, we replace the defining symbol type by the appropriate
                     * user-defined-type, i.e., inner_udt!
                     */
                    new_base_elem = TYMakeUserType (inner_udt);
                    new_base
                      = (TYIsSymb (base)
                           ? new_base_elem
                           : TYMakeAKS (new_base_elem, SHCopyShape (TYGetShape (base))));
                    UTSetTypedef (udt, new_base);
                    TYFreeType (base);
                    base = new_base;

                    /*
                     * If this is the initial call, we have to allocate and
                     * initialize our recursion detection mask "visited".
                     */
                    if (visited == NULL) {
                        /* This is the initial call, so visited has to be initialized! */
                        num_udt = UTGetNumberOfUserTypes ();
                        visited = (int *)Malloc (sizeof (int) * num_udt);
                        for (i = 0; i < num_udt; i++)
                            visited[i] = 0;
                    }
                    /*
                     * if we have not yet checked the inner_udt, recursively call
                     * CheckUdtAndSetBaseType!
                     */
                    if (visited[inner_udt] == 1) {
                        ERROR (linenum, ("type %s:%s recursively defined", UTGetMod (udt),
                                         UTGetName (udt)));
                    } else {
                        visited[udt] = 1;
                        inner_base = CheckUdtAndSetBaseType (inner_udt, visited);
                        /*
                         * Finally, we compute the resulting base-type by nesting
                         * the inner base type with the actual typedef!
                         */
                        base = TYNestTypes (base, inner_base);
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
                    visited = Free (visited);
            }
        }
        UTSetBaseType (udt, base);
    }

    DBUG_RETURN (base);
}

/******************************************************************************
 *
 * function:
 *    node *NTCtypedef(node *arg_node, node *arg_info)
 *
 * description:
 *   On the traversal down, we insert all user defined types. While doing so
 *   we check on duplicate definitions and issue ERROR-messages if neccessary.
 *   On the way back up we check on consistency (for the exact restrictions
 *   see "CheckUdtAndSetBaseType") and replace the defining type by its basetype.
 *
 ******************************************************************************/

node *
NTCtypedef (node *arg_node, node *arg_info)
{
    char *name, *mod;
    ntype *nt, *base;
    usertype udt;

    DBUG_ENTER ("NTCtypedef");
    name = TYPEDEF_NAME (arg_node);
    mod = TYPEDEF_MOD (arg_node);
    nt = TYOldType2Type (TYPEDEF_TYPE (arg_node), TY_symb);

    udt = UTFindUserType (name, mod);
    if (udt != UT_NOT_DEFINED) {
        ERROR (linenum, ("type %s:%s multiply defined; previous definition in line %d",
                         mod, name, UTGetLine (udt)));
    }
    udt = UTAddUserType (name, mod, nt, NULL, linenum);

    if (TYPEDEF_NEXT (arg_node) != NULL)
        TYPEDEF_NEXT (arg_node) = Trav (TYPEDEF_NEXT (arg_node), arg_info);

    base = CheckUdtAndSetBaseType (udt, NULL);
    TYPEDEF_TYPE (arg_node) = Free (TYPEDEF_TYPE (arg_node));
    TYPEDEF_TYPE (arg_node) = TYType2OldType (base);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCfundef(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

static node *
TypeCheckFunctionBody (node *arg_node, node *arg_info)
{
    ntype *res_type;
    ntype *ret_type;
    ntype *dtype, *itype;
    tvar *alpha;
    int i;
    bool ok;

    DBUG_ENTER ("TypeCheckFunctionBody");

    FUNDEF_TCSTAT (arg_node) = NTC_checking;

    /*
     * First, we put the ntype info into the AVIS_TYPE fields:
     */
    if (NULL != FUNDEF_ARGS (arg_node)) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    /*
     * Then, we infer the type of the body:
     */
    if (NULL != FUNDEF_BODY (arg_node)) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

#if 0
    /*
     * The result type is now available in INFO_NTC_TYPE( arg_info).
     * It should be a product type of array types or type variables
     * with identical lower and upper limits !!! (cf. NTCreturn).
     */

    DBUG_ASSERT( TYIsProdOfArrayOrFixedAlpha( INFO_NTC_TYPE( arg_info)),
                 "type inference of a function body did not return a fixed product type");
#endif

        /*
         * Finaly, we insert the inferred type into the result type, iff legal
         */
        ret_type = INFO_NTC_TYPE (arg_info);
        res_type = FUNDEF_RET_TYPE (arg_node);

        for (i = 0; i < TYGetProductSize (ret_type); i++) {
            dtype = TYGetProductMember (res_type, i);
            alpha = TYGetAlpha (dtype);
            itype = TYGetProductMember (ret_type, i);
            /*
             * Since we know that INFO_NTC_TYPE( arg_info) is fixed now, we can convert
             * it into a type varibale free form (which is required by SSINewMin and
             * SSINewMax)
             */
            itype = TYEliminateAlpha (itype);
            ok = (SSINewMin (alpha, itype) && SSINewMax (alpha, itype));
            if (!ok) {
                ABORT (NODE_LINE (arg_node),
                       ("component #%d of inferred return type (%s) is not within %s", i,
                        TYType2String (itype, FALSE, 0),
                        TYType2String (dtype, FALSE, 0)));
            } else {
                TYFreeType (itype);
            }
        }
        TYFreeTypeConstructor (ret_type);
    }
    DBUG_RETURN (arg_node);
}

node *
NTCfundef (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("NTCfundef");

    if ((FUNDEF_TCSTAT (arg_node) == NTC_not_checked) && !FUNDEF_IS_LACFUN (arg_node)) {
        arg_node = TypeCheckFunctionBody (arg_node, arg_info);
    }

    if (NULL != FUNDEF_NEXT (arg_node))
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCarg(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("NTCarg");

    AVIS_TYPE (ARG_AVIS (arg_node)) = TYOldType2Type (ARG_TYPE (arg_node), TY_symb);

    if (NULL != ARG_NEXT (arg_node))
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * NTCblock( node *arg_node, node *arg_info )
 *
 * description:
 *
 ******************************************************************************/

node *
NTCblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("NTCblock");

    /*
     * The vardecs can be ignored as they are modified during the traversal of the
     * instructions!
     */

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTClet(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
NTClet (node *arg_node, node *arg_info)
{
    ntype *rhs_type;
    ntype *type;
    node *avis;
    ids *lhs;
    int i;
#ifndef DBUG_OFF
    char *tmp_str, *tmp_str2;
#endif

    DBUG_ENTER ("NTClet");

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    rhs_type = INFO_NTC_TYPE (arg_info);

    lhs = LET_IDS (arg_node);

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap) {
        DBUG_ASSERT ((CountIds (lhs) == TYGetProductSize (rhs_type)),
                     "fun ap does not match no of lhs vars!");
        i = 0;
        while (lhs) {
            AVIS_TYPE (IDS_AVIS (lhs)) = TYGetProductMember (rhs_type, i);
            i++;
            lhs = IDS_NEXT (lhs);
        }
        TYFreeTypeConstructor (rhs_type);
    } else {
        /* lhs must be one ids only since rhs is not a function application! */
        DBUG_ASSERT ((CountIds (lhs) == 1),
                     "more than one lhs var without a function call on the rhs");
        avis = IDS_AVIS (lhs);
        /*
         * Now, we must check whether we do have a pseudo PHI function here
         */
        if ((AVIS_SSAPHITARGET (avis) == PHIT_NONE) || (AVIS_TYPE (avis) == NULL)) {
            AVIS_TYPE (avis) = rhs_type;
        } else {
            /*
             * "second branch of a conditional"!
             */
            rhs_type = TYEliminateAlpha (rhs_type);
            AVIS_TYPE (avis) = TYEliminateAlpha (AVIS_TYPE (avis));

            DBUG_EXECUTE ("NTC", tmp_str = TYType2String (rhs_type, FALSE, 0););
            DBUG_EXECUTE ("NTC", tmp_str2 = TYType2String (AVIS_TYPE (avis), FALSE, 0););
            DBUG_PRINT ("NTC", ("fusing types \"%s\" and \"%s\"...", tmp_str, tmp_str2));
            DBUG_EXECUTE ("NTC", Free (tmp_str););
            DBUG_EXECUTE ("NTC", Free (tmp_str2););

            if (TYIsArray (rhs_type)) {
                if (TYIsArray (AVIS_TYPE (avis))) {
                    /* both are array types! */
                    type = TYLubOfTypes (rhs_type, AVIS_TYPE (avis));
                    if (type == NULL) {
                        ABORT (NODE_LINE (arg_node),
                               ("variable \"%s\" has two alternative definitions of "
                                "types \"%s\" and \"%s\""
                                " which do not have a common supertype",
                                strtok (IDS_NAME (lhs), "__SSA"),
                                TYType2String (AVIS_TYPE (avis), FALSE, 0),
                                TYType2String (rhs_type, FALSE, 0)));
                    } else {
                        AVIS_TYPE (avis) = type;
                    }
                } else {
                    /* first is alpha! */
                }
            } else {
                if (TYIsArray (AVIS_TYPE (avis))) {
                    /* second is alpha! */
                } else {
                    /* both are alpha! */
                }
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *NTCreturn(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("NTCreturn");

    /*
     * First we collect the return types. NTCexprs puts them into a product type
     * which is expected in INFO_NTC_TYPE( arg_info) afterwards (cf. NTCfundef)!
     * INFO_NTC_NUM_EXPRS_SOFAR is used to count the number of exprs "on the fly"!
     */
    INFO_NTC_NUM_EXPRS_SOFAR (arg_info) = 0;

    RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);

    DBUG_ASSERT (TYIsProd (INFO_NTC_TYPE (arg_info)),
                 "NTCexprs did not create a product type");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCap(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCap (node *arg_node, node *arg_info)
{
    node *fundef;
    DBUG_ENTER ("NTCap");

    /*
     * First we collect the argument types. NTCexprs puts them into a product type
     * which is expected in INFO_NTC_TYPE( arg_info) afterwards!
     * INFO_NTC_NUM_EXPRS_SOFAR is used to count the number of exprs "on the fly"!
     */
    INFO_NTC_NUM_EXPRS_SOFAR (arg_info) = 0;

    if (NULL != AP_ARGS (arg_node)) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    } else {
        INFO_NTC_TYPE (arg_info) = TYMakeProductType (0);
    }

    DBUG_ASSERT (TYIsProd (INFO_NTC_TYPE (arg_info)),
                 "NTCexprs did not create a product type");

    /*
     * At this point, we have to distinguish two fundamentally different
     * situations:
     * -  either all argument types are fixed; then we try to infer the exact
     *    result shape!!
     * -  or at least one argument type is not yet fixed! In this case we
     *    are dealing with a recursive call, which means that we have to
     *    postpone the inference and try to go one with freshly introduced
     *    result type variables!
     */
    if (TYIsProdOfArrayOrFixedAlpha (INFO_NTC_TYPE (arg_info))) {
        /*
         * all argument types are fixed:
         */
        fundef = DispatchFunction (AP_FUNDEF (arg_node), INFO_NTC_TYPE (arg_info));
        if (FUNDEF_TCSTAT (fundef) == NTC_not_checked) {
            fundef = TypeCheckFunctionBody (fundef, arg_info);
        }
        if (TYIsProdOfArrayOrFixedAlpha (FUNDEF_RET_TYPE (fundef))) {
            INFO_NTC_TYPE (arg_info) = FUNDEF_RET_TYPE (fundef);
        } else {
#if 0
      DBUG_ASSERT( 0, "TypeCheckFunctionBody did not yield a fixed return type!");
#endif
        }

    } else {
        /*
         * At least one argument type is not yet fixed:
         *
         * we push this function application on the postponed stack and
         * generate a product type with fresh type variables for the result!
         */
        fundef = AP_FUNDEF (arg_node); /* wrapper or LaC function !*/
        INFO_NTC_TYPE (arg_info) = TYDeriveSubtype (FUNDEF_RET_TYPE (fundef));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCexprs(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCexprs (node *arg_node, node *arg_info)
{
    ntype *type = NULL;

    DBUG_ENTER ("NTCexprs");

    if (NULL != EXPRS_EXPR (arg_node)) {
        EXPRS_EXPR (arg_node) = Trav (EXPRS_EXPR (arg_node), arg_info);
        type = INFO_NTC_TYPE (arg_info);
    }
    INFO_NTC_NUM_EXPRS_SOFAR (arg_info)++;

    if (NULL != EXPRS_NEXT (arg_node)) {
        EXPRS_NEXT (arg_node) = Trav (EXPRS_NEXT (arg_node), arg_info);
    } else {
        INFO_NTC_TYPE (arg_info)
          = TYMakeEmptyProductType (INFO_NTC_NUM_EXPRS_SOFAR (arg_info));
    }

    INFO_NTC_NUM_EXPRS_SOFAR (arg_info)--;
    INFO_NTC_TYPE (arg_info)
      = TYSetProductMember (INFO_NTC_TYPE (arg_info), INFO_NTC_NUM_EXPRS_SOFAR (arg_info),
                            type);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCarray(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCarray (node *arg_node, node *arg_info)
{
    int num_elems;
    ntype *scalar = NULL;
    ntype *tmp;
    ntype *type;
    int i;

    DBUG_ENTER ("NTCarray");

    /*
     * First we collect the element types. NTCexprs puts them into a product type
     * which is expected in INFO_NTC_TYPE( arg_info) afterwards!
     * INFO_NTC_NUM_EXPRS_SOFAR is used to count the number of exprs "on the fly"!
     */
    INFO_NTC_NUM_EXPRS_SOFAR (arg_info) = 0;

    if (NULL != ARRAY_AELEMS (arg_node)) {
        ARRAY_AELEMS (arg_node) = Trav (ARRAY_AELEMS (arg_node), arg_info);
    } else {
        INFO_NTC_TYPE (arg_info) = TYMakeProductType (0);
    }

    DBUG_ASSERT (TYIsProd (INFO_NTC_TYPE (arg_info)),
                 "NTCexprs did not create a product type");

    /*
     * Now, we built the resulting (AKS-)type type from the product type found:
     */
    num_elems = TYGetProductSize (INFO_NTC_TYPE (arg_info));
    if (num_elems > 0) {
        /*
         * We are dealing with a non-empty array here. So we can derive
         * the element type from the first element found.
         * Then, we check that all elems are of the same type "scalar".
         * Note here, that "scalar" points (should point) to an AKS-type with
         * an empty shape!!
         */
        scalar = TYGetProductMember (INFO_NTC_TYPE (arg_info), 0);
        for (i = 1; i < num_elems; i++) {
            tmp = TYGetProductMember (INFO_NTC_TYPE (arg_info), i);
            if (TYCmpTypes (scalar, tmp) != TY_eq) {
                ABORT (NODE_LINE (arg_node),
                       ("Different element types used in one array"));
            }
            TYFreeType (tmp);
        }

        type = TYMakeAKS (TYGetScalar (scalar), SHCreateShape (1, num_elems));
        TYFreeTypeConstructor (scalar);
        TYFreeTypeConstructor (INFO_NTC_TYPE (arg_info));
    } else {
        /* we are dealing with an empty array here! */
        type = NULL;
        DBUG_ASSERT ((0), "empty arrays are not yet supported!");
    }

    INFO_NTC_TYPE (arg_info) = type;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCid( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
NTCid (node *arg_node, node *arg_info)
{
    ntype *type;

    DBUG_ENTER ("NTCid");

    type = AVIS_TYPE (ID_AVIS (arg_node));

    if (type == NULL) {
        ABORT (
          NODE_LINE (arg_node),
          ("Cannot infer type for %s as it may be used without a previous definition",
           ID_NAME (arg_node)));
    } else {
        INFO_NTC_TYPE (arg_info) = TYCopyType (type);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCnum( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

#define NTCBASIC(name, base)                                                             \
    node *NTC##name (node *arg_node, node *arg_info)                                     \
    {                                                                                    \
                                                                                         \
        DBUG_ENTER ("NTC" #name);                                                        \
                                                                                         \
        INFO_NTC_TYPE (arg_info)                                                         \
          = TYMakeAKS (TYMakeSimpleType (base), SHCreateShape (0));                      \
        DBUG_RETURN (arg_node);                                                          \
    }

NTCBASIC (num, T_int)
NTCBASIC (double, T_double)
NTCBASIC (float, T_float)
NTCBASIC (char, T_char)
NTCBASIC (bool, T_bool)
