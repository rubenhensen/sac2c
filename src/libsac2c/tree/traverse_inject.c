/**
 * @file
 *
 * Functions below are used to create, add to, remove from, or free
 * a traversal function list which is used for pre/post-function
 * calls (see traverse.c).
 *
 * @see traverse.c
 * @see types.h
 */
#include "traverse_inject.h"
#define DBUG_PREFIX "TRAVINJ"
#include "debug.h"
#include "memory.h"
#include "tree_basic.h"
#include "traverse.h"
#include "traverse_tables.h"

/**
 * @brief Creates a travfunlist, which is a linked-list of function pointers.
 *        See types.h for more details on the structure.
 *
 * @param fun       function pointer
 * @param traversal enum of traversal at which point we no longer want to call
 *                  the function pointer `fun'.
 * @return the travfunlist
 */
travfunlist_t *
TRAVmakeTravFunList (travfun_p fun)
{
    travfunlist_t *tmp;
    DBUG_ENTER ();

    DBUG_ASSERT (fun != NULL, "Traversal function is NULL!");

    tmp = MEMmalloc (sizeof (travfunlist_t));
    tmp->fun = fun;
    tmp->next = NULL;

    DBUG_RETURN (tmp);
}

/**
 * @brief Appends a new funlist to the end of the travfunlist. Function makes sure
 *        that there exists only one reference to a function pointer - duplicates
 *        are freed instead of being added to the list.
 *
 * @param funlist    travfunlist
 * @param newfunlist new travfunlist to append to funlist
 * @return the travfunlist
 */
travfunlist_t *
TRAVappendTravFunList (travfunlist_t *funlist, travfunlist_t *newfunlist)
{
    travfunlist_t * tmp = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (funlist != NULL, "Traversal function list is NULL!");
    DBUG_ASSERT (newfunlist != NULL, "New traversal function list is NULL!");

    tmp = funlist;
    while (tmp->fun != newfunlist->fun && tmp->next != NULL)
    {
        tmp = tmp->next;
    }

    if (tmp->fun != newfunlist->fun)
        tmp->next = newfunlist;
    else
        newfunlist = MEMfree (newfunlist);

    DBUG_RETURN (funlist);
}

/**
 * @brief Removes function pointer from the travfunlist. It is assumed that there
 *        will only ever be *one* instance of a given function pointer in the
 *        entire travfunlist - undefined behaviour otherwise.
 *
 * @param funlist  travfunlist
 * @param fun      function pointer
 * @return the travfunlist
 */
travfunlist_t *
TRAVremoveTravFunListFun (travfunlist_t *funlist, travfun_p fun)
{
    travfunlist_t * tmp, * prev;

    DBUG_ENTER ();

    DBUG_ASSERT (funlist != NULL, "Traversal function list is NULL!");
    DBUG_ASSERT (fun != NULL, "Traversal function is NULL!");

    if (funlist->fun == fun)
    {
        funlist = TRAVfreeTravFunListNode (funlist);
    }

    tmp = funlist;
    prev = NULL;
    while (tmp) {
        if (tmp->fun == fun)
            prev->next = TRAVfreeTravFunListNode (tmp);
        else
            prev = tmp;

        tmp = prev->next;
    }

    DBUG_RETURN (funlist);
}

/**
 * @brief Recursively free a travfunlist.
 *
 * @param funlist  travfunlist
 * @return NULL _or_ pointer to statically declared travfunlist node.
 */
travfunlist_t *
TRAVfreeTravFunList (travfunlist_t *funlist)
{
   DBUG_ENTER ();

   while (funlist)
   {
      funlist = TRAVfreeTravFunListNode (funlist);
   }

   DBUG_RETURN (funlist);
}

/**
 * @brief Free a single travfunlist node and attach its next to the previous one.
 *
 * @param funlist  travfunlist
 * @return NULL _or_ pointer to next travfunlist.
 */
travfunlist_t *
TRAVfreeTravFunListNode (travfunlist_t *funlist)
{
    travfunlist_t * tmp;
    DBUG_ENTER ();

    DBUG_ASSERT (funlist != NULL, "Traversal function list is NULL!");

    tmp = funlist;
    funlist = funlist->next;
    tmp = MEMfree (tmp);

    DBUG_RETURN (funlist);
}

/**
 * @brief adds the given function pointer to the prefun table stack.
 *
 * @param traversal  the traversal to attach function to
 * @param prefun     function pointer
 */
void
TRAVaddPreFun (trav_t traversal, travfun_p prefun)
{
    travfunlist_t *tmp;
    DBUG_ENTER ();

    if (pretable[traversal] == NULL) {
        pretable[traversal] = TRAVmakeTravFunList (prefun);
    } else {
        tmp = TRAVmakeTravFunList (prefun);
        pretable[traversal] = TRAVappendTravFunList (pretable[traversal], tmp);
    }

    DBUG_RETURN ();
}

/**
 * @brief removes the given function pointer from the prefun table stack.
 *
 * @param traversal  the traversal to remove function from
 * @param prefun     function pointer
 */
void
TRAVremovePreFun (trav_t traversal, travfun_p prefun)
{
    DBUG_ENTER ();

    if (pretable[traversal] != NULL) {
        pretable[traversal] = TRAVremoveTravFunListFun (pretable[traversal], prefun);
    }

    DBUG_RETURN ();
}

/**
 * @brief adds the given function pointer to the postfun table stack.
 *
 * @param traversal  the traversal to add function to
 * @param prefun     function pointer
 */
void
TRAVaddPostFun (trav_t traversal, travfun_p postfun)
{
    travfunlist_t *tmp;
    DBUG_ENTER ();

    if (posttable[traversal] == NULL) {
        posttable[traversal] = TRAVmakeTravFunList (postfun);
    } else {
        tmp = TRAVmakeTravFunList (postfun);
        posttable[traversal] = TRAVappendTravFunList (posttable[traversal], tmp);
    }

    DBUG_RETURN ();
}

/**
 * @brief removes the given function pointer from the postfun table stack.
 *
 * @param traversal  the traversal to remove function from
 * @param prefun     function pointer
 */
void
TRAVremovePostFun (trav_t traversal, travfun_p postfun)
{
    DBUG_ENTER ();

    if (posttable[traversal] != NULL) {
        posttable[traversal] = TRAVremoveTravFunListFun (posttable[traversal], postfun);
    }

    DBUG_RETURN ();
}
