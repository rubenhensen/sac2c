/*
 *
 * $Log$
 * Revision 1.7  2003/09/25 10:53:33  dkr
 * GetFoldCode() added
 *
 * Revision 1.6  2002/08/07 13:49:46  dkr
 * COMP2With added
 *
 * Revision 1.5  2002/06/02 21:39:02  dkr
 * some more TAGGED_ARRAYS stuff added
 *
 * Revision 1.4  2002/04/03 14:42:56  dkr
 * COMP2Arg removed
 *
 * Revision 1.3  2001/12/13 11:49:08  dkr
 * some more functions removed
 *
 * Revision 1.2  2001/12/13 11:12:06  dkr
 * some functions removed
 *
 * Revision 1.1  2001/12/10 15:34:16  dkr
 * Initial revision
 *
 */

#ifndef _sac_compile_tagged_h
#define _sac_compile_tagged_h

extern node *GetFoldCode (node *fundef);

extern node *Compile_Tagged (node *arg_node);

extern node *COMP2Modul (node *arg_node, node *arg_info);
extern node *COMP2Typedef (node *arg_node, node *arg_info);
extern node *COMP2Objdef (node *arg_node, node *arg_info);
extern node *COMP2Vardec (node *arg_node, node *arg_info);
extern node *COMP2Prf (node *arg_node, node *arg_info);
extern node *COMP2Assign (node *arg_node, node *arg_info);
extern node *COMP2Let (node *arg_node, node *arg_info);
extern node *COMP2Scalar (node *arg_node, node *arg_info);
extern node *COMP2Array (node *arg_node, node *arg_info);
extern node *COMP2Id (node *arg_node, node *arg_info);
extern node *COMP2Ap (node *arg_node, node *arg_info);
extern node *COMP2Return (node *arg_node, node *arg_info);
extern node *COMP2Fundef (node *arg_node, node *arg_info);
extern node *COMP2Loop (node *arg_node, node *arg_info);
extern node *COMP2Cond (node *arg_node, node *arg_info);
extern node *COMP2Block (node *arg_node, node *arg_info);
extern node *COMP2Icm (node *arg_node, node *arg_info);
extern node *COMP2Cast (node *arg_node, node *arg_info);
extern node *COMP2Spmd (node *arg_node, node *arg_info);
extern node *COMP2Sync (node *arg_node, node *arg_info);

extern node *COMP2With (node *arg_node, node *arg_info);
extern node *COMP2With2 (node *arg_node, node *arg_info);
extern node *COMP2WLsegx (node *arg_node, node *arg_info);
extern node *COMP2WLxblock (node *arg_node, node *arg_info);
extern node *COMP2WLstridex (node *arg_node, node *arg_info);
extern node *COMP2WLgridx (node *arg_node, node *arg_info);
extern node *COMP2WLcode (node *arg_node, node *arg_info);

extern node *COMP2Mt (node *arg_node, node *arg_info);
extern node *COMP2St (node *arg_node, node *arg_info);
extern node *COMP2MTsignal (node *arg_node, node *arg_info);
extern node *COMP2MTalloc (node *arg_node, node *arg_info);
extern node *COMP2MTsync (node *arg_node, node *arg_info);

#endif /* _sac_compile_tagged_h */
