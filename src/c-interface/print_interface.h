/*
 *
 * $Log$
 * Revision 3.3  2004/11/22 11:00:05  ktr
 * Ismop 2004 SacDevCamp 04
 *
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

#ifndef _SAC_PRINT_INTERFACE_H_
#define _SAC_PRINT_INTERFACE_H_

#include "types.h"

/******************************************************************************
 *
 * Print Interface
 *
 * Prefix: PI
 *
 *****************************************************************************/
extern node *PIdoPrintInterface (node *syntax_tree);

#endif /* _SAC_PRINT_INTERFACE_H_ */
