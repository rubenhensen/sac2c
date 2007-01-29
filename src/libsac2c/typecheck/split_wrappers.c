/*
 *
 * $Id$
 *
 */

#include "split_wrappers.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "ctinfo.h"
#include "LookUpTable.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "new_typecheck.h"
#include "new_types.h"
#include "type_utils.h"
#include "ct_fun.h"
#include "namespaces.h"
#include "serialize.h"
#include "deserialize.h"
#include "update_wrapper_type.h"
#include "elim_alpha_types.h"
#include "elim_bottom_types.h"

/*******************************************************************************
 *
 *
 */

/**
 * INFO structure
 */
struct INFO {
    int travno;
    lut_t *wrapperfuns;
    node *with;
    namespace_t *ns;
};

/**
 * INFO macros
 */
#define INFO_SWR_TRAVNO(n) ((n)->travno)
#define INFO_SWR_WRAPPERFUNS(n) ((n)->wrapperfuns)
#define INFO_SWR_WITH(n) ((n)->with)
#define INFO_SWR_NAMESPACE(n) ((n)->ns)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_SWR_TRAVNO (result) = 0;
    INFO_SWR_WRAPPERFUNS (result) = NULL;
    INFO_SWR_WITH (result) = NULL;
    INFO_SWR_NAMESPACE (result) = NULL;

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
 **
 ** Function:
 **   node *SWRdoSplitWrappers( node *ast)
 **
 ** Description:
 **   Replaces generic wrappers (valid for more than a single simpletype)
 **   by individual wrappers for each simpletype.
 **
 **/

/******************************************************************************
 *
 * Function:
 *   node *SWRmodule( node *arg_node, info *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
SWRmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWRmodule");

    DBUG_ASSERT ((MODULE_WRAPPERFUNS (arg_node) != NULL),
                 "MODULE_WRAPPERFUNS not found!");
    INFO_SWR_WRAPPERFUNS (arg_info) = MODULE_WRAPPERFUNS (arg_node);
    INFO_SWR_NAMESPACE (arg_info) = MODULE_NAMESPACE (arg_node);

    /*
     * create separate wrapper function for all base type constellations
     * As all wrappers are in the FUNS, we have to traverse these only!
     */
    INFO_SWR_TRAVNO (arg_info) = 1;

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    /*
     * adjust AP_FUNDEF pointers
     * As only FUNS may contain N_ap's we have to traverse these only!
     */
    INFO_SWR_TRAVNO (arg_info) = 2;
    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    /*
     * remove non-used and zombie funs!
     */
    INFO_SWR_TRAVNO (arg_info) = 3;

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    MODULE_WRAPPERFUNS (arg_node) = LUTremoveLut (MODULE_WRAPPERFUNS (arg_node));

    DBUG_RETURN (arg_node);
}

bool
isLocalInstance (node *fundef, bool result)
{
    DBUG_ENTER ("isLocalInstance");

    result = result | FUNDEF_ISLOCAL (fundef);

    DBUG_RETURN (result);
}

bool
containsLocalInstances (node *wrapper)
{
    bool result;

    DBUG_ENTER ("containsLocalInstances");

    if (FUNDEF_IMPL (wrapper) != NULL) {
        result = FUNDEF_ISLOCAL (FUNDEF_IMPL (wrapper));
    } else {
        void *fold;

        fold = TYfoldFunctionInstances (FUNDEF_WRAPPERTYPE (wrapper),
                                        (void *(*)(node *, void *))isLocalInstance,
                                        (void *)FALSE);

        /*
         * we have to use this instead of casting fold to bool
         * as they may be of different sizes which would lead to
         * a compiler warning
         */
        result = (fold != NULL);
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * Function:
 *   node *SplitWrapper( node *fundef, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
SplitWrapper (node *fundef, info *arg_info)
{
    ntype *old_type, *tmp_type;
    ntype *new_type, *new_rets;
    ntype *bottom = NULL;
    int pathes_remaining;
    node *new_fundef;
    node *new_fundefs = NULL;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("SplitWrapper");

    old_type = FUNDEF_WRAPPERTYPE (fundef);
    tmp_type = TYcopyType (old_type);
    FUNDEF_WRAPPERTYPE (fundef) = NULL;
    DBUG_PRINT ("SWR", ("splitting wrapper of %s", CTIitemName (fundef)));

    do {
        new_fundef = DUPdoDupNode (fundef);
        new_type = TYsplitWrapperType (tmp_type, &pathes_remaining);
        if (pathes_remaining == 1) {
            tmp_type = NULL;
        }
        DBUG_EXECUTE ("SWR", tmp_str = TYtype2String (new_type, TRUE, 0););
        DBUG_PRINT ("SWR",
                    ("  new wrapper split off: \n%s : " F_PTR, tmp_str, new_fundef));
        DBUG_EXECUTE ("SWR", tmp_str = ILIBfree (tmp_str););
        DBUG_EXECUTE ("SWR", tmp_str = TYtype2String (tmp_type, TRUE, 0););
        DBUG_PRINT ("SWR", ("  remaining wrapper : \n%s : ", tmp_str));
        DBUG_EXECUTE ("SWR", tmp_str = ILIBfree (tmp_str););

        FUNDEF_WRAPPERTYPE (new_fundef) = new_type;
        new_rets = TYgetWrapperRetType (new_type);
        bottom = TYgetBottom (new_rets);
        if (bottom != NULL) {
            CTIerrorLine (global.linenum, "All instances of \"%s\" contain type errors",
                          FUNDEF_NAME (new_fundef));
            CTIabortOnBottom (TYgetBottomError (bottom));
        }

        FUNDEF_RETS (new_fundef) = TUreplaceRetTypes (FUNDEF_RETS (new_fundef), new_rets);

        FUNDEF_ARGS (new_fundef)
          = TYcorrectWrapperArgTypes (FUNDEF_ARGS (new_fundef), new_type);

        /*
         * mark new wrapper as needed, so it can be easily distinguished
         * from the generic ones
         */
        FUNDEF_ISNEEDED (new_fundef) = TRUE;

        /*
         * mark the new wrapper as typechecked, as it is known to be
         * correctly typed! this is important as wrappers might be
         * loaded during typechecking!
         */
        FUNDEF_TCSTAT (new_fundef) = NTC_checked;

        /*
         * mark the wrapper as non-local if it is from
         * a different namespace than the current
         */
        if (!NSequals (FUNDEF_NS (new_fundef), INFO_SWR_NAMESPACE (arg_info))) {
            FUNDEF_ISLOCAL (new_fundef) = FALSE;
        }

        /*
         * remove the aliasing for the splitoff
         */
        if (!FUNDEF_ISLOCAL (new_fundef)) {
            FUNDEF_SYMBOLNAME (new_fundef)
              = ILIBstringCopy (SERgenerateSerFunName (SET_wrapperhead, new_fundef));

            DBUG_PRINT ("SWR",
                        ("generated symbolname is %s", FUNDEF_SYMBOLNAME (new_fundef)));

            DSremoveAliasing (FUNDEF_SYMBOLNAME (new_fundef));
        }

        /*
         * if the created wrapper contains any local instances because it
         *
         * a) is a wrapper which was locally generated
         *
         * b) is a used wrapper with added specialisations
         *
         * we must make sure it is added into the right namespace. A
         * wrapper with added specialisations can be easily identified
         * by looking at the SPECNS attribute of the generic wrapper.
         * By checking whether this splitof has any local instances,
         * we can make sure that only those wrappers which have been
         * altered are made local.
         * Locally generated wrappers will have the correct namespace
         * anyways, so we do not need to care for that here
         */
        if ((FUNDEF_SPECNS (fundef) != NULL) && containsLocalInstances (new_fundef)) {
            /*
             * store the SPECNS of the generic wrapper within the SPECNS
             * of the splitof wrapper. We cannot directly rename the
             * namespace here as the old namespace is still needed
             * in CorrectFundefPointer on the bottom up traversal.
             */
            FUNDEF_SPECNS (new_fundef) = NSdupNamespace (FUNDEF_SPECNS (fundef));
        }

        FUNDEF_NEXT (new_fundef) = new_fundefs;
        new_fundefs = new_fundef;
    } while (pathes_remaining > 1);
    FUNDEF_WRAPPERTYPE (fundef) = old_type;

    DBUG_RETURN (new_fundefs);
}

/******************************************************************************
 *
 * Function:
 *   node *CorrectFundefPointer( node *fundef, ntype *arg_types)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CorrectFundefPointer (node *fundef, ntype *arg_types)
{
    node *newfundef;

    DBUG_ENTER ("CorrectFundefPointer");

    newfundef = fundef;

    DBUG_ASSERT ((fundef != NULL), "fundef not found!");
    if (FUNDEF_ISWRAPPERFUN (fundef) && !FUNDEF_ISNEEDED (fundef)) {
        /*
         * 'fundef' points to an generic wrapper function
         *    -> search the specific wrapper function
         */
        DBUG_PRINT ("SWR", ("correcting fundef for %s", CTIitemName (fundef)));

        if (TYgetBottom (arg_types) == NULL) {
            /*
             * -> search for correct wrapper
             */
            DBUG_PRINT ("SWR", ("  search for wrapper"));

            do {
                newfundef = FUNDEF_NEXT (newfundef);
                DBUG_ASSERT (((newfundef != NULL)
                              && NSequals (FUNDEF_NS (newfundef), FUNDEF_NS (fundef))
                              && ILIBstringCompare (FUNDEF_NAME (newfundef),
                                                    FUNDEF_NAME (fundef))
                              && FUNDEF_ISWRAPPERFUN (newfundef)),
                             "no appropriate wrapper function found!");

                DBUG_ASSERT ((!FUNDEF_ISZOMBIE (newfundef)), "zombie found");
            } while (!TUsignatureMatches (FUNDEF_ARGS (newfundef), arg_types, TRUE));
            DBUG_PRINT ("SWR", ("  correct wrapper found"));
        } else {
            /**
             * as we are dealing with a bottom argument, we need to select any one
             * of the non-generic wrappers. Since at least one of them will follow
             * the generic one directly, we can choose that one.
             */
            newfundef = FUNDEF_NEXT (fundef);
            DBUG_ASSERT (((newfundef != NULL)
                          && NSequals (FUNDEF_NS (newfundef), FUNDEF_NS (fundef))
                          && ILIBstringCompare (FUNDEF_NAME (newfundef),
                                                FUNDEF_NAME (fundef))
                          && FUNDEF_ISWRAPPERFUN (newfundef)),
                         "no appropriate wrapper function found!");
        }
    }

    DBUG_RETURN (newfundef);
}

/******************************************************************************
 *
 * Function:
 *   node *SWRfundef( node *arg_node, info *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
FundefBuildWrappers (node *arg_node, info *arg_info)
{
    node *new_fundefs;

    DBUG_ENTER ("FundefBuildWrappers");

    /*
     * only process wrappers that are local (eg. created by create_wrappers)
     * as for all others there is nothing to split
     */
    if ((FUNDEF_ISWRAPPERFUN (arg_node) && (FUNDEF_ISLOCAL (arg_node)))) {
        DBUG_ASSERT ((FUNDEF_BODY (arg_node) == NULL),
                     "wrapper function has already a body!");

        /*
         * build a separate fundef for each base type constellation
         */
        new_fundefs = SplitWrapper (arg_node, arg_info);

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }

        /*
         * insert new wrapper functions after the generic wrapper
         * the generic wrapper is freed later on
         */
        new_fundefs = TCappendFundef (new_fundefs, FUNDEF_NEXT (arg_node));
        DBUG_ASSERT ((FUNDEF_BODY (arg_node) == NULL),
                     "body of generic wrapper function has not been kept empty");
        FUNDEF_NEXT (arg_node) = new_fundefs;

        /*
         * mark the old generic wrapper as not needed
         */
        FUNDEF_ISNEEDED (arg_node) = FALSE;
    } else if ((FUNDEF_ISWRAPPERFUN (arg_node) && (!FUNDEF_ISLOCAL (arg_node)))) {
        /*
         * this is a non local wrapper, so it is needed and has to be marked
         * as needed...
         */
        FUNDEF_ISNEEDED (arg_node) = TRUE;

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    } else {
        /*
         * if this is no wrapper function, just skip to the next function
         */
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }
    DBUG_RETURN (arg_node);
}

static node *
FundefAdjustPointers (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FundefAdjustPointers");

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    if ((!FUNDEF_ISWRAPPERFUN (arg_node)) && (FUNDEF_BODY (arg_node) != NULL)) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

static node *
FundefRemoveGarbage (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FundefRemoveGarbage");

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    if ((FUNDEF_ISWRAPPERFUN (arg_node)) && (!FUNDEF_ISNEEDED (arg_node))) {
        /*
         * remove statically dispatchable wrapper function and all generic wrappers
         */
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

static node *
FundefMoveToFinalNs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FundefMoveToFinalNs");

    /*
     * if the current wrapper has a SPECNS assign, this can have
     * two reasons:
     *
     * a) a specialisation was added to this wrapper in
     *    DoSpecialise
     *
     * b) this wrapper is a splitof of a wrapper built in create_wrappers
     *    and a specialisation was assigned to the splitof part of the
     *    generic wrapper.
     *
     * In both cases, we have to make sure that the wrapper gets the
     * new namespace AND is made local, so that it will be compiled
     * and serialized.
     */
    if (FUNDEF_SPECNS (arg_node) != NULL) {
        FUNDEF_NS (arg_node) = NSfreeNamespace (FUNDEF_NS (arg_node));
        FUNDEF_NS (arg_node) = FUNDEF_SPECNS (arg_node);
        FUNDEF_SPECNS (arg_node) = NULL;

        FUNDEF_ISLOCAL (arg_node) = TRUE;
        FUNDEF_WASUSED (arg_node) = FALSE;
        FUNDEF_WASIMPORTED (arg_node) = FALSE;
        FUNDEF_SYMBOLNAME (arg_node) = ILIBfree (FUNDEF_SYMBOLNAME (arg_node));
    }

    DBUG_RETURN (arg_node);
}

node *
SWRfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWRfundef");

    if (INFO_SWR_TRAVNO (arg_info) == 1) {
        /*
         * first traversal -> build wrapper functions and their bodies
         */
        arg_node = FundefBuildWrappers (arg_node, arg_info);
    } else if (INFO_SWR_TRAVNO (arg_info) == 2) {
        /*
         * second traversal -> adjust all AP_FUNDEF pointers
         *
         * This is needed if the original wrapper function was valid for more than
         * a single base type.
         */
        arg_node = FundefAdjustPointers (arg_node, arg_info);
    } else {
        DBUG_ASSERT ((INFO_SWR_TRAVNO (arg_info) == 3), "illegal INFO_SWR_TRAVNO found!");
        /*
         * third traversal -> move all wrappers to their final ns and mark them
         *                    as local if they contain specialisations
         *
         *                    remove zombies and empty wrappers
         */

        arg_node = FundefMoveToFinalNs (arg_node, arg_info);
        arg_node = FundefRemoveGarbage (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *SWRap( node *arg_node, info *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
SWRap (node *arg_node, info *arg_info)
{
    ntype *arg_types;

    DBUG_ENTER ("SWRap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    DBUG_PRINT ("SWR",
                ("Ap of function %s::%s pointed to " F_PTR ".",
                 NSgetName (AP_NS (arg_node)), AP_NAME (arg_node), AP_FUNDEF (arg_node)));

    arg_types = TUactualArgs2Ntype (AP_ARGS (arg_node));
    AP_FUNDEF (arg_node) = CorrectFundefPointer (AP_FUNDEF (arg_node), arg_types);

    DBUG_PRINT ("SWR",
                ("Ap of function %s::%s now points to " F_PTR ".",
                 NSgetName (AP_NS (arg_node)), AP_NAME (arg_node), AP_FUNDEF (arg_node)));
    arg_types = TYfreeType (arg_types);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *SWRwith( node *arg_node, info *arg_info)
 *
 * @brief inserts actual with node into the info structure
 *
 ******************************************************************************/

node *
SWRwith (node *arg_node, info *arg_info)
{
    node *old_with;
    DBUG_ENTER ("SWRwith");

    old_with = INFO_SWR_WITH (arg_info);
    INFO_SWR_WITH (arg_info) = arg_node;

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    INFO_SWR_WITH (arg_info) = old_with;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *SWRgenarray( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
SWRgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWRgenarrray");

    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);
    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
SWRfold (node *arg_node, info *arg_info)
{
    ntype *neutr_type, *body_type;
    ntype *arg_type, *arg_types;

    DBUG_ENTER ("SWRfold");

    FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

    neutr_type = TYfixAndEliminateAlpha (AVIS_TYPE (ID_AVIS (FOLD_NEUTRAL (arg_node))));
    body_type = TYfixAndEliminateAlpha (
      AVIS_TYPE (ID_AVIS (WITH_CEXPR (INFO_SWR_WITH (arg_info)))));

    arg_type = TYlubOfTypes (neutr_type, body_type);
    arg_types = TYmakeProductType (2, arg_type, TYcopyType (arg_type));

    FOLD_FUNDEF (arg_node) = CorrectFundefPointer (FOLD_FUNDEF (arg_node), arg_types);
    arg_types = TYfreeType (arg_types);
    body_type = TYfreeType (body_type);
    neutr_type = TYfreeType (neutr_type);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *SWRdoSplitWrappers( node *ast)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
SWRdoSplitWrappers (node *ast)
{
    info *info_node;

    DBUG_ENTER ("SWRdoSplitWrappers");

    TRAVpush (TR_swr);

    info_node = MakeInfo ();
    ast = TRAVdo (ast, info_node);
    info_node = FreeInfo (info_node);

    TRAVpop ();

    /*
     * for the time being, the wrapper types are
     * not beeing split properly. See the comment
     * for SplitWrapperType for details. To ensure
     * that the wrappertypes are really clean, we do
     * a UWTdoUpdateWrapperType to rebuild them.
     */
    ast = UWTdoUpdateWrapperType (ast);

    /**
     * Whether the following traversal are required or not is by no means
     * clear. However, previously, we had a call to NT2OT here which has
     * been split into EAT and EBT now.
     * FIXME: this needs to be investigated further....
     */
    ast = EATdoEliminateAlphaTypes (ast);
    ast = EBTdoEliminateBottomTypes (ast);

    DBUG_RETURN (ast);
}
