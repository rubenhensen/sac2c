/*
 *
 * $Log$
 * Revision 1.1  2004/08/26 15:06:32  khf
 * Initial revision
 *
 *
 *
 */

#ifndef _tagdependencies_h
#define _tagdependencies_h

extern node *TagDependencies (node *arg_node);

extern node *TDEPENDassign (node *arg_node, info *arg_info);
extern node *TDEPENDid (node *arg_node, info *arg_info);

extern node *TDEPENDwith (node *arg_node, info *arg_info);
extern node *TDEPENDwithop (node *arg_node, info *arg_info);

#endif
