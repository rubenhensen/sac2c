/*
 *
 * $Log$
 * Revision 1.11  2004/11/25 15:23:07  khf
 * more codebrushing
 *
 * Revision 1.10  2004/11/25 14:14:35  khf
 * SacDevCamp04: COMPILES!
 *
 * Revision 1.9  2004/08/29 18:08:38  sah
 * added some DBUG_PRINTS
 *
 * Revision 1.8  2004/07/16 17:36:23  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.7  2002/10/18 13:35:22  sbs
 * Now, this phase also detects all references to global objects and marks the according
 * N_id nodes as IS_GLOBAL.
 *
 * Revision 1.6  2002/04/29 14:30:21  sbs
 * Now, identifier are copied when creating new vardecs 8-))))
 *
 * Revision 1.5  2002/03/12 15:11:12  sbs
 * dummy vardecs now have a T_unknown rather than a null type pointer.
 *
 * Revision 1.4  2002/02/22 14:07:45  sbs
 * access macros to INFO nodes moved into tree_basic
 *
 * Revision 1.3  2002/02/22 12:30:46  sbs
 * insvd_tab moved into traverse.c
 *
 * Revision 1.2  2002/02/22 09:26:19  sbs
 * INSVDwithid added .
 *
 * Revision 1.1  2002/02/21 15:12:36  sbs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   insert_vardec.c
 *
 * prefix: INSVD
 *
 * description:
 *
 *   This compiler module inserts vardecs for all identifyers that do not have
 *   one yet and adds proper backrefs to them.
 *   It is needed for running InferDFMS prior to type checking.
 *
 *   For all other identifyers the backref is set to the according N_vardec,
 *   Narg, or N_objdef node !!!
 *
 *   While doing so, all references to global objects are tagged as such, i.e.,
 *   the    IS_GLOBAL flag of N_id nodes   is set accordingly(!)  , and the name of
 *   N_id nodes that refer to global objects is extended by the module name of the
 *   object.
 *
 *
 *
 * usage of arg_info (INFO_INSVD_...):
 *
 *   ...OBJDEFS  holds the pointer to the objdefs chain of the actual module !
 *   ...VARDECS  holds the pointer to the vardec chain of the actual fundef !
 *   ...ARGS     holds the pointer to the argument chain of the actual fundef !
 *
 *****************************************************************************/

#include <string.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "new_types.h"
#include "internal_lib.h"
#include "traverse.h"
#include "globals.h"
#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "DupTree.h"

#include "insert_vardec.h"

/*
 * INFO structure
 */
struct INFO {
    node *vardecs;
    node *args;
};

/*
 * INFO macros
 */
#define INFO_INSVD_VARDECS(n) (n->vardecs)
#define INFO_INSVD_ARGS(n) (n->args)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_INSVD_VARDECS (result) = NULL;
    INFO_INSVD_ARGS (result) = NULL;

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
 *  node * SearchForNameInVardecs( char *name, node *vardecs)
 *
 * description:
 *   looks up the name in the vardecs given. Returns the address of the vardec
 *   if found, NULL otherwise.
 *
 ******************************************************************************/
static node *
SearchForNameInVardecs (char *name, node *vardecs)
{
    DBUG_ENTER ("SearchForNameInVardecs");

    while ((vardecs != NULL) && (strcmp (VARDEC_NAME (vardecs), name) != 0)) {
        vardecs = VARDEC_NEXT (vardecs);
    }
    DBUG_RETURN (vardecs);
}

/******************************************************************************
 *
 * function:
 *  node * SearchForNameInArgs( char *name, node *args)
 *
 * description:
 *   looks up the name in the args given. Returns the address of the arg
 *   if found, NULL otherwise.
 *
 ******************************************************************************/
static node *
SearchForNameInArgs (char *name, node *args)
{
    DBUG_ENTER ("SearchForNameInArgs");

    while ((args != NULL) && (strcmp (ARG_NAME (args), name) != 0)) {
        args = ARG_NEXT (args);
    }
    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDfundef( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INSVDfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_INSVD_VARDECS (arg_info) = FUNDEF_VARDEC (arg_node);
        INFO_INSVD_ARGS (arg_info) = FUNDEF_ARGS (arg_node);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_VARDEC (arg_node) = INFO_INSVD_VARDECS (arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDlet( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INSVDlet");

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDid( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDid (node *arg_node, info *arg_info)
{
    node *vardec;

    DBUG_ENTER ("INSVDid");

    vardec = SearchForNameInVardecs (ID_SPNAME (arg_node), INFO_INSVD_VARDECS (arg_info));
    if (vardec == NULL) {
        vardec = SearchForNameInArgs (ID_SPNAME (arg_node), INFO_INSVD_ARGS (arg_info));
    }

    if (vardec == NULL) {
        ERROR (global.linenum, ("Vardec for Identifier with name:%s is not available",
                                ID_SPNAME (arg_node)));
    }

    ID_AVIS (arg_node) = VARDEC_AVIS (vardec);
    ID_SPNAME (arg_node) = ILIBfree (ID_SPNAME (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDid( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDids (node *arg_node, info *arg_info)
{
    node *vardec, *avis;
    DBUG_ENTER ("INSVDids");

    vardec = SearchForNameInVardecs (ID_SPNAME (arg_node), INFO_INSVD_VARDECS (arg_info));
    if (vardec == NULL) {
        vardec = SearchForNameInArgs (ID_SPNAME (arg_node), INFO_INSVD_ARGS (arg_info));
    }

    if (vardec == NULL) {
        /*
         * The identifyer we are looking for does not have a
         * vardec yet! So we allocate one and prepand it to vardecs.
         */

        avis
          = TBmakeAvis (IDS_SPNAME (arg_node), TYmakeAUD (TYmakeSimpleType (T_unknown)));

        vardec = TBmakeVardec (avis, INFO_INSVD_VARDECS (arg_info));

        INFO_INSVD_VARDECS (arg_info) = vardec;

        DBUG_PRINT ("IVD", ("inserting new vardec (" F_PTR ") for id %s.", vardec,
                            IDS_SPNAME (arg_node)));
        IDS_SPNAME (arg_node) = NULL;
    } else {
        IDS_SPNAME (arg_node) = ILIBfree (IDS_SPNAME (arg_node));
    }

    IDS_AVIS (arg_node) = VARDEC_AVIS (vardec);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDwith( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INSVDwith");

    /* traverse all sons in following order:*/

    if (WITH_PART (arg_node) != NULL) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    }

    if (WITH_CODE (arg_node) != NULL) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    if (WITH_WITHOP (arg_node) != NULL) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDpart( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INSVDpart");

    /* withid has to be traversed first */

    if (PART_WITHID (arg_node) != NULL) {
        PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);
    }

    if (PART_GENERATOR (arg_node) != NULL) {
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
    }

    /* at this early point there are no other N_part nodes */
    DBUG_ASSERT ((PART_NEXT (arg_node) == NULL), "PART_NEXT should not yet exist");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDcode( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INSVDcode");

    /* cblock has to be traversed first */

    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    if (CODE_CEXPRS (arg_node) != NULL) {
        CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);
    }

    DBUG_ASSERT ((CODE_NEXT (arg_node) == NULL),
                 "there should be only one code block during inse_vardec!");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * INSVDdoInsertVardecs( node *syntax_tree)
 *
 * description:
 *   Traverses through all functions and inserts vardecs for all identifyers
 *   that do not have one yet. Whenever an N_id node is met, we simply look for
 *   its declaration. If found, a backref to it is inserted (ID_VARDEC).
 *   Otherwise, a new vardec is inserted and a backref is inserted.
 *   The same holds for the ids found at N_let nodes.
 *
 ******************************************************************************/

node *
INSVDdoInsertVardec (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("INSVDdoInsertVardecs");

    info = MakeInfo ();

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 ("InsertVardec can only be called on N_modul nodes"));

    TRAVpush (TR_insvd);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
