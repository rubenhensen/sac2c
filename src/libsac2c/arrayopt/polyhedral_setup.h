/*
 * $Id$
 *
 */

#ifndef _SAC_POLYHEDRAL_SETUP_H_
#define _SAC_POLYHEDRAL_SETUP_H_

#include "types.h"

extern node *POLYSdoPolyhedralSetup (node *arg_node);
extern node *POLYSdoPolyhedralTearDown (node *arg_node);

extern node *POLYSfundef (node *arg_node, info *arg_info);
extern node *POLYSpart (node *arg_node, info *arg_info);
extern node *POLYSwith (node *arg_node, info *arg_info);
extern node *POLYSassign (node *arg_node, info *arg_info);
extern node *POLYSap (node *arg_node, info *arg_info);
extern node *POLYSlet (node *arg_node, info *arg_info);
extern node *POLYSprf (node *arg_node, info *arg_info);
extern node *POLYSsetClearAvisPart (node *arg_node, node *val);
extern void POLYSsetClearCallAp (node *arg_node, node *callerfundef, node *nassign,
                                 bool setclear);

#endif // _SAC_POLYHEDRAL_SETUP_H_
