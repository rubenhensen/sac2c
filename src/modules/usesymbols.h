/*
 *
 * $Log$
 * Revision 1.6  2004/12/05 16:45:38  sah
 * added SPIds SPId SPAp in frontend
 *
 * Revision 1.5  2004/11/25 11:35:11  sah
 * COMPILES
 *
 * Revision 1.4  2004/11/22 16:57:41  ktr
 * SACDevCamp 04 Ismop
 *
 * Revision 1.3  2004/11/11 14:29:40  sah
 * added some traversal functions for USS traversal
 *
 * Revision 1.2  2004/10/22 14:48:16  sah
 * fixed some typeos
 *
 * Revision 1.1  2004/10/22 13:50:49  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_USESYMBOLS_H_
#define _SAC_USESYMBOLS_H_

#include "types.h"

/******************************************************************************
 *
 * Use symbols
 *
 * Prefix: USS
 *
 *****************************************************************************/
extern node *USSdoUseSymbols (node *modul);

extern node *USStypedef (node *arg_node, info *arg_info);
extern node *USSfold (node *arg_node, info *arg_info);
extern node *USSarg (node *arg_node, info *arg_info);
extern node *USSvardec (node *arg_node, info *arg_info);
extern node *USSspap (node *arg_node, info *arg_info);
extern node *USSmodule (node *arg_node, info *arg_info);

#endif /* _SAC_USESYMBOLS_H_ */
