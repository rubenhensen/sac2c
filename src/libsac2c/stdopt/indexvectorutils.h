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

extern node *IVUTshapevectorFromShapeArray (node *iv);
extern node *IVUTarrayFromIv (node *iv);
extern bool IVUTisWLShapesMatch (node *pwl, node *cwl, node *cwlwith, node *pwlwith);

#endif /* _SAC_IVUT_TRAV_H_ */
