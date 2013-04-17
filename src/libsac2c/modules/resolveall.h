#ifndef _SAC_RESOLVE_ALL_H_
#define _SAC_RESOLVE_ALL_H_

#include "types.h"

/******************************************************************************
 *
 * Resolve all traversal ( rsa_tab)
 *
 * Prefix: RSA
 *
 *****************************************************************************/
extern node *RSAdoResolveAll (node *modul);

extern node *RSAuse (node *arg_node, info *arg_info);
extern node *RSAimport (node *arg_node, info *arg_info);
extern node *RSAprovide (node *arg_node, info *arg_info);
extern node *RSAexport (node *arg_node, info *arg_info);
extern node *RSAmodule (node *arg_node, info *arg_info);

#endif /* _SAC_RESOLVE_ALL_H_ */
