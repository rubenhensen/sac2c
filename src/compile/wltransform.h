/*
 *
 * $Log$
 * Revision 3.13  2004/11/25 15:15:07  skt
 * renaming
 *
 * Revision 3.12  2004/11/21 22:04:36  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 3.11  2004/07/17 17:07:16  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.10  2003/04/20 18:58:25  dkr
 * WLTRAassign added
 *
 * Revision 3.9  2001/05/03 17:32:42  dkr
 * PRINT_SV renamed into PRINT_HOMSV
 *
 * Revision 3.8  2001/04/03 17:51:30  dkr
 * macro PRINT_SV added
 *
 * Revision 3.7  2001/04/02 16:03:42  dkr
 * some macros moved to wl_bounds.h
 *
 * Revision 3.6  2001/03/29 01:34:52  dkr
 * PRINT_VARIDX_VECT added
 *
 * Revision 3.5  2001/03/28 12:40:11  dkr
 * macro PRINT_VARIDX_VECT added
 *
 * Revision 3.4  2001/01/17 14:16:28  dkr
 * some new macros and prototypes added
 *
 * Revision 3.3  2001/01/09 16:15:18  dkr
 * prototype for function AllStridesAreConstant() added
 *
 * Revision 3.2  2001/01/09 14:51:35  dkr
 * no changes done
 *
 * Revision 3.1  2000/11/20 18:01:33  sacbase
 * new release made
 *
 * Revision 2.3  2000/06/25 01:54:26  dkr
 * WLTRAfundef removed
 *
 * Revision 2.2  2000/06/23 15:30:58  dkr
 * minor changes done
 *
 * Revision 2.1  1999/02/23 12:43:02  sacbase
 * new release made
 *
 * Revision 1.6  1998/05/15 15:11:31  dkr
 * added WLTRALet
 *
 * Revision 1.5  1998/05/06 22:19:27  dkr
 * removed WLTRALet
 *
 * Revision 1.4  1998/05/06 21:42:50  dkr
 * added WLTRALet
 *
 * Revision 1.2  1998/05/06 18:09:04  dkr
 * removed WLTRAAssign
 *
 * Revision 1.1  1998/04/29 17:17:17  dkr
 * Initial revision
 *
 */

#ifndef _SAC_WLTRANSFORM_H_
#define _SAC_WLTRANSFORM_H_

#include "types.h"

#define PRINT_HOMSV(handle, vect, dims)                                                  \
    {                                                                                    \
        int d;                                                                           \
        if ((vect) != NULL) {                                                            \
            fprintf (handle, "[ ");                                                      \
            for (d = 0; d < (dims); d++) {                                               \
                if ((vect)[d] > 0) {                                                     \
                    fprintf (handle, "%i", (vect)[d]);                                   \
                } else {                                                                 \
                    fprintf (handle, "?");                                               \
                }                                                                        \
                fprintf (handle, " ");                                                   \
            }                                                                            \
            fprintf (handle, "]");                                                       \
        } else {                                                                         \
            fprintf (handle, "NULL");                                                    \
        }                                                                                \
    }

extern node *WLTRAdoWlTransform (node *syntax_tree);

extern node *WLTRAassign (node *arg_node, info *arg_info);
extern node *WLTRAcode (node *arg_node, info *arg_info);
extern node *WLTRAlet (node *arg_node, info *arg_info);
extern node *WLTRAwith (node *arg_node, info *arg_info);

extern bool WLTRAallStridesAreConstant (node *wlnode, bool trav_cont, bool trav_nextdim);

extern node *WLTRAinsertWlNodes (node *nodes, node *insert_nodes);
extern int WLTRAgridOffset (int new_bound1, int bound1, int step, int grid_b2);

#endif /* _SAC_WLTRANSFORM_H_ */
