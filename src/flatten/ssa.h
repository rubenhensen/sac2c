/*
 * $Log$
 * Revision 1.1  2004/02/02 15:48:32  skt
 * Initial revision
 *
 */

/*
 * Tool package to create resp. to reverse the ssa-form
 */

#include "types.h"

#ifndef _ssa_h

#define _ssa_h
extern node *DoSSA (node *syntax_tree);

extern node *UndoSSA (node *syntax_tree);
#endif /* ssa_h */
