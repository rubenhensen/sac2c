/*
 * $Log$
 * Revision 1.3  2004/09/18 15:58:34  ktr
 * added RestoreSSAExplicitAllocs
 *
 * Revision 1.2  2004/02/25 15:53:06  cg
 * New functions RestoreSSAOneFunction and RestoreSSAOneFundef
 * now provide access to SSA transformations on a per function
 * basis.
 * Only functions from ssa.[ch] should be used to initiate the
 * transformation process in either direction!
 *
 * Revision 1.1  2004/02/02 15:48:32  skt
 * Initial revision
 *
 */

/*
 * Tool package to create resp. to reverse the ssa-form
 */

#ifndef _ssa_h
#define _ssa_h

#include "types.h"

extern node *DoSSA (node *syntax_tree);
extern node *UndoSSA (node *syntax_tree);
extern node *RestoreSSAExplicitAllocs (node *syntax_tree);
extern node *RestoreSSAOneFunction (node *fundef);
extern node *RestoreSSAOneFundef (node *fundef);
#endif /* ssa_h */
