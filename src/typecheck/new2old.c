/*
 *
 * $Log$
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
#include "dbug.h"

#include "Error.h"

#include "traverse.h"
#include "gen_pseudo_fun.h"
#include "new_typecheck.h"
#include "new_types.h"
#include "new2old.h"

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
 */

/**
 * INFO structure
 */
struct INFO {
    node *foldfuns;
    node *last_let;
    node *vardecs;
};

/**
 * INFO macros
 */
#define INFO_NT2OT_FOLDFUNS(n) (n->foldfuns)
#define INFO_NT2OT_LAST_LET(n) (n->last_let)
#define INFO_NT2OT_VARDECS(n) (n->vardecs)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_NT2OT_FOLDFUNS (result) = NULL;
    INFO_NT2OT_LAST_LET (result) = NULL;
    INFO_NT2OT_VARDECS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTTransform( node *arg_node)
 *
 * description:
 *   adjusts all old vardec types according to the attached ntypes!
 *
 ******************************************************************************/

node *
NT2OTTransform (node *arg_node)
{
    funtab *tmp_tab;
    info *info_node;

    DBUG_ENTER ("NT2OTTransform");

    tmp_tab = act_tab;
    act_tab = nt2ot_tab;

    info_node = MakeInfo ();
    arg_node = Trav (arg_node, info_node);
    info_node = FreeInfo (info_node);

    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTmodul( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTmodul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("NT2OTmodul");

    INFO_NT2OT_FOLDFUNS (arg_info) = NULL;

    if (MODUL_IMPORTS (arg_node) != NULL) {
        MODUL_IMPORTS (arg_node) = Trav (MODUL_IMPORTS (arg_node), arg_info);
    }

    if (MODUL_TYPES (arg_node) != NULL) {
        MODUL_TYPES (arg_node) = Trav (MODUL_TYPES (arg_node), arg_info);
    }

    if (MODUL_OBJS (arg_node) != NULL) {
        MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), arg_info);
    }

    if (MODUL_FUNDECS (arg_node) != NULL) {
        MODUL_FUNDECS (arg_node) = Trav (MODUL_FUNDECS (arg_node), arg_info);
    }

    if (INFO_NT2OT_FOLDFUNS (arg_info) != NULL) {
        MODUL_FUNDECS (arg_node)
          = AppendFundef (INFO_NT2OT_FOLDFUNS (arg_info), MODUL_FUNDECS (arg_node));
    }

    INFO_NT2OT_FOLDFUNS (arg_info) = NULL;

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    if (INFO_NT2OT_FOLDFUNS (arg_info) != NULL) {
        MODUL_FUNS (arg_node)
          = AppendFundef (INFO_NT2OT_FOLDFUNS (arg_info), MODUL_FUNS (arg_node));
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

    type = FUNDEF_RET_TYPE (arg_node);
    DBUG_ASSERT ((type != NULL), "FUNDEF_RET_TYPE not found!");
    type = TYFixAndEliminateAlpha (type);
#if 0
  FUNDEF_RET_TYPE( arg_node) = TYFreeType( FUNDEF_RET_TYPE( arg_node));
  FUNDEF_RET_TYPE( arg_node) = type;
#endif

    if (TYIsProdOfArray (type)) {
        old_type = FUNDEF_TYPES (arg_node);

        if (HasDotTypes (old_type)) {
            FUNDEF_TYPES (arg_node)
              = AppendTypes (TYType2OldType (type), MakeTypes1 (T_dots));
        } else {
            FUNDEF_TYPES (arg_node) = TYType2OldType (type);
        }
        old_type = FreeAllTypes (old_type);
    } else {
        ABORT (linenum, ("could not infer proper type for fun %s; type found: %s",
                         FUNDEF_NAME (arg_node), TYType2String (type, FALSE, 0)));
    }

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    INFO_NT2OT_VARDECS (arg_info) = NULL;
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    if (INFO_NT2OT_VARDECS (arg_info) != NULL) {
        /*
         * if some new vardecs have been built, insert them !
         * this has to be done here rather than in NT2OTblock since blocks
         * in general may not be top level.
         * AFAIK, the only place where vardecs are in fact built is the
         * extension of withloop ids in NT2OTwithid.
         */
        INFO_NT2OT_VARDECS (arg_info) = Trav (INFO_NT2OT_VARDECS (arg_info), arg_info);

        FUNDEF_VARDEC (arg_node)
          = AppendVardec (INFO_NT2OT_VARDECS (arg_info), FUNDEF_VARDEC (arg_node));
        INFO_NT2OT_VARDECS (arg_info) = NULL;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
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
        DBUG_EXECUTE ("FIXNT", tmp_str = TYType2String (type, FALSE, 0););
        DBUG_PRINT ("FIXNT", ("replacing argument %s\'s type %s by ...",
                              ARG_NAME (arg_node), tmp_str));
        type = TYFixAndEliminateAlpha (type);
        DBUG_EXECUTE ("FIXNT", tmp_str2 = TYType2String (type, FALSE, 0););
#if CWC_WOULD_BE_PROPER
        AVIS_TYPE (ARG_AVIS (arg_node)) = TYFreeType (AVIS_TYPE (ARG_AVIS (arg_node)));
#endif
        AVIS_TYPE (ARG_AVIS (arg_node)) = type;
        DBUG_PRINT ("FIXNT", ("... %s", tmp_str2));
        DBUG_EXECUTE ("FIXNT", tmp_str = Free (tmp_str); tmp_str2 = Free (tmp_str2););

        if (TYIsArray (type)) {
            ARG_TYPE (arg_node) = FreeAllTypes (ARG_TYPE (arg_node));
            ARG_TYPE (arg_node) = TYType2OldType (type);
        } else {
            ABORT (linenum,
                   ("could not infer proper type for arg %s", ARG_NAME (arg_node)));
        }

        if (ARG_NEXT (arg_node) != NULL) {
            ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
        }

    } else {
        if ((ARG_TYPE (arg_node) != NULL)
            && (TYPES_BASETYPE (ARG_TYPE (arg_node)) != T_dots)) {
            ABORT (linenum,
                   ("could not infer proper type for arg %s", ARG_NAME (arg_node)));
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
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
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
        DBUG_EXECUTE ("FIXNT", tmp_str = TYType2String (type, FALSE, 0););
        DBUG_PRINT ("FIXNT", ("replacing vardec %s\'s type %s by ...",
                              VARDEC_NAME (arg_node), tmp_str));
        type = TYFixAndEliminateAlpha (type);
        DBUG_EXECUTE ("FIXNT", tmp_str2 = TYType2String (type, FALSE, 0););
#if CWC_WOULD_BE_PROPER
        AVIS_TYPE (VARDEC_AVIS (arg_node))
          = TYFreeType (AVIS_TYPE (VARDEC_AVIS (arg_node)));
#endif
        AVIS_TYPE (VARDEC_AVIS (arg_node)) = type;
        DBUG_PRINT ("FIXNT", ("... %s", tmp_str2));
        DBUG_EXECUTE ("FIXNT", tmp_str = Free (tmp_str); tmp_str2 = Free (tmp_str2););
    } else {
        ABORT (linenum,
               ("could not infer proper type for var %s", VARDEC_NAME (arg_node)));
    }

    if (TYIsArray (type)) {
        VARDEC_TYPE (arg_node) = FreeAllTypes (VARDEC_TYPE (arg_node));
        VARDEC_TYPE (arg_node) = TYType2OldType (type);
    } else {
        ABORT (linenum,
               ("could not infer proper type for var %s", VARDEC_NAME (arg_node)));
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTarray( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTarray (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ("NT2OTarray");

    if (ARRAY_TYPE (arg_node) == NULL) {
        type = NewTypeCheck_Expr (arg_node);
        ARRAY_TYPE (arg_node) = TYType2OldType (type);
        type = TYFreeType (type);
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

    res = Trav (CAST_EXPR (arg_node), arg_info);
    CAST_EXPR (arg_node) = NULL;
    arg_node = FreeNode (arg_node);

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

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    INFO_NT2OT_LAST_LET (arg_info) = old_last_let;

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
    ids *new_ids, *tmp_ids;
    node *new_vardecs;
    int i, num_vars;
    ntype *vec_type;

    DBUG_ENTER ("NT2OTwithid");

    vec_type = AVIS_TYPE (IDS_AVIS (NWITHID_VEC (arg_node)));
    vec_type = TYFixAndEliminateAlpha (vec_type);

    if (NWITHID_IDS (arg_node) == NULL) {
        if (TYIsAKS (vec_type)) {
            DBUG_PRINT ("NT2OT",
                        ("NWITHID_IDS for %s built", IDS_NAME (NWITHID_VEC (arg_node))));
            num_vars = SHGetExtent (TYGetShape (vec_type), 0);
            new_ids = NULL;
            new_vardecs = INFO_NT2OT_VARDECS (arg_info);
            for (i = 0; i < num_vars; i++) {
                tmp_ids = MakeIds (TmpVar (), NULL, ST_regular);
                new_vardecs = MakeVardec (StringCopy (IDS_NAME (tmp_ids)),
                                          MakeTypes1 (T_unknown), new_vardecs);
                AVIS_TYPE (VARDEC_AVIS (new_vardecs))
                  = TYMakeAKS (TYMakeSimpleType (T_int), SHCreateShape (0));
                IDS_VARDEC (tmp_ids) = new_vardecs;
                IDS_AVIS (tmp_ids) = VARDEC_AVIS (new_vardecs);
                IDS_NEXT (tmp_ids) = new_ids;
                new_ids = tmp_ids;
            }
            NWITHID_IDS (arg_node) = new_ids;
            INFO_NT2OT_VARDECS (arg_info) = new_vardecs;
        } else {
            DBUG_PRINT ("NT2OT", ("no NWITHID_IDS for %s built",
                                  IDS_NAME (NWITHID_VEC (arg_node))));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTwithop( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTwithop (node *arg_node, info *arg_info)
{
    node *foldfun;
    node *cexpr, *neutr;
    node *nwith;
    ntype *fold_type;
    types *old_type;

    DBUG_ENTER ("NT2OTwithop");

    if (NWITHOP_IS_FOLD (arg_node)) {
        nwith = LET_EXPR (INFO_NT2OT_LAST_LET (arg_info));
        cexpr = NWITH_CEXPR (nwith);
        neutr = NWITH_NEUTRAL (nwith);
        DBUG_ASSERT ((neutr != NULL), "NWITH_NEUTRAL does not exist");
        DBUG_ASSERT ((NODE_TYPE (neutr) == N_id), "NWITH_NEUTRAL is not a N_id");
        DBUG_ASSERT ((NODE_TYPE (cexpr) == N_id), "NWITH_CEXPR is not a N_id");
        fold_type
          = TYLubOfTypes (AVIS_TYPE (ID_AVIS (cexpr)), AVIS_TYPE (ID_AVIS (neutr)));
        old_type = TYType2OldType (fold_type);
        fold_type = TYFreeType (fold_type);

        foldfun
          = CreateFoldFun (old_type, NWITHOP_FUNDEF (arg_node), NWITHOP_PRF (arg_node),
                           IDS_NAME (LET_IDS (INFO_NT2OT_LAST_LET (arg_info))),
                           ID_NAME (cexpr));
        NWITHOP_FUNDEF (arg_node) = foldfun;

        old_type = FreeAllTypes (old_type);

        /*
         * append foldfun to INFO_NT2OT_FOLDFUNS
         */
        FUNDEF_NEXT (foldfun) = INFO_NT2OT_FOLDFUNS (arg_info);
        INFO_NT2OT_FOLDFUNS (arg_info) = foldfun;
    }

    DBUG_RETURN (arg_node);
}
