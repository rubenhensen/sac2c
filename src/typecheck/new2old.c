/*
 *
 * $Log$
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

#define NEW_INFO

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

#include "gen_pseudo_fun.h"
#include "new_typecheck.h"
#include "new_types.h"
#include "type_utils.h"

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
 *   INFO_NT2OT_FOLDFUNS   -   list of the generated fold funs
 *   INFO_NT2OT_LAST_LET   -   poiner to the last N_let node
 *   INFO_NT2OT_VARDECS    -   list of the generated vardecs
 *   INFO_NT2OT_WLIDS      -   WITHID_VEC of first partition
 */

/**
 * INFO structure
 */
struct INFO {
    node *foldfuns;
    node *last_let;
    node *vardecs;
    node *wlids;
};

/**
 * INFO macros
 */
#define INFO_NT2OT_FOLDFUNS(n) (n->foldfuns)
#define INFO_NT2OT_LAST_LET(n) (n->last_let)
#define INFO_NT2OT_VARDECS(n) (n->vardecs)
#define INFO_NT2OT_WLIDS(n) (n->wlids)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_NT2OT_FOLDFUNS (result) = NULL;
    INFO_NT2OT_LAST_LET (result) = NULL;
    INFO_NT2OT_VARDECS (result) = NULL;
    INFO_NT2OT_WLIDS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
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
    node *tmp;

    DBUG_ENTER ("NT2OTmodule");

    INFO_NT2OT_FOLDFUNS (arg_info) = NULL;

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

    if (INFO_NT2OT_FOLDFUNS (arg_info) != NULL) {
        MODULE_FUNDECS (arg_node)
          = TCappendFundef (INFO_NT2OT_FOLDFUNS (arg_info), MODULE_FUNDECS (arg_node));
    }

    INFO_NT2OT_FOLDFUNS (arg_info) = NULL;

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    while (INFO_NT2OT_FOLDFUNS (arg_info) != NULL) {
        tmp = INFO_NT2OT_FOLDFUNS (arg_info);
        INFO_NT2OT_FOLDFUNS (arg_info) = NULL;
        tmp = TRAVdo (tmp, arg_info);
        MODULE_FUNS (arg_node) = TCappendFundef (tmp, MODULE_FUNS (arg_node));
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
    ntype *type;
    types *old_type;

    DBUG_ENTER ("NT2OTfundef");

    type = TUmakeProductTypeFromRets (FUNDEF_RETS (arg_node));
    DBUG_ASSERT ((type != NULL), "FUNDEF_RET_TYPE not found!");
    type = TYfixAndEliminateAlpha (type);
    FUNDEF_RETS (arg_node) = TUreplaceRetTypes (FUNDEF_RETS (arg_node), type);

    /* process the real function type as well */
    if (FUNDEF_WRAPPERTYPE (arg_node) != NULL) {
        ntype *funtype = TYfixAndEliminateAlpha (FUNDEF_WRAPPERTYPE (arg_node));
        FUNDEF_WRAPPERTYPE (arg_node) = TYfreeType (FUNDEF_WRAPPERTYPE (arg_node));
        FUNDEF_WRAPPERTYPE (arg_node) = funtype;
    }

    if (TYisProdOfArray (type)) {
        old_type = FUNDEF_TYPES (arg_node);
        FUNDEF_TYPES (arg_node) = TYtype2OldType (type);
        old_type = FREEfreeAllTypes (old_type);

    } else {
        CTIabort ("Could not infer proper type for fun %s; type found: %s",
                  FUNDEF_NAME (arg_node), TYtype2String (type, FALSE, 0));
    }

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    INFO_NT2OT_VARDECS (arg_info) = NULL;
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (INFO_NT2OT_VARDECS (arg_info) != NULL) {
        /*
         * if some new vardecs have been built, insert them !
         * this has to be done here rather than in NT2OTblock since blocks
         * in general may not be top level.
         * AFAIK, the only place where vardecs are in fact built is the
         * extension of withloop ids in NT2OTwithid.
         */
        INFO_NT2OT_VARDECS (arg_info) = TRAVdo (INFO_NT2OT_VARDECS (arg_info), arg_info);

        FUNDEF_VARDEC (arg_node)
          = TCappendVardec (INFO_NT2OT_VARDECS (arg_info), FUNDEF_VARDEC (arg_node));
        INFO_NT2OT_VARDECS (arg_info) = NULL;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTarg( node *arg_node, info *arg_info)
 *
 * description:
 *   Fix (!) the ntype and compute the corresponding old type.
 *
 ******************************************************************************/

node *
NT2OTarg (node *arg_node, info *arg_info)
{
    ntype *type;
#ifndef DBUG_OFF
    char *tmp_str, *tmp_str2;
#endif

    DBUG_ENTER ("NT2OTarg");

    type = AVIS_TYPE (ARG_AVIS (arg_node));

    if (type != NULL) {
        DBUG_EXECUTE ("FIXNT", tmp_str = TYtype2String (type, FALSE, 0););
        DBUG_PRINT ("FIXNT", ("replacing argument %s\'s type %s by ...",
                              ARG_NAME (arg_node), tmp_str));
        type = TYfixAndEliminateAlpha (type);
        DBUG_EXECUTE ("FIXNT", tmp_str2 = TYtype2String (type, FALSE, 0););
#if CWC_WOULD_BE_PROPER
        AVIS_TYPE (ARG_AVIS (arg_node)) = TYfreeType (AVIS_TYPE (ARG_AVIS (arg_node)));
#endif
        AVIS_TYPE (ARG_AVIS (arg_node)) = type;
        DBUG_PRINT ("FIXNT", ("... %s", tmp_str2));
        DBUG_EXECUTE ("FIXNT", tmp_str = ILIBfree (tmp_str);
                      tmp_str2 = ILIBfree (tmp_str2););

        if (TYisArray (type)) {
            ARG_TYPE (arg_node) = FREEfreeAllTypes (ARG_TYPE (arg_node));
            ARG_TYPE (arg_node) = TYtype2OldType (type);
        } else {
            CTIabort ("Could not infer proper type for arg %s", ARG_NAME (arg_node));
        }

        if (ARG_NEXT (arg_node) != NULL) {
            ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
        }

    } else {
        if ((ARG_TYPE (arg_node) != NULL)
            && (TYPES_BASETYPE (ARG_TYPE (arg_node)) != T_dots)) {
            CTIabort ("Could not infer proper type for arg %s", ARG_NAME (arg_node));
        }
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

    /*
     * the VARDECs must be traversed first!!
     */
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
 *   node *NT2OTvardec( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTvardec (node *arg_node, info *arg_info)
{
    ntype *type;
#ifndef DBUG_OFF
    char *tmp_str, *tmp_str2;
#endif

    DBUG_ENTER ("NT2OTvardec");

    type = AVIS_TYPE (VARDEC_AVIS (arg_node));

    if (type != NULL) {
        DBUG_EXECUTE ("FIXNT", tmp_str = TYtype2String (type, FALSE, 0););
        DBUG_PRINT ("FIXNT", ("replacing vardec %s\'s type %s by ...",
                              VARDEC_NAME (arg_node), tmp_str));
        type = TYfixAndEliminateAlpha (type);
        DBUG_EXECUTE ("FIXNT", tmp_str2 = TYtype2String (type, FALSE, 0););
#if CWC_WOULD_BE_PROPER
        AVIS_TYPE (VARDEC_AVIS (arg_node))
          = TYfreeType (AVIS_TYPE (VARDEC_AVIS (arg_node)));
#endif
        AVIS_TYPE (VARDEC_AVIS (arg_node)) = type;
        DBUG_PRINT ("FIXNT", ("... %s", tmp_str2));
        DBUG_EXECUTE ("FIXNT", tmp_str = ILIBfree (tmp_str);
                      tmp_str2 = ILIBfree (tmp_str2););
    } else {
        CTIabort ("Could not infer proper type for var %s", VARDEC_NAME (arg_node));
    }

    if (TYisArray (type)) {
        VARDEC_TYPE (arg_node) = FREEfreeAllTypes (VARDEC_TYPE (arg_node));
        VARDEC_TYPE (arg_node) = TYtype2OldType (type);
    } else {
        CTIabort ("Could not infer proper type for var %s", VARDEC_NAME (arg_node));
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTcast( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTcast (node *arg_node, info *arg_info)
{
    node *res;

    DBUG_ENTER ("NT2OTcast");

    res = TRAVdo (CAST_EXPR (arg_node), arg_info);
    CAST_EXPR (arg_node) = NULL;
    arg_node = FREEdoFreeNode (arg_node);

    DBUG_RETURN (res);
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
    node *old_last_let;

    DBUG_ENTER ("NT2OTlet");

    old_last_let = INFO_NT2OT_LAST_LET (arg_info);
    INFO_NT2OT_LAST_LET (arg_info) = arg_node;

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    INFO_NT2OT_LAST_LET (arg_info) = old_last_let;

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
        INFO_NT2OT_WLIDS (arg_info) = NULL;
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

    if (INFO_NT2OT_WLIDS (arg_info) == NULL) {
        vec_type = AVIS_TYPE (IDS_AVIS (WITHID_VEC (arg_node)));
        vec_type = TYfixAndEliminateAlpha (vec_type);

        if (WITHID_IDS (arg_node) == NULL) {
            if (TYisAKS (vec_type)) {
                DBUG_PRINT ("NT2OT", ("WITHID_IDS for %s built",
                                      IDS_NAME (WITHID_VEC (arg_node))));
                num_vars = SHgetExtent (TYgetShape (vec_type), 0);
                new_ids = NULL;
                new_vardecs = INFO_NT2OT_VARDECS (arg_info);
                for (i = 0; i < num_vars; i++) {
                    tmp_avis
                      = TBmakeAvis (ILIBtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                              SHcreateShape (0)));
                    new_vardecs = TBmakeVardec (tmp_avis, new_vardecs);
                    new_ids = TBmakeIds (tmp_avis, new_ids);
                }
                WITHID_IDS (arg_node) = new_ids;
                INFO_NT2OT_WLIDS (arg_info) = new_ids;
                INFO_NT2OT_VARDECS (arg_info) = new_vardecs;
            } else {
                DBUG_PRINT ("NT2OT", ("no WITHID_IDS for %s built",
                                      IDS_NAME (WITHID_VEC (arg_node))));
            }
        } else {
            INFO_NT2OT_WLIDS (arg_info) = WITHID_IDS (arg_node);
        }
    } else {
        /**
         * we are dealing with a default partition here => Duplicate those of the real
         * one!
         */
        WITHID_IDS (arg_node) = DUPdoDupTree (INFO_NT2OT_WLIDS (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTfold( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTfold (node *arg_node, info *arg_info)
{
    node *foldfun;
    node *cexpr, *neutr;
    node *nwith;
    ntype *fold_type;

    DBUG_ENTER ("NT2OTfold");

    nwith = LET_EXPR (INFO_NT2OT_LAST_LET (arg_info));
    cexpr = WITH_CEXPR (nwith);
    neutr = FOLD_NEUTRAL (arg_node);
    DBUG_ASSERT ((neutr != NULL), "WITH_NEUTRAL does not exist");
    DBUG_ASSERT ((NODE_TYPE (neutr) == N_id), "WITH_NEUTRAL is not a N_id");
    DBUG_ASSERT ((NODE_TYPE (cexpr) == N_id), "WITH_CEXPR is not a N_id");
    fold_type = TYlubOfTypes (AVIS_TYPE (ID_AVIS (cexpr)), AVIS_TYPE (ID_AVIS (neutr)));

    foldfun = GPFcreateFoldFun (fold_type, FOLD_FUNDEF (arg_node), FOLD_PRF (arg_node),
                                IDS_NAME (LET_IDS (INFO_NT2OT_LAST_LET (arg_info))),
                                ID_NAME (cexpr));
    FOLD_FUNDEF (arg_node) = foldfun;
    fold_type = TYfreeType (fold_type);

    /*
     * append foldfun to INFO_NT2OT_FOLDFUNS
     */
    FUNDEF_NEXT (foldfun) = INFO_NT2OT_FOLDFUNS (arg_info);
    INFO_NT2OT_FOLDFUNS (arg_info) = foldfun;

    DBUG_RETURN (arg_node);
}
