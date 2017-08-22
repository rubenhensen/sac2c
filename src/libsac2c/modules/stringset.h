#ifndef _SAC_STRINGSET_H_
#define _SAC_STRINGSET_H_

#include "types.h"

/******************************************************************************
 *
 * String set
 *
 * Prefix: STRS
 *
 *****************************************************************************/
extern bool STRScontains (const char *string, stringset_t *set);
extern stringset_t *STRSadd (const char *string, strstype_t kind, stringset_t *set);
extern void *STRSfold (strsfoldfun_p fun, stringset_t *set, void *init);
extern stringset_t *STRSjoin (stringset_t *one, stringset_t *two);
extern stringset_t *STRSfree (stringset_t *set);
extern void STRStouch (stringset_t *set, info *arg_info);
extern stringset_t *STRSduplicate (stringset_t *src);
extern void *STRStoSafeCEncodedStringFold (const char *, strstype_t, void *);
extern void STRSprint (stringset_t *set);

#endif /* _SAC_STRINGSET_H_ */
