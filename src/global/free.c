/*
 *
 * $Log$
 * Revision 2.3  1999/02/28 21:08:56  srs
 * bugfix in FreeAssign()
 *
 * Revision 2.2  1999/02/25 09:41:24  bs
 * FreeId and FreeArray modified: compact propagation of constant
 * integer vectors will be deallocated now.
 *
 * Revision 2.1  1999/02/23 12:39:17  sacbase
 * new release made
 *
 * Revision 1.79  1999/02/11 13:37:23  cg
 * Improved debugging opportunities by hiding calls to free() behind
 * wrapper function Free().
 *
 * Revision 1.78  1999/02/06 12:53:32  srs
 * added FreeNodelistNode
 *
 * Revision 1.77  1999/01/15 15:13:19  cg
 * added functions FreeOneAccess() and FreeAllAccess().
 *
 * Revision 1.76  1998/11/10 10:32:09  sbs
 * CHECK_NULL inserted in DBUG_PRINT("FREE"..) in FreeArg.
 *
 * Revision 1.75  1998/08/11 12:08:27  dkr
 * FreeWLsegVar changed
 *
 * Revision 1.74  1998/08/11 00:03:34  dkr
 * changed FreeWLsegVar
 *
 * Revision 1.73  1998/08/07 14:36:39  dkr
 * FreeWLsegVar added
 *
 * Revision 1.72  1998/07/03 10:14:02  cg
 * freeing of N_spmd node completely changed.
 *
 * Revision 1.71  1998/06/18 13:46:29  cg
 * file is now able to deal correctly with data objects of
 * the abstract data type for the representation of schedulings.
 *
 * Revision 1.70  1998/06/09 16:45:31  dkr
 * IDX_MIN, IDX_MAX now segment-specific
 *
 * Revision 1.69  1998/06/08 08:57:34  cg
 * handling of attribute ARRAY_TYPE corrected.
 *
 * Revision 1.68  1998/06/04 16:57:07  cg
 * information about refcounted variables in the context of loops,
 * conditionals and the old with-loop are now stored in ids-chains
 * instead of N_exprs lists.
 *
 * Revision 1.67  1998/06/03 14:23:43  cg
 *  free now handles attribute FUNDEF_LIFTEDFROM correctly
 *
 * Revision 1.66  1998/05/24 00:40:13  dkr
 * removed WLGRID_CODE_TEMPLATE
 *
 * Revision 1.65  1998/05/21 13:30:35  dkr
 * renamed NCODE_DEC_RC_IDS into NCODE_INC_RC_IDS
 *
 * Revision 1.64  1998/05/17 00:08:46  dkr
 * changed FreeWLgrid, FreeWLgridVar
 *
 * Revision 1.63  1998/05/12 22:44:44  dkr
 * changed FreeNwith2:
 *   added NWITH2_IDX_MIN, NWITH2_IDX_MAX
 *
 * Revision 1.62  1998/05/12 15:51:41  dkr
 * removed ???_VARINFO
 *
 * Revision 1.60  1998/05/11 15:16:41  dkr
 * changed FreeNwith, FreeNwith2:
 *   free NWITH_RC_IDS
 *
 * Revision 1.56  1998/04/26 21:51:15  dkr
 * FreeSPMD renamed to FreeSpmd
 *
 * Revision 1.55  1998/04/24 17:15:47  dkr
 * changed usage of SPMD_IN/OUT/INOUT, SYNC_INOUT
 *
 * Revision 1.54  1998/04/24 12:12:45  dkr
 * changed FreeSPMD
 *
 * Revision 1.53  1998/04/24 01:14:37  dkr
 * added FreeSync
 *
 * Revision 1.52  1998/04/21 13:30:51  dkr
 * NWITH2_SEG renamed to NWITH2_SEGS
 *
 * Revision 1.51  1998/04/20 02:37:53  dkr
 * changed comments
 *
 * Revision 1.50  1998/04/17 17:25:19  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.49  1998/04/14 19:01:46  srs
 * changed FreeAssign()
 *
 * Revision 1.48  1998/04/13 19:01:47  dkr
 * support for wlcomp-pragmas added in FreePragma
 *
 *
 *
 *
 * Revision 1.1  1994/12/20  15:42:10  sbs
 * Initial revision
 *
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include "dbug.h"
#include "my_debug.h"

#include "types.h"
#include "tree.h"

#include "traverse.h"
#include "DataFlowMask.h"
#include "scheduling.h"

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
        int i;                                                                           \
                                                                                         \
        for (i = 0; i < MAX_MASK; i++) {                                                 \
            FREE (mac (arg_node, i));                                                    \
        }                                                                                \
    }

#define FREETRAV(node)                                                                   \
    {                                                                                    \
        if (node != NULL) {                                                              \
            node = Trav (node, node); /* dkr: remove whole subtree !!! */                \
        }                                                                                \
    }

#define FREECONT(node)                                                                   \
    ((arg_info != NULL) && (node != NULL)) ? Trav (node, arg_info) : node

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
 *
 */

shpseg *
FreeShpseg (shpseg *fr)
{
    DBUG_ENTER ("FreeShpseg");

    DBUG_PRINT ("FREE", ("Removing shpseg"));

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
            FreeShpseg (TYPES_SHPSEG (tmp));
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
        FreeAllDeps (DEPS_SUB (tmp));

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
    nodelist *tmp;

    DBUG_ENTER ("FreeNodelist");

    while (list != NULL) {
        tmp = list;
        list = NODELIST_NEXT (list);
        FREE (tmp);
    }

    tmp = NULL;

    DBUG_RETURN (tmp);
}

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
    access_t *tmp;

    DBUG_ENTER ("FreeOneAccess");

    if (fr != NULL) {
        DBUG_PRINT ("FREE",
                    ("Removing Access: psi(%s, %s)", VARDEC_OR_ARG_NAME (ACCESS_IV (fr)),
                     VARDEC_OR_ARG_NAME (ACCESS_ARRAY (fr))));

        tmp = fr;
        fr = ACCESS_NEXT (fr);

        if (ACCESS_OFFSET (tmp) != NULL) {
            ACCESS_OFFSET (tmp) = FreeShpseg (ACCESS_OFFSET (tmp));
        }

        FREE (tmp);
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

/*
 *  There are two general free functions for node structures:
 *
 *  FreeNode removes the given node and returns a pointer to the next
 *  node structure in a list of node structures.
 *  This function is suitable for all node types which form chained lists,
 *  e.g. fundef, objdef, typedef. For other node types a NULL is returned.
 *
 *  FreeTree removes the whole sub tree behind the given pointer.
 *
 */

node *
FreeNode (node *free_node)
{
    funptr *store_tab;

    DBUG_ENTER ("FreeNode");

    store_tab = act_tab;
    act_tab = free_tab;

    free_node = Trav (free_node, NULL);

    act_tab = store_tab;

    DBUG_RETURN (free_node);
}

/*--------------------------------------------------------------------------*/

node *
FreeTree (node *free_node)
{
    funptr *store_tab;

    DBUG_ENTER ("FreeTree");

    store_tab = act_tab;
    act_tab = free_tab;

    Trav (free_node, free_node);

    act_tab = store_tab;

    DBUG_RETURN ((node *)NULL);
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

/* srs: module_name __MAIN is allocated statically in sac.y
 *      the string of the module name is potentially shared.
 */
#if 0
  FREE(MODUL_NAME(arg_node));
#endif

    DBUG_PRINT ("FREE", ("Removing N_modul node ..."));

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
    FreeAllDeps (MODDEC_LINKWITH (arg_node));

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
    FreeAllDeps (CLASSDEC_LINKWITH (arg_node));

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
    FreeAllDeps (SIB_LINKWITH (arg_node));

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
    FreeAllIds (IMPLIST_ITYPES (arg_node));
    FreeAllIds (IMPLIST_ETYPES (arg_node));
    FreeAllIds (IMPLIST_OBJS (arg_node));
    FreeAllIds (IMPLIST_FUNS (arg_node));

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
    FreeAllTypes (TYPEDEF_TYPE (arg_node));
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
    FREE (OBJDEF_VARNAME (arg_node));
    FreeOneTypes (OBJDEF_TYPE (arg_node));
#if 0
  FreeNodelist(OBJDEF_NEEDOBJS(arg_node));
#endif

    DBUG_PRINT ("FREE", ("Removing N_objdef node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeFundef (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeFundef");

    DBUG_PRINT ("FREE",
                ("Removing contents of N_fundef node %s ...", ItemName (arg_node)));

    tmp = FREECONT (FUNDEF_NEXT (arg_node));

    if (FUNDEF_ICM (arg_node) != NULL)
        if (NODE_TYPE (FUNDEF_ICM (arg_node)) == N_icm) {
            FREETRAV (FUNDEF_ICM (arg_node));

            /*
             *  FUNDEF_ICM may not be freed without precondition, because it's
             *  stored on the same real son node as FUNDEF_RETURN.
             */
        }

    FREETRAV (FUNDEF_BODY (arg_node));
    FREETRAV (FUNDEF_ARGS (arg_node));

    if (FUNDEF_PRAGMA (arg_node) != NULL) {
        FREETRAV (FUNDEF_PRAGMA (arg_node));
    }

    FREE (FUNDEF_NAME (arg_node));

    if (FUNDEF_STATUS (arg_node) != ST_spmdfun) {
        FreeNodelist (FUNDEF_NEEDOBJS (arg_node));
    }

    FREEMASK (FUNDEF_MASK);

    if (FUNDEF_DFM_BASE (arg_node) != NULL) {
        FUNDEF_DFM_BASE (arg_node) = DFMRemoveMaskBase (FUNDEF_DFM_BASE (arg_node));
    }

    DBUG_PRINT ("FREE", ("Removing N_fundef node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeArg (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeArg");

    DBUG_PRINT ("FREE", ("Removing contents of N_arg node %s ...",
                         CHECK_NULL (ARG_NAME (arg_node))));
    /* ARG_NAME(arg_node) may be NULL in external decls! */

    tmp = FREECONT (ARG_NEXT (arg_node));

    FREE (ARG_NAME (arg_node));
    FreeOneTypes (ARG_TYPE (arg_node));

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
    FreeNodelist (BLOCK_NEEDFUNS (arg_node));
    FreeNodelist (BLOCK_NEEDTYPES (arg_node));
    FREEMASK (BLOCK_MASK);

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
    FreeOneTypes (VARDEC_TYPE (arg_node));

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

    FREETRAV (ASSIGN_INSTR (arg_node));
    FREEMASK (ASSIGN_MASK);
    index = (index_info *)ASSIGN_INDEX (arg_node);
    if (index)
        FREE_INDEX_INFO (index);

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
    FreeAllIds (LET_IDS (arg_node));

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
    FreeOneTypes (CAST_TYPE (arg_node));

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

    FreeAllIds (COND_THENVARS (arg_node));
    FreeAllIds (COND_ELSEVARS (arg_node));

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

    FreeAllIds (DO_USEVARS (arg_node));
    FreeAllIds (DO_DEFVARS (arg_node));

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

    FreeAllIds (WHILE_USEVARS (arg_node));
    FreeAllIds (WHILE_DEFVARS (arg_node));

    FREEMASK (WHILE_MASK);

    DBUG_PRINT ("FREE", ("Removing N_while node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeAp (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeAp");

    DBUG_PRINT ("FREE", ("Removing contents of N_ap node %s ...", AP_NAME (arg_node)));

    FREETRAV (AP_ARGS (arg_node));
    FREE (AP_NAME (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_ap node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeWith (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeWith");

    DBUG_PRINT ("FREE", ("Removing contents of N_with node ..."));

    FREETRAV (WITH_GEN (arg_node));
    FREETRAV (WITH_OPERATOR (arg_node));
    FREEMASK (WITH_MASK);

    DBUG_PRINT ("FREE", ("Removing N_with node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeGenerator (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeGenerator");

    DBUG_PRINT ("FREE", ("Removing contents of N_generator node ..."));

    FREETRAV (GEN_LEFT (arg_node));
    FREETRAV (GEN_RIGHT (arg_node));
    FREE (GEN_ID (arg_node));
    FREEMASK (GEN_MASK);

    DBUG_PRINT ("FREE", ("Removing N_generator node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeGenarray (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeGenarray");

    DBUG_PRINT ("FREE", ("Removing contents of N_genarray node ..."));

    FREETRAV (GENARRAY_ARRAY (arg_node));
    FREETRAV (GENARRAY_BODY (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_genarray node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeModarray (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeModarray");

    DBUG_PRINT ("FREE", ("Removing contents of N_modarray node ..."));

    FREETRAV (MODARRAY_ARRAY (arg_node));
    FREETRAV (MODARRAY_BODY (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_modarray node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeFoldprf (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeFoldprf");

    DBUG_PRINT ("FREE", ("Removing contents of N_folfprf node ..."));

    FREETRAV (FOLDPRF_BODY (arg_node));
    FREETRAV (FOLDPRF_NEUTRAL (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_foldprf node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeFoldfun (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeFoldfun");

    DBUG_PRINT ("FREE",
                ("Removing contents of N_foldfun node %s ...", FOLDFUN_NAME (arg_node)));

    FREETRAV (FOLDFUN_BODY (arg_node));
    FREETRAV (FOLDFUN_NEUTRAL (arg_node));
    FREE (FOLDFUN_NAME (arg_node));
    /* the module name must not be set free because it is shared. */

    DBUG_PRINT ("FREE", ("Removing N_foldfun node ..."));

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
    int *iarray;
    char *carray;

    node *tmp = NULL;

    DBUG_ENTER ("FreeArray");

    DBUG_PRINT ("FREE", ("Removing contents of N_array node ..."));

    FREETRAV (ARRAY_AELEMS (arg_node));

    if (ARRAY_TYPE (arg_node) != NULL) {
        FreeOneTypes (ARRAY_TYPE (arg_node));
    }

    DBUG_PRINT ("FREE", ("Removing N_array node ..."));

    iarray = ARRAY_INTARRAY (arg_node);
    FREE (iarray);

    carray = ARRAY_STRING (arg_node);
    FREE (carray);

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

    FREE (VINFO_TYPE (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_vinfo node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeId (node *arg_node, node *arg_info)
{
    int *carray;

    node *tmp = NULL;

    DBUG_ENTER ("FreeId");

    DBUG_PRINT ("FREE", ("Removing contents of N_id node %s ...", ID_NAME (arg_node)));

    FREE (ID_NAME (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_id node ..."));

    carray = ID_CONSTARRAY (arg_node);
    FREE (carray);

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
FreePost (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreePost");

    DBUG_PRINT ("FREE", ("Removing contents of N_post node %s ...", POST_ID (arg_node)));

    FREE (POST_ID (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_post node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreePre (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreePre");

    DBUG_PRINT ("FREE", ("Removing contents of N_pre node %s ...", PRE_ID (arg_node)));

    FREE (PRE_ID (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_pre node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeInc (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeInc");

    DBUG_PRINT ("FREE", ("Removing N_inc node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeDec (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeDec");

    DBUG_PRINT ("FREE", ("Removing N_dec node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeIcm (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeIcm");

    DBUG_PRINT ("FREE", ("Removing contents of N_icm node %s ...", ICM_NAME (arg_node)));

    tmp = FREECONT (ICM_NEXT (arg_node));

    /*
     * In 'compile' arguments of ICMs are often shared 8-(
     *
     * brute force try!
     */
#if 0
  FREETRAV(ICM_ARGS(arg_node));
#endif

    /*
     * Since the name in most (all?) cases is static,
     *  please do not free ICM_NAME(arg_node) !!!
     */

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
    FreeAllIds (PRAGMA_EFFECT (arg_node));
    FreeAllIds (PRAGMA_TOUCH (arg_node));
    FREE (PRAGMA_LINKNAME (arg_node));
    FREE (PRAGMA_COPYFUN (arg_node));
    FREE (PRAGMA_FREEFUN (arg_node));
    FREE (PRAGMA_INITFUN (arg_node));

    FREETRAV (PRAGMA_WLCOMP_APS (arg_node));

    /*
    FreeAllIds(PRAGMA_NEEDTYPES(arg_node));
    FREETRAV(PRAGMA_NEEDFUNS(arg_node));
    */

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

    if (SYNC_SCHEDULING (arg_node) != NULL) {
        SYNC_SCHEDULING (arg_node) = SCHRemoveScheduling (SYNC_SCHEDULING (arg_node));
    }

    DBUG_PRINT ("FREE", ("Removing N_sync node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
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

    if (NWITH_IN (arg_node) != NULL) {
        NWITH_IN (arg_node) = DFMRemoveMask (NWITH_IN (arg_node));
    }
    if (NWITH_INOUT (arg_node) != NULL) {
        NWITH_INOUT (arg_node) = DFMRemoveMask (NWITH_INOUT (arg_node));
    }
    if (NWITH_OUT (arg_node) != NULL) {
        NWITH_OUT (arg_node) = DFMRemoveMask (NWITH_OUT (arg_node));
    }
    if (NWITH_LOCAL (arg_node) != NULL) {
        NWITH_LOCAL (arg_node) = DFMRemoveMask (NWITH_LOCAL (arg_node));
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
        NCODE_USED (NPART_CODE (arg_node))--; /* see remarks of N_Ncode in tree_basic.h */
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

    FreeAllIds (NWITHID_IDS (arg_node));
    FreeAllIds (NWITHID_VEC (arg_node));

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

    /* if WithOp is WO_foldfun the function name has to be freed.
       The modul_name is shared. */
    if (WO_foldfun == NWITHOP_TYPE (arg_node))
        FREE (NWITHOP_FUN (arg_node));

    /* free mem allocated in MakeNWithOp */
    FREE (arg_node->info2);

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeNCode (node *arg_node, node *arg_info)
{
    node *tmp;

    DBUG_ENTER ("FreeNCode");
    DBUG_PRINT ("FREE", ("Removing N_Ncode node ..."));

    tmp = FREECONT (NCODE_NEXT (arg_node));
    FREETRAV (NCODE_CBLOCK (arg_node));
    FREETRAV (NCODE_CEXPR (arg_node));

    NCODE_INC_RC_IDS (arg_node) = FreeAllIds (NCODE_INC_RC_IDS (arg_node));
    NCODE_ACCESS (arg_node) = FreeAllAccess (NCODE_ACCESS (arg_node));

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

    if (NWITH2_IN (arg_node) != NULL) {
        NWITH2_IN (arg_node) = DFMRemoveMask (NWITH2_IN (arg_node));
    }
    if (NWITH2_INOUT (arg_node) != NULL) {
        NWITH2_INOUT (arg_node) = DFMRemoveMask (NWITH2_INOUT (arg_node));
    }
    if (NWITH2_OUT (arg_node) != NULL) {
        NWITH2_OUT (arg_node) = DFMRemoveMask (NWITH2_OUT (arg_node));
    }
    if (NWITH2_LOCAL (arg_node) != NULL) {
        NWITH2_LOCAL (arg_node) = DFMRemoveMask (NWITH2_LOCAL (arg_node));
    }

    if (NWITH2_SCHEDULING (arg_node) != NULL) {
        NWITH2_SCHEDULING (arg_node) = SCHRemoveScheduling (NWITH2_SCHEDULING (arg_node));
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

    FREE (WLSEG_IDX_MIN (arg_node));
    FREE (WLSEG_IDX_MAX (arg_node));

    for (b = 0; b < WLSEG_BLOCKS (arg_node); b++) {
        if (WLSEG_BV (arg_node, b) != NULL) {
            FREE (WLSEG_BV (arg_node, b));
        }
    }
    if (WLSEG_UBV (arg_node) != NULL) {
        FREE (WLSEG_UBV (arg_node));
    }
    if (WLSEG_SV (arg_node) != NULL) {
        FREE (WLSEG_SV (arg_node));
    }

    if (WLSEG_SCHEDULING (arg_node) != NULL) {
        WLSEG_SCHEDULING (arg_node) = SCHRemoveScheduling (WLSEG_SCHEDULING (arg_node));
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
FreeWLgrid (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeWLgrid");
    DBUG_PRINT ("FREE", ("Removing N_WLgrid node ..."));

    FREETRAV (WLGRID_NEXTDIM (arg_node));
    tmp = FREECONT (WLGRID_NEXT (arg_node));

    if (WLGRID_CODE (arg_node) != NULL) {
        NCODE_USED (WLGRID_CODE (arg_node))--;
    }

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeWLsegVar (node *arg_node, node *arg_info)
{
    int b;
    node *tmp = NULL;

    DBUG_ENTER ("FreeWLsegVar");
    DBUG_PRINT ("FREE", ("Removing N_WLsegVar node ..."));

    FREETRAV (WLSEGVAR_CONTENTS (arg_node));
    tmp = FREECONT (WLSEGVAR_NEXT (arg_node));

    FREE (WLSEGVAR_IDX_MIN (arg_node));
    FREE (WLSEGVAR_IDX_MAX (arg_node));

    for (b = 0; b < WLSEGVAR_BLOCKS (arg_node); b++) {
        if (WLSEGVAR_BV (arg_node, b) != NULL) {
            FREE (WLSEGVAR_BV (arg_node, b));
        }
    }
    if (WLSEGVAR_UBV (arg_node) != NULL) {
        FREE (WLSEGVAR_UBV (arg_node));
    }
    if (WLSEGVAR_SV (arg_node) != NULL) {
        FREE (WLSEGVAR_SV (arg_node));
    }

    if (WLSEGVAR_SCHEDULING (arg_node) != NULL) {
        WLSEGVAR_SCHEDULING (arg_node)
          = SCHRemoveScheduling (WLSEGVAR_SCHEDULING (arg_node));
    }

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
FreeWLstriVar (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeWLstriVar");
    DBUG_PRINT ("FREE", ("Removing N_WLstriVar node ..."));

    FREETRAV (WLSTRIVAR_BOUND1 (arg_node));
    FREETRAV (WLSTRIVAR_BOUND2 (arg_node));
    FREETRAV (WLSTRIVAR_STEP (arg_node));
    FREETRAV (WLSTRIVAR_CONTENTS (arg_node));
    tmp = FREECONT (WLSTRIVAR_NEXT (arg_node));

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
    }

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
        if (i != arg_no)
            FreeTree (tmp1->node[0]);
        tmp2 = tmp1;
        tmp1 = tmp1->node[1];
        FREE (tmp2);
        i++;
    }
    FREE (arg_node);
    DBUG_VOID_RETURN;
}
