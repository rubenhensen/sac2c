/*
 *
 * $Log$
 * Revision 3.78  2005/07/15 17:34:50  sah
 * fixed a DBUG_PRINT
 *
 * Revision 3.77  2005/07/15 15:57:02  sah
 * introduced namespaces
 *
 * Revision 3.76  2005/06/14 09:55:10  sbs
 * support for bottom types integrated.
 *
 * Revision 3.75  2005/06/06 13:22:05  jhb
 * added usage of PHrunCompilerSubPhase
 *
 * Revision 3.74  2005/05/31 19:26:12  sah
 * moved usertype handling to resolvesymboltypes.c
 *
 * Revision 3.73  2005/05/24 08:34:33  sbs
 * some log messages eliminated
 *
 * Revision 3.72  2005/04/12 11:07:26  sah
 * fixed call of TEmakeInfo
 *
 * Revision 3.71  2005/03/19 23:16:24  sbs
 * AUD AUD support requires traversal of CODE_NEXT
 *
 * Revision 3.70  2005/02/16 22:29:13  sah
 * fixed handling of external funs
 *
 * Revision 3.69  2005/01/10 17:27:06  cg
 * Converted error messages from Error.h to ctinfo.c
 *
 * Revision 3.68  2004/12/19 15:48:27  sbs
 * calls to TNTdoToNewTypes eliminated
 *
 * Revision 3.67  2004/12/06 12:55:30  sbs
 * some debugging of NTCarg
 *
 * Revision 3.66  2004/12/05 19:19:55  sbs
 * return type of LaC funs changed into alphas.
 *
 * Revision 3.65  2004/12/03 18:10:29  sbs
 * funcond rep eliminated EXPRS
 *
 * Revision 3.64  2004/12/01 18:43:28  sah
 * renamed a function
 *
 * Revision 3.63  2004/11/30 15:24:34  sah
 * added a fix to handle CODE_CEXPRS correctly
 *
 * Revision 3.62  2004/11/27 05:07:00  ktr
 * bugfix
 *
 * Revision 3.61  2004/11/27 02:31:32  cg
 * Function name corrected
 *
 * Revision 3.60  2004/11/27 01:34:45  jhb
 * fixed bug
 *
 * Revision 3.59  2004/11/27 00:23:44  sbs
 * *** empty log message ***
 *
 * Revision 3.58  2004/11/25 17:52:55  sbs
 * compiles
 *
 * Revision 3.57  2004/11/24 17:42:48  sbs
 * not yet
 *
 * Revision 3.56  2004/11/23 21:49:39  cg
 * brushed usage of genlib
 * min_array_rep_t turned into enum type.
 *
 * Revision 3.55  2004/11/19 21:05:39  sah
 * removed some unused code
 *
 * Revision 3.54  2004/11/19 10:16:28  sah
 * removed T_classtype
 *
 * Revision 3.53  2004/11/18 14:34:31  mwe
 * changed CheckAvis and chkavis to ToNewTypes and to tonewtypes
 *
 * Revision 3.52  2004/11/14 15:20:16  sah
 * added support for import of udts
 *
 * Revision 3.51  2004/11/11 15:20:30  sah
 * in new ast mode a typedef now has a ntype after parsing
 * changed NTCtypedef accordingly
 *
 * ... [eliminated] ....
 *
 * Revision 1.1  1999/10/20 12:51:11  sbs
 * Initial revision
 *
 */

#define NEW_INFO

#include <stdio.h>
#include <string.h>
#include "dbug.h"
#include "ctinfo.h"

#include "types.h"
#include "phase.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "DupTree.h"
#include "globals.h"
#include "print.h"
#include "shape.h"
#include "insert_vardec.h"
#include "create_wrappers.h"
#include "create_wrapper_code.h"
#include "new2old.h"
#include "import_specialization.h"

#include "user_types.h"
#include "new_types.h"
#include "type_utils.h"
#include "sig_deps.h"
#include "ssi.h"
#include "new_typecheck.h"
#include "ct_basic.h"
#include "ct_fun.h"
#include "ct_prf.h"
#include "ct_with.h"
#include "type_errors.h"
#include "type_utils.h"
#include "specialize.h"
#include "constants.h"
#include "deserialize.h"
#include "namespaces.h"
#include "resolvesymboltypes.h"

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

/**
 *
 * @defgroup ntc Type Inference
 *
 * This group contains all those files/ modules that belong to the
 * (new) type inference system.
 *
 * @{
 */

/**
 *
 * @file new_typecheck.c
 *
 * This file contains the central type inference / type checking mechanism
 * of the compiler. It relies on support from several other modules.
 * These are:
 *   ... to be continued
 */

/*******************************************************************************
 * Thus, we finally find the following usages of the arg_info node:
 *
 *    INFO_NTC_TYPE             the inferred type of the expression traversed
 *    INFO_NTC_NUM_EXPRS_SOFAR  is used to count the number of exprs while
 *                              traversing them
 * ...
 */

/**
 * INFO structure
 */
struct INFO {
    ntype *type;
    ntype *gen_type;
    int num_exprs_sofar;
    node *last_assign;
    node *ptr_return;
    node *ptr_objdefs;
};

/**
 * INFO macros
 */
#define INFO_NTC_TYPE(n) (n->type)
#define INFO_NTC_GEN_TYPE(n) (n->gen_type)
#define INFO_NTC_NUM_EXPRS_SOFAR(n) (n->num_exprs_sofar)
#define INFO_NTC_LAST_ASSIGN(n) (n->last_assign)
#define INFO_NTC_RETURN(n) (n->ptr_return)
#define INFO_NTC_OBJDEFS(n) (n->ptr_objdefs)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_NTC_TYPE (result) = NULL;
    INFO_NTC_GEN_TYPE (result) = NULL;
    INFO_NTC_NUM_EXPRS_SOFAR (result) = 0;
    INFO_NTC_LAST_ASSIGN (result) = NULL;
    INFO_NTC_RETURN (result) = NULL;
    INFO_NTC_OBJDEFS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

typedef enum { NTC_not_checked, NTC_checking, NTC_checked } NTC_stat;

/**
 *
 * @name Entry functions for calling the type inference:
 *
 * @{
 */

/******************************************************************************
 *
 * function:
 *    node *NTCdoNewTypeCheck( node *arg_node)
 *
 * description:
 *    starts the new type checking traversal!
 *
 ******************************************************************************/

node *
NTCdoNewTypeCheck (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("NTCdoNewTypeCheck");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "NTCdoNewTypeCheck() not called with N_module node!");

#ifndef NEW_AST
    /*
     * if compiling for a c library, search for specializations
     * of functions and integrate them.
     *
     * NB: this is copied from the old TC; may not work properly
     * at all, i.e., needs attention 8-)
     *
     */
    if (global.genlib.c) {
        arg_node = ImportSpecialization (arg_node);
    }
#endif

    TRAVpush (TR_ntc);

    arg_info = MakeInfo ();
    arg_node = TRAVdo (arg_node, arg_info);
    arg_info = FreeInfo (arg_info);

    TRAVpop ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCNewTypeCheck_Expr( node *arg_node)
 *
 * description:
 *    Infers the type of an expression and fixes/eliminates alpha types.
 *    This function should be used *after* the regular type check phase only!
 *
 ******************************************************************************/

ntype *
NTCnewTypeCheck_Expr (node *arg_node)
{
    info *arg_info;
    ntype *type;

    DBUG_ENTER ("NTCnewTypeCheck_Expr");

    TRAVpush (TR_ntc);

    arg_info = MakeInfo ();
    arg_node = TRAVdo (arg_node, arg_info);
    type = INFO_NTC_TYPE (arg_info);
    type = TYfixAndEliminateAlpha (type);
    arg_info = FreeInfo (arg_info);

    TRAVpop ();

    DBUG_RETURN (type);
}

/* @} */

/**
 *
 * @name Local helper functions:
 *
 * @{
 */

/******************************************************************************
 *
 * function:
 *    node *TypeCheckFunctionBody( node *fundef, info *arg_info)
 *
 * description:
 *    main function for type checking a given fundef node.
 *
 ******************************************************************************/

static node *
TypeCheckFunctionBody (node *fundef, info *arg_info)
{
    ntype *spec_type, *inf_type;
    ntype *stype, *itype;
    int i, inf_n, spec_n;
    bool ok;
#ifndef DBUG_OFF
    char *tmp_str;
    node *arg;
#endif

    DBUG_ENTER ("TypeCheckFunctionBody");

    FUNDEF_TCSTAT (fundef) = NTC_checking;

    DBUG_PRINT ("NTC", ("type checking function \"%s\" with", CTIitemName (fundef)));

    /**
     * First, we have to ensure that ALL return types are in fact type vars.
     * Normal functions have been adjusted by create_wrappers.
     * LaC funs have been introduced by lac2fun with return types unknown[*].
     * These have to be replaced by PURE type vars, i.e., without any upper
     * limit!
     */
    if (FUNDEF_ISLACFUN (fundef)) {
        FUNDEF_RETS (fundef) = TUrettypes2alpha (FUNDEF_RETS (fundef));
    }

    DBUG_EXECUTE (
      "NTC", arg = FUNDEF_ARGS (fundef);

      while (arg != NULL) {
          tmp_str = TYtype2String (AVIS_TYPE (ARG_AVIS (arg)), FALSE, 0);
          DBUG_PRINT ("NTC", ("  -> argument type: %s", tmp_str));
          tmp_str = ILIBfree (tmp_str);
          arg = ARG_NEXT (arg);
      } tmp_str
      = TYtype2String (TUmakeProductTypeFromRets (FUNDEF_RETS (fundef)), FALSE, 0);
      DBUG_PRINT ("NTC", ("  -> return type %s", tmp_str));
      tmp_str = ILIBfree (tmp_str););

    /*
     * Then, we infer the type of the body:
     */
    if (NULL != FUNDEF_BODY (fundef)) {
        FUNDEF_BODY (fundef) = TRAVdo (FUNDEF_BODY (fundef), arg_info);

        /*
         * A pointer to the return node is available in INFO_NTC_RETURN( arg_info)
         * now (cf. NTCreturn).
         */

        FUNDEF_RETURN (fundef) = INFO_NTC_RETURN (arg_info);
        INFO_NTC_RETURN (arg_info) = NULL;

    } else {
        DBUG_ASSERT (FUNDEF_ISEXTERN (fundef),
                     "non external function with NULL body found but not expected here!");
        /*
         * We simply accept the type found in the external. declaration here:
         */
        INFO_NTC_TYPE (arg_info) = TUmakeProductTypeFromRets (FUNDEF_RETS (fundef));

        DBUG_PRINT ("NTC", ("trusting imported return type"));
    }

    /*
     * The inferred result type is now available in INFO_NTC_TYPE( arg_info).
     * Iff legal, we insert it into the specified result type.
     */

    inf_type = INFO_NTC_TYPE (arg_info);
    inf_n = TYgetProductSize (inf_type);

    DBUG_EXECUTE ("NTC", tmp_str = TYtype2String (inf_type, FALSE, 0););
    DBUG_PRINT ("NTC",
                ("inferred return type of \"%s\" is %s", FUNDEF_NAME (fundef), tmp_str));
    DBUG_EXECUTE ("NTC", tmp_str = ILIBfree (tmp_str););

    spec_type = TUmakeProductTypeFromRets (FUNDEF_RETS (fundef));
    spec_n = TYgetProductSize (spec_type);

    if ((spec_n > inf_n) || ((spec_n < inf_n) && !FUNDEF_HASDOTRETS (fundef))) {
        CTIabortLine (NODE_LINE (fundef),
                      "Number of return expressions in function \"%s\" does not match"
                      " the number of return types specified",
                      FUNDEF_NAME (fundef));
    }

    for (i = 0; i < TYgetProductSize (spec_type); i++) {
        stype = TYgetProductMember (spec_type, i);
        itype = TYgetProductMember (inf_type, i);

        ok = SSInewTypeRel (itype, stype);

        if (!ok) {
            CTIabortLine (NODE_LINE (fundef),
                          "Component #%d of inferred return type (%s) is not within %s",
                          i, TYtype2String (itype, FALSE, 0),
                          TYtype2String (stype, FALSE, 0));
        }
        /**
         * Now, we check whether we could infer at least one approximation for each
         * return value of a function. However, we have to make sure that this is a
         * top-level tc-run, since runs that are triggered during tc of other functions
         * may lack approximations due to mutual recursion.
         *
         * Example:
         *
         * bool foo( bool a)
         * {
         *   if(  a) {
         *     res = foo( goo( a));
         *   } else {
         *     res = false;
         *   }
         *   return( res);
         * }
         *
         * bool goo( bool a)
         * {
         *   res = foo( _not_(a));
         *   return( res);
         * }
         *
         * When TC goo during TC foo, we will not get an approximation yet, as the
         * else branch in foo has not yet been seen.
         *
         * we are in fact in the top-level function iff (global.act_info_chn == NULL)
         * holds!
         */
        if ((global.act_info_chn == NULL) && TYisAlpha (stype)
            && (SSIgetMin (TYgetAlpha (stype)) == NULL)) {
            CTIabortLine (NODE_LINE (fundef),
                          "Component #%d of inferred return type (%s) has no lower bound;"
                          " an application of \"%s\" will not terminate",
                          i, TYtype2String (stype, FALSE, 0), FUNDEF_NAME (fundef));
        }
    }
    TYfreeType (inf_type);
    INFO_NTC_TYPE (arg_info) = NULL;

    DBUG_EXECUTE ("NTC", tmp_str = TYtype2String (spec_type, FALSE, 0););
    DBUG_PRINT ("NTC",
                ("final return type of \"%s:%s\" is: %s", CTIitemName (fundef), tmp_str));
    DBUG_EXECUTE ("NTC", tmp_str = ILIBfree (tmp_str););

    /* now the functions is entirely typechecked, so we mark it as checked */
    FUNDEF_TCSTAT (fundef) = NTC_checked;

    DBUG_RETURN (fundef);
}

/* @} */

/**
 *
 * @name Traversal functions for the type inference system:
 *
 * @{
 */

/******************************************************************************
 *
 * function:
 *    node *NTCmodule(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCmodule (node *arg_node, info *arg_info)
{
    bool ok;
    node *fundef, *ignore;

    DBUG_ENTER ("NTCmodule");
    /*
     * First, we gather all typedefs and setup the global table
     * which is kept in "new_types". Furthermore, the symbol types
     * generated during scanparse are replaced by the proper
     * udts
     */
    arg_node = RSTdoResolveSymbolTypes (arg_node);
    if ((global.break_after == PH_typecheck)
        && (0 == strcmp (global.break_specifier, "rst"))) {
        goto DONE;
    }

    /*
     * now we check for consistency and try to gather the basetypes
     */
    if (NULL != MODULE_TYPES (arg_node)) {
        MODULE_TYPES (arg_node) = TRAVdo (MODULE_TYPES (arg_node), arg_info);
    }
    DBUG_EXECUTE ("UDT", UTprintRepository (stderr););

    CTIabortOnError ();

    /*
     * Then we insert the topmost objdef into the arg_info node
     * for later reference.
     */

    INFO_NTC_OBJDEFS (arg_info) = MODULE_OBJS (arg_node);

    /*
     * Before doing the actual type inference, we want to switch to the FUN-
     * representation. This requires several preparational steps:
     *
     * I) Inserting Vardecs
     * =====================
     *
     * First, we insert a vardec node for each identifier used.
     * This has to be done prior to Lac2Fun as Lac2Fun uses dataflowmasks
     * which in turn rely on the existance of Vardecs. Note here, that the
     * vardecs do not contain any type info yet!
     */
    arg_node = INSVDdoInsertVardec (arg_node);
    if ((global.break_after == PH_typecheck)
        && (0 == strcmp (global.break_specifier, "ivd"))) {
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
    arg_node = CRTWRPdoCreateWrappers (arg_node);
    if ((global.break_after == PH_typecheck)
        && (0 == strcmp (global.break_specifier, "cwr"))) {
        goto DONE;
    }

    /*
     * Now that all ids have backrefs to vardecs and all funaps have backrefs
     * to wrapper-fundefs, Lac2Fun can finally be run.
     */
    arg_node = PHrunCompilerSubPhase (SUBPH_lac2fun, arg_node);
    arg_node = PHrunCompilerSubPhase (SUBPH_ssa, arg_node);

    /*
     * Now, we do the actual type inference ....
     *
     * First, we mark all functions that need to be checked as NTC_not_checked.
     * All others are marked NTC_checked:
     */

    ok = SSIinitAssumptionSystem (SDhandleContradiction, SDhandleElimination);
    DBUG_ASSERT (ok, "Initialisation of Assumption System went wrong!");

    ignore = SPECresetSpecChain ();

    /**
     * Before starting the type checking mechanism, we first mark all
     * non-wrapper functions as NTC_not_checked,   AND
     * we do have to run NTC on all argument nodes of user defined
     * functions. One might think that this could be done when
     * "TypeCheckFunctionBody" is run ( and in fact this was our first
     * attempt [see comment in that function] 8-), BUT,
     * the problem here is that TYDispatchFuntype
     * in some rare cases (for detecting some function shadowing...)
     * wants to inspect the new type nodes behind the argument N_avis's.
     * So these have to be present even if the function has not yet been
     * type checked.
     */
    fundef = MODULE_FUNS (arg_node);
    while (fundef != NULL) {
        if (!FUNDEF_ISWRAPPERFUN (fundef)) {
#ifndef NEW_AST
            /* the new ast is already marked properly, so there is no need
             * to do so here
             */
            FUNDEF_TCSTAT (fundef) = NTC_not_checked;
#endif
            if (!FUNDEF_ISLACFUN (fundef) && (NULL != FUNDEF_ARGS (fundef))) {
                FUNDEF_ARGS (fundef) = TRAVdo (FUNDEF_ARGS (fundef), arg_info);
            }
        } else {
            FUNDEF_TCSTAT (fundef) = NTC_checked;
        }
        fundef = FUNDEF_NEXT (fundef);
    }

    /*
     * Now we have to initialize the deserialisation unit, as
     * specializations may add new functions as dependencies
     * of bodies to the ast
     */
    DSinitDeserialize (arg_node);

    if (NULL != MODULE_FUNDECS (arg_node)) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    if (NULL != MODULE_FUNS (arg_node)) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    if ((global.break_after == PH_typecheck)
        && (0 == strcmp (global.break_specifier, "ntc"))) {
        goto DONE;
    }

    /*
     * from here on, no more functions are deserialized, so ww can
     * finish the deseralization engine
     */
    DSfinishDeserialize (arg_node);

    /*
     * Finally, we compute the old type representation from the ntypes
     * we just inferred.
     */

    arg_node = NT2OTdoTransform (arg_node);
    if ((global.break_after == PH_typecheck)
        && (0 == strcmp (global.break_specifier, "n2o"))) {
        goto DONE;
    }

    /*
     * Now, we create SAC code for all wrapper functions
     */

    arg_node = CWCdoCreateWrapperCode (arg_node);
    if ((global.break_after == PH_typecheck)
        && (0 == strcmp (global.break_specifier, "cwc"))) {
        goto DONE;
    }

    /*
     * Convert the wrapper function code into SSA form
     */

    arg_node = PHrunCompilerSubPhase (SUBPH_lac2funwc, arg_node);

    arg_node = PHrunCompilerSubPhase (SUBPH_ssawc, arg_node);

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
NTCcheckUdtAndSetBaseType (usertype udt, int *visited)
{
    ntype *base, *base_elem;
    usertype inner_udt;
    ntype *inner_base;
    ntype *new_base, *new_base_elem;
    int num_udt, i;

    DBUG_ENTER ("NTCcheckUdtandSetBaseType");

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
                        inner_base = NTCcheckUdtAndSetBaseType (inner_udt, visited);
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
 *    node *NTCtypedef(node *arg_node, info *arg_info)
 *
 * description:
 *   We check on consistency (for the exact restrictions
 *   see "CheckUdtAndSetBaseType") and replace the defining type by its
 *   basetype.
 *
 ******************************************************************************/

node *
NTCtypedef (node *arg_node, info *arg_info)
{
    ntype *base;
    usertype udt;

    DBUG_ENTER ("NTCtypedef");

    udt = UTfindUserType (TYPEDEF_NAME (arg_node), TYPEDEF_NS (arg_node));

    if (TYPEDEF_ISLOCAL (arg_node)) {
        base = NTCcheckUdtAndSetBaseType (udt, NULL);
    } else {
        base = UTgetBaseType (udt);
    }

    /*
    TYPEDEF_NTYPE( arg_node) = TYfreeType( TYPEDEF_NTYPE( arg_node));
    */
    TYPEDEF_NTYPE (arg_node) = base;

    if (TYPEDEF_NEXT (arg_node) != NULL)
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCfundef(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCfundef (node *arg_node, info *arg_info)
{
    node *specialized_fundefs;

    DBUG_ENTER ("NTCfundef");

    if ((FUNDEF_TCSTAT (arg_node) == NTC_not_checked) && !FUNDEF_ISLACFUN (arg_node)) {
        /**
         * we are checking a new function; therefore, the actual
         * info chain (kept for extended error messages only) is reset:
         */
        global.act_info_chn = NULL;
        DBUG_PRINT ("NTC_INFOCHN", ("global.act_info_chn reset to NULL"));
        arg_node = TypeCheckFunctionBody (arg_node, arg_info);
    }

    if (NULL != FUNDEF_NEXT (arg_node)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        specialized_fundefs = SPECresetSpecChain ();
        if (specialized_fundefs != NULL) {
            FUNDEF_NEXT (arg_node) = specialized_fundefs;
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCarg(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCarg (node *arg_node, info *arg_info)
{
    ntype *type;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("NTCarg");

    type = ARG_NTYPE (arg_node);
    if (TUisArrayOfUser (type) && TUisUniqueUserType (type) && !ARG_ISREFERENCE (arg_node)
        && !ARG_ISARTIFICIAL (arg_node)) {

        DBUG_EXECUTE ("UNQ", tmp_str = TYtype2String (type, FALSE, 0););
        DBUG_PRINT ("UNQ", ("argument \"%s\" of type \"%s\" marked as unique",
                            ARG_NAME (arg_node), tmp_str));
        DBUG_EXECUTE ("UNQ", tmp_str = ILIBfree (tmp_str););

        AVIS_ISUNIQUE (ARG_AVIS (arg_node)) = TRUE;
        if (!TYisAKS (type) || (TYisAKS (type) && TYgetDim (type) != 0)) {
            CTIerrorLine (NODE_LINE (arg_node),
                          "Unique type \"%s\" used in non-scalar form",
                          TYtype2String (TYgetScalar (type), FALSE, 0));
        }
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * NTCblock( node *arg_node, info *arg_info )
 *
 * description:
 *
 ******************************************************************************/

node *
NTCblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("NTCblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * NTCvardec( node *arg_node, info *arg_info )
 *
 * description:
 *
 ******************************************************************************/

node *
NTCvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("NTCvardec");

    if (AVIS_TYPE (VARDEC_AVIS (arg_node)) != NULL) {
        /**
         * this means that the vardec has been created by duplicating a type-
         * checked function. Since this may require slightly different (e.g.
         * more specific) types to be inferred, we simply eliminate the existing
         * ones!
         */
        AVIS_TYPE (VARDEC_AVIS (arg_node))
          = TYfreeType (AVIS_TYPE (VARDEC_AVIS (arg_node)));
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCassign(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
NTCassign (node *arg_node, info *arg_info)
{
    node *tmp;

    DBUG_ENTER ("NTCassign");

    tmp = INFO_NTC_LAST_ASSIGN (arg_info);

    INFO_NTC_LAST_ASSIGN (arg_info) = arg_node;
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    INFO_NTC_LAST_ASSIGN (arg_info) = tmp;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCcond(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
NTCcond (node *arg_node, info *arg_info)
{
    ntype *args;
    ntype *res;
    info *context_info;
    te_info *info;

    DBUG_ENTER ("NTCcond");

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    args = TYmakeProductType (1, INFO_NTC_TYPE (arg_info));

    context_info = MakeInfo ();
    INFO_NTC_LAST_ASSIGN (context_info) = INFO_NTC_LAST_ASSIGN (arg_info);

    info = TEmakeInfo (global.linenum, "cond", "", "", arg_node, (node *)context_info,
                       NULL, NULL);

    res = NTCCTcomputeType (NTCCTcond, info, args);

    args = TYfreeType (args);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCfuncond(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
NTCfuncond (node *arg_node, info *arg_info)
{
    node *rhs1, *rhs2;
    ntype *rhs1_type, *rhs2_type, *res;
    bool ok;

#ifndef DBUG_OFF
    char *tmp_str, *tmp2_str;
#endif

    DBUG_ENTER ("NTCfuncond");

    rhs1 = FUNCOND_THEN (arg_node);
    rhs2 = FUNCOND_ELSE (arg_node);

    /**
     * collect the first phi-type => rhs1_type!
     *
     * Since this usually is a variable that is defined within a conditional
     * (might be constant due to const propagation though) it may not have been
     * assigned a proper type yet. Therefore, we cannot simply traverse the rhs,
     * but we may have to insert a type variable instead. However, this may have
     * happened due to a vardec already...
     */
    if (NODE_TYPE (rhs1) == N_id) {
        rhs1_type = AVIS_TYPE (ID_AVIS (rhs1));
        if (rhs1_type == NULL) {
            rhs1_type = TYmakeAlphaType (NULL);
            AVIS_TYPE (ID_AVIS (rhs1)) = rhs1_type;
        }
    } else {
        PRF_ARG1 (LET_EXPR (arg_node)) = TRAVdo (rhs1, arg_info);
        rhs1_type = INFO_NTC_TYPE (arg_info);
    }

    /**
     * collect the second phi-type => rhs2_type!
     */
    if (NODE_TYPE (rhs2) == N_id) {
        rhs2_type = AVIS_TYPE (ID_AVIS (rhs2));
        if (rhs2_type == NULL) {
            rhs2_type = TYmakeAlphaType (NULL);
            AVIS_TYPE (ID_AVIS (rhs2)) = rhs2_type;
        }
    } else {
        PRF_ARG1 (LET_EXPR (arg_node)) = TRAVdo (rhs2, arg_info);
        rhs2_type = INFO_NTC_TYPE (arg_info);
    }

    /**
     * Now, we compute the result type, i.e., lub( rhs1_type, rhs2_type)
     */
    res = TYmakeAlphaType (NULL);

    DBUG_EXECUTE ("NTC", tmp_str = TYtype2String (res, FALSE, 0);
                  tmp2_str = TYtype2String (rhs1_type, FALSE, 0););
    DBUG_PRINT ("NTC", ("  making %s bigger than %s", tmp_str, tmp2_str));
    DBUG_EXECUTE ("NTC", tmp_str = ILIBfree (tmp_str); tmp2_str = ILIBfree (tmp_str););

    ok = SSInewTypeRel (rhs1_type, res);

    DBUG_EXECUTE ("NTC", tmp_str = TYtype2String (res, FALSE, 0);
                  tmp2_str = TYtype2String (rhs2_type, FALSE, 0););
    DBUG_PRINT ("NTC", ("  making %s bigger than %s", tmp_str, tmp2_str));
    DBUG_EXECUTE ("NTC", tmp_str = ILIBfree (tmp_str); tmp2_str = ILIBfree (tmp_str););

    ok = ok && SSInewTypeRel (rhs2_type, res);

    if (!ok) {
        CTIabortLine (global.linenum, "Nasty type error");
    }

    INFO_NTC_TYPE (arg_info) = res;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTClet(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
NTClet (node *arg_node, info *arg_info)
{
    ntype *rhs_type, *existing_type, *inferred_type, *max;
    node *lhs;
    int i;
    bool ok;

#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("NTClet");

    /*
     * Infer the RHS type :
     */
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    rhs_type = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    /**
     * attach the RHS type(s) to the var(s) on the LHS:
     *
     * However, the LHS may have a type already. This can be due to
     * a) a vardec
     * b) an funcond node that combines this var with another one in another
     *    branch of a conditional!
     * In both cases, we add the RHS type as a new <= constraint!
     */
    lhs = LET_IDS (arg_node);

    if ((NODE_TYPE (LET_EXPR (arg_node)) == N_ap)
        || (NODE_TYPE (LET_EXPR (arg_node)) == N_prf)) {
        if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap) {
            DBUG_ASSERT ((TCcountIds (lhs) >= TYgetProductSize (rhs_type)),
                         "fun ap yields more return values  than lhs vars available!");
        } else {
            if (TCcountIds (lhs) != 1) {
                CTIabortLine (global.linenum, "%s yields 1 instead of %d return values",
                              global.prf_string[PRF_PRF (LET_EXPR (arg_node))],
                              TCcountIds (lhs));
            }
        }
        i = 0;
        while (lhs) {
            existing_type = AVIS_TYPE (IDS_AVIS (lhs));
            if (i < TYgetProductSize (rhs_type)) {

                inferred_type = TYgetProductMember (rhs_type, i);

                if (existing_type == NULL) {
                    AVIS_TYPE (IDS_AVIS (lhs)) = inferred_type;
                } else {
                    DBUG_ASSERT (TYisAlpha (existing_type),
                                 "non-alpha type for LHS found!");
                    ok = SSInewTypeRel (inferred_type, existing_type);
                    if (!ok) {
                        CTIabortLine (NODE_LINE (arg_node),
                                      "Component #%d of inferred RHS type (%s) does not "
                                      "match %s",
                                      i, TYtype2String (inferred_type, FALSE, 0),
                                      TYtype2String (existing_type, FALSE, 0));
                    }
                }

                DBUG_EXECUTE ("NTC", tmp_str = TYtype2String (AVIS_TYPE (IDS_AVIS (lhs)),
                                                              FALSE, 0););
                DBUG_PRINT ("NTC", ("  type of \"%s\" is %s", IDS_NAME (lhs), tmp_str));
                DBUG_EXECUTE ("NTC", tmp_str = ILIBfree (tmp_str););
            } else {
                if (existing_type == NULL) {
                    CTIabortLine (global.linenum,
                                  "Cannot infer type of \"%s\" as it corresponds to "
                                  "\"...\" "
                                  "return type -- missing type declaration",
                                  IDS_NAME (lhs));
                } else {
                    /**
                     * ' commented out the following warning as it was issued to often
                    with
                     * StdIO; left it here as I do not know whether a warning in principle
                     * would be the better way to go for anyways....
                     *
                    CTIwarnLine( global.linenum,
                                 "Cannot infer type of \"%s\" as it corresponds to \"...\"
                    " "return type -- relying on type declaration", IDS_NAME( lhs));
                     */
                    DBUG_ASSERT (TYisAlpha (existing_type),
                                 "non-alpha type for LHS found!");
                    max = SSIgetMax (TYgetAlpha (existing_type));
                    DBUG_ASSERT (max != NULL, "null max for LHS type found!");
                    ok = SSInewMin (TYgetAlpha (existing_type), TYcopyType (max));
                }
            }

            i++;
            lhs = IDS_NEXT (lhs);
        }
        TYfreeTypeConstructor (rhs_type);
    } else {

        /* lhs must be one ids only since rhs is not a function application! */
        DBUG_ASSERT ((TCcountIds (lhs) == 1),
                     "more than one lhs var without a function call on the rhs");

        existing_type = AVIS_TYPE (IDS_AVIS (lhs));
        inferred_type = rhs_type;

        if (existing_type == NULL) {
            AVIS_TYPE (IDS_AVIS (lhs)) = inferred_type;
        } else {
            DBUG_ASSERT (TYisAlpha (existing_type), "non-alpha type for LHS found!");
            ok = SSInewTypeRel (inferred_type, existing_type);
            if (!ok) {
                CTIabortLine (NODE_LINE (arg_node),
                              "Inferred RHS type (%s) does not match %s",
                              TYtype2String (inferred_type, FALSE, 0),
                              TYtype2String (existing_type, FALSE, 0));
            }
        }

        DBUG_EXECUTE ("NTC",
                      tmp_str = TYtype2String (AVIS_TYPE (IDS_AVIS (lhs)), FALSE, 0););
        DBUG_PRINT ("NTC", ("  type of \"%s\" is %s", IDS_NAME (lhs), tmp_str));
        DBUG_EXECUTE ("NTC", tmp_str = ILIBfree (tmp_str););
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *NTCreturn(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCreturn (node *arg_node, info *arg_info)
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
        INFO_NTC_TYPE (arg_info) = TYmakeProductType (0);
    } else {
        RETURN_EXPRS (arg_node) = TRAVdo (RETURN_EXPRS (arg_node), arg_info);
    }

    DBUG_ASSERT (TYisProd (INFO_NTC_TYPE (arg_info)),
                 "NTCexprs did not create a product type");

    /*
     * Finally, we send the return node back to the fundef to fill FUNDEF_RETURN
     * properly !! We use INFO_NTC_RETURN( arg_info) for this purpose.
     */

    INFO_NTC_RETURN (arg_info) = arg_node;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCap(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCap (node *arg_node, info *arg_info)
{
    ntype *args, *res;
    node *wrapper;
    te_info *old_info_chn;

    DBUG_ENTER ("NTCap");

    /*
     * First we collect the argument types. NTCexprs puts them into a product type
     * which is expected in INFO_NTC_TYPE( arg_info) afterwards!
     * INFO_NTC_NUM_EXPRS_SOFAR is used to count the number of exprs "on the fly"!
     */
    INFO_NTC_NUM_EXPRS_SOFAR (arg_info) = 0;

    if (NULL != AP_ARGS (arg_node)) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    } else {
        INFO_NTC_TYPE (arg_info) = TYmakeProductType (0);
    }

    DBUG_ASSERT (TYisProd (INFO_NTC_TYPE (arg_info)),
                 "NTCexprs did not create a product type");

    args = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    wrapper = AP_FUNDEF (arg_node);
    old_info_chn = global.act_info_chn;
    global.act_info_chn
      = TEmakeInfo (global.linenum, "udf", NSgetName (FUNDEF_NS (wrapper)),
                    FUNDEF_NAME (wrapper), wrapper, INFO_NTC_LAST_ASSIGN (arg_info), NULL,
                    global.act_info_chn);
    DBUG_PRINT ("TEINFO",
                ("TE info %p created for udf ap %p", global.act_info_chn, arg_node));
    res = NTCCTcomputeType (NTCCTudf, global.act_info_chn, args);

    global.act_info_chn = old_info_chn;
    DBUG_PRINT ("NTC_INFOCHN", ("global.act_info_chn set back to %p", old_info_chn));

    TYfreeType (args);
    INFO_NTC_TYPE (arg_info) = res;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCprf(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCprf (node *arg_node, info *arg_info)
{
    ntype *args, *res;
    prf prf;
    te_info *info;

    DBUG_ENTER ("NTCprf");

    prf = PRF_PRF (arg_node);

    /*
     * First we collect the argument types. NTCexprs puts them into a product type
     * which is expected in INFO_NTC_TYPE( arg_info) afterwards!
     * INFO_NTC_NUM_EXPRS_SOFAR is used to count the number of exprs "on the fly"!
     */
    INFO_NTC_NUM_EXPRS_SOFAR (arg_info) = 0;

    if (NULL != PRF_ARGS (arg_node)) {
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
    } else {
        INFO_NTC_TYPE (arg_info) = TYmakeProductType (0);
    }

    DBUG_ASSERT (TYisProd (INFO_NTC_TYPE (arg_info)),
                 "NTCexprs did not create a product type");

    args = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    info = TEmakeInfo (global.linenum, "prf", "", global.prf_string[prf], NULL, NULL,
                       global.ntc_cffuntab[prf], NULL);
    res = NTCCTcomputeType (global.ntc_funtab[prf], info, args);

    TYfreeType (args);
    INFO_NTC_TYPE (arg_info) = res;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCexprs(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCexprs (node *arg_node, info *arg_info)
{
    ntype *type = NULL;

    DBUG_ENTER ("NTCexprs");

    if (NULL != EXPRS_EXPR (arg_node)) {
        EXPRS_EXPR (arg_node) = TRAVdo (EXPRS_EXPR (arg_node), arg_info);
        type = INFO_NTC_TYPE (arg_info);
    }
    INFO_NTC_NUM_EXPRS_SOFAR (arg_info)++;

    if (NULL != EXPRS_NEXT (arg_node)) {
        EXPRS_NEXT (arg_node) = TRAVdo (EXPRS_NEXT (arg_node), arg_info);
    } else {
        INFO_NTC_TYPE (arg_info)
          = TYmakeEmptyProductType (INFO_NTC_NUM_EXPRS_SOFAR (arg_info));
    }

    INFO_NTC_NUM_EXPRS_SOFAR (arg_info)--;
    INFO_NTC_TYPE (arg_info)
      = TYsetProductMember (INFO_NTC_TYPE (arg_info), INFO_NTC_NUM_EXPRS_SOFAR (arg_info),
                            type);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCarray(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCarray (node *arg_node, info *arg_info)
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
        ARRAY_AELEMS (arg_node) = TRAVdo (ARRAY_AELEMS (arg_node), arg_info);
    } else {
        INFO_NTC_TYPE (arg_info) = TYmakeProductType (0);
    }

    DBUG_ASSERT (TYisProd (INFO_NTC_TYPE (arg_info)),
                 "NTCexprs did not create a product type");

    elems = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    /*
     * Now, we built the resulting (AKS-)type type from the product type found:
     */
    num_elems = TYgetProductSize (elems);
    if (num_elems > 0) {

        info = TEmakeInfo (global.linenum, "prf", "", "array-constructor", NULL, NULL,
                           NULL, NULL);
        type = NTCCTcomputeType (NTCCTprf_array, info, elems);

        TYfreeType (elems);

    } else {
        /**
         * we are dealing with an empty array here!
         * To get started, we assume all empty arrays to be of type int[0].
         * If an other type is desired, it has to be casted to that type
         * (which - at the time being - is not yet supported 8-)
         */
        type
          = TYmakeProductType (1, TYmakeAKV (TYmakeSimpleType (T_int),
                                             COmakeConstant (T_int, SHcreateShape (1, 0),
                                                             NULL)));
    }

    INFO_NTC_TYPE (arg_info) = TYgetProductMember (type, 0);
    TYfreeTypeConstructor (type);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCid( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
NTCid (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ("NTCid");

    type = AVIS_TYPE (ID_AVIS (arg_node));

    if (type == NULL) {
        CTIabortLine (NODE_LINE (arg_node),
                      "Cannot infer type for %s as it may be"
                      " used without a previous definition",
                      ID_NAME (arg_node));
    } else {
        INFO_NTC_TYPE (arg_info) = TYcopyType (type);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *NTCglobobj( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
NTCglobobj (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ("NTCglobobj");

    type = OBJDEF_TYPE (GLOBOBJ_OBJDEF (arg_node));

    DBUG_ASSERT (type != NULL, "N_objdef wo type found in NTCglobobj");

    INFO_NTC_TYPE (arg_info) = TYcopyType (type);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NTCnum( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

#define NTCBASIC(name, base)                                                             \
    node *NTC##name (node *arg_node, info *arg_info)                                     \
    {                                                                                    \
        constant *cv;                                                                    \
        DBUG_ENTER ("NTC" #name);                                                        \
                                                                                         \
        cv = COaST2Constant (arg_node);                                                  \
        if (cv == NULL) {                                                                \
            INFO_NTC_TYPE (arg_info)                                                     \
              = TYmakeAKS (TYmakeSimpleType (base), SHcreateShape (0));                  \
        } else {                                                                         \
            INFO_NTC_TYPE (arg_info) = TYmakeAKV (TYmakeSimpleType (base), cv);          \
        }                                                                                \
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
 *   node *NTCcast( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NTCcast (node *arg_node, info *arg_info)
{
    te_info *info;
    ntype *type, *cast_t, *expr_t;

    DBUG_ENTER ("NTCcast");

    CAST_EXPR (arg_node) = TRAVdo (CAST_EXPR (arg_node), arg_info);
    expr_t = INFO_NTC_TYPE (arg_info);
    if (TYisProd (expr_t)) {
        /*
         * The expression we are dealing with here is a function application.
         * Therefore, only a single return type is legal. This one is to be extracted!
         */
        if (TYgetProductSize (expr_t) != 1) {
            CTIabortLine (global.linenum,
                          "Cast used for a function application with %d return values",
                          TYgetProductSize (expr_t));
        } else {
            expr_t = TYgetProductMember (expr_t, 0);
        }
    }
    cast_t = CAST_NTYPE (arg_node);

    info = TEmakeInfo (global.linenum, "prf", "", "type-cast", NULL, NULL, NULL, NULL);
    type = NTCCTcomputeType (NTCCTprf_cast, info, TYmakeProductType (2, cast_t, expr_t));

    INFO_NTC_TYPE (arg_info) = TYgetProductMember (type, 0);
    TYfreeTypeConstructor (type);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCwith( node *arg_node, info *arg_info)
 *
 * description:
 *   steers the type inference of with loops and dbug prints the individual
 *   findings of the generator inference, the body inference, and the
 *   composition of them which is done in NTCNwithop.
 *
 ******************************************************************************/

node *
NTCwith (node *arg_node, info *arg_info)
{
    ntype *gen, *body, *res, *tmp;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("NTCwith");

    /*
     * First, we infer the generator type
     */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    gen = TYgetProductMember (INFO_NTC_TYPE (arg_info), 0);
    TYfreeTypeConstructor (INFO_NTC_TYPE (arg_info));
    INFO_NTC_TYPE (arg_info) = NULL;

    DBUG_EXECUTE ("NTC", tmp_str = TYtype2String (gen, FALSE, 0););
    DBUG_PRINT ("NTC", ("  WL - generator type: %s", tmp_str));
    DBUG_EXECUTE ("NTC", tmp_str = ILIBfree (tmp_str););

    /*
     * Then, we infer the type of the WL body:
     */
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    body = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    /*
     * TODO: as the type of the body of a multi-operator withloop
     *       is a product type, we extract the first element.
     *       note that withloops with more than one operator are
     *       not allowed at this stage anyways.
     */

    DBUG_ASSERT ((TYisProd (body) && (TYgetProductSize (body) == 1)),
                 "body of withloop has none or too large product type");

    tmp = body;
    body = TYgetProductMember (body, 0);
    tmp = TYfreeTypeConstructor (tmp);

    DBUG_EXECUTE ("NTC", tmp_str = TYtype2String (body, FALSE, 0););
    DBUG_PRINT ("NTC", ("  WL - body type: %s", tmp_str));
    DBUG_EXECUTE ("NTC", tmp_str = ILIBfree (tmp_str););

    /*
     * Finally, we compute the return type from "gen" and "body".
     * This is done in NTCNwithop. The two types are transferred via
     * INFO_NTC_GEN_TYPE and INFO_NTC_TYPE, respectively.
     */
    INFO_NTC_GEN_TYPE (arg_info) = gen;
    INFO_NTC_TYPE (arg_info) = body;

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    res = TYgetProductMember (INFO_NTC_TYPE (arg_info), 0);
    TYfreeTypeConstructor (INFO_NTC_TYPE (arg_info));
    INFO_NTC_TYPE (arg_info) = res;

    DBUG_EXECUTE ("NTC", tmp_str = TYtype2String (INFO_NTC_TYPE (arg_info), FALSE, 0););
    DBUG_PRINT ("NTC", ("  WL - final type: %s", tmp_str));
    DBUG_EXECUTE ("NTC", tmp_str = ILIBfree (tmp_str););

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCpart( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NTCpart (node *arg_node, info *arg_info)
{
    node *idxs;
    ntype *idx;
    int num_ids;

    DBUG_ENTER ("NTCpart");

    /*
     * First, we check whether we can extract some shape info from the
     * generator variable, i.e, we check whether we do have scalar indices:
     */
    idxs = PART_IDS (arg_node);
    if (idxs != NULL) {
        num_ids = TCcountIds (idxs);
        idx = TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, num_ids));
    } else {
        idx = TYmakeAKD (TYmakeSimpleType (T_int), 1, SHcreateShape (0));
    }

    /*
     * Then, we infer the best possible type of the generator specification
     * and from the idx information gained from the Nwithid node:
     */
    INFO_NTC_TYPE (arg_info) = idx;
    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    /*
     * the best possible generator type is returned, so
     * we attach the generator type to the generator variable(s).
     */
    PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);

    /*
     * AND (!!) we hand the type back to NTCNwith!
     */
    DBUG_ASSERT (INFO_NTC_TYPE (arg_info) != NULL,
                 "inferred generator type corrupted in NTCNwithid");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCgenerator( node *arg_node, info *arg_info)
 *
 * description:
 *   checks compatability of the generator entries, i.e.,
 *              bounds, step, and width vectors
 *   if they are potentially conformative the most specific type possible is
 *   returned.
 *
 ******************************************************************************/

node *
NTCgenerator (node *arg_node, info *arg_info)
{
    ntype *lb, *idx, *ub, *s, *w, *gen, *res;
    te_info *info;

    DBUG_ENTER ("NTCgenerator");

    idx = INFO_NTC_TYPE (arg_info); /* generated in NTCNpart !*/
    INFO_NTC_TYPE (arg_info) = NULL;

    GENERATOR_BOUND1 (arg_node) = TRAVdo (GENERATOR_BOUND1 (arg_node), arg_info);
    lb = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    GENERATOR_BOUND2 (arg_node) = TRAVdo (GENERATOR_BOUND2 (arg_node), arg_info);
    ub = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    if (GENERATOR_STEP (arg_node) != NULL) {
        GENERATOR_STEP (arg_node) = TRAVdo (GENERATOR_STEP (arg_node), arg_info);
        s = INFO_NTC_TYPE (arg_info);
        INFO_NTC_TYPE (arg_info) = NULL;

        if (GENERATOR_WIDTH (arg_node) != NULL) {
            GENERATOR_WIDTH (arg_node) = TRAVdo (GENERATOR_WIDTH (arg_node), arg_info);
            w = INFO_NTC_TYPE (arg_info);
            INFO_NTC_TYPE (arg_info) = NULL;

            gen = TYmakeProductType (5, lb, idx, ub, s, w);
        } else {
            gen = TYmakeProductType (4, lb, idx, ub, s);
        }
    } else {
        gen = TYmakeProductType (3, lb, idx, ub);
    }

    info = TEmakeInfo (global.linenum, "wl", "", "generator", NULL, NULL, NULL, NULL);
    res = NTCCTcomputeType (NTCCTwl_idx, info, gen);
    TYfreeType (gen);

    INFO_NTC_TYPE (arg_info) = res;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCwithid( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NTCwithid (node *arg_node, info *arg_info)
{
    node *idxs, *vec;

    DBUG_ENTER ("NTCwithid");

    idxs = WITHID_IDS (arg_node);
    while (idxs) {
        /*
         * single genvars always have to be scalar int s
         */
        AVIS_TYPE (IDS_AVIS (idxs))
          = TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0));
        idxs = IDS_NEXT (idxs);
    }

    vec = WITHID_VEC (arg_node);
    if (vec != NULL) {
        /*
         * we have to leave the generator type intact, therefore, we copy it:
         */
        AVIS_TYPE (IDS_AVIS (vec))
          = TYcopyType (TYgetProductMember (INFO_NTC_TYPE (arg_info), 0));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCcode( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NTCcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("NTCcode");

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    /*
     * TODO: here we traverse into possibly a chain of exprs, so
     *       the result will be a product type! In NTCwith, we will
     *       throw away everythig except of the first element of that
     *       type. This is no nice abstraction...
     */
    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

    /**
     * traverse into defaut expressson iff existant
     */
    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *NTCgenarray( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
NTCgenarray (node *arg_node, info *arg_info)
{
    ntype *gen, *body, *res;
    ntype *shp, *dexpr, *args;
    te_info *info;

    DBUG_ENTER ("NTCNgenarray");

    gen = INFO_NTC_GEN_TYPE (arg_info);
    INFO_NTC_GEN_TYPE (arg_info) = NULL;
    body = INFO_NTC_TYPE (arg_info);

    /*
     * First, we check the shape expression:
     */
    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);
    shp = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;
    /*
     * Then, we check the shape of the default value, if available:
     */
    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
        dexpr = INFO_NTC_TYPE (arg_info);
        INFO_NTC_TYPE (arg_info) = NULL;
    } else {
        dexpr = TYcopyType (body);
    }

    args = TYmakeProductType (4, gen, shp, body, dexpr);
    info = TEmakeInfo (global.linenum, "with", "", "genarray", NULL, NULL, NULL, NULL);
    res = NTCCTcomputeType (NTCCTwl_gen, info, args);

    INFO_NTC_TYPE (arg_info) = res;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *NTCmodarray( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
NTCmodarray (node *arg_node, info *arg_info)
{
    ntype *gen, *body, *res;
    ntype *shp, *args;
    te_info *info;

    DBUG_ENTER ("NTCmodarray");

    gen = INFO_NTC_GEN_TYPE (arg_info);
    INFO_NTC_GEN_TYPE (arg_info) = NULL;
    body = INFO_NTC_TYPE (arg_info);

    /*
     * First, we check the array expression:
     */
    MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);
    shp = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    args = TYmakeProductType (3, gen, shp, body);
    info = TEmakeInfo (global.linenum, "with", "", "modarray", NULL, NULL, NULL, NULL);
    res = NTCCTcomputeType (NTCCTwl_mod, info, args);

    INFO_NTC_TYPE (arg_info) = res;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *NTCfold( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
NTCfold (node *arg_node, info *arg_info)
{
    ntype *gen, *body, *res, *elems, *acc;
    ntype *shp, *args;
    node *wrapper;
    te_info *info;
    bool ok;

    DBUG_ENTER ("NTCfold");

    gen = INFO_NTC_GEN_TYPE (arg_info);
    INFO_NTC_GEN_TYPE (arg_info) = NULL;
    body = INFO_NTC_TYPE (arg_info);

    if (FOLD_FUN (arg_node) == NULL) {
        /**
         * we are dealing with a prf-fold-wl here!
         */
        DBUG_ASSERT (FALSE, "fold WL with prf not yet implemented");
        res = NULL; /* just to please gcc 8-) */

    } else {
        /**
         * we are dealing with a udf-fold-wl here!
         *
         * First, we check the neutral expression:
         */
        if (FOLD_NEUTRAL (arg_node) == NULL) {
            CTIabortLine (global.linenum,
                          "Missing neutral element for user-defined fold function");
        }
        FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);
        shp = INFO_NTC_TYPE (arg_info);
        INFO_NTC_TYPE (arg_info) = NULL;

        /*
         * Then, we compute the type of the elements to be folded:
         */
        args = TYmakeProductType (2, shp, body);
        info = TEmakeInfo (global.linenum, "with", "", "fold", NULL, NULL, NULL, NULL);
        res = NTCCTcomputeType (NTCCTwl_fold, info, args);
        elems = TYgetProductMember (res, 0);
        res = TYfreeTypeConstructor (res);

        /*
         * Followed by a computation of the type of the fold fun:
         *
         * Since the entire fold-wl is a recursive function that in each
         * iteration applies the foldfun, we need to establish a fix-point
         * iteration here (cf. bug no.18).
         * To do so, we introduce a type variable for the accumulated parameter
         * which has to be bigger than a) the element type and b) the result
         * type.
         */
        acc = TYmakeAlphaType (NULL);
        ok = SSInewTypeRel (elems, acc);
        DBUG_ASSERT (ok, ("initialization of fold-fun in fold-wl went wrong"));

        args = TYmakeProductType (2, acc, elems);
        wrapper = FOLD_FUNDEF (arg_node);
        info = TEmakeInfo (global.linenum, "fold fun", NSgetName (FUNDEF_NS (wrapper)),
                           FUNDEF_NAME (wrapper), wrapper,
                           INFO_NTC_LAST_ASSIGN (arg_info), NULL, NULL);
        res = NTCCTcomputeType (NTCCTudf, info, args);

        ok = SSInewTypeRel (TYgetProductMember (res, 0), acc);
        if (!ok) {
            CTIabortLine (global.linenum, "Illegal fold function in fold with loop");
        }
    }

    INFO_NTC_TYPE (arg_info) = res;

    DBUG_RETURN (arg_node);
}

/* @} */

/**
 *
 * name: Module internal entry function:
 *
 * @{
 */

/******************************************************************************
 *
 * function:
 *    node *NTCtriggerTypeCheck( node *fundef)
 *
 * description:
 *   external interface for TypeCheckFunctionBody. It is used from ct_fun,
 *   where the generation of new specializations and the type inference
 *   of all potential instances of a function application is triggered.
 *
 ******************************************************************************/

node *
NTCtriggerTypeCheck (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ("NTCtriggerTypeCheck");

    if (FUNDEF_TCSTAT (fundef) == NTC_not_checked) {
        arg_info = MakeInfo ();
        fundef = TypeCheckFunctionBody (fundef, arg_info);
        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN (fundef);
}

/* @} */
/* @} */ /* defgroup ntc */
