/*
 *
 * $Log$
 * Revision 3.72  2004/11/23 10:04:04  sah
 * SaC DevCamp 04
 *
 * Revision 3.71  2004/11/08 16:35:23  sah
 * as I found out today, FUNDEF_IMPL is needed for zombie funs as
 * well in order to dispatch correctly in create_wrapper_code.c.
 * So now that attribute is not freed as well.
 *
 * [...]
 *
 */

#define NEW_INFO

#include "free.h"

/*
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_FREE_FLAG (result) = NULL;
    INFO_FREE_ASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*
 *  Important Remarks:
 *
 *  All removals of parts of or the entire syntax tree should be done
 *  by using the functions in this file.
 *
 *
 */

/*--------------------------------------------------------------------------*/
/*  Free-functions for non-node structures                                  */
/*--------------------------------------------------------------------------*/

/*
 *  Basically, there are two different free-functions for each non-node
 *  structure:
 *             FREEfree<struct> and FREEfreeAll<struct>
 *
 *  Both get a pointer to the respective structure as argument.
 *
 *  FREEfree<struct> removes only the structure referenced by the argument
 *  and returns a pointer to the next structure in the chain of structures.
 *  These functions are useful to eliminate a single structure from a list.
 *
 *  FREEfreeAll<struct> removes the whole sub tree which is referenced.
 *  All elements of list are freed in this case.
 */

/*--------------------------------------------------------------------------*/

index_info *
FREEfreeIndexInfo (index_info *fr)
{
    DBUG_ENTER ("FREEfreeIndexInfo");

    DBUG_PRINT ("FREE", ("Removing index info (WLF)"));

    DBUG_ASSERT ((fr != NULL), "cannot free a NULL index info (WLF)!");

    fr->permutation = ILIBfree (fr->permutation);
    fr->last = ILIBfree (fr->last);
    fr->const_arg = ILIBfree (fr->const_arg);

    fr = ILIBfree (fr);

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

shpseg *
FREEfreeShpseg (shpseg *fr)
{
    DBUG_ENTER ("FREEfreeShpseg");

    DBUG_PRINT ("FREE", ("Removing shpseg"));

    DBUG_ASSERT ((fr != NULL), "cannot free a NULL shpseg!");

    if (SHPSEG_NEXT (fr) != NULL) {
        SHPSEG_NEXT (fr) = FREEfreeShpseg (SHPSEG_NEXT (fr));
    }

    fr = ILIBfree (fr);

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

types *
FREEfreeOneTypes (types *fr)
{
    types *tmp;

    DBUG_ENTER ("FREEfreeOneTypes");

    if (fr != NULL) {
        DBUG_PRINT ("FREE", ("Removing types: %s", mdb_type[TYPES_BASETYPE (fr)]));
        tmp = fr;
        fr = TYPES_NEXT (fr);

        if (TYPES_DIM (tmp) > 0) {
            DBUG_ASSERT ((TYPES_SHPSEG (tmp) != NULL),
                         "SHPSEG not found although DIM is greater 0");
            TYPES_SHPSEG (tmp) = FREEfreeShpseg (TYPES_SHPSEG (tmp));
        }
        TYPES_NAME (tmp) = ILIBfree (TYPES_NAME (tmp));
        TYPES_MOD (tmp) = ILIBfree (TYPES_MOD (tmp));

        tmp = ILIBfree (tmp);
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

types *
FREEfreeAllTypes (types *fr)
{
    DBUG_ENTER ("FREEfreeAllTypes");

    while (fr != NULL) {
        fr = FREEfreeOneTypes (fr);
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

ids *
FREEfreeOneIds (ids *fr)
{
    ids *tmp;

    DBUG_ENTER ("FREEfreeOneIds");

    if (fr != NULL) {
        DBUG_PRINT ("FREE", ("Removing ids: %s", IDS_NAME (fr)));

        tmp = fr;
        fr = IDS_NEXT (fr);

        IDS_NAME (tmp) = ILIBfree (IDS_NAME (tmp));

        tmp = ILIBfree (tmp);
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

ids *
FREEfreeAllIds (ids *fr)
{
    DBUG_ENTER ("FREEfreeAllIds");

    while (fr != NULL) {
        fr = FREEfreeOneIds (fr);
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

/*
 *  FREEfreeNodelist always frees entire list.
 */

nodelist *
FREEfreeNodelist (nodelist *list)
{
    nodelist *tmp;

    DBUG_ENTER ("FREEfreeNodelist");

    while (list != NULL) {
        tmp = list;
        list = NODELIST_NEXT (list);
        tmp = ILIBfree (tmp);
    }

    DBUG_RETURN (list);
}

/*--------------------------------------------------------------------------*/

/*
 *  FREEfreeNodelistNode free a nodelist item and return the next item
 */

nodelist *
FREEfreeNodelistNode (nodelist *nl)
{
    nodelist *tmp;

    DBUG_ENTER ("FREEfreeNodelistNode");

    DBUG_ASSERT ((nl != NULL), "argument is NULL");

    tmp = nl;
    nl = NODELIST_NEXT (nl);
    tmp = ILIBfree (tmp);

    DBUG_RETURN (nl);
}

/*--------------------------------------------------------------------------*/

access_t *
FREEfreeOneAccess (access_t *fr)
{
#if 0
  access_t *tmp;
#endif

    DBUG_ENTER ("FREEfreeOneAccess");

    if (fr != NULL) {
#if 0
    DBUG_ASSERT( ((NODE_TYPE( ACCESS_IV( fr)) == N_vardec) ||
                  (NODE_TYPE( ACCESS_IV( fr)) == N_arg)),
                 "ACCESS_IV is neither a N_vardec- nor a N_arg-node!");

    DBUG_ASSERT( ((NODE_TYPE( ACCESS_ARRAY( fr)) == N_vardec) ||
                  (NODE_TYPE( ACCESS_ARRAY( fr)) == N_arg)),
                 "ACCESS_ARRAY is neither a N_vardec- nor a N_arg-node!");

    DBUG_PRINT( "FREE", ("Removing Access: sel(%s, %s)", 
                         VARDEC_OR_ARG_NAME( ACCESS_IV( fr)),
                         VARDEC_OR_ARG_NAME( ACCESS_ARRAY( fr))));

    tmp = fr;
    fr = ACCESS_NEXT( fr);
    
    if (ACCESS_OFFSET( tmp) != NULL) {
      ACCESS_OFFSET( tmp) = FREEfreeShpseg( ACCESS_OFFSET( tmp));
    }
    
    tmp = ILIBfree( tmp);
#else
        fr = NULL;
#endif
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

access_t *
FREEfreeAllAccess (access_t *fr)
{
    DBUG_ENTER ("FREEfreeAllAccess");

    while (fr != NULL) {
        fr = FREEfreeOneAccess (fr);
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

argtab_t *
FREEfreeArgtab (argtab_t *argtab)
{
    DBUG_ENTER ("FREEfreeArgtab");

    DBUG_ASSERT ((argtab != NULL), "argument is NULL");

    argtab->ptr_in = FREEfree (argtab->ptr_in);
    argtab->ptr_out = FREEfree (argtab->ptr_out);
    argtab->tag = FREEfree (argtab->tag);
    argtab->size = 0;

    argtab = FREEfree (argtab);

    DBUG_RETURN (argtab);
}

/******************************************************************************
 *
 * Function:
 *   node *FREEdoFreeNode( node *free_node)
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
FREEdoFreeNode (node *free_node)
{
    funtab *store_tab;
    info *arg_info;

    DBUG_ENTER ("FREEfreeNode");

    store_tab = act_tab;
    act_tab = free_tab;

    arg_info = MakeInfo ();
    INFO_FREE_FLAG (arg_info) = free_node;

    free_node = Trav (free_node, arg_info);

    arg_info = FreeInfo (arg_info);

    act_tab = store_tab;

    DBUG_RETURN (free_node);
}

/******************************************************************************
 *
 * Function:
 *   node *FREEdoFreeTree( node *free_node)
 *
 * Description:
 *   - if 'free_node' is not a N_fundef node:
 *        Removes the whole sub tree behind the given pointer.
 *   - if 'free_node' is a N_fundef node:
 *        Transforms the whole fundef chain into zombies and returns it.
 *
 ******************************************************************************/

node *
FREEdoFreeTree (node *free_node)
{
    funtab *store_tab;
    info *arg_info;

    DBUG_ENTER ("FREEfreeTree");

    store_tab = act_tab;
    act_tab = free_tab;

    arg_info = MakeInfo ();
    INFO_FREE_FLAG (arg_info) = NULL;

    free_node = Trav (free_node, arg_info);

    arg_info = FreeInfo (arg_info);

    act_tab = store_tab;

    DBUG_RETURN (free_node);
}

/******************************************************************************
 *
 * Function:
 *   node *FREEfreeZombie( node *fundef)
 *
 * Description:
 *   - if head of 'fundef' is a zombie:
 *       Removes the head and returns the tail.
 *   - if head if 'fundef' is not a zombie:
 *       Returns 'fundef' in unmodified form.
 *
 ******************************************************************************/

node *
FREEfreeZombie (node *fundef)
{
    node *tmp;

    DBUG_ENTER ("FREEfreeZombie");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "FREEfreeZombie() is suitable for N_fundef nodes only!");

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
        FUNDEF_NAME (fundef) = ILIBfree (FUNDEF_NAME (fundef));
#if FREE_MODNAMES
        FUNDEF_MOD (fundef) = ILIBfree (FUNDEF_MOD (fundef));
        FUNDEF_LINKMOD (fundef) = ILIBfree (FUNDEF_LINKMOD (fundef));
#endif
        FUNDEF_IMPL (fundef) = NULL;

        FUNDEF_TYPES (fundef) = FREEfreeOneTypes (FUNDEF_TYPES (fundef));
        if (FUNDEF_RET_TYPE (fundef) != NULL) {
            FUNDEF_RET_TYPE (fundef) = TYfreeType (FUNDEF_RET_TYPE (fundef));
        }
        if (FUNDEF_TYPE (fundef) != NULL) {
            FUNDEF_TYPE (fundef) = TYfreeType (FUNDEF_TYPE (fundef));
        }

        tmp = fundef;
        fundef = FUNDEF_NEXT (fundef);

        /* free entire structure */
        tmp->attribs.any = ILIBfree (tmp->attribs.any);
        tmp = ILIBfree (tmp);
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * Function:
 *   node *FREEremoveAllZombies( node *arg_node)
 *
 * Description:
 *   Removes all zombies found the given N_modul node or N_fundef chain
 *
 ******************************************************************************/

node *
FREEremoveAllZombies (node *arg_node)
{
    DBUG_ENTER ("FREEremoveAllZombies");

    DBUG_ASSERT ((arg_node != NULL), "FREEremoveAllZombies called with argument NULL");

    switch (NODE_TYPE (arg_node)) {
    case N_modul:
        if (MODUL_FUNS (arg_node) != NULL) {
            MODUL_FUNS (arg_node) = FREEremoveAllZombies (MODUL_FUNS (arg_node));
        }
        if (MODUL_FUNDECS (arg_node) != NULL) {
            MODUL_FUNDECS (arg_node) = FREEremoveAllZombies (MODUL_FUNDECS (arg_node));
        }

        break;

    case N_fundef:
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = FREEremoveAllZombies (FUNDEF_NEXT (arg_node));
        }

        arg_node = FREEfreeZombie (arg_node);
        break;

    default:
        DBUG_ASSERT ((0), "FREEremoveAllZombies() is suitable for N_modul and"
                          " N_fundef nodes only!");
        arg_node = NULL;
        break;
    }

    DBUG_RETURN (arg_node);
}
