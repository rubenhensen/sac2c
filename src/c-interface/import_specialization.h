/*
 * $Log$
 * Revision 1.1  2000/07/21 08:18:08  nmw
 * Initial revision
 *
 *
 */

#ifndef _sac_import_specialization_h
#define _sac_import_specialization_h

#include "tree.h"
#include "globals.h"

extern node *IMPSPECfundef (node *arg_node, node *arg_info);
extern node *IMPSPECmodspec (node *arg_node, node *arg_info);
extern node *IMPSPECarg (node *arg_node, node *arg_info);

extern node *ImportSpecialization (node *syntax_tree);

#endif /* _sac_map_cwrapper_h */
