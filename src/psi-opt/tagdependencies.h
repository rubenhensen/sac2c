/*
 *
 * $Log$
 * Revision 1.3  2004/10/20 08:10:29  khf
 * changed signature of startfunction
 *
 * Revision 1.2  2004/09/02 15:27:01  khf
 * TDEPENDwithop removed
 *
 * Revision 1.1  2004/08/26 15:06:32  khf
 * Initial revision
 *
 *
 *
 */

#ifndef _tagdependencies_h
#define _tagdependencies_h

extern node *TagDependencies (node *with, node *fusionable_wl);

extern node *TDEPENDassign (node *arg_node, info *arg_info);
extern node *TDEPENDid (node *arg_node, info *arg_info);

extern node *TDEPENDwith (node *arg_node, info *arg_info);

#endif
