/*
 *
 * $Log$
 * Revision 1.4  1995/11/16 19:33:24  cg
 * The free module was entirely rewritten.
 * Each node type now has its own free function.
 * Functions FreeTree and FreeNode for deleting single nodes and
 * whole sub trees.
 * Various free functions for non-node structures.
 * FreeNodelist moved from tree_compound.c
 *
 * Revision 1.3  1995/06/16  13:10:46  asi
 * added FreePrf2, which will free memory occupied by a N_prf-subtree,
 *                 but it will not free one given argument.
 *
 * Revision 1.2  1995/01/18  17:28:37  asi
 * Added FreeTree, FreeNoInfo, FreeInfoId, FreeInfoIds, FreeInfoType, FreeModul
 *
 * Revision 1.1  1994/12/27  09:28:19  sbs
 * Initial revision
 *
 *
 */

#ifndef _sac_free_h

#define _sac_free_h

extern node *FreeNode (node *);
extern node *FreeTree (node *);

extern shpseg *FreeShpseg (shpseg *fr);
extern shpseg *FreeAllShpseg (shpseg *fr);
extern types *FreeTypes (types *fr);
extern types *FreeAllTypes (types *fr);
extern ids *FreeIds (ids *fr);
extern ids *FreeAllIds (ids *fr);
extern nums *FreeNums (nums *fr);
extern nums *FreeAllNums (nums *fr);
extern strings *FreeStrings (strings *fr);
extern strings *FreeAllStrings (strings *fr);
extern nodelist *FreeNodelist (nodelist *fr);

extern node *FreeModul (node *arg_node, node *arg_info);
extern node *FreeModdec (node *arg_node, node *arg_info);
extern node *FreeClassdec (node *arg_node, node *arg_info);
extern node *FreeSib (node *arg_node, node *arg_info);
extern node *FreeImplist (node *arg_node, node *arg_info);
extern node *FreeExplist (node *arg_node, node *arg_info);
extern node *FreeTypedef (node *arg_node, node *arg_info);
extern node *FreeObjdef (node *arg_node, node *arg_info);
extern node *FreeFundef (node *arg_node, node *arg_info);
extern node *FreeArg (node *arg_node, node *arg_info);
extern node *FreeBlock (node *arg_node, node *arg_info);
extern node *FreeVardec (node *arg_node, node *arg_info);
extern node *FreeAssign (node *arg_node, node *arg_info);
extern node *FreeLet (node *arg_node, node *arg_info);
extern node *FreeCast (node *arg_node, node *arg_info);
extern node *FreeReturn (node *arg_node, node *arg_info);
extern node *FreeCond (node *arg_node, node *arg_info);
extern node *FreeDo (node *arg_node, node *arg_info);
extern node *FreeWhile (node *arg_node, node *arg_info);
extern node *FreeAp (node *arg_node, node *arg_info);
extern node *FreeWith (node *arg_node, node *arg_info);
extern node *FreeGenerator (node *arg_node, node *arg_info);
extern node *FreeGenarray (node *arg_node, node *arg_info);
extern node *FreeModarray (node *arg_node, node *arg_info);
extern node *FreeFoldprf (node *arg_node, node *arg_info);
extern node *FreeFoldfun (node *arg_node, node *arg_info);
extern node *FreeExprs (node *arg_node, node *arg_info);
extern node *FreeArray (node *arg_node, node *arg_info);
extern node *FreeVinfo (node *arg_node, node *arg_info);
extern node *FreeId (node *arg_node, node *arg_info);
extern node *FreeNum (node *arg_node, node *arg_info);
extern node *FreeFloat (node *arg_node, node *arg_info);
extern node *FreeDouble (node *arg_node, node *arg_info);
extern node *FreeBool (node *arg_node, node *arg_info);
extern node *FreeStr (node *arg_node, node *arg_info);
extern node *FreePrf (node *arg_node, node *arg_info);
extern node *FreeEmpty (node *arg_node, node *arg_info);
extern node *FreePost (node *arg_node, node *arg_info);
extern node *FreePre (node *arg_node, node *arg_info);
extern node *FreeIcm (node *arg_node, node *arg_info);
extern node *FreeDec (node *arg_node, node *arg_info);
extern node *FreeInc (node *arg_node, node *arg_info);

extern void FreePrf2 (node *arg_node, int arg_no);

#if 0 /* Old Stuff from first version of free.c */

extern void FreeIds(ids *ids);
extern void FreeIdsOnly(ids *ids);
extern void FreeImplist(node *implist);

extern void FreeTree(node *arg_node);

extern node *FreeNoInfo   (node *arg_node ,node *arg_info);
extern node *FreeInfoId   (node *arg_node ,node *arg_info);
extern node *FreeInfoIds  (node *arg_node ,node *arg_info);
extern node *FreeInfoType (node *arg_node ,node *arg_info);
extern node *FreeModul    (node *arg_node ,node *arg_info);

extern void FreePrf2(node *arg_node, int arg_no);

#endif /* 0 */

#endif /* _sac_free_h */
