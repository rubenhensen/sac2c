/*
 *
 * $Log$
 * Revision 3.4  2001/07/16 08:23:11  cg
 * Old tree construction function MakeNode eliminated.
 *
 * Revision 3.3  2001/04/24 09:16:31  dkr
 * P_FORMAT replaced by F_PTR
 *
 * Revision 3.2  2001/03/21 11:05:37  dkr
 * some superfluous includes removed
 *
 * Revision 3.1  2000/11/20 18:03:31  sacbase
 * new release made
 *
 *  [...]
 *
 * Revision 1.23  1995/09/27  15:16:54  cg
 * ATTENTION:
 * tree.c and tree.h are not part of the new virtual syntax tree.
 * They are kept for compatibility reasons with old code only !
 * All parts of their old versions which are to be used in the future are moved
 * to tree_basic and tree_compound.
 * DON'T use tree.c and tree.h when writing new code !!
 *
 *  [...]
 *
 * Revision 1.1  1994/12/20  15:42:10  sbs
 * Initial revision
 *
 */

#include <stdlib.h>

#include "tree.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "dbug.h"
#include "my_debug.h"

#define GEN_NODE(type) (type *)Malloc (sizeof (type))

/*--------------------------------------------------------------------------*/
/* The following functions are supported for compatibility reasons only     */
/*--------------------------------------------------------------------------*/

node *
AppendNodeChain (int dummy, node *chain, node *item)
{
    node *ret;

    DBUG_ENTER ("AppendNodeChain");

    ret = chain;

    if (item != NULL) {
        switch (NODE_TYPE (item)) {
        case N_typedef:
            ret = AppendTypedef (chain, item);
            break;
        case N_objdef:
            ret = AppendObjdef (chain, item);
            break;
        case N_fundef:
            ret = AppendFundef (chain, item);
            break;
        case N_vardec:
            ret = AppendVardec (chain, item);
            break;
        case N_assign:
            ret = AppendAssign (chain, item);
            break;
        case N_exprs:
            ret = ExprsConcat (chain, item);
            break;
        default:
            DBUG_ASSERT ((0), "No suitable append function found!");
            break;
        }
    }

    DBUG_RETURN (ret);
}
