/*
 *
 * $Log$
 * Revision 1.7  2004/11/26 21:30:51  ktr
 * ntype backend stage 0
 *
 * Revision 1.6  2004/11/26 11:56:11  skt
 * added const for first arg of NTUgetShapeClassFromTypes
 *
 * Revision 1.5  2004/11/22 15:42:55  ktr
 * SACDevCamp 04 Ismop
 *
 * Revision 1.4  2002/07/31 15:35:08  dkr
 * new hidden tag added
 *
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

extern shape_class_t NTUgetShapeClassFromNType (ntype *ntype);
extern hidden_class_t NTUgetHiddenClassFromNType (ntype *ntype);
extern unique_class_t NTUgetUniqueClassFromNType (ntype *ntype);

#endif /* _SAC_NAMETUPLESUTILS_H_ */
