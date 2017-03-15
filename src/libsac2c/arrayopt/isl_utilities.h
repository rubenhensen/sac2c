#ifndef _SAC_ISLU_H_
#define _SAC_ISLU_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Prefix: ISLU
 *
 *****************************************************************************/
extern int ISLUgetLoopCount (char *str, lut_t *varlut);
extern char *ISLUexprs2String (node *exprs, lut_t *varlut, char *lbl, bool isunionset,
                               char *lhsname);
extern int ISLUgetSetIntersections (node *exprspwl, node *exprscwl, node *exprsfn,
                                    node *exprscfn, lut_t *varlut, char *lhsname);
#endif /* _SAC_ISLU_H_ */
