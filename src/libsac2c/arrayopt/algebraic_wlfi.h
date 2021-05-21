/** <!--********************************************************************-->
 *
 * Algebraic With-Loop-Folding-Inference traversal
 *
 * Prefix: AWLFI
 *
 *****************************************************************************/

#ifndef _SAC_ALGEBRAIC_WLFI_H_
#define _SAC_ALGEBRAIC_WLFI_H_

#include "types.h"

extern node *AWLFIdoAlgebraicWithLoopFolding (node *arg_node);
extern node *AWLFIfindWL (node *arg_node);
extern node *AWLFIfindWlId (node *arg_node);
extern bool AWLFIisHasNoteintersect (node *arg_node);
extern bool AWLFIisIdsMemberPartition (node *arg_node, node *partn);
extern bool AWLFIisHasInverseProjection (node *arg_node);
extern bool AWLFIisHasAllInverseProjections (node *arg_node);
extern node *AWLFIoffset2Iv (node *arg_node, node **vardecs, node **preassigns,
                             node *pwlpart);
extern bool AWLFIisValidNoteintersect (node *arg_node, node *pwlid);
extern node *AWLFIdetachNoteintersect (node *arg_node);
extern node *AWLFIfindNoteintersect (node *arg_node);
extern int AWLFIfindPrfParent2 (node *arg_node, node *withidids, node **infowithid);

extern node *AWLFIflattenExpression (node *arg_node, node **vardecs, node **preassigns,
                                     ntype *ztype);
extern node *AWLFItakeDropIv (int takect, size_t dropct, node *arg_node, node **vardecs,
                              node **preassigns);
extern bool AWLFIcheckProducerWLFoldable (node *arg_node);
extern bool AWLFIcheckBothFoldable (node *pwlid, node *cwlids, int cwllevel);
extern bool AWLFIisNakedWL (int cwllevel, int pwllevel);
extern bool AWLFIisUsualWL (int cwllevel, int pwllevel);
extern node *AWLFIflattenScalarNode (node *arg_node, info *arg_info);
extern node *AWLFIattachIntersectCalc (node *arg_node, info *arg_info, node *ivavis);
extern bool AWLFIisCanAttachIntersectCalc (node *arg_node, node *ivavis, info *arg_info);
extern node *AWLFIgenerateMinMaxForArray (node *ivavis, info *arg_info, bool emax);

extern node *AWLFIfundef (node *arg_node, info *arg_info);
extern node *AWLFIblock (node *arg_node, info *arg_info);
extern node *AWLFIassign (node *arg_node, info *arg_info);
extern node *AWLFIlet (node *arg_node, info *arg_info);
extern node *AWLFIcond (node *arg_node, info *arg_info);
extern node *AWLFIfuncond (node *arg_node, info *arg_info);
extern node *AWLFIwhile (node *arg_node, info *arg_info);
extern node *AWLFIid (node *arg_node, info *arg_info);
extern node *AWLFIwith (node *arg_node, info *arg_info);
extern node *AWLFIpart (node *arg_node, info *arg_info);
extern node *AWLFIcode (node *arg_node, info *arg_info);
extern node *AWLFImodarray (node *arg_node, info *arg_info);
extern node *AWLFIgenerator (node *arg_node, info *arg_info);
extern node *AWLFIprf (node *arg_node, info *arg_info);

/* expressions per partition are: bound1, bound2, intlo, inthi, intNull,
 *                                inverseprojectionlo,
 *                                inverseprojectionhi,
 *                                wlintersect1part */
#define WLEPP (8)
#define WLARGNODE (0)
#define WLPRODUCERWL (1)
#define WLIVAVIS (2)
#define WLFIRST (3)
#define WLBOUND1ORIGINAL(partno) (WLFIRST + 0 + (WLEPP * partno))
#define WLBOUND2ORIGINAL(partno) (WLFIRST + 1 + (WLEPP * partno))
#define WLINTERSECTION1(partno) (WLFIRST + 2 + (WLEPP * partno))
#define WLINTERSECTION2(partno) (WLFIRST + 3 + (WLEPP * partno))
/*
 * These fields are normalized, i.e.,
 * the same as GENERATOR_BOUND1 and GENERATOR_BOUND2.
 *
 * FIXME: NO LONGER TRUE I HOPE
 * WLINTERSECTION2 is denormalized, because if BuildInversePartition
 * denormalized it, that denormalization
 * code would end up in the CWL block, when it must appear
 * before that block.
 */
#define WLINTERSECTIONNULL(partno) (WLFIRST + 4 + (WLEPP * partno))
#define WLINTERSECTION1PART(partno) (WLFIRST + 5 + (WLEPP * partno))
#define WLPROJECTION1(partno) (WLFIRST + 6 + (WLEPP * partno))
#define WLPROJECTION2(partno) (WLFIRST + 7 + (WLEPP * partno))
#define WLLB 0 /* Lower bound */
#define WLUB 1 /* Upper bound */

#define NOINVERSEPROJECTION (-666)
// NOINVERSEPROJECTION is just a highly visible number that is not a legal index

#endif /* _SAC_ALGEBRAIC_WLFI_H_ */
