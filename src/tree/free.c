/*
 *
 * $Log$
 * Revision 3.24  2001/05/07 14:21:26  nmw
 * FUNDEF_INT_ASSIGN is only set to NULL if the assignment is freed
 *
 * Revision 3.23  2001/04/27 17:33:58  nmw
 * refcounting for special fundef (non recursive) AP icms added
 *
 * Revision 3.22  2001/04/26 11:54:54  nmw
 * FUNDEF_USED refcounting for ICMs, too
 *
 * Revision 3.21  2001/04/26 01:48:03  dkr
 * - reference counting for functions (FUNDEF_USED) works correctly now
 * - FreeFundef never removes the fundef node but create a zombie now
 * - FreeZombie(), RemoveAllZombies() added
 *
 * Revision 3.20  2001/04/24 14:12:18  dkr
 * fixed a bug with FreeNode: inner NEXT-sons are freed now :-/
 *
 * Revision 3.19  2001/04/24 09:36:40  dkr
 * CHECK_NULL renamed into STR_OR_EMPTY
 *
 * Revision 3.18  2001/04/03 14:25:11  nmw
 * unconditional free in FreeAp when removing syntax_tree
 *
 * Revision 3.16  2001/04/02 11:16:15  nmw
 * FreeAp decrements the used counter for special fundefs and
 * removes the corresponding assignment from list
 *
 * Revision 3.15  2001/03/29 14:24:19  dkr
 * NWITH2_SCHEDULING removed
 *
 * Revision 3.12  2001/03/22 20:02:31  dkr
 * include of tree.h eliminated
 *
 * Revision 3.11  2001/03/19 16:44:39  dkr
 * WLSEG_HOMSV removed (WLSEG_SV used instead)
 *
 * Revision 3.10  2001/03/07 15:56:18  nmw
 * wrong traversal in FreeCSEinfo fixed
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
 * Revision 3.5  2001/01/29 18:32:28  dkr
 * some superfluous attributes of N_WLsegVar removed
 *
 * Revision 3.4  2001/01/09 17:26:18  dkr
 * N_WLstriVar renamed into N_WLstrideVar
 *
 * Revision 3.3  2000/12/12 12:05:13  dkr
 * nodes N_pre, N_post, N_inc, N_dec removed
 *
 * Revision 3.2  2000/12/04 10:50:19  dkr
 * FreeShpseg: DBUG_ASSERT added
 *
 * Revision 3.1  2000/11/20 18:03:24  sacbase
 * new release made
 *
 * Revision 1.13  2000/11/02 16:03:10  dkr
 * FreeIcm: ICM args are no longer shared in the AST
 * -> free them :-)
 *
 * Revision 1.12  2000/10/27 14:49:53  dkr
 * cpp-flag FREE_MODNAMES added
 *
 * Revision 1.10  2000/10/26 14:28:57  dkr
 * FreeNCodeWLAA inlined
 * FreeShpseg modified: SHPSEG_NEXT is removed now, too.
 *
 * Revision 1.9  2000/07/31 10:45:52  cg
 * Eventually, the son ICM_NEXT is removed from the N_icm node.
 * The creation function MakeIcm is adjusted accordingly.
 *
 * Revision 1.8  2000/07/21 08:19:44  nmw
 * FreeModspec added
 *
 * Revision 1.7  2000/06/23 15:32:30  nmw
 * FreeCWrapper added
 *
 * Revision 1.6  2000/06/13 12:24:55  dkr
 * functions for old with-loop removed
 *
 * Revision 1.5  2000/03/31 14:11:20  dkr
 * bug in FreeOneAccess identified
 *
 * Revision 1.4  2000/03/29 16:09:54  jhs
 * FreeST and FreeMT added.
 *
 * Revision 1.3  2000/03/15 12:59:37  dkr
 * WLSEG_HOMSV added
 *
 * Revision 1.2  2000/01/26 17:27:42  dkr
 * type of traverse-function-table changed.
 *
 * [...]
 *
 */

/*
 * For the time being modulnames are shared in the AST
 *  -> no free!
 */
#define FREE_MODNAMES 0

#define NAMES_IN_TYPES 1

#include <stdlib.h>
#include <stdio.h>

#include "dbug.h"
#include "my_debug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

#include "traverse.h"
#include "DataFlowMask.h"
#include "scheduling.h"
#include "constants.h"

#include "free.h"

/*
 *  Important Remarks:
 *
 *  All removals of parts of or the entire syntax tree should be done
 *  by using the functions in this file.
 *
 *  Module names are never be freed, because it is difficult if not
 *  impossible to decide whether it is a private heap location or
 *  a heap location shared with other nodes.
 *
 */

/*--------------------------------------------------------------------------*/
/*  Basic Free-Macros    (internal use only)                                */
/*--------------------------------------------------------------------------*/

#define FREEMASK(mac)                                                                    \
    {                                                                                    \
        int _i;                                                                          \
                                                                                         \
        for (_i = 0; _i < MAX_MASK; _i++) {                                              \
            FREE (mac (arg_node, _i));                                                   \
        }                                                                                \
    }

#define FREETRAV(node)                                                                   \
    {                                                                                    \
        if ((node) != NULL) {                                                            \
            /* dkr: remove whole subtree !!! */                                          \
            (node) = Trav (node, arg_info);                                              \
        }                                                                                \
    }

#define FREECONT(node)                                                                   \
    ((INFO_FREE_FLAG (arg_info) != arg_node) && ((node) != NULL))                        \
      ? Trav (node, arg_info)                                                            \
      : (node)

/*--------------------------------------------------------------------------*/
/*  Free-function wrapper for debugging purposes                            */
/*--------------------------------------------------------------------------*/

void
Free (void *addr)
{
    DBUG_ENTER ("Free");

    free (addr);

    DBUG_VOID_RETURN;
}

/*--------------------------------------------------------------------------*/
/*  Free-functions for non-node structures                                  */
/*--------------------------------------------------------------------------*/

/*
 *  Basically, there are two different free-functions for each non-node
 *  structure:
 *             Free<struct> and FreeAll<struct>
 *
 *  Both get a pointer to the respective structure as argument.
 *
 *  Free<struct> removes only the structure referenced by the argument
 *  and returns a pointer to the next structure in the chain of structures.
 *  These functions are useful to eliminate a single structure from a list.
 *
 *  FreeAll<struct> removes the whole sub tree which is referenced.
 *  All elements of list are freed in this case.
 */

/*--------------------------------------------------------------------------*/

shpseg *
FreeShpseg (shpseg *fr)
{
    DBUG_ENTER ("FreeShpseg");

    DBUG_PRINT ("FREE", ("Removing shpseg"));

    DBUG_ASSERT ((fr != NULL), "cannot free a NULL shpseg!");

    if (SHPSEG_NEXT (fr) != NULL) {
        SHPSEG_NEXT (fr) = FreeShpseg (SHPSEG_NEXT (fr));
    }

    FREE (fr);
    fr = NULL;

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

types *
FreeOneTypes (types *fr)
{
    types *tmp;

    DBUG_ENTER ("FreeOneTypes");

    if (fr != NULL) {
        DBUG_PRINT ("FREE", ("Removing types: %s", mdb_type[TYPES_BASETYPE (fr)]));
        tmp = fr;
        fr = TYPES_NEXT (fr);
        if (TYPES_DIM (tmp) > 0) {
            TYPES_SHPSEG (tmp) = FreeShpseg (TYPES_SHPSEG (tmp));
        }
        FREE (TYPES_NAME (tmp));
        /*
         *  fr->id is not freed by purpose !!
         */
        FREE (tmp);
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

types *
FreeAllTypes (types *fr)
{
    DBUG_ENTER ("FreeAllTypes");

    while (fr != NULL) {
        fr = FreeOneTypes (fr);
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

ids *
FreeOneIds (ids *fr)
{
    ids *tmp;

    DBUG_ENTER ("FreeOneIds");

    if (fr != NULL) {
        DBUG_PRINT ("FREE", ("Removing ids: %s", IDS_NAME (fr)));

        tmp = fr;
        fr = IDS_NEXT (fr);

        FREE (IDS_NAME (tmp));

        FREE (tmp);
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

ids *
FreeAllIds (ids *fr)
{
    DBUG_ENTER ("FreeAllIds");

    while (fr != NULL) {
        fr = FreeOneIds (fr);
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

nums *
FreeOneNums (nums *fr)
{
    nums *tmp;

    DBUG_ENTER ("FreeNums");

    if (fr != NULL) {
        DBUG_PRINT ("FREE", ("Removing Nums: %d", NUMS_NUM (fr)));

        tmp = fr;
        fr = NUMS_NEXT (fr);
        FREE (tmp);
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

nums *
FreeAllNums (nums *fr)
{
    DBUG_ENTER ("FreeAllNums");

    while (fr != NULL) {
        fr = FreeOneNums (fr);
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

deps *
FreeOneDeps (deps *fr)
{
    deps *tmp;

    DBUG_ENTER ("FreeDeps");

    if (fr != NULL) {
        DBUG_PRINT ("FREE", ("Removing Deps: %s", DEPS_NAME (fr)));

        tmp = fr;
        fr = DEPS_NEXT (fr);

        FREE (DEPS_NAME (tmp));
        FREE (DEPS_DECNAME (tmp));
        FREE (DEPS_LIBNAME (tmp));
        DEPS_SUB (tmp) = FreeAllDeps (DEPS_SUB (tmp));

        FREE (tmp);
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

deps *
FreeAllDeps (deps *fr)
{
    DBUG_ENTER ("FreeAllDeps");

    while (fr != NULL) {
        fr = FreeOneDeps (fr);
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

strings *
FreeOneStrings (strings *fr)
{
    strings *tmp;

    DBUG_ENTER ("FreeStrings");

    if (fr != NULL) {
        DBUG_PRINT ("FREE", ("Removing strings: %s", STRINGS_STRING (fr)));

        tmp = fr;
        fr = STRINGS_NEXT (fr);

        FREE (STRINGS_STRING (tmp));

        FREE (tmp);
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

strings *
FreeAllStrings (strings *fr)
{
    DBUG_ENTER ("FreeAllStrings");

    while (fr != NULL) {
        fr = FreeOneStrings (fr);
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

/*
 *  FreeNodelist always frees entire list.
 */

nodelist *
FreeNodelist (nodelist *list)
{
    nodelist *tmp = NULL;

    DBUG_ENTER ("FreeNodelist");

    while (list != NULL) {
        tmp = list;
        list = NODELIST_NEXT (list);
        FREE (tmp);
    }

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

/*
 *  FreeNodelistNode free a nodelist item and return the next item
 */

nodelist *
FreeNodelistNode (nodelist *nl)
{
    nodelist *tmp;

    DBUG_ENTER ("FreeNodelistNode");

    tmp = NODELIST_NEXT (nl);
    FREE (nl);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

access_t *
FreeOneAccess (access_t *fr)
{
#if 0
  access_t *tmp;
#endif

    DBUG_ENTER ("FreeOneAccess");

    if (fr != NULL) {
#if 0
    DBUG_ASSERT( ((NODE_TYPE( ACCESS_IV( fr)) == N_vardec) ||
                  (NODE_TYPE( ACCESS_IV( fr)) == N_arg)),
                 "ACCESS_IV is neither a N_vardec- nor a N_arg-node!");

    DBUG_ASSERT( ((NODE_TYPE( ACCESS_ARRAY( fr)) == N_vardec) ||
                  (NODE_TYPE( ACCESS_ARRAY( fr)) == N_arg)),
                 "ACCESS_ARRAY is neither a N_vardec- nor a N_arg-node!");

    DBUG_PRINT( "FREE", ("Removing Access: psi(%s, %s)", 
                         VARDEC_OR_ARG_NAME( ACCESS_IV( fr)),
                         VARDEC_OR_ARG_NAME( ACCESS_ARRAY( fr))));

    tmp = fr;
    fr = ACCESS_NEXT( fr);
    
    if (ACCESS_OFFSET( tmp) != NULL) {
      ACCESS_OFFSET( tmp) = FreeShpseg( ACCESS_OFFSET( tmp));
    }
    
    FREE( tmp);
#else
        fr = NULL;
#endif
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

access_t *
FreeAllAccess (access_t *fr)
{
    DBUG_ENTER ("FreeAllAccess");

    while (fr != NULL) {
        fr = FreeOneAccess (fr);
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/
/*  General free-functions for node structures                              */
/*--------------------------------------------------------------------------*/

/******************************************************************************
 *
 * Function:
 *   node *FreeNode( node *free_node)
 *
 * Description:
 *   - if 'free_node' is not a N_fundef node:
 *        Removes the given node and returns a pointer to the NEXT node if it
 *        exists, NULL otherwise.
 *   - if 'free_node' is a N_fundef node:
 *        Transforms the given fundef into a zombie and returns it.
 *
 ******************************************************************************/

node *
FreeNode (node *free_node)
{
    funtab *store_tab;
    node *arg_info;

    DBUG_ENTER ("FreeNode");

    store_tab = act_tab;
    act_tab = free_tab;

    arg_info = MakeInfo ();
    INFO_FREE_FLAG (arg_info) = free_node;
    INFO_FREE_ASSIGN (arg_info) = NULL;

    free_node = Trav (free_node, arg_info);

    /* no not use FreeNode to avoid recursion */
    FREE (arg_info);

    act_tab = store_tab;

    DBUG_RETURN (free_node);
}

/******************************************************************************
 *
 * Function:
 *   node *FreeTree( node *free_node)
 *
 * Description:
 *   - if 'free_node' is not a N_fundef node:
 *        Removes the whole sub tree behind the given pointer.
 *   - if 'free_node' is a N_fundef node:
 *        Transforms the whole fundef chain into zombies and returns it.
 *
 ******************************************************************************/

node *
FreeTree (node *free_node)
{
    funtab *store_tab;
    node *arg_info;

    DBUG_ENTER ("FreeTree");

    store_tab = act_tab;
    act_tab = free_tab;

    arg_info = MakeInfo ();
    INFO_FREE_FLAG (arg_info) = NULL;
    INFO_FREE_ASSIGN (arg_info) = NULL;

    Trav (free_node, arg_info);

    /* no not use FreeNode to avoid recursion */
    FREE (arg_info);

    act_tab = store_tab;

    DBUG_RETURN ((node *)NULL);
}

/******************************************************************************
 *
 * Function:
 *   node *FreeZombie( node *fundef)
 *
 * Description:
 *   - if head of 'fundef' is a zombie:
 *       Removes the head and returns the tail.
 *   - if head if 'fundef' is not a zombie:
 *       Returns 'fundef' in unmodified form.
 *
 ******************************************************************************/

node *
FreeZombie (node *fundef)
{
    node *tmp;

    DBUG_ENTER ("FreeZombie");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "FreeZombie() is suitable for N_fundef nodes only!");

    if (FUNDEF_STATUS (fundef) == ST_zombiefun) {
        if (FUNDEF_USED (fundef) == USED_INACTIVE) {
            DBUG_PRINT ("FREE", ("Removing N_fundef zombie"
                                 " (FUNDEF_USED inactive) ..."));
        } else {
            DBUG_PRINT ("FREE", ("Removing N_fundef zombie"
                                 " (FUNDEF_USED == %i) ...",
                                 FUNDEF_USED (fundef)));

            DBUG_ASSERT ((FUNDEF_USED (fundef) == 0), "N_fundef zombie is still in use!");
        }

        /*
         * remove all the zombie data
         */
#if NAMES_IN_TYPES
        if (FUNDEF_TYPES (fundef) != NULL) {
#endif
            FREE (FUNDEF_NAME (fundef));
#if FREE_MODNAMES
            FREE (FUNDEF_MOD (fundef));
            FREE (FUNDEF_LINKMOD (fundef));
#endif

            FUNDEF_TYPES (fundef) = FreeOneTypes (FUNDEF_TYPES (fundef));
#if NAMES_IN_TYPES
        }
#endif

        tmp = fundef;
        fundef = FUNDEF_NEXT (fundef);
        FREE (tmp);
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * Function:
 *   node *RemoveAllZombies( node *arg_node)
 *
 * Description:
 *   Removes all zombies found the given N_modul node or N_fundef chain
 *
 ******************************************************************************/

node *
RemoveAllZombies (node *arg_node)
{
    node *ret_node;

    DBUG_ENTER ("RemoveAllZombies");

    DBUG_ASSERT ((arg_node != NULL), "RemoveAllZombies called with argument NULL");

    switch (NODE_TYPE (arg_node)) {
    case N_modul:
        if (MODUL_FUNS (arg_node) != NULL) {
            MODUL_FUNS (arg_node) = RemoveAllZombies (MODUL_FUNS (arg_node));
        }
        ret_node = arg_node;
        break;

    case N_fundef:
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = RemoveAllZombies (FUNDEF_NEXT (arg_node));
        }
        ret_node = FreeZombie (arg_node);
        break;

    default:
        DBUG_ASSERT ((0), "RemoveAllZombies() is suitable for N_modul and"
                          " N_fundef nodes only!");
        ret_node = NULL;
        break;
    }

    DBUG_RETURN (ret_node);
}

/*--------------------------------------------------------------------------*/
/*  Specific free-functions for node structures                             */
/*--------------------------------------------------------------------------*/

/*
 *  There is one specific free function for each node type.
 *  They all fit the signature of the univeral traversal mechanism.
 *  The first argument points to the node to be freed, while the second
 *  argument is used as a flag whether removing the given node alone (==NULL)
 *  or removing the whole sub tree behind it (!=NULL).
 *
 *  Since the N_info node may look very different depending on its specific
 *  task, separate free functions are necessary for each kind of usage.
 *  For these, the universal traversal mechanism may not be used because
 *  it is unable to distinguish between different kinds of nodes which do
 *  have the same type.
 *
 *  Normally, it is easier to use the general free functions described above.
 *
 */

node *
FreeModul (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeModul");

    DBUG_PRINT ("FREE", ("Removing contents of N_modul node ..."));

    FREETRAV (MODUL_IMPORTS (arg_node));
    FREETRAV (MODUL_TYPES (arg_node));
    FREETRAV (MODUL_OBJS (arg_node));
    FREETRAV (MODUL_FUNS (arg_node));
    FREETRAV (MODUL_DECL (arg_node));
    FREETRAV (MODUL_FOLDFUNS (arg_node));
    FREETRAV (MODUL_STORE_IMPORTS (arg_node));

#if FREE_MODNAMES
    FREE (MODUL_NAME (arg_node));
#endif

    DBUG_PRINT ("FREE", ("Removing N_modul node ..."));

    arg_node = RemoveAllZombies (arg_node);

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeModdec (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeModdec");

    DBUG_PRINT ("FREE", ("Removing contents of N_moddec node ..."));

    FREETRAV (MODDEC_IMPORTS (arg_node));
    FREETRAV (MODDEC_OWN (arg_node));

    FREE (MODDEC_NAME (arg_node));
    MODDEC_LINKWITH (arg_node) = FreeAllDeps (MODDEC_LINKWITH (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_moddec node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeClassdec (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeClassdec");

    DBUG_PRINT ("FREE", ("Removing contents of N_classdec node ..."));

    FREETRAV (CLASSDEC_IMPORTS (arg_node));
    FREETRAV (CLASSDEC_OWN (arg_node));

    FREE (CLASSDEC_NAME (arg_node));
    CLASSDEC_LINKWITH (arg_node) = FreeAllDeps (CLASSDEC_LINKWITH (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_classdec node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeSib (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeSib");

    DBUG_PRINT ("FREE", ("Removing contents of N_sib node ..."));

    FREETRAV (SIB_TYPES (arg_node));
    FREETRAV (SIB_OBJS (arg_node));
    FREETRAV (SIB_FUNS (arg_node));

    FREE (SIB_NAME (arg_node));
    SIB_LINKWITH (arg_node) = FreeAllDeps (SIB_LINKWITH (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_sib node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeImplist (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeImplist");

    DBUG_PRINT ("FREE",
                ("Removing contents of N_implist node %s ...", IMPLIST_NAME (arg_node)));

    tmp = FREECONT (IMPLIST_NEXT (arg_node));

    FREE (IMPLIST_NAME (arg_node));
    IMPLIST_ITYPES (arg_node) = FreeAllIds (IMPLIST_ITYPES (arg_node));
    IMPLIST_ETYPES (arg_node) = FreeAllIds (IMPLIST_ETYPES (arg_node));
    IMPLIST_OBJS (arg_node) = FreeAllIds (IMPLIST_OBJS (arg_node));
    IMPLIST_FUNS (arg_node) = FreeAllIds (IMPLIST_FUNS (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_implist node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeExplist (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeExplist");

    DBUG_PRINT ("FREE", ("Removing contents of N_explist node ..."));

    FREETRAV (EXPLIST_ITYPES (arg_node))
    FREETRAV (EXPLIST_ETYPES (arg_node));
    FREETRAV (EXPLIST_OBJS (arg_node));
    FREETRAV (EXPLIST_FUNS (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_explist node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeTypedef (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeTypedef");

    DBUG_PRINT ("FREE",
                ("Removing contents of N_typedef node %s ...", ItemName (arg_node)));

    tmp = FREECONT (TYPEDEF_NEXT (arg_node));

    FREE (TYPEDEF_NAME (arg_node));
#if FREE_MODNAMES
    FREE (TYPEDEF_MOD (arg_node));
#endif
    TYPEDEF_TYPE (arg_node) = FreeAllTypes (TYPEDEF_TYPE (arg_node));
    FREE (TYPEDEF_COPYFUN (arg_node));
    FREE (TYPEDEF_FREEFUN (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_typedef node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeObjdef (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeObjdef");

    DBUG_PRINT ("FREE",
                ("Removing contents of N_objdef node %s ...", ItemName (arg_node)));

    tmp = FREECONT (OBJDEF_NEXT (arg_node));

    FREETRAV (OBJDEF_EXPR (arg_node));
    FREETRAV (OBJDEF_PRAGMA (arg_node));

    FREE (OBJDEF_NAME (arg_node));
#if FREE_MODNAMES
    FREE (OBJDEF_MOD (arg_node));
    FREE (OBJDEF_LINKMOD (arg_node));
#endif
    FREE (OBJDEF_VARNAME (arg_node));
    OBJDEF_TYPE (arg_node) = FreeOneTypes (OBJDEF_TYPE (arg_node));

    /*
     * The nodes contained in OBJDEF_NEEDOBJS are all(?) shared.
     * Therefore, please do not free this list!
     *
     * OBJDEF_NEEDOBJS( arg_node) = FreeNodelist( OBJDEF_NEEDOBJS( arg_node));
     */

    DBUG_PRINT ("FREE", ("Removing N_objdef node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeFundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("FreeFundef");

    DBUG_PRINT ("FREE",
                ("Removing contents of N_fundef node %s ...", ItemName (arg_node)));

    FUNDEF_NEXT (arg_node) = FREECONT (FUNDEF_NEXT (arg_node));

#if NAMES_IN_TYPES
    /*
     * CAUTION: FUNDEF_STATUS is part of FUNDEF_TYPES  8-((
     */
    DBUG_ASSERT ((FUNDEF_TYPES (arg_node) != NULL), "FUNDEF_TYPES not found!");
#endif

    if (FUNDEF_STATUS (arg_node) == ST_zombiefun) {
        DBUG_PRINT ("FREE", ("N_fundef node is already a zombie"
                             " (FUNDEF_USED == %i) ...",
                             FUNDEF_USED (arg_node)));
    } else {

        if (FUNDEF_ICM (arg_node) != NULL) {
            if (NODE_TYPE (FUNDEF_ICM (arg_node)) == N_icm) {
                /*
                 *  FUNDEF_ICM may not be freed without precondition, because it's
                 *  stored on the same real son node as FUNDEF_RETURN.
                 */
                FREETRAV (FUNDEF_ICM (arg_node));
            }
        }

        FREETRAV (FUNDEF_BODY (arg_node));
        FREETRAV (FUNDEF_ARGS (arg_node));

        if (FUNDEF_PRAGMA (arg_node) != NULL) {
            FREETRAV (FUNDEF_PRAGMA (arg_node));
        }

        FREEMASK (FUNDEF_MASK);

        if (FUNDEF_DFM_BASE (arg_node) != NULL) {
            FUNDEF_DFM_BASE (arg_node) = DFMRemoveMaskBase (FUNDEF_DFM_BASE (arg_node));
        }

        if (FUNDEF_STATUS (arg_node) != ST_spmdfun) {
            FUNDEF_NEEDOBJS (arg_node) = FreeNodelist (FUNDEF_NEEDOBJS (arg_node));
        }

        /*
         * If  ((FUNDEF_USED > 0) || (FUNDEF_USED == USED_INACTIVE))  is hold the
         * fundef node may still be referenced somewhere in the AST by a N_ap node.
         * When removing this N_ap node via FreeAp() a legal AP_FUNDEF pointer is
         * needed for checking the reference counter of the function.
         * Therefore, the fundef must be left in the AST as zombie!
         *
         * For consistency reasons the fundef is *never* removed but always left in
         * the AST as zombie, even if  (FUNDEF_USED == 0)  is hold.
         *
         * For removing zombies one of the (top-level) functions
         *   FreeZombie():
         *     removes the head of a given N_fundef chain if it is a zombie
         *   RemoveAllZombies():
         *     removes all zombies found the given N_modul node or N_fundef chain
         * should be used.
         *
         * The following data is left for the zombie:
         *   FUNDEF_TYPES (needed for FUNDEF_NAME !!!!)
         *   FUNDEF_NAME, FUNDEF_MOD, FUNDEF_LINKMOD
         */
        if (FUNDEF_USED (arg_node) == USED_INACTIVE) {
            DBUG_PRINT ("FREE", ("Leaving N_fundef node as a zombie in the AST"
                                 " (FUNDEF_USED inactive) ..."));
        } else {
            DBUG_PRINT ("FREE", ("Leaving N_fundef node as a zombie in the AST"
                                 " (FUNDEF_USED == %i) ...",
                                 FUNDEF_USED (arg_node)));
        }

        FUNDEF_STATUS (arg_node) = ST_zombiefun;
    }

    DBUG_RETURN (arg_node);
}

/*--------------------------------------------------------------------------*/

node *
FreeArg (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeArg");

    DBUG_PRINT ("FREE", ("Removing contents of N_arg node %s ...",
                         STR_OR_EMPTY (ARG_NAME (arg_node))));
    /* ARG_NAME(arg_node) may be NULL in external decls! */

    tmp = FREECONT (ARG_NEXT (arg_node));

    FREE (ARG_NAME (arg_node));
    FREETRAV (ARG_AVIS (arg_node));
    ARG_TYPE (arg_node) = FreeOneTypes (ARG_TYPE (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_arg node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeBlock (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeBlock");

    DBUG_PRINT ("FREE", ("Removing contents of N_block node ..."));

    FREETRAV (BLOCK_INSTR (arg_node));
    FREETRAV (BLOCK_VARDEC (arg_node));
    BLOCK_NEEDFUNS (arg_node) = FreeNodelist (BLOCK_NEEDFUNS (arg_node));
    BLOCK_NEEDTYPES (arg_node) = FreeNodelist (BLOCK_NEEDTYPES (arg_node));
    FREEMASK (BLOCK_MASK);
    FREE (BLOCK_CACHESIM (arg_node));
    FREETRAV (BLOCK_SSACOUNTER (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_block node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeVardec (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeVardec");

    DBUG_PRINT ("FREE",
                ("Removing contents of N_vardec node %s ...", VARDEC_NAME (arg_node)));

    tmp = FREECONT (VARDEC_NEXT (arg_node));

    FREE (VARDEC_NAME (arg_node));
    FREETRAV (VARDEC_AVIS (arg_node));
    VARDEC_TYPE (arg_node) = FreeOneTypes (VARDEC_TYPE (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_vardec node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeAssign (node *arg_node, node *arg_info)
{
    node *tmp = NULL;
    index_info *index;

    DBUG_ENTER ("FreeAssign");

    DBUG_PRINT ("FREE", ("Removing contents of N_assign node ..."));

    tmp = FREECONT (ASSIGN_NEXT (arg_node));

    INFO_FREE_ASSIGN (arg_info) = arg_node;
    FREETRAV (ASSIGN_INSTR (arg_node));
    INFO_FREE_ASSIGN (arg_info) = NULL;

    FREEMASK (ASSIGN_MASK);
    index = (index_info *)ASSIGN_INDEX (arg_node);
    if (index) {
        FREE_INDEX_INFO (index);
    }

    DBUG_PRINT ("FREE", ("Removing N_assign node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeLet (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeLet");

    DBUG_PRINT ("FREE", ("Removing contents of N_let node ..."));

    FREETRAV (LET_EXPR (arg_node));
    LET_IDS (arg_node) = FreeAllIds (LET_IDS (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_let node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeCast (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeCast");

    DBUG_PRINT ("FREE", ("Removing contents of N_cast node ..."));

    FREETRAV (CAST_EXPR (arg_node));
    CAST_TYPE (arg_node) = FreeOneTypes (CAST_TYPE (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_cast node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeReturn (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeReturn");

    DBUG_PRINT ("FREE", ("Removing contents of N_return node ..."));

    FREETRAV (RETURN_EXPRS (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_return node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeCond (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeCond");

    DBUG_PRINT ("FREE", ("Removing contents of N_cond node ..."));

    FREETRAV (COND_COND (arg_node));
    FREETRAV (COND_THEN (arg_node));
    FREETRAV (COND_ELSE (arg_node));

    COND_THENVARS (arg_node) = FreeAllIds (COND_THENVARS (arg_node));
    COND_ELSEVARS (arg_node) = FreeAllIds (COND_ELSEVARS (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_cond node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeDo (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeDo");

    DBUG_PRINT ("FREE", ("Removing contents of N_do node ..."));

    FREETRAV (DO_BODY (arg_node));
    FREETRAV (DO_COND (arg_node));

    DO_USEVARS (arg_node) = FreeAllIds (DO_USEVARS (arg_node));
    DO_DEFVARS (arg_node) = FreeAllIds (DO_DEFVARS (arg_node));

    FREEMASK (DO_MASK);

    DBUG_PRINT ("FREE", ("Removing N_do node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeWhile (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeWhile");

    DBUG_PRINT ("FREE", ("Removing contents of N_while node ..."));

    FREETRAV (WHILE_BODY (arg_node));
    FREETRAV (WHILE_COND (arg_node));

    WHILE_USEVARS (arg_node) = FreeAllIds (WHILE_USEVARS (arg_node));
    WHILE_DEFVARS (arg_node) = FreeAllIds (WHILE_DEFVARS (arg_node));

    FREEMASK (WHILE_MASK);

    DBUG_PRINT ("FREE", ("Removing N_while node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeAp (node *arg_node, node *arg_info)
{
    node *fundef;
    node *tmp = NULL;

    DBUG_ENTER ("FreeAp");

    DBUG_PRINT ("FREE", ("Removing contents of N_ap node %s ...", AP_NAME (arg_node)));

    FREETRAV (AP_ARGS (arg_node));
    FREE (AP_NAME (arg_node));
#if FREE_MODNAMES
    FREE (AP_MOD (arg_node));
#endif

    fundef = AP_FUNDEF (arg_node);

    /* decrement used counter */
    if (fundef != NULL) {
        DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                     "illegal value in AP_FUNDEF found!");

        DBUG_ASSERT (((!FUNDEF_IS_LACFUN (fundef))
                      || (FUNDEF_USED (fundef) != USED_INACTIVE)),
                     "FUNDEF_USED must be active for LaC functions!");

        /*
         * A recursive call in the body of a special function is *not*
         * counted in FUNDEF_USED !!!
         */
        if (FUNDEF_IS_LOOPFUN (fundef) &&
            /* caution: INT_ASSIGN may be NULL already!!! */
            (FUNDEF_INT_ASSIGN (fundef) != NULL)
            && (arg_node == ASSIGN_RHS (FUNDEF_INT_ASSIGN (fundef)))) {

            /*
             * only clear attribute, if this assignment is removed completly
             * and not substuituted by an icm node. here it is checked if a
             * hole assignment is freed.
             */
            if (INFO_FREE_ASSIGN (arg_info) != NULL) {
                FUNDEF_INT_ASSIGN (fundef) = NULL;
            }
            /* noop */
        } else if (FUNDEF_USED (fundef) != USED_INACTIVE) {
            (FUNDEF_USED (fundef))--;

            DBUG_ASSERT ((FUNDEF_USED (fundef) >= 0), "FUNDEF_USED dropped below 0");

            DBUG_PRINT ("FREE", ("decrementing used counter of %s to %d",
                                 FUNDEF_NAME (fundef), FUNDEF_USED (fundef)));

            if ((FUNDEF_IS_LACFUN (fundef)) && (compiler_phase < PH_compile)) {
                /* remove assignment from external assignment list */
                DBUG_ASSERT ((INFO_FREE_ASSIGN (arg_info) != NULL),
                             "INFO_FREE_ASSIGN is needed when removing an"
                             " application of a special loop-function!");

                DBUG_ASSERT ((NodeListFind (FUNDEF_EXT_ASSIGNS (fundef),
                                            INFO_FREE_ASSIGN (arg_info))
                              != NULL),
                             "Assignment not found in FUNDEF_EXT_ASSIGNS!");

                FUNDEF_EXT_ASSIGNS (fundef)
                  = NodeListDelete (FUNDEF_EXT_ASSIGNS (fundef),
                                    INFO_FREE_ASSIGN (arg_info), FALSE);
            }

            if (FUNDEF_USED (fundef) == 0) {
                /*
                 * referenced fundef no longer used
                 *  -> transform it into a zombie
                 */
                fundef = FreeNode (fundef);
            }
        }
    }

    DBUG_PRINT ("FREE", ("Removing N_ap node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeExprs (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeExprs");

    DBUG_PRINT ("FREE", ("Removing contents of N_exprs node ..."));

    tmp = FREECONT (EXPRS_NEXT (arg_node));

    FREETRAV (EXPRS_EXPR (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_exprs node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeArray (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeArray");

    DBUG_PRINT ("FREE", ("Removing contents of N_array node ..."));

    FREETRAV (ARRAY_AELEMS (arg_node));

    if (ARRAY_TYPE (arg_node) != NULL) {
        ARRAY_TYPE (arg_node) = FreeOneTypes (ARRAY_TYPE (arg_node));
    }

    if (ARRAY_ISCONST (arg_node) && ARRAY_VECLEN (arg_node) > 0)
        FREE (ARRAY_CONSTVEC (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_array node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeVinfo (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeVinfo");

    DBUG_PRINT ("FREE", ("Removing contents of N_vinfo node ..."));

    tmp = FREECONT (VINFO_NEXT (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_vinfo node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeId (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeId");

    DBUG_PRINT ("FREE", ("Removing contents of N_id node %s ...", ID_NAME (arg_node)));

    FREE (ID_NAME (arg_node));
#if FREE_MODNAMES
    FREE (ID_MOD (arg_node));
#endif

    DBUG_PRINT ("FREE", ("Removing N_id node ..."));

    if (ID_ISCONST (arg_node) && (ID_VECLEN (arg_node) > 0))
        FREE (ID_CONSTVEC (arg_node));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeNum (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeNum");

    DBUG_PRINT ("FREE", ("Removing N_num node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeChar (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeChar");

    DBUG_PRINT ("FREE", ("Removing N_char node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeFloat (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeFloat");

    DBUG_PRINT ("FREE", ("Removing N_float node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeDouble (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeDouble");

    DBUG_PRINT ("FREE", ("Removing N_double node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeBool (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeBool");

    DBUG_PRINT ("FREE", ("Removing N_bool node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeStr (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeStr");

    DBUG_PRINT ("FREE",
                ("Removing contents of N_str node %s ...", STR_STRING (arg_node)));

    FREE (STR_STRING (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_str node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreePrf (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreePrf");

    DBUG_PRINT ("FREE", ("Removing contents of N_prf node ..."));

    FREETRAV (PRF_ARGS (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_prf node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeEmpty (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeEmpty");

    DBUG_PRINT ("FREE", ("Removing N_empty node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeIcm (node *arg_node, node *arg_info)
{
    node *tmp = NULL;
    node *fundef;

    DBUG_ENTER ("FreeIcm");

    DBUG_PRINT ("FREE", ("Removing contents of N_icm node %s ...", ICM_NAME (arg_node)));

    /*
     * ICM names are all static. Therefore, please do not free them!!
     *
     * FREE( ICM_NAME( arg_node));
     */

    FREETRAV (ICM_ARGS (arg_node));

    fundef = ICM_FUNDEF (arg_node);

    /* icm does external function application of loop special fundef */
    if ((fundef != NULL) && (FUNDEF_USED (fundef) != USED_INACTIVE)
        && (!((FUNDEF_IS_LOOPFUN (fundef))
              && (INFO_FREE_ASSIGN (arg_info) == FUNDEF_INT_ASSIGN (fundef))))) {
        (FUNDEF_USED (fundef))--;

        DBUG_ASSERT ((FUNDEF_USED (fundef) >= 0), "FUNDEF_USED dropped below 0");

        DBUG_PRINT ("FREE", ("decrementing used counter of %s to %d",
                             FUNDEF_NAME (fundef), FUNDEF_USED (fundef)));

        if (FUNDEF_USED (fundef) == 0) {
            /*
             * referenced fundef no longer used
             *  -> transform it into a zombie
             */
            fundef = FreeNode (fundef);
        }
    }

    DBUG_PRINT ("FREE", ("Removing N_icm node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreePragma (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreePragma");

    DBUG_PRINT ("FREE", ("Removing contents of N_pragma node ..."));

    FREE (PRAGMA_LINKSIGN (arg_node));
    FREE (PRAGMA_READONLY (arg_node));
    FREE (PRAGMA_REFCOUNTING (arg_node));
    PRAGMA_EFFECT (arg_node) = FreeAllIds (PRAGMA_EFFECT (arg_node));
    PRAGMA_TOUCH (arg_node) = FreeAllIds (PRAGMA_TOUCH (arg_node));
    FREE (PRAGMA_LINKNAME (arg_node));
    FREE (PRAGMA_COPYFUN (arg_node));
    FREE (PRAGMA_FREEFUN (arg_node));
    FREE (PRAGMA_INITFUN (arg_node));

    FREETRAV (PRAGMA_WLCOMP_APS (arg_node));

#if 0
  PRAGMA_NEEDTYPES(arg_node) = FreeAllIds(PRAGMA_NEEDTYPES(arg_node));
  FREETRAV(PRAGMA_NEEDFUNS(arg_node));
#endif

    DBUG_PRINT ("FREE", ("Removing N_pragma node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeSpmd (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeSpmd");

    DBUG_PRINT ("FREE", ("Removing contents of N_spmd node ..."));

    FREETRAV (SPMD_REGION (arg_node));

    if (SPMD_IN (arg_node) != NULL) {
        SPMD_IN (arg_node) = DFMRemoveMask (SPMD_IN (arg_node));
    }
    if (SPMD_INOUT (arg_node) != NULL) {
        SPMD_INOUT (arg_node) = DFMRemoveMask (SPMD_INOUT (arg_node));
    }
    if (SPMD_OUT (arg_node) != NULL) {
        SPMD_OUT (arg_node) = DFMRemoveMask (SPMD_OUT (arg_node));
    }
    if (SPMD_LOCAL (arg_node) != NULL) {
        SPMD_LOCAL (arg_node) = DFMRemoveMask (SPMD_LOCAL (arg_node));
    }
    if (SPMD_SHARED (arg_node) != NULL) {
        SPMD_SHARED (arg_node) = DFMRemoveMask (SPMD_SHARED (arg_node));
    }

    FREETRAV (SPMD_ICM_BEGIN (arg_node));
    FREETRAV (SPMD_ICM_PARALLEL (arg_node));
    FREETRAV (SPMD_ICM_ALTSEQ (arg_node));
    FREETRAV (SPMD_ICM_SEQUENTIAL (arg_node));
    FREETRAV (SPMD_ICM_END (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_spmd node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeSync (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeSync");

    DBUG_PRINT ("FREE", ("Removing contents of N_sync node ..."));

    FREETRAV (SYNC_REGION (arg_node));

    if (SYNC_IN (arg_node) != NULL) {
        SYNC_IN (arg_node) = DFMRemoveMask (SYNC_IN (arg_node));
    }
    if (SYNC_INOUT (arg_node) != NULL) {
        SYNC_INOUT (arg_node) = DFMRemoveMask (SYNC_INOUT (arg_node));
    }
    if (SYNC_OUT (arg_node) != NULL) {
        SYNC_OUT (arg_node) = DFMRemoveMask (SYNC_OUT (arg_node));
    }
    if (SYNC_LOCAL (arg_node) != NULL) {
        SYNC_LOCAL (arg_node) = DFMRemoveMask (SYNC_LOCAL (arg_node));
    }

    DBUG_PRINT ("FREE", ("Removing N_sync node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeMT (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("FreeMT");

    DBUG_PRINT ("FREE", ("Removing contents of N_mt node ..."));

    FREETRAV (MT_REGION (arg_node));

    if (MT_USEMASK (arg_node) != NULL) {
        MT_USEMASK (arg_node) = DFMRemoveMask (MT_USEMASK (arg_node));
    }
    if (MT_DEFMASK (arg_node) != NULL) {
        MT_DEFMASK (arg_node) = DFMRemoveMask (MT_DEFMASK (arg_node));
    }
    if (MT_NEEDLATER (arg_node) != NULL) {
        MT_NEEDLATER (arg_node) = DFMRemoveMask (MT_NEEDLATER (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/*--------------------------------------------------------------------------*/

node *
FreeST (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("FreeMT");

    DBUG_PRINT ("FREE", ("Removing contents of N_mt node ..."));

    FREETRAV (ST_REGION (arg_node));

    if (ST_USEMASK (arg_node) != NULL) {
        ST_USEMASK (arg_node) = DFMRemoveMask (ST_USEMASK (arg_node));
    }
    if (ST_DEFMASK (arg_node) != NULL) {
        ST_DEFMASK (arg_node) = DFMRemoveMask (ST_DEFMASK (arg_node));
    }
    if (ST_NEEDLATER_ST (arg_node) != NULL) {
        ST_NEEDLATER_ST (arg_node) = DFMRemoveMask (ST_NEEDLATER_ST (arg_node));
    }
    if (ST_NEEDLATER_MT (arg_node) != NULL) {
        ST_NEEDLATER_MT (arg_node) = DFMRemoveMask (ST_NEEDLATER_MT (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/*--------------------------------------------------------------------------*/

node *
FreeInfo (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeInfo");
    DBUG_PRINT ("FREE", ("Removing N_info node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeNWith (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeNWith");
    DBUG_PRINT ("FREE", ("Removing N_with node ..."));

    FREETRAV (NWITH_PART (arg_node));
    FREETRAV (NWITH_CODE (arg_node));
    FREETRAV (NWITH_WITHOP (arg_node));
    FREE (arg_node->info2);

    if (NWITH_IN_MASK (arg_node) != NULL) {
        NWITH_IN_MASK (arg_node) = DFMRemoveMask (NWITH_IN_MASK (arg_node));
    }
    if (NWITH_OUT_MASK (arg_node) != NULL) {
        NWITH_OUT_MASK (arg_node) = DFMRemoveMask (NWITH_OUT_MASK (arg_node));
    }
    if (NWITH_LOCAL_MASK (arg_node) != NULL) {
        NWITH_LOCAL_MASK (arg_node) = DFMRemoveMask (NWITH_LOCAL_MASK (arg_node));
    }

    NWITH_DEC_RC_IDS (arg_node) = FreeAllIds (NWITH_DEC_RC_IDS (arg_node));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeNPart (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeNPart");
    DBUG_PRINT ("FREE", ("Removing N_NPart node ..."));

    FREETRAV (NPART_WITHID (arg_node));
    FREETRAV (NPART_GEN (arg_node));

    if (NPART_CODE (arg_node) != NULL) {
        /* see remarks of N_Ncode in tree_basic.h */
        NCODE_USED (NPART_CODE (arg_node))--;
        DBUG_ASSERT ((NCODE_USED (NPART_CODE (arg_node)) >= 0),
                     "NCODE_USED dropped below 0");
    }

    tmp = FREECONT (NPART_NEXT (arg_node));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeNWithID (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeNWithID");
    DBUG_PRINT ("FREE", ("Removing N_Nwithid node ..."));

    NWITHID_IDS (arg_node) = FreeAllIds (NWITHID_IDS (arg_node));
    NWITHID_VEC (arg_node) = FreeAllIds (NWITHID_VEC (arg_node));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeNGenerator (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeNGenerator");

    DBUG_PRINT ("FREE", ("Removing N_NGenerator node ..."));

    FREETRAV (NGEN_BOUND1 (arg_node));
    FREETRAV (NGEN_BOUND2 (arg_node));
    FREETRAV (NGEN_STEP (arg_node));
    FREETRAV (NGEN_WIDTH (arg_node));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeNWithOp (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeNWithOp");

    DBUG_PRINT ("FREE", ("Removing N_Nwithop node ..."));

    FREETRAV (NWITHOP_NEUTRAL (arg_node)); /* removes _SHAPE or _ARRAY as well */

    /*
     * if WithOp is WO_foldfun the function name has to be freed.
     * The modul_name is shared.
     */
    if (WO_foldfun == NWITHOP_TYPE (arg_node)) {
        FREE (NWITHOP_FUN (arg_node));
#if FREE_MODNAMES
        FREE (NWITHOP_MOD (arg_node));
#endif
    }

    /* free mem allocated in MakeNWithOp */
    FREE (arg_node->info2);

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeNCode (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeNCode");
    DBUG_PRINT ("FREE", ("Removing N_Ncode node ..."));

    tmp = FREECONT (NCODE_NEXT (arg_node));
    FREETRAV (NCODE_CBLOCK (arg_node));
    FREETRAV (NCODE_CEXPR (arg_node));

    NCODE_INC_RC_IDS (arg_node) = FreeAllIds (NCODE_INC_RC_IDS (arg_node));

    if (NCODE_WLAA_INFO (arg_node) != NULL) {
        NCODE_WLAA_ACCESS (arg_node) = FreeAllAccess (NCODE_WLAA_ACCESS (arg_node));
        NCODE_WLAA_INFO (arg_node) = FreeNode (NCODE_WLAA_INFO (arg_node));
    }

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeNwith2 (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeNWith2");
    DBUG_PRINT ("FREE", ("Removing N_Nwith2 node ..."));

    FREETRAV (NWITH2_WITHID (arg_node));
    FREETRAV (NWITH2_SEGS (arg_node));
    FREETRAV (NWITH2_CODE (arg_node));
    FREETRAV (NWITH2_WITHOP (arg_node));

    if (NWITH2_IN_MASK (arg_node) != NULL) {
        NWITH2_IN_MASK (arg_node) = DFMRemoveMask (NWITH2_IN_MASK (arg_node));
    }
    if (NWITH2_OUT_MASK (arg_node) != NULL) {
        NWITH2_OUT_MASK (arg_node) = DFMRemoveMask (NWITH2_OUT_MASK (arg_node));
    }
    if (NWITH2_LOCAL_MASK (arg_node) != NULL) {
        NWITH2_LOCAL_MASK (arg_node) = DFMRemoveMask (NWITH2_LOCAL_MASK (arg_node));
    }

    NWITH2_DEC_RC_IDS (arg_node) = FreeAllIds (NWITH2_DEC_RC_IDS (arg_node));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeWLseg (node *arg_node, node *arg_info)
{
    int b;
    node *tmp = NULL;

    DBUG_ENTER ("FreeWLseg");

    DBUG_PRINT ("FREE", ("Removing N_WLseg node ..."));

    FREETRAV (WLSEG_CONTENTS (arg_node));
    tmp = FREECONT (WLSEG_NEXT (arg_node));

    for (b = 0; b < WLSEG_BLOCKS (arg_node); b++) {
        if (WLSEG_BV (arg_node, b) != NULL) {
            FREE (WLSEG_BV (arg_node, b));
        }
    }
    FREE (WLSEG_UBV (arg_node));

    FREE (WLSEG_SV (arg_node));
    FREE (WLSEG_IDX_MIN (arg_node));
    FREE (WLSEG_IDX_MAX (arg_node));

    if (WLSEG_SCHEDULING (arg_node) != NULL) {
        WLSEG_SCHEDULING (arg_node) = SCHRemoveScheduling (WLSEG_SCHEDULING (arg_node));
    }

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeWLsegVar (node *arg_node, node *arg_info)
{
    int d;
    node *tmp = NULL;

    DBUG_ENTER ("FreeWLsegVar");

    DBUG_PRINT ("FREE", ("Removing N_WLsegVar node ..."));

    FREETRAV (WLSEGVAR_CONTENTS (arg_node));
    tmp = FREECONT (WLSEGVAR_NEXT (arg_node));

    if (WLSEGVAR_IDX_MIN (arg_node) != NULL) {
        for (d = 0; d < WLSEGVAR_DIMS (arg_node); d++) {
            FREETRAV ((WLSEGVAR_IDX_MIN (arg_node)[d]));
        }
    }
    FREE (WLSEGVAR_IDX_MIN (arg_node));
    if (WLSEGVAR_IDX_MAX (arg_node) != NULL) {
        for (d = 0; d < WLSEGVAR_DIMS (arg_node); d++) {
            FREETRAV ((WLSEGVAR_IDX_MAX (arg_node)[d]));
        }
    }
    FREE (WLSEGVAR_IDX_MAX (arg_node));

    if (WLSEGVAR_SCHEDULING (arg_node) != NULL) {
        WLSEGVAR_SCHEDULING (arg_node)
          = SCHRemoveScheduling (WLSEGVAR_SCHEDULING (arg_node));
    }

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeWLblock (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeWLblock");

    DBUG_PRINT ("FREE", ("Removing N_WLblock node ..."));

    FREETRAV (WLBLOCK_NEXTDIM (arg_node));
    FREETRAV (WLBLOCK_CONTENTS (arg_node));
    tmp = FREECONT (WLBLOCK_NEXT (arg_node));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeWLublock (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeWLublock");

    DBUG_PRINT ("FREE", ("Removing N_WLublock node ..."));

    FREETRAV (WLUBLOCK_NEXTDIM (arg_node));
    FREETRAV (WLUBLOCK_CONTENTS (arg_node));
    tmp = FREECONT (WLUBLOCK_NEXT (arg_node));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeWLstride (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeWLstride");

    DBUG_PRINT ("FREE", ("Removing N_WLstride node ..."));

    FREETRAV (WLSTRIDE_CONTENTS (arg_node));
    tmp = FREECONT (WLSTRIDE_NEXT (arg_node));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeWLstrideVar (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeWLstrideVar");

    DBUG_PRINT ("FREE", ("Removing N_WLstrideVar node ..."));

    FREETRAV (WLSTRIDEVAR_BOUND1 (arg_node));
    FREETRAV (WLSTRIDEVAR_BOUND2 (arg_node));
    FREETRAV (WLSTRIDEVAR_STEP (arg_node));
    FREETRAV (WLSTRIDEVAR_CONTENTS (arg_node));
    tmp = FREECONT (WLSTRIDEVAR_NEXT (arg_node));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeWLgrid (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeWLgrid");

    DBUG_PRINT ("FREE", ("Removing N_WLgrid node ..."));

    FREETRAV (WLGRID_NEXTDIM (arg_node));
    tmp = FREECONT (WLGRID_NEXT (arg_node));

    if (WLGRID_CODE (arg_node) != NULL) {
        NCODE_USED (WLGRID_CODE (arg_node))--;
        DBUG_ASSERT ((NCODE_USED (WLGRID_CODE (arg_node)) >= 0),
                     "NCODE_USED dropped below 0");
    }

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeWLgridVar (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeWLgridVar");

    DBUG_PRINT ("FREE", ("Removing N_WLgridVar node ..."));

    FREETRAV (WLGRIDVAR_BOUND1 (arg_node));
    FREETRAV (WLGRIDVAR_BOUND2 (arg_node));
    FREETRAV (WLGRIDVAR_NEXTDIM (arg_node));
    tmp = FREECONT (WLGRIDVAR_NEXT (arg_node));

    if (WLGRIDVAR_CODE (arg_node) != NULL) {
        NCODE_USED (WLGRIDVAR_CODE (arg_node))--;
        DBUG_ASSERT ((NCODE_USED (WLGRIDVAR_CODE (arg_node)) >= 0),
                     "NCODE_USED dropped below 0");
    }

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeCWrapper (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeCWrapper");

    DBUG_PRINT ("FREE", ("Removing N_cwrapper node ..."));

    tmp = FREECONT (CWRAPPER_NEXT (arg_node));

    FREE (CWRAPPER_NAME (arg_node));
#if FREE_MODNAMES
    FREE (CWRAPPER_MOD (arg_node));
#endif

    CWRAPPER_FUNS (arg_node) = FreeNodelist (CWRAPPER_FUNS (arg_node));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeModspec (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeModspec");

    DBUG_PRINT ("FREE", ("Removing contents of N_modspec node ..."));

    FREETRAV (MODSPEC_OWN (arg_node));

    FREE (MODSPEC_NAME (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_moddec node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeCSEinfo (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeCSEinfo");

    DBUG_PRINT ("FREE", ("Removing contents of N_cseinfo node ..."));

    tmp = FREECONT (CSEINFO_NEXT (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_cseinfo node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}
/*--------------------------------------------------------------------------*/

node *
FreeSSAcnt (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeSSAcnt");

    DBUG_PRINT ("FREE", ("Removing contents of N_ssacnt node ..."));

    FREETRAV (SSACNT_NEXT (arg_node));

    FREE (SSACNT_BASEID (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_ssacnt node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeSSAstack (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeSSAstack");

    DBUG_PRINT ("FREE", ("Removing contents of N_ssastack node ..."));

    tmp = FREECONT (SSASTACK_NEXT (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_ssastack node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeAvis (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeAvis");

    DBUG_PRINT ("FREE", ("Removing contents of N_avis node ..."));

    if (AVIS_SSACONST (arg_node) != NULL) {
        AVIS_SSACONST (arg_node) = COFreeConstant (AVIS_SSACONST (arg_node));
    }

    if (AVIS_SSASTACK (arg_node) != NULL) {
        FREETRAV (AVIS_SSASTACK (arg_node));
    }

    DBUG_PRINT ("FREE", ("Removing N_avis node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

/********************************************************************
 *
 *  behind this line for compatibility only
 *
 */

/*
 *
 *  functionname  : FreePrf2
 *  arguments     : 1) prf-node
 *                  2) argument not to be freed
 *  description   : frees whole primitive, without argument arg_no
 *  global vars   : FreeTree
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG..., NULL, FREE
 *
 *  remarks       : needed by ConstantFolding.c
 *
 */

void
FreePrf2 (node *arg_node, int arg_no)
{
    node *tmp1, *tmp2;
    int i;

    DBUG_ENTER ("FreePrf2");
    tmp1 = arg_node->node[0];
    i = 0;
    while (NULL != tmp1) {
        if (i != arg_no) {
            tmp1->node[0] = FreeTree (tmp1->node[0]);
        }
        tmp2 = tmp1;
        tmp1 = tmp1->node[1];
        FREE (tmp2);
        i++;
    }
    FREE (arg_node);
    DBUG_VOID_RETURN;
}
