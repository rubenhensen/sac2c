/*
 *
 * $Log$
 * Revision 1.3  2002/07/11 13:59:00  dkr
 * AddNtTag() added
 *
 * Revision 1.2  2002/06/02 21:42:42  dkr
 * symbols renamed
 *
 * Revision 1.1  2002/05/31 17:14:57  dkr
 * Initial revision
 *
 */

#ifndef _NameTuplesUtils_h_
#define _NameTuplesUtils_h_

#include "NameTuples.h"

extern char *CreateNtTag (char *name, types *type);
extern node *AddNtTag (node *id);

extern unq_class_t GetUnqClassFromTypes (types *type);
extern data_class_t GetDataClassFromTypes (types *type);

#endif /* _NameTuplesUtils_h_ */
