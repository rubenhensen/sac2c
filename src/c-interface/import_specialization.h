/*
 *
 * $Log$
 * Revision 3.2  2001/03/22 18:02:55  dkr
 * tree.h no longer included
 *
 * Revision 3.1  2000/11/20 18:03:40  sacbase
 * new release made
 *
 * Revision 1.1  2000/07/21 08:18:08  nmw
 * Initial revision
 *
 */

#ifndef _sac_import_specialization_h
#define _sac_import_specialization_h

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

extern node *IMPSPECfundef (node *arg_node, node *arg_info);
extern node *IMPSPECmodspec (node *arg_node, node *arg_info);
extern node *IMPSPECarg (node *arg_node, node *arg_info);

extern node *ImportSpecialization (node *syntax_tree);

#endif /* _sac_map_cwrapper_h */
