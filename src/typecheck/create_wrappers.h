/*
 *
 * $Log$
 * Revision 1.10  2004/09/30 15:11:18  sbs
 * CreteFuntype splitted into CreateFuntype and CreateFunRettype
 *
 * Revision 1.9  2004/07/30 17:29:21  sbs
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.8  2002/10/18 14:31:38  sbs
 * CRTWRPid added
 *
 * Revision 1.7  2002/08/05 17:00:38  sbs
 * first alpha version of the new type checker !!
 *
 * Revision 1.6  2002/05/31 14:43:06  sbs
 * CRTWRPlet added
 *
 * Revision 1.5  2002/03/12 15:13:32  sbs
 * CRTWRPxxxx traversal function added.
 *
 * Revision 1.4  2002/03/05 15:51:17  sbs
 * *** empty log message ***
 *
 * Revision 1.3  2002/03/05 15:43:11  sbs
 * *** empty log message ***
 *
 * Revision 1.2  2002/03/05 15:40:40  sbs
 * CRTWRP traversal embedded.
 *
 * Revision 1.1  2002/03/05 13:59:29  sbs
 * Initial revision
 *
 *
 */

#ifndef _create_wrappers_h
#define _create_wrappers_h

#include "types.h"
#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

extern node *CreateWrappers (node *arg_node);
extern ntype *CreateFunRettype (types *old_ret);
extern ntype *CreateFuntype (node *fundef);

extern node *CRTWRPmodul (node *arg_node, info *arg_info);
extern node *CRTWRPfundef (node *arg_node, info *arg_info);
extern node *CRTWRPlet (node *arg_node, info *arg_info);
extern node *CRTWRPap (node *arg_node, info *arg_info);
extern node *CRTWRPid (node *arg_node, info *arg_info);
extern node *CRTWRPNwithop (node *arg_node, info *arg_info);

#endif /* _create_wrappers_h */
