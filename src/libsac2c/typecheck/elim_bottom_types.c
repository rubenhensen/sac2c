#include <stdio.h>

#include "elim_bottom_types.h"

#define DBUG_PREFIX "EBT"
#include "debug.h"

#include "ctinfo.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "shape.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse_helper.h"

#include "new_typecheck.h"
#include "new_types.h"
#include "type_utils.h"
#include "type_errors.h"
#include "ssi.h"
#include "sig_deps.h"
#include "globals.h"
#include "lacinlining.h"

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
 * This phase eliminates all existing bottom types.
 * To achieve that, it does:
 * - transform functions that return a bottom type into bottom-functions
 * - eliminate bottom variables by
 *   a) eliminating the respective declaration
 *   b) replacing the definition of the variable by a type_error
 *      which is assigned to the (remaining if any) non-bottom lhs
 *      variables.
 * Here an example......
 *
 *      int foo( int x)
 *      {
 *         _|_ a;
 *         int b;
 *         int c;
 *
 *         a, b, c = goo( x);
 *
 *         return( b);
 *      }
 *
 * is transformed into:
 *
 *      int foo( int x)
 *      {
 *         int b;
 *         int c;
 *
 *         b, c = _type_error( _|_);
 *
 *         return( b);
 *      }
 *
 * There is one exception to this scheme though, which originates from funconds.
 * As these are non-strict, we allow one of them to be bottom.
 * As a consequence, we do need a proper definition of that variable although
 * we inferred a bottom type for it! To achieve that, we transform the rhs of
 * that definition into a type error and then change its type from bottom to
 * the corresponding non-bottom type of the other branch of the conditional.
 * For example......
 *
 *      int foo( int x)
 *      {
 *         _|_ a;
 *         int a2;
 *
 *         if( x == 1) {
 *           a = goo( x);
 *         } else {
 *         }
 *         a2 = ( x == 1 ? a : x);
 *
 *         return( a2);
 *      }
 *
 * needs to be transformed into:
 *
 *      int foo( int x)
 *      {
 *         int a;
 *         int a2;
 *
 *         if( x == 1) {
 *           a = _type_error_( _|_);
 *         } else {
 *         }
 *         a2 = ( x == 1 ? a : x);
 *
 *         return( a2);
 *      }
 *
 * Note here, that the "bottom identifier" "a" has NOT been eliminated
 * but turned into a non-bottom type, namely "int" which is derived from the
 * other alternative from the funcond, i.e., from "x". However, the type
 * error remains captured by the application of "_type_error_" instead
 * of "goo".
 *
 * NB: Apart from these bottom type eliminations we also eliminate
 *     LaC functions that obtain bottom arguments. This needs to be done
 *     as these functions have NOT been type checked at all!
 *
 */

/*
 * usages of 'arg_info':
 *
 *   INFO_THENBOTTS   - at least one bottom in then branch found
 *   INFO_ELSEBOTTS   - at least one bottom in else branch found
 *   INFO_FROMAP      - fundef traversal called from N_AP
 *   INFO_FUNDEF      - pointer to current fundef to prevent infinite recursion
 *   INFO_TYPEERROR   - indicates that the rhs of a let is to be transformed
 *                      into this type error
 *   INFO_DROPASSIGN  - true if we decide within the ASSIGN_STMT that this
 *                      assign needs to be removed
 */

/**
 * INFO structure
 */
struct INFO {
    bool then_botts;
    bool else_botts;
    bool fromap;
    node *fundef;
    node *type_error;
    bool dropassign;
};

/**
 * INFO macros
 */
#define INFO_THENBOTTS(n) ((n)->then_botts)
#define INFO_ELSEBOTTS(n) ((n)->else_botts)
#define INFO_FROMAP(n) ((n)->fromap)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_TYPEERROR(n) ((n)->type_error)
#define INFO_DROPASSIGN(n) ((n)->dropassign)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_THENBOTTS (result) = FALSE;
    INFO_ELSEBOTTS (result) = FALSE;
    INFO_FROMAP (result) = FALSE;
    INFO_FUNDEF (result) = NULL;
    INFO_TYPEERROR (result) = NULL;
    INFO_DROPASSIGN (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static node *
TransformIntoTypeError (node *fundef)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef, "cannot transform non fundef node");

    DBUG_ASSERT (TUretsContainBottom (FUNDEF_RETS (fundef)),
                 "cannot transform a fundef without bottom return types!");

    FUNDEF_BODY (fundef) = FREEoptFreeNode(FUNDEF_BODY (fundef));

    /*
     * we mark the function as a special type error function. this
     * is done to ease the detection of these functions. otherwise
     * one would have to traverse through all the return types to
     * check, which would be a noticeable performance drawback.
     */
    FUNDEF_ISTYPEERROR (fundef) = TRUE;

    /*
     * furthermore we set the function to non-inline. as it has no
     * body, it cannot be inlined anyways but better make sure.
     */
    FUNDEF_ISINLINE (fundef) = FALSE;

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *EBTdoEliminateBottomTypes( node *arg_node)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
EBTdoEliminateBottomTypes (node *arg_node)
{
    info *info_node;

    DBUG_ENTER ();

    TRAVpush (TR_ebt);

    info_node = MakeInfo ();
    arg_node = TRAVdo (arg_node, info_node);
    info_node = FreeInfo (info_node);

    TRAVpop ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *EBTmodule( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
EBTmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_TYPES (arg_node) = TRAVopt (MODULE_TYPES (arg_node), arg_info);
    MODULE_OBJS (arg_node) = TRAVopt (MODULE_OBJS (arg_node), arg_info);
    MODULE_FUNDECS (arg_node) = TRAVopt (MODULE_FUNDECS (arg_node), arg_info);
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *EBTfundef( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
EBTfundef (node *arg_node, info *arg_info)
{
    ntype *ftype, *bottom;

    DBUG_ENTER ();

    if (!FUNDEF_ISLACFUN (arg_node) || INFO_FROMAP (arg_info)) {
        INFO_FUNDEF (arg_info) = arg_node;

        DBUG_PRINT ("----> Processing function %s\n", CTIitemName (arg_node));

        ftype = TUmakeProductTypeFromRets (FUNDEF_RETS (arg_node));

        bottom = TYgetBottom (ftype);
        if (bottom != NULL) {
            DBUG_PRINT ("bottom component found in function %s", CTIitemName (arg_node));
            if (FUNDEF_ISPROVIDED (arg_node) || FUNDEF_ISEXPORTED (arg_node)) {
                CTIerror (LINE_TO_LOC (global.linenum),
                          "All instances of \"%s\" contain type errors",
                          FUNDEF_NAME (arg_node));
                CTIabortOnBottom (TYgetBottomError (bottom));
            } else {
                /**
                 * we transform the entire body into one 'type error' assignment
                 *
                 * In case of LaC funs this is more difficult. Since the signatures
                 * of these funs are not user specified, they do not have an upper
                 * limit which prevents from a successfull type error generation.
                 * (cf. bug no 115).However, since this bottom has been propagated into
                 * the calling site, we can simply eliminate these functions entirely!
                 */
                arg_node = TransformIntoTypeError (arg_node);
            }

        } else {
            DBUG_ASSERT (TYisProdOfArray (ftype), "inconsistent return type found");
            DBUG_PRINT ("ProdOfArray return type found for function %s",
                        CTIitemName (arg_node));

            FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

            if (FUNDEF_ISLOOPFUN (arg_node) && INFO_THENBOTTS (arg_info)) {
                FUNDEF_ISLOOPFUN (arg_node) = FALSE;
                FUNDEF_ISLACINLINE (arg_node) = TRUE;
            }
        }

        INFO_THENBOTTS (arg_info) = FALSE;
        INFO_ELSEBOTTS (arg_info) = FALSE;
    }

    if (!INFO_FROMAP (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EBTblock( node *arg_node, info *arg_info)
 *
 * @brief ensure that the BODY is traversed BEFORE the vardecs!
 * @return original node
 *
 *****************************************************************************/

node *
EBTblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *EBTvardec( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
EBTvardec (node *arg_node, info *arg_info)
{
    node *this_vardec;

    DBUG_ENTER ();

    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    if (TYisBottom (VARDEC_NTYPE (arg_node))) {
        /**
         * eliminate this vardec completely!
         */
        DBUG_PRINT ("eliminating bottom vardec for %s", VARDEC_NAME (arg_node));
        this_vardec = arg_node;
        arg_node = VARDEC_NEXT (arg_node);
        VARDEC_NEXT (this_vardec) = NULL;
        this_vardec = FREEdoFreeTree (this_vardec);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EBTassign( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
EBTassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /**
     * First we go down in order to collect those bottom funcond vars that need to
     * be created by the prf 'type_error'.
     */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    /**
     * Now, we can go into the instruction. In case of
     * - N_cond we place the type_errors generated by the funconds into the
     *   corresponding blocks!
     */
    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_DROPASSIGN (arg_info)) {
        arg_node = FREEdoFreeNode (arg_node);
        INFO_DROPASSIGN (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EBTlet( node *arg_node, info *arg_info)
 *
 *   @brief if args contain bottom: convert rhs into type-error
 *          else if lhs vars contain bottom: convert rhs into type-error.
 *          Anyways do filter-out bottom lhs vars.
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
EBTlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    if (INFO_TYPEERROR (arg_info) != NULL) {
        LET_EXPR (arg_node) = FREEdoFreeTree (LET_EXPR (arg_node));
        LET_EXPR (arg_node) = INFO_TYPEERROR (arg_info);
        INFO_TYPEERROR (arg_info) = NULL;
    }

    /*
     * we might potentially have removed all LHS ids. In that case,
     * we drop the entire assign.
     */
    if (LET_IDS (arg_node) == NULL) {
        INFO_DROPASSIGN (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EBTids( node *arg_node, info *arg_info)
 *
 *   @brief filter-out lhs vars. iff INFO_TYPEERROR is NULL and lhs contains
 *          bottom vars, create type-error!
 *          iff AVIS_BOTRT is set, replace the bottom type with the specified
 *          one.
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
EBTids (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    avis = IDS_AVIS (arg_node);

    if (TYisBottom (AVIS_TYPE (avis))) {
        if (INFO_TYPEERROR (arg_info) == NULL) {
            DBUG_PRINT ("creating type error due to bottom LHS %s", AVIS_NAME (avis));
            INFO_TYPEERROR (arg_info)
              = TCmakePrf1 (F_type_error, TBmakeType (TYcopyType (AVIS_TYPE (avis))));
        }

        if (AVIS_BOTRT (avis) != NULL) {
            DBUG_PRINT ("lifting bottom LHS %s", AVIS_NAME (avis));
            AVIS_TYPE (avis) = TYfreeType (AVIS_TYPE (avis));
            AVIS_TYPE (avis) = AVIS_BOTRT (avis);
            AVIS_BOTRT (avis) = NULL;

            /**
             * this seems to be saa-bind related stuff??!
             */
            AVIS_DIM (avis) = FREEoptFreeNode(AVIS_DIM (avis));
            AVIS_SHAPE (avis) = FREEoptFreeNode(AVIS_SHAPE (avis));

        } else {
            DBUG_PRINT ("eliminating bottom LHS %s", AVIS_NAME (avis));
            arg_node = FREEdoFreeNode (arg_node);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EBTap( node *arg_node, info *arg_info)
 *
 * @brief create a type error iff args contain bottom;
 *        delete LaC fun iff args contain bottom, traverse into it otherwise.
 * @return original node
 *
 *****************************************************************************/
node *
EBTap (node *arg_node, info *arg_info)
{
    ntype *argt, *bottom;

    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    argt = TUactualArgs2Ntype (AP_ARGS (arg_node));
    bottom = TYgetBottom (argt);

    if (bottom != NULL) {
        if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))
            && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
            DBUG_PRINT ("lacfun %s found...", CTIitemName (AP_FUNDEF (arg_node)));
            DBUG_PRINT ("deleting %s", CTIitemName (AP_FUNDEF (arg_node)));
            AP_FUNDEF (arg_node) = FREEdoFreeNode (AP_FUNDEF (arg_node));
        }
        INFO_TYPEERROR (arg_info)
          = TCmakePrf1 (F_type_error, TBmakeType (TYcopyType (bottom)));

    } else {
        if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))
            && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
            DBUG_PRINT ("lacfun %s found...", CTIitemName (AP_FUNDEF (arg_node)));
            info *new_info = MakeInfo ();
            INFO_FROMAP (new_info) = TRUE;
            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), new_info);
            new_info = FreeInfo (new_info);
        }
    }

    argt = TYfreeType (argt);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EBTprf( node *arg_node, info *arg_info)
 *
 * @brief iff this is F_type_error_ we signal this to EBTassign by putting
 *        a copy of it into INFO_TYPEERROR. This is needed to preserve the
 *        extra arguments at the end that arise from wrapper functions.
 *        Cf. issue 2294 for details.
 * @return original node
 *
 *****************************************************************************/
node *
EBTprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (PRF_PRF (arg_node) == F_type_error || PRF_PRF (arg_node) == F_guard_error) {
        DBUG_PRINT ("F_type_error found, duplicating for argument preservation");
        INFO_TYPEERROR (arg_info) = DUPdoDupTree (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EBTcond( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
EBTcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_THENBOTTS (arg_info) && INFO_ELSEBOTTS (arg_info)) {
        /**
         * There are errors in both branches, i.e., abort
         */
        CTIabort (LINE_TO_LOC (global.linenum), "Conditional with type errors in both branches");
    }
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EBTfuncond( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
EBTfuncond (node *arg_node, info *arg_info)
{
    ntype *ttype, *etype;

    DBUG_ENTER ();

    ttype = AVIS_TYPE (ID_AVIS (FUNCOND_THEN (arg_node)));
    etype = AVIS_TYPE (ID_AVIS (FUNCOND_ELSE (arg_node)));

    if (TYisBottom (ttype)) {
        DBUG_ASSERT (!TYisBottom (etype), "two bottom args for funcond found");
        AVIS_BOTRT (ID_AVIS (FUNCOND_THEN (arg_node))) = TYeliminateAKV (etype);
        INFO_THENBOTTS (arg_info) = TRUE;
    }
    if (TYisBottom (etype)) {
        DBUG_ASSERT (!TYisBottom (ttype), "two bottom args for funcond found");
        AVIS_BOTRT (ID_AVIS (FUNCOND_ELSE (arg_node))) = TYeliminateAKV (ttype);
        INFO_ELSEBOTTS (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
