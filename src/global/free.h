/*
 *
 * $Log$
 * Revision 1.2  1995/01/18 17:28:37  asi
 * Added FreeTree, FreeNoInfo, FreeInfoId, FreeInfoIds, FreeInfoType, FreeModul
 *
 * Revision 1.1  1994/12/27  09:28:19  sbs
 * Initial revision
 *
 *
 */

#ifndef _sac_free_h

#define _sac_free_h

extern void FreeIds (ids *ids);
extern void FreeIdsOnly (ids *ids);
extern void FreeImplist (node *implist);

extern void FreeTree (node *arg_node);

extern node *FreeNoInfo (node *arg_node, node *arg_info);
extern node *FreeInfoId (node *arg_node, node *arg_info);
extern node *FreeInfoIds (node *arg_node, node *arg_info);
extern node *FreeInfoType (node *arg_node, node *arg_info);
extern node *FreeModul (node *arg_node, node *arg_info);

#endif /* _sac_free_h */
