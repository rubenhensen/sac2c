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
extern mutc_storage_class_class_t NTUMutcgetStorageClassFromTypes (types *type);
extern mutc_scope_class_t NTUgetMutcScopeFromTypes (types *type);
extern mutc_usage_class_t NTUgetMutcUsageFromTypes (types *type);
extern bitarray_class_t NTUgetBitarrayFromTypes (types *type);
extern distributed_class_t NTUgetDistributedFromTypes (types *type);

extern char *NTUcreateNtTagFromNType (const char *name, ntype *ntype);

extern shape_class_t NTUgetShapeClassFromNType (ntype *ntype);
extern hidden_class_t NTUgetHiddenClassFromNType (ntype *ntype);
extern unique_class_t NTUgetUniqueClassFromNType (ntype *ntype);
extern mutc_storage_class_class_t NTUgetMutcStorageClassFromNType (ntype *ntype);
extern mutc_scope_class_t NTUgetMutcScopeFromNType (ntype *ntype);
extern mutc_usage_class_t NTUgetMutcUsageFromNType (ntype *ntype);
extern bitarray_class_t NTUgetBitarrayFromNtype (ntype *ntype);
extern distributed_class_t NTUgetDistributedFromNType (ntype *type);

#endif /* _SAC_NAMETUPLESUTILS_H_ */
