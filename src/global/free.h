/*
 *
 * $Log$
 * Revision 1.9  1997/03/19 13:34:57  cg
 * added functions FreeAllDeps() and FreeOneDeps()
 *
 * Revision 1.8  1995/12/29  10:24:18  cg
 * inserted call to malloc_verify into macro FREE, added FreeInfo
 *
 * Revision 1.7  1995/12/20  08:14:25  cg
 * new function FreeChar for new N_char node
 *
 * Revision 1.6  1995/12/18  16:14:08  cg
 * macro FREE modified: now you can compile with -DNOFREE to disable
 * all memory deallocation caused by this macro.
 *
 * Revision 1.5  1995/12/01  16:16:12  cg
 * new macro FREE is designed to replace macros with the same name
 * in typecheck, compile, optimize, etc.
 *
 * Revision 1.4  1995/11/16  19:33:24  cg
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

#include "dbug.h"
#include "my_debug.h"
#include "malloc.h"

#ifndef NOFREE

#define FREE(address)                                                                    \
    if ((address) != NULL) {                                                             \
        DBUG_PRINT ("FREEMEM", ("Free memory at adress: " P_FORMAT, (address)));         \
        free ((address));                                                                \
        DBUG_EXECUTE ("MEMVERIFY", malloc_verify (););                                   \
        address = NULL;                                                                  \
    }

#else

#define FREE(address)

#endif /* NOFREE */

extern node *FreeNode (node *);
extern node *FreeTree (node *);

extern shpseg *FreeShpseg (shpseg *fr);
extern types *FreeOneTypes (types *fr);
extern types *FreeAllTypes (types *fr);
extern ids *FreeOneIds (ids *fr);
extern ids *FreeAllIds (ids *fr);
extern nums *FreeOneNums (nums *fr);
extern nums *FreeAllNums (nums *fr);
extern deps *FreeOneDeps (deps *fr);
extern deps *FreeAllDeps (deps *fr);
extern strings *FreeOneStrings (strings *fr);
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
extern node *FreeChar (node *arg_node, node *arg_info);
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
extern node *FreePragma (node *arg_node, node *arg_info);
extern node *FreeInfo (node *arg_node, node *arg_info);

extern void FreePrf2 (node *arg_node, int arg_no);

/*******************************************************************/

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
