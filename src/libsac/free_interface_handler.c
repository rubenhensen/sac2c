/*
 * $Log$
 * Revision 1.1  2000/07/20 11:38:07  nmw
 * Initial revision
 *
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "sac.h"
#include "sac_free_interface_handler.h"

typedef struct ACT_MOD_S {
    void (*fun) ();
    struct ACT_MOD_S *next;
} act_modules_t;

act_modules_t *head = NULL;

/******************************************************************************
 *
 * function:
 *   void SAC_FIH_AddFreeFunction( void (*freefun)() )
 *
 * description:
 *   adds given free function of the calling module to the internal list
 *   of module free functions to call at cleanup
 *
 ******************************************************************************/

void
SAC_FIH_AddFreeFunction (void (*freefun) ())
{
    act_modules_t *new;

    new = (act_modules_t *)SAC_MALLOC (sizeof (act_modules_t));
    new->fun = freefun;
    new->next = head;
    head = new;
}

/******************************************************************************
 *
 * function:
 *   void SAC_FIH_FreeAllModules()
 *
 * description:
 *   calls all free function of used modules
 *
 ******************************************************************************/

void
SAC_FIH_FreeAllModules ()
{
    act_modules_t *mod;

    mod = head;
    while (mod != NULL) {
        (*(mod->fun)) (); /* call this modules freefun */
        mod = mod->next;
    }
}
