/*
 *
 * $Log$
 * Revision 3.18  2002/09/05 12:05:23  dkr
 * -b7:n2o added
 *
 * Revision 3.17  2002/09/05 09:44:09  dkr
 * NewTypeCheck_Expr() modified
 *
 * Revision 3.16  2002/09/04 12:59:46  sbs
 * type checking of arrays changed; now sig deps will be created as well.
 *
 * Revision 3.15  2002/09/03 14:41:45  sbs
 * DupTree machanism for duplicating condi funs established
 *
 * Revision 3.14  2002/08/31 04:57:09  dkr
 * NewTypeCheck_Expr() added
 *
 * Revision 3.13  2002/08/15 20:59:24  dkr
 * Lac2Fun(), CheckAvis() added for wrapper code
 *
 * Revision 3.12  2002/08/14 16:22:21  dkr
 * SSATransform() after CreateWrapperCode() added
 *
 * Revision 3.11  2002/08/09 13:00:43  dkr
 * call of CreateWrapperCode() added
 *
 * Revision 3.10  2002/08/06 08:26:49  sbs
 * some vars initialized to please gcc for the product version.
 *
 * Revision 3.9  2002/08/05 17:00:38  sbs
 * first alpha version of the new type checker !!
 *
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
#include "DupTree.h"
#include "globals.h"
#include "lac2fun.h"
#include "CheckAvis.h"
#include "SSATransform.h"
#include "insert_vardec.h"
#include "create_wrappers.h"
#include "create_wrapper_code.h"
#include "new2old.h"

#include "user_types.h"
#include "new_types.h"
#include "sig_deps.h"
#include "new_typecheck.h"
#include "ct_basic.h"
#include "ct_fun.h"
#include "ct_prf.h"
#include "ct_with.h"
#include "type_errors.h"
#include "specialize.h"

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
#define INFO_NTC_GEN_TYPE(n) ((ntype *)(n->dfmask[1]))
#define INFO_NTC_NUM_EXPRS_SOFAR(n) (n->flag)
#define INFO_NTC_LAST_ASSIGN(n) (n->node[0])

typedef enum { NTC_not_checked, NTC_checking, NTC_checked } NTC_stat;

/******************************************************************************
 *
 * function:
 *    node *NewTypeCheck( node *arg_node)
 *
 * description:
 *    starts the new type checking traversal!
 *
 ******************************************************************************/

node *
NewTypeCheck (node *arg_node)
{
    funtab *tmp_tab;
    node *arg_info;

    DBUG_ENTER ("NewTypeCheck");

    tmp_tab = act_tab;
    act_tab = ntc_tab;

    arg_info = MakeInfo ();
    arg_node = Trav (arg_node, arg_info);
    arg_info = FreeNode (arg_info);

    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    ntype *NewTypeCheck_Expr( node *arg_node)
 *
 * description:
 *    Infers the type of an expression and fixes/eliminates alpha types.
 *    This function should be used *after* the regular type check phase only!
 *
 ******************************************************************************/

ntype *
NewTypeCheck_Expr (node *arg_node)
{
    funtab *tmp_tab;
    node *arg_info;
    ntype *type;

    DBUG_ENTER ("NewTypeCheck_Expr");

    tmp_tab = act_tab;
    act_tab = ntc_tab;

    arg_info = MakeInfo ();
    arg_node = Trav (arg_node, arg_info);
    type = INFO_NTC_TYPE (arg_info);
    type = TYFixAndEliminateAlpha (type);
    arg_info = FreeNode (arg_info);

    act_tab = tmp_tab;

    DBUG_RETURN (type);
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
 *    node *TypeCheckFunctionBody( node *fundef, node *arg_info)
 *
 * description:
 *    main function for type checking a given fundef node.
 *
 ******************************************************************************/

static node *
TypeCheckFunctionBody (node *fundef, node *arg_info)
{
    ntype *spec_type, *inf_type;
    ntype *stype, *itype;
    int i;
    bool ok;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("TypeCheckFunctionBody");

    FUNDEF_TCSTAT (fundef) = NTC_checking;

    DBUG_EXECUTE ("NTC", tmp_str = TYType2String (FUNDEF_RET_TYPE (fundef), FALSE, 0););
    DBUG_PRINT ("NTC", ("type checking function \"%s\" return type %s",
                        FUNDEF_NAME (fundef), tmp_str));
    DBUG_EXECUTE ("NTC", tmp_str = Free (tmp_str););

    /*
     * First, we convert the old argument types into ntype nodes and attach
     * these to the AVIS_TYPE fields of the N_arg nodes:
     */
    if (NULL != FUNDEF_ARGS (fundef)) {
        FUNDEF_ARGS (fundef) = Trav (FUNDEF_ARGS (fundef), arg_info);
    }

    /*
     * Then, we infer the type of the body:
     */
    if (NULL != FUNDEF_BODY (fundef)) {
        FUNDEF_BODY (fundef) = Trav (FUNDEF_BODY (fundef), arg_info);

        /*
         * The inferred result type is now available in INFO_NTC_TYPE( arg_info).
         * Iff legal, we insert it into the specified result type:
         */

        inf_type = INFO_NTC_TYPE (arg_info);

        DBUG_EXECUTE ("NTC", tmp_str = TYType2String (inf_type, FALSE, 0););
        DBUG_PRINT ("NTC", ("inferred return type of \"%s\" is %s", FUNDEF_NAME (fundef),
                            tmp_str));
        DBUG_EXECUTE ("NTC", tmp_str = Free (tmp_str););

        spec_type = FUNDEF_RET_TYPE (fundef);

        for (i = 0; i < TYGetProductSize (spec_type); i++) {
            stype = TYGetProductMember (spec_type, i);
            itype = TYGetProductMember (inf_type, i);

            ok = SSINewTypeRel (itype, stype);

            if (!ok) {
                ABORT (NODE_LINE (fundef),
                       ("component #%d of inferred return type (%s) is not within %s", i,
                        TYType2String (itype, FALSE, 0),
                        TYType2String (stype, FALSE, 0)));
            }
        }
        TYFreeType (inf_type);
        INFO_NTC_TYPE (arg_info) = NULL;

        DBUG_EXECUTE ("NTC", tmp_str = TYType2String (spec_type, FALSE, 0););
        DBUG_PRINT ("NTC", ("final return type of \"%s\" is: %s", FUNDEF_NAME (fundef),
                            tmp_str));
        DBUG_EXECUTE ("NTC", tmp_str = Free (tmp_str););
    }
    DBUG_RETURN (fundef);
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
    bool ok;
    node *fundef, *ignore;

    DBUG_ENTER ("NTCmodul");
    /*
     * First, we gather all typedefs and setup the global table
     * which is kept in "new_types".
     */
    if (NULL != MODUL_TYPES (arg_node)) {
        MODUL_TYPES (arg_node) = Trav (MODUL_TYPES (arg_node), arg_info);
    }
    DBUG_EXECUTE ("UDT", UTPrintRepository (stderr););
    ABORT_ON_ERROR;

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
     * application of a user defined function to contain a backref to the
     * function that is actually applied. Since this -in general- cannot be
     * statically decided, the backref points to a wrapper function which
     * contains the (intersection type based) function type of the overloaded
     * function as well as pointers to all potential implementations. These
     * structures are created by "CreateWrappers".
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

    ok = SSIInitAssumptionSystem (SDHandleContradiction, SDHandleElimination);
    DBUG_ASSERT (ok, "Initialisation of Assumption System went wrong!");

    ignore = SPECResetSpecChain ();

    fundef = MODUL_FUNS (arg_node);
    while (fundef != NULL) {
        if (FUNDEF_STATUS (fundef) != ST_wrapperfun) {
            FUNDEF_TCSTAT (fundef) = NTC_not_checked;
        } else {
            FUNDEF_TCSTAT (fundef) = NTC_checked;
        }
        fundef = FUNDEF_NEXT (fundef);
    }

    if (NULL != MODUL_FUNS (arg_node)) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    if ((break_after == PH_typecheck) && (0 == strcmp (break_specifier, "ntc"))) {
        goto DONE;
    }

    /*
     * Now, we create SAC code for all wrapper functions
     */

    arg_node = CreateWrapperCode (arg_node);
    if ((break_after == PH_typecheck) && (0 == strcmp (break_specifier, "cwc"))) {
        goto DONE;
    }

    /*
     * Finally, we compute the old type representation from the ntypes
     * we just inferred.
     */

    arg_node = NT2OTTransform (arg_node);
    if ((break_after == PH_typecheck) && (0 == strcmp (break_specifier, "n2o"))) {
        goto DONE;
    }

    /*
     * Convert the wrapper function code into SSA form
     */
    arg_node = Lac2Fun (arg_node);
    arg_node = CheckAvis (arg_node);
    arg_node = SSATransform (arg_node);

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
            ERROR (linenum, ("typedef of %s:%s is illegal; should be either"
                             " scalar type or array type of fixed shape",
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
 *   see "CheckUdtAndSetBaseType") and replace the defining type by its
 *   basetype.
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
    nt = TYOldType2Type (TYPEDEF_TYPE (arg_node));

    udt = UTFindUserType (name, mod);
    if (udt != UT_NOT_DEFINED) {
        ERROR (linenum, ("type %s:%s multiply defined;"
                         " previous definition in line %d",
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

node *
NTCfundef (node *arg_node, node *arg_info)
{
    node *specialized_fundefs;

    DBUG_ENTER ("NTCfundef");

    if ((FUNDEF_TCSTAT (arg_node) == NTC_not_checked) && !FUNDEF_IS_LACFUN (arg_node)) {
        arg_node = TypeCheckFunctionBody (arg_node, arg_info);
    }

    if (NULL != FUNDEF_NEXT (arg_node)) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        specialized_fundefs = SPECResetSpecChain ();
        if (specialized_fundefs != NULL) {
            FUNDEF_NEXT (arg_node) = specialized_fundefs;
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

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

    AVIS_TYPE (ARG_AVIS (arg_node)) = TYOldType2Type (ARG_TYPE (arg_node));

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

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
     * The vardecs are ignored so far.....
     */

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCassign(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
NTCassign (node *arg_node, node *arg_info)
{
    node *tmp;

    DBUG_ENTER ("NTCassign");

    tmp = INFO_NTC_LAST_ASSIGN (arg_info);

    INFO_NTC_LAST_ASSIGN (arg_info) = arg_node;
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    INFO_NTC_LAST_ASSIGN (arg_info) = tmp;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCcond(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
NTCcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("NTCcond");

    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);

    TEAssureBoolS ("predicate", INFO_NTC_TYPE (arg_info));

    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

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
    node *avis;
    ids *lhs;
    int i;
    bool ok;
#ifndef DBUG_OFF
    char *tmp_str, *tmp2_str;
#endif

    DBUG_ENTER ("NTClet");

    /*
     * Infer the RHS type :
     */
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    rhs_type = INFO_NTC_TYPE (arg_info);

    /*
     * attach the RHS type(s) to the var(s) on the LHS:
     */
    lhs = LET_IDS (arg_node);

    if ((NODE_TYPE (LET_EXPR (arg_node)) == N_ap)
        || (NODE_TYPE (LET_EXPR (arg_node)) == N_prf)) {
        DBUG_ASSERT ((CountIds (lhs) == TYGetProductSize (rhs_type)),
                     "fun ap does not match no of lhs vars!");
        i = 0;
        while (lhs) {
            AVIS_TYPE (IDS_AVIS (lhs)) = TYGetProductMember (rhs_type, i);

            DBUG_EXECUTE ("NTC", tmp_str
                                 = TYType2String (AVIS_TYPE (IDS_AVIS (lhs)), FALSE, 0););
            DBUG_PRINT ("NTC", ("  type of \"%s\" is %s", IDS_NAME (lhs), tmp_str));
            DBUG_EXECUTE ("NTC", tmp_str = Free (tmp_str););

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
        if (AVIS_SSAPHITARGET (avis) == PHIT_NONE) {
            AVIS_TYPE (avis) = rhs_type;

            DBUG_EXECUTE ("NTC", tmp_str = TYType2String (AVIS_TYPE (avis), FALSE, 0););
            DBUG_PRINT ("NTC", ("  type of \"%s\" is %s", IDS_NAME (lhs), tmp_str));
            DBUG_EXECUTE ("NTC", tmp_str = Free (tmp_str););

        } else {
            /*
             * we are dealing with a PHI target here:
             */
            if (AVIS_TYPE (avis) == NULL) {
                AVIS_TYPE (avis) = TYMakeAlphaType (NULL);

                DBUG_EXECUTE ("NTC",
                              tmp_str = TYType2String (AVIS_TYPE (avis), FALSE, 0););
                DBUG_PRINT ("NTC", ("  type of \"%s\" is %s", IDS_NAME (lhs), tmp_str));
                DBUG_EXECUTE ("NTC", tmp_str = Free (tmp_str););
            }

            DBUG_EXECUTE ("NTC", tmp_str = TYType2String (AVIS_TYPE (avis), FALSE, 0););
            DBUG_EXECUTE ("NTC", tmp2_str = TYType2String (rhs_type, FALSE, 0););
            DBUG_PRINT ("NTC", ("  making %s bigger than %s", tmp_str, tmp2_str));
            DBUG_EXECUTE ("NTC", tmp_str = Free (tmp_str););
            DBUG_EXECUTE ("NTC", tmp2_str = Free (tmp_str););

            ok = SSINewTypeRel (rhs_type, AVIS_TYPE (avis));
            if (!ok) {
                ABORT (linenum, ("nasty type error"));
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

    if (RETURN_EXPRS (arg_node) == NULL) {
        /* we are dealing with a void function here! */
        INFO_NTC_TYPE (arg_info) = TYMakeProductType (0);
    } else {
        RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);
    }

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
    ntype *args, *res;
    node *wrapper;
    te_info *info;

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

    args = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    wrapper = AP_FUNDEF (arg_node);
    info = TEMakeInfo (linenum, "udf", FUNDEF_NAME (wrapper), wrapper,
                       INFO_NTC_LAST_ASSIGN (arg_info));
    res = NTCCTComputeType (NTCFUN_udf, info, args);

    TYFreeType (args);
    INFO_NTC_TYPE (arg_info) = res;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCprf(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCprf (node *arg_node, node *arg_info)
{
    ntype *args, *res;
    prf prf;
    te_info *info;

    DBUG_ENTER ("NTCprf");

    /*
     * First we collect the argument types. NTCexprs puts them into a product type
     * which is expected in INFO_NTC_TYPE( arg_info) afterwards!
     * INFO_NTC_NUM_EXPRS_SOFAR is used to count the number of exprs "on the fly"!
     */
    INFO_NTC_NUM_EXPRS_SOFAR (arg_info) = 0;

    if (NULL != PRF_ARGS (arg_node)) {
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
    } else {
        INFO_NTC_TYPE (arg_info) = TYMakeProductType (0);
    }

    DBUG_ASSERT (TYIsProd (INFO_NTC_TYPE (arg_info)),
                 "NTCexprs did not create a product type");

    args = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    prf = PRF_PRF (arg_node);
    info = TEMakeInfo (linenum, "prf", prf_name_str[prf], NULL, NULL);
    res = NTCCTComputeType (NTCPRF_funtab[prf], info, args);

    TYFreeType (args);
    INFO_NTC_TYPE (arg_info) = res;

    DBUG_RETURN (arg_node);

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
    ntype *type, *elems;
    te_info *info;

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

    elems = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    /*
     * Now, we built the resulting (AKS-)type type from the product type found:
     */
    num_elems = TYGetProductSize (elems);
    if (num_elems > 0) {

        info = TEMakeInfo (linenum, "prf", "array-constructor", NULL, NULL);
        type = NTCCTComputeType (NTCPRF_array, info, elems);

        TYFreeType (elems);

    } else {
        /* we are dealing with an empty array here! */
        type = NULL;
        DBUG_ASSERT ((0), "empty arrays are not yet supported!");
    }

    INFO_NTC_TYPE (arg_info) = TYGetProductMember (type, 0);
    TYFreeTypeConstructor (type);

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
        ABORT (NODE_LINE (arg_node), ("Cannot infer type for %s as it may be"
                                      " used without a previous definition",
                                      ID_NAME (arg_node)));
    } else {
        INFO_NTC_TYPE (arg_info) = TYCopyType (type);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NTCnum( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

#define NTCBASIC(name, base)                                                             \
    node *NTC##name (node *arg_node, node *arg_info)                                     \
    {                                                                                    \
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

/******************************************************************************
 *
 * function:
 *    node *NTCNwith( node *arg_node, node *arg_info)
 *
 * description:
 *   steers the type inference of with loops and dbug prints the individual
 *   findings of the generator inference, the body inference, and the
 *   composition of them which is done in NTCNwithop.
 *
 ******************************************************************************/

node *
NTCNwith (node *arg_node, node *arg_info)
{
    ntype *gen, *body, *res;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("NTCNwith");

    /*
     * First, we infer the generator type
     */
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    gen = TYGetProductMember (INFO_NTC_TYPE (arg_info), 0);
    TYFreeTypeConstructor (INFO_NTC_TYPE (arg_info));
    INFO_NTC_TYPE (arg_info) = NULL;

    DBUG_EXECUTE ("NTC", tmp_str = TYType2String (gen, FALSE, 0););
    DBUG_PRINT ("NTC", ("  WL - generator type: %s", tmp_str));
    DBUG_EXECUTE ("NTC", tmp_str = Free (tmp_str););

    /*
     * Then, we infer the type of the WL body:
     */
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    body = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    DBUG_EXECUTE ("NTC", tmp_str = TYType2String (body, FALSE, 0););
    DBUG_PRINT ("NTC", ("  WL - body type: %s", tmp_str));
    DBUG_EXECUTE ("NTC", tmp_str = Free (tmp_str););

    /*
     * Finally, we compute the return type from "gen" and "body".
     * This is done in NTCNwithop. The two types are transferred via
     * INFO_NTC_GEN_TYPE and INFO_NTC_TYPE, respectively.
     */
    INFO_NTC_GEN_TYPE (arg_info) = gen;
    INFO_NTC_TYPE (arg_info) = body;

    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    res = TYGetProductMember (INFO_NTC_TYPE (arg_info), 0);
    TYFreeTypeConstructor (INFO_NTC_TYPE (arg_info));
    INFO_NTC_TYPE (arg_info) = res;

    DBUG_EXECUTE ("NTC", tmp_str = TYType2String (INFO_NTC_TYPE (arg_info), FALSE, 0););
    DBUG_PRINT ("NTC", ("  WL - final type: %s", tmp_str));
    DBUG_EXECUTE ("NTC", tmp_str = Free (tmp_str););

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCNpart( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NTCNpart (node *arg_node, node *arg_info)
{
    ids *idxs;
    ntype *idx;
    int num_ids;

    DBUG_ENTER ("NTCNpart");

    /*
     * First, we check whether we can extract some shape info from the
     * generator variable, i.e, we check whether we do have scalar indices:
     */
    idxs = NPART_IDS (arg_node);
    if (idxs != NULL) {
        num_ids = CountIds (idxs);
        idx = TYMakeAKS (TYMakeSimpleType (T_int), SHCreateShape (1, num_ids));
    } else {
        idx = TYMakeAKD (TYMakeSimpleType (T_int), 1, SHCreateShape (0));
    }

    /*
     * Then, we infer the best possible type of the generator specification
     * and from the idx information gained from the Nwithid node:
     */
    INFO_NTC_TYPE (arg_info) = idx;
    NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);

    /*
     * the best possible generator type is returned, so
     * we attach the generator type to the generator variable(s).
     */
    NPART_WITHID (arg_node) = Trav (NPART_WITHID (arg_node), arg_info);

    /*
     * AND (!!) we hand the type back to NTCNwith!
     */
    DBUG_ASSERT (INFO_NTC_TYPE (arg_info) != NULL,
                 "inferred generator type corrupted in NTCNwithid");

    DBUG_ASSERT ((NPART_NEXT (arg_node) == NULL),
                 "with-loop with more than one part found!");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCNgenerator( node *arg_node, node *arg_info)
 *
 * description:
 *   checks compatability of the generator entries, i.e.,
 *              bounds, step, and width vectors
 *   if they are potentially conformative the most specific type possible is
 *   returned.
 *
 ******************************************************************************/

node *
NTCNgenerator (node *arg_node, node *arg_info)
{
    ntype *lb, *idx, *ub, *s, *w, *gen, *res;
    te_info *info;

    DBUG_ENTER ("NTCNgenerator");

    idx = INFO_NTC_TYPE (arg_info); /* generated in NTCNpart !*/
    INFO_NTC_TYPE (arg_info) = NULL;

    NGEN_BOUND1 (arg_node) = Trav (NGEN_BOUND1 (arg_node), arg_info);
    lb = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    NGEN_BOUND2 (arg_node) = Trav (NGEN_BOUND2 (arg_node), arg_info);
    ub = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    if (NGEN_STEP (arg_node) != NULL) {
        NGEN_STEP (arg_node) = Trav (NGEN_STEP (arg_node), arg_info);
        s = INFO_NTC_TYPE (arg_info);
        INFO_NTC_TYPE (arg_info) = NULL;

        if (NGEN_WIDTH (arg_node) != NULL) {
            NGEN_WIDTH (arg_node) = Trav (NGEN_WIDTH (arg_node), arg_info);
            w = INFO_NTC_TYPE (arg_info);
            INFO_NTC_TYPE (arg_info) = NULL;

            gen = TYMakeProductType (5, lb, idx, ub, s, w);
        } else {
            gen = TYMakeProductType (4, lb, idx, ub, s);
        }
    } else {
        gen = TYMakeProductType (3, lb, idx, ub);
    }

    info = TEMakeInfo (linenum, "wl", "generator", NULL, NULL);
    res = NTCCTComputeType (NTCWL_idx, info, gen);
    TYFreeType (gen);

    INFO_NTC_TYPE (arg_info) = res;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCNwithid( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NTCNwithid (node *arg_node, node *arg_info)
{
    ids *idxs, *vec;

    DBUG_ENTER ("NTCNwithid");

    idxs = NWITHID_IDS (arg_node);
    while (idxs) {
        /*
         * single genvars always have to be scalar int s
         */
        AVIS_TYPE (IDS_AVIS (idxs))
          = TYMakeAKS (TYMakeSimpleType (T_int), SHCreateShape (0));
        idxs = IDS_NEXT (idxs);
    }

    vec = NWITHID_VEC (arg_node);
    if (vec != NULL) {
        /*
         * we have to leave the generator type intact, therefore, we copy it:
         */
        AVIS_TYPE (IDS_AVIS (vec))
          = TYCopyType (TYGetProductMember (INFO_NTC_TYPE (arg_info), 0));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCNcode( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NTCNcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("NTCNcode");

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);

    DBUG_ASSERT ((NCODE_NEXT (arg_node) == NULL),
                 "with-loop with more than one code block found!");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCNwithop( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NTCNwithop (node *arg_node, node *arg_info)
{
    ntype *gen, *body, *res, *elems, *fold_res;
    ntype *shp, *args;
    node *wrapper;
    te_info *info;
    bool ok;

    DBUG_ENTER ("NTCNwithop");

    gen = INFO_NTC_GEN_TYPE (arg_info);
    INFO_NTC_GEN_TYPE (arg_info) = NULL;
    body = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        /*
         * First, we check the shape expression:
         */
        NWITHOP_SHAPE (arg_node) = Trav (NWITHOP_SHAPE (arg_node), arg_info);
        shp = INFO_NTC_TYPE (arg_info);
        INFO_NTC_TYPE (arg_info) = NULL;

        args = TYMakeProductType (3, gen, shp, body);
        info = TEMakeInfo (linenum, "with", "genarray", NULL, NULL);
        res = NTCCTComputeType (NTCWL_gen, info, args);

        break;

    case WO_modarray:
        /*
         * First, we check the array expression:
         */
        NWITHOP_ARRAY (arg_node) = Trav (NWITHOP_ARRAY (arg_node), arg_info);
        shp = INFO_NTC_TYPE (arg_info);
        INFO_NTC_TYPE (arg_info) = NULL;

        args = TYMakeProductType (3, gen, shp, body);
        info = TEMakeInfo (linenum, "with", "modarray", NULL, NULL);
        res = NTCCTComputeType (NTCWL_mod, info, args);

        break;

    case WO_foldfun:
        /*
         * First, we check the neutral expression:
         */
        if (NWITHOP_NEUTRAL (arg_node) == NULL) {
            ABORT (linenum, ("missing neutral element for user-defined fold function"));
        }
        NWITHOP_NEUTRAL (arg_node) = Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        shp = INFO_NTC_TYPE (arg_info);
        INFO_NTC_TYPE (arg_info) = NULL;

        /*
         * Then, we compute the type of the elements to be folded:
         */
        args = TYMakeProductType (2, shp, body);
        info = TEMakeInfo (linenum, "with", "fold", NULL, NULL);
        res = NTCCTComputeType (NTCWL_fold, info, args);
        elems = TYGetProductMember (res, 0);

        /*
         * Followed by a computation of the type of the fold fun:
         */
        args = TYMakeProductType (2, TYCopyType (elems), TYCopyType (elems));
        wrapper = NWITHOP_FUNDEF (arg_node);
        info = TEMakeInfo (linenum, "fold fun", FUNDEF_NAME (wrapper), wrapper,
                           INFO_NTC_LAST_ASSIGN (arg_info));
        fold_res = NTCCTComputeType (NTCFUN_udf, info, args);

        ok = SSINewTypeRel (TYGetProductMember (fold_res, 0), elems);

        if (!ok) {
            ABORT (linenum, ("illegal fold function in fold with loop; "));
        }
        break;

    case WO_foldprf:
        /* here is no break missing */
    default:
        DBUG_ASSERT (FALSE, "fold WL with prf not yet implemented");
        res = NULL; /* just to please gcc 8-) */
    }
    INFO_NTC_TYPE (arg_info) = res;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCTriggerTypeCheck( node *fundef)
 *
 * description:
 *   external interface for TypeCheckFunctionBody. It is used from ct_fun,
 *   where the generation of new specializations and the type inference
 *   of all potential instances of a function application is triggered.
 *
 ******************************************************************************/

node *
NTCTriggerTypeCheck (node *fundef)
{
    node *arg_info;

    DBUG_ENTER ("NTCTriggerTypeCheck");

    if (FUNDEF_TCSTAT (fundef) == NTC_not_checked) {
        arg_info = MakeInfo ();
        fundef = TypeCheckFunctionBody (fundef, arg_info);
        arg_info = FreeNode (arg_info);
    }

    DBUG_RETURN (fundef);
}
