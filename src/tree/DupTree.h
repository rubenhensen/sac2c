/*
 * $Log$
 * Revision 1.12  2000/07/12 15:20:38  dkr
 * DuplicateTypes removed (use DupTypes instead!)
 *
 * Revision 1.11  2000/07/04 14:38:11  jhs
 * Added Dups for MTalloc, MTsignal, MTsync.
 *
 * Revision 1.10  2000/06/23 15:33:27  dkr
 * signature of DupTree changed
 * function DupTreeInfo added
 *
 * Revision 1.9  2000/03/17 18:30:44  dkr
 * type lut_t* replaced by LUT_t
 *
 * Revision 1.8  2000/03/15 12:58:46  dkr
 * macro DUPVECT added
 *
 * Revision 1.7  2000/03/02 13:06:30  jhs
 * Added DupSt and DupMt.
 *
 * Revision 1.6  2000/02/17 16:18:37  cg
 * Function DuplicateTypes() moved from typecheck.c.
 * New function DupTypes() added.
 *
 * Revision 1.5  2000/02/03 17:30:52  dkr
 * DupTreeLUT and DupNodeLUT added
 *
 * Revision 1.4  2000/01/31 13:28:52  dkr
 * Some Functions renamed or specialized
 *
 * Revision 1.3  2000/01/26 23:26:12  dkr
 * DupTreePre() and DupTreePost() added.
 * Some code brushing done.
 *
 * Revision 1.2  2000/01/26 17:27:53  dkr
 * type of traverse-function-table changed.
 *
 * Revision 1.1  2000/01/21 11:16:27  dkr
 * Initial revision
 *
 * Revision 2.6  1999/09/01 17:11:01  jhs
 * Fixed Duplicating of masks in DupAssign.
 *
 * Revision 2.5  1999/07/07 15:04:25  sbs
 * DupVinfo added; it implicitly generates consistent VINFO_DOLLAR
 * pointers!!!
 *
 * Revision 2.4  1999/04/13 14:01:48  cg
 * added function DupBlock for duplication of N_block nodes.
 *
 * Revision 2.3  1999/03/17 16:16:31  bs
 * Macro DUP(s,d) modified. Now s->counter will also be duplicated.
 *
 * Revision 2.2  1999/02/25 10:58:18  bs
 * DupArray added
 *
 * Revision 2.1  1999/02/23 12:41:18  sacbase
 * new release made
 *
 * Revision 1.34  1998/08/07 14:37:06  dkr
 * DupWLsegVar added
 *
 * [ ... ]
 *
 * Revision 1.1  1995/05/01  15:32:32  asi
 * Initial revision
 */

#ifndef _sac_DupTree_h
#define _sac_DupTree_h

#include "LookUpTable.h"
#include "types.h"

#define DUP_NORMAL 0
#define DUP_INLINE 1
#define DUP_INVARIANT 2
#define DUP_WLF 3

#define DUPVECT(new_vect, old_vect, dims, type)                                          \
    {                                                                                    \
        int d;                                                                           \
        if ((old_vect) != NULL) {                                                        \
            (new_vect) = (type *)MALLOC (dims * sizeof (type));                          \
            for (d = 0; d < dims; d++) {                                                 \
                (new_vect)[d] = (old_vect)[d];                                           \
            }                                                                            \
        }                                                                                \
    }

extern node *DupTreeLUT (node *arg_node, LUT_t lut);
extern node *DupTree (node *arg_node);
extern node *DupTreeInfo (node *arg_node, node *arg_info);
extern node *DupNodeLUT (node *arg_node, LUT_t lut);
extern node *DupNode (node *arg_node);

extern shpseg *DupShpSeg (shpseg *shp_seg);
extern ids *DupOneIds (ids *ids, node *arg_info);
extern ids *DupIds (ids *ids, node *arg_info);
extern types *DupTypes (types *source);

extern node *DupVinfo (node *arg_node, node *arg_info);
extern node *DupNum (node *arg_node, node *arg_info);
extern node *DupBool (node *arg_node, node *arg_info);
extern node *DupFloat (node *arg_node, node *arg_info);
extern node *DupDouble (node *arg_node, node *arg_info);
extern node *DupChar (node *arg_node, node *arg_info);
extern node *DupStr (node *arg_node, node *arg_info);
extern node *DupId (node *arg_node, node *arg_info);
extern node *DupCast (node *arg_node, node *arg_info);
extern node *DupReturn (node *arg_node, node *arg_info);
extern node *DupBlock (node *arg_node, node *arg_info);
extern node *DupTypedef (node *arg_node, node *arg_info);
extern node *DupObjdef (node *arg_node, node *arg_info);
extern node *DupVardec (node *arg_node, node *arg_info);
extern node *DupArg (node *arg_node, node *arg_info);
extern node *DupLet (node *arg_node, node *arg_info);
extern node *DupArray (node *arg_node, node *arg_info);
extern node *DupCond (node *arg_node, node *arg_info);
extern node *DupDo (node *arg_node, node *arg_info);
extern node *DupWhile (node *arg_node, node *arg_info);
extern node *DupExprs (node *arg_node, node *arg_info);
extern node *DupAssign (node *arg_node, node *arg_info);
extern node *DupEmpty (node *arg_node, node *arg_info);
extern node *DupPrf (node *arg_node, node *arg_info);
extern node *DupAp (node *arg_node, node *arg_info);
extern node *DupEmpty (node *arg_node, node *arg_info);
extern node *DupFundef (node *arg_node, node *arg_info);
extern node *DupPragma (node *arg_node, node *arg_info);
extern node *DupIcm (node *arg_node, node *arg_info);
extern node *DupSpmd (node *arg_node, node *arg_info);
extern node *DupSync (node *arg_node, node *arg_info);
extern node *DupMt (node *arg_node, node *arg_info);
extern node *DupSt (node *arg_node, node *arg_info);
extern node *DupMTsignal (node *arg_node, node *arg_info);
extern node *DupMTsync (node *arg_node, node *arg_info);
extern node *DupMTalloc (node *arg_node, node *arg_info);

/* frontend with-loop */
extern node *DupNwith (node *arg_node, node *arg_info);
extern node *DupNwithop (node *arg_node, node *arg_info);
extern node *DupNpart (node *arg_node, node *arg_info);
extern node *DupNcode (node *arg_node, node *arg_info);
extern node *DupNwithid (node *arg_node, node *arg_info);
extern node *DupNgen (node *arg_node, node *arg_info);

/* backend with-loop */
extern node *DupNwith2 (node *arg_node, node *arg_info);
extern node *DupWLseg (node *arg_node, node *arg_info);
extern node *DupWLblock (node *arg_node, node *arg_info);
extern node *DupWLublock (node *arg_node, node *arg_info);
extern node *DupWLstride (node *arg_node, node *arg_info);
extern node *DupWLgrid (node *arg_node, node *arg_info);
extern node *DupWLsegVar (node *arg_node, node *arg_info);
extern node *DupWLstriVar (node *arg_node, node *arg_info);
extern node *DupWLgridVar (node *arg_node, node *arg_info);

extern node *DupTreePre (node *arg_node, node *arg_info);
extern node *DupTreePost (node *arg_node, node *arg_info);

#endif /* _sac_DupTree_h */
