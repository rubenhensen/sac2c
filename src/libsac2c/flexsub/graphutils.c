
/** <!--********************************************************************-->
 *
 * @file graphutils.c
 *
 * description: some utility functions for vertices and list of vertices
 *
 *****************************************************************************/

#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "dfwalk.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"
#include "memory.h"
#include "tree_compound.h"
#include "types.h"
#include "dynelem.h"
#include "elemstack.h"
#include "dynarray.h"
#include "dynmatrix.h"
#include "graphtypes.h"
#include "graphutils.h"

bool
GUvertInList (node *n, nodelist *nl)
{

    while (nl != NULL) {

        if (NODELIST_NODE (nl) == n) {
            return 1;
        }

        nl = NODELIST_NEXT (nl);
    }

    return 0;
}

nodelist *
GUmergeLists (nodelist *nla, nodelist *nlb)
{

    nodelist *nlx, *itr_nlx = NULL, *itr_nla, *itr_nlb;

    itr_nla = nla;
    itr_nlb = nlb;

    nlx = NULL;

    while (itr_nla != NULL) {

        /*
         * First check whether the nodes in nla are in nlb. If not, add the nodes to
         * the combined list.
         */

        if (!GUvertInList (NODELIST_NODE (nla), nlb)) {

            if (nlx == NULL) {

                nlx = (nodelist *)MEMmalloc (sizeof (nodelist));
                itr_nlx = nlx;

            } else {

                NODELIST_NEXT (itr_nlx) = (nodelist *)MEMmalloc (sizeof (nodelist));
                itr_nlx = NODELIST_NEXT (itr_nlx);
            }

            NODELIST_NODE (itr_nlx) = NODELIST_NODE (itr_nla);
            NODELIST_NEXT (itr_nlx) = NULL;
        }

        itr_nla = NODELIST_NEXT (itr_nla);
    }

    /*
     * Now add the nodes in nlb to nlx
     */

    itr_nlb = nlb;

    while (itr_nlb != NULL) {

        if (nlx == NULL) {

            nlx = (nodelist *)MEMmalloc (sizeof (nodelist));
            itr_nlx = nlx;

        } else {

            NODELIST_NEXT (itr_nlx) = (nodelist *)MEMmalloc (sizeof (nodelist));
            itr_nlx = NODELIST_NEXT (itr_nlx);
        }

        NODELIST_NODE (itr_nlx) = NODELIST_NODE (itr_nlb);
        NODELIST_NEXT (itr_nlx) = NULL;

        itr_nlb = NODELIST_NEXT (itr_nlb);
    }

    return nlx;
}

void
GUremoveEdge (node *src, node *tar)
{

    node *prev_itr, *curr_itr;

    /*
     * First remove vertices from the children list of the edge source
     */

    prev_itr = NULL;

    curr_itr = TFVERTEX_CHILDREN (src);

    while (curr_itr != NULL) {

        if (TFEDGE_TARGET (curr_itr) == tar) {

            if (prev_itr == NULL) {

                /*
                 * The first node in the list is a match.
                 */

                TFVERTEX_CHILDREN (src) = FREEdoFreeNode (TFVERTEX_CHILDREN (src));
                curr_itr = TFVERTEX_CHILDREN (src);

            } else {

                TFEDGE_NEXT (prev_itr) = FREEdoFreeNode (curr_itr);
                curr_itr = TFEDGE_NEXT (prev_itr);
            }

            continue;
        }

        prev_itr = curr_itr;
        curr_itr = TFEDGE_NEXT (curr_itr);
    }

    /*
     * Now remove vetices from the lists of parents of tar
     */

    prev_itr = NULL;

    curr_itr = TFVERTEX_PARENTS (tar);

    while (curr_itr != NULL) {

        if (TFEDGE_TARGET (curr_itr) == src) {

            if (prev_itr == NULL) {

                /*
                 * The first node in the list is a match.
                 */

                TFVERTEX_PARENTS (tar) = FREEdoFreeNode (TFVERTEX_PARENTS (tar));
                curr_itr = TFVERTEX_PARENTS (src);

            } else {

                TFEDGE_NEXT (prev_itr) = FREEdoFreeNode (curr_itr);
                curr_itr = TFEDGE_NEXT (prev_itr);
            }

            continue;
        }

        prev_itr = curr_itr;
        curr_itr = TFEDGE_NEXT (curr_itr);
    }
}
