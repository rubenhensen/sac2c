/*
 *
 * $Log$
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
#include "dbug.h"

#include "Error.h"

#include "new_typecheck.h"
#include "new_types.h"
#include "new2old.h"
#include "traverse.h"

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

/******************************************************************************
 *
 * function:
 *    node *NT2OTTransform( node *arg_node)
 *
 * description:
 *    adjusts all old vardec types according to the attached ntypes!
 *
 ******************************************************************************/

node *
NT2OTTransform (node *arg_node)
{
    funtab *tmp_tab;
    node *info_node;

    DBUG_ENTER ("NT2OTTransform");

    tmp_tab = act_tab;
    act_tab = nt2ot_tab;

    info_node = MakeInfo ();
    arg_node = Trav (arg_node, info_node);
    info_node = FreeNode (info_node);

    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NT2OTfundef( node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NT2OTfundef (node *arg_node, node *arg_info)
{
    ntype *type;

    DBUG_ENTER ("NT2OTfundef");

    type = TYFixAndEliminateAlpha (FUNDEF_RET_TYPE (arg_node));

    if (TYIsProdOfArray (type)) {
        FUNDEF_TYPES (arg_node) = FreeAllTypes (FUNDEF_TYPES (arg_node));
        FUNDEF_TYPES (arg_node) = TYType2OldType (type);
    } else {
        ABORT (linenum,
               ("could not infer proper type for fun %s", FUNDEF_NAME (arg_node)));
    }

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NT2OTarg( node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NT2OTarg (node *arg_node, node *arg_info)
{
    ntype *type;

    DBUG_ENTER ("NT2OTarg");

    type = AVIS_TYPE (ARG_AVIS (arg_node));

    if (type != NULL) {
        type = TYFixAndEliminateAlpha (type);
    } else {
        ABORT (linenum, ("could not infer proper type for arg %s", ARG_NAME (arg_node)));
    }
    if (TYIsArray (type)) {
        ARG_TYPE (arg_node) = FreeAllTypes (ARG_TYPE (arg_node));
        ARG_TYPE (arg_node) = TYType2OldType (type);
    } else {
        ABORT (linenum, ("could not infer proper type for arg %s", ARG_NAME (arg_node)));
    }
    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NT2OTvardec( node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NT2OTvardec (node *arg_node, node *arg_info)
{
    ntype *type;

    DBUG_ENTER ("NT2OTvardec");

    type = AVIS_TYPE (VARDEC_AVIS (arg_node));

    if (type != NULL) {
        type = TYFixAndEliminateAlpha (type);
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
 *    node *NT2OTarray( node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NT2OTarray (node *arg_node, node *arg_info)
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
