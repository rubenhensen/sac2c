/*
 *
 * $Log$
 * Revision 1.15  1997/12/06 17:14:55  srs
 * changed FREE
 *
 * Revision 1.14  1997/11/23 15:17:52  dkr
 * CC-flag: show_malloc -> SHOW_MALLOC
 *
 * Revision 1.13  1997/11/13 16:11:31  srs
 * free functions for the new WL-syntaxtree
 *
 * Revision 1.12  1997/10/29 14:55:44  srs
 * changed FREE and removed HAVE_MALLOC_O
 *
 * Revision 1.11  1997/10/29 13:01:00  srs
 * new macro FREE (show_malloc).
 * removed HAVE_MALLOC_O
 *
 * Revision 1.10  1997/04/24 14:05:37  sbs
 * HAVE_MALLOC_O inserted
 *
 * Revision 1.9  1997/03/19  13:34:57  cg
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
#include "internal_lib.h"

#ifndef NOFREE

#ifdef SHOW_MALLOC
#define FREE(address)                                                                    \
    if ((address) != NULL) {                                                             \
        DBUG_PRINT ("FREEMEM", ("Free memory at adress: " P_FORMAT, (address)));         \
        address = ((void *)address) - malloc_align_step;                                 \
        current_allocated_mem -= *(int *)(address);                                      \
                                                                                         \
        free (address);                                                                  \
        address = NULL;                                                                  \
    }
#else /* not SHOW_MALLOC */
#define FREE(address)                                                                    \
    if ((address) != NULL) {                                                             \
        DBUG_PRINT ("FREEMEM", ("Free memory at adress: " P_FORMAT, (address)));         \
        free ((address));                                                                \
        address = NULL;                                                                  \
    }
#endif

#else /* NOFREE */

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

/* new withloops */
extern node *FreeNWith (node *arg_node, node *arg_info);
extern node *FreeNPart (node *arg_node, node *arg_info);
extern node *FreeNWithID (node *arg_node, node *arg_info);
extern node *FreeNGenerator (node *arg_node, node *arg_info);
extern node *FreeNWithOp (node *arg_node, node *arg_info);
extern node *FreeNCode (node *arg_node, node *arg_info);

/* special functions */
extern void FreePrf2 (node *arg_node, int arg_no); /* CF */

#endif /* _sac_free_h */
