/*
 * $Id$
 */

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
#include "split_wrappers.h"
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
#include "map_call_graph.h"

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
    ntype *accu;
};

/**
 * INFO macros
 */
#define INFO_NTC_TYPE(n) (n->type)
#define INFO_NTC_GEN_TYPE(n) (n->gen_type)
#define INFO_NTC_NUM_EXPRS_SOFAR(n) (n->num_exprs_sofar)
#define INFO_NTC_LAST_ASSIGN(n) (n->last_assign)
#define INFO_NTC_RETURN(n) (n->ptr_return)
#define INFO_NTC_EXP_ACCU(n) (n->accu)

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
    INFO_NTC_EXP_ACCU (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

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
    bool ok;
    node *fundef;
    node *ignore;

    DBUG_ENTER ("NTCdoNewTypeCheck");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "NTCdoNewTypeCheck() not called with N_module node!");

    ok = SSIinitAssumptionSystem (SDhandleContradiction, SDhandleElimination);
    DBUG_ASSERT (ok, "Initialisation of Assumption System went wrong!");

    ignore = SPECresetSpecChain ();

    /**
     * Before starting the type checking mechanism, we first mark all
     * wrapper functions as NTC_checked (as these have no bodies).
     * For all other functions, we rely on FUNDEF_TCSTAT being set
     * properly. This is done by the TBmakeFundef function and
     * the module system (for imported/used functions).
     */
    fundef = MODULE_FUNS (arg_node);
    while (fundef != NULL) {
        if (FUNDEF_ISWRAPPERFUN (fundef)) {
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

#if 0
  /*
   * if compiling for a c library, search for specializations
   * of functions and integrate them.
   *
   * NB: this is copied from the old TC; may not work properly
   * at all, i.e., needs attention 8-)
   *
   */
  if (global.genlib.c) {
    arg_node = ImportSpecialization(arg_node);
  }
#endif

    TRAVpush (TR_ntc);

    arg_info = MakeInfo ();
    arg_node = TRAVdo (arg_node, arg_info);
    arg_info = FreeInfo (arg_info);

    TRAVpop ();

    /*
     * from here on, no more functions are deserialized, so we can
     * finish the deseralization engine
     */
    DSfinishDeserialize (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *NTCdoNewTypeCheckOneFunction( node *arg_node)
 *
 *****************************************************************************/

static node *
TagAsUnchecked (node *fundef, info *info)
{
    DBUG_ENTER ("TagAsUnchecked");

    FUNDEF_TCSTAT (fundef) = NTC_not_checked;

    DBUG_RETURN (fundef);
}

node *
NTCdoNewTypeCheckOneFunction (node *arg_node)
{
    ntype *old_rets = NULL, *new_rets = NULL;

    DBUG_ENTER ("NTCdoNewTypeCheckOneFunction");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 "NTCdoNewTypeCheckOneFunction can only be applied to N_fundef");

    if (!FUNDEF_ISWRAPPERFUN (arg_node) && !FUNDEF_ISLACFUN (arg_node)
        && (FUNDEF_BODY (arg_node) != NULL)) {
        int oldmaxspec;
        node *oldfundefnext;
        info *arg_info;

        /*
         * De-activate specialising
         */
        oldmaxspec = global.maxspec;
        global.maxspec = 0;

        /*
         * Rescue FUNDEF_NEXT
         */
        oldfundefnext = FUNDEF_NEXT (arg_node);
        FUNDEF_NEXT (arg_node) = NULL;

        /*
         * Apply typechecker
         */
        MCGdoMapCallGraph (arg_node, TagAsUnchecked, NULL, MCGcontLacFun, NULL);
        arg_node = TagAsUnchecked (arg_node, NULL);

        if (FUNDEF_RETS (arg_node) != NULL) {
            /**
             * collect the old return types for later comparison
             */
            old_rets = TUmakeProductTypeFromRets (FUNDEF_RETS (arg_node));

            FUNDEF_RETS (arg_node) = TUrettypes2alphaMax (FUNDEF_RETS (arg_node));
        }

        TRAVpush (TR_ntc);

        arg_info = MakeInfo ();
        arg_node = TRAVdo (arg_node, arg_info);
        arg_info = FreeInfo (arg_info);

        TRAVpop ();

        /*
         * Apple mysterious n2o traversal
         */
        arg_node = NT2OTdoTransformOneFunction (arg_node);

        /**
         * check for return type upgrades:
         */
        if (FUNDEF_RETS (arg_node) != NULL) {
            new_rets = TUmakeProductTypeFromRets (FUNDEF_RETS (arg_node));
            FUNDEF_WASUPGRADED (arg_node) = !TYeqTypes (old_rets, new_rets);
            old_rets = TYfreeType (old_rets);
            new_rets = TYfreeType (new_rets);
        } else {
            FUNDEF_WASUPGRADED (arg_node) = FALSE;
        }

        /**
         * increase global optimisation counter
         * if function was upgraded
         */
        if (FUNDEF_WASUPGRADED (arg_node)) {
            global.optcounters.tup_upgrades++;
        }

        /*
         * Restore FUNDEF_NEXT and global.maxspec
         */
        global.maxspec = oldmaxspec;
        FUNDEF_NEXT (arg_node) = oldfundefnext;
    }

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
        FUNDEF_RETS (fundef) = TUrettypes2alphaMax (FUNDEF_RETS (fundef));
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
                     "non external function with NULL body found"
                     " but not expected here!");

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
                ("final return type of \"%s\" is: %s", CTIitemName (fundef), tmp_str));
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
    DBUG_ENTER ("NTCmodule");

    if (NULL != MODULE_FUNDECS (arg_node)) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    if (NULL != MODULE_FUNS (arg_node)) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

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

        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }
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

#if 0
  type = ARG_NTYPE( arg_node);
  if( TUisArrayOfUser( type) && TUisUniqueUserType( type)
      && !ARG_ISREFERENCE(arg_node) && !ARG_ISARTIFICIAL(arg_node) ) {
  
    DBUG_EXECUTE( "UNQ", tmp_str = TYtype2String( type, FALSE, 0););
    DBUG_PRINT( "UNQ", ("argument \"%s\" of type \"%s\" marked as unique",
                       ARG_NAME( arg_node), 
                       tmp_str));
    DBUG_EXECUTE( "UNQ", tmp_str = ILIBfree( tmp_str););
  
    AVIS_ISUNIQUE( ARG_AVIS(arg_node))=TRUE;
    if( !TYisAKS( type)
        || (TYisAKS( type) && TYgetDim( type) != 0)) {
      CTIerrorLine( NODE_LINE( arg_node),
                    "Unique type \"%s\" used in non-scalar form",
                    TYtype2String( TYgetScalar(type), FALSE, 0));
    }
  }
#endif

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
 *  existing types are converted into type vars with upper bounds!
 *
 ******************************************************************************/

node *
NTCvardec (node *arg_node, info *arg_info)
{
    ntype *type;
    DBUG_ENTER ("NTCvardec");

    type = AVIS_TYPE (VARDEC_AVIS (arg_node));
    if (type != NULL) {
        /**
         * this means that the vardec has been created
         *  (a) through a variable declaration   or
         *  (b) by duplicating a type-checked function   or
         *  (c) by an earlier run of the type-checker
         * In ALL these cases, we want to make sure that only more specific
         * types can be inferred!
         * While in (a) and (c) the type is fixed, in (b) it may be an alpha!
         *     TUtype2alphaMax( type);
         * would do that job. However, this precludes us from recognizing
         * uses of non-defined vars when running the first time!
         * Hence, we just eliminate the existing types and start from scratch.
         */
        AVIS_TYPE (VARDEC_AVIS (arg_node)) = NULL;
        type = TYfreeType (type);
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

    info = TEmakeInfo (global.linenum, TE_cond, "predicate");

    res = NTCCTcomputeType (NTCCTcond, info, args);

    args = TYfreeType (args);
    res = TYfreeType (res);

    DBUG_PRINT ("NTC", ("traversing then branch..."));
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    DBUG_PRINT ("NTC", ("traversing else branch..."));
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

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
    ntype *pred, *rhs1, *rhs2;
    ntype *args, *res;
    te_info *info;

    DBUG_ENTER ("NTCfuncond");

    FUNCOND_IF (arg_node) = TRAVdo (FUNCOND_IF (arg_node), arg_info);
    pred = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    FUNCOND_THEN (arg_node) = TRAVdo (FUNCOND_THEN (arg_node), arg_info);
    rhs1 = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    FUNCOND_ELSE (arg_node) = TRAVdo (FUNCOND_ELSE (arg_node), arg_info);
    rhs2 = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    args = TYmakeProductType (3, pred, rhs1, rhs2);

    info = TEmakeInfo (global.linenum, TE_funcond, "conditional");

    /**
     * Here, we need to be able to approximate the result type from
     * less that ALL argument types, as we may deal with recursion
     * which means we need to get an approximation from ONE branch
     * of a conditional to be able to get one for the other branch.
     * This is achieved by using
     *    NTCCTcomputeTypeNonStrict   instead of
     *    NTCCTcomputeType:
     */
    res = NTCCTcomputeTypeNonStrict (NTCCTfuncond, info, args);

    args = TYfreeType (args);

    INFO_NTC_TYPE (arg_info) = TYgetProductMember (res, 0);
    res = TYfreeTypeConstructor (res);

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
    ct_funptr ntc_fun;

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

    /**
     * Now, we investigate the pointer to the function definition:
     */
    wrapper = AP_FUNDEF (arg_node);

    if (!FUNDEF_ISWRAPPERFUN (wrapper) && !FUNDEF_ISLACFUN (wrapper)) {
        /**
         * the fun call has been dispatched already!
         * (i.e., we are in type-upgrade)
         * Essentially, we need to get the existing return type only!
         * This is done by NTCCTdispatched_udf.
         * However, in case of bottom argument types we need to propagate these!
         * This is done in NTCCTcomputeType.
         */
        ntc_fun = NTCCTudfDispatched;

    } else {
        /**
         * the fun call has not yet been dispatched; we may have to specialize!
         * This is done in NTCCTudf.
         * However, in case of bottom argument types we need to propagate these!
         * This is done in NTCCTcomputeType.
         */
        ntc_fun = NTCCTudf;
    }

    old_info_chn = global.act_info_chn;
    global.act_info_chn
      = TEmakeInfoUdf (global.linenum, TE_udf, NSgetName (FUNDEF_NS (wrapper)),
                       FUNDEF_NAME (wrapper), wrapper, INFO_NTC_LAST_ASSIGN (arg_info),
                       global.act_info_chn);
    DBUG_PRINT ("TEINFO",
                ("TE info %p created for udf ap %p", global.act_info_chn, arg_node));
    res = NTCCTcomputeType (ntc_fun, global.act_info_chn, args);

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

    if (prf == F_accu) {
        if (INFO_NTC_EXP_ACCU (arg_info) != NULL) {
            res = TYmakeProductType (1, TYcopyType (INFO_NTC_EXP_ACCU (arg_info)));
        } else {
            INFO_NTC_EXP_ACCU (arg_info) = TYmakeAlphaType (NULL);
            res = TYmakeProductType (1, INFO_NTC_EXP_ACCU (arg_info));
        }
    } else if (prf == F_type_error) {
        /*
         * for F_type_error prfs we just return the bottom
         * type found in arg1.
         */
        res = TYmakeProductType (1, TYcopyType (TYPE_TYPE (PRF_ARG1 (arg_node))));

    } else {
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

        info = TEmakeInfoPrf (global.linenum, TE_prf, global.prf_string[prf], prf);
        res = NTCCTcomputeType (global.ntc_funtab[prf], info, args);

        TYfreeType (args);
    }
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
    ntype *type, *elems;
    te_info *info;
#ifndef DBUG_OFF
    char *tmp_str1, *tmp_str2;
#endif

    DBUG_ENTER ("NTCarray");

    if (NULL != ARRAY_AELEMS (arg_node)) {
        /*
         * First we collect the element types. NTCexprs puts them into a product
         * type which is expected in INFO_NTC_TYPE( arg_info) afterwards!
         * INFO_NTC_NUM_EXPRS_SOFAR is used to count the number of exprs
         * "on the fly"!
         *
         * ATTENTION!!
         * We need to have the ARRAY_SHAPE in order to compute proper result types!
         * In the initial type check, this is always an int[n] shape which means
         * that it can be ignored. However, later, i.e., in TUP, this
         * information may have changed (compare bug 111!).
         * To cope with that situation properly, we add an artificial type
         *   int[ARRAY_SHAPE]  which in NTCCTprf_array is combined with the element
         * type by TYnestTypes.
         */
        INFO_NTC_NUM_EXPRS_SOFAR (arg_info) = 1;

        ARRAY_AELEMS (arg_node) = TRAVdo (ARRAY_AELEMS (arg_node), arg_info);

        DBUG_ASSERT (TYisProd (INFO_NTC_TYPE (arg_info)),
                     "NTCexprs did not create a product type");

        /**
         * Now, we create the type    int[ARRAY_SHAPE]:
         */
        INFO_NTC_NUM_EXPRS_SOFAR (arg_info)--;
        INFO_NTC_TYPE (arg_info)
          = TYsetProductMember (INFO_NTC_TYPE (arg_info),
                                INFO_NTC_NUM_EXPRS_SOFAR (arg_info),
                                TYmakeAKS (TYmakeSimpleType (T_int),
                                           SHcopyShape (ARRAY_SHAPE (arg_node))));
        elems = INFO_NTC_TYPE (arg_info);
        INFO_NTC_TYPE (arg_info) = NULL;

        /**
         * Now, we built the resulting (AKS-)type type from the product type found:
         */
        info = TEmakeInfoPrf (global.linenum, TE_prf, "array-constructor", 0);
        type = NTCCTcomputeType (NTCCTprf_array, info, elems);

        TYfreeType (elems);

    } else {
        ntype *scalar;

        /**
         * we are dealing with an empty array here!
         * so we use the base element information from ARRAY_ELEMTYPE
         * to construct the type.
         */
        DBUG_ASSERT (TYisArray (ARRAY_ELEMTYPE (arg_node)),
                     "found non-array type as elemtype!");

        scalar = TYgetScalar (ARRAY_ELEMTYPE (arg_node));

        DBUG_ASSERT (TUshapeKnown (ARRAY_ELEMTYPE (arg_node)),
                     "found an array constructor for an empty array with non "
                     "AKS element type!");

        /*
         * the the time being, we only build AKV empty arrays
         * for user defined types!
         */
        DBUG_EXECUTE ("NTC", tmp_str1 = SHshape2String (0, ARRAY_SHAPE (arg_node));
                      tmp_str2 = TYtype2String (ARRAY_ELEMTYPE (arg_node), FALSE, 0););
        DBUG_PRINT ("NTC", ("computing type of empty array-constructor with outer "
                            "shape %s and element type %s",
                            tmp_str1, tmp_str2));
        DBUG_EXECUTE ("NTC", tmp_str1 = ILIBfree (tmp_str1);
                      tmp_str2 = ILIBfree (tmp_str2););

        if (TYisSimple (scalar)) {
            type = TYmakeAKV (TYcopyType (scalar),
                              COmakeConstant (TYgetSimpleType (scalar),
                                              SHappendShapes (ARRAY_SHAPE (arg_node),
                                                              TYgetShape (ARRAY_ELEMTYPE (
                                                                arg_node))),
                                              NULL));
        } else {
            type = TYmakeAKS (TYcopyType (scalar),
                              SHappendShapes (ARRAY_SHAPE (arg_node),
                                              TYgetShape (ARRAY_ELEMTYPE (arg_node))));
        }

        type = TYmakeProductType (1, type);

        DBUG_EXECUTE ("NTC", tmp_str1 = TYtype2String (type, FALSE, 0););
        DBUG_PRINT ("NTC", ("yields %s", tmp_str1));
        DBUG_EXECUTE ("NTC", tmp_str1 = ILIBfree (tmp_str1););
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
 *   node *NTCtype( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NTCtype (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("NTCtype");
    INFO_NTC_TYPE (arg_info) = TYcopyType (TYPE_TYPE (arg_node));
    DBUG_RETURN (arg_node);
}

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

    info = TEmakeInfoPrf (global.linenum, TE_prf, "type-cast", 0);
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
    ntype *gen, *body, *res, *tmp, *mem_outer_accu;
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
     *
     * Since this may yield an explicit accu type in INFO_NTC_EXP_ACCU,
     * and fold-wls may be nested, we need to stack these pointers before
     * traversing the code of this WL.
     */
    mem_outer_accu = INFO_NTC_EXP_ACCU (arg_info);
    INFO_NTC_EXP_ACCU (arg_info) = NULL;

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    body = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

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

    /**
     * eventually, we need to restore a potential outer accu for fold-wls:
     */
    INFO_NTC_EXP_ACCU (arg_info) = mem_outer_accu;

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

    info = TEmakeInfo (global.linenum, TE_generator, "generator");
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
    ntype *remaining_blocks, *this_block, *blocks;
    te_info *info;

    DBUG_ENTER ("NTCcode");

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

    DBUG_ASSERT ((TYisProd (INFO_NTC_TYPE (arg_info))
                  && (TYgetProductSize (INFO_NTC_TYPE (arg_info)) == 1)),
                 "multi-operator wl encountered in TC but not supported!");

    /**
     * traverse into further code blocks iff existant
     */
    if (CODE_NEXT (arg_node) != NULL) {
        this_block = INFO_NTC_TYPE (arg_info);
        INFO_NTC_TYPE (arg_info) = NULL;

        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
        remaining_blocks = INFO_NTC_TYPE (arg_info);
        INFO_NTC_TYPE (arg_info) = NULL;

        info = TEmakeInfo (global.linenum, TE_with, "multi generator");
        blocks = TYmakeProductType (2, TYgetProductMember (this_block, 0),
                                    TYgetProductMember (remaining_blocks, 0));
        this_block = TYfreeTypeConstructor (this_block);
        remaining_blocks = TYfreeTypeConstructor (remaining_blocks);

        INFO_NTC_TYPE (arg_info) = NTCCTcomputeType (NTCCTwl_multicode, info, blocks);
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
    info = TEmakeInfo (global.linenum, TE_with, "genarray");
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
    info = TEmakeInfo (global.linenum, TE_with, "modarray");
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
    ntype *neutr, *args;
    node *wrapper;
    te_info *info;
    bool ok;

    DBUG_ENTER ("NTCfold");

    gen = INFO_NTC_GEN_TYPE (arg_info);
    INFO_NTC_GEN_TYPE (arg_info) = NULL;
    body = INFO_NTC_TYPE (arg_info);

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
    neutr = INFO_NTC_TYPE (arg_info);
    INFO_NTC_TYPE (arg_info) = NULL;

    /*
     * Then, we compute the type of the elements to be folded:
     */
    args = TYmakeProductType (3, gen, neutr, body);
    info = TEmakeInfo (global.linenum, TE_with, "fold");
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
     * As we may be dealing with explicit accumulators, this type variable may
     * exist already. If this is the case, it is contained in INFO_NTC_EXP_ACCU
     * otherwise that field is NULL.
     */
    if (INFO_NTC_EXP_ACCU (arg_info) != NULL) {
        /**
         * As the accu is explicit, we have the following situation:
         *
         *   {
         *      a = accu( ...);
         *      e = ....;
         *      val = fun( a, e);
         *   } : val;
         * Therefore, it suffices to take the alpha type of a (from
         * INFO_NTC_EXP_ACCU( arg_info)), make the neutral element a
         * subtype of it (which triggers the initial approximation for
         * fun( a, e) ), and then make the type of val a subtype of
         * the alpha type again in order to ensure the fix-point calculation.
         */
        acc = TYcopyType (INFO_NTC_EXP_ACCU (arg_info));
        INFO_NTC_EXP_ACCU (arg_info) = NULL;

        res = TYmakeProductType (1, elems);

        ok = SSInewTypeRel (neutr, acc);
        DBUG_ASSERT (ok, ("initialization of fold-fun in fold-wl went wrong"));

        ok = SSInewTypeRel (elems, acc);

    } else {
        acc = TYmakeAlphaType (NULL);
        ok = SSInewTypeRel (neutr, acc);
        DBUG_ASSERT (ok, ("initialization of fold-fun in fold-wl went wrong"));

        args = TYmakeProductType (2, acc, elems);
        wrapper = FOLD_FUNDEF (arg_node);
        info = TEmakeInfoUdf (global.linenum, TE_foldf, NSgetName (FUNDEF_NS (wrapper)),
                              FUNDEF_NAME (wrapper), wrapper,
                              INFO_NTC_LAST_ASSIGN (arg_info), NULL);
        res = NTCCTcomputeType (NTCCTudf, info, args);

        ok = SSInewTypeRel (TYgetProductMember (res, 0), acc);
    }
    if (!ok) {
        CTIabortLine (global.linenum, "Illegal fold function in fold with loop");
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
