/*
 *
 * $Log$
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

#ifndef _sac_wltransform_h
#define _sac_wltransform_h

/*
 * symbolic bounds for strides/grids and IDX_MIN, IDX_MAX
 */
#define IDX_SHAPE (-1) /* equals the shape */
#define IDX_OTHER (-2) /* other */

#define IDX_IS_NUM(idx) (idx >= 0)

#define GET_SHAPE_IDX(shape, dim)                                                        \
    ((shape != NULL) ? SHPSEG_SHAPE (shape, dim) : IDX_SHAPE)

#define PRINT_VARIDX_VECT(handle, vect, dims)                                            \
    {                                                                                    \
        int d;                                                                           \
        if ((vect) != NULL) {                                                            \
            fprintf (handle, "[ ");                                                      \
            for (d = 0; d < dims; d++) {                                                 \
                if (NODE_TYPE (((vect)[d])) == N_num) {                                  \
                    fprintf (handle, "%i ", NUM_VAL (((vect)[d])));                      \
                } else {                                                                 \
                    DBUG_ASSERT ((NODE_TYPE (((vect)[d])) == N_id),                      \
                                 "entry of var. index vector is neither N_num or "       \
                                 "N_id!");                                               \
                    fprintf (handle, "%s ", ID_NAME (((vect)[d])));                      \
                }                                                                        \
            }                                                                            \
            fprintf (handle, "]");                                                       \
        } else {                                                                         \
            fprintf (handle, "NULL");                                                    \
        }                                                                                \
    }

extern node *WlTransform (node *syntax_tree);

extern node *WLTRAwith (node *arg_node, node *arg_info);
extern node *WLTRAcode (node *arg_node, node *arg_info);
extern node *WLTRAlet (node *arg_node, node *arg_info);

extern node *InsertWLnodes (node *nodes, node *insert_nodes);
extern int GridOffset (int new_bound1, int bound1, int step, int grid_b2);
extern bool AllStridesAreConstant (node *wlnode, bool trav_cont, bool trav_nextdim);

#endif /* _sac_wltransform_h */
