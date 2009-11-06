/*
 * $Id$
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
extern node *USSobjdef (node *arg_node, info *arg_info);
extern node *USSspfold (node *arg_node, info *arg_info);
extern node *USSavis (node *arg_node, info *arg_info);
extern node *USScast (node *arg_node, info *arg_info);
extern node *USSarray (node *arg_node, info *arg_info);
extern node *USSret (node *arg_node, info *arg_info);
extern node *USSspap (node *arg_node, info *arg_info);
extern node *USSspmop (node *arg_node, info *arg_info);
extern node *USSspid (node *arg_node, info *arg_info);
extern node *USSmodule (node *arg_node, info *arg_info);

#endif /* _SAC_USESYMBOLS_H_ */
