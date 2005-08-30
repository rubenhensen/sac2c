/*
 *
 * $Log$
 * Revision 1.45  2005/08/30 20:23:14  ktr
 * AVIS nodes without AVIS_TYPE are now equipped with a default type (int).
 * This is important as dead variables can arise during the opt cycle.
 *
 * Revision 1.44  2005/08/29 11:25:21  ktr
 * NTC may now run in the optimization cycle
 *
 * Revision 1.43  2005/08/11 13:50:39  sbs
 * changed the lifting of bottom types when type_error applications are built
 * Now, the result type is at least AKS
 *
 * Revision 1.42  2005/08/10 19:08:46  sbs
 * frees now te_infos as well.
 *
 * Revision 1.41  2005/07/26 16:03:07  sbs
 * frees the tvar and sig_dep heaps now.
 *
 * Revision 1.40  2005/07/26 14:32:08  sah
 * moved creation of special fold funs to
 * dispatchfuncall as new2old is running
 * prior to the module system which again relies
 * on the fact that no foldfuns have been
 * created, yet.
 *
 * Revision 1.39  2005/07/26 12:43:21  sah
 * new2old no longer removes casts
 *
 * Revision 1.38  2005/07/20 14:31:25  sbs
 * reflagged lac funs into lacinline funs whenever the code is transformed
 * due to bottom types
 *
 * Revision 1.37  2005/07/12 10:22:48  sah
 * renamed CreateTypeErrorBody to ReplaceBodyByTypeError
 * as it now handles an entire fundef and sets the
 * FUNDEF_RETURN properly.
 *
 * Revision 1.36  2005/06/27 22:28:10  sacbase
 * another typo fixed
 *
 * Revision 1.35  2005/06/14 23:40:58  sbs
 * CreateTypeErrorBody bigs fixed
 *
 * Revision 1.34  2005/06/14 09:55:10  sbs
 * support for bottom types integrated.
 *
 * Revision 1.33  2005/06/04 20:11:42  sbs
 * type conversion from new to old type disabled by if 0's
 *
 * Revision 1.32  2005/03/20 00:22:16  sbs
 * NT2OTpart added.
 * insertion of scalarised index vectors corrected for default partitions.
 *
 * Revision 1.31  2005/01/14 12:31:57  cg
 * Converted error handling to ctinfo.c
 *
 * Revision 1.30  2004/12/19 13:32:56  sbs
 * now, new2old is called on the lifted fold funs as well
 *
 * Revision 1.29  2004/12/13 18:43:47  ktr
 * NT2OTwithid will only perform syntax tree modification as long as
 * withid is given by N_ids nodes ( before explicit allocation)
 *
 * Revision 1.28  2004/12/08 18:00:11  ktr
 * removed ARRAY_TYPE/ARRAY_NTYPE
 *
 * Revision 1.27  2004/12/07 15:47:38  sbs
 * NT2OTwithid fixed.
 *
 * Revision 1.26  2004/11/27 01:25:32  khf
 * adjusted name of start function
 *
 * Revision 1.25  2004/11/24 17:42:48  sbs
 * not yet
 *
 * Revision 1.24  2004/11/22 13:54:09  sbs
 * changed NT2OTwithop into NT2OTfold
 *
 * Revision 1.23  2004/10/26 09:38:38  sah
 * TYFixAndEliminateAlpha is called on funtion types now as well
 *
 * Revision 1.22  2004/09/27 19:07:18  sbs
 * FUNDEF_RET_TYPES can be replaced by fixed types as all sharing
 * has been eliminated (cf. create_wrapper_code.c)
 *
 * Revision 1.21  2004/09/27 14:07:33  sbs
 * tried to replace FUNDEF_RET_TYPE with fixed type - wont work
 * => commented out again....
 *
 * Revision 1.20  2004/08/26 18:15:01  sbs
 * fixed a bug in CreateFoldFun:
 * now, the correct signature is created.
 * => bug48.
 *
 * Revision 1.19  2004/08/08 13:30:50  sbs
 * some debugging information added.
 *
 * Revision 1.18  2004/07/30 17:29:21  sbs
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.17  2004/07/05 17:23:43  sbs
 * now, we do not only compute old types from ntypes, but we also
 * fix the ntype type variables for proper use later on.
 * Unfortunately, at the time being, cwc introduces some strange sharing
 * which requires the type var's not to be freed (UGLY!!) these places
 * are marked by CWC_WOULD_BE_PROPER macros 8-)
 *
 * Revision 1.16  2004/02/20 08:14:00  mwe
 * now functions with and without body are separated
 * changed tree traversal (added traverse of MODUL_FUNDECS)
 *
 * Revision 1.15  2003/09/25 15:20:14  dkr
 * no changes done
 *
 * Revision 1.14  2002/10/31 16:15:26  sbs
 * ... for return values now is preserved 8-)))
 *
 * Revision 1.13  2002/10/30 13:23:59  sbs
 * handling of dot args introduced.
 *
 * Revision 1.12  2002/10/29 19:06:28  dkr
 * bug in NT2OTwithid() fixed: TYFixAndEliminateAlpha() used now
 *
 * Revision 1.11  2002/10/28 14:54:55  sbs
 * NT2OTcast added.
 *
 * Revision 1.10  2002/10/24 14:09:35  sbs
 * bug in vardec handling fixed
 *
 * Revision 1.9  2002/10/23 06:35:39  sbs
 * NT2OTwithid added. It inserts scalar index variables whenever possible now.
 *
 * Revision 1.8  2002/10/08 16:36:24  dkr
 * NT2OTreturn() removed
 *
 * Revision 1.7  2002/10/08 16:32:45  dkr
 * FUNDEF_RETURN is inferned by new_typecheck.c already...
 *
 * Revision 1.6  2002/10/08 10:35:35  dkr
 * - infers FUNDEF_RETURN now
 * - pseudo fold funs are created
 *
 * Revision 1.5  2002/09/04 16:19:01  dkr
 * NT2OTfundef: DBUG_ASSERT added
 *
 * Revision 1.4  2002/09/02 12:38:55  dkr
 * new_typecheck.h included
 *
 * Revision 1.3  2002/08/31 04:56:27  dkr
 * NT2OTarray added: ARRAY_TYPE is set now
 *
 * Revision 1.2  2002/08/13 12:19:04  dkr
 * NT2OTarg added
 * NT2OTfundef modified: wrapper funs are handled like normal functions
 * now
 *
 * Revision 1.1  2002/08/05 16:58:31  sbs
 * Initial revision
 *
 */

#include <stdio.h>
#include <string.h>
#include "new2old.h"

#include "dbug.h"
#include "ctinfo.h"
#include "internal_lib.h"
#include "free.h"
#include "DupTree.h"
#include "shape.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse_helper.h"

#include "gen_pseudo_fun.h"
#include "new_typecheck.h"
#include "new_types.h"
#include "type_utils.h"
#include "type_errors.h"
#include "ssi.h"
#include "sig_deps.h"

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
 * This phase does NOT ONLY compute old types from the new ones but
 * IT ALSO fixes the ntypes, i.e., it eliminates type vars!!!!!!
 */

/*
 * usages of 'arg_info':
 *
 *   INFO_VARDECS    -   list of the generated vardecs
 *   INFO_WLIDS      -   WITHID_VEC of first partition
 *   INFO_THENBOTTS  -   type_error let
 *   INFO_ELSEBOTTS  -   type_error let
 *   INFO_ONEFUNCTION - traverse one function only (and associated lacfuns)
 *   INFO_FUNDEF     - pointer to current fundef to prevent infinite recursion
 *   INFO_VARDECMODE -   M_fix / M_filter
 */

/**
 * INFO structure
 */
struct INFO {
    node *vardecs;
    node *wlids;
    node *then_botts;
    node *else_botts;
    bool onefunction;
    node *fundef;
    enum { M_fix, M_filter } mode;
};

/**
 * INFO macros
 */
#define INFO_VARDECS(n) (n->vardecs)
#define INFO_WLIDS(n) (n->wlids)
#define INFO_THENBOTTS(n) (n->then_botts)
#define INFO_ELSEBOTTS(n) (n->else_botts)
#define INFO_ONEFUNCTION(n) (n->onefunction)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_VARDECMODE(n) (n->mode)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_VARDECS (result) = NULL;
    INFO_WLIDS (result) = NULL;
    INFO_THENBOTTS (result) = NULL;
    INFO_ELSEBOTTS (result) = NULL;
    INFO_ONEFUNCTION (result) = FALSE;
    INFO_FUNDEF (result) = NULL;
    INFO_VARDECMODE (result) = M_fix;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

static bool
IdsContainBottom (node *ids)
{
    bool res = FALSE;

    DBUG_ENTER ("IdsContainBottom");

    while (ids != NULL) {
        res = res || TYisBottom (AVIS_TYPE (IDS_AVIS (ids)));
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (res);
}

static node *
AddTypeError (node *assign, node *bottom_id, ntype *other_type)
{
    node *ids;
    DBUG_ENTER ("AddTypeError");

    if (assign == NULL) {
        /**
         * No errors yet : create an assignment of the form
         *
         *   bottom_id = type_error( err_msg);
         *
         */
        assign
          = TBmakeAssign (TBmakeLet (TBmakeIds (ID_AVIS (bottom_id), NULL),
                                     TCmakePrf1 (F_type_error,
                                                 TCmakeStrCopy (TYgetBottomError (
                                                   AVIS_TYPE (ID_AVIS (bottom_id)))))),
                          NULL);

    } else {
        /**
         * We have seen other errors before, add a new LHS
         */
        ids = LET_IDS (ASSIGN_INSTR (assign));
        ids = TBmakeIds (ID_AVIS (bottom_id), ids);
        LET_IDS (ASSIGN_INSTR (assign)) = ids;
    }
    /**
     * Finally, we change the type of bottom_id to other_type.
     */
    AVIS_TYPE (ID_AVIS (bottom_id)) = TYfreeType (AVIS_TYPE (ID_AVIS (bottom_id)));
    AVIS_TYPE (ID_AVIS (bottom_id)) = TYeliminateAKV (other_type);
    /**
     * and we eliminate the defining N_let:
     */
    DBUG_ASSERT ((AVIS_SSAASSIGN (ID_AVIS (bottom_id)) != NULL),
                 "missing AVIS_SSAASSIGN!");
    DBUG_ASSERT ((NODE_TYPE (AVIS_SSAASSIGN (ID_AVIS (bottom_id))) == N_assign),
                 "AVIS_SSAASSIGN points to non N_assign node!");
    DBUG_ASSERT ((NODE_TYPE (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (bottom_id))))
                  == N_let),
                 "AVIS_SSAASSIGN does not point to an N_let assignment!");

    ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (bottom_id)))
      = FREEdoFreeTree (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (bottom_id))));

    DBUG_RETURN (assign);
}

static node *
ReplaceBodyByTypeError (node *fundef, ntype *inferred_type, ntype *res_type)
{
    node *block, *avis;
    node *vardecs = NULL;
    node *ids = NULL;
    node *exprs = NULL;
    node *ret = NULL;
    char *err_msg = NULL;
    int i, n;
    char *tmp;

    DBUG_ENTER ("ReplaceBodyByTypeError");

    n = TYgetProductSize (res_type);
    for (i = n - 1; i >= 0; i--) {
        avis = TBmakeAvis (ILIBtmpVar (), TYcopyType (TYgetProductMember (res_type, i)));
        if (TYisBottom (TYgetProductMember (inferred_type, i))) {
            if (err_msg == NULL) {
                err_msg = ILIBstringCopy (
                  TYgetBottomError (TYgetProductMember (inferred_type, i)));
            } else {
                tmp
                  = ILIBstringConcat (err_msg, TYgetBottomError (
                                                 TYgetProductMember (inferred_type, i)));
                err_msg = ILIBfree (err_msg);
                err_msg = tmp;
            }
        }
        vardecs = TBmakeVardec (avis, vardecs);
        ids = TBmakeIds (avis, ids);
        exprs = TBmakeExprs (TBmakeId (avis), exprs);
    }

    ret = TBmakeReturn (exprs);

    block = TBmakeBlock (TBmakeAssign (TBmakeLet (ids, TCmakePrf1 (F_type_error,
                                                                   TBmakeStr (err_msg))),
                                       TBmakeAssign (ret, NULL)),
                         vardecs);

    FUNDEF_BODY (fundef) = FREEdoFreeTree (FUNDEF_BODY (fundef));

    FUNDEF_BODY (fundef) = block;
    FUNDEF_RETURN (fundef) = ret;

    FUNDEF_RETS (fundef) = TUreplaceRetTypes (FUNDEF_RETS (fundef), res_type);

    /**
     * finally, we need to make sure that this function is tagged appropriately
     * As it does not have a body anymore, we need to remove potential LAC flags
     * and ensure the code will be inlined.
     */
    if (FUNDEF_ISLACFUN (fundef)) {
        FUNDEF_ISDOFUN (fundef) = FALSE;
        FUNDEF_ISCONDFUN (fundef) = FALSE;
        FUNDEF_ISLACINLINE (fundef) = TRUE;
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTdoTransform( node *arg_node)
 *
 * description:
 *   adjusts all old vardec types according to the attached ntypes!
 *
 ******************************************************************************/

node *
NT2OTdoTransform (node *arg_node)
{
    info *info_node;

    DBUG_ENTER ("NT2OTdoTransform");

    TRAVpush (TR_nt2ot);

    info_node = MakeInfo ();
    arg_node = TRAVdo (arg_node, info_node);
    info_node = FreeInfo (info_node);

    TRAVpop ();

    /**
     * Since all alpha types are gone now, we may may free all tvars, all
     *  sig_deps, and all te_infos:
     */
    SSIfreeAllTvars ();
    SDfreeAllSignatureDependencies ();
    TEfreeAllTypeErrorInfos ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTdoTransformOneFunction( node *arg_node)
 *
 * description:
 *   adjusts all old vardec types according to the attached ntypes!
 *
 ******************************************************************************/

node *
NT2OTdoTransformOneFunction (node *arg_node)
{
    info *info_node;

    DBUG_ENTER ("NT2OTdoTransformOneFunction");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 "NT2OTdoTransformOneFunction can only be applied to fundefs");

    if (!FUNDEF_ISLACFUN (arg_node)) {
        TRAVpush (TR_nt2ot);

        info_node = MakeInfo ();
        INFO_ONEFUNCTION (info_node) = TRUE;
        arg_node = TRAVdo (arg_node, info_node);
        info_node = FreeInfo (info_node);

        TRAVpop ();

        /**
         * Since all alpha types are gone now, we may may free all tvars, all
         *  sig_deps, and all te_infos:
         */
        SSIfreeAllTvars ();
        SDfreeAllSignatureDependencies ();
        TEfreeAllTypeErrorInfos ();
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTmodule( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("NT2OTmodule");

    if (MODULE_IMPORTS (arg_node) != NULL) {
        MODULE_IMPORTS (arg_node) = TRAVdo (MODULE_IMPORTS (arg_node), arg_info);
    }

    if (MODULE_TYPES (arg_node) != NULL) {
        MODULE_TYPES (arg_node) = TRAVdo (MODULE_TYPES (arg_node), arg_info);
    }

    if (MODULE_OBJS (arg_node) != NULL) {
        MODULE_OBJS (arg_node) = TRAVdo (MODULE_OBJS (arg_node), arg_info);
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
 *   node *NT2OTfundef( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTfundef (node *arg_node, info *arg_info)
{
    ntype *otype, *ftype, *fltype, *bottom;

    DBUG_ENTER ("NT2OTfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    otype = TUmakeProductTypeFromRets (FUNDEF_RETS (arg_node));
    DBUG_ASSERT ((otype != NULL), "FUNDEF_RET_TYPE not found!");
    ftype = TYfixAndEliminateAlpha (otype);
    FUNDEF_RETS (arg_node) = TUreplaceRetTypes (FUNDEF_RETS (arg_node), ftype);

    /* process the real function type as well */
    if (FUNDEF_WRAPPERTYPE (arg_node) != NULL) {
        ntype *funtype = TYfixAndEliminateAlpha (FUNDEF_WRAPPERTYPE (arg_node));
        FUNDEF_WRAPPERTYPE (arg_node) = TYfreeType (FUNDEF_WRAPPERTYPE (arg_node));
        FUNDEF_WRAPPERTYPE (arg_node) = funtype;
    }

    if (TYcountNoMinAlpha (ftype) > 0) {

        if (FUNDEF_ISPROVIDED (arg_node) || FUNDEF_ISEXPORTED (arg_node)) {
            CTIabortLine (NODE_LINE (arg_node),
                          "One component of inferred return type (%s) has no lower bound;"
                          " an application of \"%s\" will not terminate",
                          TYtype2String (ftype, FALSE, 0), FUNDEF_NAME (arg_node));
        } else {
            DBUG_PRINT ("FIXNT", ("eliminating function %s due to lacking result type",
                                  FUNDEF_NAME (arg_node)));
            arg_node = FREEdoFreeNode (arg_node);
        }
    } else {
        bottom = TYgetBottom (ftype);
        if (bottom != NULL) {
            DBUG_PRINT ("FIXNT", ("bottom component found in function %s",
                                  FUNDEF_NAME (arg_node)));
            if (ILIBstringCompare (FUNDEF_NAME (arg_node), "main")
                || FUNDEF_ISPROVIDED (arg_node) || FUNDEF_ISEXPORTED (arg_node)) {
                CTIabortOnBottom (TYgetBottomError (bottom));
            } else {
                /**
                 * we transform the entire body into one 'type error' assignment
                 */
                fltype = TYliftBottomFixAndEliminateAlpha (otype);

                arg_node = ReplaceBodyByTypeError (arg_node, ftype, fltype);
            }

        } else {
            DBUG_ASSERT (TYisProdOfArray (ftype), "inconsistent return type found");
            DBUG_PRINT ("FIXNT", ("ProdOfArray return type found for function %s",
                                  FUNDEF_NAME (arg_node)));

            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
            }

            INFO_VARDECS (arg_info) = NULL;
            if (FUNDEF_BODY (arg_node) != NULL) {
                FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            }

            if (INFO_VARDECS (arg_info) != NULL) {
                /*
                 * if some new vardecs have been built, insert them !
                 * this has to be done here rather than in NT2OTblock since blocks
                 * in general may not be top level.
                 * AFAIK, the only place where vardecs are in fact built is the
                 * extension of withloop ids in NT2OTwithid.
                 */
                INFO_VARDECS (arg_info) = TRAVdo (INFO_VARDECS (arg_info), arg_info);

                FUNDEF_VARDEC (arg_node)
                  = TCappendVardec (INFO_VARDECS (arg_info), FUNDEF_VARDEC (arg_node));
                INFO_VARDECS (arg_info) = NULL;
            }
            if (FUNDEF_ISDOFUN (arg_node) && INFO_THENBOTTS (arg_info) != NULL) {
                FUNDEF_ISDOFUN (arg_node) = FALSE;
                FUNDEF_ISLACINLINE (arg_node) = TRUE;
            }
        }
    }

    INFO_THENBOTTS (arg_info) = NULL;
    INFO_ELSEBOTTS (arg_info) = NULL;

    if ((!INFO_ONEFUNCTION (arg_info)) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *NT2OTap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
NT2OTap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("NT2OTap");

    arg_node = TRAVcont (arg_node, arg_info);

    if (INFO_ONEFUNCTION (arg_info) && FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        info *new_info = MakeInfo ();
        INFO_ONEFUNCTION (new_info) = TRUE;
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), new_info);
        new_info = FreeInfo (new_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTavis( node *arg_node, info *arg_info)
 *
 * description:
 *   Fix (!) the ntype.
 *
 ******************************************************************************/

node *
NT2OTavis (node *arg_node, info *arg_info)
{
    ntype *type;
#ifndef DBUG_OFF
    char *tmp_str, *tmp_str2;
#endif

    DBUG_ENTER ("NT2OTarg");

    type = AVIS_TYPE (arg_node);

    if (type != NULL) {

        DBUG_EXECUTE ("FIXNT", tmp_str = TYtype2String (type, FALSE, 0););
        DBUG_PRINT ("FIXNT", ("replacing argument/vardec %s\'s type %s by ...",
                              AVIS_NAME (arg_node), tmp_str));
        type = TYfixAndEliminateAlpha (type);
        DBUG_EXECUTE ("FIXNT", tmp_str2 = TYtype2String (type, FALSE, 0););
#if CWC_WOULD_BE_PROPER
        AVIS_TYPE (arg_node) = TYfreeType (AVIS_TYPE (arg_node));
#endif
        AVIS_TYPE (arg_node) = type;
        DBUG_PRINT ("FIXNT", ("... %s", tmp_str2));
        DBUG_EXECUTE ("FIXNT", tmp_str = ILIBfree (tmp_str);
                      tmp_str2 = ILIBfree (tmp_str2););

        if (!(TYisArray (type) || TYisBottom (type))) {
            CTIabort ("Could not infer proper type for arg %s", ARG_NAME (arg_node));
        }
    } else {
        /*
         * AVIS_TYPE CAN BE NULL iff nt2ot is run in the opt cycle
         */
        AVIS_TYPE (arg_node) = TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTblock( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("NT2OTblock");

    INFO_VARDECMODE (arg_info) = M_fix;
    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    INFO_VARDECMODE (arg_info) = M_filter;
    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTvardec( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTvardec (node *arg_node, info *arg_info)
{
    node *this_vardec;

    DBUG_ENTER ("NT2OTvardec");

    if (INFO_VARDECMODE (arg_info) == M_fix) {
        VARDEC_AVIS (arg_node) = TRAVdo (VARDEC_AVIS (arg_node), arg_info);
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    if (INFO_VARDECMODE (arg_info) == M_filter) {
        if (TYisBottom (VARDEC_NTYPE (arg_node))) {
            /**
             * eliminate this vardec completely!
             */
            DBUG_PRINT ("FIXNT",
                        ("eliminating bottom vardec for %s", VARDEC_NAME (arg_node)));
            this_vardec = arg_node;
            arg_node = VARDEC_NEXT (arg_node);
            VARDEC_NEXT (this_vardec) = NULL;
            this_vardec = FREEdoFreeTree (this_vardec);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTlet( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("NT2OTlet");

    if (IdsContainBottom (LET_IDS (arg_node))) {
        DBUG_PRINT ("FIXNT", ("bottom LHS found; eliminating N_let \"%s...\"",
                              IDS_NAME (LET_IDS (arg_node))));
        arg_node = FREEdoFreeTree (arg_node);
    } else {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *NT2OTassign( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
NT2OTassign (node *arg_node, info *arg_info)
{
    node *this_assign;
    DBUG_ENTER ("NT2OTassign");

    /**
     * First we go down in order to collect those funcond vars that need to be
     * created by the prf 'type_error'.
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /**
     * Now, we can go into the instruction. In case of N_lets that have bottom
     * vars on their LHS, we will obtain NULL, which signals us to eliminate
     * this very N_assign.
     */
    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }
    if (ASSIGN_INSTR (arg_node) == NULL) {
        this_assign = arg_node;
        arg_node = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (this_assign) = NULL;
        this_assign = FREEdoFreeNode (this_assign);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *NT2OTcond( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
NT2OTcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("NT2OTcond");

    if ((INFO_THENBOTTS (arg_info) != NULL) && (INFO_ELSEBOTTS (arg_info) != NULL)) {
        /**
         * There are errors in both branches, i.e., abort
         */
        CTIabortLine (global.linenum, "Conditional with type errors in both branches");
    }
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    if (INFO_THENBOTTS (arg_info) != NULL) {
        ASSIGN_NEXT (INFO_THENBOTTS (arg_info)) = BLOCK_INSTR (COND_THEN (arg_node));
        BLOCK_INSTR (COND_THEN (arg_node)) = INFO_THENBOTTS (arg_info);
    }
    if (INFO_ELSEBOTTS (arg_info) != NULL) {
        ASSIGN_NEXT (INFO_ELSEBOTTS (arg_info)) = BLOCK_INSTR (COND_ELSE (arg_node));
        BLOCK_INSTR (COND_ELSE (arg_node)) = INFO_ELSEBOTTS (arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *NT2OTfuncond( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
NT2OTfuncond (node *arg_node, info *arg_info)
{
    ntype *ttype, *etype;

    DBUG_ENTER ("NT2OTfuncond");

    ttype = AVIS_TYPE (ID_AVIS (FUNCOND_THEN (arg_node)));
    etype = AVIS_TYPE (ID_AVIS (FUNCOND_ELSE (arg_node)));

    if (TYisBottom (ttype)) {
        DBUG_ASSERT (!TYisBottom (etype), "two bottom args for funcond found");
        INFO_THENBOTTS (arg_info)
          = AddTypeError (INFO_THENBOTTS (arg_info), FUNCOND_THEN (arg_node), etype);
    }
    if (TYisBottom (etype)) {
        DBUG_ASSERT (!TYisBottom (ttype), "two bottom args for funcond found");
        INFO_ELSEBOTTS (arg_info)
          = AddTypeError (INFO_ELSEBOTTS (arg_info), FUNCOND_ELSE (arg_node), ttype);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *NT2OTpart( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
NT2OTpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("NT2OTpart");

    PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);

    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    } else {
        INFO_WLIDS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTwithid( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTwithid (node *arg_node, info *arg_info)
{
    node *new_ids, *tmp_avis;
    node *new_vardecs;
    int i, num_vars;
    ntype *vec_type;

    DBUG_ENTER ("NT2OTwithid");

    if (INFO_WLIDS (arg_info) == NULL) {
        vec_type = AVIS_TYPE (IDS_AVIS (WITHID_VEC (arg_node)));
        vec_type = TYfixAndEliminateAlpha (vec_type);

        if (WITHID_IDS (arg_node) == NULL) {
            if (TYisAKS (vec_type)) {
                DBUG_PRINT ("NT2OT", ("WITHID_IDS for %s built",
                                      IDS_NAME (WITHID_VEC (arg_node))));
                num_vars = SHgetExtent (TYgetShape (vec_type), 0);
                new_ids = NULL;
                new_vardecs = INFO_VARDECS (arg_info);
                for (i = 0; i < num_vars; i++) {
                    tmp_avis
                      = TBmakeAvis (ILIBtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                              SHcreateShape (0)));
                    new_vardecs = TBmakeVardec (tmp_avis, new_vardecs);
                    new_ids = TBmakeIds (tmp_avis, new_ids);
                }
                WITHID_IDS (arg_node) = new_ids;
                INFO_WLIDS (arg_info) = new_ids;
                INFO_VARDECS (arg_info) = new_vardecs;
            } else {
                DBUG_PRINT ("NT2OT", ("no WITHID_IDS for %s built",
                                      IDS_NAME (WITHID_VEC (arg_node))));
            }
        } else {
            INFO_WLIDS (arg_info) = WITHID_IDS (arg_node);
        }
    } else {
        /**
         * we are dealing with a default partition here => Duplicate those of the real
         * one!
         */
        WITHID_IDS (arg_node) = DUPdoDupTree (INFO_WLIDS (arg_info));
    }

    DBUG_RETURN (arg_node);
}
