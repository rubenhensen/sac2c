/*
 *
 * $Log$
 * Revision 1.1  2001/12/10 15:34:16  dkr
 * Initial revision
 *
 */

#ifndef _sac_compile_tagged_h
#define _sac_compile_tagged_h

#if 0
extern node *MakeAdjustRcIcm( char *name, types* type, int rc, int num);
extern node *MakeIncRcIcm( char *name, types *type, int rc, int num);
extern node *MakeDecRcIcm( char *name, types* type, int rc, int num);
#endif

extern node *Compile_Tagged (node *arg_node);

extern node *COMP2Modul (node *arg_node, node *arg_info);
extern node *COMP2Typedef (node *arg_node, node *arg_info);
extern node *COMP2Objdef (node *arg_node, node *arg_info);
extern node *COMP2Vardec (node *arg_node, node *arg_info);
extern node *COMP2Prf (node *arg_node, node *arg_info);
extern node *COMP2Assign (node *arg_node, node *arg_info);
extern node *COMP2Let (node *arg_node, node *arg_info);
extern node *COMP2Array (node *arg_node, node *arg_info);
extern node *COMP2Id (node *arg_node, node *arg_info);
extern node *COMP2Ap (node *arg_node, node *arg_info);
extern node *COMP2Return (node *arg_node, node *arg_info);
extern node *COMP2Arg (node *arg_node, node *arg_info);
extern node *COMP2Fundef (node *arg_node, node *arg_info);
extern node *COMP2Loop (node *arg_node, node *arg_info);
extern node *COMP2Cond (node *arg_node, node *arg_info);
extern node *COMP2Block (node *arg_node, node *arg_info);
extern node *COMP2Icm (node *arg_node, node *arg_info);
extern node *COMP2Cast (node *arg_node, node *arg_info);
extern node *COMP2Spmd (node *arg_node, node *arg_info);
extern node *COMP2Sync (node *arg_node, node *arg_info);

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

#if 0
extern char *GenericFun( int which, types *type);
extern node *GetFoldCode( node *fundef);
extern node *GetFoldVardecs( node *fundef);
#endif

#endif /* _sac_compile_tagged_h */
