/*
 *
 * $Log$
 * Revision 3.2  2001/03/22 18:54:38  dkr
 * include of tree.h eliminated
 *
 * Revision 3.1  2000/11/20 18:03:42  sacbase
 * new release made
 *
 * Revision 1.4  2000/08/02 14:27:33  nmw
 * PIW and PIH traversal functions moved to separate files
 *
 * Revision 1.3  2000/07/24 15:01:48  nmw
 * PIWobjdef added
 *
 * Revision 1.2  2000/07/12 10:07:45  nmw
 * RCS-header added
 *
 */

#ifndef _sac_print_interface_h
#define _sac_print_interface_h

#include "globals.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

extern node *PrintInterface (node *syntax_tree);

#endif /* _sac_print_interface_h */
