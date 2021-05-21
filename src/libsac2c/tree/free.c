#include "free.h"

#define DBUG_PREFIX "FREE"
#include "debug.h"

#include "traverse.h"
#include "str.h"
#include "memory.h"
#include "free_info.h"
#include "tree_basic.h"
#include "new_types.h"
#include "namespaces.h"
#include "globals.h"

/*
 * static global variables
 */

static int num_zombies = 0;

/*
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FREE_FLAG (result) = NULL;
    INFO_FREE_ASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

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
    DBUG_ENTER ();

    DBUG_PRINT ("Removing index info (WLF)");

    DBUG_ASSERT (fr != NULL, "cannot free a NULL index info (WLF)!");

    fr->permutation = MEMfree (fr->permutation);
    fr->last = MEMfree (fr->last);
    fr->const_arg = MEMfree (fr->const_arg);

    fr = MEMfree (fr);

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

    DBUG_ENTER ();

    while (list != NULL) {
        tmp = list;
        list = NODELIST_NEXT (list);
        tmp = MEMfree (tmp);
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

    DBUG_ENTER ();

    DBUG_ASSERT (nl != NULL, "argument is NULL");

    tmp = nl;
    nl = NODELIST_NEXT (nl);
    tmp = MEMfree (tmp);

    DBUG_RETURN (nl);
}

/*--------------------------------------------------------------------------*/

access_t *
FREEfreeOneAccess (access_t *fr)
{
#if 0
  access_t *tmp;
#endif

    DBUG_ENTER ();

    if (fr != NULL) {
#if 0
    DBUG_ASSERT (((NODE_TYPE (ACCESS_IV (fr)) == N_vardec) ||
                  (NODE_TYPE (ACCESS_IV (fr)) == N_arg)), "ACCESS_IV is neither a N_vardec- nor a N_arg-node!");

    DBUG_ASSERT (((NODE_TYPE (ACCESS_ARRAY (fr)) == N_vardec) ||
                  (NODE_TYPE (ACCESS_ARRAY (fr)) == N_arg)), "ACCESS_ARRAY is neither a N_vardec- nor a N_arg-node!");

    DBUG_PRINT ("Removing Access: sel(%s, %s)",
                         VARDEC_OR_ARG_NAME (ACCESS_IV (fr)),
                         VARDEC_OR_ARG_NAME (ACCESS_ARRAY (fr)));

    tmp = fr;
    fr = ACCESS_NEXT (fr);

    if (ACCESS_OFFSET (tmp) != NULL) {
      ACCESS_OFFSET (tmp) = FREEfreeShpseg (ACCESS_OFFSET (tmp));
    }

    tmp = MEMfree (tmp);
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
    DBUG_ENTER ();

    while (fr != NULL) {
        fr = FREEfreeOneAccess (fr);
    }

    DBUG_RETURN (fr);
}

/*--------------------------------------------------------------------------*/

argtab_t *
FREEfreeArgtab (argtab_t *argtab)
{
    DBUG_ENTER ();

    DBUG_ASSERT (argtab != NULL, "argument is NULL");

    argtab->ptr_in = MEMfree (argtab->ptr_in);
    argtab->ptr_out = MEMfree (argtab->ptr_out);
    argtab->tag = MEMfree (argtab->tag);
    argtab->size = 0;

    argtab = MEMfree (argtab);

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
    info *arg_info;
    bool store_valid_ssaform;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    INFO_FREE_FLAG (arg_info) = free_node;

    /*
     * During the free traversal we may temporarily violate SSA form.
     */
    store_valid_ssaform = global.valid_ssaform;
    global.valid_ssaform = FALSE;

    TRAVpush (TR_free);

    free_node = TRAVdo (free_node, arg_info);

    TRAVpop ();

    global.valid_ssaform = store_valid_ssaform;

    arg_info = FreeInfo (arg_info);

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
    info *arg_info;
    bool store_valid_ssaform;

    DBUG_ENTER ();

    arg_info = MakeInfo ();
    INFO_FREE_FLAG (arg_info) = NULL;

    /*
     * During the free traversal we may temporarily violate SSA form.
     */
    store_valid_ssaform = global.valid_ssaform;
    global.valid_ssaform = FALSE;

    TRAVpush (TR_free);

    free_node = TRAVdo (free_node, arg_info);

    TRAVpop ();

    global.valid_ssaform = store_valid_ssaform;

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (free_node);
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
static node *
FreeZombie (node *fundef)
{
    node *tmp;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "FreeZombie() is suitable for N_fundef nodes only!");

    if (FUNDEF_ISZOMBIE (fundef)) {

        DBUG_PRINT ("removing zombie at " F_PTR, (void *)fundef);

        /*
         * remove all the zombie data
         */
        FUNDEF_NAME (fundef) = MEMfree (FUNDEF_NAME (fundef));
        FUNDEF_NS (fundef) = NSfreeNamespace (FUNDEF_NS (fundef));
        FUNDEF_IMPL (fundef) = NULL;

        if (FUNDEF_WRAPPERTYPE (fundef) != NULL) {
            FUNDEF_WRAPPERTYPE (fundef) = TYfreeType (FUNDEF_WRAPPERTYPE (fundef));
        }

        tmp = fundef;
        fundef = FUNDEF_NEXT (fundef);

        /* free entire structure */
        tmp->sons.N_fundef = NULL;
        tmp->attribs.N_fundef = NULL;
        tmp = MEMfree (tmp);

        num_zombies -= 1;
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * Function:
 *   node *FREEremoveAllZombies( node *arg_node)
 *
 * Description:
 *   Removes all zombies found the given N_fundef chain
 *
 ******************************************************************************/

node *
FREEremoveAllZombies (node *arg_node)
{
    DBUG_ENTER ();

    DBUG_ASSERT (arg_node != NULL, "FREEremoveAllZombies called with argument NULL");

    if (global.local_funs_grouped && (FUNDEF_LOCALFUNS (arg_node) != NULL)
        && (num_zombies > 0)) {
        FUNDEF_LOCALFUNS (arg_node) = FREEremoveAllZombies (FUNDEF_LOCALFUNS (arg_node));
    }

    if ((FUNDEF_NEXT (arg_node) != NULL) && (num_zombies > 0)) {
        FUNDEF_NEXT (arg_node) = FREEremoveAllZombies (FUNDEF_NEXT (arg_node));
    }

    arg_node = FreeZombie (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn node *FREEzombify( node *arg_node)
 *
 * @brief Marks function as zombie
 *
 * @param arg_node Fundef node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
FREEzombify (node *arg_node)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 "Only N_fundef nodes may be zombified.");

    FUNDEF_ISZOMBIE (arg_node) = TRUE;

    FUNDEF_ISEXPORTED (arg_node) = FALSE;
    FUNDEF_ISPROVIDED (arg_node) = FALSE;
    FUNDEF_WASIMPORTED (arg_node) = FALSE;
    FUNDEF_WASUSED (arg_node) = FALSE;
    FUNDEF_ISLOCAL (arg_node) = FALSE;
    FUNDEF_ISSTICKY (arg_node) = FALSE;
    FUNDEF_ISSACARGCONVERSION (arg_node) = FALSE;
    FUNDEF_ISNEEDED (arg_node) = FALSE;
    FUNDEF_ISCONDFUN (arg_node) = FALSE;
    FUNDEF_ISLOOPFUN (arg_node) = FALSE;
    FUNDEF_ISSPMDFUN (arg_node) = FALSE;
    FUNDEF_ISMTFUN (arg_node) = FALSE;
    FUNDEF_ISSTFUN (arg_node) = FALSE;
    FUNDEF_ISXTFUN (arg_node) = FALSE;
    FUNDEF_ISWRAPPERFUN (arg_node) = FALSE;
    FUNDEF_ISEXTERN (arg_node) = FALSE;
    FUNDEF_ISGENERIC (arg_node) = FALSE;
    FUNDEF_ISINLINE (arg_node) = FALSE;
    FUNDEF_ISINLINECOMPLETED (arg_node) = FALSE;
    FUNDEF_ISLACINLINE (arg_node) = FALSE;
    FUNDEF_ALLOWSINFIX (arg_node) = FALSE;
    FUNDEF_HASDOTARGS (arg_node) = FALSE;
    FUNDEF_HASDOTRETS (arg_node) = FALSE;
    FUNDEF_ISSPECIALISATION (arg_node) = FALSE;
    FUNDEF_ISOBJECTWRAPPER (arg_node) = FALSE;
    FUNDEF_WASOPTIMIZED (arg_node) = FALSE;
    FUNDEF_WASUPGRADED (arg_node) = FALSE;
    FUNDEF_FIXPOINTFOUND (arg_node) = FALSE;
    /*
     * It would be quite handy here to use an XML generated
     * function to clear all existing flags at once.
     */

    num_zombies += 1;

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
