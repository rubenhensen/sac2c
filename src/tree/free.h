/*
 *
 * $Log$
 * Revision 3.7  2001/02/12 10:53:00  nmw
 * N_ssacnt and N_cseinfo added
 *
 * Revision 3.6  2001/02/02 09:22:42  dkr
 * no changes done
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
 * Revision 1.1  1994/12/27  09:28:19  sbs
 * Initial revision
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

/* special functions */
extern void FreePrf2 (node *arg_node, int arg_no); /* CF */

#endif /* _sac_free_h */
