/*
 *
 * $Log$
 * Revision 1.12  2002/10/31 19:46:26  dkr
 * CorrectFundef() renamed into SearchWrapper()
 *
 * Revision 1.11  2002/10/30 16:11:35  dkr
 * trivial wrappers are no longer built but dispatched statically
 *
 * Revision 1.10  2002/10/30 13:23:59  sbs
 * handling of dot args introduced.
 *
 * Revision 1.9  2002/10/18 14:33:17  sbs
 * some DBUG output added and some FLAG handling of freshly created N_id nodes
 * added.
 *
 * Revision 1.8  2002/09/05 14:45:55  dkr
 * CWCwithop() added
 *
 * Revision 1.7  2002/09/03 18:54:41  dkr
 * this modul is complete now :-)
 *
 * Revision 1.6  2002/08/28 11:36:05  dkr
 * SignatureMatches() added
 *
 * Revision 1.5  2002/08/15 21:27:50  dkr
 * MODUL_WRAPPERFUNS added (not used yet ...)
 *
 * Revision 1.4  2002/08/13 15:59:09  dkr
 * some more cwc stuff added (not finished yet)
 *
 * Revision 1.3  2002/08/09 14:50:53  dkr
 * CWCap added
 *
 * Revision 1.2  2002/08/09 13:15:20  dkr
 * CWCmodul, CWCfundef added
 *
 * Revision 1.1  2002/08/09 13:00:02  dkr
 * Initial revision
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "new_typecheck.h"
#include "new_types.h"

#define INFO_CWC_TRAVNO(n) ((n)->flag)
#define INFO_CWC_WRAPPERFUNS(n) ((LUT_t) ((n)->dfmask[0]))

/**
 **
 ** Function:
 **   node *CreateWrapperCode( node *ast)
 **
 ** Description:
 **   Modifies all wrappers of the AST in a way that they represent correct
 **   SAC functions and that they contain correct code for dispatching
 **   overloaded functions at runtime. In more detail:
 **     - Replaces generic wrappers (valid for more than a single simpletype)
 **       by individual wrappers for each simpletype.
 *      - Generates the bodies of the wrapper functions.
 **   Moreover, all references to replaced wrapper functions are corrected
 **   accordingly:
 **     - AP_FUNDEF,
 **     - NWITHOP_FUNDEF.
 **
 **/

/******************************************************************************
 *
 * Function:
 *   node *CWCmodul( node *arg_node, node *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CWCmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CWCmodul");

    DBUG_ASSERT ((MODUL_WRAPPERFUNS (arg_node) != NULL), "MODUL_WRAPPERFUNS not found!");
    INFO_CWC_WRAPPERFUNS (arg_info) = MODUL_WRAPPERFUNS (arg_node);

    /*
     * create separate wrapper function for all base type constellations
     */
    INFO_CWC_TRAVNO (arg_info) = 1;
    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    /*
     * adjust AP_FUNDEF pointers
     */
    INFO_CWC_TRAVNO (arg_info) = 2;
    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    /*
     * create separate wrapper function for all base type constellations
     */
    INFO_CWC_TRAVNO (arg_info) = 3;
    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    MODUL_WRAPPERFUNS (arg_node) = RemoveLUT (MODUL_WRAPPERFUNS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *SplitWrapper( node *fundef)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
SplitWrapper (node *fundef)
{
    ntype *old_type, *tmp_type;
    ntype *new_type;
    bool finished;
    node *new_fundef;
    node *new_fundefs = NULL;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("SplitWrapper");

    old_type = FUNDEF_TYPE (fundef);
    tmp_type = TYCopyType (old_type);
    FUNDEF_TYPE (fundef) = NULL;
    DBUG_PRINT ("CWC", ("splitting wrapper of %s", FUNDEF_NAME (fundef)));
    do {
        new_fundef = DupNode (fundef);
        new_type = TYSplitWrapperType (tmp_type, &finished);
        DBUG_EXECUTE ("CWC", tmp_str = TYType2String (new_type, TRUE, 0););
        DBUG_PRINT ("CWC", ("  new wrapper split off: %s", tmp_str));
        DBUG_EXECUTE ("CWC", tmp_str = Free (tmp_str););

        FUNDEF_TYPE (new_fundef) = new_type;
        FUNDEF_RET_TYPE (new_fundef) = TYGetWrapperRetType (new_type);
        FUNDEF_ARGS (new_fundef)
          = TYCorrectWrapperArgTypes (FUNDEF_ARGS (new_fundef), new_type);
        FUNDEF_NEXT (new_fundef) = new_fundefs;
        new_fundefs = new_fundef;
    } while (!finished);
    FUNDEF_TYPE (fundef) = old_type;
    tmp_type = TYFreeType (tmp_type);

    DBUG_RETURN (new_fundefs);
}

/******************************************************************************
 *
 * Function:
 *   node *InsertWrapperCode( node *fundef)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
InsertWrapperCode (node *fundef)
{
    node *spec_fundef;
    node *ret;
    node *assigns;
    node *vardec;
    node *vardecs1, *vardecs2;

    DBUG_ENTER ("InsertWrapperCode");

    spec_fundef = TYStaticDispatchWrapper (fundef);
    if (spec_fundef == NULL) {
        /*
         * generate wrapper code together with the needed vardecs
         */
        vardecs1 = TYCreateWrapperVardecs (fundef);
        vardecs2 = NULL;
        assigns = TYCreateWrapperCode (fundef, vardecs1, &vardecs2);

        /*
         * vardecs -> return exprs
         */
        ret = NULL;
        vardec = vardecs1;
        while (vardec != NULL) {
            node *id_node = MakeId_Copy (VARDEC_NAME (vardec));
            ID_VARDEC (id_node) = vardec;
            SET_FLAG (ID, id_node, IS_GLOBAL, FALSE);
            /* ok since GOs may not be returned */
            SET_FLAG (ID, id_node, IS_REFERENCE, FALSE);
            ID_AVIS (id_node) = VARDEC_AVIS (vardec);
            ret = MakeExprs (id_node, ret);
            vardec = VARDEC_NEXT (vardec);
        }
        FUNDEF_RETURN (fundef) = ret = MakeReturn (ret);

        /*
         * append return statement to assignments
         */
        assigns = AppendAssign (assigns, MakeAssign (ret, NULL));

        /*
         * insert function body
         */
        FUNDEF_BODY (fundef) = MakeBlock (assigns, AppendVardec (vardecs1, vardecs2));

        /*
         * mark wrapper function as a inline function
         */
        FUNDEF_INLINE (fundef) = TRUE;
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * Function:
 *   bool SignatureMatches( node *formal, node *actual)
 *
 * Description:
 *
 *
 ******************************************************************************/

static bool
SignatureMatches (node *formal, node *actual)
{
    ntype *formal_type, *actual_type, *tmp_type;
    bool match = TRUE;
#ifndef DBUG_OFF
    char *tmp_str, *tmp2_str;
#endif

    DBUG_ENTER ("SignatureMatches");

    while ((formal != NULL) && (ARG_TYPE (formal) != NULL)
           && (TYPES_BASETYPE (ARG_TYPE (formal)) != T_dots)) {
        DBUG_ASSERT ((actual != NULL), "inconsistant application found!");
        DBUG_ASSERT (((NODE_TYPE (formal) == N_arg) && (NODE_TYPE (actual) == N_exprs)),
                     "illegal args found!");

        formal_type = AVIS_TYPE (ARG_AVIS (formal));
        tmp_type = NewTypeCheck_Expr (EXPRS_EXPR (actual));
        actual_type = TYFixAndEliminateAlpha (tmp_type);
        tmp_type = TYFreeType (tmp_type);
        DBUG_EXECUTE ("CWC", tmp_str = TYType2String (formal_type, FALSE, 0);
                      tmp2_str = TYType2String (actual_type, FALSE, 0););
        DBUG_PRINT ("CWC",
                    ("comparing formal type %s with actual type %s", tmp_str, tmp2_str));
        DBUG_EXECUTE ("CWC", tmp_str = Free (tmp_str); tmp2_str = Free (tmp2_str););

        if (!TYLeTypes (actual_type, formal_type)) {
            match = FALSE;
            break;
        }
        DBUG_PRINT ("CWC", ("result: %d", match));

        formal = ARG_NEXT (formal);
        actual = EXPRS_NEXT (actual);
    }

    DBUG_RETURN (match);
}

/******************************************************************************
 *
 * Function:
 *   node *SearchWrapper( node *old_fundef, char *funname, node *args)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
SearchWrapper (node *fundef, char *funname, node *args)
{
    DBUG_ENTER ("SearchWrapper");

    DBUG_ASSERT ((fundef != NULL), "fundef not found!");
    if (FUNDEF_STATUS (fundef) == ST_zombiefun) {
        DBUG_PRINT ("CWC", ("correcting fundef for %s", funname));
        do {
            fundef = FUNDEF_NEXT (fundef);
            DBUG_ASSERT (((fundef != NULL) && (!strcmp (funname, FUNDEF_NAME (fundef)))
                          && (FUNDEF_STATUS (fundef) == ST_wrapperfun)),
                         "no appropriate wrapper function found!");
        } while (!SignatureMatches (FUNDEF_ARGS (fundef), args));

        if (FUNDEF_BODY (fundef) == NULL) {
            fundef = TYStaticDispatchWrapper (fundef);
            DBUG_ASSERT ((fundef != NULL), "static dipatch of empty wrapper failed");
        }
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * Function:
 *   node *CWCfundef( node *arg_node, node *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CWCfundef (node *arg_node, node *arg_info)
{
    node *new_fundef;
    node *new_fundefs;

    DBUG_ENTER ("CWCfundef");

    if (INFO_CWC_TRAVNO (arg_info) == 1) {
        /*
         * first traversal -> build wrapper functions and there bodies
         */

        if (FUNDEF_STATUS (arg_node) == ST_wrapperfun) {
            DBUG_ASSERT ((FUNDEF_BODY (arg_node) == NULL),
                         "wrapper function has already a body!");

            /*
             * build a separate fundef for each base type constellation
             */
            new_fundefs = SplitWrapper (arg_node);

            /*
             * build code for all wrapper functions
             */
            new_fundef = new_fundefs;
            DBUG_ASSERT ((new_fundef != NULL), "no wrapper functions found!");
            do {
                new_fundef = InsertWrapperCode (new_fundef);
                new_fundef = FUNDEF_NEXT (new_fundef);
            } while (new_fundef != NULL);

            if (FUNDEF_NEXT (arg_node) != NULL) {
                FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
            }

            /*
             * insert new wrapper functions behind the original one and
             * free original wrapper function (-> zombie function)
             */
            new_fundefs = AppendFundef (new_fundefs, FUNDEF_NEXT (arg_node));
            arg_node = FreeNode (arg_node);
            DBUG_ASSERT (((arg_node != NULL)
                          && (FUNDEF_STATUS (arg_node) == ST_zombiefun)),
                         "zombie fundef not found!");
            FUNDEF_NEXT (arg_node) = new_fundefs;
        }
    } else if (INFO_CWC_TRAVNO (arg_info) == 2) {
        /*
         * second traversal -> adjust all AP_FUNDEF pointers
         *
         * This is needed if the original wrapper function was valid for more than
         * a single base type.
         */
        if ((FUNDEF_STATUS (arg_node) != ST_wrapperfun)
            && (FUNDEF_BODY (arg_node) != NULL)) {
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        }

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        }
    } else {
        DBUG_ASSERT ((INFO_CWC_TRAVNO (arg_info) == 3), "illegal INFO_CWC_TRAVNO found!");
        /*
         * third traversal -> remove zombies and empty wrappers
         */

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        }

        if (FUNDEF_STATUS (arg_node) == ST_zombiefun) {
            arg_node = FreeZombie (arg_node);
        } else if ((FUNDEF_STATUS (arg_node) == ST_wrapperfun)
                   && (FUNDEF_BODY (arg_node) == NULL)) {
            arg_node = FreeZombie (FreeNode (arg_node));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CWCap( node *arg_node, node *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CWCap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CWCap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    AP_FUNDEF (arg_node)
      = SearchWrapper (AP_FUNDEF (arg_node), AP_NAME (arg_node), AP_ARGS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CWCwithop( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CWCwithop (node *arg_node, node *arg_info)
{
    node *args;

    DBUG_ENTER ("CWCwithop");

    if (NWITHOP_TYPE (arg_node) == WO_foldfun) {
        args = MakeExprs (DupNode (NWITHOP_NEUTRAL (arg_node)), NULL);
        EXPRS_NEXT (args) = DupNode (args);

        NWITHOP_FUNDEF (arg_node)
          = SearchWrapper (NWITHOP_FUNDEF (arg_node), NWITHOP_FUN (arg_node), args);

        args = FreeTree (args);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CreateWrapperCode( node *ast)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CreateWrapperCode (node *ast)
{
    funtab *tmp_tab;
    node *info_node;

    DBUG_ENTER ("CreateWrapperCode");

    tmp_tab = act_tab;
    act_tab = cwc_tab;

    info_node = MakeInfo ();
    ast = Trav (ast, info_node);
    info_node = FreeNode (info_node);

    act_tab = tmp_tab;

    DBUG_RETURN (ast);
}
