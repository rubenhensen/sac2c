/** <!--********************************************************************-->
 *
 * @file lub.c
 *
 * prefix: TFPLB
 *
 * description: This file calls functions to preprocess the tpye hierarchy graph
 * for answering least upper bound queries. This is done with the aid of a
 * compiler pass.
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
#include "dbug.h"
#include "memory.h"
#include "tree_compound.h"
#include "types.h"
#include "dynelem.h"
#include "elemstack.h"
#include "dynarray.h"
#include "dynmatrix.h"
#include "graphtypes.h"
#include "dfwalk.h"
#include "tfprintutils.h"
#include "lubtree.h"
#include "lubcross.h"
#include "binheap.h"
#include "graphinterface.h"

int
GINisReachable (node *n1, node *n2, compinfo *ci)
{

    DBUG_ENTER ("GINisReachable");

    int result = 0;
    int reachtree = 0, reachcross = 0;
    int cola, colb, row;
    int reaching_csrc_after_pre;
    int reaching_csrc_after_premax;

    if (TFVERTEX_POST (n1) > TFVERTEX_POST (n2)) {

        if ((TFVERTEX_PRE (n2) >= TFVERTEX_PRE (n1)
             && TFVERTEX_PRE (n2) < TFVERTEX_PREMAX (n1))) {

            reachtree = 1;
        }

        cola = TFVERTEX_REACHCOLA (n1);
        colb = TFVERTEX_REACHCOLB (n1);
        row = TFVERTEX_ROW (n2);

        if (!TFVERTEX_ISROWMARKED (n2)) {

            reachcross = 0;

        } else {

            if (!TFVERTEX_ISRCHCOLAMARKED (n1)) {
                reaching_csrc_after_pre = 0;
            } else {
                reaching_csrc_after_pre = getMatrixValue (COMPINFO_TLC (ci), cola, row);
            }

            if (!TFVERTEX_ISRCHCOLBMARKED (n1)) {
                reaching_csrc_after_premax = 0;
            } else {
                reaching_csrc_after_premax
                  = getMatrixValue (COMPINFO_TLC (ci), colb, row);
            }

            if (reaching_csrc_after_pre - reaching_csrc_after_premax > 0) {
                reachcross = 1;
            } else {
                reachcross = 0;
            }
        }

        if (reachtree || reachcross) {
            result = 1;
        }
    }

    DBUG_RETURN (result);
}

static void
GINreorderVerticesInDAG (node *n1, node *n2)
{

    DBUG_ENTER ("GINreorderVertices");

    node *temp;

    /*
     * Check whether n2 can reach n1 through a tree or cross edge based path.
     */

    if (TFVERTEX_POST (n2) > TFVERTEX_POST (n1)) {
        temp = n1;
        n1 = n2;
        n2 = temp;
    }

    DBUG_VOID_RETURN;
}

node *
GINlcaFromVertices (node *n1, node *n2, compinfo *ci)
{

    DBUG_ENTER ("GINlcaFromNodes");

    matrix *pcpt_matrix, *pcpc_matrix;
    int pcpt_col, pcpt_row;
    elem *pcpt_elem;
    int lower_pcpt_pre, upper_pcpt_pre;
    node *lower_pcpt_node, *upper_pcpt_node;
    int pcpc_plca_pre;
    node *sptree_plca, *pcpt_plca1, *pcpt_plca2, *pcpc_plca;
    /*
     * Get the pcpt_matrix and pcpc_matrix. We will need this later on to find the
     * pcpt_plca1, pcpt_plca2 and pcpc_plca.
     */

    pcpt_matrix = LUBINFO_PCPTMAT (COMPINFO_LUB (ci));
    pcpc_matrix = LUBINFO_PCPTMAT (COMPINFO_LUB (ci));

    /*
     * Potentially reorder vertices to correctly index the matrices.
     */

    GINreorderVerticesInDAG (n1, n2);
    sptree_plca = LUBtreeLCAfromNodes (n1, n2, ci);

    pcpt_col = TFVERTEX_REACHCOLA (n1);
    pcpt_row = TFVERTEX_ROW (n2);

    /*
     * We have the row and column indices now, we can get the pcpt-matrix element
     * for that row and column.
     */

    pcpt_elem = getMatrixElem (pcpt_matrix, pcpt_row, pcpt_col);

    lower_pcpt_pre = ((int *)ELEM_DATA (pcpt_elem))[0];

    if (lower_pcpt_pre == -1) {

        lower_pcpt_node
          = (node *)ELEM_DATA (DYNARRAY_ELEMS_POS (COMPINFO_PREARR (ci), 0));

    } else {

        lower_pcpt_node = (node *)ELEM_DATA (
          DYNARRAY_ELEMS_POS (COMPINFO_PREARR (ci), lower_pcpt_pre - 1));
    }

    upper_pcpt_pre = ((int *)ELEM_DATA (pcpt_elem))[1];

    if (upper_pcpt_pre == -1) {

        upper_pcpt_node
          = (node *)ELEM_DATA (DYNARRAY_ELEMS_POS (COMPINFO_PREARR (ci), 0));

    } else {

        upper_pcpt_node = (node *)ELEM_DATA (
          DYNARRAY_ELEMS_POS (COMPINFO_PREARR (ci), upper_pcpt_pre - 1));
    }

    /*
     * now, we have two cross edge sources one with a topological number less than
     * n1 and another with a preorder number greater than n1 and another with a
     * preorder number less than n1 reaching n2 through cross edges. Its time to
     * calculate two more potential LCAs based on this information.
     */

    pcpt_plca1 = LUBtreeLCAfromNodes (lower_pcpt_node, n2, ci);
    pcpt_plca2 = LUBtreeLCAfromNodes (n2, upper_pcpt_node, ci);

    pcpc_plca_pre = getMatrixValue (pcpc_matrix, pcpt_row, pcpt_row);
    pcpc_plca
      = (node *)ELEM_DATA (DYNARRAY_ELEMS_POS (COMPINFO_PREARR (ci), pcpc_plca_pre - 1));

    /*
     * Now we just need to find the plca that has the lowest topological number
     * amongst sptree_plca, pcpt_plca1, pcpt_plca2 and pcpc_plca.
     */

    node *n[4] = {sptree_plca, pcpt_plca1, pcpt_plca2, pcpc_plca};
    int i;
    node *result = sptree_plca;

    for (i = 1; i < 4; i++) {
        if (TFVERTEX_TOPO (n[i]) > TFVERTEX_TOPO (result)) {
            result = n[i];
        }
    }

    DBUG_RETURN (result);
}
