/*
 *
 * $Log$
 * Revision 3.12  2001/04/26 01:39:25  dkr
 * FreeZombie(), RemoveAllZombies() added
 *
 * Revision 3.11  2001/04/24 09:15:55  dkr
 * P_FORMAT replaced by F_PTR
 *
 * Revision 3.9  2001/02/15 16:58:05  nmw
 * FreeSSAstack added
 *
 * Revision 3.8  2001/02/12 17:03:26  nmw
 * N_avis node added
 *
 * Revision 3.7  2001/02/12 10:53:00  nmw
 * N_ssacnt and N_cseinfo added
 *
 * Revision 3.5  2001/01/09 17:26:26  dkr
 * N_WLstriVar renamed into N_WLstrideVar
 *
 * Revision 3.4  2000/12/15 17:11:40  sbs
 * changed malloc.h into stdlib.h because some systems do not provide
 * free in the former (e.g. the alpha)!.
 *
 * Revision 3.3  2000/12/15 17:08:11  sbs
 * include "malloc.h" changed into include <malloc.h> for
 * the alpha to find it!
 *
 * Revision 3.2  2000/12/12 12:13:59  dkr
 * nodes N_pre, N_post, N_inc, N_dec added
 *
 * Revision 3.1  2000/11/20 18:03:24  sacbase
 * new release made
 *
 * ... [eliminated]
 *
 */

#ifndef _sac_free_h
#define _sac_free_h

#include <stdlib.h>

#include "dbug.h"
#include "my_debug.h"
#include "globals.h"
#include "internal_lib.h"

#ifdef DBUG_OFF
#define DOFREE(a) free (a)
#else
#define DOFREE(a) Free (a)
#endif

/*
 * For debugging purposes, free() is not called directly but through the
 * wrapper function Free(). This allows us to set a break point on Free().
 * Together with conditions on breakpoints, this feature allows to detect
 * when a certain address is freed.
 */

#ifndef NOFREE

#ifdef SHOW_MALLOC
#define FREE(address)                                                                    \
    if ((address) != NULL) {                                                             \
        DBUG_PRINT ("FREEMEM", ("Free memory at adress: " F_PTR, (address)));            \
        address = (void *)((char *)address - malloc_align_step);                         \
        current_allocated_mem -= *(int *)(address);                                      \
                                                                                         \
        DOFREE ((address));                                                              \
        address = NULL;                                                                  \
    }
#else /* not SHOW_MALLOC */
#define FREE(address)                                                                    \
    if ((address) != NULL) {                                                             \
        DBUG_PRINT ("FREEMEM", ("Free memory at adress: " F_PTR, (address)));            \
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

/*
 * user functions for nodes
 */

extern void Free (void *addr);

extern node *FreeNode (node *arg_node);
extern node *FreeTree (node *arg_node);

extern node *FreeZombie (node *fundef);
extern node *RemoveAllZombies (node *arg_node);

/* for compatibility only */
extern void FreePrf2 (node *arg_node, int arg_no); /* CF */

/*
 * user functions for non-node data
 */

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

/*
 * traversal functions
 */

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
extern node *FreeWLsegVar (node *arg_node, node *arg_info);
extern node *FreeWLblock (node *arg_node, node *arg_info);
extern node *FreeWLublock (node *arg_node, node *arg_info);
extern node *FreeWLstride (node *arg_node, node *arg_info);
extern node *FreeWLstrideVar (node *arg_node, node *arg_info);
extern node *FreeWLgrid (node *arg_node, node *arg_info);
extern node *FreeWLgridVar (node *arg_node, node *arg_info);

extern node *FreeCWrapper (node *arg_node, node *arg_info);
extern node *FreeModspec (node *arg_node, node *arg_info);
extern node *FreeCSEinfo (node *arg_node, node *arg_info);
extern node *FreeSSAcnt (node *arg_node, node *arg_info);
extern node *FreeAvis (node *arg_node, node *arg_info);
extern node *FreeSSAstack (node *arg_node, node *arg_info);

#endif /* _sac_free_h */
