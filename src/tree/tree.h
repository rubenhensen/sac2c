/*
 *
 * $Log$
 * Revision 1.3  2000/10/24 14:29:03  dkr
 * MakeTypes removed
 *
 * Revision 1.2  2000/07/11 10:23:00  dkr
 * macro GEN_NODE moved to tree.c
 *
 * Revision 1.1  2000/01/21 15:38:43  dkr
 * Initial revision
 *
 * Revision 2.2  1999/09/10 14:20:23  jhs
 * Removed those ugly macros: MAKENODE_NUM, MAKENODE_ID, MAKENODE_BOOL,
 * MAKENODE_FLOAT, MAKENODE_DOUBLE, MAKENODE_ID_REUSE_IDS.
 *
 * Revision 2.1  1999/02/23 12:39:51  sacbase
 * new release made
 *
 * Revision 1.66  1999/01/06 13:04:14  cg
 * extern declaration of prf_name_str moved from tree.h to tree_basic.h
 *
 * Revision 1.65  1997/11/26 11:01:05  srs
 * removed use of old macros from acssass_macros.h
 *
 * Revision 1.64  1996/02/12 16:32:47  cg
 * macro MAKENODE_ID_REUSE_IDS corrected: refcount will be copied
 * from ids-structure to node-structure now
 *
 * Revision 1.63  1995/12/28  10:31:18  cg
 * Malloc is used instead of malloc in GEN_NODE
 *
 * Revision 1.62  1995/12/04  13:08:02  hw
 * changed makro MODEMODE_ID(no, str) ( str will be copied now)
 *
 * Revision 1.61  1995/11/01  16:24:16  cg
 * moved function AppendIdsChain to tree_compound.[ch]
 *
 * Revision 1.60  1995/10/06  17:10:44  cg
 * call to MakeIds adjusted to new signature (3 parameters)
 *
 * Revision 1.59  1995/09/27  15:16:54  cg
 * ATTENTION:
 * tree.c and tree.h are not part of the new virtual syntax tree.
 * They are kept for compatibility reasons with old code only !
 * All parts of their old versions which are to be used in the future are moved
 * to tree_basic and tree_compound.
 * DON'T use tree.c and tree.h when writing new code !!
 *
 * [removed]
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 */

#ifndef _sac_tree_h_
#define _sac_tree_h_

/*
 * The following included header files contain all the syntax tree
 * information. They are included here for compatability reasons only!
 * Please, include them directly when your files are converted to the
 * new virtual syntax tree.
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

extern node *MakeNode (nodetype nodetype);
extern node *AppendNodeChain (int dummy, node *first, node *second);

#endif /* _sac_tree_h  */
