/*
 *
 * $Log$
 * Revision 3.17  2002/08/09 16:36:21  sbs
 * basic support for N_mop written.
 *
 * Revision 3.16  2002/06/25 13:55:51  sbs
 * FreeDot added.
 *
 * Revision 3.15  2002/03/01 02:36:53  dkr
 * FreeArgtab() added
 *
 * Revision 3.14  2001/05/18 10:38:58  cg
 * Old memory management macros MALLOC and FREE removed.
 *
 * Revision 3.13  2001/05/17 13:29:29  cg
 * Eliminated some obsolete function prototypes.
 * Removed some ugly de-allocation macros.
 * Redefined macro FREE(n) for temporary compatibility reasons.
 *
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

/*
 * Top-level free functions.
 *
 * These are the only functions which should be called from outside
 * the free module for freeing syntax (sub) trees.
 */

extern node *FreeNode (node *arg_node);
extern node *FreeTree (node *arg_node);

extern node *FreeZombie (node *fundef);
extern node *RemoveAllZombies (node *arg_node);

/*
 * user functions for non-node data
 */

extern index_info *FreeIndexInfo (index_info *fr);
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
extern argtab_t *FreeArgtab (argtab_t *argtab);

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
extern node *FreeMop (node *arg_node, node *arg_info);
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
extern node *FreeDot (node *arg_node, node *arg_info);
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
