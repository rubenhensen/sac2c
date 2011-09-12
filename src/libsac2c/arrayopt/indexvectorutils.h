/*
 * $Id$
 */
#ifndef _SAC_IVUT_TRAV_H_
#define _SAC_IVUT_TRAV_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Prefix: IVUT
 *
 *****************************************************************************/

extern node *IVUToffset2Iv (node *arg_node, node **vardecs, node **preassigns,
                            node *cwlpart);
extern node *IVUTarrayFromProxy (node *iv);
extern node *IVUTarrayFromIv (node *iv);
extern bool IVUTisShapesMatch (node *plet, node *clet, node *cletshap);
extern node *IVUTfindIv (node *arg_node, node *cwlpart);
extern constant *IVUToffset2Constant (node *arg_node);

#endif /* _SAC_IVUT_TRAV_H_ */
