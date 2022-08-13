#include "dispatchfuncalls.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"

#define DBUG_PREFIX "DFC"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "new_typecheck.h"
#include "new_types.h"
#include "type_utils.h"
#include "create_wrapper_code.h"
#include "namespaces.h"
#include "ct_fun.h"
#include "globals.h"

/*******************************************************************************
 *
 *
 */

/**
 * INFO structure
 */
struct INFO {
    bool dispatched;
    node *fundef;
    node *cexprs;
    node *let;
    node *foldfuns;
};

/**
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_DISPATCHED(n) ((n)->dispatched)
#define INFO_CEXPRS(n) ((n)->cexprs)
#define INFO_LASTLET(n) ((n)->let)
#define INFO_FOLDFUNS(n) ((n)->foldfuns)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_DISPATCHED (result) = FALSE;
    INFO_CEXPRS (result) = NULL;
    INFO_LASTLET (result) = NULL;
    INFO_FOLDFUNS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static int
CountSpecializations (int num_fundefs, node **fundeflist)
{
    int i, res;

    DBUG_ENTER ();

    res = 0;
    for (i = 0; i < num_fundefs; i++) {
        DBUG_ASSERT ((fundeflist[i] != NULL) && (NODE_TYPE (fundeflist[i]) == N_fundef),
                     "CountSpecializations called with illegal fundeflist!");
        if (FUNDEF_ISSPECIALISATION (fundeflist[i])) {
            res++;
        }
    }
    DBUG_RETURN (res);
}

/*
 * to find whether the fun in fold operation is OpenMP reduction clause supported operator
 * or not
 */

static node *
GetOMPReductionOp (node *arg_node)
{
    DBUG_ENTER ();

    char *fun_name = FUNDEF_NAME (FOLD_FUNDEF (arg_node));

    if (STReq ("ScalarArith", NSgetName (FUNDEF_NS (FOLD_FUNDEF (arg_node))))) {
        switch (*fun_name) {
        case '+':
            DBUG_PRINT ("Fold has a scalar add operation\n");
            FOLD_OMPREDUCTIONOP (arg_node) = OMP_REDUCTION_SCL_ADD;
            break;
        case '*':
            DBUG_PRINT ("Fold has a scalar mul operation\n");
            FOLD_OMPREDUCTIONOP (arg_node) = OMP_REDUCTION_SCL_MUL;
            break;
        default:
            break;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFCmodule( node *arg_node, info *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DFCmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    if (INFO_FOLDFUNS (arg_info) != NULL) {
        MODULE_FUNS (arg_node)
          = TCappendFundef (INFO_FOLDFUNS (arg_info), MODULE_FUNS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DispatchFunCall( node *fundef, ntype *arg_types, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DispatchFunCall (node *fundef, ntype *arg_types, info *arg_info)
{
    dft_res *dft_res;
    node *new_fundef = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (fundef != NULL, "fundef not found!");
    if (FUNDEF_ISWRAPPERFUN (fundef)) {
        /*
         * 'fundef' points to an generic function
         *   -> try to dispatch the function application statically in order to
         *      avoid superfluous wrapper function calls
         */
        DBUG_PRINT ("correcting fundef for %s", CTIitemName (fundef));

        /*
         * if we have a bottom type, we cannot dispatch statically.
         * so we do not even try in this case
         */
        if (TYgetBottom (arg_types) == NULL) {
            /*
             * try to dispatch the function application statically
             */
            dft_res = NTCCTdispatchFunType (fundef, arg_types);
            if (dft_res == NULL) {
                DBUG_ASSERT (TYgetProductSize (arg_types) == 0,
                             "illegal dispatch result found!");
                /*
                 * no args found -> static dispatch possible
                 *
                 * fundef can be found in FUNDEF_IMPL (dirty hack!)
                 */
                new_fundef = FUNDEF_IMPL (fundef);
                DBUG_PRINT ("  (1) dispatched statically %s", CTIitemName (new_fundef));
            } else if ((dft_res->num_partials
                        == CountSpecializations (dft_res->num_partials,
                                                 dft_res->partials))
                       && (dft_res->num_deriveable_partials
                           == CountSpecializations (dft_res->num_deriveable_partials,
                                                    dft_res->deriveable_partials))) {
                /*
                 * static dispatch possible!!
                 *
                 * We now know, that EITHER dft_res->def OR dft_res->deriveable
                 * are the only "definition" that matches here. However, there MAY
                 * still be specialisations of that function in  dft_res->partials or in
                 * dft_res->num_deriveable_partials that suit the given arguments
                 * better.
                 * If we would dispatch to the definition here, we might loose such
                 * specializations due to DFR. If we do not dispatch here, we may
                 * a) inhibit inlining (and thus further optimization)
                 * b) create additional runtime overhead (when we and not
                 *    dispatching this call statically at all)
                 * To make the best of this situation, we decide to dipatch, iff
                 * i) there do not exist further specializations   OR
                 * ii) the definition is declared as inline.
                 * That way, we do not loose specializations that might be used
                 * in a later stage of optimization  AND
                 * we avoid problem (a) mentioned above.
                 *
                 * NB: maybe one would want to have something like a "final dispatch"
                 * which would enforce such a dispatch anyways because it is clear
                 * that the inferred argument types will not improve any further,
                 * i.e., we would avoid (b) from above.
                 * But that would require a new mode and a new call to this traversal.
                 * For the time being (May 2008) we simply consider the potential
                 * overhead due to (b) negligible.
                 */
                if (dft_res->def != NULL) {
                    DBUG_ASSERT (dft_res->deriveable == NULL,
                                 "def and deriveable found!");
                    new_fundef = dft_res->def;
                } else if (dft_res->deriveable != NULL) {
                    new_fundef = dft_res->deriveable;
                }
                if (new_fundef != NULL) {
                    if (((dft_res->num_partials + dft_res->num_deriveable_partials) > 0)
                        && !FUNDEF_ISINLINE (new_fundef)) {
                        new_fundef = NULL;
                    } else {
                        DBUG_PRINT ("  (2) dispatched statically %s",
                                    CTIitemName (new_fundef));
                    }
                }
            } else if (!CWChasWrapperCode (fundef)) {
                /*
                 * static dispatch impossible,
                 *    but no wrapper function could be created either!!
                 * if only a single instance is available, do the dispatch
                 * statically and give a warning message, otherwise we are stuck here!
                 */
                if ((dft_res->num_partials + dft_res->num_deriveable_partials == 1)
                    && (dft_res->def == NULL) && (dft_res->deriveable == NULL)) {
                    new_fundef = (dft_res->num_partials == 1)
                                   ? dft_res->partials[0]
                                   : dft_res->deriveable_partials[0];
                    CTIwarn (LINE_TO_LOC (global.linenum),
                             "Application of var-arg function %s found which may"
                             " cause a type error",
                             CTIitemName (new_fundef));
                    DBUG_PRINT ("  dispatched statically although only partial"
                                " has been found (T_dots)!");
                } else {
                    DBUG_UNREACHABLE (
                      "wrapper with T_dots found which could be dispatched statically!");
                }
            } else {
                /*
                 * static dispatch impossible -> keep the wrapper
                 */
                DBUG_PRINT ("  static dispatch impossible");
            }
        }
    }
    if (new_fundef != NULL) {
        INFO_DISPATCHED (arg_info) = TRUE;
    } else {
        new_fundef = fundef;
    }

    DBUG_RETURN (new_fundef);
}

/******************************************************************************
 *
 * Function:
 *   node *DFCfundef( node *arg_node, info *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DFCfundef (node *arg_node, info *arg_info)
{
    node *old_fundef;
    DBUG_ENTER ();

    /**
     * we do not dispatch within wrapper functions, and we only look at
     * LaC functions if we are in one-fundef mode and not at the top level or
     * if we are currently at the top level. The reason here is * that in
     * one-fundef mode and only in that mode, we treat LAC-funs as inline
     * (cf DFCap). This choice became necessary to be able to tag the correct
     * callgraph for LaCfuns as FUNDEF_ISINLINECOMPLETED( arg_node) = FALSE
     * which is required for a proper inlining in fundef mode.
     *
     * NB: onefundef mode is removed because of the new CYCLEPHASEFUN
     * implementation. (DEVCAMP 2012)
     */
    if (!FUNDEF_ISWRAPPERFUN (arg_node)
        && ((INFO_FUNDEF (arg_info) != NULL) || !FUNDEF_ISLACFUN (arg_node))) {

        DBUG_PRINT ("traversing function body of %s", CTIitemName (arg_node));

        old_fundef = INFO_FUNDEF (arg_info);
        INFO_FUNDEF (arg_info) = arg_node;

        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

        INFO_FUNDEF (arg_info) = old_fundef;

        DBUG_PRINT ("leaving function body of %s", CTIitemName (arg_node));

        if (INFO_DISPATCHED (arg_info)) {
            FUNDEF_ISINLINECOMPLETED (arg_node) = FALSE;
            DBUG_PRINT ("FUNDEF_ISINLINECOMPLETED set to FALSE for %s",
                        CTIitemName (arg_node));
        }
    }

    if (INFO_FUNDEF (arg_info) == NULL) {
        INFO_DISPATCHED (arg_info) = FALSE;
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFCap( node *arg_node, info *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DFCap (node *arg_node, info *arg_info)
{
    bool old_dispatched;
    ntype *arg_types;

    DBUG_ENTER ();

    AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

    DBUG_PRINT ("Ap of function %s::%s pointed to " F_PTR ".",
                NSgetName (AP_NS (arg_node)), AP_NAME (arg_node), (void *)AP_FUNDEF (arg_node));

    arg_types = TUactualArgs2Ntype (AP_ARGS (arg_node));
    AP_FUNDEF (arg_node) = DispatchFunCall (AP_FUNDEF (arg_node), arg_types, arg_info);

    DBUG_PRINT ("Ap of function %s:%s now points to " F_PTR ".",
                NSgetName (AP_NS (arg_node)), AP_NAME (arg_node), (void *)AP_FUNDEF (arg_node));
    arg_types = TYfreeType (arg_types);

    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {

        old_dispatched = INFO_DISPATCHED (arg_info);
        INFO_DISPATCHED (arg_info) = FALSE;
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        INFO_DISPATCHED (arg_info) = (INFO_DISPATCHED (arg_info) || old_dispatched);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *DFCwith( node *arg_node, info *arg_info)
 *
 * @brief inserts actual with node into the info structure
 *
 ******************************************************************************/

node *
DFCwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    INFO_CEXPRS (arg_info) = WITH_CEXPRS (arg_node);
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    INFO_CEXPRS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFCgenarray( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DFCgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);
    GENARRAY_DEFAULT (arg_node) = TRAVopt (GENARRAY_DEFAULT (arg_node), arg_info);

    if (GENARRAY_NEXT (arg_node) != NULL) {
        DBUG_ASSERT (EXPRS_NEXT (INFO_CEXPRS (arg_info)) != NULL,
                     "Fewer cexprs than withops!");

        INFO_CEXPRS (arg_info) = EXPRS_NEXT (INFO_CEXPRS (arg_info));
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *DFCmodarray( node *arg_node, info *arg_info)
 *
 * @brief Traverses the modarray withop, updates the cexprs in the info node
 *        and continues with the next withop.
 *
 * @param arg_node N_modarray node
 * @param arg_info info structure
 *
 * @return updated N_modarray node
 ******************************************************************************/
node *
DFCmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);

    if (MODARRAY_NEXT (arg_node) != NULL) {
        DBUG_ASSERT (EXPRS_NEXT (INFO_CEXPRS (arg_info)) != NULL,
                     "Fewer cexprs than withops!");

        INFO_CEXPRS (arg_info) = EXPRS_NEXT (INFO_CEXPRS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *DFCfold( node *arg_node, info *arg_info)
 *
 * @brief Dispatches the call to the fold-function to one of the newly split
 *        wrappers.
 *
 * @param arg_node N_fold node
 * @param arg_info info structure
 *
 * @return N_fold with corrected fundef pointer
 ******************************************************************************/
node *
DFCfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_ASSERT (FOLD_FUNDEF (arg_node) != NULL, "missing FOLD_FUNDEF");

    FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

    ntype *neutr_type
      = TYfixAndEliminateAlpha (AVIS_TYPE (ID_AVIS (FOLD_NEUTRAL (arg_node))));
    ntype *body_type = TYfixAndEliminateAlpha (
      AVIS_TYPE (ID_AVIS (EXPRS_EXPR (INFO_CEXPRS (arg_info)))));

    /* create an array of types: length = 2 + number of partial args */
    ntype *arg_types = TYmakeEmptyProductType (TCcountExprs (FOLD_ARGS (arg_node)) + 2);

    /* fill the positions in the array, first from the FOLD_ARGS chain */
    size_t a_pos = 0;
    for (node *pa_arg = FOLD_ARGS (arg_node); pa_arg != NULL;
         pa_arg = EXPRS_NEXT (pa_arg)) {
        arg_types = TYsetProductMember (arg_types, a_pos++,
                                        TYfixAndEliminateAlpha ((ntype *)AVIS_TYPE (
                                          ID_AVIS (EXPRS_EXPR (pa_arg)))));
    }

    /* ...and then twice the neutral type */
    ntype *arg_type = TYlubOfTypes (neutr_type, body_type);
    /* the last two args have the same type */
    arg_types = TYsetProductMember (arg_types, a_pos++, arg_type);
    arg_types = TYsetProductMember (arg_types, a_pos++, TYcopyType (arg_type));

    FOLD_FUNDEF (arg_node)
      = DispatchFunCall (FOLD_FUNDEF (arg_node), arg_types, arg_info);
    if (global.backend == BE_omp) {
        arg_node = GetOMPReductionOp (arg_node);
    }

    /*
     * cleanup
     */
    arg_types = TYfreeType (arg_types);
    body_type = TYfreeType (body_type);
    neutr_type = TYfreeType (neutr_type);

    if (FOLD_NEXT (arg_node) != NULL) {
        DBUG_ASSERT (EXPRS_NEXT (INFO_CEXPRS (arg_info)) != NULL,
                     "Fewer cexprs than withops!");

        INFO_CEXPRS (arg_info) = EXPRS_NEXT (INFO_CEXPRS (arg_info));
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *DFCpropagate( node *arg_node, info *arg_info)
 *
 * @brief Traverses the propagate withop, updates the cexprs in the info node
 *        and continues with the next withop.
 *
 * @param arg_node N_propagate node
 * @param arg_info info structure
 *
 * @return updated N_propagate node
 ******************************************************************************/
node *
DFCpropagate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    PROPAGATE_DEFAULT (arg_node) = TRAVdo (PROPAGATE_DEFAULT (arg_node), arg_info);

    if (PROPAGATE_NEXT (arg_node) != NULL) {
        DBUG_ASSERT (EXPRS_NEXT (INFO_CEXPRS (arg_info)) != NULL,
                     "Fewer cexprs than withops!");

        INFO_CEXPRS (arg_info) = EXPRS_NEXT (INFO_CEXPRS (arg_info));
        PROPAGATE_NEXT (arg_node) = TRAVdo (PROPAGATE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
DFClet (node *arg_node, info *arg_info)
{
    node *old_lastlet;

    DBUG_ENTER ();

    old_lastlet = INFO_LASTLET (arg_info);
    INFO_LASTLET (arg_info) = arg_node;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_LASTLET (arg_info) = old_lastlet;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFCdoDispatchFunCalls( node *ast)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DFCdoDispatchFunCalls (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_dfc);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
