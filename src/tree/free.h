/*
 *
 * $Log$
 * Revision 3.2  2000/12/12 12:13:59  dkr
 * nodes N_pre, N_post, N_inc, N_dec added
 *
 * Revision 3.1  2000/11/20 18:03:24  sacbase
 * new release made
 *
 * Revision 1.5  2000/07/21 08:20:15  nmw
 * FreeModspec added
 *
 * Revision 1.4  2000/06/23 15:32:44  nmw
 * FreeCWrapper added
 *
 * Revision 1.3  2000/06/13 12:25:28  dkr
 * functions for old with-loop removed
 *
 * Revision 1.2  2000/03/29 16:09:54  jhs
 * FreeST and FreeMT added.
 *
 * Revision 1.1  2000/01/21 15:38:23  dkr
 * Initial revision
 *
 * Revision 2.3  1999/10/04 12:33:53  sbs
 * globals.h included
 * (needed by FREE
 *
 * Revision 2.2  1999/02/28 21:07:40  srs
 * added macro FREE_INDEX_INFO
 *
 * Revision 2.1  1999/02/23 12:39:19  sacbase
 * new release made
 *
 * Revision 1.27  1999/02/11 13:37:23  cg
 * Improved debugging opportunities by hiding calls to free() behind
 * wrapper function Free().
 *
 * Revision 1.26  1999/02/06 12:53:23  srs
 * added FreeNodelistNode()
 *
 * Revision 1.25  1999/01/15 15:13:19  cg
 * added functions FreeOneAccess() and FreeAllAccess().
 *
 * Revision 1.24  1998/08/07 14:36:48  dkr
 * FreeWLsegVar added
 *
 * Revision 1.23  1998/04/26 21:51:28  dkr
 * FreeSPMD renamed to FreeSpmd
 *
 * Revision 1.22  1998/04/23 17:32:19  dkr
 * added FreeSync
 *
 * Revision 1.21  1998/04/17 17:26:00  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.20  1998/04/02 17:40:54  dkr
 * added FreeConc
 *
 * Revision 1.19  1998/04/01 23:55:39  dkr
 * added FreeWLstriVar, FreeWLgridVar
 *
 * Revision 1.18  1998/03/27 18:38:09  dkr
 * WLproj renamed in WLstride:
 *   WLPROJ... -> WLSTRIDE...
 *   FreeWLproj -> FreeWLstride
 *
 * Revision 1.17  1998/03/16 00:07:33  dkr
 * added FreeWLseg, FreeWLblock, FreeWLublock, FreeWLproj, FreeWLgrid, FreeNwith2
 *
 * Revision 1.16  1997/12/08 19:19:01  dkr
 * no arithmetic on void-pointers anymore (FREE)
 *
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
#include "globals.h"
#include "internal_lib.h"

#ifdef DBUG_OFF
#define DOFREE(a) free (a)
#else
#define DOFREE(a) Free (a)
#endif

/*
 * For debugging purposes, free() is not called directly but through the wrapper
 * function Free(). This allows us to set a break point on Free(). Together with
 * conditions on breakpoints, this feature allows to detect when a certain address
 * is freed.
 */

#ifndef NOFREE

#ifdef SHOW_MALLOC
#define FREE(address)                                                                    \
    if ((address) != NULL) {                                                             \
        DBUG_PRINT ("FREEMEM", ("Free memory at adress: " P_FORMAT, (address)));         \
        address = (void *)((char *)address - malloc_align_step);                         \
        current_allocated_mem -= *(int *)(address);                                      \
                                                                                         \
        DOFREE (address);                                                                \
        address = NULL;                                                                  \
    }
#else /* not SHOW_MALLOC */
#define FREE(address)                                                                    \
    if ((address) != NULL) {                                                             \
        DBUG_PRINT ("FREEMEM", ("Free memory at adress: " P_FORMAT, (address)));         \
        DOFREE ((address));                                                              \
        address = NULL;                                                                  \
    }
#endif

#else /* NOFREE */

#define FREE(address)

#endif /* NOFREE */

/* struct INDEX_INFO (Withloop Folding) */
#define FREE_INDEX_INFO(tmp)                                                             \
    {                                                                                    \
        FREE (tmp->permutation);                                                         \
        FREE (tmp->last);                                                                \
        FREE (tmp->const_arg);                                                           \
        FREE (tmp);                                                                      \
        tmp = NULL;                                                                      \
    }

extern void Free (void *addr);

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
extern nodelist *FreeNodelistNode (nodelist *nl);
extern access_t *FreeOneAccess (access_t *fr);
extern access_t *FreeAllAccess (access_t *fr);

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
extern node *FreeIcm (node *arg_node, node *arg_info);
extern node *FreePragma (node *arg_node, node *arg_info);
extern node *FreeInfo (node *arg_node, node *arg_info);
extern node *FreeSpmd (node *arg_node, node *arg_info);
extern node *FreeSync (node *arg_node, node *arg_info);
extern node *FreeMT (node *arg_node, node *arg_info);
extern node *FreeST (node *arg_node, node *arg_info);

/* with-loop */
extern node *FreeNWith (node *arg_node, node *arg_info);
extern node *FreeNPart (node *arg_node, node *arg_info);
extern node *FreeNWithID (node *arg_node, node *arg_info);
extern node *FreeNGenerator (node *arg_node, node *arg_info);
extern node *FreeNWithOp (node *arg_node, node *arg_info);
extern node *FreeNCode (node *arg_node, node *arg_info);

extern node *FreeNwith2 (node *arg_node, node *arg_info);
extern node *FreeWLseg (node *arg_node, node *arg_info);
extern node *FreeWLblock (node *arg_node, node *arg_info);
extern node *FreeWLublock (node *arg_node, node *arg_info);
extern node *FreeWLstride (node *arg_node, node *arg_info);
extern node *FreeWLgrid (node *arg_node, node *arg_info);
extern node *FreeWLsegVar (node *arg_node, node *arg_info);
extern node *FreeWLstriVar (node *arg_node, node *arg_info);
extern node *FreeWLgridVar (node *arg_node, node *arg_info);

extern node *FreeCWrapper (node *arg_node, node *arg_info);
extern node *FreeModspec (node *arg_node, node *arg_info);

/* special functions */
extern void FreePrf2 (node *arg_node, int arg_no); /* CF */

#endif /* _sac_free_h */
