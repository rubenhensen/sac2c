#ifndef _SAC_IVUT_TRAV_H_
#define _SAC_IVUT_TRAV_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Prefix: IVUT
 *
 *****************************************************************************/

extern constant *IVUToffset2Constant (node *arg_node, node *mat);
extern node *IVUToffset2Vect (node *arg_node, node **vardecs, node **preassigns,
                              node *cwlpart);
extern node *IVUTarrayFromProxy (node *iv);
extern node *IVUTarrayFromProxySel (node *iv);
extern node *IVUTarrayFromProxyIdxsel (node *iv);
extern node *IVUTmatFromIv (node *iv);
extern bool IVUTisShapesMatch (node *plet, node *clet, node *cletshap);
extern node *IVUTfindIvWithid (node *arg_node, node *cwlpart);
extern constant *IVUTiV2Constant (node *arg_node);
extern node *IVUTfindOffset2Iv (node *arg_node);
extern node *IVUToffset2IV (node *arg_node);
extern bool IVUToffsetMatchesOffset (node *iv1, node *iv2);
extern bool IVUTisIvMatchesWithid (node *iv, node *withidvec, node *withidids);

#endif /* _SAC_IVUT_TRAV_H_ */
