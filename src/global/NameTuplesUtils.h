/* $Log$ */

#ifndef _SAC_NAMETUPLESUTILS_H_
#define _SAC_NAMETUPLESUTILS_H_

#include "types.h"

/******************************************************************************
 *
 * Name Tuples Utils
 *
 * Prefix: NTU
 *
 *****************************************************************************/
extern char *NTUcreateNtTag (const char *name, types *type);

extern node *NTUaddNtTag (node *id);

extern shape_class_t NTUgetShapeClassFromTypes (types *type);
extern hidden_class_t NTUgetHiddenClassFromTypes (types *type);
extern unique_class_t NTUgetUniqueClassFromTypes (types *type);

extern char *NTUcreateNtTagFromNType (const char *name, ntype *ntype);

extern shape_class_t NTUgetShapeClassFromNType (ntype *ntype);
extern hidden_class_t NTUgetHiddenClassFromNType (ntype *ntype);
extern unique_class_t NTUgetUniqueClassFromNType (ntype *ntype);

#endif /* _SAC_NAMETUPLESUTILS_H_ */
