/*
 *
 * $Log$
 * Revision 1.23  1997/10/31 16:17:18  srs
 * memory for module names is NOT deallocated anymore.
 * reason: FREE() on constant string (__MAIN) not possible.
 *
 * Revision 1.22  1997/10/31 10:32:09  dkr
 * with defined NEWTREE, node->nnode is (partly) not used anymore
 *
 * Revision 1.21  1997/08/04 19:11:38  dkr
 * no FREE, FREETRAV in FreeIcm
 *
 * Revision 1.20  1997/03/19 13:34:57  cg
 * added functions FreeAllDeps() and FreeOneDeps()
 *
 * Revision 1.19  1996/02/11  20:19:01  sbs
 * some minor corrections on stuff concerning N_vinfo
 *
 * Revision 1.18  1996/01/22  09:38:55  cg
 * modified FreeObjdef with respect to new pragmas
 *
 * Revision 1.17  1996/01/07  16:52:11  cg
 * N_typedef and N_objdef node have no longer N_pragma subnodes
 *
 * Revision 1.16  1995/12/29  10:22:52  cg
 * modified FreeSib according to new node structure,
 * added FreeInfo
 *
 * Revision 1.15  1995/12/20  08:13:05  cg
 * modified FreePragma with respect to using arrays for pragmas linksign,
 * refcounting, and readonly.
 * new function FreeChar for new N_char node
 *
 * Revision 1.14  1995/12/18  16:12:44  cg
 * last free() changed to macro FREE().
 * types->id no longer freed by FreeOneTypes and FreeAllTypes
 *
 * Revision 1.13  1995/12/07  14:16:18  cg
 * Now, the free functions traverse chained lists onlyn with
 * respect to the current setting of nnode, which may be different
 * to the original setting in the respective Make function due to
 * old code. This applies to the following nodes:
 * N_arg, N_vardec, N_assign, N_exprs, N_vinfo, N_icm
 *
 * Revision 1.12  1995/12/01  16:14:27  cg
 * added function FreePragma for new node type N_pragma.
 * renamed function FreeTypes to FreeOneTypes in contrast to FreeAllTypes.
 * same with other free functions for non-node structures.
 *
 * Revision 1.11  1995/11/16  19:33:24  cg
 * The free module was entirely rewritten.
 * Each node type now has its own free function.
 * Functions FreeTree and FreeNode for deleting single nodes and
 * whole sub trees.
 * Various free functions for non-node structures.
 * FreeNodelist moved from tree_compound.c
 *
 * Revision 1.10  1995/08/24  14:01:56  cg
 * minor changes concerning objects etc.
 *
 * Revision 1.9  1995/06/16  13:10:46  asi
 * added FreePrf2, which will free memory occupied by a N_prf-subtree,
 *                 but it will not free one given argument.
 *
 * Revision 1.8  1995/03/24  15:42:31  asi
 * Bug fixed in FreeMask
 *
 * Revision 1.7  1995/03/17  15:54:56  hw
 * changed function FreeInfoType
 *
 * Revision 1.6  1995/01/26  15:16:55  asi
 * *** empty log message ***
 *
 * Revision 1.5  1995/01/18  17:28:37  asi
 * Added FreeTree, FreeNoInfo, FreeInfoId, FreeInfoIds, FreeInfoType, FreeModul
 *
 * Revision 1.4  1994/12/30  16:59:33  sbs
 * added FreeIds
 *
 * Revision 1.3  1994/12/20  17:42:23  hw
 * added includes dbug.h & stdio.h
 *
 * Revision 1.2  1994/12/20  17:34:35  hw
 * bug fixed in FreeIdsOnly
 * exchanged stdio.h with stdlib.h
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
#include "tree_basic.h"
#include "tree_compound.h"

#include "traverse.h"

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
 *  To guarantee the compatibility with old code, 'nnode' is used
 *  temporarily in some free functions for node structures,
 *  i.e. the next node in chained lists is only traversed if there
 *  is one and if 'nnode' allows it concerning its old usage.
 *
 *
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
            node = Trav (node, arg_info);                                                \
        }                                                                                \
    }

#define FREECONT(node)                                                                   \
    ((arg_info != NULL) && (node != NULL) && (arg_node->nnode == nnode_default))         \
      ? Trav (node, arg_info)                                                            \
      : node

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

types *
FreeOneTypes (types *fr)
{
    types *tmp;

    DBUG_ENTER ("FreeTypes");

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

types *
FreeAllTypes (types *fr)
{
    DBUG_ENTER ("FreeAllTypes");

    while (fr != NULL) {
        fr = FreeOneTypes (fr);
    }

    DBUG_RETURN (fr);
}

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

ids *
FreeAllIds (ids *fr)
{
    DBUG_ENTER ("FreeAllIds");

    while (fr != NULL) {
        fr = FreeOneIds (fr);
    }

    DBUG_RETURN (fr);
}

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

nums *
FreeAllNums (nums *fr)
{
    DBUG_ENTER ("FreeAllNums");

    while (fr != NULL) {
        fr = FreeOneNums (fr);
    }

    DBUG_RETURN (fr);
}

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

deps *
FreeAllDeps (deps *fr)
{
    DBUG_ENTER ("FreeAllDeps");

    while (fr != NULL) {
        fr = FreeOneDeps (fr);
    }

    DBUG_RETURN (fr);
}

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

strings *
FreeAllStrings (strings *fr)
{
    DBUG_ENTER ("FreeAllStrings");

    while (fr != NULL) {
        fr = FreeOneStrings (fr);
    }

    DBUG_RETURN (fr);
}

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
 *  it is unable to distinguish between deiiferent kinds of nodes which do
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
    FREETRAV (MODUL_STORE_IMPORTS (arg_node));

    /* srs: module_name __MAIN is allocated statically in sac.y
       the string of the module name is potentially shared.
       FREE(MODUL_NAME(arg_node)); */

    DBUG_PRINT ("FREE", ("Removing N_modul node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

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

node *
FreeImplist (node *arg_node, node *arg_info)
{
    node *tmp = NULL;
    int nnode_default = arg_node->nnode;

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

node *
FreeTypedef (node *arg_node, node *arg_info)
{
    node *tmp = NULL;
    int nnode_default = arg_node->nnode;

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

node *
FreeObjdef (node *arg_node, node *arg_info)
{
    node *tmp = NULL;
    int nnode_default = arg_node->nnode;

    DBUG_ENTER ("FreeObjdef");

    DBUG_PRINT ("FREE",
                ("Removing contents of N_objdef node %s ...", ItemName (arg_node)));

    tmp = FREECONT (OBJDEF_NEXT (arg_node));

    FREETRAV (OBJDEF_EXPR (arg_node));
    FREETRAV (OBJDEF_PRAGMA (arg_node));

    FREE (OBJDEF_NAME (arg_node));
    FREE (OBJDEF_VARNAME (arg_node));
    FreeOneTypes (OBJDEF_TYPE (arg_node));
    /*  FreeNodelist(OBJDEF_NEEDOBJS(arg_node));  */

    DBUG_PRINT ("FREE", ("Removing N_objdef node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

node *
FreeFundef (node *arg_node, node *arg_info)
{
    node *tmp = NULL;
    int nnode_default = arg_node->nnode;

    DBUG_ENTER ("FreeFundef");

    DBUG_PRINT ("FREE",
                ("Removing contents of N_fundef node %s ...", ItemName (arg_node)));

    tmp = FREECONT (FUNDEF_NEXT (arg_node));

    FREETRAV (FUNDEF_BODY (arg_node));
    FREETRAV (FUNDEF_ARGS (arg_node));
    FREETRAV (FUNDEF_PRAGMA (arg_node));

    if ((FUNDEF_ICM (arg_node) != NULL) && (NODE_TYPE (FUNDEF_ICM (arg_node)) == N_icm)) {
        FREETRAV (FUNDEF_ICM (arg_node));

        /*
         *  FUNDEF_ICM may not be freed without precondition, because it's
         *  stored on the same real son node as FUNDEF_RETURN.
         */
    }

    FREE (FUNDEF_NAME (arg_node));
    FreeNodelist (FUNDEF_NEEDOBJS (arg_node));

    FREEMASK (FUNDEF_MASK);

    DBUG_PRINT ("FREE", ("Removing N_fundef node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

node *
FreeArg (node *arg_node, node *arg_info)
{
    node *tmp = NULL;
    int nnode_default = 1;

    DBUG_ENTER ("FreeArg");

    DBUG_PRINT ("FREE", ("Removing contents of N_arg node %s ...", ARG_NAME (arg_node)));

    tmp = FREECONT (ARG_NEXT (arg_node));

    FREE (ARG_NAME (arg_node));
    FreeOneTypes (ARG_TYPE (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_arg node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

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

node *
FreeVardec (node *arg_node, node *arg_info)
{
    node *tmp = NULL;
    int nnode_default = 1;

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

node *
FreeAssign (node *arg_node, node *arg_info)
{
    node *tmp = NULL;
    int nnode_default = 2;

    DBUG_ENTER ("FreeAssign");

    DBUG_PRINT ("FREE", ("Removing contents of N_assign node ..."));

    tmp = FREECONT (ASSIGN_NEXT (arg_node));

    FREETRAV (ASSIGN_INSTR (arg_node));
    FREEMASK (ASSIGN_MASK);

    DBUG_PRINT ("FREE", ("Removing N_assign node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

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

node *
FreeCond (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeCond");

    DBUG_PRINT ("FREE", ("Removing contents of N_cond node ..."));

    FREETRAV (COND_COND (arg_node));
    FREETRAV (COND_THEN (arg_node));
    FREETRAV (COND_ELSE (arg_node));

    /*
     * N_info does not fit into virtual syntax tree.
     */

    if (arg_node->node[3] != NULL) {
        FREETRAV (COND_THENVARS (arg_node));
        FREETRAV (COND_ELSEVARS (arg_node));
        FREE (arg_node->node[3]);
    }

    DBUG_PRINT ("FREE", ("Removing N_cond node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

node *
FreeDo (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeDo");

    DBUG_PRINT ("FREE", ("Removing contents of N_do node ..."));

    FREETRAV (DO_BODY (arg_node));
    FREETRAV (DO_COND (arg_node));

    /*
     * N_info does not fit into virtual syntax tree.
     */

    if (arg_node->node[2] != NULL) {
        FREETRAV (DO_USEVARS (arg_node));
        FREETRAV (DO_DEFVARS (arg_node));
        FREE (arg_node->node[2]);
    }

    FREEMASK (DO_MASK);

    DBUG_PRINT ("FREE", ("Removing N_do node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

node *
FreeWhile (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeWhile");

    DBUG_PRINT ("FREE", ("Removing contents of N_while node ..."));

    FREETRAV (WHILE_BODY (arg_node));
    FREETRAV (WHILE_COND (arg_node));

    /*
     * N_info does not fit into virtual syntax tree.
     */

    if (arg_node->node[2] != NULL) {
        FREETRAV (WHILE_USEVARS (arg_node));
        FREETRAV (WHILE_DEFVARS (arg_node));
        FREE (arg_node->node[2]);
    }

    FREEMASK (WHILE_MASK);

    DBUG_PRINT ("FREE", ("Removing N_while node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

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

    DBUG_PRINT ("FREE", ("Removing N_foldfun node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

node *
FreeExprs (node *arg_node, node *arg_info)
{
    node *tmp = NULL;
    int nnode_default = 2;

    DBUG_ENTER ("FreeExprs");

    DBUG_PRINT ("FREE", ("Removing contents of N_exprs node ..."));

    tmp = FREECONT (EXPRS_NEXT (arg_node));

    FREETRAV (EXPRS_EXPR (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_exprs node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

node *
FreeArray (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeArray");

    DBUG_PRINT ("FREE", ("Removing contents of N_array node ..."));

    FREETRAV (ARRAY_AELEMS (arg_node));
    FreeOneTypes (ARRAY_TYPE (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_array node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

node *
FreeVinfo (node *arg_node, node *arg_info)
{
    node *tmp = NULL;
    int nnode_default = 1;

    DBUG_ENTER ("FreeVinfo");

    DBUG_PRINT ("FREE", ("Removing contents of N_vinfo node ..."));

    tmp = FREECONT (VINFO_NEXT (arg_node));

    FREE (VINFO_TYPE (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_vinfo node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

node *
FreeId (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeId");

    DBUG_PRINT ("FREE", ("Removing contents of N_id node %s ...", ID_NAME (arg_node)));

    FREE (ID_NAME (arg_node));

    DBUG_PRINT ("FREE", ("Removing N_id node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

node *
FreeNum (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeNum");

    DBUG_PRINT ("FREE", ("Removing N_num node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

node *
FreeChar (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeChar");

    DBUG_PRINT ("FREE", ("Removing N_char node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

node *
FreeFloat (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeFloat");

    DBUG_PRINT ("FREE", ("Removing N_float node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

node *
FreeDouble (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeDouble");

    DBUG_PRINT ("FREE", ("Removing N_double node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

node *
FreeBool (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeBool");

    DBUG_PRINT ("FREE", ("Removing N_bool node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

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

node *
FreeEmpty (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeEmpty");

    DBUG_PRINT ("FREE", ("Removing N_empty node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

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

node *
FreeInc (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeInc");

    DBUG_PRINT ("FREE", ("Removing N_inc node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

node *
FreeDec (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeDec");

    DBUG_PRINT ("FREE", ("Removing N_dec node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

node *
FreeIcm (node *arg_node, node *arg_info)
{
    node *tmp = NULL;
    int nnode_default = 2;

    DBUG_ENTER ("FreeIcm");

    DBUG_PRINT ("FREE", ("Removing contents of N_icm node %s ...", ICM_NAME (arg_node)));

    tmp = FREECONT (ICM_NEXT (arg_node));

    /* brute force try!
      FREETRAV(ICM_ARGS(arg_node));
    */
    /* Since the name in most (all?) cases is static, please no freeing!
      FREE(ICM_NAME(arg_node));
     */

    DBUG_PRINT ("FREE", ("Removing N_icm node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

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

    /*
    FreeAllIds(PRAGMA_NEEDTYPES(arg_node));
    FREETRAV(PRAGMA_NEEDFUNS(arg_node));
    */

    DBUG_PRINT ("FREE", ("Removing N_pragma node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

node *
FreeInfo (node *arg_node, node *arg_info)
{
    node *tmp = NULL;

    DBUG_ENTER ("FreeInfo");

    DBUG_PRINT ("FREE", ("Removing N_info node ..."));

    FREE (arg_node);

    DBUG_RETURN (tmp);
}

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

/********************************************************************/

#if 0



void FreeIds(ids * ids)

{
  DBUG_ENTER("FreeIds");
  if(ids->next != NULL)
    FreeIds(ids->next);
  DBUG_ASSERT((ids->id!=NULL),"Ids without name of identificator");
  FREE(ids->id);
  FREE(ids);
  DBUG_VOID_RETURN;
}

void FreeIdsOnly(ids * ids)

{
  DBUG_ENTER("FreeIdsOnly");
  if(ids->next != NULL)
    FreeIdsOnly(ids->next);
  FREE(ids);
  DBUG_VOID_RETURN;
}

void FreeImplist(node * implist)

{
  int i;

  DBUG_ENTER("FreeImplist");
  if(implist->node[0] != NULL)
    FreeImplist(implist->node[0]);
  for(i=1; i<MAX_SONS; i++)
    if(implist->node[i] != NULL)
      FreeIds((ids *)implist->node[i]);
  FREE(implist);
  DBUG_VOID_RETURN;
}

void FreeMask(node *arg_node)
{
  int i;
  DBUG_ENTER("FreeMask");
  for(i=0;i<MAX_MASK;i++)
    {
    if (arg_node->mask[i]!=NULL)
      FREE(arg_node->mask[i]);
    }
  DBUG_VOID_RETURN;
}

void FreeShpseg(shpseg *shpseg)
{
  DBUG_ENTER("FreeTypes");
  if (shpseg->next!=NULL)
    FreeShpseg(shpseg->next);
  FREE(shpseg);
  DBUG_VOID_RETURN;
}

void FreeTypes(types *type)
{
  DBUG_ENTER("FreeTypes");
  if (type->next!=NULL)
    FreeTypes(type->next);
  if (type->shpseg!=NULL)
    FreeShpseg(type->shpseg);
  if (type->name!=NULL)
    FREE(type->name);
  if (type->id!=NULL)
    FREE(type->id);
  if (type->name_mod!=NULL)
    FREE(type->name_mod);
  if (type->id_mod!=NULL)
    FREE(type->id_mod);
  DBUG_VOID_RETURN;
}

void FreeVarArg(node *var)
{
  DBUG_ENTER("FreeVarArg");
  if (var->node[0]!=NULL)
    FreeVarArg(var->node[0]);
  if (var->info.types!=NULL) 
    FreeTypes(var->info.types);
  FREE(var);
  DBUG_VOID_RETURN;
}

node *FreeModul(node *arg_node, node *arg_info)
{
  DBUG_ENTER("FreeModul");
  if (arg_node->node[0]!=NULL)
    arg_node->node[0]=Trav(arg_node->node[0],arg_info);
  if (arg_node->node[1]!=NULL)
    arg_node->node[1]=Trav(arg_node->node[1],arg_info);
  if (arg_node->node[2]!=NULL)
    arg_node->node[2]=Trav(arg_node->node[2],arg_info);
  if (arg_node->node[3]!=NULL)
    arg_node->node[3]=Trav(arg_node->node[3],arg_info);
  if (arg_node->node[4]!=NULL)
    arg_node->node[4]=Trav(arg_node->node[4],arg_info);
  FREE(arg_node);
  DBUG_RETURN((node *)NULL);
}

node *FreeNoInfo(node *arg_node, node *arg_info)
{
  int i;
  
  DBUG_ENTER("FreeNoInfo");
#ifndef NEWTREE
  for(i=0; i<arg_node->nnode; i++)
#else
  for(i=0; i<MAX_SONS; i++)
    if (arg_node->node[i] != NULL)
#endif
      arg_node->node[i]=Trav(arg_node->node[i],arg_info);
  FreeMask(arg_node);
  FREE(arg_node);
  DBUG_RETURN((node *)NULL);
}

node *FreeInfoIds(node *arg_node, node *arg_info)
{
  int i;
  
  DBUG_ENTER("FreeInfoIds");
#ifndef NEWTREE
  for(i=0; i<arg_node->nnode; i++)
#else
  for(i=0; i<MAX_SONS; i++)
    if (arg_node->node[i] != NULL)
#endif
      arg_node->node[i]=Trav(arg_node->node[i],arg_info);
  FreeMask(arg_node);
  FREE(arg_node);
  DBUG_RETURN((node *)NULL);
}

node *FreeInfoId(node *arg_node, node *arg_info)
{
  int i;
  
  DBUG_ENTER("FreeInfoId");
#ifndef NEWTREE
  for(i=0; i<arg_node->nnode; i++)
#else
  for(i=0; i<MAX_SONS; i++)
    if (arg_node->node[i] != NULL)
#endif
      arg_node->node[i]=Trav(arg_node->node[i],arg_info);
  FreeMask(arg_node);
  FREE(arg_node->info.id);
  FREE(arg_node);
  DBUG_RETURN((node *)NULL);
}

node *FreeInfoType(node *arg_node, node *arg_info)
{
  int i;

  DBUG_ENTER("FreeInfoType");
#ifndef NEWTREE
  for(i=0; i<arg_node->nnode; i++)
#else
  for(i=0; i<MAX_SONS; i++)
#endif
    if(NULL != arg_node->node[i])
    {
      arg_node->node[i]=Trav(arg_node->node[i],arg_info);
    }
  FreeMask(arg_node);
  FreeTypes(arg_node->info.types);
  FREE(arg_node);
  DBUG_RETURN((node *)NULL);
}

/*
 *
 *  functionname  : FreeTree
 *  arguments     : 1) ptr to root of the (sub)tree
 *  description   : changes to functionlist free_tab, which frees the (sub)tree
 *  global vars   : syntax_tree, act_tab, free_tab
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
void FreeTree(node *arg_node)
{
  funptr *tmp_tab;
  
  DBUG_ENTER("FreeTree");
  tmp_tab=act_tab;
  act_tab=free_tab;
  arg_node=Trav(arg_node, NULL);
  act_tab=tmp_tab;
  DBUG_VOID_RETURN;
}

#endif /* 0 */
